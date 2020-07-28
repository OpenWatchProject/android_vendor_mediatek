# Some things are specific to Android 9.0 and later
TRUSTONIC_ANDROID_8 =
ifeq ($(PLATFORM_VERSION), 8.0.0)
TRUSTONIC_ANDROID_8 = yes
endif
ifeq ($(PLATFORM_VERSION), 8.1.0)
TRUSTONIC_ANDROID_8 = yes
endif

# Some TUI things are specific to QC BSP (use QCPATH variable as indicator)
ifneq ($(QCPATH),)
# This is a QC BSP
QC_SOURCE_TREE = yes
else
QC_SOURCE_TREE =
endif

include $(call all-subdir-makefiles)
