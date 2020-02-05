@echo off

set arg1=%1

IF %arg1%.==. GOTO No1

set SHADER_SRC_DIR=..\TBSGFramework\data\Shaders
set SHADER_OUT_DIR=..\..\Projects\froggy\Client\data\Shaders\bin
set ORBIS_ADDITIONAL_OPTIONS=

echo additional include dir: %arg1%
echo "Compiling shaders..."
if not exist "%SHADER_OUT_DIR%" mkdir "%SHADER_OUT_DIR%"
for /R %cd%\%SHADER_SRC_DIR%\PSSL\PS %%G IN (*.PSSL) DO (orbis-wave-psslc.exe -I%arg1% %%G %ORBIS_ADDITIONAL_OPTIONS% -profile sce_ps_orbis -o %cd%\%SHADER_OUT_DIR%\%%~nG.sb)
for /R %cd%\%SHADER_SRC_DIR%\PSSL\VS %%G IN (*.PSSL) DO (orbis-wave-psslc.exe -I%arg1% %%G %ORBIS_ADDITIONAL_OPTIONS% -profile sce_vs_vs_orbis -o %cd%\%SHADER_OUT_DIR%\%%~nG.sb)
for /R %cd%\%SHADER_SRC_DIR%\PSSL\CS %%G IN (*.PSSL) DO (orbis-wave-psslc.exe -I%arg1% %%G %ORBIS_ADDITIONAL_OPTIONS% -profile sce_cs_orbis -o %cd%\%SHADER_OUT_DIR%\%%~nG.sb)

for /R %cd%\%SHADER_SRC_DIR%\Unified %%G IN (*_p.hlsl) DO (orbis-wave-psslc.exe -I%arg1% %%G %ORBIS_ADDITIONAL_OPTIONS% -profile sce_ps_orbis -o %cd%\%SHADER_OUT_DIR%\%%~nG.sb)
for /R %cd%\%SHADER_SRC_DIR%\Unified %%G IN (*_v.hlsl) DO (orbis-wave-psslc.exe -I%arg1% %%G %ORBIS_ADDITIONAL_OPTIONS% -profile sce_vs_vs_orbis -o %cd%\%SHADER_OUT_DIR%\%%~nG.sb)
for /R %cd%\%SHADER_SRC_DIR%\Unified %%G IN (*_c.hlsl) DO (orbis-wave-psslc.exe -I%arg1% %%G %ORBIS_ADDITIONAL_OPTIONS% -profile sce_cs_orbis -o %cd%\%SHADER_OUT_DIR%\%%~nG.sb)
echo "Compiling shaders completed."
goto :Endl

:No1
echo No first parameter found!! make sure to pass in the additional include directory!
pause

:Endl
