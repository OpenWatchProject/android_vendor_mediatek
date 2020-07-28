set PROJECT=k57v1_64_op02
adb wait-for-device
adb root
adb wait-for-device
adb push out\target\product\%PROJECT%\obj\EXECUTABLES\abftest_intermediates\abftest /data/
adb shell "chmod 777 /data/abftest"
adb shell "/data/abftest"