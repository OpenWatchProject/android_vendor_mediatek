#
# libthermalalgo
#


LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libthermalalgo
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES_arm := libthermalalgo.so

#bobule workaround pdk build error, needing review
LOCAL_MULTILIB := 32
LOCAL_MODULE_SUFFIX := .so

include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_SRC_FILES:= ta_main.cpp
LOCAL_CFLAGS += $(MTK_CDEFS)
LOCAL_SHARED_LIBRARIES := liblog libutils libdl
LOCAL_MODULE:= thermalloadalgod
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#bobule workaround pdk build error, needing review
LOCAL_MULTILIB := 32
LOCAL_PRELINK_MODULE := false
LOCAL_INIT_RC := init.thermalloadalgod.rc
include $(MTK_EXECUTABLE)
