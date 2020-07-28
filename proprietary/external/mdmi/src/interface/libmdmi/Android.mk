LOCAL_PATH:= $(call my-dir)

include_headers := \
    mdmi_public_headers \
    libmdmi_core_headers

source_files := \
    $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp))

shared_libraries := \
    libmdmi_core \
    libcutils

#================== DEBUG ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_debug
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_DEBUG
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== LTE ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_lte
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_LTE
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== GSM ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_gsm
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_GSM
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== UMTS ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_umts
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_UMTS
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== WIFI ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_wifi
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_WIFI
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== IMS ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_ims
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_IMS
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== COMMANDS ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_commands
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_COMMANDS
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== WCDMA ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_wcdma
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_WCDMA
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== HSUPA ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_hsupa
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_HSUPA
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== HSDPA ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_hsdpa
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_HSDPA
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

#================== EMBMS ===================
include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := libVzw_mdmi_embms
LOCAL_CFLAGS := -DBUILD_MDMI_LIB_EMBMS
LOCAL_HEADER_LIBRARIES := $(include_headers)
LOCAL_SRC_FILES := $(source_files)
LOCAL_SHARED_LIBRARIES := $(shared_libraries)
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)