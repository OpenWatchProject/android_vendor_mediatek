#!/bin/bash -

top_folder_apath=`pwd`
local_folder_rpath=`dirname $0`

$local_folder_rpath/update-makefiles.sh

function del_unused_library
{
	package_name=$1
	bp_file=$2

	awk 'BEGIN{name="'"$package_name"'"; eprint=1; store=""}
		{
			if(match($0,"cc_library")) {
				eprint=0;
				store=$0;
			} else if (eprint==0 && match($0, "name: \""name"_vendor\",")) {
				eprint=1;
				print store;
			}
			if(eprint)
				print;
			if(eprint==0 && $0 == "}") {
				eprint=1;
			}
		}
	' $bp_file > ./bp.tmp
	mv bp.tmp $bp_file
}

folders=`find -L $local_folder_rpath -name types.hal -exec dirname {} \;`
for folder in $folders
do
	cd $top_folder_apath/$folder
	if [ -e Android.mk ];then
		rm Android.mk
	fi
	if [ -e Android.bp ];then
		package_name=`cat *Device.hal | grep "^package.*;$" | sed -r 's/package (.*);/\1/g'`
		del_unused_library $package_name "Android.bp"
	fi
done
