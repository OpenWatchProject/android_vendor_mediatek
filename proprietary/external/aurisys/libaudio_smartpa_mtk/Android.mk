ifeq ($(findstring mt6660, $(MTK_AUDIO_SPEAKER_PATH)), mt6660)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/smartpa_param.cpp \
    src/parser.cpp

LOCAL_CFLAGS += -DMTK_AURISYS_FRAMEWORK_SUPPORT

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/aurisys/interface \
    $(MTK_PATH_SOURCE)/hardware/audio/common/V3/include \
    $(MTK_PATH_SOURCE)/hardware/audio/common/include \
    $(MTK_PATH_SOURCE)/hardware/audio/common/utility \
    external/tinyalsa/include \
    external/boringssl/src/include

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog \
    libnvram \
    libtinyalsa \
    libcrypto \
    libssl

LOCAL_MODULE := libaudiosmartpamtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
include $(MTK_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    test/smartpa_nvtest.c

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/nvram/libfile_op

# LOCAL_CFLAGS += -D__DEBUG

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog \
    libnvram \
    libfile_op

LOCAL_MODULE := smartpa_nvtest
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
include $(MTK_EXECUTABLE)
endif
