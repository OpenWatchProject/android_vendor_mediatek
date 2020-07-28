LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_STATIC_LIB := true

MODULE_SRCS := \
	$(LOCAL_DIR)/new.cpp \

include make/module.mk
