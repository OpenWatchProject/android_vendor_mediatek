PKG_ROOT=vendor/mediatek/proprietary/hardware/interfaces
PACKAGE=vendor.mediatek.hardware.apmonitor@2.0
HIDL_GEN_OUT=$PKG_ROOT/apmonitor/2.0/default/

hidl-gen -o $HIDL_GEN_OUT -Landroidbp -rvendor.mediatek.hardware:$PKG_ROOT -randroid.hidl:system/libhidl/transport $PACKAGE
hidl-gen -o $HIDL_GEN_OUT -Lc++-impl -rvendor.mediatek.hardware:$PKG_ROOT -randroid.hidl:system/libhidl/transport $PACKAGE

sh vendor/mediatek/proprietary/hardware/interfaces/update-hash.sh