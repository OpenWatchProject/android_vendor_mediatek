#!/bin/bash

usage()
{
	echo "Usage: ./dtbo_config.sh -k <KERNEL VERSION> -a <ARCH> -p <CURRENT_PROJECT_NAME> -b <BASE_PROJECT_NAME> -s <MTK_DTBO_FEATURE>"
	echo "for example: "
	echo "	    ./dtbo_config.sh -k kernel-4.4 -a arm64 -p k55_v1_64_ota  -b k55_v1_64 -s yes"
	exit
}

Supported_Kernel_List=( "kernel-3.18" "kernel-4.4" "kernel-4.9" "kernel-4.9-lc" "kernel-4.14" "kernel-4.19")
if [ "$#" -ne 10 ] ; then
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
		-b )	shift
			BASE_PROJECT=$1
			;;
		-s )	shift
			FEATURE=$1
			;;
	esac
	shift
done

configfile=$(dirname "$0")/../../../../../$KERNEL/arch/$ARCH/configs/"$PROJECT"_defconfig
debugconfigfile=$(dirname "$0")/../../../../../$KERNEL/arch/$ARCH/configs/"$PROJECT"_debug_defconfig

if [[ ( $KERNEL = "kernel-3.18" ||  $ARCH = "arm" ) ]] ; then
	projectdts=$(dirname "$0")/../../../../../$KERNEL/arch/$ARCH/boot/dts/"$PROJECT".dts
	dts_new_path="no"
else
	projectdts=$(dirname "$0")/../../../../../$KERNEL/arch/$ARCH/boot/dts/mediatek/"$PROJECT".dts
	dts_new_path="yes"
fi

if [ -f $projectdts ] ; then
	nr=`grep -m 1 -in "cust.dtsi" $projectdts | cut -d: -f1`
	if [ -z $nr ] ; then
		echo "WARNING: Cannot find pattern: \"cust.dtsi\""
		echo "Can be ignored only if you're cloning Tablet project"
	else
		sed -i ""$nr"d" $projectdts
		sed -i ""$nr"i #include <$BASE_PROJECT/cust.dtsi>" $projectdts
	fi
else
	echo "ERROR: Project DTS does not exist: $projectdts!!"
	exit 1
fi

if [ $ARCH = "arm64" ] ; then
	dtb_ovl_fo="CONFIG_BUILD_ARM64_DTB_OVERLAY_IMAGE_NAMES"
	dtb_fo="CONFIG_BUILD_ARM64_APPENDED_DTB_IMAGE_NAMES"
elif [ $ARCH = "arm" ] ; then
	dtb_ovl_fo="CONFIG_BUILD_ARM_DTB_OVERLAY_IMAGE_NAMES"
	dtb_fo="CONFIG_BUILD_ARM_APPENDED_DTB_IMAGE_NAMES"
else
	echo "WRONG ARCH:$ARCH"
	exit 1
fi


for ((index=0; index<${#Supported_Kernel_List[@]}; index++)); do
	found=
	if [[ $KERNEL = ${Supported_Kernel_List[index]} ]]; then
		echo "Correct Kernel Version!"
		found=1
		break
	fi
done
if [ ! $found ]; then
	echo "ERROR: Wrong Kernel Version: $KERNEL!!"
	exit 1
fi

if [ -e $configfile ] && [ -e $debugconfigfile ] ; then
	if [ $FEATURE = "no" ] ; then
		if [ $dts_new_path = "yes" ] ; then
			PROJECT="mediatek/$PROJECT"
		fi

		line="$dtb_fo=\"$PROJECT\""
		line_ovl="$dtb_ovl_fo=\"\""

		sed -i "s|$dtb_fo=.*|$line|g" $configfile
		sed -i "s|$dtb_fo=.*|$line|g" $debugconfigfile
		sed -i "/$dtb_ovl_fo/d" $configfile
		sed -i "/$dtb_ovl_fo/d" $debugconfigfile

	elif [ $FEATURE = "yes" ] ; then
		if [ $dts_new_path = "yes" ] ; then
			dts_file=$(dirname "$0")/../../../../../$KERNEL/arch/$ARCH/boot/dts/mediatek/$PROJECT.dts
		else
			dts_file=$(dirname "$0")/../../../../../$KERNEL/arch/$ARCH/boot/dts/$PROJECT.dts
		fi

		platform_dts=`grep -m 1 "#include [<\"]mt8.*\.dtsi[>\"]" $dts_file | sed 's/#include [<"]//g' | sed 's/\.dtsi[>"]//g'`
		if [ -z $platform_dts ] ; then
			echo "NOT TABLET PROJECT"
			platform_dts=`grep -m 1 "#include [<\"]mt.*\.dts[>\"]" $dts_file | sed 's/#include [<"]//g' | sed 's/\.dts[>"]//g'`
			if [ -z $platform_dts ] ; then
				echo "WARNING: CANNOT FIND PLATFORM ID in DTS!"
				platform_dts="no"
			fi
		else
			platform_dts=$platform_dts"_dtbo"
		fi

		echo "PLATFORM ID = $platform_dts"

		if [ $dts_new_path = "yes" ] ; then
			line="$dtb_fo=\"mediatek/$platform_dts\""
			line_ovl="$dtb_ovl_fo=\"mediatek/$PROJECT\""

		else
			line="$dtb_fo=\"$platform_dts\""
			line_ovl="$dtb_ovl_fo=\"$PROJECT\""
		fi

		if [ $platform_dts = "no" ] ; then
			cof_nr=`grep -m 1 -in $dtb_ovl_fo $configfile | cut -d: -f1`
			deb_nr=`grep -m 1 -in $dtb_ovl_fo $debugconfigfile | cut -d: -f1`
			if [ -z $cof_nr ] || [ -z $deb_nr ]; then
				echo "WARNING: Cannot find pattern: $dtb_ovl_fo!"
				exit 1
			else
				sed -i ""$cof_nr"d" $configfile
				sed -i ""$deb_nr"d" $debugconfigfile
				sed -i ""$cof_nr"i $line_ovl" $configfile
				sed -i ""$deb_nr"i $line_ovl" $debugconfigfile
			fi
		else
			sed -i "/$dtb_ovl_fo/d" $configfile
			sed -i "/$dtb_ovl_fo/d" $debugconfigfile
			sed -i "s|$dtb_fo=.*|$line\n$line_ovl|g" $configfile
			sed -i "s|$dtb_fo=.*|$line\n$line_ovl|g" $debugconfigfile
		fi
	else
		echo "ERROR: Wrong DTBO FEATURE: $FEATURE!!"
		exit 1
	fi

else
	echo "ERROR: $configfile or $debugconfigfile does not exist!!"
	exit 1
fi

