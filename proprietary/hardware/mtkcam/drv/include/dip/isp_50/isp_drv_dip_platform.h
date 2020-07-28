/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _ISP_DRV_DIP_PLATFORM_H_
#define _ISP_DRV_DIP_PLATFORM_H_


//////////////////////////////////////////////////////////////////

#define MAX_METLOG_SIZE    1024


#define MX_SMXIO_PIXEL 128
#define MX_SMXIO_NUM 2
#define MX_SMXIO1_BTYES 5 //Bus size 36 bits
#define MX_SMXIO2_BTYES 2 //Bus size 16 bits
#define MX_SMXIO3_BTYES 2 //Bus size 16 bits
#define MX_SMXIO4_BTYES 1 //Bus size 8 bits
#define MX_SMXIO_HEIGHT 8000 //The maximum value is 7656 (32M)

#define SMX_BUF_ALIGNMENT 4

//32M 4:3 6632x4976 - 16:9 7656x4312	
//25M 4:3 5888x4416 - 16:9 6800x3824
#define MAX_SMX1_BUF_SIZE ((MX_SMXIO_PIXEL*MX_SMXIO1_BTYES*MX_SMXIO_HEIGHT)+SMX_BUF_ALIGNMENT)*2  //10.24M*2
#define MAX_SMX2_BUF_SIZE ((MX_SMXIO_PIXEL*MX_SMXIO2_BTYES*MX_SMXIO_HEIGHT)+SMX_BUF_ALIGNMENT)*2  //4.096M*2
#define MAX_SMX3_BUF_SIZE ((MX_SMXIO_PIXEL*MX_SMXIO3_BTYES*MX_SMXIO_HEIGHT)+SMX_BUF_ALIGNMENT)*2  //4.096M*2
#define MAX_SMX4_BUF_SIZE ((MX_SMXIO_PIXEL*MX_SMXIO4_BTYES*MX_SMXIO_HEIGHT)+SMX_BUF_ALIGNMENT)*2  //2.048M*2

/**
DIP cq module info
*/
#define ISP_CQ_APB_INST             0x0
#define ISP_CQ_NOP_INST             0x1
#define ISP_DRV_CQ_END_TOKEN        0x1C000000
#define ISP_DRV_CQ_DUMMY_TOKEN      ((ISP_CQ_NOP_INST<<26)|0x4184)
#define ISP_CQ_DUMMY_PA             0x88100000

#define ISP_DIP_CQ_DUMMY_WR_TOKEN   0x27000


///////////////////////////////////////////////////////////////////////////
//DIP_X_CTL_YUV_EN, reg:0x15022004
#define DIP_X_REG_CTL_YUV_EN_MFB                   (1UL<<0)
#define DIP_X_REG_CTL_YUV_EN_C02B                  (1UL<<1)
#define DIP_X_REG_CTL_YUV_EN_C24                   (1UL<<2)
#define DIP_X_REG_CTL_YUV_EN_G2C                   (1UL<<3)
#define DIP_X_REG_CTL_YUV_EN_C42                   (1UL<<4)
#define DIP_X_REG_CTL_YUV_EN_NBC                   (1UL<<5)
#define DIP_X_REG_CTL_YUV_EN_NBC2                  (1UL<<6)
#define DIP_X_REG_CTL_YUV_EN_PCA                   (1UL<<7)
#define DIP_X_REG_CTL_YUV_EN_SEEE                  (1UL<<8)
#define DIP_X_REG_CTL_YUV_EN_COLOR                 (1UL<<9)
#define DIP_X_REG_CTL_YUV_EN_NR3D                  (1UL<<10)
#define DIP_X_REG_CTL_YUV_EN_SL2B                  (1UL<<11)
#define DIP_X_REG_CTL_YUV_EN_SL2C                  (1UL<<12)
#define DIP_X_REG_CTL_YUV_EN_SL2D                  (1UL<<13)
#define DIP_X_REG_CTL_YUV_EN_SL2E                  (1UL<<14)
#define DIP_X_REG_CTL_YUV_EN_SRZ1                  (1UL<<15)
#define DIP_X_REG_CTL_YUV_EN_SRZ2                  (1UL<<16)
#define DIP_X_REG_CTL_YUV_EN_CRZ                   (1UL<<17)
#define DIP_X_REG_CTL_YUV_EN_MIX1                  (1UL<<18)
#define DIP_X_REG_CTL_YUV_EN_MIX2                  (1UL<<19)
#define DIP_X_REG_CTL_YUV_EN_MIX3                  (1UL<<20)
#define DIP_X_REG_CTL_YUV_EN_CRSP                  (1UL<<21)
#define DIP_X_REG_CTL_YUV_EN_C24B                  (1UL<<22)
#define DIP_X_REG_CTL_YUV_EN_MDPCROP               (1UL<<23)
#define DIP_X_REG_CTL_YUV_EN_C02                   (1UL<<24)
#define DIP_X_REG_CTL_YUV_EN_FE                    (1UL<<25)
#define DIP_X_REG_CTL_YUV_EN_MFBW                  (1UL<<26)
#define DIP_X_REG_CTL_YUV_EN_rsv_27                (1UL<<27)
#define DIP_X_REG_CTL_YUV_EN_PLNW1                 (1UL<<28)
#define DIP_X_REG_CTL_YUV_EN_PLNR1                 (1UL<<29)
#define DIP_X_REG_CTL_YUV_EN_PLNW2                 (1UL<<30)
#define DIP_X_REG_CTL_YUV_EN_PLNR2                 (1UL<<31)

//DIP_X_CTL_YUV2_EN, reg:0x15022008
#define DIP_X_REG_CTL_YUV2_EN_FM                   (1UL<<0)
#define DIP_X_REG_CTL_YUV2_EN_SRZ3                 (1UL<<1)
#define DIP_X_REG_CTL_YUV2_EN_SRZ4                 (1UL<<2)
#define DIP_X_REG_CTL_YUV2_EN_SRZ5                 (1UL<<3)
#define DIP_X_REG_CTL_YUV2_EN_HFG                  (1UL<<4)
#define DIP_X_REG_CTL_YUV2_EN_SL2I                 (1UL<<5)
#define DIP_X_REG_CTL_YUV2_EN_NDG                  (1UL<<6)
#define DIP_X_REG_CTL_YUV2_EN_NDG2                 (1UL<<7)
#define DIP_X_REG_CTL_YUV2_EN_SMX2                 (1UL<<8)
#define DIP_X_REG_CTL_YUV2_EN_SMX3                 (1UL<<9)
#define DIP_X_REG_CTL_YUV2_EN_MIX4                 (1UL<<10)


