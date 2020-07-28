	AKM_Open()
	readGYRO_from_reg();
	AKM_SetGyroData();
	readMAG_from_reg();
	AKM_SetMagData();
	AKM_Calibrate();