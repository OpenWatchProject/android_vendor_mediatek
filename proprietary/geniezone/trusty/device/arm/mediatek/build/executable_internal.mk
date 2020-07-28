ifeq ($(strip $(LOCAL_MODULE_CLASS)),)
LOCAL_MODULE_CLASS := EXECUTABLES
endif
ifeq ($(strip $(LOCAL_MODULE_SUFFIX)),)
LOCAL_MODULE_SUFFIX := $(TARGET_EXECUTABLE_SUFFIX)
endif

$(call target-executable-hook)

include $(GZ_BUILD_SYSTEM)/dynamic_binary.mk

$(linked_module): PRIVATE_TARGET_OUT_INTERMEDIATE_LIBRARIES := $($(LOCAL_2ND_ARCH_VAR_PREFIX)TARGET_OUT_INTERMEDIATE_LIBRARIES)

$(linked_module): $(all_objects) $(all_libraries)
	$(gz-transform-o-to-executable)

