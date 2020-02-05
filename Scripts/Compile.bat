REM @echo off
SET MSBuild="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\MSBuild.exe"

if "%TARGET%"=="" (
SET TARGET=Clean,Build
)

if %PLATFORM% EQU PS4 (
%MSBuild% TBSG.sln -property:Configuration=%CONFIGURATION% -property:Platform=%PLATFORM% -target:%TARGET% -m -property:PlatformToolset=Clang -property:PreBuildEventUseInBuild=false -property:PostBuildEventUseInBuild=false
) else (
%MSBuild% TBSG.sln -property:Configuration=%CONFIGURATION% -property:Platform=%PLATFORM% -target:%TARGET% -m -property:PostBuildEventUseInBuild=false
)
