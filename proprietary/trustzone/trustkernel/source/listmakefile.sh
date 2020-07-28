#!/bin/bash 

list_makefile () {
	if [ -f $1/Android.mk ]; then
		echo $1/Android.mk
		ret=1
	else 
		if (( $ret == 0 )); then
			for dir in `find $1 -maxdepth 1 -type d -not -path $1`; do
				list_makefile $dir
				ret=0
			done
		fi
	fi
}

path=vendor/mediatek/proprietary/trustzone/trustkernel/source/external
ret=0
list_makefile $path
