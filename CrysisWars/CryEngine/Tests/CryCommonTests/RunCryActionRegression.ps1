param([switch]$Asan, [string]$GameRoot = 'G:/projects/MWEL-testbed', [string]$Family, [string]$RendererRuntime)

$ErrorActionPreference = 'Stop'
$PSNativeCommandUseErrorActionPreference = $false
$root = (Resolve-Path (Join-Path $PSScriptRoot '../../..')).Path
$lane = if ($Asan) { 'AsanProfile' } else { 'Profile' }
$runnerLane = if ($Asan) { 'AsanRunner' } else { 'Profile' }
$stage = Join-Path $root "Bin64/Tests/CryActionRegression/$lane"
$runner = if ($Asan) { Join-Path $root "Bin64/Tests/CryActionRegression/$runnerLane" } else { Join-Path $root 'Bin64/Tests/Profile' }
$logs = Join-Path $root 'BinTemp'
$profileRuntime = Join-Path $root 'Bin64_Profile'
if ($Family -and $Family -notmatch '^[A-Z]{2,3}[0-9]{2}$') { throw 'Invalid family name; use a plan identifier such as AN04.' }
$reportLane = if ($Family) { "$lane-$Family" } else { $lane }
$msbuild = Join-Path ${env:ProgramFiles} 'Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe'
if (!(Test-Path -LiteralPath $msbuild)) { throw "MSBuild not found: $msbuild" }

function Invoke-ProjectBuild([string]$Project, [string[]]$Properties, [string]$Log) {
    $start = [System.Diagnostics.ProcessStartInfo]::new()
    $start.FileName = $msbuild
    $start.WorkingDirectory = $root
    $start.UseShellExecute = $false
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    foreach ($argument in (@($Project, '/m', '/v:minimal', '/p:Configuration=Profile', '/p:Platform=x64',
        '/p:BuildProjectReferences=false', '/clp:ErrorsOnly;Summary', "/flp:logfile=$Log;verbosity=normal") + $Properties)) {
        $start.ArgumentList.Add($argument)
    }
    # Windows environment names are case-insensitive; inherited Path/PATH duplicates break MSBuild.
    $environment = [System.Collections.Generic.Dictionary[string,string]]::new([StringComparer]::OrdinalIgnoreCase)
    foreach ($entry in [Environment]::GetEnvironmentVariables().GetEnumerator()) {
        $environment[[string]$entry.Key] = [string]$entry.Value
    }
    $start.Environment.Clear()
    foreach ($entry in $environment.GetEnumerator()) { $start.Environment.Add($entry.Key, $entry.Value) }
    $process = [System.Diagnostics.Process]::Start($start)
    $stdout = $process.StandardOutput.ReadToEndAsync()
    $stderr = $process.StandardError.ReadToEndAsync()
    $process.WaitForExit()
    Write-Host $stdout.GetAwaiter().GetResult()
    Write-Host $stderr.GetAwaiter().GetResult()
    if ($process.ExitCode) { throw "Build failed ($($process.ExitCode)): $Log" }
}

$production = Join-Path $profileRuntime 'CryAction.dll'
$before = (Get-FileHash -LiteralPath $production -Algorithm SHA256).Hash
$systemProduction = Join-Path $profileRuntime 'CrySystem.dll'
$systemBefore = (Get-FileHash -LiteralPath $systemProduction -Algorithm SHA256).Hash
$physicsProduction = Join-Path $profileRuntime 'CryPhysics.dll'
$physicsBefore = (Get-FileHash -LiteralPath $physicsProduction -Algorithm SHA256).Hash
New-Item -ItemType Directory -Force -Path $stage, $runner, $logs | Out-Null
$evidencePath = Join-Path $logs "CryAction-regression-$reportLane-evidence.json"
$sourceFiles = @(Get-ChildItem -LiteralPath $PSScriptRoot -File | Where-Object {
    ($_.Name -like 'CryAction*' -or $_.Name -like 'RunCryAction*' -or $_.Name -like 'CryCommonTests.vcxproj*') -and $_.Extension -ne '.md'
}) + @(Get-ChildItem -LiteralPath (Join-Path $root 'CryEngine/CryAction') -Recurse -File | Where-Object {
    $_.Extension -in '.cpp', '.h', '.inl', '.vcxproj', '.rc'
})
$sourceFiles += @(Get-Item -LiteralPath (Join-Path $root 'CryEngine/CrySystem/CryMemoryManager.cpp'),
    (Join-Path $root 'CryEngine/CrySystem/CryAllocationHeader.h'), (Join-Path $root 'CryEngine/CrySystem/CrySystem.vcxproj'),
    (Join-Path $root 'CryEngine/CryPhysics/tetrlattice.cpp'), (Join-Path $root 'CryEngine/CryPhysics/CryPhysics.vcxproj'),
    (Join-Path $root 'CryEngine/RenderDll/Common/Textures/Texture.h'),
    (Join-Path $root 'CryEngine/RenderDll/XRenderD3D9/D3DTexture.cpp'),
    (Join-Path $root 'CryEngine/RenderDll/XRenderD3D9/XRenderD3D10.vcxproj'))
