ptr->writeReg(0x0208, 0x54e7ee00, CAM_A);       //CAM.CAM_A.CAMCQ_R1.CAMCQ_CQ_THR0_BASEADDR
ptr->writeReg(0x020c, 0x620, CAM_A);            //CAM.CAM_A.CAMCQ_R1.CAMCQ_CQ_THR0_DESC_SIZE
ptr->writeReg(0x0204, 0x11, CAM_A);             //CAM.CAM_A.CAMCQ_R1.CAMCQ_CQ_THR0_CTL
ptr->writeReg(0x41e8, 0x40000040, CAM_A);       //CAM.CAM_A.BPCI_R1.BPCI_CON
ptr->writeReg(0x42a8, 0x10000040, CAM_A);       //CAM.CAM_A.LSCI_R1.LSCI_CON
ptr->writeReg(0x0200, 0x113, CAM_A);            //CAM.CAM_A.CAMCQ_R1.CAMCQ_CQ_EN
ptr->writeReg(0x4018, 0xc0000000, CAM_A);       //CAM.CAM_A.CAMDMATOP1_R1.CAMDMATOP1_SPECIAL_FUN_EN1                   

//from camsys_start_0.c setting
ptr->writeReg(0x0054, 0x1, CAM_A);  			//CAM.CAM_A.CAMCTL_R1.CAMCTL_START 
ptr->writeReg(0x3b04, 0x1001, CAM_A);  		    //CAM.CAM_A.TG_R1.TG_VF_CON                        
ptr->writeReg(0x0038, 0x0, CAM_A);  			//CAM.CAM_A.CAMCTL_R1.CAMCTL_RAWI_TRIG 