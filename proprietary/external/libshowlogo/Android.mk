####################################################################
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


LOCAL_SRC_FILES  := show_logo_common.c\
                  decompress_common.c\
                  show_animation_common.c\
                  charging_animation.cpp

ifeq ($(MSSI_MTK_PUMP_EXPRESS_SUPPORT), yes)
LOCAL_CFLAGS += -DMSSI_MTK_PUMP_EXPRESS_SUPPORT
endif
ifeq ($(MSSI_MTK_PUMP_EXPRESS_PLUS_SUPPORT), yes)
LOCAL_CFLAGS += -DMSSI_MTK_PUMP_EXPRESS_PLUS_SUPPORT
endif


ifdef MSSI_MTK_CARRIEREXPRESS_PACK
ifneq ($(MSSI_MTK_CARRIEREXPRESS_PACK), no)
	LOCAL_CFLAGS += -DMSSI_MTK_CARRIEREXPRESS_PACK
	LOCAL_CPPFLAGS += -DMSSI_MTK_CARRIEREXPRESS_PACK

ifeq ($(filter OP01, $(subst _, $(space), $(MTK_REGIONAL_OP_PACK))), OP01)
	LOCAL_CFLAGS += -DMTK_CARRIEREXPRESS_PACK_OP01
endif

ifeq ($(filter OP02, $(subst _, $(space), $(MTK_REGIONAL_OP_PACK))), OP02)
	LOCAL_CFLAGS += -DMTK_CARRIEREXPRESS_PACK_OP02
endif

ifeq ($(filter OP09, $(subst _, $(space), $(MTK_REGIONAL_OP_PACK))), OP09)
	LOCAL_CFLAGS += -DMTK_CARRIEREXPRESS_PACK_OP09
endif

ifneq ($(filter NONE, $(subst _, $(space), $(OPTR_SPEC_SEG_DEF))), NONE)
ifeq ($(filter OP01, $(subst _, $(space), $(OPTR_SPEC_SEG_DEF))), OP01)
	LOCAL_CFLAGS += -DGLOBAL_DEVICE_DEFAULT_OPTR=1
endif
ifeq ($(filter OP02, $(subst _, $(space), $(OPTR_SPEC_SEG_DEF))), OP02)
	LOCAL_CFLAGS += -DGLOBAL_DEVICE_DEFAULT_OPTR=2
endif
ifeq ($(filter OP09, $(subst _, $(space), $(OPTR_SPEC_SEG_DEF))), OP09)
	LOCAL_CFLAGS += -DGLOBAL_DEVICE_DEFAULT_OPTR=9
endif
endif

endif
endif

LOCAL_CFLAGS += -DMTK_LCM_PHYSICAL_ROTATION=\"$(MTK_LCM_PHYSICAL_ROTATION)\"

LOCAL_SHARED_LIBRARIES := libcutils libutils libc libstdc++ libz libdl liblog libgui libui libbase

LOCAL_STATIC_LIBRARIES += libfs_mgr

LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/lk/include/target
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(TOP)/external/zlib/
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/libs/nativewindow/include
LOCAL_C_INCLUDES += system/core/fs_mgr/include
LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/external/libsysenv

LOCAL_MODULE := libshowlogo
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32

LOCAL_PRELINK_MODULE := false

include $(MTK_SHARED_LIBRARY)
