REM @echo off

CALL npm install

CALL electron-forge make .

CALL electron-forge package .
