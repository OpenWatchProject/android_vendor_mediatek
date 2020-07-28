ptr->writeReg(0x0198, 0x0, CAM_C);                  //CAM.CAM_C.CQ.THR0_BASEADDR
ptr->writeReg(0x019c, 0x0, CAM_C);                  //CAM.CAM_C.CQ.THR0_DESC_SIZE
ptr->writeReg(0x0194, 0x10, CAM_C);                 //CAM.CAM_C.CQ.THR0_CTL
ptr->writeReg(0x118c, 0x80000020, CAM_C);           //CAM.CAM_C.DMA.BPCI_CON
ptr->writeReg(0x11ec, 0x80000020, CAM_C);           //CAM.CAM_C.DMA.LSCI_CON
ptr->writeReg(0x0190, 0x10, CAM_C);                 //CAM.CAM_C.CQ.EN
ptr->writeReg(0x1018, 0x0, CAM_C);                  //CAM.CAM_C.DMA.SPECIAL_FUN_EN