//DIP_X_CTL_RGB_EN, reg:0x1502200C
#define DIP_X_REG_CTL_RGB_EN_UNP                   (1UL<<0)
#define DIP_X_REG_CTL_RGB_EN_UFD                   (1UL<<1)
#define DIP_X_REG_CTL_RGB_EN_PGN                   (1UL<<2)
#define DIP_X_REG_CTL_RGB_EN_SL2                   (1UL<<3)
#define DIP_X_REG_CTL_RGB_EN_UDM                   (1UL<<4)
#define DIP_X_REG_CTL_RGB_EN_LCE                   (1UL<<5)
#define DIP_X_REG_CTL_RGB_EN_G2G                   (1UL<<6)
#define DIP_X_REG_CTL_RGB_EN_GGM                   (1UL<<7)
#define DIP_X_REG_CTL_RGB_EN_GDR1                  (1UL<<8)
#define DIP_X_REG_CTL_RGB_EN_GDR2                  (1UL<<9)
#define DIP_X_REG_CTL_RGB_EN_DBS2                  (1UL<<10)
#define DIP_X_REG_CTL_RGB_EN_OBC2                  (1UL<<11)
#define DIP_X_REG_CTL_RGB_EN_RMG2                  (1UL<<12)
#define DIP_X_REG_CTL_RGB_EN_BNR2                  (1UL<<13)
#define DIP_X_REG_CTL_RGB_EN_RMM2                  (1UL<<14)
#define DIP_X_REG_CTL_RGB_EN_LSC2                  (1UL<<15)
#define DIP_X_REG_CTL_RGB_EN_RCP2                  (1UL<<16)
#define DIP_X_REG_CTL_RGB_EN_PAK2                  (1UL<<17)
#define DIP_X_REG_CTL_RGB_EN_RNR                   (1UL<<18)
#define DIP_X_REG_CTL_RGB_EN_SL2G                  (1UL<<19)
#define DIP_X_REG_CTL_RGB_EN_SL2H                  (1UL<<20)
#define DIP_X_REG_CTL_RGB_EN_BIT21                 (1UL<<21)
#define DIP_X_REG_CTL_RGB_EN_SL2K                  (1UL<<22)
#define DIP_X_REG_CTL_RGB_EN_FLC                   (1UL<<23)
#define DIP_X_REG_CTL_RGB_EN_FLC2                  (1UL<<24)
#define DIP_X_REG_CTL_RGB_EN_SMX1                  (1UL<<25)
#define DIP_X_REG_CTL_RGB_EN_PAKG2                 (1UL<<26)
#define DIP_X_REG_CTL_RGB_EN_G2G2                  (1UL<<27)
#define DIP_X_REG_CTL_RGB_EN_GGM2                  (1UL<<28)
#define DIP_X_REG_CTL_RGB_EN_WSHIFT                (1UL<<29)
#define DIP_X_REG_CTL_RGB_EN_WSYNC                 (1UL<<30)
#define DIP_X_REG_CTL_RGB_EN_MDPCROP2              (1UL<<31)

//DIP_X_CTL_RGB2_EN, reg:0x150220E8
#define DIP_X_REG_CTL_RGB2_EN_SMX4                 (1UL<<0)
#define DIP_X_REG_CTL_RGB2_EN_ADBS2                (1UL<<1)
#define DIP_X_REG_CTL_RGB2_EN_DCPN2                (1UL<<2)
#define DIP_X_REG_CTL_RGB2_EN_CPN2                 (1UL<<3)


//DIP_X_CTL_DMA_EN, reg:0x15022010
#define DIP_X_REG_CTL_DMA_EN_NONE                  (0)
#define DIP_X_REG_CTL_DMA_EN_IMGI                  (1UL<<0)
#define DIP_X_REG_CTL_DMA_EN_IMGBI                 (1UL<<1)
#define DIP_X_REG_CTL_DMA_EN_IMGCI                 (1UL<<2)
#define DIP_X_REG_CTL_DMA_EN_UFDI                  (1UL<<3)
#define DIP_X_REG_CTL_DMA_EN_VIPI                  (1UL<<4)
#define DIP_X_REG_CTL_DMA_EN_VIP2I                 (1UL<<5)
#define DIP_X_REG_CTL_DMA_EN_VIP3I                 (1UL<<6)
#define DIP_X_REG_CTL_DMA_EN_LCEI                  (1UL<<7)
#define DIP_X_REG_CTL_DMA_EN_DEPI                  (1UL<<8)
#define DIP_X_REG_CTL_DMA_EN_DMGI                  (1UL<<9)
//#define DIP_X_REG_CTL_DMA_EN_MFBO                         (1UL<<10)  //PAK2O
#define DIP_X_REG_CTL_DMA_EN_PAK2O                 (1UL<<10)  //PAK2O
#define DIP_X_REG_CTL_DMA_EN_IMG2O                 (1UL<<11)
#define DIP_X_REG_CTL_DMA_EN_IMG2BO                (1UL<<12)
#define DIP_X_REG_CTL_DMA_EN_IMG3O                 (1UL<<13)
#define DIP_X_REG_CTL_DMA_EN_IMG3BO                (1UL<<14)
#define DIP_X_REG_CTL_DMA_EN_IMG3CO                (1UL<<15)
#define DIP_X_REG_CTL_DMA_EN_FEO                   (1UL<<16)
#define DIP_X_REG_CTL_DMA_EN_ADL2                  (1UL<<17)
#define DIP_X_REG_CTL_DMA_EN_SMX1I                 (1UL<<18)
#define DIP_X_REG_CTL_DMA_EN_SMX2I                 (1UL<<19)
#define DIP_X_REG_CTL_DMA_EN_SMX3I                 (1UL<<20)
#define DIP_X_REG_CTL_DMA_EN_SMX1O                 (1UL<<21)
#define DIP_X_REG_CTL_DMA_EN_SMX2O                 (1UL<<22)
#define DIP_X_REG_CTL_DMA_EN_SMX3O                 (1UL<<23)
#define DIP_X_REG_CTL_DMA_EN_SMX4I                 (1UL<<24)
#define DIP_X_REG_CTL_DMA_EN_SMX4O                 (1UL<<25)


#define DIP_X_REG_CTL_DMA_EN_WROTO                 (1UL<<28)
#define DIP_X_REG_CTL_DMA_EN_WDMAO                 (1UL<<29)
#define DIP_X_REG_CTL_DMA_EN_JPEGO                 (1UL<<30)
#define DIP_X_REG_CTL_DMA_EN_VENCO                 (1UL<<31)

//DIP_X_CQ_THR0_CTL ~ DIP_X_CQ_THR14_CTL, reg0x15022104~~
#define DIP_X_CQ_THRX_CTL_EN                       (1L<<0)
#define DIP_X_CQ_THRX_CTL_THRX_MODE                (1L<<4)

//DIP_X_CTL_CQ_INT_EN, reg:0x15022024
#define DIP_X_CTL_CQ_INT_TH0                       (1L<<0)
#define DIP_X_CTL_CQ_INT_TH1                       (1L<<1)
#define DIP_X_CTL_CQ_INT_TH2                       (1L<<2)
#define DIP_X_CTL_CQ_INT_TH3                       (1L<<3)
#define DIP_X_CTL_CQ_INT_TH4                       (1L<<4)
#define DIP_X_CTL_CQ_INT_TH5                       (1L<<5)
#define DIP_X_CTL_CQ_INT_TH6                       (1L<<6)
#define DIP_X_CTL_CQ_INT_TH7                       (1L<<7)
#define DIP_X_CTL_CQ_INT_TH8                       (1L<<8)
#define DIP_X_CTL_CQ_INT_TH9                       (1L<<9)
#define DIP_X_CTL_CQ_INT_TH10                      (1L<<10)
#define DIP_X_CTL_CQ_INT_TH11                      (1L<<11)
#define DIP_X_CTL_CQ_INT_TH12                      (1L<<12)
#define DIP_X_CTL_CQ_INT_TH13                      (1L<<13)
#define DIP_X_CTL_CQ_INT_TH14                      (1L<<14)
#define DIP_X_CTL_CQ_INT_TH15                      (1L<<15)
#define DIP_X_CTL_CQ_INT_TH16                      (1L<<16)
#define DIP_X_CTL_CQ_INT_TH17                      (1L<<17)
#define DIP_X_CTL_CQ_INT_TH18                      (1L<<18)
#define DIP_X_CTL_CQ_INT_DONE                      (1L<<29)
#define DIP_X_CTL_CQ_INT_CODE_ERR                  (1L<<30)
#define DIP_X_CTL_CQ_INT_APB_OUT_RANGE             (1L<<31)

