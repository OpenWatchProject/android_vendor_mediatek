:: Disable AGING_TIMEOUT mechanism
adb shell "/data/bin/iwpriv wlan0 driver 'SET_SW_CTRL 0x10010004 1'"