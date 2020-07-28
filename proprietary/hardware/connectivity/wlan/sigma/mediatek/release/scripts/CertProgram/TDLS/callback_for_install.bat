set testMode=%1
set LOCAL_DIR=%~dp0

echo "Turn on TDLS test mode if needed..."

if exist %LOCAL_DIR%\wifi.cfg (
    adb shell getprop | findstr persist.vendor.connsys | findstr 6630 > nul && adb push %LOCAL_DIR%\wifi_fw.cfg data/misc/wifi/
)