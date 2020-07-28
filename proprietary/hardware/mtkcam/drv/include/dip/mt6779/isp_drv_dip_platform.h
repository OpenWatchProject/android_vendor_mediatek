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
#define MX_SMXIO2_BTYES 3 //Bus size 20bits
#define MX_SMXIO3_BTYES 3 //Bus size 20bits
#define MX_SMXIO4_BTYES 2 //Bus size 10 bits
#define MX_SMXIO_HEIGHT 8000 //The maximum value is 7656 (32M)

#define SMX_BUF_ALIGNMENT 4

//32M 4:3 6632x4976 - 16:9 7656x4312
//25M 4:3 5888x4416 - 16:9 6800x3824
#define MAX_SMX1_BUF_SIZE ((MX_SMXIO_PIXEL*MX_SMXIO1_BTYES*MX_SMXIO_HEIGHT)+SMX_BUF_ALIGNMENT)*2  //10.24M
#define MAX_SMX2_BUF_SIZE ((MX_SMXIO_PIXEL*MX_SMXIO2_BTYES*MX_SMXIO_HEIGHT)+SMX_BUF_ALIGNMENT)*2  //6.144M
#define MAX_SMX3_BUF_SIZE ((MX_SMXIO_PIXEL*MX_SMXIO3_BTYES*MX_SMXIO_HEIGHT)+SMX_BUF_ALIGNMENT)*2  //6.144M
#define MAX_SMX4_BUF_SIZE ((MX_SMXIO_PIXEL*MX_SMXIO4_BTYES*MX_SMXIO_HEIGHT)+SMX_BUF_ALIGNMENT)*2  //4.096

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
//DIPCTL_D1A_DIPCTL_RGB_EN1, reg:0x15022010
#define DIPCTL_RGB_EN1_UNP_D1                   (1UL<<0)
#define DIPCTL_RGB_EN1_UFD_D1                   (1UL<<1)
#define DIPCTL_RGB_EN1_SMT_D1                   (1UL<<2)
#define DIPCTL_RGB_EN1_PAKG_D1                  (1UL<<3)
#define DIPCTL_RGB_EN1_BPC_D1                   (1UL<<4)
#define DIPCTL_RGB_EN1_OBC_D1                   (1UL<<5)
#define DIPCTL_RGB_EN1_ZFUS_D1                  (1UL<<6)
#define DIPCTL_RGB_EN1_DGN_D1                   (1UL<<7)
#define DIPCTL_RGB_EN1_LSC_D1                   (1UL<<8)
#define DIPCTL_RGB_EN1_WB_D1                    (1UL<<9)
#define DIPCTL_RGB_EN1_HLR_D1                   (1UL<<10)
#define DIPCTL_RGB_EN1_LTM_D1                   (1UL<<11)
#define DIPCTL_RGB_EN1_DM_D1                    (1UL<<12)
#define DIPCTL_RGB_EN1_LDNR_D1                  (1UL<<13)
#define DIPCTL_RGB_EN1_CRP_D2                   (1UL<<14)
#define DIPCTL_RGB_EN1_PAK_D1                   (1UL<<15)
#define DIPCTL_RGB_EN1_WIF_D1                   (1UL<<16)
#define DIPCTL_RGB_EN1_WIF_D2                   (1UL<<17)
#define DIPCTL_RGB_EN1_GDR_D1                   (1UL<<18)
#define DIPCTL_RGB_EN1_ALIGN_D1                 (1UL<<19)
#define DIPCTL_RGB_EN1_SLK_D1                   (1UL<<20)
#define DIPCTL_RGB_EN1_SLK_D6                   (1UL<<21)
#define DIPCTL_RGB_EN1_WIF_D3                   (1UL<<22)

//DIPCTL_D1A_DIPCTL_RGB_EN2, reg:0x15022014
#define DIPCTL_RGB_EN2_FLC_D1                   (1UL<<0)
#define DIPCTL_RGB_EN2_CCM_D1                   (1UL<<1)
#define DIPCTL_RGB_EN2_GGM_D1                   (1UL<<2)
#define DIPCTL_RGB_EN2_WSYNC_D1                 (1UL<<3)
#define DIPCTL_RGB_EN2_CCM_D2                   (1UL<<4)
#define DIPCTL_RGB_EN2_GGM_D2                   (1UL<<5)
#define DIPCTL_RGB_EN2_SMT_D4                   (1UL<<6)
#define DIPCTL_RGB_EN2_UNP_D4                   (1UL<<7)
#define DIPCTL_RGB_EN2_PAK_D4                   (1UL<<8)
#define DIPCTL_RGB_EN2_MCRP_D2                  (1UL<<9)
#define DIPCTL_RGB_EN2_GGM_D4                   (1UL<<10)
#define DIPCTL_RGB_EN2_G2C_D4                   (1UL<<11)
#define DIPCTL_RGB_EN2_C42_D4                   (1UL<<12)
#define DIPCTL_RGB_EN2_CRP_D4                   (1UL<<13)
#define DIPCTL_RGB_EN2_SW_P2_IDENDITY_EN        (1UL<<31)

//DIPCTL_D1A_DIPCTL_YUV_EN1, reg:0x15022018
#define DIPCTL_YUV_EN1_G2CX_D1                  (1UL<<0)
#define DIPCTL_YUV_EN1_C42_D1                   (1UL<<1)
#define DIPCTL_YUV_EN1_MIX_D3                   (1UL<<2)
#define DIPCTL_YUV_EN1_YNR_D1                   (1UL<<3)
#define DIPCTL_YUV_EN1_NDG_D1                   (1UL<<4)
#define DIPCTL_YUV_EN1_MIX_D1                   (1UL<<5)
#define DIPCTL_YUV_EN1_C24_D3                   (1UL<<6)
#define DIPCTL_YUV_EN1_C2G_D1                   (1UL<<7)
#define DIPCTL_YUV_EN1_IGGM_D1                  (1UL<<8)
#define DIPCTL_YUV_EN1_CCM_D3                   (1UL<<9)
#define DIPCTL_YUV_EN1_LCE_D1                   (1UL<<10)
#define DIPCTL_YUV_EN1_GGM_D3                   (1UL<<11)
#define DIPCTL_YUV_EN1_DCE_D1                   (1UL<<12)
#define DIPCTL_YUV_EN1_DCES_D1                  (1UL<<13)
#define DIPCTL_YUV_EN1_G2C_D1                   (1UL<<14)
#define DIPCTL_YUV_EN1_C42_D2                   (1UL<<15)
#define DIPCTL_YUV_EN1_EE_D1                    (1UL<<16)
#define DIPCTL_YUV_EN1_SMT_D2                   (1UL<<17)
#define DIPCTL_YUV_EN1_UNP_D2                   (1UL<<18)
#define DIPCTL_YUV_EN1_PAK_D2                   (1UL<<19)
#define DIPCTL_YUV_EN1_CNR_D1                   (1UL<<20)
#define DIPCTL_YUV_EN1_NDG_D2                   (1UL<<21)
#define DIPCTL_YUV_EN1_SMT_D3                   (1UL<<22)
#define DIPCTL_YUV_EN1_UNP_D3                   (1UL<<23)
#define DIPCTL_YUV_EN1_PAK_D3                   (1UL<<24)
#define DIPCTL_YUV_EN1_COLOR_D1                 (1UL<<25)
#define DIPCTL_YUV_EN1_MIX_D2                   (1UL<<26)
#define DIPCTL_YUV_EN1_MIX_D4                   (1UL<<27)
#define DIPCTL_YUV_EN1_CRP_D1                   (1UL<<28)

//DIPCTL_D1A_DIPCTL_YUV_EN2 reg:0x1502201C
#define DIPCTL_YUV_EN2_NR3D_D1                  (1UL<<0)
#define DIPCTL_YUV_EN2_CRSP_D1                  (1UL<<1)
#define DIPCTL_YUV_EN2_PLNW_D1                  (1UL<<2)
#define DIPCTL_YUV_EN2_PAK_D6                   (1UL<<3)
#define DIPCTL_YUV_EN2_PAK_D7                   (1UL<<4)
#define DIPCTL_YUV_EN2_PAK_D8                   (1UL<<5)
#define DIPCTL_YUV_EN2_C24_D2                   (1UL<<6)
#define DIPCTL_YUV_EN2_MCRP_D1                  (1UL<<7)
#define DIPCTL_YUV_EN2_CRZ_D1                   (1UL<<8)
#define DIPCTL_YUV_EN2_PLNW_D2                  (1UL<<9)
#define DIPCTL_YUV_EN2_SRZ_D1                   (1UL<<10)
#define DIPCTL_YUV_EN2_SRZ_D3                   (1UL<<11)
#define DIPCTL_YUV_EN2_SRZ_D4                   (1UL<<12)
#define DIPCTL_YUV_EN2_UNP_D6                   (1UL<<13)
#define DIPCTL_YUV_EN2_UNP_D7                   (1UL<<14)
#define DIPCTL_YUV_EN2_UNP_D8                   (1UL<<15)
#define DIPCTL_YUV_EN2_PLNR_D2                  (1UL<<16)
#define DIPCTL_YUV_EN2_C02_D2                   (1UL<<17)
#define DIPCTL_YUV_EN2_C24_D1                   (1UL<<18)
#define DIPCTL_YUV_EN2_UNP_D9                   (1UL<<19)
#define DIPCTL_YUV_EN2_UNP_D10                  (1UL<<20)
#define DIPCTL_YUV_EN2_UNP_D11                  (1UL<<21)
#define DIPCTL_YUV_EN2_PLNR_D1                  (1UL<<22)
#define DIPCTL_YUV_EN2_C02_D1                   (1UL<<23)
#define DIPCTL_YUV_EN2_SLK_D2                   (1UL<<24)
#define DIPCTL_YUV_EN2_SLK_D3                   (1UL<<25)
#define DIPCTL_YUV_EN2_SLK_D4                   (1UL<<26)
#define DIPCTL_YUV_EN2_SLK_D5                   (1UL<<27)
#define DIPCTL_YUV_EN2_FM_D1                    (1UL<<28)
#define DIPCTL_YUV_EN2_LPCNR_D1                 (1UL<<29)
#define DIPCTL_YUV_EN2_BS_D1                    (1UL<<30)
#define DIPCTL_YUV_EN2_DFE_D1                   (1UL<<31)

