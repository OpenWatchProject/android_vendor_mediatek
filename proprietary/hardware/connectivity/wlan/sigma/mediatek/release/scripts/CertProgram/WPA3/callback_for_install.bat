set testMode=%1
set LOCAL_DIR=%~dp0
echo "Install certificate for WPA3 ..."

adb shell chmod 777 /data/misc/wifi/
adb push %LOCAL_DIR%\wifi.cfg data/misc/wifi/
adb push %LOCAL_DIR%\cas.pem data/misc/wifi/
adb push %LOCAL_DIR%\wifiuser.pem data/misc/wifi/
adb push %LOCAL_DIR%\wifiuser.key data/misc/wifi/