//DIP_X_CTL_TDR_CTL, reg:0x15022050
#define DIP_X_REG_CTL_TDR_SOF_RST_EN               (1L<<0)
#define DIP_X_REG_CTL_CTL_EXTENSION_EN             (1L<<1)
#define DIP_X_REG_CTL_TDR_SZ_DET                   (1L<<2)
#define DIP_X_REG_CTL_TDR_RN                       (1L<<3)

//DIP_X_REG_CTL_TDR_TCM_EN, reg:0x15022058
#define DIP_X_REG_CTL_TDR_NDG_TCM_EN               (1L<<0)
#define DIP_X_REG_CTL_TDR_IMGI_TCM_EN              (1L<<1)
#define DIP_X_REG_CTL_TDR_IMGBI_TCM_EN             (1L<<2)
#define DIP_X_REG_CTL_TDR_IMGCI_TCM_EN             (1L<<3)
#define DIP_X_REG_CTL_TDR_UFDI_TCM_EN              (1L<<4)
#define DIP_X_REG_CTL_TDR_LCEI_TCM_EN              (1L<<5)
#define DIP_X_REG_CTL_TDR_VIPI_TCM_EN              (1L<<6)
#define DIP_X_REG_CTL_TDR_VIP2I_TCM_EN             (1L<<7)
#define DIP_X_REG_CTL_TDR_VIP3I_TCM_EN             (1L<<8)
#define DIP_X_REG_CTL_TDR_DMGI_TCM_EN              (1L<<9)
#define DIP_X_REG_CTL_TDR_DEPI_TCM_EN              (1L<<10)
#define DIP_X_REG_CTL_TDR_IMG2O_TCM_EN             (1L<<11)
#define DIP_X_REG_CTL_TDR_IMG2BO_TCM_EN            (1L<<12)
#define DIP_X_REG_CTL_TDR_IMG3O_TCM_EN             (1L<<13)
#define DIP_X_REG_CTL_TDR_IMG3BO_TCM_EN            (1L<<14)
#define DIP_X_REG_CTL_TDR_IMG3CO_TCM_EN            (1L<<15)
#define DIP_X_REG_CTL_TDR_FEO_TCM_EN               (1L<<16)
#define DIP_X_REG_CTL_TDR_PAK2O_TCM_EN             (1L<<17)
#define DIP_X_REG_CTL_TDR_UFD_TCM_EN               (1L<<18)
#define DIP_X_REG_CTL_TDR_UNP_TCM_EN               (1L<<19)
#define DIP_X_REG_CTL_TDR_UDM_TCM_EN               (1L<<20)
#define DIP_X_REG_CTL_TDR_LSC2_TCM_EN              (1L<<21)
#define DIP_X_REG_CTL_TDR_SL2_TCM_EN               (1L<<22)
#define DIP_X_REG_CTL_TDR_SL2B_TCM_EN              (1L<<23)
#define DIP_X_REG_CTL_TDR_SL2C_TCM_EN              (1L<<24)
#define DIP_X_REG_CTL_TDR_SL2D_TCM_EN              (1L<<25)
#define DIP_X_REG_CTL_TDR_SL2E_TCM_EN              (1L<<26)
#define DIP_X_REG_CTL_TDR_G2C_TCM_EN               (1L<<27)
#define DIP_X_REG_CTL_TDR_NDG2_TCM_EN              (1L<<28)
//#define DIP_X_REG_CTL_TDR_rsv_28                   (1L<<28)
#define DIP_X_REG_CTL_TDR_SRZ1_TCM_EN              (1L<<29)
#define DIP_X_REG_CTL_TDR_SRZ2_TCM_EN              (1L<<30)
#define DIP_X_REG_CTL_TDR_LCE_TCM_EN               (1L<<31)


//DIP_X_REG_CTL_TDR_TCM2_EN,  0x1502205C
#define DIP_X_REG_CTL_TDR_CRZ_TCM_EN               (1L<<0)
#define DIP_X_REG_CTL_TDR_FLC2_TCM_EN              (1L<<1)
#define DIP_X_REG_CTL_TDR_NR3D_TCM_EN              (1L<<2)
#define DIP_X_REG_CTL_TDR_MDPCROP_TCM_EN           (1L<<3)
#define DIP_X_REG_CTL_TDR_CRSP_TCM_EN              (1L<<4)
#define DIP_X_REG_CTL_TDR_C02_TCM_EN               (1L<<5)
#define DIP_X_REG_CTL_TDR_C02B_TCM_EN              (1L<<6)
#define DIP_X_REG_CTL_TDR_C24_TCM_EN               (1L<<7)
#define DIP_X_REG_CTL_TDR_C42_TCM_EN               (1L<<8)
#define DIP_X_REG_CTL_TDR_C24B_TCM_EN              (1L<<9)
#define DIP_X_REG_CTL_TDR_MFB_TCM_EN               (1L<<10)
#define DIP_X_REG_CTL_TDR_PCA_TCM_EN               (1L<<11)
#define DIP_X_REG_CTL_TDR_SEEE_TCM_EN              (1L<<12)
#define DIP_X_REG_CTL_TDR_NBC_TCM_EN               (1L<<13)
#define DIP_X_REG_CTL_TDR_NBC2_TCM_EN              (1L<<14)
#define DIP_X_REG_CTL_TDR_DBS2_TCM_EN              (1L<<15)
#define DIP_X_REG_CTL_TDR_RMG2_TCM_EN              (1L<<16)
#define DIP_X_REG_CTL_TDR_BNR2_TCM_EN              (1L<<17)
#define DIP_X_REG_CTL_TDR_RMM2_TCM_EN              (1L<<18)
#define DIP_X_REG_CTL_TDR_SRZ3_TCM_EN              (1L<<19)
#define DIP_X_REG_CTL_TDR_SRZ4_TCM_EN              (1L<<20)
#define DIP_X_REG_CTL_TDR_RCP2_TCM_EN              (1L<<21)
#define DIP_X_REG_CTL_TDR_SRZ5_TCM_EN              (1L<<22)
#define DIP_X_REG_CTL_TDR_RNR_TCM_EN               (1L<<23)
#define DIP_X_REG_CTL_TDR_SL2G_TCM_EN              (1L<<24)
#define DIP_X_REG_CTL_TDR_SL2H_TCM_EN              (1L<<25)
#define DIP_X_REG_CTL_TDR_SL2I_TCM_EN              (1L<<26)
#define DIP_X_REG_CTL_TDR_HFG_TCM_EN               (1L<<27)
//#define DIP_X_REG_CTL_TDR_HLR2_TCM_EN              (1L<<28)
#define DIP_X_REG_CTL_TDR_TCM_EN_RSV28             (1L<<28)
#define DIP_X_REG_CTL_TDR_SL2K_TCM_EN              (1L<<29)
#define DIP_X_REG_CTL_TDR_ADL2_TCM_EN              (1L<<30)
#define DIP_X_REG_CTL_TDR_FLC_TCM_EN               (1L<<31)