//DIPCTL_D1A_DIPCTL_DMA_EN1 reg:0x15022020
#define DIPCTL_DMA_EN1_IMGI_D1                  (1UL<<0)
#define DIPCTL_DMA_EN1_IMGBI_D1                 (1UL<<1)
#define DIPCTL_DMA_EN1_IMGCI_D1                 (1UL<<2)
#define DIPCTL_DMA_EN1_CPI_D1                   (1UL<<3)
#define DIPCTL_DMA_EN1_CPBI_D1                  (1UL<<4)
#define DIPCTL_DMA_EN1_CRZO_D1                  (1UL<<5)
#define DIPCTL_DMA_EN1_CRZBO_D1                 (1UL<<6)
#define DIPCTL_DMA_EN1_SMTI_D1                  (1UL<<7)
#define DIPCTL_DMA_EN1_SMTI_D2                  (1UL<<8)
#define DIPCTL_DMA_EN1_SMTI_D3                  (1UL<<9)
#define DIPCTL_DMA_EN1_SMTI_D4                  (1UL<<10)
#define DIPCTL_DMA_EN1_SMTO_D1                  (1UL<<11)
#define DIPCTL_DMA_EN1_SMTO_D2                  (1UL<<12)
#define DIPCTL_DMA_EN1_SMTO_D3                  (1UL<<13)
#define DIPCTL_DMA_EN1_SMTO_D4                  (1UL<<14)
#define DIPCTL_DMA_EN1_TIMGO_D1                 (1UL<<15)
#define DIPCTL_DMA_EN1_UFDI_D1                  (1UL<<16)
#define DIPCTL_DMA_EN1_DMGI_D1                  (1UL<<17)
#define DIPCTL_DMA_EN1_DEPI_D1                  (1UL<<18)
#define DIPCTL_DMA_EN1_LCEI_D1                  (1UL<<19)
#define DIPCTL_DMA_EN1_DCESO_D1                 (1UL<<20)
#define DIPCTL_DMA_EN1_FEO_D1                   (1UL<<21)
#define DIPCTL_DMA_EN1_ADL_D1                   (1UL<<22)

#define DIPCTL_DMA_EN1_WROTO                    (1UL<<28)
#define DIPCTL_DMA_EN1_WDMAO                    (1UL<<29)
#define DIPCTL_DMA_EN1_JPEGO                    (1UL<<30)
#define DIPCTL_DMA_EN1_VENCO                    (1UL<<31)

//DIPCTL_D1A_DIPCTL_DMA_EN2 reg:0x15022024
#define DIPCTL_DMA_EN2_VIPI_D1                  (1UL<<0)
#define DIPCTL_DMA_EN2_VIPBI_D1                 (1UL<<1)
#define DIPCTL_DMA_EN2_VIPCI_D1                 (1UL<<2)
#define DIPCTL_DMA_EN2_IMG3O_D1                 (1UL<<3)
#define DIPCTL_DMA_EN2_IMG3BO_D1                (1UL<<4)
#define DIPCTL_DMA_EN2_IMG3CO_D1                (1UL<<5)
#define DIPCTL_DMA_EN2_CPI_D2                   (1UL<<6)
#define DIPCTL_DMA_EN2_CPBI_D2                  (1UL<<7)
#define DIPCTL_DMA_EN2_CPO_D1                   (1UL<<8)
#define DIPCTL_DMA_EN2_CPBO_D1                  (1UL<<9)
#define DIPCTL_DMA_EN2_OFT_D1                   (1UL<<10)
#define DIPCTL_DMA_EN2_OFTL_D1                  (1UL<<11)

//CQ_D1A_CQ_THR0_CTL ~ CQ_D1A_CQ_THR18_CTL reg:0x15022204 ~~
#define CQ_D1A_THRX_CTL_EN                      (1L<<0)
#define CQ_D1A_THRX_CTL_MODE                    (1L<<4)

//DIPCTL_D1A_DIPCTL_CQ_INT1_EN reg:0x150220D0
#define DIPCTL_CQ_INT1_EN_THR0                  (1UL<<0)
#define DIPCTL_CQ_INT1_EN_THR1                  (1UL<<1)
#define DIPCTL_CQ_INT1_EN_THR2                  (1UL<<2)
#define DIPCTL_CQ_INT1_EN_THR3                  (1UL<<3)
#define DIPCTL_CQ_INT1_EN_THR4                  (1UL<<4)
#define DIPCTL_CQ_INT1_EN_THR5                  (1UL<<5)
#define DIPCTL_CQ_INT1_EN_THR6                  (1UL<<6)
#define DIPCTL_CQ_INT1_EN_THR7                  (1UL<<7)
#define DIPCTL_CQ_INT1_EN_THR8                  (1UL<<8)
#define DIPCTL_CQ_INT1_EN_THR9                  (1UL<<9)
#define DIPCTL_CQ_INT1_EN_THR10                 (1UL<<10)
#define DIPCTL_CQ_INT1_EN_THR11                 (1UL<<11)
#define DIPCTL_CQ_INT1_EN_THR12                 (1UL<<12)
#define DIPCTL_CQ_INT1_EN_THR13                 (1UL<<13)
#define DIPCTL_CQ_INT1_EN_THR14                 (1UL<<14)
#define DIPCTL_CQ_INT1_EN_THR15                 (1UL<<15)
#define DIPCTL_CQ_INT1_EN_THR16                 (1UL<<16)
#define DIPCTL_CQ_INT1_EN_THR17                 (1UL<<17)
#define DIPCTL_CQ_INT1_EN_THR18                 (1UL<<18)
#define DIPCTL_CQ_INT1_EN_VB_ERR                (1UL<<27)
#define DIPCTL_CQ_INT1_EN_TRIG_ERR              (1UL<<28)
#define DIPCTL_CQ_INT1_EN_DON                   (1UL<<29)
#define DIPCTL_CQ_INT1_EN_CODE_ERR              (1UL<<30)
#define DIPCTL_CQ_INT1_EN_APB_OUT_RANGE         (1UL<<31)

//DIPCTL_D1A_DIPCTL_TDR_CTL reg:0x15022060
#define DIPCTL_TDR_CTL_TDR_EN                   (1UL<<0)

//DIPCTL_D1A_DIPCTL_TDR_SEL reg:0x15022064
#define DIPCTL_TDR_SEL_CTL_EXT_EN               (1UL<<2)
#define DIPCTL_TDR_SEL_SOF_RST_EN               (1UL<<4)
#define DIPCTL_TDR_SEL_SIZE_DET_EN              (1UL<<6)
#define DIPCTL_TDR_SEL_LAST_TILE                (1UL<<8)
#define DIPCTL_TDR_SEL_TILE_IRQ                 (1UL<<10)
//YWadd #define DIPCTL_TDR_SEL_TILE_EDGE
//YWadd #define DIPCTL_TDR_SEL_CRZ_EDGE

#define DIPCTL_IMGI_D1_V_FLIP_EN                 (1UL<<0)
#define DIPCTL_UFDI_D1_V_FLIP_EN                 (1UL<<1)
#define DIPCTL_SMTI_D1_V_FLIP_EN                 (1UL<<2)
#define DIPCTL_SMTO_D1_V_FLIP_EN                 (1UL<<3)
#define DIPCTL_IMGBI_D1_V_FLIP_EN                (1UL<<4)
#define DIPCTL_IMGCI_D1_V_FLIP_EN                (1UL<<5)
#define DIPCTL_TIMGO_D1_V_FLIP_EN                (1UL<<6)
#define DIPCTL_SMTI_D4_V_FLIP_EN                 (1UL<<7)
#define DIPCTL_SMTO_D4_V_FLIP_EN                 (1UL<<8)
#define DIPCTL_DMGI_D1_V_FLIP_EN                 (1UL<<9)
#define DIPCTL_DEPI_D1_V_FLIP_EN                 (1UL<<10)
#define DIPCTL_LCEI_D1_V_FLIP_EN                 (1UL<<11)
#define DIPCTL_DCESO_D1_V_FLIP_EN                (1UL<<12)
#define DIPCTL_SMTI_D2_V_FLIP_EN                 (1UL<<13)
#define DIPCTL_SMTO_D2_V_FLIP_EN                 (1UL<<14)
#define DIPCTL_SMTI_D3_V_FLIP_EN                 (1UL<<15)
#define DIPCTL_SMTO_D3_V_FLIP_EN                 (1UL<<16)
#define DIPCTL_CRZO_D1_V_FLIP_EN                 (1UL<<17)
#define DIPCTL_CRZBO_D1_V_FLIP_EN                (1UL<<18)
#define DIPCTL_FEO_D1_V_FLIP_EN                  (1UL<<19)

