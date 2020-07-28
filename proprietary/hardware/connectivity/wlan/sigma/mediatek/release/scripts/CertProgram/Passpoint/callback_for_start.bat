@echo off

if "%1" == "start_wpa_cli" goto :start_wpa_cli

set LOCAL_DIR=%~dp0
set HS20_DIR=/data/misc/wifi/hs20

::Push action file
adb push %LOCAL_DIR%\hs20-action.sh %HS20_DIR%/
adb shell chmod 777 %HS20_DIR%/hs20-action.sh

start "" "%~f0" start_wpa_cli %1
goto :end

::Start wpa_cli daemon for monitor specified events
:start_wpa_cli
title HS20-WPA_CLI-DAEMON
mode con cols=45 lines=45
color 0A
echo "Start wpa_cli for monitor events..."
adb shell "wpa_cli -g@android:wpa_wlan0 -iwlan0 -a %HS20_DIR%/hs20-action.sh"
goto :end

:end

