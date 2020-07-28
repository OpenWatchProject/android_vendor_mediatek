#only support mt6628 loopback test currently.
LOCAL_PATH:= $(call my-dir)

ENABLE_BUILD = no
ifeq ($(MTK_WLAN_SUPPORT),yes)
  ifeq ($(MTK_WLAN_CHIP),MT6620)
    #ENABLE_BUILD = yes
    EXTRA_CFLAGS = -DMT6620
  endif
  ifeq ($(MTK_WLAN_CHIP),MT5931)
    #ENABLE_BUILD = yes
    EXTRA_CFLAGS = -DMT5931
  endif
  ifeq ($(MTK_WLAN_CHIP),MT6628)
    ENABLE_BUILD = yes
    EXTRA_CFLAGS = -DMT6628
  endif
endif

ifeq ($(ENABLE_BUILD), yes)
  include $(CLEAR_VARS)
  LOCAL_SHARED_LIBRARIES := libcutils
  LOCAL_STATIC_LIBRARIES := libiw

  LOCAL_SRC_FILES := test_lib.c utility.c wifi_set_power.c
  LOCAL_CFLAGS += -Wall -Werror -D_ANDROID_ $(EXTRA_CFLAGS)
  LOCAL_PRELINK_MODULE := false
  LOCAL_MODULE := libwifiloopback
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
  include $(MTK_SHARED_LIBRARY)

  BUILD_TEST_APP = true
  ifeq ($(BUILD_TEST_APP),true)
    include $(CLEAR_VARS)
    LOCAL_SHARED_LIBRARIES := libcutils libwifiloopback
    LOCAL_STATIC_LIBRARIES := libiw

    LOCAL_SRC_FILES := main.c
    LOCAL_CFLAGS += -Wall -Werror
    LOCAL_MODULE := mtk_wifi_test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
    include $(MTK_EXECUTABLE)
  endif
endif