#define DIPCTL_VIPI_D1_V_FLIP_EN                 (1UL<<20)
#define DIPCTL_VIPBI_D1_V_FLIP_EN                (1UL<<21)
#define DIPCTL_VIPCI_D1_V_FLIP_EN                (1UL<<22)
#define DIPCTL_IMG3O_D1_V_FLIP_EN                (1UL<<23)
#define DIPCTL_IMG3BO_D1_V_FLIP_EN               (1UL<<24)
#define DIPCTL_IMG3CO_D1_V_FLIP_EN               (1UL<<25)
#if 0
//DIPCTL_D1A_DIPCTL_TDR_TCM_EN1 reg:0x15022068
#define DIPCTL_TDR_TCM_EN1_UNP_D1               (1UL<<0)
#define DIPCTL_TDR_TCM_EN1_UFD_D1               (1UL<<1)
#define DIPCTL_TDR_TCM_EN1_SMT_D1               (1UL<<2)
#define DIPCTL_TDR_TCM_EN1_PAKG_D1              (1UL<<3)
#define DIPCTL_TDR_TCM_EN1_BPC_D1               (1UL<<4)
#define DIPCTL_TDR_TCM_EN1_OBC_D1               (1UL<<5)
#define DIPCTL_TDR_TCM_EN1_ZFUS_D1              (1UL<<6)
#define DIPCTL_TDR_TCM_EN1_DGN_D1               (1UL<<7)
#define DIPCTL_TDR_TCM_EN1_LSC_D1               (1UL<<8)
#define DIPCTL_TDR_TCM_EN1_WB_D1                (1UL<<9)
#define DIPCTL_TDR_TCM_EN1_HLR_D1               (1UL<<10)
#define DIPCTL_TDR_TCM_EN1_LTM_D1               (1UL<<11)
#define DIPCTL_TDR_TCM_EN1_DM_D1                (1UL<<12)
#define DIPCTL_TDR_TCM_EN1_LDNR_D1              (1UL<<13)
#define DIPCTL_TDR_TCM_EN1_CRP_D2               (1UL<<14)
#define DIPCTL_TDR_TCM_EN1_PAK_D1               (1UL<<15)
#define DIPCTL_TDR_TCM_EN1_WIF_D1               (1UL<<16)
#define DIPCTL_TDR_TCM_EN1_WIF_D2               (1UL<<17)
#define DIPCTL_TDR_TCM_EN1_GDR_D1               (1UL<<18)
#define DIPCTL_TDR_TCM_EN1_ALIGN_D1             (1UL<<19)
#define DIPCTL_TDR_TCM_EN1_SLK_D1               (1UL<<20)
#define DIPCTL_TDR_TCM_EN1_SLK_D6               (1UL<<21)
#define DIPCTL_TDR_TCM_EN1_WIF_D3               (1UL<<22)

//DIPCTL_D1A_DIPCTL_TDR_TCM_EN2 reg:0x1502206C
#define DIPCTL_TDR_TCM_EN2_FLC_D1               (1UL<<0)
#define DIPCTL_TDR_TCM_EN2_CCM_D1               (1UL<<1)
#define DIPCTL_TDR_TCM_EN2_GGM_D1               (1UL<<2)
#define DIPCTL_TDR_TCM_EN2_WSYNC_D1             (1UL<<3)
#define DIPCTL_TDR_TCM_EN2_CCM_D2               (1UL<<4)
#define DIPCTL_TDR_TCM_EN2_GGM_D2               (1UL<<5)
#define DIPCTL_TDR_TCM_EN2_SMT_D4               (1UL<<6)
#define DIPCTL_TDR_TCM_EN2_UNP_D4               (1UL<<7)
#define DIPCTL_TDR_TCM_EN2_PAK_D4               (1UL<<8)
#define DIPCTL_TDR_TCM_EN2_MCRP_D2              (1UL<<9)
#define DIPCTL_TDR_TCM_EN2_GGM_D4               (1UL<<10)
#define DIPCTL_TDR_TCM_EN2_G2C_D4               (1UL<<11)
#define DIPCTL_TDR_TCM_EN2_C42_D4               (1UL<<12)
#define DIPCTL_TDR_TCM_EN2_CRP_D4               (1UL<<13)

//DIPCTL_D1A_DIPCTL_TDR_TCM_EN3 reg:0x15022070
#define DIPCTL_TDR_TCM_EN3_G2CX_D1              (1UL<<0)
#define DIPCTL_TDR_TCM_EN3_C42_D1               (1UL<<1)
#define DIPCTL_TDR_TCM_EN3_MIX_D3               (1UL<<2)
#define DIPCTL_TDR_TCM_EN3_YNR_D1               (1UL<<3)
#define DIPCTL_TDR_TCM_EN3_NDG_D1               (1UL<<4)
#define DIPCTL_TDR_TCM_EN3_MIX_D1               (1UL<<5)
#define DIPCTL_TDR_TCM_EN3_C24_D3               (1UL<<6)
#define DIPCTL_TDR_TCM_EN3_C2G_D1               (1UL<<7)
#define DIPCTL_TDR_TCM_EN3_IGGM_D1              (1UL<<8)
#define DIPCTL_TDR_TCM_EN3_CCM_D3               (1UL<<9)
#define DIPCTL_TDR_TCM_EN3_LCE_D1               (1UL<<10)
#define DIPCTL_TDR_TCM_EN3_GGM_D3               (1UL<<11)
#define DIPCTL_TDR_TCM_EN3_DCE_D1               (1UL<<12)
#define DIPCTL_TDR_TCM_EN3_DCES_D1              (1UL<<13)
#define DIPCTL_TDR_TCM_EN3_G2C_D1               (1UL<<14)
#define DIPCTL_TDR_TCM_EN3_C42_D2               (1UL<<15)
#define DIPCTL_TDR_TCM_EN3_EE_D1                (1UL<<16)
#define DIPCTL_TDR_TCM_EN3_SMT_D2               (1UL<<17)
#define DIPCTL_TDR_TCM_EN3_UNP_D2               (1UL<<18)
#define DIPCTL_TDR_TCM_EN3_PAK_D2               (1UL<<19)
#define DIPCTL_TDR_TCM_EN3_CNR_D1               (1UL<<20)
#define DIPCTL_TDR_TCM_EN3_NDG_D2               (1UL<<21)
#define DIPCTL_TDR_TCM_EN3_SMT_D3               (1UL<<22)
#define DIPCTL_TDR_TCM_EN3_UNP_D3               (1UL<<23)
#define DIPCTL_TDR_TCM_EN3_PAK_D3               (1UL<<24)
#define DIPCTL_TDR_TCM_EN3_COLOR_D1             (1UL<<25)
#define DIPCTL_TDR_TCM_EN3_MIX_D2               (1UL<<26)
#define DIPCTL_TDR_TCM_EN3_MIX_D4               (1UL<<27)
#define DIPCTL_TDR_TCM_EN3_CRP_D1               (1UL<<28)

//DIPCTL_D1A_DIPCTL_TDR_TCM_EN4 reg:0x15022074
#define DIPCTL_TDR_TCM_EN4_NR3D_D1              (1UL<<0)
#define DIPCTL_TDR_TCM_EN4_CRSP_D1              (1UL<<1)
#define DIPCTL_TDR_TCM_EN4_PLNW_D1              (1UL<<2)
#define DIPCTL_TDR_TCM_EN4_PAK_D6               (1UL<<3)
#define DIPCTL_TDR_TCM_EN4_PAK_D7               (1UL<<4)
#define DIPCTL_TDR_TCM_EN4_PAK_D8               (1UL<<5)
#define DIPCTL_TDR_TCM_EN4_C24_D2               (1UL<<6)
#define DIPCTL_TDR_TCM_EN4_MCRP_D1              (1UL<<7)
#define DIPCTL_TDR_TCM_EN4_CRZ_D1               (1UL<<8)
#define DIPCTL_TDR_TCM_EN4_PLNW_D2              (1UL<<9)
#define DIPCTL_TDR_TCM_EN4_SRZ_D1               (1UL<<10)
#define DIPCTL_TDR_TCM_EN4_SRZ_D3               (1UL<<11)
#define DIPCTL_TDR_TCM_EN4_SRZ_D4               (1UL<<12)
#define DIPCTL_TDR_TCM_EN4_UNP_D6               (1UL<<13)
#define DIPCTL_TDR_TCM_EN4_UNP_D7               (1UL<<14)
#define DIPCTL_TDR_TCM_EN4_UNP_D8               (1UL<<15)
#define DIPCTL_TDR_TCM_EN4_PLNR_D2              (1UL<<16)
#define DIPCTL_TDR_TCM_EN4_C02_D2               (1UL<<17)
#define DIPCTL_TDR_TCM_EN4_C24_D1               (1UL<<18)
#define DIPCTL_TDR_TCM_EN4_UNP_D9               (1UL<<19)
#define DIPCTL_TDR_TCM_EN4_UNP_D10              (1UL<<20)
#define DIPCTL_TDR_TCM_EN4_UNP_D11              (1UL<<21)
#define DIPCTL_TDR_TCM_EN4_PLNR_D1              (1UL<<22)
#define DIPCTL_TDR_TCM_EN4_C02_D1               (1UL<<23)
#define DIPCTL_TDR_TCM_EN4_SLK_D2               (1UL<<24)
#define DIPCTL_TDR_TCM_EN4_SLK_D3               (1UL<<25)
#define DIPCTL_TDR_TCM_EN4_SLK_D4               (1UL<<26)
#define DIPCTL_TDR_TCM_EN4_SLK_D5               (1UL<<27)
#define DIPCTL_TDR_TCM_EN4_FM_D1                (1UL<<28)
#define DIPCTL_TDR_TCM_EN4_LPCNR_D1             (1UL<<29)
#define DIPCTL_TDR_TCM_EN4_BS_D1                (1UL<<30)
#define DIPCTL_TDR_TCM_EN4_DFE_D1               (1UL<<31)

