
ptr->writeReg(0x0198, 0x56c11cf8, CAM_B);       //CAM.CAM_B.CQ.THR0_BASEADDR
ptr->writeReg(0x019c, 0x940, CAM_B);            //CAM.CAM_B.CQ.THR0_DESC_SIZE
ptr->writeReg(0x0194, 0x11, CAM_B);             //CAM.CAM_B.CQ.THR0_CTL
ptr->writeReg(0x118c, 0x8000001a, CAM_B);       //CAM.CAM_B.DMA.BPCI_CON
ptr->writeReg(0x11ec, 0x80000020, CAM_B);       //CAM.CAM_B.DMA.LSCI_CON
ptr->writeReg(0x0190, 0x113, CAM_B);            //CAM.CAM_B.CQ.EN
ptr->writeReg(0x1018, 0xe9000000, CAM_B);       //CAM.CAM_B.DMA.SPECIAL_FUN_EN
