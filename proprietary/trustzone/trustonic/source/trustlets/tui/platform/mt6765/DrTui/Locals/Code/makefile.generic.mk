ifeq ($(DEVICE),GENERIC)
	DEVICE_PATH=generic/board
	PLATFORM = ARM_V7A_STD
	DRIVER_KEYFILE := Locals/Build/pairVendorTltSig.pem
INCLUDE_DIRS += \
$(COMP_PATH_drsec_plat)/public
endif
