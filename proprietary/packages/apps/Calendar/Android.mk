ifneq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# Include res dir from chips
#replace with mtk-ex
#chips_dir := ../../../frameworks/opt/chips/res
chips_dir := ../../../frameworks/ex/chips/res
color_picker_dir := ../../../../../../frameworks/opt/colorpicker/res
datetimepicker_dir := ../../../frameworks/opt/datetimepicker/res
timezonepicker_dir := ../../../frameworks/opt/timezonepicker/res
res_dirs := $(chips_dir) $(color_picker_dir) $(datetimepicker_dir) $(timezonepicker_dir) res
ifeq (tablet,$(strip $(MTK_PRODUCT_LINE)))
ifeq (yes,$(strip $(MTK_GMO_ROM_OPTIMIZE)))
    res_dirs += res_tb
endif
endif
src_dirs := src

LOCAL_JACK_COVERAGE_INCLUDE_FILTER := com.android.calendar.*

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under,$(src_dirs))

# bundled
#LOCAL_STATIC_JAVA_LIBRARIES += \
#        android-common \
#        libchips \
#        calendar-common

# unbundled
#replace with mtk-ex
LOCAL_STATIC_JAVA_LIBRARIES := \
        android-common \
        colorpicker \
        mediatek-opt-datetimepicker \
        mediatek-opt-timezonepicker \
        android-support-v4 \
        calendar-common \
        com.mediatek.calendar.ext \
        com.mediatek.vcalendar \
        android-common-chips

LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += mediatek-common
#LOCAL_JACK_ENABLED := disabled

#in framework when has it.
LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_RESOURCE_DIR := $(addprefix $(LOCAL_PATH)/, $(res_dirs))
LOCAL_PACKAGE_NAME := MtkCalendar
LOCAL_OVERRIDES_PACKAGES := Calendar

LOCAL_MODULE_OWNER := mtk


#M:change certificate for multi-user permission
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_FLAG_FILES := proguard.flags \
                             ../../../frameworks/opt/datetimepicker/proguard.flags

LOCAL_PRODUCT_MODULE := true

LOCAL_AAPT_FLAGS := --auto-add-overlay
#replace with mtk-ex
#LOCAL_AAPT_FLAGS += --extra-packages com.android.ex.chips
LOCAL_AAPT_FLAGS += --extra-packages com.android.mtkex.chips
LOCAL_AAPT_FLAGS += --extra-packages com.android.colorpicker
LOCAL_AAPT_FLAGS += --extra-packages com.android.datetimepicker
LOCAL_AAPT_FLAGS += --extra-packages com.android.timezonepicker

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
endif