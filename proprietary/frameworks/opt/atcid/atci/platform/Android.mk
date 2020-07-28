LOCAL_SRC_FILES += \
    platform/atcid_adaptation.cpp \
    platform/atcid_cust_cmd_platform.c

LOCAL_SHARED_LIBRARIES += \
    vendor.mediatek.hardware.atci@1.0 \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    libnvram \
    libfile_op

ifeq ($(MTK_WLAN_SUPPORT),yes)
    LOCAL_SHARED_LIBRARIES += libwifitest
    LOCAL_CFLAGS += -DMTK_WLAN_FEATURE
endif

ifeq ($(MTK_GPS_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_GPS_FEATURE
endif

ifeq ($(MTK_TC1_FEATURE),yes)
    LOCAL_SRC_FILES += \
        src/atcid_cust_tc1_cmd.c
    LOCAL_SHARED_LIBRARIES += libtc1rft
    LOCAL_CFLAGS += -DMTK_TC1_FEATURE
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/connectivity/wifi_tc1_rft/
endif

LOCAL_C_INCLUDES += \
        $(MTK_PATH_SOURCE)/kernel/drivers/video \
        $(MTK_PATH_SOURCE)/kernel/drivers/leds \
        $(MTK_PATH_SOURCE)/hardware/connectivity/wlan/libwifitest \
        $(MTK_PATH_SOURCE)/external/nvram/libnvram \
        $(MTK_PATH_SOURCE)/external/nvram/libfile_op \
        $(MTK_PATH_SOURCE)/frameworks/opt/atcid/atci/platform
