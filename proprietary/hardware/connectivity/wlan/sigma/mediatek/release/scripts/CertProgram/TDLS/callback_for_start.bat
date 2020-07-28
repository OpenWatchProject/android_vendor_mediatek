set gen3IC=no
adb shell getprop | findstr persist.vendor.connsys | findstr 6630 > nul && (set gen3IC=yes)
adb shell getprop | findstr persist.vendor.connsys | findstr 6797 > nul && (set gen3IC=yes)
if %gen3IC%==yes adb shell /data/bin/iwpriv wlan0 set_sw_ctrl 0xa0400000 0x1

adb shell /data/bin/iwpriv wlan0 set_sw_ctrl 0x10010003 0xf