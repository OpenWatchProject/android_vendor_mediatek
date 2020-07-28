#ifndef _EMI_DRV_H_
#define _EMI_DRV_H_

//#define IO_PHYS            	    0x10000000
//#define EMI_BASE                (IO_PHYS + 0x00203000)

/*EMI PSRAM (NOR) and DRAM control registers*/
#define EMI_CONA                 (0x0000)  /* EMI control register for bank 0 */
#define EMI_CONB                 (0x0008)  /* EMI control register for bank 1 */
#define EMI_CONC                 (0x0010)  /* EMI control register for bank 2 */
#define EMI_COND                 (0x0018)  /* EMI control register for bank 3 */
#define EMI_CONE                 (0x0020)  /* EMI control register for bank 0 */
#define EMI_CONF                 (0x0028)  /* EMI control register for bank 1 */
#define EMI_CONG                 (0x0030)  /* EMI control register for bank 0 */
#define EMI_CONH                 (0x0038)  /* EMI control register for bank 1 */
#define EMI_CONI                 (0x0040)  /* EMI control register 0 for Mobile-RAM */
#define EMI_CONJ                 (0x0048)  /* EMI control register 1 for Mobile-RAM */
#define EMI_CONK                 (0x0050)  /* EMI control register 2 for Mobile-RAM */
#define EMI_CONL                 (0x0058)  /* EMI control register 3 for Mobile-RAM */
#define EMI_CONM                 (0x0060)
#define EMI_CONN                 (0x0068)
#define CAL_EN                   (1 << 8)
#define EMI_GENA                 (0x0070)
#define EMI_REMAP                 EMI_GENA
#define EMI_DRCT                 (0x0078)
#define EMI_DDRV                 (0x0080)
#define EMI_GEND                 (0x0088)
#define EMI_PPCT                 (0x0090) /* EMI Performance and Power Control Register */

#define EMI_DLLV                 (0x00A0)

#define EMI_DFTC                 (0x00F0)
#define EMI_DFTD                 (0x00F8)

/* EMI bandwith filter and MPU control registers */
#define EMI_ARBA                 (0x0100)
#define EMI_ARBB                 (0x0108)
#define EMI_ARBC                 (0x0110)
#define EMI_ARBD                 (0x0118)
#define EMI_ARBE                 (0x0120)
#define EMI_ARBF                 (0x0128)
#define EMI_ARBG                 (0x0130)

#define EMI_SLCT                 (0x0150)
#define EMI_ABCT	             (0x0158)

/* EMI Memory Protect Unit */
#define EMI_MPUA                 (0x0160)
#define EMI_MPUB                 (0x0168)
#define EMI_MPUC                 (0x0170)
#define EMI_MPUD                 (0x0178)
#define EMI_MPUE                (0x0180)
#define EMI_MPUF	        (0x0188)
#define EMI_MPUG	        (0x0190)
#define EMI_MPUH	        (0x0198)

#define EMI_MPUI	        (0x01A0)
#define EMI_MPUI_2ND	    (0x01A4)
#define EMI_MPUJ            (0x01A8)
#define EMI_MPUJ_2ND	    (0x01AC)
#define EMI_MPUK            (0x01B0)
#define EMI_MPUK_2ND        (0x01B4)
#define EMI_MPUL            (0x01B8)
#define EMI_MPUL_2ND        (0x01BC)
#define EMI_MPUM            (0x01C0)
#define EMI_MPUN            (0x01C8)
#define EMI_MPUO            (0x01D0)
#define EMI_MPUP            (0x01D8)
#define EMI_MPUQ            (0x01E0)
#define EMI_MPUR            (0x01E8)
#define EMI_MPUS            (0x01F0)
#define EMI_MPUT            (0x01F8)
#define EMI_MPUU            (0x0200)

