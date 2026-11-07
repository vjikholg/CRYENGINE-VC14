# CryAction Regression Tests

## Full matrix validation (2026-11-06)

The x64 Profile and ASan lanes each passed 240 isolated cases across 33 family
labels and all 97 default tests. Both runs recorded zero failures, crashes,
timeouts, blocked cases, or sanitizer reports. UI01 offscreen rendering passed
by drawing and reading an actual pixel from a D3D10 render target.

The full run supersedes the earlier failure and blocked entries below. The
[local validation archive](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-final-repairs-20261106/REPORT.md)
contains the source and staged runtime hashes. ASan instrumented the host,
CryAction and CrySystem. Physics and the renderer ran as Profile dependencies.
Game startup, asset compatibility, and gameplay smoke testing remain separate
integration work.

## Historical NW04, AN07 and OS01 follow-up (2026-11-06)

The requested production repairs and focused regression coverage are implemented.
NW04 now has 13 cases, AN07 has six, and OS01 has seven. The previous three NW04
aliases have been replaced with actual recording, physics and chunk owner paths.
The manifest contains 240 cases across all 33 planned family labels. This does
not establish completion of every family's planned integration or a green full
suite. See the [repair report, focused results and compatibility limits](/g:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/CryActionRepairProgress.md).

## Historical UI follow-up (2026-11-06)

UI01/UI02 production defects and their allocator-shadow prerequisite are repaired
in source and validated in isolated x64 Profile/ASan component lanes. Each lane
contains 35 UI cases: **34 PASS, 0 FAIL, 0 CRASH, 0 TIMEOUT, 1 BLOCKED**, with no
sanitizer findings or unexpected terminations. UI01 has 20 PASS/1 BLOCKED; UI02
has 14 PASS. The blocked case is offscreen D3D10 texture readback. Actual font
rasterization remains an integration prerequisite, not verified coverage.

At the end of the UI pass, the manifest registered 224 cases across 31 family labels, including
unfinished NW04 scaffolding. This is not 31 completed families or a fully executed
224-case suite. NW04, AN07 and OS01 are outside the UI pass. The 153-case results
below describe the retained historical baseline; unrelated defects remain open.

See [UI repairs, policies, complete case mapping, allocator/build limitations and reproduction](/g:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/CryActionUIProgress.md).

## Scope and baseline verification (2026-10-19)

This records a **partial implementation of the 33-family plan**. The historical
baseline manifest contained **153 isolated cases across 24 families**. Nine families then had
no implemented cases. A represented family is not necessarily complete, and an
isolated component result does not establish full runtime compatibility.

The reported 67-case / 14-family starting point matches the historical complete
reports. It did not match the source at the start of this task: 12 AN04 additions
had raised the previously validated manifest to 79, and 16 unvalidated AN01 reader
cases raised the working manifest to 95. The latest build had failed on a test
comparing the return value of the void ICryPak::FWrite template overload. That
fixture error was corrected by checking file position and independent readback.

The recovered 95-case baseline was executed before adding the new component work:

| Lane | PASS | FAIL | CRASH | TIMEOUT | BLOCKED | ASan diagnostic cases |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Profile | 83 | 9 | 3 | 0 | 0 | 0 |
| Profile + ASan | 82 | 13 | 0 | 0 | 0 | 3 |

Historical reports are preserved in the [original evidence archive](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-baseline-before-20261019)
and the [recovered 95-case archive](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-baseline-95-20261019).
Intermediate fixture diagnostics are separate from the final run in the
[152-case archive](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-intermediate-152-20261019).

## Build and reproduction

Use PowerShell 7 and the existing local helper:

```powershell
& 'G:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/RunCryActionRegression.ps1'
& 'G:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/RunCryActionRegression.ps1' -Asan
& 'G:/projects/CRYENGINE-VC14/CrysisWars/CryEngine/Tests/CryCommonTests/RunCryActionRegression.ps1' -Family RT03
```