//DIPCTL_D1A_DIPCTL_TDR_TCM_EN5 reg:0x15022078
#define DIPCTL_TDR_TCM_EN5_IMGI_D1              (1UL<<0)
#define DIPCTL_TDR_TCM_EN5_IMGBI_D1             (1UL<<1)
#define DIPCTL_TDR_TCM_EN5_IMGCI_D1             (1UL<<2)
#define DIPCTL_TDR_TCM_EN5_CPI_D1               (1UL<<3)
#define DIPCTL_TDR_TCM_EN5_CPBI_D1              (1UL<<4)
#define DIPCTL_TDR_TCM_EN5_CRZO_D1              (1UL<<5)
#define DIPCTL_TDR_TCM_EN5_CRZBO_D1             (1UL<<6)
#define DIPCTL_TDR_TCM_EN5_SMTI_D1              (1UL<<7)
#define DIPCTL_TDR_TCM_EN5_SMTI_D2              (1UL<<8)
#define DIPCTL_TDR_TCM_EN5_SMTI_D3              (1UL<<9)
#define DIPCTL_TDR_TCM_EN5_SMTI_D4              (1UL<<10)
#define DIPCTL_TDR_TCM_EN5_SMTO_D1              (1UL<<11)
#define DIPCTL_TDR_TCM_EN5_SMTO_D2              (1UL<<12)
#define DIPCTL_TDR_TCM_EN5_SMTO_D3              (1UL<<13)
#define DIPCTL_TDR_TCM_EN5_SMTO_D4              (1UL<<14)
#define DIPCTL_TDR_TCM_EN5_TIMGO_D1             (1UL<<15)
#define DIPCTL_TDR_TCM_EN5_UFDI_D1              (1UL<<16)
#define DIPCTL_TDR_TCM_EN5_DMGI_D1              (1UL<<17)
#define DIPCTL_TDR_TCM_EN5_DEPI_D1              (1UL<<18)
#define DIPCTL_TDR_TCM_EN5_LCEI_D1              (1UL<<19)
#define DIPCTL_TDR_TCM_EN5_DCESO_D1             (1UL<<20)
#define DIPCTL_TDR_TCM_EN5_FEO_D1               (1UL<<21)
#define DIPCTL_TDR_TCM_EN5_ADL_D1               (1UL<<22)

//DIPCTL_D1A_DIPCTL_TDR_TCM_EN6 reg:0x1502207C
#define DIPCTL_TDR_TCM_EN6_VIPI_D1              (1UL<<0)
#define DIPCTL_TDR_TCM_EN6_VIPBI_D1             (1UL<<1)
#define DIPCTL_TDR_TCM_EN6_VIPCI_D1             (1UL<<2)
#define DIPCTL_TDR_TCM_EN6_IMG3O_D1             (1UL<<3)
#define DIPCTL_TDR_TCM_EN6_IMG3BO_D1            (1UL<<4)
#define DIPCTL_TDR_TCM_EN6_IMG3CO_D1            (1UL<<5)
#define DIPCTL_TDR_TCM_EN6_CPI_D2               (1UL<<6)
#define DIPCTL_TDR_TCM_EN6_CPBI_D2              (1UL<<7)
#define DIPCTL_TDR_TCM_EN6_CPO_D1               (1UL<<8)
#define DIPCTL_TDR_TCM_EN6_CPBO_D1              (1UL<<9)
#define DIPCTL_TDR_TCM_EN6_OFT_D1               (1UL<<10)
#define DIPCTL_TDR_TCM_EN6_OFTL_D1              (1UL<<11)
#endif


////////////////////////////////////DMA Format///////////////////////////////
//IMGI Format
#define DIP_IMGI_FMT_YUV422_1P          0
#define DIP_IMGI_FMT_YUV422_2P          1
#define DIP_IMGI_FMT_YUV422_3P          2
#define DIP_IMGI_FMT_YUV420_2P          3
#define DIP_IMGI_FMT_YUV420_3P          4
#define DIP_IMGI_FMT_Y_ONLY             5
//no need #define DIP_IMGI_FMT_MFB_BLEND_MODE     6
//no need #define DIP_IMGI_FMT_MFB_MIX_MODE       7
#define DIP_IMGI_FMT_BAYER8             8
#define DIP_IMGI_FMT_BAYER10            9
#define DIP_IMGI_FMT_BAYER12           10
#define DIP_IMGI_FMT_BAYER14           11
#define DIP_IMGI_FMT_BAYER8_2BYTEs     12
#define DIP_IMGI_FMT_BAYER10_2BYTEs    13
#define DIP_IMGI_FMT_BAYER12_2BYTEs    14
#define DIP_IMGI_FMT_BAYER14_2BYTEs    15
#define DIP_IMGI_FMT_BAYER10_MIPI      16
#define DIP_IMGI_FMT_BAYER15_2BYTEs    17 //YWtodo
//ADAS only OFT16b (YUV8)     20
//ADAS only OFT24b (YUV12)    21
//ADAS only OFT40b (YUV10)    22
#define DIP_IMGI_FMT_RGB565            24
#define DIP_IMGI_FMT_RGB888            25
#define DIP_IMGI_FMT_XRGB8888          26
#define DIP_IMGI_FMT_RGB101010         27
#define DIP_IMGI_FMT_RGB121212         28

//UFDI Format
#define DIP_UFDI_FMT_UFO_LENGTH    0
#define DIP_UFDI_FMT_WEIGHTING     1

//DMGI Format
#define DIP_DMGI_FMT_DRZ    0
#define DIP_DMGI_FMT_FM     1

//DEPI Format
#define DIP_DEPI_FMT_DRZ    0
#define DIP_DEPI_FMT_FM     1

//VIPI Format
#define DIP_VIPI_FMT_YUV422_1P     0
#define DIP_VIPI_FMT_YUV422_2P     1
#define DIP_VIPI_FMT_YUV422_3P     2
#define DIP_VIPI_FMT_YUV420_2P     3
#define DIP_VIPI_FMT_YUV420_3P     4
#define DIP_VIPI_FMT_Y_ONLY        5
//no need #define DIP_VIPI_FMT_RGB565       24
//no need #define DIP_VIPI_FMT_RGB888       25
//no need #define DIP_VIPI_FMT_XRGB8888     26
//no need #define DIP_VIPI_FMT_RGB101010    27
//no need #define DIP_VIPI_FMT_RGB121212    28

//TIMGO Format
#define DIP_TIMGO_FMT_BAYER8             8
#define DIP_TIMGO_FMT_BAYER10            9
#define DIP_TIMGO_FMT_BAYER12           10
#define DIP_TIMGO_FMT_BAYER14           11
#define DIP_TIMGO_FMT_BAYER8_2BYTEs     12
#define DIP_TIMGO_FMT_BAYER10_2BYTEs    13
#define DIP_TIMGO_FMT_BAYER12_2BYTEs    14
#define DIP_TIMGO_FMT_BAYER14_2BYTEs    15
#define DIP_TIMGO_FMT_BAYER10_MIPI      16
#define DIP_TIMGO_FMT_BAYER15_2BYTEs    17 //YWtodo
#define DIP_TIMGO_FMT_BAYER16           18 //YWtodo
#define DIP_TIMGO_FMT_BAYER19           19 //YWtodo

//DCESO Format  //no need to sel

//CRZO Format (IMG2O)
#define DIP_CRZO_FMT_YUV422_1P    0
#define DIP_CRZO_FMT_YUV422_2P    1

//IMG3O Format
#define DIP_IMG3O_FMT_YUV422_1P    0
#define DIP_IMG3O_FMT_YUV422_2P    1
#define DIP_IMG3O_FMT_YUV422_3P    2
#define DIP_IMG3O_FMT_YUV420_2P    3
#define DIP_IMG3O_FMT_YUV420_3P    4