#define EMI_MPUA2		(0x0260)
#define EMI_MPUB2		(0x0268)
#define EMI_MPUC2		(0x0270)
#define EMI_MPUD2		(0x0278)
#define EMI_MPUE2		(0x0280)
#define EMI_MPUF2		(0x0288)
#define EMI_MPUG2		(0x0290)
#define EMI_MPUH2		(0x0298)
#define EMI_MPUI2		(0x02A0)
#define EMI_MPUI2_2ND	(0x02A4)
#define EMI_MPUJ2		(0x02A8)
#define EMI_MPUJ2_2ND	(0x02AC)
#define EMI_MPUK2		(0x02B0)
#define EMI_MPUK2_2ND	(0x02B4)
#define EMI_MPUL2		(0x02B8)
#define EMI_MPUL2_2ND	(0x02BC)
#define EMI_MPUM2		(0x02C0)
#define EMI_MPUN2		(0x02C8)
#define EMI_MPUO2		(0x02D0)
#define EMI_MPUP2		(0x02D8)
#define EMI_MPUQ2		(0x02E0)
#define EMI_MPUR2		(0x02E8)
#define EMI_MPUU2		(0x0300)
#define EMI_MPUY2		(0x0320)

/* EMI IO delay, driving and MISC control registers */
#define EMI_IDLA            (0x0200)
#define EMI_IDLB            (0x0208)
#define EMI_IDLC            (0x0210)
#define EMI_IDLD            (0x0218)
#define EMI_IDLE            (0x0220)
#define EMI_IDLF            (0x0228)
#define EMI_IDLG            (0x0230)
#define EMI_IDLH            (0x0238)
#define EMI_IDLI            (0x0240) // IO input delay (DQS0 ~ DQS4)
#define EMI_IDLJ            (0x0248)
#define EMI_IDLK            (0x0250)

#define EMI_ODLA           (0x0258)
#define EMI_ODLB           (0x0260)
#define EMI_ODLC           (0x0268)
#define EMI_ODLD           (0x0270)
#define EMI_ODLE           (0x0278)
#define EMI_ODLF           (0x0280)
#define EMI_ODLG           (0x0288)

#define EMI_DUTA           (0x0290)
#define EMI_DUTB           (0x0298)
#define EMI_DUTC           (0x02A0)

#define EMI_DRVA           (0x02A8)
#define EMI_DRVB           (0x02B0)

#define EMI_IOCL           (0x02B8)
#define EMI_IOCM           (0x02C0) //IvanTseng, for 4T mode
#define EMI_IODC           (0x02C8)

#define EMI_ODTA           (0x02D0)
#define EMI_ODTB           (0x02D8)

/* EMI auto-tracking control registers */
#define EMI_DQSA           (0x0300)
#define EMI_DQSB           (0x0308)
#define EMI_DQSC           (0x0310)
#define EMI_DQSD           (0x0318)


#define EMI_DQSE           (0x0320)
#define EMI_DQSV           (0x0328)

#define EMI_CALA           (0x0330)
#define EMI_CALB           (0x0338)
#define EMI_CALC           (0x0340)
#define EMI_CALD           (0x0348)


#define EMI_CALE           (0x0350) //DDR data auto tracking control
#define EMI_CALF           (0x0358)
#define EMI_CALG           (0x0360) //DDR data auto tracking control
#define EMI_CALH           (0x0368)

#define EMI_CALI           (0x0370)
#define EMI_CALJ           (0x0378)
#define EMI_CALK           (0x0380)
#define EMI_CALL           (0x0388)


#define EMI_CALM           (0x0390)
#define EMI_CALN           (0x0398)

#define EMI_CALO           (0x03A0)
#define EMI_CALP           (0x03A8)

#define EMI_DUCA           (0x03B0)
#define EMI_DUCB           (0x03B8)
#define EMI_DUCC           (0x03C0)
#define EMI_DUCD           (0x03C8)
#define EMI_DUCE           (0x03D0)