$evidence = [ordered]@{
    started_utc = [DateTime]::UtcNow.ToString('o')
    revision = (& git -C $root rev-parse HEAD)
    dirty_status = @(& git -C $root status --porcelain)
    configuration = 'Profile'; platform = 'x64'; toolset = 'v143'; language = 'C++20'
    asan = [bool]$Asan; family = $Family; seed = '0x43525932'; timeout_ms = 120000
    runtime_directory = $stage; runner_directory = $runner; game_root_read_only_input = $GameRoot
    production_sha256_before = $before
    production_system_sha256_before = $systemBefore
    production_physics_sha256_before = $physicsBefore
    source_sha256 = @($sourceFiles | Get-FileHash -Algorithm SHA256 | Select-Object Path, Hash)
    fixture_policy = 'Literal fixtures are in hashed test sources; generated files are PID-specific and removed after successful cleanup.'
    allocator_scope = 'ASan host and CryAction only; staged CrySystem/pool allocator and other dependencies are not instrumented.'
}
$evidence | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evidencePath -Encoding utf8
$asanProperty = if ($Asan) { @('/p:EnableASAN=true') } else { @() }
Invoke-ProjectBuild 'CryEngine/CryAction/CryAction.vcxproj' (@(
    '/p:CryActionRegressionTests=true', '/p:TargetName=CryActionRegression',
    "/p:OutDir=$stage\", "/p:IntDir=$root\BinTemp\x64\Profile\CryActionRegression$lane\") + $asanProperty) `
    (Join-Path $logs "CryAction-regression-$lane-module-build.log")
$runnerProperties = @()
if ($Asan) {
    $runnerProperties = @("/p:OutDir=$runner\", "/p:IntDir=$root\BinTemp\x64\Profile\CryCommonTestsActionAsan\")
}
Invoke-ProjectBuild 'CryEngine/Tests/CryCommonTests/CryCommonTests.vcxproj' ($runnerProperties + $asanProperty) `
    (Join-Path $logs "CryAction-regression-$lane-runner-build.log")
foreach ($file in Get-ChildItem -LiteralPath $profileRuntime -File) {
    if (($file.Extension -eq '.dll' -or $file.Name.EndsWith('.manifest')) -and !$file.Name.StartsWith('CryActionRegression')) {
        Copy-Item -LiteralPath $file.FullName -Destination $stage -Force
    }
}
if ($RendererRuntime) {
    Copy-Item -LiteralPath $RendererRuntime -Destination (Join-Path $stage 'CryRenderD3D10.dll') -Force
} else {
    Invoke-ProjectBuild 'CryEngine/RenderDll/XRenderD3D9/XRenderD3D10.vcxproj' @(
        '/p:CryActionRegressionTests=true', "/p:OutDir=$stage\",
        "/p:IntDir=$root\BinTemp\x64\Profile\CryRendererActionRegression$lane\") `
        (Join-Path $logs "CryAction-regression-$lane-renderer-build.log")
}
# NW04's recorded seed must reach the generator used by real plane fragmentation.
# Build that dependency into this lane; preserve the normal physics DLL/PDB.
Invoke-ProjectBuild 'CryEngine/CryPhysics/CryPhysics.vcxproj' @(
    '/p:EnableASAN=false', '/p:CryActionRegressionTests=true', "/p:OutDir=$stage\",
    "/p:IntDir=$root\BinTemp\x64\Profile\CryPhysicsActionRegression$lane\") `
    (Join-Path $logs "CryAction-regression-$lane-physics-build.log")
if ($Asan) {
    $runtime = Get-ChildItem -Path (Join-Path ${env:ProgramFiles} 'Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/*/bin/Hostx64/x64/clang_rt.asan_dynamic-x86_64.dll') |
        Sort-Object FullName -Descending | Select-Object -First 1
    if (!$runtime) { throw 'ASan runtime is unavailable; rerun without -Asan for normal validation.' }
    Copy-Item -LiteralPath $runtime.FullName -Destination $stage -Force
    Copy-Item -LiteralPath $runtime.FullName -Destination $runner -Force
    # Instrument the allocation owner too: pooled reuse must reset ASan shadow
    # left by annotated STL containers in CryAction. The Profile output is untouched.
    Copy-Item -LiteralPath (Join-Path $profileRuntime 'pdcurses.lib') -Destination $stage -Force
    Invoke-ProjectBuild 'CryEngine/CrySystem/CrySystem.vcxproj' @(
        '/p:EnableASAN=true', '/p:CryActionAsanRuntime=true', "/p:OutDir=$stage\",
        "/p:IntDir=$root\BinTemp\x64\Profile\CrySystemActionAsan\") `
        (Join-Path $logs 'CryAction-regression-AsanProfile-system-build.log')
    $evidence.allocator_scope = 'ASan host, CryAction and CrySystem; pool allocation/release shadow hooks enabled. Other dependencies remain uninstrumented.'
}
if ((Get-FileHash -LiteralPath $production -Algorithm SHA256).Hash -ne $before) {
    throw 'The test build unexpectedly modified the production CryAction.dll.'
}
$evidence.production_sha256_after = (Get-FileHash -LiteralPath $production -Algorithm SHA256).Hash
$evidence.production_system_sha256_after = (Get-FileHash -LiteralPath $systemProduction -Algorithm SHA256).Hash
if ($evidence.production_system_sha256_after -ne $systemBefore) { throw 'The test build unexpectedly modified the production CrySystem.dll.' }
$evidence.production_physics_sha256_after = (Get-FileHash -LiteralPath $physicsProduction -Algorithm SHA256).Hash
if ($evidence.production_physics_sha256_after -ne $physicsBefore) { throw 'The test build unexpectedly modified the production CryPhysics.dll.' }
$evidence.runtime_sha256 = @(Get-ChildItem -LiteralPath $stage -File -Filter '*.dll' |
    Get-FileHash -Algorithm SHA256 | Select-Object Path, Hash)
$evidence.runner_sha256 = (Get-FileHash -LiteralPath (Join-Path $runner 'CryCommonTests.exe') -Algorithm SHA256).Hash
$evidence | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evidencePath -Encoding utf8
$oldOptions = $env:ASAN_OPTIONS
$oldPrefix = $env:CRY_ACTION_ASAN_LOG_PREFIX
$oldDumpDirectory = $env:CRY_ALLOCATOR_DUMP_DIRECTORY
try {
    $env:CRY_ALLOCATOR_DUMP_DIRECTORY = Join-Path $logs 'CryActionAllocatorDumps'
    New-Item -ItemType Directory -Force -Path $env:CRY_ALLOCATOR_DUMP_DIRECTORY | Out-Null
    if ($Asan) {
        $env:CRY_ACTION_ASAN_LOG_PREFIX = Join-Path $logs ('CryAction-ASan-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff'))
        $env:ASAN_OPTIONS = 'halt_on_error=1:log_path="' + $env:CRY_ACTION_ASAN_LOG_PREFIX.Replace('\', '/') + '"'
    } else { $env:CRY_ACTION_ASAN_LOG_PREFIX = $null }
    $exe = Join-Path $runner 'CryCommonTests.exe'
    & $exe | Tee-Object -FilePath (Join-Path $logs "CryAction-regression-$lane-default-tests.log")
    $baselineResult = $LASTEXITCODE
    if ($baselineResult) { Write-Warning "Existing unit suite failed ($baselineResult); continuing isolated CryAction cases." }
    # The installed asset tree is read-only input; all generated reports/fixtures stay in VC14.
    $caseArguments = @('--cryaction-regression', $stage, $GameRoot, (Join-Path $logs "CryAction-regression-$reportLane.tsv"))
    if ($Family) { $caseArguments += $Family }
    & $exe @caseArguments | Tee-Object -FilePath (Join-Path $logs "CryAction-regression-$reportLane-console.log")
    $result = $LASTEXITCODE
    $rows = @(Import-Csv -LiteralPath (Join-Path $logs "CryAction-regression-$reportLane.tsv") -Delimiter "`t")
    $evidence.finished_utc = [DateTime]::UtcNow.ToString('o')
    $evidence.default_suite_exit = $baselineResult
    $evidence.component_suite_exit = $result
    $evidence.case_count = $rows.Count
    $evidence.status_totals = @($rows | Group-Object status | Select-Object Name, Count)
    $evidence.assertion_failed_cases = @($rows | Where-Object { [int]$_.failed_checks -gt 0 }).Count
    $evidence.failed_checks = ($rows | Measure-Object -Property failed_checks -Sum).Sum
    $evidence.observed_checks = ($rows | Measure-Object -Property checks -Sum).Sum
    $evidence.sanitizer_diagnostic_cases = @($rows | Where-Object asan_report).Count
    $evidence.sanitizer_diagnostic_files = @($rows | Where-Object asan_report | Select-Object -ExpandProperty asan_report -Unique).Count
    $evidence.unexpected_termination_cases = @($rows | Where-Object {
        $_.process_outcome -in 'exception', 'sanitizer-termination', 'incomplete-exit'
    }).Count
    $evidence.counting = 'Each row has one primary status. Assertion failures and sanitizer/unexpected termination are overlapping evidence, never extra cases. Unimplemented cases are excluded.'
    $evidence | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $evidencePath -Encoding utf8
    if ($result -eq 0 -and $baselineResult -ne 0) { $result = 1 }
} finally {
    $env:ASAN_OPTIONS = $oldOptions
    $env:CRY_ACTION_ASAN_LOG_PREFIX = $oldPrefix
    $env:CRY_ALLOCATOR_DUMP_DIRECTORY = $oldDumpDirectory
}
exit $result
