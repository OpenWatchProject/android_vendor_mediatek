#!/bin/bash

usage()
{
	echo "Usage: ./fbe_config.sh -k <KERNEL VERSION> -a <ARCH> -p <CURRENT_PROJECT_NAME> -e <ENABLE_FLAG>"
	echo "for example: "
	echo "	    ./fbe_config.sh -k kernel-4.4 -a arm64 -p k71v1_64_bsp_tee -e yes"
	exit
}

if [ "$#" -ne 8 ] ; then
	usage
fi

while [ "$1" != "" ]; do
	case $1 in
		-k )	shift
			KERNEL=$1
			;;
		-a )	shift
			ARCH=$1
			;;
		-p )	shift
			PROJECT=$1
			;;
		-e )	shift
			ENABLE_FLAG=$1
			;;
	esac
	shift
done
echo "start"

configfile=$(dirname "$0")/../../../../../$KERNEL/arch/$ARCH/configs/"$PROJECT"_defconfig
debugconfigfile=$(dirname "$0")/../../../../../$KERNEL/arch/$ARCH/configs/"$PROJECT"_debug_defconfig
check_defconfigs_path=$(dirname "$0")/../../../../../$KERNEL/scripts/check_defconfigs
echo "$configfile"
  ext4_security="CONFIG_EXT4_FS_SECURITY"
	ext4_encrypt_config="CONFIG_EXT4_ENCRYPTION=y"
	crypt_ctr="CONFIG_CRYPTO_CTR"
	crypt_sh256="CONFIG_CRYPTO_SHA256"

if [ $KERNEL != "kernel-4.4" ] &&  [ $KERNEL != "kernel-3.18" ] && [ $KERNEL != "kernel-4.9" ] && [ $KERNEL != "kernel-4.9-lc" ] && [ $KERNEL != "kernel-4.14" ]; then
	echo "ERROR: Wrong Kernel Version: $KERNEL!!"
	exit 1
fi

if [ -e $configfile ] && [ -e $debugconfigfile ] ; then
  if [ $ENABLE_FLAG = "yes" ]; then
		sed -i "/$ext4_encrypt_config/d" $configfile
		sed -i "/$ext4_encrypt_config/d" $debugconfigfile
		sed -i "/$ext4_security/a$ext4_encrypt_config" $configfile
		sed -i "/$ext4_security/a$ext4_encrypt_config" $debugconfigfile
		sed -i "/$crypt_ctr/d" $configfile
		sed -i "/$crypt_ctr/d" $debugconfigfile
		sed -i "/$crypt_sh256/d" $configfile
		sed -i "/$crypt_sh256/d" $debugconfigfile
    echo "set config"
  else
 		sed -i "/$ext4_encrypt_config/d" $configfile
		sed -i "/$ext4_encrypt_config/d" $debugconfigfile
	fi
else
	echo "ERROR: $configfile or $debugconfigfile does not exist!!"
	exit 1
fi

cd $(dirname "$0")/../../../../../$KERNEL
eval "./scripts/check_defconfigs -u arch/$ARCH/configs/"$PROJECT"_debug_defconfig"
eval "./scripts/check_defconfigs -u arch/$ARCH/configs/"$PROJECT"_defconfig"

