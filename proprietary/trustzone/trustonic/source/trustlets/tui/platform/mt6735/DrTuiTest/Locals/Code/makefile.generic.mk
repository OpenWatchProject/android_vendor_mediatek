ifeq ($(DEVICE),GENERIC)
	DEVICE_PATH=generic/board
	PLATFORM = ARM_V7A_STD
	DRIVER_KEYFILE := Locals/Build/pairVendorTltSig.pem
endif
