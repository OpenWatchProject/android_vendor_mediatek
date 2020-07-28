LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	fgauge_nvram.cpp \
	fg_log.cpp

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/custom/common/cgen/inc \
    $(MTK_PATH_CUSTOM)/cgen/inc \
    $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
    $(MTK_PATH_CUSTOM)/cgen/cfgdefault

LOCAL_SHARED_LIBRARIES := libcutils libutils libdl libnvram liblog

LOCAL_MODULE:= fuelgauged_nvram
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#bobule workaround pdk build error, needing review
LOCAL_MULTILIB := 32
LOCAL_INIT_RC := fuelgauged_nvram_init.rc
include $(MTK_EXECUTABLE)
