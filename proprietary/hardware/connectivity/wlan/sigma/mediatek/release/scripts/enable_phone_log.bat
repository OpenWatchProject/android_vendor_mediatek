echo "Start to enable phone log.."

adb shell uname -r|findstr 3.18>nul || goto SUPPRESS_KERNEL_44_LOG_TOOMUCH
echo "Disable kernel log too much for 3.18"
adb shell "echo 3 > /proc/mtprintk"
goto ENABLE_WPA_EXCE_LOG

:SUPPRESS_KERNEL_44_LOG_TOOMUCH
echo "Disable kernel log too much for 4.4"
adb shell "echo 2 > /proc/mtprintk"

:ENABLE_WPA_EXCE_LOG
echo "Enable WPA excessive log"
adb shell "wpa_cli -g@android:wpa_wlan0 IFNAME=wlan0 log_level excessive"

:ENABLE_DRIVER_LOG
echo "Enable driver debug log"
adb shell "echo 0xff:0x7f > /proc/net/wlan/dbgLevel"
::adb shell "echo \"8\" > /proc/sys/kernel/printk"

echo "Finish enabling phone log."