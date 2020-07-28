set testMode=%1
set LOCAL_DIR=%~dp0

echo "Turn on voe test mode if needed..."

adb push %LOCAL_DIR%\wifi_fw.cfg /data/misc/wifi
::adb push %LOCAL_DIR%\wifi.cfg /data/misc/wifi
adb shell rm /data/misc/wifi/wifi.cfg 

if exist %LOCAL_DIR%\wifi.cfg (
    set list=6765 6761 6779 6767 6768 6785
    for %%a in (!list!) do (
        echo %%a
        adb shell getprop | findstr ro.hardware | findstr %%a > nul && goto connac_check_voe_test_mode
    )
)
goto end

:connac_check_voe_test_mode
echo "Check VOE Test mode enabled or not on CONNAC platform."
adb shell cat /data/misc/wifi/wifi.cfg|findstr "SwTestMode 0x14">nul || goto connac_enable_voe_test_mode
goto end

:connac_enable_voe_test_mode
echo "Turn on VOE Test mode on CONNAC platform."
adb shell "echo SwTestMode 0x14 >> /data/misc/wifi/wifi.cfg"

:end