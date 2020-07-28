set testMode=%1
set LOCAL_DIR=%~dp0

echo "Turn on WMMAC test mode if needed..."

adb push %LOCAL_DIR%\wifi_fw.cfg /data/misc/wifi
