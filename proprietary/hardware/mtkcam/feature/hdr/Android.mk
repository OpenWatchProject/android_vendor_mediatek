ifneq ($(strip $(MTK_EMULATOR_SUPPORT)), yes)
ifneq ($(BUILD_MTK_LDVT), yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

###############################################################
# Version definitions
MTK_HDR_MODULE_BOOST_VERSION_MAJOR    := 1
MTK_HDR_MODULE_BOOST_VERSION_MINOR    := 0
MTK_HDR_MODULE_BOOST_VERSION          := \
	$(MTK_HDR_MODULE_BOOST_VERSION_MAJOR).$(MTK_HDR_MODULE_BOOST_VERSION_MINOR)

###############################################################
# Module path
MTK_HDR_MODULE := modules
MTK_HDR_MODULE_BOOST := $(MTK_HDR_MODULE)/boost
###############################################################

include $(call all-makefiles-under, $(LOCAL_PATH))

endif
endif
