LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:=audiopolicy_parameters_includes

LOCAL_EXPORT_C_INCLUDE_DIRS:=$(LOCAL_PATH)/cfgfileinc

include $(BUILD_HEADER_LIBRARY)