/* EMI bus monitor control registers */
#define EMI_BMEN           (0x0400)
#define EMI_BCNT           (0x0408)
#define EMI_TACT           (0x0410)
#define EMI_TSCT           (0x0418)
#define EMI_WACT           (0x0420)
#define EMI_WSCT           (0x0428)
#define EMI_BACT           (0x0430)
#define EMI_BSCT           (0x0438)
#define EMI_MSEL           (0x0440)
#define EMI_TSCT2           (0x0448)
#define EMI_TSCT3           (0x0450)
#define EMI_WSCT2           (0x0458)
#define EMI_WSCT3           (0x0460)
#define EMI_MSEL2           (0x0468)
#define EMI_MSEL3           (0x0470)
#define EMI_MSEL4           (0x0478)
#define EMI_MSEL5           (0x0480)
#define EMI_MSEL6           (0x0488)
#define EMI_MSEL7           (0x0490)
#define EMI_MSEL8           (0x0498)
#define EMI_MSEL9           (0x04A0)
#define EMI_MSEL10           (0x04A8)
#define EMI_BMID0            (0x04B0)
#define EMI_BMID1            (0x04B8)
#define EMI_BMID2            (0x04C0)
#define EMI_BMID3            (0x04C8)
#define EMI_BMID4            (0x04D0)
#define EMI_BMID5            (0x04D8)

#define EMI_TTYPE1            (0x0500)
#define EMI_TTYPE2            (0x0508)
#define EMI_TTYPE3            (0x0510)
#define EMI_TTYPE4            (0x0518)
#define EMI_TTYPE5            (0x0520)
#define EMI_TTYPE6            (0x0528)
#define EMI_TTYPE7            (0x0530)
#define EMI_TTYPE8            (0x0538)
#define EMI_TTYPE9            (0x0540)
#define EMI_TTYPE10            (0x0548)
#define EMI_TTYPE11            (0x0550)
#define EMI_TTYPE12            (0x0558)
#define EMI_TTYPE13            (0x0560)
#define EMI_TTYPE14            (0x0568)
#define EMI_TTYPE15            (0x0570)
#define EMI_TTYPE16            (0x0578)
#define EMI_TTYPE17            (0x0580)
#define EMI_TTYPE18            (0x0588)
#define EMI_TTYPE19            (0x0590)
#define EMI_TTYPE20            (0x0598)
#define EMI_TTYPE21            (0x05A0)

/* EMI MBIST control registers*/
#define EMI_MBISTA            (0x0600)
#define EMI_MBISTB            (0x0608)
#define EMI_MBISTC            (0x0610)
#define EMI_MBISTD            (0x0618)
#define EMI_MBISTE            (0x0620) /* EMI MBIST status register */


/* EMI Flow control register A */
#define EMI_RFCA            (0x0630)
#define EMI_RFCB            (0x0638)
#define EMI_RFCC            (0x0640)
#define EMI_RFCD            (0x0648)

/* EMI memory protection align 64K */
#define EMI_MPU_ALIGNMENT   0x10000
#define EMI_PHY_OFFSET       0x40000000
#define SEC_PHY_SIZE        0x06000000

#define NO_PROTECTION       0
#define SEC_RW              1
#define SEC_RW_NSEC_R       2
#define SEC_RW_NSEC_W       3
#define SEC_R_NSEC_R        4
#define FORBIDDEN           5

#define SECURE_OS_MPU_REGION_ID    0
#define ATF_MPU_REGION_ID          1

#define LOCK                1
#define UNLOCK              0
#define EMIMPU_DX_SEC_BIT   30

#if defined(MACH_TYPE_MT6735) || defined(MACH_TYPE_MT6753)
#define EMIMPU_DOMAIN_NUM  8
#define EMIMPU_REGION_NUM  16

#define SET_ACCESS_PERMISSON(lock, d7, d6, d5, d4, d3, d2, d1, d0) ((((d3) << 9) | ((d2) << 6) | ((d1) << 3) | (d0)) | ((((d7) << 9) | ((d6) << 6) | ((d5) << 3) | (d4)) << 16) | (lock << 15))

#else
#define EMIMPU_DOMAIN_NUM  4
#define EMIMPU_REGION_NUM  8
#define SET_ACCESS_PERMISSON(lock, d3, d2, d1, d0) ((((d3) << 9) | ((d2) << 6) | ((d1) << 3) | (d0))  | (lock << 15))
#endif

extern uint64_t sip_emimpu_write(unsigned int offset, unsigned int reg_value);
extern uint32_t sip_emimpu_read(unsigned int offset);
extern void emimpu_set_security_access(void);
extern void emimpu_setup(void);

#endif
