#!/system/bin/sh
if [ "$1" == "" ]; then
	exit
fi
ifname=$1
mac_addr=`ifconfig $ifname | grep HWaddr | awk '{print $5}'`
temp=`ifconfig $ifname | grep "inet addr"`
ip=`echo $temp | awk -F: '{print $2}' | awk '{print $1}'`
mask=`echo $temp | awk -F: '{print $4}'`
dns1=`getprop net.dns1`
dns2=`getprop net.dns2`
if [ "$mac_addr" != "" ]; then
echo mac_addr=$mac_addr
fi
if [ "$ip" != "" ]; then
echo ip=$ip
fi
if [ "$mask" != "" ]; then
echo mask=$mask
fi
if [ "$dns1" != "" ]; then
echo dns1=$dns1
fi
if [ "$dns2" != "" ]; then
echo dns2=$dns2
fi