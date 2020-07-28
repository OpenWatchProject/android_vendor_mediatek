@echo off

:: ca/dut config
for /F "tokens=1,2 delims==" %%i IN (scripts/config.txt) do set %%i=%%j

if "%1" == "start_ca" goto :start_ca
if "%1" == "start_pf" goto :start_pf

for /f %%a in (scripts/testMode.txt) do (set testMode=%%a)
echo Start for ^<^<%testMode%^>^>
echo.

tasklist | findstr Control-Agent>nul && (adb shell ps -A| findstr WFA-DUT > nul && msg %username% E002: sigma tool is running now! && goto :end)
:: check adb connection
adb get-state|findstr "unknown">nul && (msg %username% E003: no dut connected! && goto :end)
:: check build type, only eng or user debug load can are allowed to run sigma tool
adb root | findstr /c:"production builds">nul && (msg %username% E004: sigma test can only run on eng or userdebug load && goto :end)

adb wait-for-device
:: check necessary scripts can be executed or not
for /f %%i in (scripts\install_list.txt) do (adb shell ls -l %BIN_DIR%/%%~nxi | findstr [-r][-w]x>nul || msg %username% E005: %%~nxi is not exist or can't be executed! && goto :end)
adb shell ps -A|findstr wpa_supplicant>nul || goto turn_on_wifi_in_sigma_mode
:: check if wlan normal mode is enabled
adb shell getprop wlan.driver.status | findstr /C:"ok">nul || goto start_dut_and_ca
echo wlan is in normal mode, turn off first
adb shell svc wifi disable > nul
ping 127.0.0.1 -n 4 > nul

:turn_on_wifi_in_sigma_mode
echo "Set permissive mode"
adb shell setenforce permissive
adb shell getenforce

for %%i in (%NEED_WLAN0% %NEED_P2P0%) do (
    if %testMode% == %%i (
        echo turning wifi on, wait 2 seconds
        adb shell "echo S > /dev/wmtWifi"
        ping 127.0.0.1 -n 2 > nul
        adb shell setprop ctl.start wpa_supplicant
        echo wait 5s to check if wifi enabled
        ping 172.0.0.1 -n 5 > nul
        adb shell ps -A|findstr wpa_supplicant>nul || goto start_wlan_fail
        goto :add_iface
    )
)
echo wpa_supplicant is not started. Add %testMode% to NEED_WLAN0 or NEED_P2P0 in config.txt if needed.
:add_iface
for %%i in (%NEED_WLAN0%) do (
    if %testMode% == %%i (
        adb shell rm %BIN_DIR%/wpa_supplicant.conf
        echo add wlan0 interface..
        adb shell "cat /vendor/etc/wifi/wpa_supplicant.conf >> %BIN_DIR%/wpa_supplicant.conf"
        adb shell "cat /vendor/etc/wifi/wpa_supplicant_overlay.conf >> %BIN_DIR%/wpa_supplicant.conf"
        adb shell "wpa_cli -iwlan0 -g@android:wpa_wlan0 interface_add wlan0 %BIN_DIR%/wpa_supplicant.conf nl80211 /data/vendor/wifi/wpa/sockets"
        adb shell "wpa_cli -iwlan0 -g@android:wpa_wlan0 interface"|findstr wlan0>nul || goto add_iface_if_fail
    )
)

for %%i in (%NEED_P2P0%) do (
    if %testMode% == %%i (
        adb shell rm %BIN_DIR%/p2p_supplicant.conf
        echo add p2p0 interface..
        adb shell "cat /vendor/etc/wifi/wpa_supplicant.conf >> %BIN_DIR%/p2p_supplicant.conf"
        adb shell "cat /vendor/etc/wifi/p2p_supplicant_overlay.conf >> %BIN_DIR%/p2p_supplicant.conf"
        adb shell "wpa_cli -iwlan0 -g@android:wpa_wlan0 interface_add p2p0 %BIN_DIR%/p2p_supplicant.conf nl80211 /data/vendor/wifi/wpa/sockets"
        adb shell "wpa_cli -iwlan0 -g@android:wpa_wlan0 interface"|findstr p2p0>nul || goto add_iface_if_fail
    )
)

:: add routing rule
adb shell ip rule add pref 9999 from all fwmark 0x10063/0x1ffff table main
adb shell ip rule add pref 9999 from all fwmark 0x0/0xffff table main

:: update prop for dut
adb shell "setprop vendor.wifi.sigma.mode %testMode%"

echo Reset socket buffer window size
adb shell "echo 212992 > /proc/sys/net/core/wmem_default"

del log\ca_log.log
del log\dut_log.log
del log\pf_log.log

if exist scripts\enable_phone_log.bat (call scripts\enable_phone_log.bat)
echo.
color 0A
echo Ready to start dut and ca?
pause

:: start dut and ca
:start_dut_and_ca

set CALLBACK=%PROGRAM_DIR%\%testMode%\callback_for_start.bat
echo %CALLBACK%
if exist %CALLBACK% (call %CALLBACK% %testMode%)

start "" "%~f0" start_ca %1
adb forward --remove-all
if "%PORT_FORWARD%"=="true" (
adb forward tcp:%CONNECT_PORT% tcp:9997
start "" "%~f0" start_pf %1
) else (
adb forward tcp:%LISTEN_PORT% tcp:9997
)

cls
title WFA-DUT
mode con cols=45 lines=5
color 0A
echo WFA-DUT is running ...
echo redirecting the output to log\dut_log.log
adb shell "%BIN_DIR%/wfa_dut lo 6669" 1> log\dut_log.log 2>&1
adb get-state|findstr "unknown">nul && msg %username% E006: adb connection lost! || msg %username% E008: error happens on wfa_dut!
goto :end

:start_ca
title Control-Agent
mode con cols=45 lines=5
color 0A
:: wait 2 seconds
ping 127.0.0.1 -n 2 > nul
echo Control-Agent is running ...
echo redirecting the output to log\ca_log.log
adb shell "%BIN_DIR%/wfa_ca lo 9997 127.0.0.1 6669" 1> log/ca_log.log 2>&1
msg %username% E007: error happens on wfa_ca!
goto :end

:start_pf
title PFService
mode con cols=45 lines=5
color 0A
:: wait 2 seconds
ping 127.0.0.1 -n 3 > nul
echo Port %LISTEN_PORT% forwarding to %CONNECT_PORT%  ...
scripts\PFService\mtk_pf.exe lo %LISTEN_PORT% 127.0.0.1 %CONNECT_PORT% log\pf_log.log
msg %username% E010: error happens on mtk_pf!
goto :end

:start_wlan_fail
msg %username% E009: enable wlan in sigma mode fail!
goto :end

:add_iface_if_fail
msg %username% E009: enable wlan in sigma mode fail!
goto :end

:end
start notepad.exe scripts\FAQ.txt
exit

