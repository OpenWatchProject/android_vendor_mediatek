#!/bin/bash

source system/tools/hidl/update-makefiles-helper.sh

mydir=$(dirname $0 | sed -e 's/^\.\///')
do_makefiles_update \
  "vendor.mediatek.hardware:${mydir}" \
  "android.hardware:hardware/interfaces" \
  "android.hidl:system/libhidl/transport"