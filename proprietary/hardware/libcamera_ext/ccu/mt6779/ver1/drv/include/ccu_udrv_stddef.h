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


#ifndef _CCU_DRV_STDDEF_H_
#define _CCU_DRV_STDDEF_H_

//////////////////////////////////////////////////////////////////////////////////////////////
/**
    CCU path: line buffer
*/
#define CCU_RRZ_MAX_LINE_BUFFER_IN_PIXEL (4608)
#define CCU_BNR_MAX_LINE_BUFFER_IN_PIXEL (5376)

#define DMX_SRAM    (5376/8)//(864)
#define RMX_SRAM    (4608/8)//(576)
#define BMX_SRAM    (5376/8)//(864)
#define PMX_SRAM    (1920/8)//(384)
#define AMX_SRAM    (5376/8)//SRAM_SIZE[15:0]
/**
    CCU path: hw image heaer depth
*/
#define CCU_IMAGE_HEADER    (16)

/**
    CQ SW maximum ring depth.
*/
#define MAX_RING_SIZE   16

/**
    for AFO hw constraint under twin mode
    note: 128 for vertical direction
             16 ( win/byte)
*/
#define AFO_STRIDE      (128*16)
#define AFO_MAX_SIZE    (128*128*16)
#define AFO_DUMMY_STRIDE    (1*16)
#define AFO_DUMMY_WIN   (AFO_DUMMY_STRIDE * 128)

/**
    for AAO
*/
#define AAO_MAX_SIZE ((120*(32+8+16+8+16)*90 + 128*24*4)/8)

/**
   for FLKO
*/
#define FLKO_MAX_SIZE(ImgH) ((ImgH*6*16)/8)

/**
    for PDO
    pdo data size is 2 byte per pix
*/
#define PDO_STRIDE_ALIGN(W) ({\
    MUINT32 _align = 0;\
    if((W % 16) != 0){\
        _align = ((W + 15)/16 * 16);\
    }\
    else{\
        _align = W;\
    }\
    _align;\
})

#define PDO_STRIDE_CONS(W) ( (PDO_STRIDE_ALIGN(W) + 16)*2)
#define PDO_MAX_SIZE(W,H) (PDO_STRIDE_CONS(W) * H)


//////////////////////////////////////////////////////////////////////////////////////////////

/**
    CCU interrupt status
*/
//CCU_CTL_RAW_INT_STATUSX
typedef enum{
    CCU_INT_ST           = (1L<<0),
}ENUM_CCU_INT;

#if 0

//CCU_CTL_RAW_INT2_STATUS
typedef enum{
    IMGO_DONE_ST        = (1L<<0),
    UFEO_DONE_ST        = (1L<<1),
    RRZO_DONE_ST        = (1L<<2),
    EISO_DONE_ST        = (1L<<3),
    FLKO_DONE_ST        = (1L<<4),
    AFO_DONE_ST         = (1L<<5),
    LCSO_DONE_ST        = (1L<<6),
    AAO_DONE_ST         = (1L<<7),
    BPCI_DONE_ST        = (1L<<9),
    LSCI_DONE_ST        = (1L<<10),
    //CACI_DONE_ST        (1L<<11)
    PDO_DONE_ST         = (1L<<13)
}ENUM_CCU_DMA_INT;

/**
    CCU A function enable bit
*/
typedef enum{
    TG_EN_          = (1L<<0),
    DMX_EN_         = (1L<<1),
    SGM_EN_         = (1L<<2),
    RMG_EN_         = (1L<<3),
    RMM_EN_         = (1L<<4),
    OBC_EN_         = (1L<<5),
    BNR_EN_         = (1L<<6),
    LSC_EN_         = (1L<<7),
    //CAC_EN_         = (1L<<8),
    RPG_EN_         = (1L<<9),
    RRZ_EN_         = (1L<<10),
    RMX_EN_         = (1L<<11),
    PAKG_EN_        = (1L<<12),
    BMX_EN_         = (1L<<13),
    CPG_EN_         = (1L<<14),
    PAK_EN_         = (1L<<15),
    UFE_EN_         = (1L<<16),
    AF_EN_          = (1L<<17),
    SGG1_EN_        = (1L<<18),
    AA_EN_          = (1L<<19),
    QBIN1_EN_       = (1L<<20),
    LCS_EN_         = (1L<<21),
    QBIN2_EN_       = (1L<<22),
    RCP_EN_         = (1L<<23),
    RCP3_EN_        = (1L<<24),
    PMX_EN_         = (1L<<25),
    PKP_EN_         = (1L<<26),
    BIN_EN_         = (1L<<27),
    DBS_EN_         = (1L<<28),
    DBN_EN_         = (1L<<29),
    PBN_EN_         = (1L<<30),
    UFEG_EN_        = (1L<<31),
}ENUM_CCU_CTL_EN;