Both lanes build x64 Profile, v143, C++20. Existing dependencies in the workspace
are staged; project references are not rebuilt. The script builds a separate
CryActionRegression.dll, checks the normal CryAction.dll SHA-256, runs the default
CryCommonTests suite, then runs the opt-in cases in separate local processes.
Unknown families return exit 2. Family filters select component cases only; the
default unit suite still runs. Family reports have a suffix to preserve full runs.

The default GameRoot is the existing read-only asset input at
G:/projects/MWEL-testbed. Override it with -GameRoot if needed. No reference source
tree is compiled or edited. Generated fixtures, logs, and explicitly configured
allocator dumps are placed under this workspace. PID-specific successful fixtures
are deleted by their owning tests; terminated children may leave fixture files.

To reproduce one case after building:

```powershell
& 'G:/projects/CRYENGINE-VC14/CrysisWars/Bin64/Tests/Profile/CryCommonTests.exe' `
  --cryaction-regression-case `
  'G:/projects/CRYENGINE-VC14/CrysisWars/Bin64/Tests/CryActionRegression/Profile' `
  'G:/projects/MWEL-testbed' 'state-index-single' `
  'G:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/state-index-single.log'
```

Prefer the pipeline for risky cases: it enforces the 120-second process timeout
and collects exit codes and sanitizer paths. Direct case mode has no parent timeout.
The test host suppresses the runtime crash dialog after preview initialization.
Engine RNG is reset to 0x43525932 in every fresh test DLL instance. The legacy
rotation test also seeds its own CRT random permutation fixture.

Flow cases additionally initialize the local CryScriptSystem and CryEntitySystem
factories and class registry before constructing the real CFlowSystem. A missing
factory or failed initialization is an explicit prerequisite, not a passing case.
They do not initialize a playable game/framework or claim orderly whole-system
shutdown. The graph cleanup case destroys a real FlowSystem with three live graphs
and then releases those graphs after their production destruction notifications.

## Historical full-baseline execution results (2026-10-19)

| Lane | PASS | FAIL | CRASH | TIMEOUT | BLOCKED | ASan diagnostic cases |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| x64 Profile | 138 | 12 | 3 | 0 | 0 | 0 |
| x64 Profile + ASan | 138 | 15 | 0 | 0 | 0 | 4 |

Both lanes contain exactly 153 distinct case rows. Profile observed 9,339
checks, 23 failed checks in 12 cases, and four unexpected terminations (three
exceptions and the deliberate early exit after the 121-server capacity assertion).
That early-exit case overlaps the assertion-failing count. ASan observed 9,333
checks, 22 failed checks in 11 cases, and four sanitizer terminations; those
two evidence sets are disjoint in this run. The four ASan diagnoses are included
in its 15 FAIL cases, never added to the case total. Each lane's primary status
columns sum to 153. Both pipelines correctly return exit 1 while defects remain.

The same documented undefined behavior can produce different outcomes across
runs. In this run the parameterless HTTP command crashed in Profile and passed
under ASan, whose CrySystem XML dependency is uninstrumented. The 121-server case
failed its logical capacity assertion and exited early in Profile, while ASan
stopped it at the scratch-array overwrite. These are not stable exit-code promises.

Fresh sanitizer evidence (one unique case ID/PID path per diagnosis):

- [Empty-link read](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-ASan-20261019-120723-860-case-8.2228): access violation in CAnimationGraph::IsDirectlyLinked.
- [121-server scratch overwrite](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-ASan-20261019-120723-860-case-22.24144): stack-buffer-overflow in GetCurrentServers.
- [StateIndex pool/STL write](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-ASan-20261019-120723-860-case-30.7672): unresolved unknown-crash during vector reallocation; instrumentation boundary remains under investigation.
- [Negative reader count](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-ASan-20261019-120723-860-case-82.35496): negative-size-param in the actual reader memcpy.

Both final module/runner builds succeeded with zero errors. These last module
builds were incremental; earlier builds in this task compiled the newly added
test translation units. Existing warnings were retained.

The default suite passes 97 cases with zero failed checks in each lane. Each
CryAction case has its own child process; processing continues after termination.

Primary status counts are mutually exclusive and add up to the manifest size:

- PASS: completion marker, exit zero, at least one assertion, no failed checks,
  and no detected ASan diagnostic.
