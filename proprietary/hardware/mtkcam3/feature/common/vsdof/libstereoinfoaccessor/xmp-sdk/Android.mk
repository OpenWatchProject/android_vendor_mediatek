LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

SOURCE_FILES := $(wildcard $(LOCAL_PATH)/XMPCore/source/*.cpp)
SOURCE_FILES += $(wildcard $(LOCAL_PATH)/source/*.cpp)
SOURCE_FILES += $(wildcard $(LOCAL_PATH)/third-party/zuid/interfaces/*.cpp)
SOURCE_FILES += $(wildcard $(LOCAL_PATH)/test/*.cpp)

LOCAL_SRC_FILES := $(SOURCE_FILES:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES +=                             \
    $(LOCAL_PATH)/public/include                \
    $(LOCAL_PATH)/public/include/client-glue    \
    $(LOCAL_PATH)/source/                       \
    $(LOCAL_PATH)/third-party/zuid/interfaces/  \
    $(LOCAL_PATH)/XMPCore/source/               \
    $(LOCAL_PATH)/XMPCore/resource/linux/       \
    $(LOCAL_PATH)/build/                        \
    $(LOCAL_PATH)/test/                         \
    $(TOP)/external/zlib/                       \
    $(TOP)/external/expat/lib

LOCAL_CFLAGS += -DHAVE_MEMMOVE -DUNIX_ENV -DSTDC -fexceptions

APP_STL := stlport_shared

LOCAL_SHARED_LIBRARIES := libexpat libz libcutils liblog libutils

LOCAL_MODULE := libxmp_vsdof
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_STATIC_LIBRARY)