/**
    CCU DMA function enable
*/
typedef enum{
    IMGO_EN_    = (1L<<0),
    UFEO_EN_    = (1L<<1),
    RRZO_EN_    = (1L<<2),
    AFO_EN_     = (1L<<3),
    LCSO_EN_    = (1L<<4),
    AAO_EN_     = (1L<<5),
    BPCI_EN_    = (1L<<7),
    LSCI_EN_    = (1L<<8),
    //CACI_EN_    = (1L<<9),
    PDO_EN_     = (1L<<10),
    AMX_EN_     = (1L<<25),
    SL2F_EN_    = (1L<<26),
    VBN_EN_     = (1L<<27),
}ENUM_CCU_CTL_DMA_EN;

/**
    CCU interrupt enable
*/
typedef enum{
    VS_INT_EN_              = (1L<<0),
    TG_INT1_EN_             = (1L<<1),
    TG_INT2_EN_             = (1L<<2),
    EXPDON_EN_              = (1L<<3),
    TG_ERR_EN_              = (1L<<4),
    TG_GBERR_EN_            = (1L<<5),
    CQ_CODE_ERR_EN_         = (1L<<6),
    CQ_APB_ERR_EN_          = (1L<<7),
    CQ_VS_ERR_EN_           = (1L<<8),
    HW_PASS1_DON_EN_        = (1L<<11),
    SOF_INT_EN_             = (1L<<12),
    SOF_WAIT_EN_            = (1L<<13),
    RMX_ERR_EN_             = (1L<<16),
    BMX_ERR_EN_             = (1L<<17),
    RRZO_ERR_EN_            = (1L<<18),
    AFO_ERR_EN_             = (1L<<19),
    IMGO_ERR_EN_            = (1L<<20),
    AAO_ERR_EN_             = (1L<<21),
    LCSO_ERR_EN_            = (1L<<23),
    BPC_ERR_EN_             = (1L<<24),
    LSC_ERR_EN_             = (1L<<25),
    //CAC_ERR_EN_             = (1L<<26),
    DMA_ERR_EN_             = (1L<<29),
    SW_PASS1_DON_EN_        = (1L<<30),
    INT_WCLR_EN_            = (1L<<31),
}ENUM_CCU_CTL_RAW_INT_EN;

/**
    CCU DMA interrupt enable
*/
typedef enum{
    IMGO_DONE_EN_   = (1L<<0),
    UFEO_DONE_EN_   = (1L<<1),
    RRZO_DONE_EN_   = (1L<<2),
    EISO_DONE_EN_   = (1L<<3),
    FLKO_DONE_EN_   = (1L<<4),
    AFO_DONE_EN_    = (1L<<5),
    LCSO_DONE_EN_   = (1L<<6),
    AAO_DONE_EN_    = (1L<<7),
    BPCI_DONE_EN_   = (1L<<9),
    LSCI_DONE_EN_   = (1L<<10),
    CACI_DONE_EN_   = (1L<<11),
    PDO_DONE_EN_    = (1L<<13),
}ENUM_CCU_CTL_RAW_INT2_EN;