- FAIL: failed assertions, nonzero ordinary exit, sanitizer termination, or missing
  completion/zero checks. A sanitizer diagnosis is never an extra case.
- CRASH: Windows exception exit code (including a diagnostic exception).
- TIMEOUT: parent terminated the child after 120 seconds.
- BLOCKED: an implemented case could not satisfy its loader/initialization/ABI
  prerequisite. An unimplemented family is excluded from these totals.

The TSV records checks and failed_checks separately from case counts. The
complete, elapsed_ms, process_outcome, and prerequisite columns retain process
information. Assertion-failing cases, ASan diagnostic cases, and unexpectedly
terminated cases are overlapping evidence categories. Unexpected termination
means exception, sanitizer-termination, or incomplete-exit; it excludes timeouts
and blocked cases, which have their own counts. A case may fail an assertion
before terminating, and is counted once in its primary status.

Current evidence:

- [Profile case report](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-Profile.tsv)
- [ASan case report](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-AsanProfile.tsv)
- [Profile configuration/source/module hashes](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-Profile-evidence.json)
- [ASan configuration/source/module hashes](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-AsanProfile-evidence.json)

Each evidence JSON records the revision, dirty status, relevant production/test
source hashes, configuration, fixed seed, staged DLL paths/hashes, runner hash,
production DLL hash before/after, and execution totals. Literal fixture bytes are
in the hashed sources. Runtime XML/BMP/graph fixtures are generated from those
sources. This is not a captured original shipping-runtime corpus or branch coverage.

## Failure and fixture classification

Production algorithms were not repaired. Intended correct-behavior assertions
remain red where the current implementation violates them.

| Issue | Classification and disposition |
| --- | --- |
| AN01 truncated scalar/array, EOF, negative count, cursor beyond end | Production reader defect: ReadFromBuffer copies without validating count or remaining bytes. Require unchanged output/cursor for rejected reads. An invalid cursor is reachable after an earlier unchecked truncated read. |
| AN01 malformed string payload/terminator/zero encoded length | Production reader validation defect. The writer always emits a positive length including NUL. Intended rejection is transactional; the void reader still has no propagated error result. These are explicit malformed-input policy tests, not claims that the old API already reports an error. |
| AN04 empty IsDirectlyLinked | Production defect: element zero of an empty link vector is used. Keep the assertion for false and retain crash/ASan evidence. An above-last lookup passing does not prove the related legacy OOB read is absent. |
| AN03 single-state explicit match | Production Builder/Init defect: a restricted-to-zero selector can be omitted when only its opposite value has a restriction table. Enforced explicit matching must still recognize state zero. Preserve the failing assertion; no cast or expectation substitution. |
| AN02 roundtrip ASan write in CStateIndex::Query | Unresolved memory-instrumentation prerequisite at the custom pool/STL boundary. The write occurs in instrumented vector reallocation inside a CrySystem pool region; the noninstrumented pool does not provide a trustworthy suballocation shadow boundary. No StateIndex algorithm defect or false positive is established. Retain the diagnostic and failed case without suppression. |
| NW01 121-server scratch capacity | Production defect: growth writes beyond m_astrServers[120]. Depending on undefined behavior, the case can crash, trigger ASan, or fail the logical capacity check and exit before cleanup. |
| NW01 256-character target | Production defect: strcpy writes its terminator beyond the 256-byte logical scratch capacity. A test-owned extra canary byte makes this visible even when engine pools hide it from ASan. The actual enumeration method still performs the copy. |
| NW01 repeated announcement | Production defect: Compare compares an incoming description with existing data, so an unchanged endpoint/description/data announcement is duplicated. Require one entry and one initial ping. |
| PS03 BMP size-only overflow | Production defect: the size calculation wraps. Accept explicit zero rejection or the exact 16-GiB-plus-54-byte result; no huge allocation or payload access occurs. |
| PS02 empty inventory XML attributes | Incorrect historical expectation, already corrected in the initial working tree. Default zero-valued attributes are omitted. Retain omission checks and production readback/current/holstered/last-ID checks. |
| AN01 reader fixture build error | Incorrect fixture assumption about void FWrite. Corrected with file-position and independent literal-byte readback checks. |
| AN05 indirect/tied fixture | Incorrect fixture: guard evaluation needs the constructed state's real current inputs, and XML costs already account for the loader's minus-one convention. Corrected by supplying actual state/inputs and using literal contract-derived costs. |
| NW07 RPC XML fixture ownership | Incorrect fixture: getXML returns an owning CryString allocated in CrySystem, whose per-module accounting cannot be transferred here. Use borrowed IXmlStringData and a local copy. No production code changed. |
| NW05 callback observer allocation | Fixture-owned growing transcript triggered a pool/STL instrumentation diagnostic. Replaced with a bounded eight-slot callback transcript; all payload/order assertions remain. This does not fix or suppress the separate production StateIndex diagnostic. |
| ASan diagnostic attribution | Incorrect runner bookkeeping: Windows reused child PIDs, so a PID-only log name could attach an old diagnosis to a later completed passing case. Log names now contain the unique run prefix, case ID, and PID. Final reports were rerun after this correction; affected intermediate reports are not current validation. |
| RT03 initial entity registry absence | Missing execution prerequisite, resolved by initializing the actual local script/entity modules. If unavailable on another machine, report BLOCKED with the factory/init reason. |
| RT03 one-more-than-limit attempt | Incorrect recoverable-rejection expectation: Profile CryError terminates instead of returning InvalidFlowNodeTypeId. The old dialog yielded a recorded timeout. Current assertions validate all legal IDs through 4095; recovery past the fatal boundary is not claimed. |
| NW07 parameterless RPC | Production code unconditionally requests child 1 despite documenting parameterless commands. The direct valid-behavior case is retained, but may pass depending on unchecked XML storage. CrySystem is not instrumented in this lane; passing does not establish bounds safety. |