typedef enum{
    DIP_HW_A   = 0,
    //DIP_HW_B,            //not supported
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
    DIP_A_CTL_EN = 0,       //0x15022010 ~ 0x15022024
    DIP_A_CTL_MUX,          //0x15022040 ~ 0x15022044
    DIP_A_CTL_FMT,          //0x15022050 ~ 0x15022058
    DIP_A_CTL_TDR,          //0x15022060 ~ 0x1502207C
    DIP_A_CTL_DONE,         //0x15022030 ~ 0x15022034
    DIP_A_TDRI,             //0x15021004 ~ 0x1502100C
    DIP_A_VECTICAL_FLIP,    //0x15021010
    DIP_A_SPECIAL_FUN,      //0x1502101C
    DIP_A_DCES_DCM,         //0x15022118
    DIP_A_IMGI_D1,          //0x15021104 ~ 0x15021114
    DIP_A_UFDI_D1,          //0x15021130 ~ 0x15021144
    DIP_A_SMTI_D1,          //0x15021160 ~ 0x15021174
    DIP_A_SMTO_D1,          //0x15021190 ~ 0x150211A4
    DIP_A_SMTO_D1_CROP,     //0x150211B4
    DIP_A_IMGBI_D1,         //0x15021200 ~ 0x15021214
    DIP_A_IMGCI_D1,         //0x15021230 ~ 0x15021244
    DIP_A_TIMGO_D1,         //0x15021260 ~ 0x15021274
    DIP_A_TIMGO_D1_CROP,    //0x15021284
    DIP_A_SMTI_D4,          //0x150212D0 ~ 0x150212E4
    DIP_A_SMTO_D4,          //0x15021300 ~ 0x15021314
    DIP_A_SMTO_D4_CROP,     //0x15021324
    DIP_A_DMGI_D1,          //0x15021370 ~ 0x15021384
    DIP_A_DEPI_D1,          //0x150213A0 ~ 0x150213B4
    DIP_A_LCEI_D1,          //0x150213D0 ~ 0x150213E4
    DIP_A_DCESO_D1,         //0x15021400 ~ 0x15021414
    DIP_A_SMTI_D2,          //0x15021470 ~ 0x15021484
    DIP_A_SMTO_D2,          //0x150214A0 ~ 0x150214B4
    DIP_A_SMTO_D2_CROP,     //0x150214C4
    DIP_A_SMTI_D3,          //0x15021510 ~ 0x15021524
    DIP_A_SMTO_D3,          //0x15021540 ~ 0x15021554
    DIP_A_SMTO_D3_CROP,     //0x15021564
    DIP_A_CRZO_D1,          //0x150215B0 ~ 0x150215C4
    DIP_A_CRZO_D1_CROP,     //0x150215D4
    DIP_A_CRZBO_D1,         //0x15021620 ~ 0x15021634
    DIP_A_CRZBO_D1_CROP,    //0x15021644
    DIP_A_FEO_D1,           //0x15021690 ~ 0x150216A4
    DIP_A_VIPI_D1,          //0x15021700 ~ 0x15021714
    DIP_A_VIPBI_D1,         //0x15021730 ~ 0x15021744
    DIP_A_VIPCI_D1,         //0x15021760 ~ 0x15021774
    DIP_A_IMG3O_D1,         //0x15021790 ~ 0x150217A4
    DIP_A_IMG3O_D1_CROP,    //0x150217B4
    DIP_A_IMG3BO_D1,        //0x15021800 ~ 0x15021814
    DIP_A_IMG3BO_D1_CROP,   //0x15021824
    DIP_A_IMG3CO_D1,        //0x15021870 ~ 0x15021884
    DIP_A_IMG3CO_D1_CROP,   //0x15021894
//DIPAMD?
    DIP_A_UNP_D1,           //0x15023000 ~ 0x15023004
    DIP_A_UFD_D1,           //0x15023040 ~ 0x15023074
    DIP_A_SMT_D1,           //0x15023080 ~ 0x150230A0
    DIP_A_BPC_D1,           //0x15023100 ~ 0x150231E4
    DIP_A_OBC_D1,           //0x15023200 ~ 0x1502326C
    DIP_A_ZFUS_D1,          //0x15023280 ~ 0x15023290
    DIP_A_DGN_D1,           //0x15023300 ~ 0x15023310
    DIP_A_LSC_D1,           //0x15023340 ~ 0x15023368
    DIP_A_WB_D1,            //0x15023380 ~ 0x15023388
    DIP_A_HLR_D1,           //0x150233C0 ~ 0x15023400
    DIP_A_LTM_D1,           //0x15023440 ~ 0x1502351C
    DIP_A_CRP_D2,           //0x15023540 ~ 0x15023544
    DIP_A_PAK_D1,           //0x15023580
    DIP_A_DM_D1,            //0x15023600 ~ 0x15023664
    DIP_A_SLK_D1,           //0x15023680 ~ 0x150236AC
    DIP_A_LDNR_D1,          //0x150236C0 ~ 0x1502374C
    DIP_A_SLK_D6,           //0x15023780 ~ 0x150237AC
    DIP_A_FLC_D1,           //0x15024000 ~ 0x1502400C
    DIP_A_CCM_D1,           //0x15024040 ~ 0x15024060
    DIP_A_CCM_D2,           //0x15024080 ~ 0x150240A0
    DIP_A_GGM_D1,           //0x150240C0 ~ 0x150243C4
    DIP_A_GGM_D2,           //0x15024400 ~ 0x15024704
    DIP_A_WSYNC_D1,         //0x15024A80 ~ 0x15024A8C
    DIP_A_UNP_D4,           //0x15024AC0 ~ 0x15024AC4
    DIP_A_PAK_D4,           //0x15024B00
    DIP_A_SMT_D4,           //0x15024B40 ~ 0x15024B60
    DIP_A_MCRP_D2,          //0x15024B80 ~ 0x15024B84
    DIP_A_UNP_D6,           //0x15025080 ~ 0x15025084
    DIP_A_UNP_D7,           //0x150250C0 ~ 0x150250C4
    DIP_A_UNP_D8,           //0x15025100 ~ 0x15025104
    DIP_A_C02_D2,           //0x15025180 ~ 0x15025188
    DIP_A_C24_D1,           //0x150251C0
    DIP_A_G2CX_D1,          //0x15025200 ~ 0x15025230
    DIP_A_C42_D1,           //0x15025240
    DIP_A_MIX_D3,           //0x15025280 ~ 0x15025284
    DIP_A_SLK_D2,           //0x150252C0 ~ 0x150252EC
    DIP_A_YNR_D1,           //0x15025300 ~ 0x1502584C
    DIP_A_NDG_D1,           //0x15025900 ~ 0x15025914
    DIP_A_SRZ_D4,           //0x15025940 ~ 0x15025960
    DIP_A_C02_D1,           //0x15025980 ~ 0x15025988
    DIP_A_UNP_D9,           //0x150259C0 ~ 0x150259C4
    DIP_A_UNP_D10,          //0x15025A00 ~ 0x15025A04
    DIP_A_UNP_D11,          //0x15025A40 ~ 0x15025A44
    DIP_A_MIX_D1,           //0x15026000 ~ 0x15026004
    DIP_A_C24_D3,           //0x15026040
    DIP_A_C2G_D1,           //0x15026080 ~ 0x15026094
    DIP_A_IGGM_D1,          //0x150260C0 ~ 0x150268C0
    DIP_A_CCM_D3,           //0x150269C0 ~ 0x150269E0
    DIP_A_LCE_D1,           //0x15026A00 ~ 0x15026A78
    DIP_A_GGM_D3,           //0x15026A80 ~ 0x15026D84
    DIP_A_DCE_D1,           //0x15027000 ~ 0x15027090
    DIP_A_DCES_D1,          //0x15027140 ~ 0x15027158
    DIP_A_G2C_D1,           //0x15027180 ~ 0x15027194
    DIP_A_C42_D2,           //0x150271C0
    DIP_A_EE_D1,            //0x15027200 ~ 0x150272A0
    DIP_A_SLK_D4,           //0x150272C0 ~ 0x150272EC
    DIP_A_SMT_D2,           //0x15027300 ~ 0x15027320
    DIP_A_UNP_D2,           //0x15027340 ~ 0x15027344
    DIP_A_PAK_D2,           //0x15027380
    DIP_A_CNR_D1,           //0x150273C0 ~ 0x150274AC
    DIP_A_SLK_D3,           //0x150274C0 ~ 0x150274EC
    DIP_A_NDG_D2,           //0x15027500 ~ 0x15027514
    DIP_A_SRZ_D3,           //0x15027540 ~ 0x15027560
    DIP_A_SMT_D3,           //0x15027580 ~ 0x150275A0
    DIP_A_UNP_D3,           //0x150275C0 ~ 0x150275C4
    DIP_A_PAK_D3,           //0x15027600
    DIP_A_COLOR_D1,         //0x15027640 ~ 0x150278AC
    DIP_A_MIX_D2,           //0x15027B40 ~ 0x15027B44
    DIP_A_MIX_D4,           //0x15027BC0 ~ 0x15027BC4
    DIP_A_NR3D_D1,          //0x15028000 ~ 0x15028250
    DIP_A_SLK_D5,           //0x15028600 ~ 0x1502862C
    DIP_A_C24_D2,           //0x15028640
    DIP_A_CRSP_D1,          //0x15028680 ~ 0x15028690
    DIP_A_MCRP_D1,          //0x150286C0 ~ 0x150286C4
    DIP_A_CRZ_D1,           //0x15028700 ~ 0x15028734
    DIP_A_SRZ_D1,           //0x150287C0 ~ 0x150287E0
    DIP_A_FM_D1,            //0x15028800 ~ 0x15028808
    DIP_A_LPCNR_D1,         //0x15028840 ~ 0x150288A8
    DIP_A_BS_D1,            //0x15028940
    DIP_A_PAK_D6,           //0x15028980
    DIP_A_PAK_D7,           //0x150289C0
    DIP_A_PAK_D8,           //0x15028A00
    DIP_A_DFE_D1,           //0x15028A40 ~ 0x15028A50
    DIP_A_CRP_D1,           //0x15028A80 ~ 0x15028A84
    DIP_A_LTMTC_D1_A,       //0x1502B000 ~ 0x1502CBD7
    DIP_A_LTMTC_D1_B,       //0x1502B000 ~ 0x1502CBD7
    DIP_A_END_,             //must be kept at the end of descriptor
    //CQ ==> 0x15022200~0x150222f0 (set them via physical address directly)
    DIP_A_MODULE_MAX,
    DIP_A_DUMMY = 0xff
}DIP_A_MODULE_ENUM;


#define DEQUE_DMA_MASK 0xf0200021
#define DEQUE_DMA2_MASK 0x00000008

#define DIP_OFFSET 0x21000

