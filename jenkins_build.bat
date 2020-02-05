@echo off
setlocal EnableDelayedExpansion
setlocal EnableExtensions

echo Build %1 for %3 with the configuration %2
set mspath="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\MSBuild\15.0\Bin\msbuild.exe"
if %3 EQU PS4 (
echo Building for PS4
!mspath! Y2018C-Y2-PR-TBSG\TBSG\TBSG.sln /property:Configuration=%2 /property:Platform=%3 /target:%1 /p:PlatformToolset=Clang /p:PreBuildEventUseInBuild=false /p:PostBuildEventUseInBuild=false
) else (
echo Building for PC
!mspath! Y2018C-Y2-PR-TBSG\TBSG\TBSG.sln /property:Configuration=%2 /property:Platform=%3 /target:%1 /p:PostBuildEventUseInBuild=false
)
