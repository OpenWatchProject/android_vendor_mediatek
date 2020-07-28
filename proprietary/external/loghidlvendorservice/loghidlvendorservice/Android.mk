LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= loghidlvendorservice_main.cpp\
	log_hidl_service.cpp

LOCAL_MODULE := loghidlvendorservice
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_INIT_RC := loghidlvendorservice.rc
LOCAL_C_INCLUDES += \
    external/libcxx/include \
    bionic \
    bionic/libstdc++/include \
	external/openssl/include\
	external/libpcap

LOCAL_STATIC_LIBRARIES += libpcap
LOCAL_SHARED_LIBRARIES := libsysutils libcutils libnetutils libcrypto liblog libc++ libutils \
	libdl\
	libbase \
	libhardware \
	libbinder \
	libhidlbase \
	libhidltransport \
	libutils \
	vendor.mediatek.hardware.log@1.0

#LOCAL_STATIC_LIBRARIES += libdrvb
include $(MTK_EXECUTABLE)
