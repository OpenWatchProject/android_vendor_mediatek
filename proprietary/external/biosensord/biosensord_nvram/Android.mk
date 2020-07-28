LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= biosensor_nvram.c

LOCAL_CFLAGS += $(MTK_CDFS)

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/custom/common/cgen/inc \
    $(MTK_PATH_CUSTOM)/cgen/inc \
    $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
    $(MTK_PATH_CUSTOM)/cgen/cfgdefault \
    $(MTK_PATH_SOURCE)/external/biosensord/libbiosensor

LOCAL_SHARED_LIBRARIES := libcutils libutils libdl libbiosensor libnvram

LOCAL_MODULE:= biosensord_nvram
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#bobule workaround pdk build error, needing review
#LOCAL_MULTILIB := 32
LOCAL_PRELINK_MODULE := false
include $(MTK_EXECUTABLE)

