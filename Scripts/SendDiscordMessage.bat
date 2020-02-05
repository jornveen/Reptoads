@echo off

REM Below is a test message
REM SET "MESSAGE=<:switch:555017219029860396> :desktop: Failed build of LuciferEngine-Debug-PC (88) https://jbs1.nhtv.nl:8443/job/Y2018-Y2/job/LuciferEngine/job/LuciferEngine-Debug-PC/88/"

ECHO Sending message on Discord: %MESSAGE%

SET URL="https://discordapp.com/api/webhooks/547732567588339712/pPEufPQ2YdnGfTQR3xmSo9JM6iCXycQcrD_f3Nk6tW1baF2UwAUMKHIBU7MBe1aXAxhR"
if "%DOXYGENJOB%"=="yes" (
    REM SET URL="https://discordapp.com/api/webhooks/563284570527760394/e65rwnBr85oRYKjRXoTwqhwSyxKbTclo1kVOBDYgW_IeFeMVdC5zCRR10ic5oMp_WjvX"
)

ECHO {"content": "%MESSAGE%"} > last-discord-message.txt

CALL curl -X POST -H "Content-Type: application/json" --data "@last-discord-message.txt" --max-time 10 %URL%
