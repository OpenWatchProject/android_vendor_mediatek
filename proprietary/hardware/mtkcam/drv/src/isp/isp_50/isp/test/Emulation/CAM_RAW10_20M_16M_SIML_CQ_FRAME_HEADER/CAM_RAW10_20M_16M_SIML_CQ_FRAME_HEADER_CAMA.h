
  ptr->writeReg(0x0198, 0x56c11800, CAM_A);             //CAM.CAM_A.CQ.THR0_BASEADDR
  ptr->writeReg(0x019c, 0x4f8, CAM_A);                  //CAM.CAM_A.CQ.THR0_DESC_SIZE
  ptr->writeReg(0x0194, 0x11, CAM_A);                   //CAM.CAM_A.CQ.THR0_CTL
  ptr->writeReg(0x118c, 0x10000008, CAM_A);             //CAM.CAM_A.DMA.BPCI_CON
  ptr->writeReg(0x11ec, 0x4000001d, CAM_A);             //CAM.CAM_A.DMA.LSCI_CON
  ptr->writeReg(0x0190, 0x100, CAM_A);                  //CAM.CAM_A.CQ.EN
  ptr->writeReg(0x1018, 0x69000000, CAM_A);             //CAM.CAM_A.DMA.SPECIAL_FUN_EN
