LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	fgauge_main.cpp \
	fg_log.cpp

ifeq ($(MTK_GAUGE_VERSION), 30)
LOCAL_CFLAGS += -DMTK_GM_30
endif

LOCAL_SHARED_LIBRARIES := libcutils libutils libdl liblog

LOCAL_MODULE:= fuelgauged
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#bobule workaround pdk build error, needing review
LOCAL_MULTILIB := 32
LOCAL_INIT_RC := fuelgauged_init.rc
include $(MTK_EXECUTABLE)
