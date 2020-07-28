#!/bin/bash

source system/tools/hidl/update-makefiles-helper.sh

do_makefiles_update \
  "vendor.microtrust.hardware:vendor/mediatek/proprietary/trustzone/microtrust/source/common/interfaces" \
  "vendor.mediatek.hardware:vendor/mediatek/proprietary/hardware/interfaces" \
  "android.hardware:hardware/interfaces" \
  "android.hidl:system/libhidl/transport"
