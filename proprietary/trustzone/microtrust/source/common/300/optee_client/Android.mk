ifeq ($(MICROTRUST_TEE_SUPPORT),yes)

LOCAL_PATH := $(call my-dir)

################################################################################
# Build libteec.so - TEE (Trusted Execution Environment) shared library        #
################################################################################
include $(CLEAR_VARS)

# CFG_TEE_CLIENT_LOG_LEVEL
#   Client (User Non Secure) log level
#   Supported values: 0 (no traces) to 4 (all traces)
CFG_TEE_CLIENT_LOG_LEVEL?=1

# CFG_TEE_CLIENT_LOG_FILE
# The location of the client log file when logging to file is enabled.
# CFG_TEE_CLIENT_LOG_FILE ?= \"/data/vendor/teec.log\"


ifneq ($(CFG_TEE_CLIENT_LOG_FILE),)
LOCAL_CFLAGS += -DTEEC_LOG_FILE=$(CFG_TEE_CLIENT_LOG_FILE)
endif

#Pass default tee to open
ifeq ($(strip $(DEV_FEATURE_GP_SUPPORT)), yes)
LOCAL_CFLAGS+=-DDEFAULT_TEE_GPTEE
endif

LOCAL_CFLAGS += -DDEBUGLEVEL_$(CFG_TEE_CLIENT_LOG_LEVEL)
LOCAL_CFLAGS += -DBINARY_PREFIX=\"TEEC\"

LOCAL_SRC_FILES := libteec/src/tee_client_api.cpp \
                  libteec/src/teec_trace.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/public \
                    $(LOCAL_PATH)/libteec/include \
					$(LOCAL_PATH)/libteec/src


LOCAL_MODULE := libTEECommon
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/public

LOCAL_SHARED_LIBRARIES := \
    libutils

include $(BUILD_SHARED_LIBRARY)


endif
