LOCAL_DIR := $(GET_LOCAL_DIR)

LIB_MODULE := $(LOCAL_DIR)/user-lib-prebuilt

$(eval XBIN_NAME := $(notdir $(LIB_MODULE)))\
$(eval XBIN_TOP_MODULE := $(LIB_MODULE))\
$(eval XBIN_TYPE := USER_TASK)\
$(eval XBIN_ARCH := $(TRUSTY_USER_ARCH))\
$(eval XBIN_BUILDDIR := $(BUILDDIR)/prebuilts/user-lib-prebuilt)\
$(eval XBIN_LINKER_SCRIPT := lib/lib/trusty/arch/arm/user_task-trusty.ld)\
$(eval XBIN_LDFLAGS := )\
$(eval include $(LOCAL_DIR)/xbin.mk)

