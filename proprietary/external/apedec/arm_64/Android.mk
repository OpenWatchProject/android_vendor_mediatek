LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/apedec_all.c \
    src/ape_decoder_swip.c
    #src/apedec_all.c \
    #src/gnu_asm/ARMv6_aligned_dot_and_add_64.s\
    #src/gnu_asm/ARMv6_aligned_dot_and_sub_64.s\
    #src/gnu_asm/ARMv6_unaligned_dot_and_add_64.s\
    #src/gnu_asm/ARMv6_unaligned_dot_and_sub_64.s\
    #src/gnu_asm/ARMv6_dot_product.S\
    #src/gnu_asm/ARMv6_predictor.s

LOCAL_C_INCLUDES := $(MTK_PATH_SOURCE)/external/apedec/arm_64/inc
                     
LOCAL_CFLAGS += -DARM -D__arm__ -D__ANDROID__ -D'__inline=inline' -D'__int64=long long' -D'STATIC_DECLARE=static' -DSTATIC_ENHANCE -D'EXTERN=static'

ifeq ($(symbol), scramble)
LOCAL_CFLAGS +=-D__SCRAMBLE_SYMBOL__
endif

LOCAL_MODULE_PATH_32 := $(LOCAL_PATH)/out32/
LOCAL_MODULE_PATH_64 := $(LOCAL_PATH)/out64/

#LOCAL_MODULE_TAGS:=user
LOCAL_MODULE := libapedec_mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_PRELINK_MODULE:=false
LOCAL_ARM_MODE:=arm
LOCAL_SHARED_LIBRARIES:=\
	libnativehelper \
	libcutils \
	libutils
LOCAL_MULTILIB := 64
include $(MTK_STATIC_LIBRARY)
