ifneq ($(LOCAL_MAKEFILE),)
  TUI_LOCAL_DIR := $(dir $(LOCAL_MAKEFILE))
  OUTPUT_NAME := drTui
  EXTRA_LIBS +=$(MPU_API_OUT_DIR)/$(TEE_MODE)/mpu_api.lib
  include $(TUI_LOCAL_DIR)/driverTui.mk
else
  EXTRA_LIBS +=$(MPU_API_OUT_DIR)/$(TEE_MODE)/mpu_api.lib
  include $(CURDIR)/Locals/Code/makefile_plat.mk
endif
