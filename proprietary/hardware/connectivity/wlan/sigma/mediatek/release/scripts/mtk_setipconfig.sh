#!/system/bin/sh
# $1 interface name
# $2 ip address
# $3 net mask
# $4 dns1
# $5 dns2
#
ifconfig $1 $2 netmask $3
if [ "$4" != "" ]; then
setprop dhcp.$1.dns1 $4
setprop net.dns1 $4
fi

if [ "$5" != "" ]; then
setprop dhcp.$1.dns1 $5
setprop net.dns2 $5
fi
