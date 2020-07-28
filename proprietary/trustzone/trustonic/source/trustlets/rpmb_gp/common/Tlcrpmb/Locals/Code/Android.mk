# =============================================================================
#
# Makefile responsible for:
# - building a test binary - tlcRpmb
# - building the TLC library - libtlcRpmbLib
#
# =============================================================================

# Do not remove this - Android build needs the definition
LOCAL_PATH	:= $(call my-dir)

# =============================================================================
# library tlcrpmbLib
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcrpmbLib

# Add your folders with header files here
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/inc \
	$(COMP_PATH_Tlrpmb)/Public \
	$(COMP_PATH_TlSdk)/Public/MobiCore/inc

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= tlcrpmb.cpp

# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libMcClient
LOCAL_SHARED_LIBRARIES += libsec_mem

ifeq ($(MTK_K64_SUPPORT), yes)
MOBICORE_LIB_PATH +=\
    $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/external/mobicore/MobiCoreDriverLib/ClientLib/public \
    $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/external/mobicore/common/MobiCore/inc
else
MOBICORE_LIB_PATH +=\
    $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/external/mobicore/MobiCoreDriverLib/ClientLib/public \
    $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/external/mobicore/common/MobiCore/inc
endif

LOCAL_C_INCLUDES += \
    $(MOBICORE_LIB_PATH) \
    $(LOCAL_PATH)/../../../Tlrpmb/Locals/Code/public

LOCAL_CFLAGS += -DTBASE_API_LEVEL=3

#
# Attention:
# Enabling CFG_RPMB_KEY_PROGRAMED_IN_KERNEL means accepting the risk of revealing key in
# Linux Kernel. Mediatek won’t take the responsibility for loss incurred by the key revealing.
#
#LOCAL_CFLAGS += -DCFG_RPMB_KEY_PROGRAMED_IN_KERNEL


include $(BUILD_STATIC_LIBRARY)


# =============================================================================
# binary tlcrpmb
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcrpmb_gp

# Add your source files here
LOCAL_SRC_FILES	+= main.cpp

LOCAL_STATIC_LIBRARIES := tlcrpmbLib
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libMcClient
LOCAL_SHARED_LIBRARIES += libsec_mem

ifeq ($(MTK_K64_SUPPORT), yes)
MOBICORE_LIB_PATH +=\
    $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/external/mobicore/MobiCoreDriverLib/ClientLib/public \
    $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/external/mobicore/common/MobiCore/inc
else
MOBICORE_LIB_PATH +=\
    $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/external/mobicore/MobiCoreDriverLib/ClientLib/public \
    $(TOP)/vendor/trustonic/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/external/mobicore/common/MobiCore/inc
endif

LOCAL_C_INCLUDES += \
    $(MOBICORE_LIB_PATH) \
    $(LOCAL_PATH)/../../../Tlrpmb/Locals/Code/public

#
# Attention:
# Enabling CFG_RPMB_KEY_PROGRAMED_IN_KERNEL means accepting the risk of revealing key in
# Linux Kernel. Mediatek won’t take the responsibility for loss incurred by the key revealing.
#
#LOCAL_CFLAGS += -DCFG_RPMB_KEY_PROGRAMED_IN_KERNEL

include $(BUILD_EXECUTABLE)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
