@echo off

set TEST_MODULE=nvramtest
set TEST_FILE=X:\fpb-m0.mp9\out\target\product\demo97v1_64_4cam\data\nativetest\%TEST_MODULE%

adb root
adb remount
adb shell mkdir /data/nativetest/%TEST_MODULE%
adb push %TEST_FILE% /data/nativetest/%TEST_MODULE%
adb shell chmod 777 /data/nativetest/%TEST_MODULE%/%TEST_MODULE%
echo.
echo run cmd:
echo adb shell /data/nativetest/%TEST_MODULE%/%TEST_MODULE%
echo.
echo log cmd:
echo adb shell logcat -c^&^&adb shell logcat -s nvram_test -v threadtime^>nvramtest.log