
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	GpuAuxAPI.cpp \
	GuiExtAux.cpp \
	mtk_queue.cpp \
	mtk_gralloc.cpp \
	mtk_gralloc0.cpp \
	mtk_gralloc1.cpp \
	utils.cpp \
	lib/aux_gralloc.cpp \
	lib/aux_gralloc2.cpp \
	lib/aux_gralloc3.cpp \
	lib/aux_mapper.cpp \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(TOP)/$(MTK_ROOT)/hardware/dpframework/inc \
	$(TOP)/$(MTK_ROOT)/hardware/include \
	$(TOP)/system/core/libsync/include \

LOCAL_SHARED_LIBRARIES := \
	libdpframework \
	liblog \
	libutils \
	libcutils \
	libhardware \
	libnativewindow \
	libgralloc_extra \
	libsync \
	libgpud \
	libhidlbase \
	android.hardware.graphics.common@1.0 \
	android.hardware.graphics.common@1.1 \
	android.hardware.graphics.common@1.2 \
	android.hardware.graphics.mapper@2.0 \
	android.hardware.graphics.mapper@2.1 \
	android.hardware.graphics.mapper@3.0 \
	vendor.mediatek.hardware.mms@1.5 \

LOCAL_HEADER_LIBRARIES := libui_headers

LOCAL_CFLAGS += -DLOG_TAG=\"GPUAUX\"
LOCAL_CFLAGS += -Wno-unused-function
LOCAL_CFLAGS += -DMTK_GPUD_SUPPORT
LOCAL_CFLAGS += -DGPUD_GRALLOC_VERSION_MAJOR=1

LOCAL_EXPORT_C_INCLUDE_DIRS := \
	$(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES += \
	libgrallocusage \
	libarect \

ifeq ($(strip $(MTK_PQ_VIDEO_WHITELIST_SUPPORT)), yes)
	LOCAL_CFLAGS += -DMTK_PQ_VIDEO_WHITELIST_SUPPORT
endif

ifeq ($(strip $(MTK_VIDEO_TRANSITION)), yes)
	LOCAL_CFLAGS += -DMTK_VIDEO_TRANSITION
endif

ifeq ($(strip $(MTK_GAMEPQ_SUPPORT)), yes)
	LOCAL_CFLAGS += -DMTK_GAMEPQ_SUPPORT
endif

LOCAL_MODULE := libgpu_aux
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

include $(MTK_SHARED_LIBRARY)