/**
    CCU multiplexer
*/
typedef enum{
    //DMX_SEL
    DMX_SEL_0   = (0),
    DMX_SEL_1   = (1L<<0),
    DMX_SEL_2   = (1L<<1),
    //HDS1_SEL
    HDS1_SEL_0  = (0),
    HDS1_SEL_1  = (1L<<4),
    //SGG_SEL
    SGG_SEL_0   = (0),
    SGG_SEL_1   = (1L<<12),
    //LCS_SEL
    LCS_SEL_0   = (0),
    LCS_SEL_1   = (1L<<14),
    //IMG_SEL
    IMG_SEL_0   = (0),
    IMG_SEL_1   = (1L<<16),
    IMG_SEL_2   = (1L<<17),
    //UFE_SEL
    UFE_SEL_0   = (0),
    UFE_SEL_1   = (1L<<18),
    UFE_SEL_2   = (1L<<19),
    //FLK1_SEL
    FLK1_SEL_0  = (0),
    FLK1_SEL_1  = (1L<<20),
    //PMX_SEL
    PMX_SEL_0   = (0),
    PMX_SEL_1   = (1L<<22),
    //RCP3_SEL
    RCP3_SEL_0  = (0),
    RCP3_SEL_1  = (1L<<24),
    //UFEG_SEL
    UFEG_SEL_0  = (0),
    UFEG_SEL_1  = (1L<<26),
}ENUM_CCU_CTL_SEL;

/**
    CCU A/B Done_Sel
*/
typedef enum{
    IMGO_A_ =   (1L<<0),
    UFEO_A_ =   (1L<<1),
    RRZO_A_ =   (1L<<2),
    AFO_A_  =   (1L<<3),
    LCSO_A_ =   (1L<<4),
    AAO_A_  =   (1L<<5),
    BPCI_A_ =   (1L<<6),
    LSCI_A_ =   (1L<<7),
    CACI_A_ =   (1L<<8),
    PDO_A_  =   (1L<<10),
    TG_A_   =   (1L<<11),
    IMGO_B_ =   (1L<<16),
    UFEO_B_ =   (1L<<17),
    RRZO_B_ =   (1L<<18),
    AFO_B_  =   (1L<<19),
    LCSO_B_ =   (1L<<20),
    AAO_B_  =   (1L<<21),
    BPCI_B_ =   (1L<<23),
    LSCI_B_ =   (1L<<24),
    CACI_B_ =   (1L<<25),
    PDO_B_  =   (1L<<26),
    TG_B_   =   (1L<<27),
}ENUM_AB_DONE_SEL;

/**
    CCU C/D Done_Sel
*/
typedef enum{
    IMGO_C_ =   (1L<<0),
    UFEO_C_ =   (1L<<1),
    RRZO_C_ =   (1L<<2),
    AFO_C_  =   (1L<<3),
    LCSO_C_ =   (1L<<4),
    AAO_C_  =   (1L<<5),
    BPCI_C_ =   (1L<<7),
    LSCI_C_ =   (1L<<8),
    CACI_C_ =   (1L<<9),
    PDO_C_  =   (1L<<10),
    TG_C_   =   (1L<<11),
    IMGO_D_ =   (1L<<16),
    UFEO_D_ =   (1L<<17),
    RRZO_D_ =   (1L<<18),
    AFO_D_  =   (1L<<19),
    LCSO_D_ =   (1L<<20),
    AAO_D_  =   (1L<<21),
    BPCI_D_ =   (1L<<23),
    LSCI_D_ =   (1L<<24),
    CACI_D_ =   (1L<<25),
    PDO_D_  =   (1L<<26),
    TG_D_   =   (1L<<27),
}ENUM_CD_DONE_SEL;

/**
    CCU UNI Done_Sel
*/
typedef enum{
    DONE_SEL_EN_ =  (1L<<0),
    RSSO_A_      =  (1L<<1),
    EISO_A_      =  (1L<<2),
    FLKO_A_      =  (1L<<3),
}ENUM_UNI_DONE_SEL;

/**
    TG data format
*/
typedef enum{
    TG_FMT_RAW8     = 0,
    TG_FMT_RAW10    = 1,
    TG_FMT_RAW12    = 2,
    TG_FMT_YUV422   = 3,
    TG_FMT_RAW14    = 4,
    TG_FMT_RGB565   = 5,
    TG_FMT_RGB888   = 6,
    TG_FMT_JPG      = 7,
}E_CCU_CTL_TG_FMT;

