bootanimation_CommonCFlags = -DGL_GLEXT_PROTOTYPES -DEGL_EGLEXT_PROTOTYPES
bootanimation_CommonCFlags += -Wall -Werror -Wunused -Wunreachable-code


# bootanimation executable
# =========================================================

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS += ${bootanimation_CommonCFlags}

LOCAL_SHARED_LIBRARIES := \
    libOpenSLES \
    libandroidfw \
    libbase \
    libbinder \
    libmtkbootanimation \
    libcutils \
    liblog \
    libutils \

LOCAL_SRC_FILES:= \
    BootAnimationUtil.cpp \

ifeq ($(PRODUCT_IOT),true)
LOCAL_SRC_FILES += \
    iot/iotbootanimation_main.cpp \
    iot/BootAction.cpp

LOCAL_SHARED_LIBRARIES += \
    libandroidthings \
    libbase \
    libbinder

LOCAL_STATIC_LIBRARIES += cpufeatures

else

LOCAL_SRC_FILES += \
    bootanimation_main.cpp \
    audioplay.cpp \

endif  # PRODUCT_IOT

LOCAL_MODULE:= mtkbootanimation

LOCAL_INIT_RC := mtkbootanim.rc

ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif

ifeq (OP01,$(word 1,$(subst _, ,$(OPTR_SPEC_SEG_DEF))))
    ifeq ($(strip $(MTK_BSP_PACKAGE)), yes)
        include $(BUILD_EXECUTABLE)
    endif
else ifeq (OP02,$(word 1,$(subst _, ,$(OPTR_SPEC_SEG_DEF))))
    ifeq ($(strip $(MTK_BSP_PACKAGE)), yes)
        include $(BUILD_EXECUTABLE)
    endif
else ifeq (OP09,$(word 1,$(subst _, ,$(OPTR_SPEC_SEG_DEF))))
    ifeq ($(strip $(MTK_BSP_PACKAGE)), yes)
        include $(BUILD_EXECUTABLE)
    endif
endif

# libbootanimation
# ===========================================================

include $(CLEAR_VARS)
LOCAL_MODULE := libmtkbootanimation
LOCAL_CFLAGS += ${bootanimation_CommonCFlags}

LOCAL_SRC_FILES:= \
    BootAnimation.cpp


ifdef MSSI_MTK_CARRIEREXPRESS_PACK
    ifneq ($(strip $(MSSI_MTK_CARRIEREXPRESS_PACK)), no)
        LOCAL_CFLAGS += -DMSSI_MTK_CARRIEREXPRESS_PACK
        LOCAL_CPPFLAGS += -DMSSI_MTK_CARRIEREXPRESS_PACK
    endif
endif

LOCAL_CFLAGS += ${bootanimation_CommonCFlags}
LOCAL_C_INCLUDES += \
    external/tinyalsa/include \
    frameworks/wilhelm/include

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libandroidfw \
    libutils \
    libbinder \
    libui \
    libhwui \
    libEGL \
    libETC1 \
    libGLESv2 \
    libmedia \
    libGLESv1_CM \
    libgui \
    libtinyalsa \
    libbase


LOCAL_C_INCLUDES += $(TOP)/$(MTK_ROOT)/frameworks-ext/native/include
LOCAL_C_INCLUDES += external/skia/include
ifdef TARGET_32_BIT_SURFACEFLINGER
LOCAL_32_BIT_ONLY := true
endif

ifeq (OP01,$(word 1,$(subst _, ,$(OPTR_SPEC_SEG_DEF))))
    ifeq ($(strip $(MTK_BSP_PACKAGE)), yes)
        include $(BUILD_SHARED_LIBRARY)
    endif
else ifeq (OP02,$(word 1,$(subst _, ,$(OPTR_SPEC_SEG_DEF))))
    ifeq ($(strip $(MTK_BSP_PACKAGE)), yes)
        include $(BUILD_SHARED_LIBRARY)
    endif
else ifeq (OP09,$(word 1,$(subst _, ,$(OPTR_SPEC_SEG_DEF))))
    ifeq ($(strip $(MTK_BSP_PACKAGE)), yes)
        include $(BUILD_SHARED_LIBRARY)
    endif
endif
