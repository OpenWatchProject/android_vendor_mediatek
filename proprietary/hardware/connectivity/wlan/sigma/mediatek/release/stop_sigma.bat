@echo off
for /f "tokens=2" %%i in ('tasklist /V ^| findstr Control-Agent') do (taskkill /FI "pid eq %%i")
for /f "tokens=2" %%i in ('tasklist /V ^| findstr WFA-DUT') do (taskkill /FI "pid eq %%i")
for /f "tokens=2" %%i in ('tasklist /V ^| findstr PFService') do (taskkill /FI "pid eq %%i")
for /f "tokens=2" %%i in ('tasklist /V ^| findstr HS20-WPA_CLI-DAEMON') do (taskkill /FI "pid eq %%i")
adb shell "killall hostapd"
if "%~1"=="force" goto :stop
choice /C YN /M "press Y to turn off wlan, press N to leave wlan in sigma mode"
if ERRORLEVEL 2 goto :eof
:stop
adb shell ps -A|findstr wpa_supplicant>nul || echo wlan already off && goto :eof
rem adb shell ls /data/misc/wifi/sockets/wpa_ctrl* | findstr /C:"No such file">nul || echo wlan is in normal mode && goto :eof
echo stop wifi
rem adb shell setprop wlan.driver.status unloaded
rem adb shell wpa_cli -g@android:wpa_wlan0 IFNAME=wlan0 TERMINATE
adb shell setprop ctl.stop wpa_supplicant
adb shell "echo 0 > /dev/wmtWifi"

echo turning wifi off, wait 2 seconds
ping 127.0.0.1 -n 2 > nul
goto :stop