set testMode=%1
set LOCAL_DIR=%~dp0

echo "Turn on 11n test mode if needed..."

adb shell chmod 777 /data/misc/wifi/
adb push %LOCAL_DIR%\cas.pem data/misc/wifi/
adb push %LOCAL_DIR%\wifiuser.key data/misc/wifi/
adb push %LOCAL_DIR%\wifiuser.pem data/misc/wifi/

if exist %LOCAL_DIR%\wifi.cfg (
    set list=663 6779 6765 6762 6761 6768
    for %%a in (!list!) do (
        echo %%a
        adb shell getprop | findstr persist.vendor.connsys.chipid | findstr %%a > nul && adb push %LOCAL_DIR%\wifi.cfg data/misc/wifi/
    )
)

if exist %LOCAL_DIR%\wifi_fw.cfg (
    adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 663 > nul || adb push %LOCAL_DIR%\wifi_fw.cfg data/misc/wifi/
)