static ISP_DRV_CQ_MODULE_INFO_STRUCT mIspDipCQModuleInfo[DIP_A_MODULE_MAX]
=  {{DIP_A_CTL_EN,           0x1010, 6   },  //0x15022010 ~ 0x15022024
    {DIP_A_CTL_MUX,          0x1040, 2   },  //0x15022040 ~ 0x15022044
    {DIP_A_CTL_FMT,          0x1050, 3   },  //0x15022050 ~ 0x15022058
    {DIP_A_CTL_TDR,          0x1060, 8   },  //0x15022060 ~ 0x1502207C
    {DIP_A_CTL_DONE,         0x1030, 2   },  //0x15022030 ~ 0x15022034
    {DIP_A_TDRI,             0x0004, 3   },  //0x15021004 ~ 0x1502100C
    {DIP_A_VECTICAL_FLIP,    0x0010, 1   },  //0x15021010
    {DIP_A_SPECIAL_FUN,      0x001C, 1   },  //0x1502101C
    {DIP_A_DCES_DCM,         0x1118, 1   },  //0x15022118
    {DIP_A_IMGI_D1,          0x0104, 5   },  //0x15021104 ~ 0x15021114
    {DIP_A_UFDI_D1,          0x0134, 5   },  //0x15021134 ~ 0x15021144
    {DIP_A_SMTI_D1,          0x0160, 6   },  //0x15021160 ~ 0x15021174
    {DIP_A_SMTO_D1,          0x0190, 6   },  //0x15021190 ~ 0x150211A4
    {DIP_A_SMTO_D1_CROP,     0x01B4, 1   },  //0x150211B4
    {DIP_A_IMGBI_D1,         0x0204, 5   },  //0x15021204 ~ 0x15021214
    {DIP_A_IMGCI_D1,         0x0234, 5   },  //0x15021234 ~ 0x15021244
    {DIP_A_TIMGO_D1,         0x0264, 5   },  //0x15021264 ~ 0x15021274
    {DIP_A_TIMGO_D1_CROP,    0x0284, 1   },  //0x15021284
    {DIP_A_SMTI_D4,          0x02D0, 6   },  //0x150212D0 ~ 0x150212E4
    {DIP_A_SMTO_D4,          0x0300, 6   },  //0x15021300 ~ 0x15021314
    {DIP_A_SMTO_D4_CROP,     0x0324, 1   },  //0x15021324
    {DIP_A_DMGI_D1,          0x0374, 5   },  //0x15021374 ~ 0x15021384
    {DIP_A_DEPI_D1,          0x03A4, 5   },  //0x150213A4 ~ 0x150213B4
    {DIP_A_LCEI_D1,          0x03D4, 5   },  //0x150213D4 ~ 0x150213E4
    {DIP_A_DCESO_D1,         0x0404, 5   },  //0x15021404 ~ 0x15021414
    {DIP_A_SMTI_D2,          0x0470, 6   },  //0x15021470 ~ 0x15021484
    {DIP_A_SMTO_D2,          0x04A0, 6   },  //0x150214A0 ~ 0x150214B4
    {DIP_A_SMTO_D2_CROP,     0x04C4, 1   },  //0x150214C4
    {DIP_A_SMTI_D3,          0x0510, 6   },  //0x15021510 ~ 0x15021524
    {DIP_A_SMTO_D3,          0x0540, 6   },  //0x15021540 ~ 0x15021554
    {DIP_A_SMTO_D3_CROP,     0x0564, 1   },  //0x15021564
    {DIP_A_CRZO_D1,          0x05B4, 5   },  //0x150215B4 ~ 0x150215C4
    {DIP_A_CRZO_D1_CROP,     0x05D4, 1   },  //0x150215D4
    {DIP_A_CRZBO_D1,         0x0624, 5   },  //0x15021624 ~ 0x15021634
    {DIP_A_CRZBO_D1_CROP,    0x0644, 1   },  //0x15021644
    {DIP_A_FEO_D1,           0x0694, 5   },  //0x15021694 ~ 0x150216A4
    {DIP_A_VIPI_D1,          0x0704, 5   },  //0x15021704 ~ 0x15021714
    {DIP_A_VIPBI_D1,         0x0734, 5   },  //0x15021734 ~ 0x15021744
    {DIP_A_VIPCI_D1,         0x0764, 5   },  //0x15021764 ~ 0x15021774
    {DIP_A_IMG3O_D1,         0x0794, 5   },  //0x15021794 ~ 0x150217A4
    {DIP_A_IMG3O_D1_CROP,    0x07B4, 1   },  //0x150217B4
    {DIP_A_IMG3BO_D1,        0x0804, 5   },  //0x15021804 ~ 0x15021814
    {DIP_A_IMG3BO_D1_CROP,   0x0824, 1   },  //0x15021824
    {DIP_A_IMG3CO_D1,        0x0874, 5   },  //0x15021874 ~ 0x15021884
    {DIP_A_IMG3CO_D1_CROP,   0x0894, 1   },  //0x15021894
//DIPAMD?
    {DIP_A_UNP_D1,           0x2000, 2   },  //0x15023000 ~ 0x15023004
    {DIP_A_UFD_D1,           0x2040, 14  },  //0x15023040 ~ 0x15023074
    {DIP_A_SMT_D1,           0x2080, 9   },  //0x15023080 ~ 0x150230A0
    {DIP_A_BPC_D1,           0x2100, 58  },  //0x15023100 ~ 0x150231E4
    {DIP_A_OBC_D1,           0x2200, 28  },  //0x15023200 ~ 0x1502326C
    {DIP_A_ZFUS_D1,          0x2280, 5   },  //0x15023280 ~ 0x15023290
    {DIP_A_DGN_D1,           0x2300, 5   },  //0x15023300 ~ 0x15023310
    {DIP_A_LSC_D1,           0x2340, 11  },  //0x15023340 ~ 0x15023368
    {DIP_A_WB_D1,            0x2380, 3   },  //0x15023380 ~ 0x15023388
    {DIP_A_HLR_D1,           0x23C0, 17  },  //0x150233C0 ~ 0x15023400
    {DIP_A_LTM_D1,           0x2440, 56  },  //0x15023440 ~ 0x1502351C
    {DIP_A_CRP_D2,           0x2540, 2   },  //0x15023540 ~ 0x15023544
    {DIP_A_PAK_D1,           0x2580, 1   },  //0x15023580
    {DIP_A_DM_D1,            0x2600, 26  },  //0x15023600 ~ 0x15023664
    {DIP_A_SLK_D1,           0x2680, 12  },  //0x15023680 ~ 0x150236AC
    {DIP_A_LDNR_D1,          0x26C0, 36  },  //0x150236C0 ~ 0x1502374C
    {DIP_A_SLK_D6,           0x2780, 12  },  //0x15023780 ~ 0x150237AC
    {DIP_A_FLC_D1,           0x3000, 4   },  //0x15024000 ~ 0x1502400C
    {DIP_A_CCM_D1,           0x3040, 9   },  //0x15024040 ~ 0x15024060
    {DIP_A_CCM_D2,           0x3080, 9   },  //0x15024080 ~ 0x150240A0
    {DIP_A_GGM_D1,           0x30C0, 194 },  //0x150240C0 ~ 0x150243C4
    {DIP_A_GGM_D2,           0x3400, 194 },  //0x15024400 ~ 0x15024704
    {DIP_A_WSYNC_D1,         0x3A80, 4   },  //0x15024A80 ~ 0x15024A8C
    {DIP_A_UNP_D4,           0x3AC0, 2   },  //0x15024AC0 ~ 0x15024AC4
    {DIP_A_PAK_D4,           0x3B00, 1   },  //0x15024B00
    {DIP_A_SMT_D4,           0x3B40, 9   },  //0x15024B40 ~ 0x15024B60
    {DIP_A_MCRP_D2,          0x3B80, 2   },  //0x15024B80 ~ 0x15024B84
    {DIP_A_UNP_D6,           0x4080, 2   },  //0x15025080 ~ 0x15025084
    {DIP_A_UNP_D7,           0x40C0, 2   },  //0x150250C0 ~ 0x150250C4
    {DIP_A_UNP_D8,           0x4100, 2   },  //0x15025100 ~ 0x15025104
    {DIP_A_C02_D2,           0x4180, 3   },  //0x15025180 ~ 0x15025188
    {DIP_A_C24_D1,           0x41C0, 1   },  //0x150251C0
    {DIP_A_G2CX_D1,          0x4200, 13  },  //0x15025200 ~ 0x15025230
    {DIP_A_C42_D1,           0x4240, 1   },  //0x15025240
    {DIP_A_MIX_D3,           0x4280, 2   },  //0x15025280 ~ 0x15025284
    {DIP_A_SLK_D2,           0x42C0, 12  },  //0x150252C0 ~ 0x150252EC
    {DIP_A_YNR_D1,           0x4300, 340 },  //0x15025300 ~ 0x1502584C
    {DIP_A_NDG_D1,           0x4900, 6   },  //0x15025900 ~ 0x15025914
    {DIP_A_SRZ_D4,           0x4940, 9   },  //0x15025940 ~ 0x15025960
    {DIP_A_C02_D1,           0x4980, 3   },  //0x15025980 ~ 0x15025988
    {DIP_A_UNP_D9,           0x49C0, 2   },  //0x150259C0 ~ 0x150259C4
    {DIP_A_UNP_D10,          0x4A00, 2   },  //0x15025A00 ~ 0x15025A04
    {DIP_A_UNP_D11,          0x4A40, 2   },  //0x15025A40 ~ 0x15025A44
    {DIP_A_MIX_D1,           0x5000, 2   },  //0x15026000 ~ 0x15026004
    {DIP_A_C24_D3,           0x5040, 1   },  //0x15026040
    {DIP_A_C2G_D1,           0x5080, 6   },  //0x15026080 ~ 0x15026094
    {DIP_A_IGGM_D1,          0x50C0, 513 },  //0x150260C0 ~ 0x150268C0
    {DIP_A_CCM_D3,           0x59C0, 9   },  //0x150269C0 ~ 0x150269E0
    {DIP_A_LCE_D1,           0x5A00, 31  },  //0x15026A00 ~ 0x15026A78
    {DIP_A_GGM_D3,           0x5A80, 194 },  //0x15026A80 ~ 0x15026D84
    {DIP_A_DCE_D1,           0x6000, 37  },  //0x15027000 ~ 0x15027090
    {DIP_A_DCES_D1,          0x6140, 7   },  //0x15027140 ~ 0x15027158
    {DIP_A_G2C_D1,           0x6180, 6   },  //0x15027180 ~ 0x15027194
    {DIP_A_C42_D2,           0x61C0, 1   },  //0x150271C0
    {DIP_A_EE_D1,            0x6200, 41  },  //0x15027200 ~ 0x150272A0
    {DIP_A_SLK_D4,           0x62C0, 12  },  //0x150272C0 ~ 0x150272EC
    {DIP_A_SMT_D2,           0x6300, 9   },  //0x15027300 ~ 0x15027320
    {DIP_A_UNP_D2,           0x6340, 2   },  //0x15027340 ~ 0x15027344
    {DIP_A_PAK_D2,           0x6380, 1   },  //0x15027380
    {DIP_A_CNR_D1,           0x63C0, 60  },  //0x150273C0 ~ 0x150274AC
    {DIP_A_SLK_D3,           0x64C0, 12  },  //0x150274C0 ~ 0x150274EC
    {DIP_A_NDG_D2,           0x6500, 6   },  //0x15027500 ~ 0x15027514
    {DIP_A_SRZ_D3,           0x6540, 9   },  //0x15027540 ~ 0x15027560
    {DIP_A_SMT_D3,           0x6580, 9   },  //0x15027580 ~ 0x150275A0
    {DIP_A_UNP_D3,           0x65C0, 2   },  //0x150275C0 ~ 0x150275C4
    {DIP_A_PAK_D3,           0x6600, 1   },  //0x15027600
    {DIP_A_COLOR_D1,         0x6640, 156 },  //0x15027640 ~ 0x150278AC
    {DIP_A_MIX_D2,           0x6B40, 2   },  //0x15027B40 ~ 0x15027B44
    {DIP_A_MIX_D4,           0x6BC0, 2   },  //0x15027BC0 ~ 0x15027BC4
    {DIP_A_NR3D_D1,          0x7000, 149 },  //0x15028000 ~ 0x15028250
    {DIP_A_SLK_D5,           0x7600, 12  },  //0x15028600 ~ 0x1502862C
    {DIP_A_C24_D2,           0x7640, 1   },  //0x15028640
    {DIP_A_CRSP_D1,          0x7680, 5   },  //0x15028680 ~ 0x15028690
    {DIP_A_MCRP_D1,          0x76C0, 2   },  //0x150286C0 ~ 0x150286C4
    {DIP_A_CRZ_D1,           0x7700, 14  },  //0x15028700 ~ 0x15028734
    {DIP_A_SRZ_D1,           0x77C0, 9   },  //0x150287C0 ~ 0x150287E0
    {DIP_A_FM_D1,            0x7800, 3   },  //0x15028800 ~ 0x15028808
    {DIP_A_LPCNR_D1,         0x7840, 27  },  //0x15028840 ~ 0x150288A8
    {DIP_A_BS_D1,            0x7940, 1   },  //0x15028940
    {DIP_A_PAK_D6,           0x7980, 1   },  //0x15028980
    {DIP_A_PAK_D7,           0x79C0, 1   },  //0x150289C0
    {DIP_A_PAK_D8,           0x7A00, 1   },  //0x15028A00
    {DIP_A_DFE_D1,           0x7A40, 5   },  //0x15028A40 ~ 0x15028A50
    {DIP_A_CRP_D1,           0x7A80, 2   },  //0x15028A80 ~ 0x15028A84
    {DIP_A_LTMTC_D1_A,       0xA000, 900 },  //0x1502B000 ~ 0x1502CBD4
    {DIP_A_LTMTC_D1_B,       0xAE10, 882 },  //0x1502B000 ~ 0x1502CBD4
    {DIP_A_END_,             0x0000, 1   }
};



