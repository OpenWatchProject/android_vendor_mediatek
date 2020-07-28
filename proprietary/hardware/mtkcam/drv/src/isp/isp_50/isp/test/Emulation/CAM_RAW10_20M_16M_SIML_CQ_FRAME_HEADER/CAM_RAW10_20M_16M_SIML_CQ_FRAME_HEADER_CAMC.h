
ptr->writeReg(0x0198, 0x56c12638, CAM_C);       //CAM.CAM_C.CQ.THR0_BASEADDR
ptr->writeReg(0x019c, 0xd88, CAM_C);            //CAM.CAM_C.CQ.THR0_DESC_SIZE
ptr->writeReg(0x0194, 0x11, CAM_C);             //CAM.CAM_C.CQ.THR0_CTL
ptr->writeReg(0x118c, 0x10000008, CAM_C);       //CAM.CAM_C.DMA.BPCI_CON
ptr->writeReg(0x11ec, 0x4000001d, CAM_C);       //CAM.CAM_C.DMA.LSCI_CON
ptr->writeReg(0x0190, 0x10, CAM_C);             //CAM.CAM_C.CQ.EN
ptr->writeReg(0x1018, 0x69000000, CAM_C);       //CAM.CAM_C.DMA.SPECIAL_FUN_EN