The [earlier timeout record](/g:/projects/CRYENGINE-VC14/CrysisWars/BinTemp/CryAction-regression-earlier-timeout-20261019.tsv)
records the 120,011-ms fatal-boundary attempt, PID 36580, exit 3, and 3,690 checks
observed before timeout. It is historical diagnostic evidence, not an extra case
in the final manifest totals. No crash/timeout is relabeled as a normal passing
case. The current legal-domain test does not execute the fatal registration.

## Coverage ledger

All implemented rows invoke actual production owners/methods. Counts are cases,
not warning-site totals or source/branch percentages. Every family remains partial
unless its complete planned owning/consuming integration is established.

| Family | Cases | Current evidence and remaining work |
| --- | ---: | --- |
| AN01 | 24 | Scalars/arrays/unaligned suffixes/canaries, malformed counts/strings, EOF, literal file fixture, writer/readback, zero/no-file behavior. A real graph round trip verifies that a truncated file fails at the graph loader. The oversized SetFile collaborator remains. |
| AN02 | 1 | Actual StateIndex writer/reader, independent packed-field decoder and query comparison. The final ASan run reported no pool diagnostic. Broader graph/factory binary cases and original-runtime cross-load remain. |
| AN03 | 5 | Builder/query, wildcard/intersection/ranking, explicit matching and stored ranges, including the repaired single-state case. Compact maxima and full graph consumption remain. |
| AN04 | 20 | Empty/single/sorted/duplicate/endpoints/sentinels, 1024 fixed-seed queries, XML Init and real ActivateNextState timing consumer. No character callbacks or complete per-frame finalization. Duplicate slot selection is observed, not a newly imposed first/last policy. |
| AN05 | 6 | Real XML graph and constructed state/inputs; same-state, direct, cheaper indirect, implicit fallback, ties and repeated searches with literal costs/path endpoints. Ties allow either equal-cost candidate rather than freezing pointer-derived cross-process choice. Missing character movement/guard providers and full transition consumer. |
| AN06 | 6 | Real CAnimationGraphStates with 0/1/3 live layers, highest-layer output/empty masking, duplicate and reverse-order listeners, detach/destruction and borrowed-layer ownership. Missing layered query binding/serialization, character blend histories and large layer bounds. |
| AN07 | 6 | Actual graph DebugDisplay with 0/1/3 input queries, unsigned IDs, literal percent text and real factory pointer; real local missing-extension RMI Sync and captured warning. Pointer/query formatting and warning double-formatting repaired. Full overlay/character visual integration remains. See repair report. |
| AN08 | 6 | Actual constructed AI handler/game-object with entity/character/skeleton collaborators, absent collaborators, flag combinations, exact/other/nearby float decisions and short trace. Historical focused Profile/ASan six passing; not rerun in the UI repair pass. |
| AN09 | 4 | Actual parameter expansion/nested substitution/exclusion. Inherited templates, expanded links/views and final graph construction missing. |
| AI01 | 5 | Actual readability sound-group empty/one/three selection, recent-history exclusion/reset and group fallback. No playback; XML loader/path parsing, zero-probability AI-warning collaborator, face/lookup consumers and sizing remain. |
| RT01 | 6 | Actual view-shake producer/update/reset with per-shake callback order, update-only fields and deterministic resulting shift; real FlowSystem destruction with three live graphs. Other reverse cleanup/group/vehicle/broken-object owners remain. |
| RT02 | 4 | Actual dialog AddLine/Complete/GetLine/count APIs, noncontiguous actor IDs, invalid actor/indices, post-completion mutation, fixed-width signatures. Full actor/item/level/profile/vehicle enumeration remains. |
| RT03 | 2 | Actual type registration/duplicate/lookup/factory release, real node creation/ResolveAddress/link/activation/callbacks; all remaining legal IDs through real 12-bit maximum 4095. Fatal next registration is separate historical evidence. Port-255 schemas, material/vehicle IDs and script consumers remain. |
| RT04 | 6 | Registered production extension lifecycle, deferred/repeated activation, Init/PostInit, acquire/deactivate/release/sticky teardown and creation/init failure. Historical focused Profile six passing. Update-path and broader lifecycle integration remain; not rerun in the UI pass. |
| RT05 | 6 | Production inspector draw capture for empty/one/three records, signed rows, fade and small graph record formatting. Historical focused Profile six passing; full overlay visual integration remains. Not rerun in the UI pass. |
| PS01 | 5 | Rotation ordering/settings/random permutation/reset and XML writer capture. Missing mod/level metadata, other parsers and rotation loading with level registry. |
| PS02 | 3 | Real inventory XML serializer/readback, zero omission, capacity and high-bit EntityIds. Missing ammo/class registry, dialog/vehicle and whole-world save integration. |
| PS03 | 11 | Both actual thumbnail implementations, replacement, RGB/RGBA/fill/empty, invalid dimensions, and real BMP headers/rows/padding/orientation/canaries/reload. Full RSF and screenshot/save-owner integration remain. |
| PS04 | 6 | Actual WriteXMLNode/ReadXMLTagData/ReadTag/SkipXMLTagData; null/plain/compressed chunks, ordered chunks/cursor, truncated read and wrong tag, independent fixed-width header and parsed logical XML. Missing short-write injection, huge-size guard and complete RSF save ownership. |
| PS05 | 6 | Constructed real time-demo recorder loads independent literal-layout v1 empty/one/three and v2 plain/compressed fixtures; truncated v2 returns failure; fields/counts/high-bit flags verified. Missing production Save/Record/Play, input/entity-event replay, original corpus and malformed-count guards. |
| PS06 | 4 | Metadata values/canaries/clone ownership at 1/254/255 bytes. Recorder framing and disabled sampler source-string path remain. |
| NW01 | 15 | Real add/enumerate/get/remove/parse plus expiry at 10/11 seconds, adjacent removals, ping averaging/refresh, complete long targets, capacity growth and duplicate announcements. Malformed record and version-parser coverage remains. |
| NW02 | 3 | Real GameContext hex helpers, literal hex golden, invalid character rejection and 0/1/7/8/9 bytes. Descriptor layout/stack length and negotiation remain. |
| NW03 | 3 | Actual RMI registration/protocol definition/duplicate/order/nUser/reliability/flags. Lua/property limits, decoder/Trampoline and wire bytes remain. |
| NW04 | 13 | Actual replay, retries, signed references, sentinel/frame gates, log formatting and malformed record guards; recording owner with both production script serializers; fresh real structural joints; real plane generation/loading and preserved chunk prefixes. Signed traversal/range checks and physics grid seeding repaired. Autonomous solver callbacks, live transport and cross-version save/network compatibility remain unverified. See repair report. |
| NW05 | 4 | Actual CGameplayRecorder Event/Register/Unregister callback order/multiplicity, duplicate suppression and opaque high-bit ID transport. Missing actual Lua/vehicle producer and entity-ID consumer resolution; no wire compatibility claimed. |
| NW06 | 8 | Constructed GameStats with local framework/recorder/actor/level and report collaborators; player events/removal, counts, ordered fields and game/pre-game mode. Team map inputs are seeded, not created by a production team event route. Historical focused Profile eight passing; full team integration remains. Not rerun in the UI pass. |
| NW07 | 7 | Actual listener callbacks/queue and real CrySystem MD5 with independent fixed digest, valid/invalid authorization, malformed XML, ordering, escaped output and parameterless command. In-memory transport only; no socket opened. Missing INT_MAX count guard and exact composed-length observer. |
| UI01 | 21 | Actual constructed UI and recording renderer/font/input/script/framework; primitives, mixed order, textures, scissor/blend, full accumulated uint16 domain, renderer/index limits, oversized/invalid circle, 0/1/3 precache consumer. All 21 passed in Profile and ASan, including real D3D10 offscreen pixel readback. Full game visual compatibility remains. |
| UI02 | 14 | Actual conversion and DrawText consumer; empty/ASCII/2047/2048/2049, .UTF8 boundary, .1252 and .932, invalid/interior/null, repeated calls and borrowed lifetime. Profile and ASan: 14 PASS. Bounded truncation and invalid-input policies documented in the owner header. Real-font rasterization remains unverified. |
| OS01 | 7 | Actual framework and Windows registry API with test-owned key; normal/legacy/malformed/length/missing cases, terminator and handle ownership; actual and controlled temporary-path boundary cases. Registry/path defects repaired. No real CD-key access or editor launch; ViewNote process/callback lifecycle is compiled but not exercised. See repair report. |
| MEM01 | 2 | Actual ItemParams XML conversions/order/count and recursive size growth. Missing recording ICrySizer, wide totals and other owners. |
| BLD01 | 1 | Loaded test PE AMD64/PE32+, factory export/resource presence. Normal DLL hash preserved; no full factory/init/update/shutdown compatibility or preferred-base repair. |

