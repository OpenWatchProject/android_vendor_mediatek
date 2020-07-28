
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk


ifneq ($(strip $(FPGA_EARLY_PORTING)),yes) # build only when project is not fpga related

ifeq ($(MTK_CAM_LOMO_SUPPORT), yes)
#-----------------------------------------------------------
LOCAL_SRC_FILES += LomoEffect.cpp
#-----------------------------------------------------------
else
#-----------------------------------------------------------
LOCAL_SRC_FILES += LomoEffectDummy.cpp
#-----------------------------------------------------------
endif # MTK_CAM_LOMO_SUPPORT

LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/system

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external

ifeq ($(IS_LEGACY),0)
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/feature/lomo_hal_jni
else
    LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/include
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include/mtkcam/jni
endif

LOCAL_C_INCLUDES += $(TOP)/
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES :=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
#
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.lomoeffect@1.0
LOCAL_SHARED_LIBRARIES += android.hardware.camera.common@1.0
LOCAL_SHARED_LIBRARIES += android.hardware.camera.device@3.2
#LOCAL_SHARED_LIBRARIES += android.hidl.base@1.0

LOCAL_SHARED_LIBRARIES += android.hidl.memory@1.0
LOCAL_SHARED_LIBRARIES += libhidlmemory
LOCAL_SHARED_LIBRARIES += libion
LOCAL_SHARED_LIBRARIES += libion_mtk

# for setprop/getprop
LOCAL_SHARED_LIBRARIES += libcutils

ifeq ($(MTK_CAM_LOMO_SUPPORT),yes)
#LOCAL_SHARED_LIBRARIES += libcam.drv.sharelomohaljni

# no need to static link this lib because of dlopen
#LOCAL_SHARED_LIBRARIES  +=  libcam.jni.lomohaljni

endif

#
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LOCAL_MODULE := vendor.mediatek.hardware.camera.lomoeffect@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
ifeq ($(MTK_CAM_HAL_VERSION), 3)
LOCAL_MULTILIB := first
endif

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)

endif # NOT fpga related  project

