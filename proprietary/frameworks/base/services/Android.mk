LOCAL_PATH:= $(call my-dir)

# merge all required services into one jar
# ============================================================
include $(CLEAR_VARS)

LOCAL_MODULE := mediatek-services
#LOCAL_DEX_PREOPT_APP_IMAGE := true
#LOCAL_DEX_PREOPT_GENERATE_PROFILE := true
#LOCAL_DEX_PREOPT_PROFILE_CLASS_LISTING := $(LOCAL_PATH)/art-profile

LOCAL_SRC_FILES := $(call all-java-files-under,core/java/com/mediatek/server)
LOCAL_SRC_FILES += core/java/com/mediatek/server/anr/EventLogTags.logtags

# Uncomment to enable output of certain warnings (deprecated, unchecked)
# LOCAL_JAVACFLAGS := -Xlint

# Services that will be built as part of meidatek-services.jar
# These should map to directory names relative to this
# Android.mk.
services := \
       mtksearchengineservice \
       hdmilocalservice \
       powerhalmgrservice \

ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)
ifneq ($(wildcard vendor/mediatek/proprietary/hardware/omadm/Android.mk),)
services += omadmservice
endif
endif
endif

ifeq ($(strip $(MSSI_MTK_VOW_2E2K_SUPPORT)), yes)
services += vow
endif

ifneq ($(wildcard vendor/mediatek/proprietary/frameworks/opt/fm/Android.bp),)
services += fmradioservice
endif

# The convention is to name each service module 'services.$(module_name)'
LOCAL_STATIC_JAVA_LIBRARIES := $(addprefix services.,$(services))
LOCAL_STATIC_JAVA_LIBRARIES += duraspeednative

LOCAL_JAVA_LIBRARIES := services.core mediatek-framework mediatek-common\
    android.hardware.power-V1.0-java \
    vendor.mediatek.hardware.power-V2.0-java \
    mediatek-framework

ifneq ($(wildcard vendor/mediatek/proprietary/frameworks/opt/fm/Android.bp),)
LOCAL_JAVA_LIBRARIES += mediatek-fm-framework
endif

ifeq ($(strip $(MTK_FULLSCREEN_SWITCH_SUPPORT)), yes)
LOCAL_STATIC_JAVA_LIBRARIES += FullscreenSwitchService
endif

LOCAL_STATIC_JAVA_LIBRARIES += com_mediatek_amplus

LOCAL_MODULE_PATH = $(PRODUCT_OUT)/system/framework

include $(BUILD_JAVA_LIBRARY)

ifeq (,$(ONE_SHOT_MAKEFILE))
# A full make is happening, so make everything.
include $(call all-makefiles-under,$(LOCAL_PATH))
else
# If we ran an mm[m] command, we still want to build the individual
# services that we depend on. This differs from the above condition
# by only including service makefiles and not any tests or other
# modules.
include $(patsubst %,$(LOCAL_PATH)/%/Android.mk,$(services))
endif