/**
    IMGO WDMA data format
*/
typedef enum{
    IMGO_FMT_YUV422_1P  = 0,
    IMGO_FMT_RAW8       = 8,
    IMGO_FMT_RAW10      = 9,
    IMGO_FMT_RAW12      = 10,
    IMGO_FMT_RAW14      = 11,
    IMGO_FMT_RAW8_2B    = 12,
    IMGO_FMT_RAW10_2B   = 13,
    IMGO_FMT_RAW12_2B   = 14,
    IMGO_FMT_RAW14_2B   = 15,
}E_CCU_CTL_IMGO_FMT;

/**
    RRZO WDMA data format
*/
typedef enum{
    RRZO_FMT_RAW8       = 0,
    RRZO_FMT_RAW10      = 1,
    RRZO_FMT_RAW12      = 2,
}E_CCU_CTL_RRZO_FMT;

/**
    TG data swap(YUV,RGB only)
*/
typedef enum{
    TG_SW_UYVY = 0,
    TG_SW_YUYV = 1,
    TG_SW_VYUY = 2,
    TG_SW_YVYU = 3,

    TG_SW_RGB  = 0,

    TG_SW_BGR  = 2,
}E_CCU_CTL_TG_SWAP;

/**
    bayer data pix pattern
*/
typedef enum{
    B_G_G_R     = 0,
    G_B_R_G     = 1,
    G_R_B_G     = 2,
    R_G_G_B     = 3,
}E_CCU_CTL_TG_PIXID;



//////////////////////////////////////////////////////////////////////////////////////////////
/**
    UNI function enable bit
*/
typedef enum{
    UNP2_A_EN_      = (1L<<0),
    QBIN3_A_EN_     = (1L<<1),
    SGG3_A_EN_      = (1L<<2),
    FLK_A_EN_       = (1L<<3),
    EIS_A_EN_       = (1L<<4),
    SGG2_A_EN_      = (1L<<5),
    HDS_A_EN_       = (1L<<6),
    RSS_A_EN_       = (1L<<7),
    GSE_A_EN_       = (1L<<8),
}ENUM_UNI_TOP_EN;

/**
    UNI DMA function enable
*/
typedef enum{
    RAWI_A_EN_  = (1L<<0),
    FLKO_A_EN_  = (1L<<1),
    EISO_A_EN_  = (1L<<2),
    RSSO_A_EN_  = (1L<<3),
}ENUM_UNI_TOP_DMA_EN;

/**
    UNI interrupt enable
*/
typedef enum{
    DMA_ERR_INT_EN_     = (1L<<0),
    FLKO_A_ERR_INT_EN_  = (1L<<1),
    EISO_A_ERR_INT_EN_  = (1L<<2),
    RSSO_A_ERR_INT_EN_  = (1L<<3),
    UNI_INT_WCLR_EN_    = (1L<<31),
}ENUM_UNI_TOP_DMA_INT_EN;


/**
    UNI multiplexer
*/
typedef enum{
    //HDS2_A_SEL
    HDS2_A_SEL_0    = (0),
    HDS2_A_SEL_1    = (1L<<0),
    //FLK2_A_SEL
    FLK2_A_SEL_0    = (0),
    FLK2_A_SEL_1    = (1L<<8),
}ENUM_UNI_TOP_SEL;

/**
   RAWI data format
*/
typedef enum{
    RAWI_FMT_RAW8       = 8,
    RAWI_FMT_RAW10      = 9,
    RAWI_FMT_RAW12      = 10,
    RAWI_FMT_RAW14      = 11,
    RAWI_FMT_RAW8_2B    = 12,
    RAWI_FMT_RAW10_2B   = 13,
    RAWI_FMT_RAW12_2B   = 14,
    RAWI_FMT_RAW14_2B   = 15,
}E_UNI_TOP_FMT;


//////////////////////////////////////////////////////////////////////////////////////////////
/**
    DIP interrupt status
*/