## Prerequisites and limits

- All 33 family labels now have registered cases. Existing component coverage
  does not establish completion of missing owning or consuming integration paths.
- The local script/entity modules support the Flow fixture. They do not supply a
  full game framework, renderer/font pipeline, animated character or gameplay
  world. CrySoundSystem and a compatible game DLL are absent from the checked
  workspace runtime inventory. Full boot/update/shutdown and asset compatibility
  need those modules and a known-good game configuration/corpus.
- The current ASan lane instruments the host, CryAction and CrySystem, with pool
  lifetime shadow hooks. Other staged dependencies, including the rebuilt
  Profile CryPhysics, remain uninstrumented. The historical StateIndex diagnostic
  remains archived. The full matrix subsequently passed that case in ASan. No UBSan or
  whole-engine leak/shutdown validation is claimed.
- The historical baseline pass added probes without changing normal algorithms.
  The later UI and NW04/AN07/OS01 passes include the production repairs described
  in their linked reports. Published serialization widths and field names remain
  unchanged; that alone does not establish cross-version compatibility.
- CryAction.dll remains SHA-256
  E9FA2C5B4CFCF13174D82221167FF3F820B9B7FA50B0B4D526194318D6F5A707.
  Test bridge ABI 2 uses fixed-width scalar requests and callbacks; test objects
  are created/exercised/destroyed in their owning DLL. Existing public XML data is
  borrowed and copied locally where module-specific string accounting matters.