//DIP_X_REG_CTL_TDR_TCM3_EN, 0x150220E0
#define DIP_X_REG_CTL_TDR_SMX1_TCM_EN              (1L<<0)
#define DIP_X_REG_CTL_TDR_SMX2_TCM_EN              (1L<<1)
#define DIP_X_REG_CTL_TDR_SMX3_TCM_EN              (1L<<2)
#define DIP_X_REG_CTL_TDR_MDPCROP2_TCM_EN          (1L<<3)
#define DIP_X_REG_CTL_TDR_COLOR_TCM_EN             (1L<<4)
#define DIP_X_REG_CTL_TDR_SMX1I_TCM_EN             (1L<<5)
#define DIP_X_REG_CTL_TDR_SMX2I_TCM_EN             (1L<<6)
#define DIP_X_REG_CTL_TDR_SMX3I_TCM_EN             (1L<<7)
#define DIP_X_REG_CTL_TDR_SMX1O_TCM_EN             (1L<<8)
#define DIP_X_REG_CTL_TDR_SMX2O_TCM_EN             (1L<<9)
#define DIP_X_REG_CTL_TDR_SMX3O_TCM_EN             (1L<<10)
#define DIP_X_REG_CTL_TDR_SMX4_TCM_EN              (1L<<11)
#define DIP_X_REG_CTL_TDR_SMX4I_TCM_EN             (1L<<12)
#define DIP_X_REG_CTL_TDR_SMX4O_TCM_EN             (1L<<13)
#define DIP_X_REG_CTL_TDR_ADBS2_TCM_EN             (1L<<14)
#define DIP_X_REG_CTL_TDR_FE_TCM_EN                (1L<<15)
#define DIP_X_REG_CTL_TDR_DCPN2_TCM_EN             (1L<<16)
#define DIP_X_REG_CTL_TDR_CPN2_TCM_EN              (1L<<17)



////////////////////////////////////DMA Format///////////////////////////////
//imgi
#define DIP_IMGI_FMT_YUV422_1P    0
#define DIP_IMGI_FMT_YUV422_2P    1
#define DIP_IMGI_FMT_YUV422_3P    2
#define DIP_IMGI_FMT_YUV420_2P    3
#define DIP_IMGI_FMT_YUV420_3P    4
#define DIP_IMGI_FMT_Y_ONLY    5
#define DIP_IMGI_FMT_MFB_BLEND_MODE    6
#define DIP_IMGI_FMT_MFB_MIX_MODE    7
#define DIP_IMGI_FMT_BAYER8    8
#define DIP_IMGI_FMT_BAYER10    9
#define DIP_IMGI_FMT_BAYER12    10
#define DIP_IMGI_FMT_BAYER14    11
#define DIP_IMGI_FMT_BAYER8_2BYTEs    12
#define DIP_IMGI_FMT_BAYER10_2BYTEs    13
#define DIP_IMGI_FMT_BAYER12_2BYTEs    14
#define DIP_IMGI_FMT_BAYER14_2BYTEs    15
#define DIP_IMGI_FMT_BAYER10_MIPI    16
#define DIP_IMGI_FMT_RGB565    24
#define DIP_IMGI_FMT_RGB888    25
#define DIP_IMGI_FMT_XRGB8888    26
#define DIP_IMGI_FMT_RGB101010    27
#define DIP_IMGI_FMT_RGB121212    28

//vipi_fmt
#define DIP_VIPI_FMT_YUV422_1P                  0
#define DIP_VIPI_FMT_YUV422_2P                  1
#define DIP_VIPI_FMT_YUV422_3P                  2
#define DIP_VIPI_FMT_YUV420_2P                  3
#define DIP_VIPI_FMT_YUV420_3P                  4
#define DIP_VIPI_FMT_Y_ONLY                  5
#define DIP_VIPI_FMT_RGB565                  24
#define DIP_VIPI_FMT_RGB888                  25
#define DIP_VIPI_FMT_XRGB8888                  26
#define DIP_VIPI_FMT_RGB101010                  27
#define DIP_VIPI_FMT_RGB121212                  28

//ufdi fmt
#define DIP_UFDI_FMT_UFO_LENGTH     0
#define DIP_UFDI_FMT_WEIGHTING      1

//dmgi fmt
#define DIP_DMGI_FMT_DRZ     0
#define DIP_DMGI_FMT_FM      1

//depi fmt
#define DIP_DEPI_FMT_DRZ     0
#define DIP_DEPI_FMT_FM      1

//img3o_fmt
#define DIP_IMG3O_FMT_YUV422_1P    0
#define DIP_IMG3O_FMT_YUV422_2P    1
#define DIP_IMG3O_FMT_YUV422_3P    2
#define DIP_IMG3O_FMT_YUV420_2P    3
#define DIP_IMG3O_FMT_YUV420_3P    4

//img3o_fmt
#define DIP_IMG2O_FMT_YUV422_1P    0
#define DIP_IMG2O_FMT_YUV422_2P    1


typedef enum{
    DIP_HW_A   = 0,
    //DIP_HW_B,            //not supported in everest
    DIP_HW_MAX,
    MAX_DIP_HW_MODULE = DIP_HW_MAX
}DIP_HW_MODULE;

/**
    R/W register structure
*/
typedef struct
{
    unsigned int     module;
    unsigned int     Addr;
    unsigned int     Data;
}ISP_DRV_REG_IO_STRUCT;

typedef struct cq_desc_t{
    union {
        struct {
            unsigned int osft_addr_lsb   :16;
            unsigned int cnt             :10;
            unsigned int inst            :3;
            unsigned int osft_addr_msb   :3;
        } token;
        unsigned int cmd;
    } u;
    unsigned int v_reg_addr;
}ISP_DRV_CQ_CMD_DESC_STRUCT;


typedef struct {
    unsigned int id;
    unsigned int sw_addr_ofst;
    unsigned int reg_num;
}ISP_DRV_CQ_MODULE_INFO_STRUCT;


/**
DIP hw CQ
*/
typedef enum
{
    ISP_DRV_DIP_CQ_THRE0 = 0,
    ISP_DRV_DIP_CQ_THRE1,
    ISP_DRV_DIP_CQ_THRE2,
    ISP_DRV_DIP_CQ_THRE3,
    ISP_DRV_DIP_CQ_THRE4,
    ISP_DRV_DIP_CQ_THRE5,
    ISP_DRV_DIP_CQ_THRE6,
    ISP_DRV_DIP_CQ_THRE7,
    ISP_DRV_DIP_CQ_THRE8,
    ISP_DRV_DIP_CQ_THRE9,
    ISP_DRV_DIP_CQ_THRE10,
    ISP_DRV_DIP_CQ_THRE11,  //vss usage
    ISP_DRV_DIP_CQ_NUM,    //baisc set, dip cqs
    ISP_DRV_DIP_CQ_NONE,
    ///////////////////
    /* we only need 12 CQ threads in this chip,
       so we move the follwoing enum thread behind ISP_DRV_DIP_CQ_NUM */
    ISP_DRV_DIP_CQ_THRE12,
    ISP_DRV_DIP_CQ_THRE13,
    ISP_DRV_DIP_CQ_THRE14,
    ISP_DRV_DIP_CQ_THRE15,	// For CQ_THREAD15, it does not connect to GCE for this chip.
    ISP_DRV_DIP_CQ_THRE16,	// For CQ_THREAD16, it does not connect to GCE for this chip.
    ISP_DRV_DIP_CQ_THRE17,	// For CQ_THREAD17, it does not connect to GCE for this chip.
    ISP_DRV_DIP_CQ_THRE18,	// For CQ_THREAD18, it does not connect to GCE for this chip.

}E_ISP_DIP_CQ;