//DIP_CTL_CQ_INT_STATUS
typedef enum{
    ENUM_DIP_INT_CQ_THR0    = (1L<<0),
    ENUM_DIP_INT_CQ_THR1    = (1L<<1),
    ENUM_DIP_INT_CQ_THR2    = (1L<<2),
    ENUM_DIP_INT_CQ_THR3    = (1L<<3),
    ENUM_DIP_INT_CQ_THR4    = (1L<<4),
    ENUM_DIP_INT_CQ_THR5    = (1L<<5),
    ENUM_DIP_INT_CQ_THR6    = (1L<<6),
    ENUM_DIP_INT_CQ_THR7    = (1L<<7),
    ENUM_DIP_INT_CQ_THR8    = (1L<<8),
    ENUM_DIP_INT_CQ_THR9    = (1L<<9),
    ENUM_DIP_INT_CQ_THR10   = (1L<<10),
    ENUM_DIP_INT_CQ_THR11   = (1L<<11),
    ENUM_DIP_INT_CQ_THR12   = (1L<<12),
    ENUM_DIP_INT_CQ_THR13   = (1L<<13),
    ENUM_DIP_INT_CQ_THR14   = (1L<<14),
    ENUM_DIP_INT_CQ_THR15   = (1L<<15),
    ENUM_DIP_INT_CQ_THR16   = (1L<<16),
    ENUM_DIP_INT_CQ_THR17   = (1L<<17),
    ENUM_DIP_INT_CQ_THR18   = (1L<<18),
}ENUM_DIP_INT;


typedef enum{
    ENUM_DIP_CTL_DMA_EN_NONE            = (0),
    ENUM_DIP_CTL_DMA_EN_IMGI            = (1L<<0),
    ENUM_DIP_CTL_DMA_EN_IMGBI           = (1L<<1),
    ENUM_DIP_CTL_DMA_EN_IMGCI           = (1L<<2),
    ENUM_DIP_CTL_DMA_EN_UFDI            = (1L<<3),
    ENUM_DIP_CTL_DMA_EN_VIPI            = (1L<<4),
    ENUM_DIP_CTL_DMA_EN_VIP2I           = (1L<<5),
    ENUM_DIP_CTL_DMA_EN_VIP3I           = (1L<<6),
    ENUM_DIP_CTL_DMA_EN_LCEI            = (1L<<7),
    ENUM_DIP_CTL_DMA_EN_DEPI            = (1L<<8),
    ENUM_DIP_CTL_DMA_EN_DMGI            = (1L<<9),
    ENUM_DIP_CTL_DMA_EN_MFBO            = (1L<<10),
    ENUM_DIP_CTL_DMA_EN_IMG2O           = (1L<<11),
    ENUM_DIP_CTL_DMA_EN_IMG2BO          = (1L<<12),
    ENUM_DIP_CTL_DMA_EN_IMG3O           = (1L<<13),
    ENUM_DIP_CTL_DMA_EN_IMG3BO          = (1L<<14),
    ENUM_DIP_CTL_DMA_EN_IMG3CO          = (1L<<15),
    ENUM_DIP_CTL_DMA_EN_FEO             = (1L<<16),
    ENUM_DIP_CTL_DMA_EN_IMGXO_IMGO      = (1L<<20),  // not isp register = (reserved register),
    ENUM_DIP_CTL_DMA_EN_WROTO           = (1L<<21),  // not isp register = (reserved register),
    ENUM_DIP_CTL_DMA_EN_WDMAO           = (1L<<22),  // not isp register = (reserved register),
    ENUM_DIP_CTL_DMA_EN_VENC            = (1L<<23),  // not isp register = (reserved register),
}ENUM_DIP_CTL_DMA_EN;


