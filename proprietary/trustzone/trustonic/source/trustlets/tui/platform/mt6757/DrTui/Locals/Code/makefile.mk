ifneq ($(LOCAL_MAKEFILE),)
  TUI_LOCAL_DIR := $(dir $(LOCAL_MAKEFILE))
  OUTPUT_NAME := drTui
  include $(TUI_LOCAL_DIR)/driverTui.mk
else
  include $(CURDIR)/Locals/Code/makefile_plat.mk
endif