///////////////////////////////////////////////////////////
/**
DIP CQ descriptor
*/
typedef enum {
    DIP_A_CTL = 0,          //0x15022004~0x1502201c
    DIP_A_CTL2,             //0x150220e8~0x150220e8
    DIP_A_CTL_TDR,          //0x15022050~0x1502205c
    DIP_A_CTL_TDR2,         //0x150220E0
    DIP_A_CTL_DONE,         //0x1502207c
    DIP_A_TDRI,             //0x15022304~0x1502230c
    DIP_A_VECTICAL_FLIP,    //0x15022310
    DIP_A_SPECIAL_FUN,      //0x1502231C
    DIP_A_IMG2O,            //0x15022330~0x15022348
    DIP_A_IMG2O_CRSP,       //0x15022358~0x15022358
    DIP_A_IMG2BO,           //0x15022360~0x15022378
    DIP_A_IMG2BO_CRSP,      //0x15022388~0x15022388
    DIP_A_IMG3O,            //0x15022390~0x150223a8
    DIP_A_IMG3O_CRSP,       //0x150223b8~0x150223b8
    DIP_A_IMG3BO,           //0x150223c0~0x150223d8
    DIP_A_IMG3BO_CRSP,      //0x150223e8~0x150223e8
    DIP_A_IMG3CO,           //0x150223f0~0x15022408
    DIP_A_IMG3CO_CRSP,      //0x15022418~0x15022418
    DIP_A_FEO,              //0x15022420~0x15022438
    DIP_A_PAK2O,            //0x15022450~0x15022468
    DIP_A_PAK2O_CROP,       //0x15022478~0x15022478
    DIP_A_IMGI,             //0x15022500~0x15022518
    DIP_A_IMGBI,            //0x15022530~0x15022548
    DIP_A_IMGCI,            //0x15022560~0x15022578
    DIP_A_VIPI,             //0x15022590~0x150225a8
    DIP_A_VIP2I,            //0x150225c0~0x150225d8
    DIP_A_VIP3I,            //0x150225f0~0x15022608
    DIP_A_DMGI,             //0x15022620~0x15022638
    DIP_A_DEPI,             //0x15022650~0x15022668
    DIP_A_LCEI,             //0x15022680~0x15022698
    DIP_A_UFDI,             //0x150226b0~0x150226c8
    DIP_A_UFD,              //0x15023040~0x1502305C
    DIP_A_PGN,              //0x15023440~0x15023454
    DIP_A_SL2,              //0x150235C0~0x150235EC
    DIP_A_UDM,              //0x15023540~0x15023598
    DIP_A_G2G,              //0x15024080~0x1502409C
    DIP_A_MFB,              //0x15025100~0x15025158
    DIP_A_FLC,              //0x15024000~0x15024018
    DIP_A_FLC2,             //0x15024040~0x15024058
    DIP_A_G2C,              //0x15025240~0x15025270
    DIP_A_C42,              //0x15025040
    DIP_A_SRZ3,             //0x15026940~0x15026960
    DIP_A_SRZ4,             //0x15025880~0x150258A0
    DIP_A_SRZ5,             //0x15025180~0x150251A0
    DIP_A_SEEE,             //0x150266C0~0x15026750
    DIP_A_NDG2,             //0x15026980~0x15026994
    DIP_A_NR3D,             //0x15027380~0x15027418
    DIP_A_SL2B,             //0x15025800~0x1502582C
    DIP_A_SL2C,             //0x15026900~0x1502692C
    DIP_A_SRZ1,             //0x15025900~0x15025920
    DIP_A_SRZ2,             //0x15026680~0x150266A0
    DIP_A_CRZ,              //0x15027300~0x15027334
    DIP_A_MIX1,             //0x150258C0~0x150258C4
    DIP_A_MIX2,             //0x15026640~0x15026644
    DIP_A_MIX3,             //0x150252C0~0x150252C4
    DIP_A_MIX4,             //0x15027540~0x15027544
    DIP_A_SL2D,             //0x15026780~0x150267AC
    DIP_A_SL2E,             //0x15027440~0x1502746C
    DIP_A_MDP,              //0x15022d20~0x15022d24
    DIP_A_ANR2,             //0x15026800~0x150268EC
    DIP_A_UNP,              //0x15023000
    DIP_A_C02,              //0x15025080~0x15025088
    DIP_A_FE,               //0x15025940~0x15025950
    DIP_A_CRSP,             //0x15027480~0x15027494
    DIP_A_C02B,             //0x150250C0~0x150250C8
    DIP_A_C24,              //0x15025000
    DIP_A_C24B,             //0x150274C0
    DIP_A_LCE,              //0x15024100~0x15024170
    DIP_A_FM,               //0x15026A00~0x15026A08
    DIP_A_GGM,              //0x15024180~0x15024480
    DIP_A_PCA,              //0x15026000~0x15026618
    DIP_A_OBC2,             //0x150231C0~0x150231DC
    DIP_A_BNR2,             //0x15023240~0x150232BC
    DIP_A_RMG2,             //0x15023200~0x15023208
    DIP_A_RMM2,             //0x150232C0~0x150232E0
    DIP_A_RCP2,             //0x15023f60~0x15023f64
    DIP_A_ADBS2,            //0x15023600~0x15023650
    DIP_A_DCPN2,            //0x15023680~0x1502369C
    DIP_A_CPN2,             //0x150236C0~0x150236FC
    DIP_A_LSC2,             //0x15023340~0x15023368
    DIP_A_DBS2,             //0x15023140~0x15023170
    DIP_A_ANR,              //0x15025300~0x150257E4
    DIP_A_SL2G,             //0x15023180~0x150231AC
    DIP_A_SL2H,             //0x15023500~0x1502352C
    DIP_A_HLR2,             //0x15024580~0x15024580
    DIP_A_HLR2_1,           //0x15024588~0x150245ac
    DIP_A_RNR,              //0x150234C0~0x150234FC
    DIP_A_HFG,              //0x15027280~0x150272B4
    DIP_A_SL2I,             //0x150272C0~0x150272EC
    DIP_A_SL2K,             //0x15023400~0x1502342C
    DIP_A_G2G2,             //0x150240C0~0x150240DC
    DIP_A_GGM2,             //0x15024640~0x15024940
    DIP_A_WSHIFT,           //0x15024B00~0x15024B10
    DIP_A_WSYNC,            //0x15024B40~0x15024B50
    DIP_A_COLOR,            //0x15027000~0x15027268
    DIP_A_NDG,              //0x15025840~0x15025854
    DIP_A_END_,             //must be kept at the end of descriptor
    //CQ ==> 0x15022100~0x150221c0 (set them via physical address directly)
    DIP_A_MODULE_MAX,
    DIP_A_DUMMY = 0xff
}DIP_A_MODULE_ENUM;


#define DEQUE_DMA_MASK 0xf0012801

#define DIP_OFFSET 0x22000

