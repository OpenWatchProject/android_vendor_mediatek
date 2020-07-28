ifeq ($(TARGET_BUILD_VARIANT),eng)

ifneq ($(LOCAL_MAKEFILE),)
  TUI_LOCAL_DIR := $(dir $(LOCAL_MAKEFILE))
  OUTPUT_NAME := drTuiTest
  include $(TUI_LOCAL_DIR)/driverTuiTest.mk
else
  include $(CURDIR)/Locals/Code/makefile_test.mk
endif

endif
