@echo off
:preconfig

adb shell /data/bin/iwpriv wlan0 set_sw_ctrl 0xa0400000 0x1
adb shell "echo 0xff:0x2f > /proc/net/wlan/dbgLevel"

echo "Disable fast tx mode for MT6630/MT6631/MT6632/CONNAC"
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6630 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6797 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6759 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6758 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6771 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6775 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6632 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6765 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6761 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6779 > nul && (adb shell iwpriv wlan0 set_sw_ctrl 0xa0400000 0x6)

echo "Disable SmartGear mode for 2Nss projects"
adb shell getprop | findstr persist.vendor.connsys.chipid | findstr 6779 > nul && (adb shell "iwpriv wlan0 driver 'set_chip SmartGear 9 2'")

:preconfig_done

adb shell getprop | findstr ro.hardware | findstr 6755 > nul && goto jade_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6757 > nul && goto olympus_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6797 > nul && goto everest_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6735 > nul && goto denali_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6580 > nul && goto rainier_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6758 > nul && goto vinson_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6771 > nul && goto vinson_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6775 > nul && goto vinson_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6763 > nul && goto bianco_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6739 > nul && goto zion_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6765 > nul && goto connac_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6761 > nul && goto connac_performance_mode
adb shell getprop | findstr ro.hardware | findstr 6779 > nul && goto connac_performance_mode
goto end

:olympus_performance_mode
:jade_performance_mode
adb shell "echo 4 4 > /proc/ppm/policy/ut_fix_core_num"
adb shell "echo 0 0 > /proc/ppm/policy/ut_fix_freq_idx"
goto end

:everest_performance_mode
adb shell "echo 4 4 2 > /proc/ppm/policy/ut_fix_core_num"
adb shell "echo 0 0 0 > /proc/ppm/policy/ut_fix_freq_idx"
goto end

:vinson_performance_mode
adb shell "echo 4 4 > /proc/ppm/policy/ut_fix_core_num"
adb shell "echo 0 0  > /proc/ppm/policy/ut_fix_freq_idx"
goto end

:rainier_performance_mode
:denali_performance_mode
adb shell "echo 0 > /proc/hps/enabled"
adb shell "echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"
adb shell "echo 1 > /sys/devices/system/cpu/cpu0/online"
adb shell "echo 1 > /sys/devices/system/cpu/cpu1/online"
adb shell "echo 1 > /sys/devices/system/cpu/cpu2/online"
adb shell "echo 1 > /sys/devices/system/cpu/cpu3/online"
goto end

:bianco_performance_mode
adb shell "echo enable 0 > /sys/kernel/debug/mcdi/mcdi_state"
adb shell "echo 4 4 > /proc/ppm/policy/ut_fix_core_num"
adb shell "echo 0 0 > /proc/ppm/policy/ut_fix_freq_idx"
adb shell "echo soidle3 0 > /sys/kernel/debug/cpuidle/soidle3_state"
adb shell "echo soidle 0 > /sys/kernel/debug/cpuidle/soidle_state"
adb shell "echo dpidle 0 > /sys/kernel/debug/cpuidle/dpidle_state"
goto end

:zion_performance_mode
adb shell "echo 10:3000000000 > /proc/net/wlan/autoPerfCfg"
goto end

:connac_performance_mode
adb shell "svc power stayon true"
adb shell "echo 0 0 > /proc/ppm/policy/ut_fix_freq_idx"
adb shell "echo 100 > /proc/perfmgr/eas/debug_ta_boost"

:cervino_maximize_socket_window_size
REM adb shell "echo Maximize socket window size to 8MB"
REM adb shell "echo 8388608 > /proc/sys/net/core/wmem_default"
REM adb shell "cat /proc/sys/net/core/wmem_default"

goto end

:end

