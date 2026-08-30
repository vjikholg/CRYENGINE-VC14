@echo off
setlocal

set "VC8_ROOT=C:\Program Files (x86)\Microsoft Visual Studio 8\VC"
set "CRYSIS_ROOT=%~dp0..\..\.."
set "NVDXT_ROOT=%CRYSIS_ROOT%\CryEngine\RenderDll\Common\Textures\Image\NVDXT"
set "OUTPUT_DIR=%CRYSIS_ROOT%\BinTemp\x64\LegacyNvDxtOracle"
set "DATA_DIR=%CRYSIS_ROOT%\CryEngine\Tests\CryCommonTests\TestData\DxtCompatibility"

if not exist "%VC8_ROOT%\vcvarsall.bat" (
	echo Visual C++ 2005 was not found at "%VC8_ROOT%".
	exit /b 1
)

if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

call "%VC8_ROOT%\vcvarsall.bat" amd64 || exit /b 1
cl /nologo /EHsc /MD /O2 /D_CRT_SECURE_NO_DEPRECATE ^
	/I"%NVDXT_ROOT%" ^
	"%~dp0LegacyNvDxtOracle.cpp" ^
	/Fo"%OUTPUT_DIR%\LegacyNvDxtOracle.obj" ^
	/Fe"%OUTPUT_DIR%\LegacyNvDxtOracle.exe" ^
	/link /LIBPATH:"%NVDXT_ROOT%\release" nvDXTlibMTDLL.vc8.x64.lib user32.lib || exit /b 1

"%OUTPUT_DIR%\LegacyNvDxtOracle.exe" "%DATA_DIR%\mwll_tankcolor_dxt1.dds" "%DATA_DIR%\mwll_tankcolor_dxt1.nvdxt.rgba" || exit /b 1
"%OUTPUT_DIR%\LegacyNvDxtOracle.exe" "%DATA_DIR%\mwll_dot_dxt3.dds" "%DATA_DIR%\mwll_dot_dxt3.nvdxt.rgba" || exit /b 1
"%OUTPUT_DIR%\LegacyNvDxtOracle.exe" "%DATA_DIR%\mwll_light_color_dxt5.dds" "%DATA_DIR%\mwll_light_color_dxt5.nvdxt.rgba" || exit /b 1

echo nvDXT golden outputs regenerated successfully.
