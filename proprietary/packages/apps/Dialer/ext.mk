# Mediatek add ext res and libs.
EXT_RES_DIRS += \
    ext/incallui/res \
    ext/dialer/res  \
    java/com/mediatek/incallui/dsda/res \
    java/com/mediatek/incallui/blindect/res \

# M: Vilte project not support multi-window @{
$(info Vilte $(MSSI_MTK_VILTE_SUPPORT))
ifeq (yes, $(filter yes, $(strip $(MSSI_MTK_VILTE_SUPPORT))))
EXT_RES_DIRS += ext/incallui/vt_config/disable
$(info disable multi-window for InCallUi $(res_dirs))
else
EXT_RES_DIRS += ext/incallui/vt_config/enable
$(info enabl multi-window for InCallUi $(res_dirs))
endif
# @}

# M: [InCallUI]needed by AddMemberEditView who extends MTKRecipientEditTextView
# M: [InCallUI]FIXME: should replace this with google default RecipientEditTextView
LOCAL_RESOURCE_DIR += \
    $(addprefix $(LOCAL_PATH)/, $(EXT_RES_DIRS)) \
    vendor/mediatek/proprietary/frameworks/ex/chips/res \

EXT_SRC_DIRS := \
    java/com/mediatek
LOCAL_SRC_FILES += $(call all-java-files-under, $(EXT_SRC_DIRS))

LOCAL_JAVA_LIBRARIES += mediatek-telecom-common \
                        mediatek-telephony-base \
                        mediatek-common \
                        ims-common \

LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.incallui.ext.vendor \
                               com.mediatek.dialer.ext.vendor \
                               wfo-common \
                               android-common-chips \
                               com.mediatek.tatf.common \
                               vendor.mediatek.hardware.pplagent-V1.0-java

EXT_DIALER_MANIFEST_FILES += \
    java/com/mediatek/incallui/wfc/AndroidManifest.xml \
    java/com/mediatek/incallui/volte/AndroidManifest.xml \
    java/com/mediatek/incallui/dsda/AndroidManifest.xml \
    java/com/mediatek/incallui/blindect/AndroidManifest.xml \
    java/com/mediatek/incallui/tatf/AndroidManifest.xml


LOCAL_FULL_LIBS_MANIFEST_FILES += \
    $(addprefix $(LOCAL_PATH)/, $(EXT_DIALER_MANIFEST_FILES))

LOCAL_AAPT_FLAGS += \
    --extra-packages com.mediatek.incallui.blindect \
    --extra-packages com.android.mtkex.chips
