#!/bin/bash

source system/tools/hidl/update-makefiles-helper.sh

function do_hash_update() {
  local root_or_cwd=${ANDROID_BUILD_TOP%%/}${ANDROID_BUILD_TOP:+/}

  local current_package=$(package_root_to_package $1)
  local current_dir=$root_or_cwd$(package_root_to_root $1)

  #echo "Updating makefiles for $current_package in $current_dir."

  check_dirs "$root_or_cwd" $@ || return 1

  local packages=$(get_packages $current_dir $current_package) || return 1
  local root_arguments=$(get_root_arguments $@) || return 1

  for p in $packages; do
    #echo "Updating $p";
    local additional_options=
    if [[ -f $(get_package_dir $current_dir $current_package $p)/.hidl_for_test ]]; then additional_options="-t"; fi
    hidl-gen -Lhash $root_arguments $p
    echo "`hidl-gen -Lhash $root_arguments $p`" >> current_new.txt 2>&1;
    rc=$?; if [[ $rc != 0 ]]; then return $rc; fi
  done
}


function do_check_hash() {
  local new_hash_file="current_new.txt"
  local old_hash_file="$(dirname $0 | sed -e 's/^\.\///')/current.txt"

  local old_module=`sed -e "s/#.*//g" $old_hash_file | awk '{if (length !=0) print $2}'` || return 1
  local new_module=`awk '{print $2}' $new_hash_file` || return 1

  # delete hash line and modify exist module's hash code
  for i in $old_module; do
  if [ ! "`grep -wrn "$i" $new_hash_file`" ]; then
  sed -i "/${i}/d" $old_hash_file; fi
  done

  for i in $old_module; do
    for j in $new_module;do
    if [ $j == $i ]; then
    num_old=`grep -wn "$i" $old_hash_file |cut -d ":" -f1`
    num_new=`grep -wn "$i" $new_hash_file |cut -d ":" -f1`
    line=`awk 'NR=='"$num_new"'{print}' $new_hash_file`
    sed -i "${num_old}s/^.*$/$line/" $old_hash_file; fi
    done
  done

  # append new hash line to the end
  for m in $new_module; do
    if [ ! "`grep -wrn "$m" $old_hash_file`" ]; then
     add=`grep -wrn "$m" $new_hash_file |cut -d ":" -f1`
     addline=`awk 'NR=='"$add"'{print}' $new_hash_file`
     echo "$addline" >> $old_hash_file
    fi
  done

}


mydir=$(dirname $0 | sed -e 's/^\.\///')
do_hash_update \
  "vendor.mediatek.hardware:${mydir}" \
  "android.hardware:hardware/interfaces" \
  "android.hidl:system/libhidl/transport"

do_check_hash
rm current_new.txt
