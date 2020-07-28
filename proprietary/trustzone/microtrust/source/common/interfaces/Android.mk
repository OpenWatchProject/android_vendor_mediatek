# update interfaces before compile, this should exec before include interfaces/*/Android.bp
$(info $(shell vendor/mediatek/proprietary/trustzone/microtrust/source/common/interfaces/update.sh))

LOCAL_PATH := $(call my-dir)
# LOCAL_CFLAGS += -DMICROTRUST_TEE_DEBUG_BUILD
include $(shell find -L $(LOCAL_PATH)/*/ -type f -name Android.mk)
