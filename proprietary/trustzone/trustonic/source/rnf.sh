#!/bin/bash

function usage()
{
	echo "Usage: rnf.sh [FILE_NAME_BEFORE] [FILE_NAME_AFTER]"
}

function param_check()
{
	if [ "${FILE_NAME_BEFORE}" == "" ]; then
		usage;
		exit 0
	fi

	if [ "${FILE_NAME_AFTER}" == "" ]; then
		usage;
		exit 0
	fi
}

function rnf()
{
	FILES=$(find . -name ${FILE_NAME_BEFORE})

	for f in ${FILES}; do
		OUT_FILE_NAME=`dirname $f`/${FILE_NAME_AFTER}
		echo $f '-->' ${OUT_FILE_NAME}
		mv $f $OUT_FILE_NAME
		done
}

FILE_NAME_BEFORE=$1
FILE_NAME_AFTER=$2
param_check;
rnf;
