
set root=Z:\worktmp\codebase\HIDL
set project=lego6737m_basic
set repeat=1
set filter=CdmaSmsTest.*



adb remount

adb shell rm /data/nativetest/rilproxytest/rilproxytest

adb push %root%\out\target\product\%project%\vendor\lib\libmtk-rilproxy.so     /system/vendor/lib


adb push %root%\out\target\product\%project%\data\nativetest\rilproxytest\rilproxytest   /data/nativetest/rilproxytest/rilproxytest


adb shell chmod 700 /data/nativetest/rilproxytest/rilproxytest
adb shell /data/nativetest/rilproxytest/rilproxytest   --gtest_repeat=%repeat%  --gtest_filter=%filter%

PAUSE

