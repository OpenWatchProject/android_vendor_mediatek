############################################################################
# Translator binary to translate KPIs.
# Native shared library, loaded by DMC framework dynamically
############################################################################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libtranslator_oem_v1.0
LOCAL_MODULE_OWNER := mtk
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

# AP Monitor(APM) messages definition header
LOCAL_HEADER_LIBRARIES := \
    libapm_msg_defs_headers

LOCAL_SRC_FILES := \
    $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp)) \
    $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/translator/v1.0/*.cpp))

# Modem Monitor(MDM) utility library
LOCAL_STATIC_LIBRARIES := libmdmonitor

LOCAL_SHARED_LIBRARIES := \
    libtranslator_utils \
    libcutils \
    libutils

# Install module to vendor partition
LOCAL_PROPRIETARY_MODULE := true

LOCAL_LDLIBS := -llog
LOCAL_MODULE_TAGS := optional

include $(MTK_SHARED_LIBRARY)