static ISP_DRV_CQ_MODULE_INFO_STRUCT mIspDipCQModuleInfo[DIP_A_MODULE_MAX]
=  {{DIP_A_CTL,             0x0004, 7  },  //0x15022004 ~ 0x1502201c
   {DIP_A_CTL2,             0x00e8, 1  },  //0x150220e8~0x150220e8
   {DIP_A_CTL_TDR,          0x0050, 4  },  //0x15022050 ~ 0x1502205c
   {DIP_A_CTL_TDR2,         0x00e0, 1  },  //0x150220E0
   {DIP_A_CTL_DONE,         0x007c, 1  },  //0x1502207c
   {DIP_A_TDRI,             0x0304, 3  },  //0x15022304~0x1502230c
   {DIP_A_VECTICAL_FLIP,    0x0310, 1  },  //0x15022310
   {DIP_A_SPECIAL_FUN,      0x031c, 1  },  //0x1502231c
   {DIP_A_IMG2O,            0x0334, 6  },  //0x15022334 ~ 0x15022348
   {DIP_A_IMG2O_CRSP,       0x0358, 1  },  //0x15022358
   {DIP_A_IMG2BO,           0x0364, 6  },  //0x15022364 ~ 0x15022378
   {DIP_A_IMG2BO_CRSP,      0x0388, 1  },  //0x15022388
   {DIP_A_IMG3O,            0x0394, 6  },  //0x15022394 ~ 0x150223a8
   {DIP_A_IMG3O_CRSP,       0x03b8, 1  },  //0x150223b8
   {DIP_A_IMG3BO,           0x03c4, 6  },  //0x150223c4 ~ 0x150223d8
   {DIP_A_IMG3BO_CRSP,      0x03e8, 1  },  //0x150223e8
   {DIP_A_IMG3CO,           0x03f4, 6  },  //0x150223f4 ~ 0x15022408
   {DIP_A_IMG3CO_CRSP,      0x0418, 1  },  //0x15022418
   {DIP_A_FEO,              0x0424, 6  },  //0x15022424 ~ 0x15022438
   {DIP_A_PAK2O,            0x0454, 6  },  //0x15022454 ~ 0x15022468
   {DIP_A_PAK2O_CROP,       0x0478, 1  },  //0x15022478
   {DIP_A_IMGI,             0x0504, 6  },  //0x15022504 ~ 0x15022518     
   {DIP_A_IMGBI,            0x0534, 6  },  //0x15022534 ~ 0x15022548
   {DIP_A_IMGCI,            0x0564, 6  },  //0x15022564 ~ 0x15022578
   {DIP_A_VIPI,             0x0594, 6  },  //0x15022594 ~ 0x150225a8
   {DIP_A_VIP2I,            0x05c4, 6  },  //0x150225c4 ~ 0x150225d8
   {DIP_A_VIP3I,            0x05f4, 6  },  //0x150225f4 ~ 0x15022608
   {DIP_A_DMGI,             0x0624, 6  },  //0x15022624 ~ 0x15022638
   {DIP_A_DEPI,             0x0654, 6  },  //0x15022654 ~ 0x15022668
   {DIP_A_LCEI,             0x0684, 6  },  //0x15022684 ~ 0x15022698
   {DIP_A_UFDI,             0x06b4, 6  },  //0x150226b4 ~ 0x150226c8
   {DIP_A_UFD,              0x1040, 8  },  //0x15023040 ~ 0x1502305C
   {DIP_A_PGN,              0x1440, 6  },  //0x15023440~0x15023454
   {DIP_A_SL2,              0x15C0, 12 },  //0x150235C0~0x150235EC
   {DIP_A_UDM,              0x1540, 23 },  //0x15023540~0x15023598
   {DIP_A_G2G,              0x2080, 8  },  //0x15024080~0x1502409C
   {DIP_A_MFB,              0x3100, 23 },  //0x15025100~0x15025158
   {DIP_A_FLC,              0x2000, 7  },  //0x15024000~0x15024018
   {DIP_A_FLC2,             0x2040, 7  },  //0x15024040~0x15024058
   {DIP_A_G2C,              0x3240, 13 },  //0x15025240~0x15025270
   {DIP_A_C42,              0x3040, 1  },  //0x15025040
   {DIP_A_SRZ3,             0x4940, 9  },  //0x15026940~0x15026960
   {DIP_A_SRZ4,             0x3880, 9  },  //0x15025880~0x150258A0
   {DIP_A_SRZ5,             0x3180, 9  },  //0x15025180~0x150251A0
   {DIP_A_SEEE,             0x46c0, 37 },  //0x150266C0~0x15026750
   {DIP_A_NDG2,             0x4980, 6  },  //0x15026980~0x15026994
   {DIP_A_NR3D,             0x5380, 39 },  //0x15027380~0x15027418
   {DIP_A_SL2B,             0x3800, 12 },  //0x15025800~0x1502582C
   {DIP_A_SL2C,             0x4900, 12 },  //0x15026900~0x1502692C
   {DIP_A_SRZ1,             0x3900, 9  },  //0x15025900~0x15025920
   {DIP_A_SRZ2,             0x4680, 9  },  //0x15026680~0x150266A0
   {DIP_A_CRZ,              0x5300, 14 },  //0x15027300~0x15027334
   {DIP_A_MIX1,             0x38C0, 2  },  //0x150258C0~0x150258C4
   {DIP_A_MIX2,             0x4640, 2  },  //0x15026640~0x15026644
   {DIP_A_MIX3,             0x32C0, 2  },  //0x150252C0~0x150252C4
   {DIP_A_MIX4,             0x5540, 2  },  //0x15027540~0x15027544
   {DIP_A_SL2D,             0x4780, 12 },  //0x15026780~0x150267AC
   {DIP_A_SL2E,             0x5440, 12 },  //0x15027440~0x1502746C
   {DIP_A_MDP,              0x0d20, 2  },  //0x15022d20 ~ 0x15022d24
   {DIP_A_ANR2,             0x4800, 60 },  //0x15026800~0x150268EC
   {DIP_A_UNP,              0x1000, 1  },  //0x15023000
   {DIP_A_C02,              0x3080, 3  },  //0x15025080~0x15025088
   {DIP_A_FE,               0x3940, 5  },  //0x15025940~0x15025950
   {DIP_A_CRSP,             0x5480, 6  },  //0x15027480~0x15027494
   {DIP_A_C02B,             0x30C0, 3  },  //0x150250C0~0x150250C8
   {DIP_A_C24,              0x3000, 1  },  //0x15025000
   {DIP_A_C24B,             0x54c0, 1  },  //0x150274C0
   {DIP_A_LCE,              0x2100, 29 },  //0x15024100~0x15024170
   {DIP_A_FM,               0x4A00, 3  },  //0x15026A00~0x15026A08
   {DIP_A_GGM,              0x2180, 193},  //0x15024180~0x15024480
   {DIP_A_PCA,              0x4000, 391},  //0x15026000~0x15026618
   {DIP_A_OBC2,             0x11c0, 8  },  //0x150231C0~0x150231DC
   {DIP_A_BNR2,             0x1240, 32 },  //0x15023240~0x150232BC
   {DIP_A_RMG2,             0x1200, 3  },  //0x15023200~0x15023208
   {DIP_A_RMM2,             0x12C0, 9  },  //0x150232C0~0x150232E0
   {DIP_A_RCP2,             0x1f60, 2  },  //0x15023f60 ~ 0x15023f64
   {DIP_A_ADBS2,            0x1600, 21 },  //0x15023600~0x15023650
   {DIP_A_DCPN2,            0x1680, 8  },  //0x15023680~0x1502369C
   {DIP_A_CPN2,             0x16C0, 16 },  //0x150236C0~0x150236FC
   {DIP_A_LSC2,             0x1340, 11 },  //0x15023340~0x15023368
   {DIP_A_DBS2,             0x1140, 13 },  //0x15023140~0x15023170
   {DIP_A_ANR,              0x3300, 314},  //0x15025300~0x150257E4
   {DIP_A_SL2G,             0x1180, 12 },  //0x15023180~0x150231AC
   {DIP_A_SL2H,             0x1500, 12 },  //0x15023500~0x1502352C
   {DIP_A_HLR2,             0x2580, 1  },  //0x15024580 ~ 0x15024580
   {DIP_A_HLR2_1,           0x2588, 10 },  //0x15024588 ~ 0x150245Ac
   {DIP_A_RNR,              0x14C0, 16 },  //0x150234C0~0x150234FC
   {DIP_A_HFG,              0x5280, 14 },  //0x15027280~0x150272B4
   {DIP_A_SL2I,             0x52C0, 12 },  //0x150272C0~0x150272EC
   {DIP_A_SL2K,             0x1400, 12 },  //0x15023400~0x1502342C
   {DIP_A_G2G2,             0x20C0, 8  },  //0x150240C0~0x150240DC
   {DIP_A_GGM2,             0x2640, 193},  //0x15024640~0x15024940
   {DIP_A_WSHIFT,           0x2B00, 5  },  //0x15024B00~0x15024B10
   {DIP_A_WSYNC,            0x2B40, 5  },  //0x15024B40~0x15024B50
   {DIP_A_COLOR,            0x5000, 155},  //0x15027000~0x15027268
   {DIP_A_NDG,              0x3840, 6  },  //0x15025840~0x15025854
   {DIP_A_END_,             0x0000, 1}
};

