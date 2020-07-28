ptr->writeReg(0x0208, 0x508e3a20, CAM_B);       //CAM.CAM_B.CAMCQ_R1.CAMCQ_CQ_THR0_BASEADDR
ptr->writeReg(0x020c, 0x620, CAM_B);            //CAM.CAM_B.CAMCQ_R1.CAMCQ_CQ_THR0_DESC_SIZE
ptr->writeReg(0x0204, 0x11, CAM_B);             //CAM.CAM_B.CAMCQ_R1.CAMCQ_CQ_THR0_CTL
ptr->writeReg(0x41e8, 0x80000040, CAM_B);       //CAM.CAM_B.BPCI_R1.BPCI_CON
ptr->writeReg(0x42a8, 0x80000040, CAM_B);       //CAM.CAM_B.LSCI_R1.LSCI_CON
ptr->writeReg(0x0200, 0x10, CAM_B);             //CAM.CAM_B.CAMCQ_R1.CAMCQ_CQ_EN
ptr->writeReg(0x4018, 0x8107e000, CAM_B);       //CAM.CAM_B.CAMDMATOP1_R1.CAMDMATOP1_SPECIAL_FUN_EN1

//camsys_start.c
ptr->writeReg(0x1bc4, 0x1, CAM_B);              //CAM.CAM_B.GGM_R1.GGM_SRAM_PINGPONG
ptr->writeReg(0x349c, 0x1, CAM_B);              //CAM.CAM_B.YNRS_R1.YNRS_SRAM_PINGPONG
ptr->writeReg(0x0038, 0x0, CAM_B);              //CAM.CAM_B.CAMCTL_R1.CAMCTL_RAWI_TRIG
