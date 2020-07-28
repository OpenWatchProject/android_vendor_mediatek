ifeq (,$(filter $(strip $(MTK_PQ_SUPPORT)), no PQ_OFF))

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    main_mtk_pq_hal.cpp

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libhardware \
    vendor.mediatek.hardware.pq@2.3

LOCAL_CPPFLAGS += -fexceptions
LOCAL_MODULE:= vendor.mediatek.hardware.pq@2.2-service
LOCAL_INIT_RC := vendor.mediatek.hardware.pq@2.2-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw

include $(MTK_EXECUTABLE)

endif
