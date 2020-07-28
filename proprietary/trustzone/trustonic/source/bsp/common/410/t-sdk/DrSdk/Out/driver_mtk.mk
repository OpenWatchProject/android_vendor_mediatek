# MEDIATEK PROPRIETARY

# BUILD_VARIANT
ifeq ($(TARGET_BUILD_VARIANT),)
$(info - TARGET_BUILD_VARIANT  is  not  set,  default  is : 0)
TARGET_BUILD_VARIANT ?= 0
endif

$(info **************** Mediatek ******************************)
$(info TARGET_BUILD_VARIANT : $(TARGET_BUILD_VARIANT))
$(info ********************************************************)

ifeq ($(TARGET_BUILD_VARIANT),eng)
# Check if it's internal dev
ifneq ($(wildcard $(TOP)/vendor/mediatek/proprietary/trustzone/trustonic/secure/),)
CC_OPTS += -DMEMORY_MONITOR=1
endif
else
CC_OPTS += -DMEMORY_MONITOR=0
endif
