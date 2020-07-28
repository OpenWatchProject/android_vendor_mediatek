ptr->writeReg(0x0208, 0x508e3400, CAM_A);       //CAM.CAM_A.CAMCQ_R1.CAMCQ_CQ_THR0_BASEADDR
ptr->writeReg(0x020c, 0x620, CAM_A);            //CAM.CAM_A.CAMCQ_R1.CAMCQ_CQ_THR0_DESC_SIZE
ptr->writeReg(0x0204, 0x11, CAM_A);             //CAM.CAM_A.CAMCQ_R1.CAMCQ_CQ_THR0_CTL
ptr->writeReg(0x41e8, 0x80000040, CAM_A);       //CAM.CAM_A.BPCI_R1.BPCI_CON
ptr->writeReg(0x42a8, 0x80000040, CAM_A);       //CAM.CAM_A.LSCI_R1.LSCI_CON
ptr->writeReg(0x0200, 0x111, CAM_A);            //CAM.CAM_A.CAMCQ_R1.CAMCQ_CQ_EN
ptr->writeReg(0x4018, 0x80000000, CAM_A);       //CAM.CAM_A.CAMDMATOP1_R1.CAMDMATOP1_SPECIAL_FUN_EN1


//camsys_start.c
ptr->writeReg(0x1bc4, 0x1, CAM_A);              //CAM.CAM_A.GGM_R1.GGM_SRAM_PINGPONG
ptr->writeReg(0x349c, 0x1, CAM_A);              //CAM.CAM_A.YNRS_R1.YNRS_SRAM_PINGPONG
//ptr->writeReg(0x3b04, 0x1001, CAM_A);         //CAM.CAM_A.TG_R1.TG_VF_CON
ptr->writeReg(0x3b04, 0x1000, CAM_A);           //CAM.CAM_A.TG_R1.TG_VF_CON (VF_DATA_OFF first)
ptr->writeReg(0x0038, 0x0, CAM_A);              //CAM.CAM_A.CAMCTL_R1.CAMCTL_RAWI_TRIG
