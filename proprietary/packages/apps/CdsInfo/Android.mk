ifneq ($(strip $(TARGET_BUILD_VARIANT)),user)
ifneq ($(strip $(MTK_GMO_RAM_OPTIMIZE)), yes)
ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := CDS_INFO
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

LOCAL_JAVA_LIBRARIES += telephony-common \
                        mediatek-telephony-base \
                        mediatek-telephony-common
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.nvram-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.netdagent-V1.0-java

include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
endif
endif
endif

ifeq ($(strip $(MTK_GMO_RAM_OPTIMIZE)), yes)
ifeq ($(strip $(TARGET_BUILD_VARIANT)), eng)
ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := CDS_INFO
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

LOCAL_JAVA_LIBRARIES += telephony-common \
                        mediatek-telephony-base \
                        mediatek-telephony-common
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.nvram-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.netdagent-V1.0-java

include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
endif
endif
endif