typedef enum{
    ENUM_DIP_CTL_YUV_EN_MFB         = (1L<<0),
    ENUM_DIP_CTL_YUV_EN_C02B        = (1L<<1),
    ENUM_DIP_CTL_YUV_EN_C24         = (1L<<2),
    ENUM_DIP_CTL_YUV_EN_G2C         = (1L<<3),
    ENUM_DIP_CTL_YUV_EN_C42         = (1L<<4),
    ENUM_DIP_CTL_YUV_EN_NBC         = (1L<<5),
    ENUM_DIP_CTL_YUV_EN_NBC2        = (1L<<6),
    ENUM_DIP_CTL_YUV_EN_PCA         = (1L<<7),
    ENUM_DIP_CTL_YUV_EN_SEEE        = (1L<<8),
//  ENUM_DIP_CTL_YUV_EN_            = (1L<<9),
    ENUM_DIP_CTL_YUV_EN_NR3D        = (1L<<10),
    ENUM_DIP_CTL_YUV_EN_SL2B        = (1L<<11),
    ENUM_DIP_CTL_YUV_EN_SL2C        = (1L<<12),
    ENUM_DIP_CTL_YUV_EN_SL2D        = (1L<<13),
    ENUM_DIP_CTL_YUV_EN_SL2E        = (1L<<14),
    ENUM_DIP_CTL_YUV_EN_SRZ1        = (1L<<15),
    ENUM_DIP_CTL_YUV_EN_SRZ2        = (1L<<16),
    ENUM_DIP_CTL_YUV_EN_CRZ         = (1L<<17),
    ENUM_DIP_CTL_YUV_EN_MIX1        = (1L<<18),
    ENUM_DIP_CTL_YUV_EN_MIX2        = (1L<<19),
    ENUM_DIP_CTL_YUV_EN_MIX3        = (1L<<20),
    ENUM_DIP_CTL_YUV_EN_CRSP        = (1L<<21),
    ENUM_DIP_CTL_YUV_EN_C24B        = (1L<<22),
    ENUM_DIP_CTL_YUV_EN_MDPCROP     = (1L<<23),
    ENUM_DIP_CTL_YUV_EN_C02         = (1L<<24),
    ENUM_DIP_CTL_YUV_EN_FE          = (1L<<25),
    ENUM_DIP_CTL_YUV_EN_MFBW        = (1L<<26),
//  ENUM_DIP_CTL_YUV_EN_            = (1L<<27),
    ENUM_DIP_CTL_YUV_EN_PLNW1       = (1L<<28),
    ENUM_DIP_CTL_YUV_EN_PLNR1       = (1L<<29),
    ENUM_DIP_CTL_YUV_EN_PLNW2       = (1L<<30),
    ENUM_DIP_CTL_YUV_EN_PLNR2       = (1L<<31),
}ENUM_DIP_CTL_YUV_EN;

typedef enum{
    ENUM_DIP_CTL_YUV_EN_FM         = (1L<<0),
    ENUM_DIP_CTL_YUV_EN_SRZ3       = (1L<<1),
    ENUM_DIP_CTL_YUV_EN_SRZ4       = (1L<<2),
    ENUM_DIP_CTL_YUV_EN_SRZ5       = (1L<<3),
    ENUM_DIP_CTL_YUV_EN_HFG        = (1L<<4),
    ENUM_DIP_CTL_YUV_EN_SL2I       = (1L<<5),
}ENUM_DIP_CTL_YUV2_EN;



typedef enum{
    ENUM_DIP_CTL_RGB_EN_UNP         = (1L<<0),
    ENUM_DIP_CTL_RGB_EN_UFD         = (1L<<1),
    ENUM_DIP_CTL_RGB_EN_PGN         = (1L<<2),
    ENUM_DIP_CTL_RGB_EN_SL2         = (1L<<3),
    ENUM_DIP_CTL_RGB_EN_UDM         = (1L<<4),
    ENUM_DIP_CTL_RGB_EN_LCE         = (1L<<5),
    ENUM_DIP_CTL_RGB_EN_G2G         = (1L<<6),
    ENUM_DIP_CTL_RGB_EN_GGM         = (1L<<7),
    ENUM_DIP_CTL_RGB_EN_GDR1        = (1L<<8),
    ENUM_DIP_CTL_RGB_EN_GDR2        = (1L<<9),
    ENUM_DIP_CTL_RGB_EN_DBS2        = (1L<<10),
    ENUM_DIP_CTL_RGB_EN_OBC2        = (1L<<11),
    ENUM_DIP_CTL_RGB_EN_RMG2        = (1L<<12),
    ENUM_DIP_CTL_RGB_EN_BNR2        = (1L<<13),
    ENUM_DIP_CTL_RGB_EN_RMM2        = (1L<<14),
    ENUM_DIP_CTL_RGB_EN_LSC2        = (1L<<15),
    ENUM_DIP_CTL_RGB_EN_RCP2        = (1L<<16),
    ENUM_DIP_CTL_RGB_EN_PAK2        = (1L<<17),
    ENUM_DIP_CTL_RGB_EN_RNR         = (1L<<18),
    ENUM_DIP_CTL_RGB_EN_SL2G        = (1L<<19),
    ENUM_DIP_CTL_RGB_EN_SL2H        = (1L<<20),
}ENUM_DIP_CTL_RGB_EN;


