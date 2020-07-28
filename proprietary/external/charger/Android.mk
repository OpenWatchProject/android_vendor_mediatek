LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    common.cpp \
    lights.cpp \
    bootlogo.cpp \
    main.cpp \
    key_control.cpp \
    charging_control.cpp

LOCAL_CFLAGS += $(MTK_CDEFS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libshowlogo/                   \
 $(LOCAL_PATH)/include \
 $(TOP)/external/zlib/

LOCAL_MODULE:= kpoc_charger
# Move to system partition for Android O migration
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
LOCAL_INIT_RC := kpoc_charger.rc

#bobule workaround pdk build error, needing review
LOCAL_MULTILIB := 32

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libc \
    libstdc++ \
    libz \
    libdl \
    liblog \
    libui \
    libshowlogo \
    libhardware_legacy \
    libsuspend \
    android.hardware.light@2.0 \
    android.hardware.health@2.0 \
    libbase \
    libhidlbase \
    libhidltransport \

LOCAL_STATIC_LIBRARIES := libhealthhalutils
# Do not build healthd since we use android.hardware.health@2.0
LOCAL_OVERRIDES_MODULES := healthd

include $(MTK_EXECUTABLE)
