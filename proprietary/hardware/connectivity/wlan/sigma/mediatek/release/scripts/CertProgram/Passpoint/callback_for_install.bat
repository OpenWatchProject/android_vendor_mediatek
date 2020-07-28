@echo off
set LOCAL_DIR=%~dp0
set HS20_DIR=/data/misc/wifi/hs20
set HS20_PPS_DIR=%HS20_DIR%/PPS
set HS20_CERT_DIR=%HS20_DIR%/Certs
set HS20_EAP_DIR=/data/misc/wifi/

::Push hs20 osu client
echo [hs20]Push hs20 osu client
adb shell mkdir %HS20_DIR%
adb shell chmod 777 %HS20_DIR%
adb shell mkdir %HS20_DIR%/Logs
adb shell chmod 777 %HS20_DIR%/Logs
adb push %LOCAL_DIR%\hs20-osu-client %HS20_DIR%/
adb shell chmod 777 %HS20_DIR%/hs20-osu-client

::Push relevant ppsmo
echo [hs20]Pushing ppsmo files...
adb push %LOCAL_DIR%\STAUT-ARUBAOSU/. %HS20_PPS_DIR%/
adb push %LOCAL_DIR%\STAUT-RUCKUSOSU/. %HS20_PPS_DIR%/

::Push certifications
echo [hs20]Pushing certifications...
adb push %LOCAL_DIR%\Certs/. %HS20_CERT_DIR%/

::Push osu client relevant xmls
echo [hs20]Pushing osu client xmls...
adb push %LOCAL_DIR%\spp.xsd %HS20_DIR%/
adb shell chmod 777 %HS20_DIR%/spp.xsd
adb push %LOCAL_DIR%\devdetail.xml %HS20_DIR%/
adb shell chmod 777 %HS20_DIR%/devdetail.xml
adb push %LOCAL_DIR%\devinfo.xml %HS20_DIR%/
adb shell chmod 777 %HS20_DIR%/devinfo.xml

::Push cacert.pem for ttls
adb push %LOCAL_DIR%\cacert.pem %HS20_EAP_DIR%/
adb shell chmod 777 %HS20_EAP_DIR%/cacert.pem
adb push %LOCAL_DIR%\cacert.pem %HS20_DIR%/
adb shell chmod 777 %HS20_DIR%/cacert.pem

::Install web application
ren %LOCAL_DIR%\wpadebug.abc wpadebug.apk
adb install -r %LOCAL_DIR%\wpadebug.apk

::Push hosts
adb push %LOCAL_DIR%\hosts /etc/