typedef enum{
    CCU_CTL_CQ_EN_P2_CQ1_EN                      = (1L<<0),
    CCU_CTL_CQ_EN_P2_CQ2_EN                      = (1L<<1),
    CCU_CTL_CQ_EN_P2_CQ3_EN                      = (1L<<2),
}ENUM_CCU_CTL_CQ_EN_P2;


//////////////////////////////////////////////////////////////////////////////////////////////
/**
    camsv interrupt
*/
#define CCUSV_INT_STATUS_VS1(CCUSV_PATH)                (1L<<0)
#define CCUSV_INT_STATUS_TG_SOF1(CCUSV_PATH)            (1L<<7)
#define CCUSV_INT_STATUS_PASS1_DON(CCUSV_PATH)          (1L<<10)

/**
    SV TG data format
*/
typedef enum{
    SV_TG_FMT_RAW8      = 0,
    SV_TG_FMT_RAW10     = 1,
    SV_TG_FMT_RAW12     = 2,
    SV_TG_FMT_YUV422    = 3,
    SV_TG_FMT_RAW14     = 4,
    SV_TG_FMT_JPG       = 7,
}E_CCUSV_TG_FMT;

/**
    SV PAK mode
*/
typedef enum{
    SV_PAK_MODE_RAW8      = 0,
    SV_PAK_MODE_RAW10     = 1,
    SV_PAK_MODE_RAW12     = 2,
    SV_PAK_MODE_RAW14     = 3,
}E_CCUSV_PAK_MODE;

/**
    SV IMGO WDMA data format
*/
typedef enum{
    SV_IMGO_STRIDE_FMT_OTHERS      = 0,
    SV_IMGO_STRIDE_FMT_YUV         = 1,
    SV_IMGO_STRIDE_FMT_JPG      = 2,
}E_CCUSV_IMGO_STRIDE_FMT;

/**
    SV IMGO WDMA bus size
*/
typedef enum{
    SV_DMA_BUS_SIZE_BIT8      = 0,
    SV_DMA_BUS_SIZE_BIT16     = 1,
    SV_DMA_BUS_SIZE_BIT24     = 2,
    SV_DMA_BUS_SIZE_BIT32     = 3,
    SV_DMA_BUS_SIZE_BIT64     = 7,
}E_CCUSV_DMA_BUS_SIZE;

/**
    SV IMGO WDMA data format
*/
typedef enum{
    SV_IMGO_FMT_RAW14   = 0,
    SV_IMGO_FMT_RAW12   = 1,
    SV_IMGO_FMT_RAW10   = 2,
    SV_IMGO_FMT_RAW8    = 3,
}E_CCUSV_IMGO_FMT;

/**
    SV TG data swap(yuv,rgb only)
*/

typedef enum{
    SV_TG_SW_UYVY   = 0,
    SV_TG_SW_YUYV   = 1,
    SV_TG_SW_VYUY   = 2,
    SV_TG_SW_YVYU   = 3,

    SV_TG_SW_RGB    = 0,
    SV_TG_SW_BGR    = 2,
}E_CCUSV_TG_SWAP;

/**
    SV A function enable
*/
typedef enum{
    CCUSV_MODULE_TG_EN                    = (1L<<0),
    CCUSV_MODULE_PAK_EN                   = (1L<<2),
    CCUSV_MODULE_PAK_SEL                  = (1L<<3),
    CCUSV_MODULE_IMGO_EN                  = (1L<<4),
    CCUSV_MODULE_DB_EN                    = (1L<<30),
    CCUSV_MODULE_DB_LOCK                  = (1L<<31),
}ENUM_CCUSV_CCUSV_MODULE_EN;

