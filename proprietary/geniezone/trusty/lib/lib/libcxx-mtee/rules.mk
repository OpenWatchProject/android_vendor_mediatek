LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += $(LOCAL_DIR)/include

MODULE_STATIC_LIB := true

MODULE_SRCS := \
	$(LOCAL_DIR)/bind.cpp \
	$(LOCAL_DIR)/new.cpp \
	$(LOCAL_DIR)/optional.cpp \
	$(LOCAL_DIR)/utility.cpp \
	$(LOCAL_DIR)/memory.cpp \


MODULE_DEPS := \
	lib/libc \
	lib/libm-mtee \
	
	
include make/module.mk

