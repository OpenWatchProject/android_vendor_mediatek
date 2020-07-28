LOCAL_PATH:= $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE:= libfod_sample_headers
LOCAL_EXPORT_C_INCLUDE_DIRS += \
    $(GZ_ROOT_DIR)/trusty/lib/interface/mtee/include/common/include/ \
    $(GZ_ROOT_DIR)/trusty/lib/lib/mtee_serv/ \
    $(GZ_ROOT_DIR)/trusty/lib/lib/mtee_api/include
include $(BUILD_HEADER_LIBRARY)

##############################################################################
###############################################################################

include $(CLEAR_VARS)

LOCAL_MODULE    := fod_sample.elf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(GZ_APP_OUT)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES += fod_sample_main.c \
                   manifest.c \
                   matrix.c

LOCAL_MULTILIB := 32

### Add include path here
LOCAL_HEADER_LIBRARIES += libfod_sample_headers

LOCAL_CFLAGS := -DCFG_GZ_SECURE_DSP_WITH_M4U=1

ifneq ("$(wildcard $(LOCAL_PATH)/$(MTK_PLATFORM_DIR)_vpu/vpu_main_sDSP_0)","")
$(shell /usr/bin/xxd -i $(LOCAL_PATH)/$(MTK_PLATFORM_DIR)_vpu/vpu_main_sDSP_0 > $(LOCAL_PATH)/vpu_main_sDSP_0.c)
$(shell sed -e 's/vendor_mediatek_proprietary_geniezone_trusty_app_sample_fod_sample_$(MTK_PLATFORM_DIR)_vpu_//' -i $(LOCAL_PATH)/vpu_main_sDSP_0.c)
LOCAL_CFLAGS += -DSUPPORT_sDSP_0
endif

ifneq ("$(wildcard $(LOCAL_PATH)/$(MTK_PLATFORM_DIR)_vpu/vpu_main_sDSP_1)","")
$(shell /usr/bin/xxd -i $(LOCAL_PATH)/$(MTK_PLATFORM_DIR)_vpu/vpu_main_sDSP_1 > $(LOCAL_PATH)/vpu_main_sDSP_1.c)
$(shell sed -e 's/vendor_mediatek_proprietary_geniezone_trusty_app_sample_fod_sample_$(MTK_PLATFORM_DIR)_vpu_//' -i $(LOCAL_PATH)/vpu_main_sDSP_1.c)
LOCAL_CFLAGS += -DSUPPORT_sDSP_1
endif


LOCAL_STATIC_LIBRARIES := libc-trusty \
                          libc.mod \
                          libmtee_serv \
                          libmtee_api \

include $(GZ_EXECUTABLE)