/**
    SV A interrupt enable
*/
typedef enum{
    CCUSV_INT_EN_VS1_INT_EN               = (1L<<0),
    CCUSV_INT_EN_TG_INT1_EN               = (1L<<1),
    CCUSV_INT_EN_TG_INT2_EN               = (1L<<2),
    CCUSV_INT_EN_EXPDON1_INT_EN           = (1L<<3),
    CCUSV_INT_EN_TG_ERR_INT_EN            = (1L<<4),
    CCUSV_INT_EN_TG_GBERR_INT_EN          = (1L<<5),
    CCUSV_INT_EN_TG_DROP_INT_EN           = (1L<<6),
    CCUSV_INT_EN_TG_SOF_INT_EN            = (1L<<7),
    CCUSV_INT_EN_PASS1_DON_INT_EN         = (1L<<10),
    CCUSV_INT_EN_IMGO_ERR_INT_EN          = (1L<<16),
    CCUSV_INT_EN_IMGO_OVERR_INT_EN        = (1L<<17),
    CCUSV_INT_EN_IMGO_DROP_INT_EN         = (1L<<19),
    CCUSV_INT_EN_SW_PASS1_DON_INT_EN      = (1L<<20),
    CCUSV_INT_EN_INT_WCLR_EN              = (1L<<31),
}ENUM_CCUSV_CCUSV_INT_EN;

/**
    CCUSV interrupt status
*/

typedef enum{
    SV_VS1_ST              = (1L<<0),
    SV_TG_INT1_ST          = (1L<<1),
    SV_TG_INT2_ST          = (1L<<2),
    SV_EXPDON_ST           = (1L<<3),
    SV_SOF_INT_ST          = (1L<<7),
    SV_HW_PASS1_DON_ST     = (1L<<10),
    SV_SW_PASS1_DON_ST     = (1L<<20)
}ENUM_CCUSV_INT;

/**
    SV B function enable
*/
typedef enum{
    CCUSV_MODULE_TG_EN_B                 = (1L<<0),
    CCUSV_MODULE_PAK_EN_B                = (1L<<2),
    CCUSV_MODULE_PAK_SEL_B               = (1L<<3),
    CCUSV_MODULE_IMGO_EN_B               = (1L<<4),
    CCUSV_MODULE_DB_EN_B                 = (1L<<30),
    CCUSV_MODULE_DB_LOCK_B               = (1L<<31),
}ENUM_CCUSV_CCUSV_MODULE_EN_B;

/**
    SV B interrupt enable
*/
typedef enum{
    CCUSV_INT_EN_VS1_INT_EN_B            = (1L<<0),
    CCUSV_INT_EN_TG_INT1_EN_B            = (1L<<1),
    CCUSV_INT_EN_TG_INT2_EN_B            = (1L<<2),
    CCUSV_INT_EN_EXPDON1_INT_EN_B        = (1L<<3),
    CCUSV_INT_EN_TG_ERR_INT_EN_B         = (1L<<4),
    CCUSV_INT_EN_TG_GBERR_INT_EN_B       = (1L<<5),
    CCUSV_INT_EN_TG_DROP_INT_EN_B        = (1L<<6),
    CCUSV_INT_EN_TG_SOF_INT_EN_B         = (1L<<7),
    CCUSV_INT_EN_PASS1_DON_INT_EN_B      = (1L<<10),
    CCUSV_INT_EN_IMGO_ERR_INT_EN_B       = (1L<<16),
    CCUSV_INT_EN_IMGO_OVERR_INT_EN_B     = (1L<<17),
    CCUSV_INT_EN_IMGO_DROP_INT_EN_B      = (1L<<19),
    CCUSV_INT_EN_SW_PASS1_DON_INT_EN_B   = (1L<<20),
    CCUSV_INT_EN_INT_WCLR_EN_B           = (1L<<31),
}ENUM_CCUSV_CCUSV_INT_EN_B;

typedef enum{
    _1_pix_ = 0,
    _2_pix_ = 1,
    _4_pix_ = 2,
}E_PIX_MODE;
#endif

#endif

