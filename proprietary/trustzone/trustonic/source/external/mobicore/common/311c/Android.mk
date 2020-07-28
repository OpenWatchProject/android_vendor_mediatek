# Some things are specific to Android 6.0 and later (use stlport absence as indicator)
ifneq ($(wildcard external/stlport/libstlport.mk),)
# Up to Lollipop
TRUSTONIC_ANDROID_LEGACY_SUPPORT = yes
else
# Since Marshmallow
TRUSTONIC_ANDROID_LEGACY_SUPPORT =
endif

include $(call all-subdir-makefiles)
