LOCAL_PATH:= $(call my-dir)

###[libpkm]###
include $(CLEAR_VARS)
LOCAL_MODULE := libpkm

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/src/include

LOCAL_HEADER_LIBRARIES += libapm_msg_defs_headers

LOCAL_SRC_FILES :=  src/PacketMonitor.cpp \
                    src/libpkm.cpp \
                    src/MdmAdapter.cpp \
                    src/SecurityAssociation.cpp

LOCAL_STATIC_LIBRARIES := libmtkpcap libmdmonitor

LOCAL_SHARED_LIBRARIES := libutils \
                          libcutils \
                          libc++ \
                          libdecrypt \
                          libpcap \
                          liblog \
                          libhidlbase \
                          libhidltransport \
                          libhwbinder \
                          vendor.mediatek.hardware.mdmonitor@1.0 \
                          vendor.mediatek.hardware.apmonitor@2.0 \
                          libapmonitor_vendor

LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DMTK_MDMI_SUPPORT
LOCAL_CPPFLAGS += -fexceptions
include $(MTK_SHARED_LIBRARY)

###[pkm_service]###
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/src/include

LOCAL_SRC_FILES := src/main.cpp
LOCAL_MODULE:= pkm_service
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_INIT_RC := src/etc/pkm_service.rc
include $(MTK_EXECUTABLE)
