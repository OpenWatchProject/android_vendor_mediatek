adb shell "setprop vendor.wifi.sigma.ip.addr %PCENDPOINT_IP_ADDRESS%"
adb shell "setprop vendor.wifi.sigma.ip.netmask %PCENDPOINT_IP_NETMASK%"

:: Set driver's log level to default
adb shell "echo 0xff:0x2f > /proc/net/wlan/dbgLevel"

:: Enlarge kernel's fragment queue size to 20MB
adb shell "echo 20971520 > /proc/sys/net/ipv4/ipfrag_high_thresh"