///////////////////////////////////////////////////////////
/**
dip initial setting
*/
#define ISP_DIP_INIT_SETTING_COUNT  117
static ISP_DRV_REG_IO_STRUCT mIspDipInitReg[ISP_DIP_INIT_SETTING_COUNT]
    =  {{DIP_HW_A, 0x0020, 0x0},   //DIP_X_REG_CTL_INT_EN
        {DIP_HW_A, 0x0024, 0x0},   //DIP_X_REG_CTL_CQ_INT_EN
        {DIP_HW_A, 0x0028, 0x0},   //DIP_X_REG_CTL_CQ_INT2_EN
        {DIP_HW_A, 0x002C, 0x0},   //DIP_X_REG_CTL_CQ_INT3_EN
        {DIP_HW_A, 0x0204, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR0_CTL
        {DIP_HW_A, 0x0210, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR1_CTL
        {DIP_HW_A, 0x021C, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR2_CTL
        {DIP_HW_A, 0x0228, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR3_CTL
        {DIP_HW_A, 0x0234, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR4_CTL
        {DIP_HW_A, 0x0240, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR5_CTL
        {DIP_HW_A, 0x024C, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR6_CTL
        {DIP_HW_A, 0x0258, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR7_CTL
        {DIP_HW_A, 0x0264, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR8_CTL
        {DIP_HW_A, 0x0270, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR9_CTL
        {DIP_HW_A, 0x027C, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR10_CTL
        {DIP_HW_A, 0x0288, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR11_CTL
        {DIP_HW_A, 0x0294, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR12_CTL
        {DIP_HW_A, 0x02A0, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR13_CTL
        {DIP_HW_A, 0x02AC, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR14_CTL
        {DIP_HW_A, 0x02B8, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR15_CTL
        {DIP_HW_A, 0x02C4, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},	  //DIP_X_CQ_THR16_CTL
        {DIP_HW_A, 0x02D0, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR17_CTL
        {DIP_HW_A, 0x02DC, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR18_CTL
        {DIP_HW_A, 0x020C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR0_DESC_SIZE
        {DIP_HW_A, 0x0218, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR1_DESC_SIZE
        {DIP_HW_A, 0x0224, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR2_DESC_SIZE
        {DIP_HW_A, 0x0230, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR3_DESC_SIZE
        {DIP_HW_A, 0x023C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR4_DESC_SIZE
        {DIP_HW_A, 0x0248, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR5_DESC_SIZE
        {DIP_HW_A, 0x0254, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR6_DESC_SIZE
        {DIP_HW_A, 0x0260, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR7_DESC_SIZE
        {DIP_HW_A, 0x026C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR8_DESC_SIZE
        {DIP_HW_A, 0x0278, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR9_DESC_SIZE
        {DIP_HW_A, 0x0284, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR10_DESC_SIZE
        {DIP_HW_A, 0x0290, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR11_DESC_SIZE
        {DIP_HW_A, 0x029C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR12_DESC_SIZE
        {DIP_HW_A, 0x02A8, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR13_DESC_SIZE
        {DIP_HW_A, 0x02B4, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR14_DESC_SIZE
        {DIP_HW_A, 0x02C0, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR15_DESC_SIZE
        {DIP_HW_A, 0x02CC, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR16_DESC_SIZE
        {DIP_HW_A, 0x02D8, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR17_DESC_SIZE
        {DIP_HW_A, 0x02E4, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_CQ_THR18_DESC_SIZE
        {DIP_HW_A, 0x34C, 0x80000040},    //DIP_X_IMG2O_CON
        {DIP_HW_A, 0x350, 0x00400040},    //DIP_X_IMG2O_CON2
        {DIP_HW_A, 0x354, 0x00150015},    //DIP_X_IMG2O_CON3
        {DIP_HW_A, 0x37C, 0x80000040},    //DIP_X_IMG2BO_CON
        {DIP_HW_A, 0x380, 0x00400040},    //DIP_X_IMG2bO_CON2
        {DIP_HW_A, 0x384, 0x00150015},    //DIP_X_IMG2bO_CON3
        {DIP_HW_A, 0x3AC, 0x80000040},    //DIP_X_IMG3O_CON
        {DIP_HW_A, 0x3B0, 0x00400040},    //DIP_X_IMG3O_CON2
        {DIP_HW_A, 0x3B4, 0x00150015},    //DIP_X_IMG3O_CON3
        {DIP_HW_A, 0x3DC, 0x80000020},    //DIP_X_IMG3BO_CON
        {DIP_HW_A, 0x3E0, 0x00200020},    //DIP_X_IMG3BO_CON2
        {DIP_HW_A, 0x3E4, 0x000A000A},    //DIP_X_IMG3BO_CON3
        {DIP_HW_A, 0x40C, 0x80000020},    //DIP_X_IMG3CO_CON
        {DIP_HW_A, 0x410, 0x00200020},    //DIP_X_IMG3CO_CON2
        {DIP_HW_A, 0x414, 0x000A000A},    //DIP_X_IMG3CO_CON3
        {DIP_HW_A, 0x43C, 0x80000040},    //DIP_X_FEO_CON
        {DIP_HW_A, 0x440, 0x00400040},    //DIP_X_FEO_CON2
        {DIP_HW_A, 0x444, 0x00150015},    //DIP_X_FEO_CON3
        {DIP_HW_A, 0x46C, 0x80000040},    //DIP_X_PAK2O_CON
        {DIP_HW_A, 0x470, 0x00400040},    //DIP_X_PAK2O_CON2
        {DIP_HW_A, 0x474, 0x00150015},    //DIP_X_PAK2O_CON3
        {DIP_HW_A, 0x51C, 0x80000080},    //DIP_X_IMGI_CON
        {DIP_HW_A, 0x520, 0x00800080},    //DIP_X_IMGI_CON2
        {DIP_HW_A, 0x524, 0x002A002A},    //DIP_X_IMGI_CON3
        {DIP_HW_A, 0x54C, 0x80000040},    //DIP_X_IMGBI_CON
        {DIP_HW_A, 0x550, 0x00400040},    //DIP_X_IMGBI_CON2
        {DIP_HW_A, 0x554, 0x00150015},    //DIP_X_IMGBI_CON3
        {DIP_HW_A, 0x57C, 0x80000040},    //DIP_X_IMGCI_CON
        {DIP_HW_A, 0x580, 0x00400040},    //DIP_X_IMGCI_CON2
        {DIP_HW_A, 0x584, 0x00150015},    //DIP_X_IMGCI_CON3
        {DIP_HW_A, 0x5AC, 0x80000080},    //DIP_X_VIPI_CON
        {DIP_HW_A, 0x5B0, 0x00800080},    //DIP_X_VIPI_CON2
        {DIP_HW_A, 0x5B4, 0x002A002A},    //DIP_X_VIPI_CON3
        {DIP_HW_A, 0x5DC, 0x80000040},    //DIP_X_VIP2I_CON
        {DIP_HW_A, 0x5E0, 0x00400040},    //DIP_X_VIP2I_CON2
        {DIP_HW_A, 0x5E4, 0x00150015},    //DIP_X_VIP2I_CON3
        {DIP_HW_A, 0x60C, 0x80000040},    //DIP_X_VIP3I_CON
        {DIP_HW_A, 0x610, 0x00400040},    //DIP_X_VIP3I_CON2
        {DIP_HW_A, 0x614, 0x00150015},    //DIP_X_VIP3I_CON3
        {DIP_HW_A, 0x63C, 0x80000020},    //DIP_X_DMGI_CON
        {DIP_HW_A, 0x640, 0x00200020},    //DIP_X_DMGI_CON2
        {DIP_HW_A, 0x644, 0x000A000A},    //DIP_X_DMGI_CON3
        {DIP_HW_A, 0x66C, 0x80000020},    //DIP_X_DEPI_CON
        {DIP_HW_A, 0x670, 0x00200020},    //DIP_X_DEPI_CON2
        {DIP_HW_A, 0x674, 0x000A000A},    //DIP_X_DEPI_CON3
        {DIP_HW_A, 0x69C, 0x80000020},    //DIP_X_LCEI_CON
        {DIP_HW_A, 0x6A0, 0x00200020},    //DIP_X_LCEI_CON2
        {DIP_HW_A, 0x6A4, 0x000A000A},    //DIP_X_LCEI_CON3
        {DIP_HW_A, 0x6CC, 0x80000020},    //DIP_X_UFDI_CON
        {DIP_HW_A, 0x6D0, 0x00200020},    //DIP_X_UFDI_CON2
        {DIP_HW_A, 0x6D4, 0x000A000A},    //DIP_X_UFDI_CON3
        {DIP_HW_A, 0x7EC, 0x80000040},    //DIP_X_SMX1O_CON
        {DIP_HW_A, 0x7F0, 0x00400040},    //DIP_X_SMX1O_CON2
        {DIP_HW_A, 0x7F4, 0x00400040},    //DIP_X_SMX1O_CON3
        {DIP_HW_A, 0x81C, 0x80000040},    //DIP_X_SMX2O_CON
        {DIP_HW_A, 0x820, 0x00400040},    //DIP_X_SMX2O_CON2
        {DIP_HW_A, 0x824, 0x00400040},    //DIP_X_SMX2O_CON3
        {DIP_HW_A, 0x84C, 0x80000040},    //DIP_X_SMX3O_CON
        {DIP_HW_A, 0x850, 0x00400040},    //DIP_X_SMX3O_CON2
        {DIP_HW_A, 0x854, 0x00400040},    //DIP_X_SMX3O_CON3
        {DIP_HW_A, 0x87C, 0x80000040},    //DIP_X_SMX4O_CON
        {DIP_HW_A, 0x880, 0x00400040},    //DIP_X_SMX4O_CON2
        {DIP_HW_A, 0x884, 0x00400040},    //DIP_X_SMX4O_CON3
        {DIP_HW_A, 0x8AC, 0x80000040},    //DIP_A_SMX1I_CON
        {DIP_HW_A, 0x8B0, 0x00400040},    //DIP_A_SMX1I_CON2
        {DIP_HW_A, 0x8B4, 0x00400040},    //DIP_A_SMX1I_CON3
        {DIP_HW_A, 0x8DC, 0x80000040},    //DIP_A_SMX2I_CON
        {DIP_HW_A, 0x8E0, 0x00400040},    //DIP_A_SMX2I_CON2
        {DIP_HW_A, 0x8E4, 0x00400040},    //DIP_A_SMX2I_CON3
        {DIP_HW_A, 0x90C, 0x80000040},    //DIP_A_SMX3I_CON
        {DIP_HW_A, 0x910, 0x00400040},    //DIP_A_SMX3I_CON2
        {DIP_HW_A, 0x914, 0x00400040},    //DIP_A_SMX3I_CON3
        {DIP_HW_A, 0x93C, 0x80000040},    //DIP_A_SMX4I_CON
        {DIP_HW_A, 0x940, 0x00400040},    //DIP_A_SMX4I_CON2
        {DIP_HW_A, 0x944, 0x00400040}     //DIP_A_SMX4I_CON3
       };

// for p2 tpipe dump information
#define DUMP_TPIPE_SIZE         12000
#define DUMP_TPIPE_NUM_PER_LINE 10

#define GET_MAX_CQ_DESCRIPTOR_SIZE()({\
    MUINT32 __size = 0;\
    __size = ((sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)*DIP_A_MODULE_MAX));\
    __size;\
})


#define ISP_DIP_CQ_DUMMY_BUFFER 0x1000
///////////////////////////////////////////////////////////////////////////

//fg_mode
#define DIP_FG_MODE_DISABLE  0
#define DIP_FG_MODE_ENABLE   1
//
#define CAM_MODE_FRAME  0
#define CAM_MODE_TPIPE  1
#define CAM_ISP_PIXEL_BYTE_FP               (3)

/*Chip Dependent Constanct*/
#define DIP_A_BASE_HW   0x15022000
#define DIP_REG_RANGE           (0x6000) //PAGE_SIZE*6 = 4096*6, dependent on device tree setting 
#define TPIPE_BUFFER_SIZE (65536) //Chip Dependent

#define MAX_BURST_QUE_NUM   10
#define MAX_DUP_CQ_NUM      2
#define MAX_CMDQ_RING_BUFFER_SIZE_NUM   40 //(10+2+1)*3*2.
#define DIP_RING_BUFFER_CQ_SIZE ISP_DRV_DIP_CQ_THRE2

#define TPIPE_WIDTH 256 //use mdp maximum tile size/2 to check how many tile in this diagram.
#endif //_ISP_DRV_DIP_PLATFORM_H_

