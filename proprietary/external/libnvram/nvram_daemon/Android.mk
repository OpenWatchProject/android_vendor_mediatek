LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm


ifeq ($(strip $(MTK_INTERNAL_LOG_ENABLE)),yes)
    LOCAL_CFLAGS += -DMTK_INTERNAL_LOG_ENABLE
endif

LOCAL_SHARED_LIBRARIES:= libc libcutils libnvram libcustom_nvram libfile_op libnvram_daemon_callback liblog

ifneq ($(strip $(MTK_BASIC_PACKAGE)),yes)
  LOCAL_SHARED_LIBRARIES+= libhwm
endif

LOCAL_SRC_FILES:= \
    nvram_daemon.c\
    nvram_battery.c\
    nvram_wifi.c \
    nvram_bt.c \
    nvram_productinfo.c \

ifneq ($(strip $(MTK_BASIC_PACKAGE)),yes)
    LOCAL_SRC_FILES+= \
        nvram_acc.c \
        nvram_gyro.c \
        nvram_ps.c
else
    LOCAL_CFLAGS += -DMTK_BASIC_PACKAGE
endif

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/nvram/libfile_op \
    $(MTK_PATH_SOURCE)/external/sensor-tools \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen
LOCAL_MODULE:=nvram_daemon
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32
LOCAL_INIT_RC := nvram_daemon.rc
LOCAL_MODULE_TAGS := optional
include $(MTK_EXECUTABLE)