///////////////////////////////////////////////////////////
/**
dip initial setting
*/
#define ISP_DIP_INIT_SETTING_COUNT  123
static ISP_DRV_REG_IO_STRUCT mIspDipInitReg[ISP_DIP_INIT_SETTING_COUNT]
=  {{DIP_HW_A, 0x1128, 0x1},   //DIPCTL_D1A_DIPCTL_TOP_DCM_DIS
    {DIP_HW_A, 0x10A0, 0x80000000},   //DIPCTL_D1A_DIPCTL_INT1_EN
    {DIP_HW_A, 0x10B0, 0x0},   //DIPCTL_D1A_DIPCTL_INT2_EN
    {DIP_HW_A, 0x10C0, 0x0},   //DIPCTL_D1A_DIPCTL_INT3_EN
    {DIP_HW_A, 0x10D0, 0x0},   //DIPCTL_D1A_DIPCTL_CQ_INT1_EN
    {DIP_HW_A, 0x10E0, 0x0},   //DIPCTL_D1A_DIPCTL_CQ_INT2_EN
    {DIP_HW_A, 0x10F0, 0x0},   //DIPCTL_D1A_DIPCTL_CQ_INT3_EN
    {DIP_HW_A, 0x1204, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR0_CTL
    {DIP_HW_A, 0x121C, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR1_CTL
    {DIP_HW_A, 0x1228, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR2_CTL
    {DIP_HW_A, 0x1234, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR3_CTL
    {DIP_HW_A, 0x1240, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR4_CTL
    {DIP_HW_A, 0x124C, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR5_CTL
    {DIP_HW_A, 0x1258, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR6_CTL
    {DIP_HW_A, 0x1264, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR7_CTL
    {DIP_HW_A, 0x1270, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR8_CTL
    {DIP_HW_A, 0x127C, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR9_CTL
    {DIP_HW_A, 0x1288, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR10_CTL
    {DIP_HW_A, 0x1294, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR11_CTL
    {DIP_HW_A, 0x12A0, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR12_CTL
    {DIP_HW_A, 0x12AC, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR13_CTL
    {DIP_HW_A, 0x12B8, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR14_CTL
    {DIP_HW_A, 0x12C4, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR15_CTL
    {DIP_HW_A, 0x12D0, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},	 //CQ_D1A_CQ_THR16_CTL
    {DIP_HW_A, 0x12DC, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR17_CTL
    {DIP_HW_A, 0x12E8, (CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE)},   //CQ_D1A_CQ_THR18_CTL
    {DIP_HW_A, 0x1210, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR0_DESC_SIZE
    {DIP_HW_A, 0x1224, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR1_DESC_SIZE
    {DIP_HW_A, 0x1230, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR2_DESC_SIZE
    {DIP_HW_A, 0x123C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR3_DESC_SIZE
    {DIP_HW_A, 0x1248, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR4_DESC_SIZE
    {DIP_HW_A, 0x1254, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR5_DESC_SIZE
    {DIP_HW_A, 0x1260, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR6_DESC_SIZE
    {DIP_HW_A, 0x126C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR7_DESC_SIZE
    {DIP_HW_A, 0x1278, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR8_DESC_SIZE
    {DIP_HW_A, 0x1284, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR9_DESC_SIZE
    {DIP_HW_A, 0x1290, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR10_DESC_SIZE
    {DIP_HW_A, 0x129C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR11_DESC_SIZE
    {DIP_HW_A, 0x12A8, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR12_DESC_SIZE
    {DIP_HW_A, 0x12B4, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR13_DESC_SIZE
    {DIP_HW_A, 0x12C0, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR14_DESC_SIZE
    {DIP_HW_A, 0x12CC, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR15_DESC_SIZE
    {DIP_HW_A, 0x12D8, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR16_DESC_SIZE
    {DIP_HW_A, 0x12E4, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR17_DESC_SIZE
    {DIP_HW_A, 0x12F0, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //CQ_D1A_CQ_THR18_DESC_SIZE
    {DIP_HW_A, 0x118, 0x80000100},    //IMGI_D1A_IMGI_CON
    {DIP_HW_A, 0x11C, 0x01000100},    //IMGI_D1A_IMGI_CON2
    {DIP_HW_A, 0x120, 0x00500050},    //IMGI_D1A_IMGI_CON3
    {DIP_HW_A, 0x148, 0x80000040},    //UFDI_D1A_UFDI_CON
    {DIP_HW_A, 0x14C, 0x00400040},    //UFDI_D1A_UFDI_CON2
    {DIP_HW_A, 0x150, 0x00140014},    //UFDI_D1A_UFDI_CON3
    {DIP_HW_A, 0x178, 0x80000040},    //SMTI_D1A_SMTI_CON
    {DIP_HW_A, 0x17C, 0x00400040},    //SMTI_D1A_SMTI_CON2
    {DIP_HW_A, 0x180, 0x00140014},    //SMTI_D1A_SMTI_CON3
    {DIP_HW_A, 0x1A8, 0x80000100},    //SMTO_D1A_SMTO_CON
    {DIP_HW_A, 0x1AC, 0x01000100},    //SMTO_D1A_SMTO_CON2
    {DIP_HW_A, 0x1B0, 0x00A000A0},    //SMTO_D1A_SMTO_CON3
    {DIP_HW_A, 0x218, 0x80000080},    //IMGBI_D1A_IMGBI_CON
    {DIP_HW_A, 0x21C, 0x00800080},    //IMGBI_D1A_IMGBI_CON2
    {DIP_HW_A, 0x220, 0x00280028},    //IMGBI_D1A_IMGBI_CON3
    {DIP_HW_A, 0x248, 0x80000080},    //IMGCI_D1A_IMGCI_CON
    {DIP_HW_A, 0x24C, 0x00800080},    //IMGCI_D1A_IMGCI_CON2
    {DIP_HW_A, 0x250, 0x00280028},    //IMGCI_D1A_IMGCI_CON3
    {DIP_HW_A, 0x278, 0x80000080},    //TIMGO_D1A_TIMGO_CON
    {DIP_HW_A, 0x27C, 0x00800080},    //TIMGO_D1A_TIMGO_CON2
    {DIP_HW_A, 0x280, 0x00500050},    //TIMGO_D1A_TIMGO_CON3
    {DIP_HW_A, 0x2E8, 0x80000040},    //SMTI_D4A_SMTI_CON
    {DIP_HW_A, 0x2EC, 0x00400040},    //SMTI_D4A_SMTI_CON2
    {DIP_HW_A, 0x2F0, 0x00140014},    //SMTI_D4A_SMTI_CON3
    {DIP_HW_A, 0x318, 0x80000040},    //SMTO_D4A_SMTO_CON
    {DIP_HW_A, 0x31C, 0x00400040},    //SMTO_D4A_SMTO_CON2
    {DIP_HW_A, 0x320, 0x00280028},    //SMTO_D4A_SMTO_CON3
    {DIP_HW_A, 0x388, 0x80000100},    //DMGI_D1A_DMGI_CON
    {DIP_HW_A, 0x38C, 0x01000100},    //DMGI_D1A_DMGI_CON2
    {DIP_HW_A, 0x390, 0x00500050},    //DMGI_D1A_DMGI_CON3
    {DIP_HW_A, 0x3B8, 0x800000C0},    //DEPI_D1A_DEPI_CON
    {DIP_HW_A, 0x3BC, 0x00C000C0},    //DEPI_D1A_DEPI_CON2
    {DIP_HW_A, 0x3C0, 0x00400040},    //DEPI_D1A_DEPI_CON3
    {DIP_HW_A, 0x3E8, 0x80000080},    //LCEI_D1A_LCEI_CON
    {DIP_HW_A, 0x3EC, 0x00800080},    //LCEI_D1A_LCEI_CON2
    {DIP_HW_A, 0x3F0, 0x00300030},    //LCEI_D1A_LCEI_CON3
    {DIP_HW_A, 0x418, 0x80000080},    //DCESO_D1A_DCESO_CON
    {DIP_HW_A, 0x41C, 0x00800080},    //DCESO_D1A_DCESO_CON2
    {DIP_HW_A, 0x420, 0x00500050},    //DCESO_D1A_DCESO_CON3
    {DIP_HW_A, 0x488, 0x80000040},    //SMTI_D2A_SMTI_CON
    {DIP_HW_A, 0x48C, 0x00400040},    //SMTI_D2A_SMTI_CON2
    {DIP_HW_A, 0x490, 0x00140014},    //SMTI_D2A_SMTI_CON3
    {DIP_HW_A, 0x4B8, 0x80000080},    //SMTO_D2A_SMTO_CON
    {DIP_HW_A, 0x4BC, 0x00800080},    //SMTO_D2A_SMTO_CON2
    {DIP_HW_A, 0x4C0, 0x00500050},    //SMTO_D2A_SMTO_CON3
    {DIP_HW_A, 0x528, 0x80000040},    //SMTI_D3A_SMTI_CON
    {DIP_HW_A, 0x52C, 0x00400040},    //SMTI_D3A_SMTI_CON2
    {DIP_HW_A, 0x530, 0x00140014},    //SMTI_D3A_SMTI_CON3
    {DIP_HW_A, 0x558, 0x80000040},    //SMTO_D3A_SMTO_CON
    {DIP_HW_A, 0x55C, 0x00400040},    //SMTO_D3A_SMTO_CON2
    {DIP_HW_A, 0x560, 0x00280028},    //SMTO_D3A_SMTO_CON3
    {DIP_HW_A, 0x5C8, 0x80000080},    //CRZO_D1A_CRZO_CON
    {DIP_HW_A, 0x5CC, 0x00800080},    //CRZO_D1A_CRZO_CON2
    {DIP_HW_A, 0x5D0, 0x00500050},    //CRZO_D1A_CRZO_CON3
    {DIP_HW_A, 0x638, 0x80000040},    //CRZBO_D1A_CRZBO_CON
    {DIP_HW_A, 0x63C, 0x00400040},    //CRZBO_D1A_CRZBO_CON2
    {DIP_HW_A, 0x640, 0x00280028},    //CRZBO_D1A_CRZBO_CON3
    {DIP_HW_A, 0x6A8, 0x80000040},    //FEO_D1A_FEO_CON
    {DIP_HW_A, 0x6AC, 0x00400040},    //FEO_D1A_FEO_CON2
    {DIP_HW_A, 0x6B0, 0x00280028},    //FEO_D1A_FEO_CON3
    {DIP_HW_A, 0x718, 0x80000080},    //VIPI_D1A_VIPI_CON
    {DIP_HW_A, 0x71C, 0x00800080},    //VIPI_D1A_VIPI_CON2
    {DIP_HW_A, 0x720, 0x00280028},    //VIPI_D1A_VIPI_CON3
    {DIP_HW_A, 0x748, 0x80000040},    //VIPBI_D1A_VIPBI_CON
    {DIP_HW_A, 0x74C, 0x00400040},    //VIPBI_D1A_VIPBI_CON2
    {DIP_HW_A, 0x750, 0x00140014},    //VIPBI_D1A_VIPBI_CON3
    {DIP_HW_A, 0x778, 0x80000040},    //VIPCI_D1A_VIPCI_CON
    {DIP_HW_A, 0x77C, 0x00400040},    //VIPCI_D1A_VIPCI_CON2
    {DIP_HW_A, 0x780, 0x00140014},    //VIPCI_D1A_VIPCI_CON3
    {DIP_HW_A, 0x7A8, 0x80000080},    //IMG3O_D1A_IMG3O_CON
    {DIP_HW_A, 0x7AC, 0x00800080},    //IMG3O_D1A_IMG3O_CON2
    {DIP_HW_A, 0x7B0, 0x00500050},    //IMG3O_D1A_IMG3O_CON3
    {DIP_HW_A, 0x818, 0x80000040},    //IMG3BO_D1A_IMG3BO_CON
    {DIP_HW_A, 0x81C, 0x00400040},    //IMG3BO_D1A_IMG3BO_CON2
    {DIP_HW_A, 0x820, 0x00280028},    //IMG3BO_D1A_IMG3BO_CON3
    {DIP_HW_A, 0x888, 0x80000040},    //IMG3CO_D1A_IMG3CO_CON
    {DIP_HW_A, 0x88C, 0x00400040},    //IMG3CO_D1A_IMG3CO_CON2
    {DIP_HW_A, 0x890, 0x00280028}     //IMG3CO_D1A_IMG3CO_CON3
};



// for p2 tpipe dump information
#undef DUMP_TPIPE_SIZE
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
#define DIP_A_BASE_HW   0x15021000
#define DIP_REG_RANGE           (0xC000) //PAGE_SIZE*6 = 4096*6, dependent on device tree setting
//
#undef MAX_TILE_TOT_NO
#define MAX_TILE_TOT_NO (256)
#define MAX_ISP_DUMP_HEX_PER_TILE (256)
#define MAX_ISP_TILE_TDR_HEX_NO (MAX_TILE_TOT_NO*MAX_ISP_DUMP_HEX_PER_TILE)
#define TPIPE_BUFFER_SIZE (MAX_ISP_TILE_TDR_HEX_NO) //Chip Dependent

#define MAX_DIP_CMDQ_BUFFER_SIZE (0x1000) /*(GET_MAX_CQ_DESCRIPTOR_SIZE()/ISP_DIP_CQ_DUMMY_BUFFER+1)*ISP_DIP_CQ_DUMMY_BUFFER;*/

#define DIP_TDRI_ADDR_MASK       (0xffff0000)
#define DIP_IMGI_BASEADDR_OFFSET (0x100>>2)
#define DIP_DUMP_ADDR_MASK       (0xffffffff)


#define TPIPE_WIDTH 256 //use mdp maximum tile size/2 to check how many tile in this diagram.
#define MAX_BURST_QUE_NUM   10
#define MAX_DUP_CQ_NUM      2
#define MAX_CMDQ_RING_BUFFER_SIZE_NUM   40 //(10+2+1)*3*2.
#define DIP_RING_BUFFER_CQ_SIZE ISP_DRV_DIP_CQ_THRE2
#define TPIPE_BLD_SIZE 4800 // Max 25 tiles: 192x25
#define ISP_MDP_DEQUE_TIME_US 200000 //us as unit
#define ISP_MDP_STARTSTREAM_TIME_US 33000 //us as unit
#define MAX_LOGBUF_LEN 256
#endif //_ISP_DRV_DIP_PLATFORM_H_

