LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

MTK_PATH_GLOBAL_HDR_PLATFORM := $(LOCAL_PATH)/../$(TARGET_BOARD_PLATFORM)
LOCAL_PATH_HDR_PLATFORM := ../$(TARGET_BOARD_PLATFORM)
LOCAL_PATH_HDR_MODULE_BOOST := ../$(MTK_HDR_MODULE_BOOST)

$(info MTK HDR platform path is $(MTK_PATH_GLOBAL_HDR_PLATFORM))

ifeq (,$(wildcard $(MTK_PATH_GLOBAL_HDR_PLATFORM)))
# Skeleton for the platform dependent implementation.
# They work as *stubs* for yet-to-be-developed code,
# and are most useful in porting stage of a new platform.
$(info use default parts)
LOCAL_PATH_HDR_PLATFORM := default
LOCAL_PATH_HDR_MODULE_BOOST += default
else
$(info use real parts)
LOCAL_PATH_HDR_MODULE_BOOST += $(MTK_HDR_MODULE_BOOST_VERSION)
endif

LOCAL_PATH_HDR_MODULE_BOOST := $(subst $(space),/,$(LOCAL_PATH_HDR_MODULE_BOOST))

$(info HDR boost module path is $(LOCAL_PATH_HDR_MODULE_BOOST))
$(info HDR boost version is $(MTK_HDR_MODULE_BOOST_VERSION))

ifeq ($(MTK_CAM_HDR_SUPPORT), yes)

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), kiboplus mt6757 mt6797 mt6799 mt6759 mt6763 mt6771 mt6775 mt6765))

-include $(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

LOCAL_MODULE := libhdrproc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

LOCAL_SRC_FILES := \
	HDRProc2.cpp \
	HDRAlgo.cpp \
	HDRUtils.cpp \
	HDRFactory.cpp \
	hal/HDRHAL.cpp \
	vendor_algo/VendorHDR.cpp \
	$(LOCAL_PATH_HDR_PLATFORM)/Platform.cpp \
	$(LOCAL_PATH_HDR_MODULE_BOOST)/Boost.cpp

LOCAL_C_INCLUDES := \
	system/media/camera/include \
	$(MTK_PATH_SOURCE)/hardware/mtkcam/include \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
	$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning \
	$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libhdr_lib/include
# HDR Facedetection
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include
# custom folder
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc  # CUSTOM (common part)
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/$(MTK_CAM_SW_VERSION)  # CUSTOM (common part)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc # CUSTOM (platform)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa # workaround aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION) # CUSTOM (platform)
#For AEPlineTable.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/custom/aaa
#For BuildInTypes.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/def
ifneq (,$(wildcard $(MTK_PATH_GLOBAL_HDR_PLATFORM)))
LOCAL_C_INCLUDES += \
	$(MTK_PATH_GLOBAL_HDR_PLATFORM)
endif
#external
LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/external

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libutils \
	libcutils \
	libcam.feature_utils \
	libmtkcam_stdutils \
	libcam.iopipe \
	libcameracustom \
	libmtkcam_modulehelper \
	libcamalgo.hdr \
	libmtkcam_hwutils \
    libhardware \
	libcamalgo.fdft

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DLOG_TAG=\"hdrproc\"
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DUSE_SYSTRACE

# workaroud to judge whether libperfservicenative exists nor not
# remove this condition after MTK_PERFSERVICE_SUPPORT works as expected
ifneq ($(MTK_BASIC_PACKAGE), yes)
LOCAL_CFLAGS += -DUSE_AFFINITY
LOCAL_WHOLE_STATIC_LIBRARIES += libsched
endif

ifeq ($(TARGET_BOARD_PLATFORM), mt6771)
# dng
#dngop
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libdngop_lib/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libawb_core_lib/$(TARGET_BOARD_PLATFORM)/include
# vpu
LOCAL_C_INCLUDES += $(TOP)/kernel-4.4/drivers/misc/mediatek/vpu/2.0/
# log.h
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/vpu/include
# ion/ion.h : standard ion api
#LOCAL_C_INCLUDES += $(TOP)/system/core/libion/include

# libion_mtk/include/ion.h : path for mtk ion api
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external

# linux/ion.h : ioctl define for ion
#LOCAL_C_INCLUDES += $(TOP)/system/core/libion/kernel-headers
LOCAL_C_INCLUDES += $(TOP)/system/core/include
LOCAL_C_INCLUDES += $(TOP)/kernel/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/kernel/include

# linux/ion_drv.h :define for ion_mm_data_t
#LOCAL_C_INCLUDES += $(TOP)/device/mediatek/common/kernel-headers


LOCAL_SHARED_LIBRARIES += libvpu
LOCAL_SHARED_LIBRARIES += libc++
LOCAL_SHARED_LIBRARIES += libion
LOCAL_SHARED_LIBRARIES += libion_mtk
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core

#dngop
LOCAL_SHARED_LIBRARIES += libcamalgo.dngop

#LOCAL_CFLAGS += -DFAKEHDR_ENABLE

endif #TARGET_BOARD_PLATFORM

LOCAL_SHARED_LIBRARIES += libfeature.face
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils

include $(MTK_SHARED_LIBRARY)

endif # TARGET_BOARD_PLATFORM

endif # MTK_CAM_HDR_SUPPORT
