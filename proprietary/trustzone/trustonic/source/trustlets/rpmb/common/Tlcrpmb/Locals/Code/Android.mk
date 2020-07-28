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
	$(LOCAL_PATH)/inc

# Add your source files here (relative paths)
LOCAL_SRC_FILES	+= tlcrpmb.cpp

# Need the MobiCore client library
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libMcClient
LOCAL_SHARED_LIBRARIES += liblog
#LOCAL_SHARED_LIBRARIES += libsec_mem

LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/trustzone/trustonic/source/external/mobicore/common/$(TRUSTONIC_TEE_VERSION)/ClientLib/include \
    $(MTK_PATH_SOURCE)/trustzone/trustonic/internal/trustlets/rpmb/common/Tlrpmb/Locals/Code/public

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_STATIC_LIBRARY)


# =============================================================================
# binary tlcrpmb
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE	:= tlcrpmb

# Add your source files here
LOCAL_SRC_FILES	+= main.cpp

LOCAL_STATIC_LIBRARIES := tlcrpmbLib
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libMcClient
LOCAL_SHARED_LIBRARIES += liblog
#LOCAL_SHARED_LIBRARIES += libsec_mem

LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/trustzone/trustonic/source/external/mobicore/common/$(TRUSTONIC_TEE_VERSION)/ClientLib/include \
    $(MTK_PATH_SOURCE)/trustzone/trustonic/internal/trustlets/rpmb/common/Tlrpmb/Locals/Code/public

LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_EXECUTABLE)

# =============================================================================

# adding the root folder to the search path appears to make absolute paths
# work for import-module - lets see how long this works and what surprises
# future developers get from this.
$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
