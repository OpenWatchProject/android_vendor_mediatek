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


#ifndef _ISP_DRV_STDDEF_H_
#define _ISP_DRV_STDDEF_H_

//////////////////////////////////////////////////////////////////////////////////////////////
/**
    CAM path: hw image header depth
*/
#define CAM_IMAGE_HEADER    (11) //TimeStamp LSB and MSB can't be written. 13 spare registers are need to minus two spare registers.

/**
    CQ SW maximum ring depth.
*/
#define MAX_RING_SIZE   16

/**
    for AFO hw constraint under twin mode
    note: 128 for vertical direction
*/
#define AFO_NORMAL_SZ       (48)
#define AF_WIN_H            (128)
#define AF_WIN_V            (128)
#define AFO_MAX_SIZE        (AF_WIN_H * AF_WIN_V * AFO_NORMAL_SZ)
#define AFO_DUMMY_STRIDE    (1 * AFO_NORMAL_SZ)
#define AFO_DUMMY_WIN       (AFO_DUMMY_STRIDE * AF_WIN_V)

/**
    for AAO
*/
#define AA_WIN_H     (128)
#define AA_WIN_V     (128)
#define AWB_WIN      (128)
#define AE_WIN       (16)
#define AE_OE_WIN    (32)
#define AE_HIST_BIN  (256)
#define AE_HIST_BINS (24)
#define AE_HIST_NUM  (6)
#define AE_HIST_TYPE (2)
#define AAO_MAX_SIZE ((AA_WIN_H*AA_WIN_V*(AWB_WIN+AE_WIN+AE_OE_WIN) + AE_HIST_TYPE*AE_HIST_BIN*AE_HIST_BINS*AE_HIST_NUM)/8)

/**
    ISP5.0/6.0 hw bug
    Bin_mode_0 donimate all other bins' bin_number.
    if Bin_mode_0 = 1, bin_mode_1 = 0 => bin_mode_1 will be also 256 bin number , but content is 128 bin's result. (content will have error)
*/
#define AE_LE_BIN_MODE_WORK_AROUND 1
#define AE_SE_BIN_MODE_WORK_AROUND 1

#if (AE_LE_BIN_MODE_WORK_AROUND == 1)
#define _AE_LE_BIN_SIZE(ptr,slot) {\
    slot[0] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_HST_SET,AA_AE_PIX_BIN_MODE_0);\
    for(MUINT32 _cnt=1;_cnt<AE_HIST_NUM;_cnt++)\
        slot[_cnt] = slot[0];\
}
#else
#define _AE_LE_BIN_SIZE(ptr,slot) {\
    slot[0] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_HST_SET,AA_AE_PIX_BIN_MODE_0);\
    slot[1] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_HST_SET,AA_AE_PIX_BIN_MODE_1);\
    slot[2] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_HST_SET,AA_AE_PIX_BIN_MODE_2);\
    slot[3] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_HST_SET,AA_AE_PIX_BIN_MODE_3);\
    slot[4] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_HST_SET_1,AA_AE_PIX_BIN_MODE_4);\
    slot[5] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_HST_SET_1,AA_AE_PIX_BIN_MODE_5);\
}
#endif

#if (AE_SE_BIN_MODE_WORK_AROUND == 1)
#define _AE_SE_BIN_SIZE(ptr,slot) {\
    slot[0] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_SE_HST_SET,AA_AE_PIX_SE_BIN_MODE_0);\
    for(MUINT32 _cnt=1;_cnt<AE_HIST_NUM;_cnt++)\
        slot[_cnt] = slot[0];\
}
#else
#define _AE_SE_BIN_SIZE(ptr,slot) {\
    slot[0] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_SE_HST_SET,AA_AE_PIX_SE_BIN_MODE_0);\
    slot[1] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_SE_HST_SET,AA_AE_PIX_SE_BIN_MODE_1);\
    slot[2] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_SE_HST_SET,AA_AE_PIX_SE_BIN_MODE_2);\
    slot[3] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_SE_HST_SET,AA_AE_PIX_SE_BIN_MODE_3);\
    slot[4] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_SE_HST_SET_1,AA_AE_PIX_SE_BIN_MODE_4);\
    slot[5] = CAM_READ_BITS(ptr,AA_R1_AA_AE_PIX_SE_HST_SET_1,AA_AE_PIX_SE_BIN_MODE_5);\
}
#endif

#define AE_LE_BIN_SIZE(ptr) ({\
    MUINT32 _tmp = 0, _size = 0;\
    MUINT32 slot[AE_HIST_NUM];\
    _AE_LE_BIN_SIZE(ptr,slot);\
    _tmp = CAM_READ_REG(ptr,AA_R1_AA_AE_PIX_HST_CTL);\
    if(CAM_READ_BITS(ptr,AA_R1_AA_AE_STAT_EN,AA_AE_HST_DMA_OUT_EN)){\
        for(MUINT32 _cnt=0;_cnt<AE_HIST_NUM;_cnt++){\
            if(_tmp & 0x1){\
                if(slot[_cnt] == 0)\
                    _size += (AE_HIST_BINS*(AE_HIST_BIN/2));\
                else\
                    _size += (AE_HIST_BINS*AE_HIST_BIN);\
            }\
            _tmp = _tmp >> 1;\
        }\
    }\
    _size;})

#define AE_SE_BIN_SIZE(ptr) ({\
    MUINT32 _tmp = 0, _size = 0;\
    MUINT32 slot[AE_HIST_NUM];\
    _AE_SE_BIN_SIZE(ptr,slot);\
    _tmp = CAM_READ_REG(ptr,AA_R1_AA_AE_PIX_HST_CTL);\
    _tmp = _tmp >> 8;\
    if(CAM_READ_BITS(ptr,AA_R1_AA_AE_STAT_EN,AA_AE_HST_DMA_OUT_EN)){\
        for(MUINT32 _cnt=0;_cnt<AE_HIST_NUM;_cnt++){\
            if(_tmp & 0x1){\
                if(slot[_cnt] == 0)\
                    _size += (AE_HIST_BINS*(AE_HIST_BIN/2));\
                else\
                    _size += (AE_HIST_BINS*AE_HIST_BIN);\
            }\
            _tmp = _tmp >> 1;\
        }\
    }\
    _size;})

/**
    for TSFSO
*/
#define TSFS_WIN_H     (128)
#define TSFS_WIN_V     (128)
#define TSFS_CHANNEL   (3) //R/G/B
#define TSFSO_MAX_SIZE (TSFS_WIN_H*TSFS_WIN_V*TSFS_CHANNEL*4)//4byte per channel

/**
   for FLKO
   NOTE: FLK_WIN_YNUM*FLK_WIN_YSIZE <= ImgH
*/
#define FLK_WIN_XNUM    (6)
#define FLK_PIX_B       (6)//byte per pixel
#define FLKO_MAX_SIZE(ImgH) (ImgH*FLK_WIN_XNUM*FLK_PIX_B)

/**
    for PDO
    pdo data size is 2 byte per pix
*/
#define PDO_STRIDE_ALIGN(W) ({\
    MUINT32 _align = 0;\
    if(((W) % 16) != 0){\
        _align = (((W) + 15)/16 * 16);\
    }\
    else{\
        _align = (W);\
    }\
    _align;\
})
#define PDO_PIX_B       (2)
#define PDO_STRIDE_CONS(W) ( (PDO_STRIDE_ALIGN(W) + 16)*PDO_PIX_B)
#define PDO_MAX_SIZE(W,H) (PDO_STRIDE_CONS(W) * H)
/* pbin block size is 16*8, one pixel has L&R */
#define PBIN_WIDTH(W) (((W) >> 4) << 1)
#define PBIN_HEIGHT(H) ((H) >> 3)

//////////////////////////////////////////////////////////////////////////////////////////////

/**
    CAM interrupt status
*/
//CAM_CTL_RAW_INT_STATUSX
typedef enum{
    VS_INT_ST            = (1L<<0),
    TG_INT1_ST           = (1L<<1),
    TG_INT2_ST           = (1L<<2),
    EXPDON_ST            = (1L<<3),
    TG_ERR_ST            = (1L<<4),
    TG_GBERR_ST          = (1L<<5),
    SOF_INT_ST           = (1L<<6),
    SOF_WAIT_ST          = (1L<<7),
    DCIF_SOF_INT_ST      = (1L<<8),
    CQ_CODE_ERR_ST       = (1L<<9),
    CQ_APB_ERR_ST        = (1L<<10),
    CQ_VS_ERR_ST         = (1L<<11),
    HW_PASS1_DON_ST      = (1L<<12),
    RAW_PASS1_DON_ST     = (1L<<13),
    SW_PASS1_DON_ST      = (1L<<14),
    AA_R1_READ_READY_ST  = (1L<<15),
    AA_R1_READ_ERROR_ST  = (1L<<16),
    MAE_R1_READ_READY_ST = (1L<<17),
    MAE_R1_READ_ERROR_ST = (1L<<18),
    GGM_R1_COLL_INT_ST   = (1L<<19),
    GGM_R2_COLL_INT_ST   = (1L<<20),
    YNRS_R1_COLL_INT_ST  = (1L<<21),
    AF_R1_WIN_INT_ST     = (1L<<22),
    BPCI_R1_DONE_ST      = (1L<<23),
    BPCI_R2_DONE_ST      = (1L<<24),
    BPCI_R3_DONE_ST      = (1L<<25),
    BPCI_R4_DONE_ST      = (1L<<26),
    PDI_R1_DONE_ST       = (1L<<27),
    MLSCI_R1_DONE_ST     = (1L<<28),
    DMA_ERR_ST           = (1L<<29)
}ENUM_CAM_INT;

//CAM_CTL_RAW_INT2_STATUS
typedef enum{
    IMGO_DONE_ST     = (1L<<0),
    LTMSO_DONE_ST    = (1L<<1),
    RRZO_DONE_ST     = (1L<<2),
    LCESHO_DONE_ST   = (1L<<3),
    LCESO_DONE_ST    = (1L<<4),
    AAO_DONE_ST      = (1L<<5),
    FLKO_DONE_ST     = (1L<<6),
    LHSO_DONE_ST     = (1L<<7),
    RSSO_R2_DONE_ST  = (1L<<8),
    UFEO_DONE_ST     = (1L<<9),
    AFO_DONE_ST      = (1L<<10),
    UFGO_DONE_ST     = (1L<<11),
    RSSO_DONE_ST     = (1L<<12),
    LMVO_DONE_ST     = (1L<<13),
    YUVO_DONE_ST     = (1L<<14),
    YUVBO_DONE_ST    = (1L<<15),
    YUVCO_DONE_ST    = (1L<<16),
    TSFSO_DONE_ST    = (1L<<17),
    PDO_DONE_ST      = (1L<<18),
    MQEO_DONE_ST     = (1L<<19),
    MAEO_DONE_ST     = (1L<<20),
    BPCO_DONE_ST     = (1L<<21),
    CRZO_DONE_ST     = (1L<<22),
    CRZBO_DONE_ST    = (1L<<23),
    CRZO_R2_DONE_ST  = (1L<<24),
    CRZBO_R2_DONE_ST = (1L<<25),
    RAWI_R2_DONE_ST  = (1L<<26),
    RAWI_R3_DONE_ST  = (1L<<27),
    RAWI_R4_DONE_ST  = (1L<<28),
    RAWI_R5_DONE_ST  = (1L<<29),
    UFDI_R2_DONE_ST  = (1L<<30),
    LSCI_R1_DONE_ST  = (1L<<31)
}ENUM_CAM_DMA_INT;

/**
    CAM A function enable bit
*/
typedef enum{
    TG_R1_EN_    = (1L<<0),
    SEP_R1_EN_   = (1L<<1),
    BPC_R1_EN_   = (1L<<2),
    OBC_R1_EN_   = (1L<<3),
    FUS_R1_EN_   = (1L<<4),
    DGN_R1_EN_   = (1L<<5),
    LSC_R1_EN_   = (1L<<6),
    WB_R1_EN_    = (1L<<7),
    LTM_R1_EN_   = (1L<<8),
    MRG_R7_EN_   = (1L<<9),
    PAKG_R1_EN_  = (1L<<10),
    MRG_R5_EN_   = (1L<<11),
    LCES_R1_EN_  = (1L<<12),
    QBN_R4_EN_   = (1L<<13),
    MRG_R4_EN_   = (1L<<14),
    LTMS_R1_EN_  = (1L<<15),
    MRG_R1_EN_   = (1L<<16),
    AA_R1_EN_    = (1L<<17),
    FLK_R1_EN_   = (1L<<18),
    CRP_R3_EN_   = (1L<<19),
    PAK_R1_EN_   = (1L<<20),
    UNP_R2_EN_   = (1L<<21),
    MRG_R10_EN_  = (1L<<22),
    BS_R1_EN_    = (1L<<23),
    BS_R2_EN_    = (1L<<24),
    BS_R3_EN_    = (1L<<25),
    MOBC_R2_EN_  = (1L<<26),
    MOBC_R3_EN_  = (1L<<27),
    SLK_R1_EN_   = (1L<<28)
}ENUM_CAM_CTL_EN;

typedef enum{
    DBN_R1_EN_   = (1L<<0),
    BIN_R1_EN_   = (1L<<1),
    FBND_R1_EN_  = (1L<<2),
    FCIRC_R1_EN_ = (1L<<3),
    BPC_R2_EN_   = (1L<<4),
    OBC_R2_EN_   = (1L<<5),
    ZFUS_R1_EN_  = (1L<<6),
    HLR_R1_EN_   = (1L<<7),
    RRZ_R1_EN_   = (1L<<8),
    UFG_R1_EN_   = (1L<<9),
    QBN_R5_EN_   = (1L<<10),
    GSE_R1_EN_   = (1L<<11),
    MRG_R8_EN_   = (1L<<12),
    HDS_R1_EN_   = (1L<<13),
    RSS_R1_EN_   = (1L<<14),
    LMV_R1_EN_   = (1L<<15),
    CRP_R1_EN_   = (1L<<16),
    AF_R1_EN_    = (1L<<17),
    QBN_R1_EN_   = (1L<<18),
    MRG_R2_EN_   = (1L<<19),
    QBN_R2_EN_   = (1L<<20),
    TSFS_R1_EN_  = (1L<<21),
    LHS_R1_EN_   = (1L<<22),
    PDE_R1_EN_   = (1L<<23),
    CRP_R8_EN_   = (1L<<24),
    PBN_R1_EN_   = (1L<<25),
    SLK_R2_EN_   = (1L<<26),
    DM_R1_EN_    = (1L<<27),
    CCM_R1_EN_   = (1L<<28),
    GGM_R1_EN_   = (1L<<29),
    G2C_R1_EN_   = (1L<<30),
    C42_R1_EN_   = (1L<<31)
}ENUM_CAM_CTL_EN2;

typedef enum{
    CRSP_R1_EN_ = (1L<<0),
    PLNW_R1_EN_ = (1L<<1),
    PAK_R3_EN_  = (1L<<2),
    PAK_R4_EN_  = (1L<<3),
    MQE_R1_EN_  = (1L<<4),
    MRG_R9_EN_  = (1L<<5),
    MOBC_R1_EN_ = (1L<<6),
    MLSC_R1_EN_ = (1L<<7),
    PAK_R2_EN_  = (1L<<8),
    MAE_R1_EN_  = (1L<<9),
    UFE_R1_EN_  = (1L<<10),
    MRG_R6_EN_  = (1L<<11),
    QBN_R3_EN_  = (1L<<12),
    YNRS_R1_EN_ = (1L<<13),
    FLC_R1_EN_  = (1L<<14),
    CRZ_R1_EN_  = (1L<<15),
    PLNW_R2_EN_ = (1L<<16),
    MRG_R11_EN_ = (1L<<17),
    MRG_R12_EN_ = (1L<<18),
    MRG_R13_EN_ = (1L<<19),
    MRG_R14_EN_ = (1L<<20),
    CRP_R4_EN_  = (1L<<21),
    GGM_R2_EN_  = (1L<<22),
    G2C_R2_EN_  = (1L<<23),
    C42_R2_EN_  = (1L<<24),
    RSS_R2_EN_  = (1L<<25),
    CRZ_R2_EN_  = (1L<<26),
    PLNW_R3_EN_ = (1L<<27),
    PAK_R5_EN_  = (1L<<28),
    UFD_R2_EN_  = (1L<<29),
    CRSP_R2_EN_ = (1L<<30)
}ENUM_CAM_CTL_EN3;

typedef enum{
    DCPN_R1_EN_ = (1L<<0),
    IBPC_R1_EN_ = (1L<<1),
    IOBC_R1_EN_ = (1L<<2),
    IRM_R1_EN_  = (1L<<3),
    RNR_R1_EN_  = (1L<<4),
    UNP_R3_EN_  = (1L<<5),
    UNP_R4_EN_  = (1L<<6),
    UNP_R5_EN_  = (1L<<7),
    BPC_R3_EN_  = (1L<<8),
    OBC_R3_EN_  = (1L<<9),
    BPC_R4_EN_  = (1L<<10),
    OBC_R4_EN_  = (1L<<11),
    GBN_R3_EN_  = (1L<<12),
    CRP_R7_EN_  = (1L<<13),
    GBN_R1_EN_  = (1L<<14),
    GBN_R2_EN_  = (1L<<15),
    MRG_R3_EN_  = (1L<<16),
    GBN_R4_EN_  = (1L<<17),
    CRP_R2_EN_  = (1L<<18),
    RLB_R1_EN_  = (1L<<19),
    ADL_R1_EN_  = (1L<<20)
}ENUM_CAM_CTL_EN4;

/**
    CAM DMA function enable
*/

typedef enum{
    IMGO_R1_EN_   = (1L<<0),
    LTMSO_R1_EN_  = (1L<<1),
    RRZO_R1_EN_   = (1L<<2),
    RAWI_R2_EN_   = (1L<<3),
    LCESO_R1_EN_  = (1L<<4),
    AAO_R1_EN_    = (1L<<5),
    FLKO_R1_EN_   = (1L<<6),
    BPCI_R1_EN_   = (1L<<7),
    LSCI_R1_EN_   = (1L<<8),
    UFEO_R1_EN_   = (1L<<9),
    AFO_R1_EN_    = (1L<<10),
    UFGO_R1_EN_   = (1L<<11),
    RSSO_R1_EN_   = (1L<<12),
    LMVO_R1_EN_   = (1L<<13),
    YUVBO_R1_EN_  = (1L<<14),
    TSFSO_R1_EN_  = (1L<<15),
    PDO_R1_EN_    = (1L<<16),
    MQEO_R1_EN_   = (1L<<17),
    MAEO_R1_EN_   = (1L<<18),
    BPCI_R2_EN_   = (1L<<19),
    BPCI_R3_EN_   = (1L<<20),
    BPCI_R4_EN_   = (1L<<21),
    RAWI_R3_EN_   = (1L<<22),
    RAWI_R4_EN_   = (1L<<23),
    RAWI_R5_EN_   = (1L<<24),
    PDI_R1_EN_    = (1L<<25),
    CRZO_R1_EN_   = (1L<<26),
    CRZBO_R1_EN_  = (1L<<27),
    UFDI_R2_EN_   = (1L<<28),
    BPCO_R1_EN_   = (1L<<29),
    LHSO_R1_EN_   = (1L<<30),
    YUVCO_R1_EN_  = (1L<<31)
}ENUM_CAM_CTL_DMA_EN;

#define WDMA_EN_MASK (                                  \
    IMGO_R1_EN_     | \
    LTMSO_R1_EN_    | \
    RRZO_R1_EN_     | \
    LCESO_R1_EN_    | \
    AAO_R1_EN_      | \
    FLKO_R1_EN_     | \
    UFEO_R1_EN_     | \
    AFO_R1_EN_      | \
    UFGO_R1_EN_     | \
    RSSO_R1_EN_     | \
    LMVO_R1_EN_     | \
    YUVBO_R1_EN_    | \
    TSFSO_R1_EN_    | \
    PDO_R1_EN_      | \
    CRZO_R1_EN_     | \
    CRZBO_R1_EN_    | \
    YUVCO_R1_EN_    \
    )

/**
 *    CAMSV DMA reset status
 */
#define IMGO_SOFT_RST_STAT        (1L<<0)
#define RRZO_SOFT_RST_STAT        (1L<<1)
#define AAO_SOFT_RST_STAT         (1L<<2)
#define AFO_SOFT_RST_STAT         (1L<<3)
#define LCSO_SOFT_RST_STAT        (1L<<4)
#define UFEO_SOFT_RST_STAT        (1L<<5)
#define PDO_SOFT_RST_STAT         (1L<<6)
#define BPCI_SOFT_RST_STAT        (1L<<16)
#define CACI_SOFT_RST_STAT        (1L<<17)
#define LSCI_SOFT_RST_STAT        (1L<<18)

/**
 *    dma IMGO/UFO done mask
 */
#define DMA_ST_MASK_CAMSV_IMGO_OR_UFO     (              \
        IMGO_SOFT_RST_STAT |\
        UFEO_SOFT_RST_STAT)

typedef enum{
    LCESHO_R1_EN_ = (1L<<0),
    CRZO_R2_EN_   = (1L<<1),
    CRZBO_R2_EN_  = (1L<<2),
    RSSO_R2_EN_   = (1L<<3),
    MLSCI_R1_EN_  = (1L<<4),
    YUVO_R1_EN_   = (1L<<5)
}ENUM_CAM_CTL_DMA2_EN;

#define WDMA2_EN_MASK ( \
    CRZO_R2_EN_     | \
    CRZBO_R2_EN_    | \
    RSSO_R2_EN_     | \
    YUVO_R1_EN_     \
)

/**
    CAM interrupt enable
*/
typedef enum{
    VS_INT_EN_            =(1L<<0),
    TG_INT1_EN_           =(1L<<1),
    TG_INT2_EN_           =(1L<<2),
    EXPDON_EN_            =(1L<<3),
    TG_ERR_EN_            =(1L<<4),
    TG_GBERR_EN_          =(1L<<5),
    SOF_INT_EN_           =(1L<<6),
    SOF_WAIT_EN_          =(1L<<7),
    DCIF_SOF_INT_EN_      =(1L<<8),
    CQ_CODE_ERR_EN_       =(1L<<9),
    CQ_APB_ERR_EN_        =(1L<<10),
    CQ_VS_ERR_EN_         =(1L<<11),
    HW_PASS1_DON_EN_      =(1L<<12),
    RAW_PASS1_DON_EN_     =(1L<<13),
    SW_PASS1_DON_EN_      =(1L<<14),
    AA_R1_READ_READY_EN_  =(1L<<15),
    AA_R1_READ_ERROR_EN_  =(1L<<16),
    MAE_R1_READ_READY_EN_ =(1L<<17),
    MAE_R1_READ_ERROR_EN_ =(1L<<18),
    GGM_R1_COLL_INT_EN_   =(1L<<19),
    GGM_R2_COLL_INT_EN_   =(1L<<20),
    YNRS_R1_COLL_INT_EN_  =(1L<<21),
    AF_R1_WIN_INT_EN_     =(1L<<22),
    BPCI_R1_DONE_EN_      =(1L<<23),
    BPCI_R2_DONE_EN_      =(1L<<24),
    BPCI_R3_DONE_EN_      =(1L<<25),
    BPCI_R4_DONE_EN_      =(1L<<26),
    PDI_R1_DONE_EN_       =(1L<<27),
    MLSCI_R1_DONE_EN_     =(1L<<28),
    DMA_ERR_EN_           =(1L<<29),
    INT_WCLR_EN_          =(1L<<31)
}ENUM_CAM_CTL_RAW_INT_EN;

/**
    CAM DMA interrupt enable
*/
typedef enum{
    IMGO_R1_DONE_EN_   = (1L<<0),
    LTMSO_R1_DONE_EN_  = (1L<<1),
    RRZO_R1_DONE_EN_   = (1L<<2),
    LCESHO_R1_DONE_EN_ = (1L<<3),
    LCESO_R1_DONE_EN_  = (1L<<4),
    AAO_R1_DONE_EN_    = (1L<<5),
    FLKO_R1_DONE_EN_   = (1L<<6),
    LHSO_R1_DONE_EN_   = (1L<<7),
    RSSO_R2_DONE_EN_   = (1L<<8),
    UFEO_R1_DONE_EN_   = (1L<<9),
    AFO_R1_DONE_EN_    = (1L<<10),
    UFGO_R1_DONE_EN_   = (1L<<11),
    RSSO_R1_DONE_EN_   = (1L<<12),
    LMVO_R1_DONE_EN_   = (1L<<13),
    YUVO_R1_DONE_EN_   = (1L<<14),
    YUVBO_R1_DONE_EN_  = (1L<<15),
    YUVCO_R1_DONE_EN_  = (1L<<16),
    TSFSO_R1_DONE_EN_  = (1L<<17),
    PDO_R1_DONE_EN_    = (1L<<18),
    MQEO_R1_DONE_EN_   = (1L<<19),
    MAEO_R1_DONE_EN_   = (1L<<20),
    BPCO_R1_DONE_EN_   = (1L<<21),
    CRZO_R1_DONE_EN_   = (1L<<22),
    CRZBO_R1_DONE_EN_  = (1L<<23),
    CRZO_R2_DONE_EN_   = (1L<<24),
    CRZBO_R2_DONE_EN_  = (1L<<25),
    RAWI_R2_DONE_EN_   = (1L<<26),
    RAWI_R3_DONE_EN_   = (1L<<27),
    RAWI_R4_DONE_EN_   = (1L<<28),
    RAWI_R5_DONE_EN_   = (1L<<29),
    UFDI_R2_DONE_EN_   = (1L<<30),
    LSCI_R1_DONE_EN_   = (1L<<31)
}ENUM_CAM_CTL_RAW_INT2_EN;

/**
    CAM DMA interrupt enable
*/
typedef enum{
     IMGO_R1_DROP_FRAME_EN_   = (1L<<0),
     LTMSO_R1_DROP_FRAME_EN_  = (1L<<1),
     RRZO_R1_DROP_FRAME_EN_   = (1L<<2),
     LCESHO_R1_DROP_FRAME_EN_ = (1L<<3),
     LCESO_R1_DROP_FRAME_EN_  = (1L<<4),
     AAO_R1_DROP_FRAME_EN_    = (1L<<5),
     FLKO_R1_DROP_FRAME_EN_   = (1L<<6),
     LHSO_R1_DROP_FRAME_EN_   = (1L<<7),
     RSSO_R2_DROP_FRAME_EN_   = (1L<<8),
     UFEO_R1_DROP_FRAME_EN_   = (1L<<9),
     AFO_R1_DROP_FRAME_EN_    = (1L<<10),
     UFGO_R1_DROP_FRAME_EN_   = (1L<<11),
     RSSO_R1_DROP_FRAME_EN_   = (1L<<12),
     LMVO_R1_DROP_FRAME_EN_   = (1L<<13),
     YUVO_R1_DROP_FRAME_EN_   = (1L<<14),
     YUVBO_R1_DROP_FRAME_EN_  = (1L<<15),
     YUVCO_R1_DROP_FRAME_EN_  = (1L<<16),
     TSFSO_R1_DROP_FRAME_EN_  = (1L<<17),
     PDO_R1_DROP_FRAME_EN_    = (1L<<18),
     MQEO_R1_DROP_FRAME_EN_   = (1L<<19),
     MAEO_R1_DROP_FRAME_EN_   = (1L<<20),
     BPCO_R1_DROP_FRAME_EN_   = (1L<<21),
     CRZO_R1_DROP_FRAME_EN_   = (1L<<22),
     CRZBO_R1_DROP_FRAME_EN_  = (1L<<23),
     CRZO_R2_DROP_FRAME_EN_   = (1L<<24),
     CRZBO_R2_DROP_FRAME_EN_  = (1L<<25),
     MLSC_R1_FIFO_UFLOW_EN_   = (1L<<30),
     LSC_R1_FIFO_UFLOW_EN_    = (1L<<31)
}ENUM_CAM_CTL_RAW_INT3_EN;

typedef enum{
    IMGO_R1_ERR_EN_   =(1L<<0),
    LTMSO_R1_ERR_EN_  =(1L<<1),
    RRZO_R1_ERR_EN_   =(1L<<2),
    LCESHO_R1_ERR_EN_ =(1L<<3),
    LCESO_R1_ERR_EN_  =(1L<<4),
    AAO_R1_ERR_EN_    =(1L<<5),
    FLKO_R1_ERR_EN_   =(1L<<6),
    LHSO_R1_ERR_EN_   =(1L<<7),
    RSSO_R2_ERR_EN_   =(1L<<8),
    UFEO_R1_ERR_EN_   =(1L<<9),
    AFO_R1_ERR_EN_    =(1L<<10),
    UFGO_R1_ERR_EN_   =(1L<<11),
    RSSO_R1_ERR_EN_   =(1L<<12),
    LMVO_R1_ERR_EN_   =(1L<<13),
    YUVO_R1_ERR_EN_   =(1L<<14),
    YUVBO_R1_ERR_EN_  =(1L<<15),
    YUVCO_R1_ERR_EN_  =(1L<<16),
    TSFSO_R1_ERR_EN_  =(1L<<17),
    PDO_R1_ERR_EN_    =(1L<<18),
    MQEO_R1_ERR_EN_   =(1L<<19),
    MAEO_R1_ERR_EN_   =(1L<<20),
    BPCO_R1_ERR_EN_   =(1L<<21),
    CRZO_R1_ERR_EN_   =(1L<<22),
    CRZBO_R1_ERR_EN_  =(1L<<23),
    CRZO_R2_ERR_EN_   =(1L<<24),
    CRZBO_R2_ERR_EN_  =(1L<<25)
}ENUM_CAM_CTL_RAW_INT4_EN;

typedef enum{
    CQ_THR0_DONE_EN_  =(1L<<0),
    CQ_THR1_DONE_EN_  =(1L<<1),
    CQ_THR2_DONE_EN_  =(1L<<2),
    CQ_THR3_DONE_EN_  =(1L<<3),
    CQ_THR4_DONE_EN_  =(1L<<4),
    CQ_THR5_DONE_EN_  =(1L<<5),
    CQ_THR6_DONE_EN_  =(1L<<6),
    CQ_THR7_DONE_EN_  =(1L<<7),
    CQ_THR8_DONE_EN_  =(1L<<8),
    CQ_THR9_DONE_EN_  =(1L<<9),
    CQ_THR10_DONE_EN_ =(1L<<10),
    CQ_THR11_DONE_EN_ =(1L<<11),
    CQ_THR12_DONE_EN_ =(1L<<12),
    CQ_THR13_DONE_EN_ =(1L<<13),
    CQ_THR14_DONE_EN_ =(1L<<14),
    CQ_THR15_DONE_EN_ =(1L<<15),
    CQ_THR16_DONE_EN_ =(1L<<16),
    CQ_THR17_DONE_EN_ =(1L<<17),
    CQ_THR18_DONE_EN_ =(1L<<18),
    CQ_THR19_DONE_EN_ =(1L<<19),
    CQ_THR20_DONE_EN_ =(1L<<20),
    CQ_THR21_DONE_EN_ =(1L<<21),
    CQ_THR22_DONE_EN_ =(1L<<22),
    CQ_THR23_DONE_EN_ =(1L<<23),
    CQ_THR24_DONE_EN_ =(1L<<24),
    CQ_THR25_DONE_EN_ =(1L<<25),
    CQ_THR26_DONE_EN_ =(1L<<26),
    CQ_THR27_DONE_EN_ =(1L<<27),
    CQ_THR28_DONE_EN_ =(1L<<28)
}ENUM_CAM_CTL_RAW_INT5_EN;

/**
    CAM multiplexer
*/

/* CAMCTL_R1_CAMCTL_SEL */
typedef enum{
    RAW_SEL_0      = 0,
    RAW_SEL_1      = (1L<<0),
    RAW_SEL_2      = (2L<<0),
    RAW_SEL_3      = (3L<<0),
    RAW_SEL_4      = (4L<<0),
}ENUM_RAW_SEL;
typedef enum{
    SEP_SEL_0      = 0,
    SEP_SEL_1      = (1L<<3),
}ENUM_SEP_SEL;
typedef enum{
    DGN_SEL_0      = 0,
    DGN_SEL_1      = (1L<<4),
}ENUM_DGN_SEL;
typedef enum{
    RRZ_SEL_0      = 0,
    RRZ_SEL_1      = (1L<<5),
}ENUM_RRZ_SEL;
typedef enum{
    UFEG_SEL_0     = 0,
    UFEG_SEL_1     = (1L<<6),
}ENUM_UFEG_SEL;
typedef enum{
    BPC_R2_SEL_0   = 0,
    BPC_R2_SEL_1   = (1L<<7),
}ENUM_BPC_R2_SEL;
typedef enum{
    LCES_SEL_0     = 0 ,
    LCES_SEL_1     = (1L<<8),
}ENUM_LCES_SEL;
typedef enum{
    CRP_R3_SEL_0   = 0,
    CRP_R3_SEL_1   = (1L<<9),
    CRP_R3_SEL_2   = (2L<<9),
    CRP_R3_SEL_3   = (3L<<9),
    CRP_R3_SEL_4   = (4L<<9),
    CRP_R3_SEL_5   = (5L<<9),
    CRP_R3_SEL_6   = (6L<<9),
    CRP_R3_SEL_7   = (7L<<9),
}ENUM_CRP_R3_SEL;
typedef enum{
    IMG_SEL_0      = 0,
    IMG_SEL_1      = (1L<<12),
    IMG_SEL_2      = (2L<<12),
}ENUM_IMG_SEL;
typedef enum{
    IMGO_SEL_0     = 0,
    IMGO_SEL_1     = (1L<<14),
    IMGO_SEL_2     = (2L<<14),
}ENUM_IMGO_SEL;
typedef enum{
    LTMS_SEL_0     = 0,
    LTMS_SEL_1     = (1<<16),
}ENUM_LTMS_SEL;
typedef enum{
    AF_SEL_0       = 0,
    AF_SEL_1       = (1<<17),
}ENUM_AF_SEL;
typedef enum{
    AA_SEL_0       = 0 ,
    AA_SEL_1       = (1<<18),
}ENUM_AA_SEL;
typedef enum{
    FLK_SEL_0      = 0,
    FLK_SEL_1      = (1L<<19),
    FLK_SEL_2      = (2L<<19),
    FLK_SEL_3      = (3L<<19),
    FLK_SEL_4      = (4L<<19),
    FLK_SEL_5      = (5L<<19),
}ENUM_FLK_SEL;
typedef enum{
    PDO_SEL_0      = 0,
    PDO_SEL_1      = (1L<<22),
}ENUM_PDO_SEL;

typedef enum{
    CRP_R1_SEL_0   = 0,
    CRP_R1_SEL_1   = (1L<<25),
}ENUM_CRP_R1_SEL;
typedef enum{
    HDS_SEL_0      = 0,
    HDS_SEL_1      = (1L<<26),
}ENUM_HDS_SEL;
typedef enum{
    MRG_R6_SEL_0   = 0,
    MRG_R6_SEL_1   = (1L<<27),
}ENUM_MRG_R6_SEL;
typedef enum{
    MRG_R3_O_SEL_0 = 0,
    MRG_R3_O_SEL_1 = (1L<<30),
}ENUM_MRG_R3_O_SEL;
typedef enum{
    MRG_R7_O_SEL_0 = 0,
    MRG_R7_O_SEL_1 = (1L<<31),
}ENUM_MRG_R7_O_SEL;

/* CAMCTL_R1_CAMCTL_SEL2 */
typedef enum{
    DM_R1_SEL_0    = 0,
    DM_R1_SEL_1    = (1L<<2),
}ENUM_DM_R1_SEL;

typedef enum{
    MRG_R13_O_SEL_0  = 0,
    MRG_R13_O_SEL_1  = (1L<<3),
}ENUM_MRG_R13_SEL;

typedef enum{
    RAWI_UFO_SEL_0   = 0,
    RAWI_UFO_SEL_1   = (1L<<4),
}ENUM_RAWI_UFO_SEL;


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
}E_CAM_CTL_TG_FMT;

/**
    IMGO WDMA data format
*/
typedef enum{
    IMGO_FMT_YUV422_1P  = 0,
    IMGO_FMT_YUV422_2P  = 1,
    IMGO_FMT_YUV422_3P  = 2,
    IMGO_FMT_YUV420_2P  = 3,
    IMGO_FMT_YUV420_3P  = 4,
    IMGO_FMT_RAW8       = 8,
    IMGO_FMT_RAW10      = 9,
    IMGO_FMT_RAW12      = 10,
    IMGO_FMT_RAW14      = 11,
    IMGO_FMT_RAW8_2B    = 12,
    IMGO_FMT_RAW10_2B   = 13,
    IMGO_FMT_RAW12_2B   = 14,
    IMGO_FMT_RAW14_2B   = 15,
    IMGO_FMT_RAW10_MIPI = 16,
    IMGO_FMT_RAW15      = 17,
    IMGO_FMT_RAW16      = 18,
    IMGO_FMT_RAW19      = 19,
    IMGO_FMT_RAW24      = 20,
}E_CAM_CTL_IMGO_FMT;

/**
    RRZO WDMA data format
*/
typedef enum{
    RRZO_FMT_RAW8       = 0,
    RRZO_FMT_RAW10      = 1,
    RRZO_FMT_RAW12      = 2,
    RRZO_FMT_RAW14      = 3,
}E_CAM_CTL_RRZO_FMT;

/**
    CRZO WDMA data format
*/
typedef enum{
    CRZO_FMT_YUV422_1P  = 0,
    CRZO_FMT_YUV422_2P  = 1,
}E_CAM_CTL_CRZO_FMT;

typedef enum{
    CRZO_R2_FMT_YUV422_1P  = 0,
    CRZO_R2_FMT_YUV422_2P  = 1,
    CRZO_R2_FMT_YUV420_2P  = 3,
}E_CAM_CTL_CRZO_R2_FMT;

/**
    YUVO WDMA data format
*/
typedef enum{
    YUVO_FMT_YUV422_1P  = 0,
    YUVO_FMT_YUV422_2P  = 1,
    YUVO_FMT_YUV422_3P  = 2,
    YUVO_FMT_YUV420_2P  = 3,
    YUVO_FMT_YUV420_3P  = 4,
}E_CAM_CTL_YUVO_FMT;

/**
    RRZO_FG_MODE
*/
typedef enum{
    RRZO_MODE_BAYER     = 0,
    RRZO_MODE_FG        = 1,
}E_CAM_CTL_RRZO_FG_MODE;

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
}E_CAM_CTL_TG_SWAP;

/**
    bayer data pix pattern
*/
typedef enum{
    B_G_G_R     = 0,
    G_B_R_G     = 1,
    G_R_B_G     = 2,
    R_G_G_B     = 3,
}E_CAM_CTL_TG_PIXID;

//////////////////////////////////////////////////////////////////////////////////////////////

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
    RAWI_FMT_RAW10_MIPI = 16,
    RAWI_FMT_RAW24      = 17,
}E_RAWI_TOP_FMT;

//////////////////////////////////////////////////////////////////////////////////////////////
/**
    camsv interrupt
*/
#define CAMSV_INT_STATUS_VS1(CAMSV_PATH)                (1L<<0)
#define CAMSV_INT_STATUS_TG_SOF1(CAMSV_PATH)            (1L<<7)
#define CAMSV_INT_STATUS_PASS1_DON(CAMSV_PATH)          (1L<<10)

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
}E_CAMSV_TG_FMT;

/**
    SV PAK mode
*/
typedef enum{
    SV_PAK_MODE_RAW8      = 0,
    SV_PAK_MODE_RAW10     = 1,
    SV_PAK_MODE_RAW12     = 2,
    SV_PAK_MODE_RAW14     = 3,
}E_CAMSV_PAK_MODE;

/**
    SV IMGO WDMA data format
*/
typedef enum{
    SV_IMGO_STRIDE_FMT_OTHERS      = 0,
    SV_IMGO_STRIDE_FMT_YUV         = 1,
    SV_IMGO_STRIDE_FMT_JPG      = 2,
}E_CAMSV_IMGO_STRIDE_FMT;

/**
    SV IMGO WDMA bus size
*/
typedef enum{
    SV_DMA_BUS_SIZE_BIT8      = 0,
    SV_DMA_BUS_SIZE_BIT16     = 1,
    SV_DMA_BUS_SIZE_BIT24     = 2,
    SV_DMA_BUS_SIZE_BIT32     = 3,
    SV_DMA_BUS_SIZE_BIT64     = 7,
}E_CAMSV_DMA_BUS_SIZE;

/**
    SV IMGO WDMA data format
*/
typedef enum{
    SV_IMGO_FMT_RAW14   = 0,
    SV_IMGO_FMT_RAW12   = 1,
    SV_IMGO_FMT_RAW10   = 2,
    SV_IMGO_FMT_RAW8    = 3,
}E_CAMSV_IMGO_FMT;

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
}E_CAMSV_TG_SWAP;

/**
    SV A function enable
*/
typedef enum{
    CAMSV_MODULE_TG_EN                    = (1L<<0),
    CAMSV_MODULE_PAK_EN                   = (1L<<2),
    CAMSV_MODULE_PAK_SEL                  = (1L<<3),
    CAMSV_MODULE_IMGO_EN                  = (1L<<4),
    CAMSV_MODULE_DB_EN                    = (1L<<30),
    CAMSV_MODULE_DB_LOCK                  = (1L<<31),
}ENUM_CAMSV_CAMSV_MODULE_EN;

/**
    SV A interrupt enable
*/
typedef enum{
    CAMSV_INT_EN_VS1_INT_EN               = (1L<<0),
    CAMSV_INT_EN_TG_INT1_EN               = (1L<<1),
    CAMSV_INT_EN_TG_INT2_EN               = (1L<<2),
    CAMSV_INT_EN_EXPDON1_INT_EN           = (1L<<3),
    CAMSV_INT_EN_TG_ERR_INT_EN            = (1L<<4),
    CAMSV_INT_EN_TG_GBERR_INT_EN          = (1L<<5),
    CAMSV_INT_EN_TG_DROP_INT_EN           = (1L<<6),
    CAMSV_INT_EN_TG_SOF_INT_EN            = (1L<<7),
    CAMSV_INT_EN_PASS1_DON_INT_EN         = (1L<<10),
    CAMSV_INT_EN_IMGO_ERR_INT_EN          = (1L<<16),
    CAMSV_INT_EN_IMGO_OVERR_INT_EN        = (1L<<17),
    CAMSV_INT_EN_IMGO_DROP_INT_EN         = (1L<<19),
    CAMSV_INT_EN_SW_PASS1_DON_INT_EN      = (1L<<20),
    CAMSV_INT_EN_INT_WCLR_EN              = (1L<<31),
}ENUM_CAMSV_CAMSV_INT_EN;

/**
    CAMSV interrupt status
*/

typedef enum{
    SV_VS1_ST              = (1L<<0),
    SV_TG_INT1_ST          = (1L<<1),
    SV_TG_INT2_ST          = (1L<<2),
    SV_EXPDON_ST           = (1L<<3),
    SV_SOF_INT_ST          = (1L<<7),
    SV_HW_PASS1_DON_ST     = (1L<<10),
    SV_SW_PASS1_DON_ST     = (1L<<20)
}ENUM_CAMSV_INT;

/**
    SV B function enable
*/
typedef enum{
    CAMSV_MODULE_TG_EN_B                 = (1L<<0),
    CAMSV_MODULE_PAK_EN_B                = (1L<<2),
    CAMSV_MODULE_PAK_SEL_B               = (1L<<3),
    CAMSV_MODULE_IMGO_EN_B               = (1L<<4),
    CAMSV_MODULE_DB_EN_B                 = (1L<<30),
    CAMSV_MODULE_DB_LOCK_B               = (1L<<31),
}ENUM_CAMSV_CAMSV_MODULE_EN_B;

/**
    SV B interrupt enable
*/
typedef enum{
    CAMSV_INT_EN_VS1_INT_EN_B            = (1L<<0),
    CAMSV_INT_EN_TG_INT1_EN_B            = (1L<<1),
    CAMSV_INT_EN_TG_INT2_EN_B            = (1L<<2),
    CAMSV_INT_EN_EXPDON1_INT_EN_B        = (1L<<3),
    CAMSV_INT_EN_TG_ERR_INT_EN_B         = (1L<<4),
    CAMSV_INT_EN_TG_GBERR_INT_EN_B       = (1L<<5),
    CAMSV_INT_EN_TG_DROP_INT_EN_B        = (1L<<6),
    CAMSV_INT_EN_TG_SOF_INT_EN_B         = (1L<<7),
    CAMSV_INT_EN_PASS1_DON_INT_EN_B      = (1L<<10),
    CAMSV_INT_EN_IMGO_ERR_INT_EN_B       = (1L<<16),
    CAMSV_INT_EN_IMGO_OVERR_INT_EN_B     = (1L<<17),
    CAMSV_INT_EN_IMGO_DROP_INT_EN_B      = (1L<<19),
    CAMSV_INT_EN_SW_PASS1_DON_INT_EN_B   = (1L<<20),
    CAMSV_INT_EN_INT_WCLR_EN_B           = (1L<<31),
}ENUM_CAMSV_CAMSV_INT_EN_B;

/******************************************************************************
 * @enum E_PIX
 *
 * @Pixel mode: For reg read/write, reference to PIX_BUS in CAM_CTL_FMT_SEL
 *
 ******************************************************************************/
typedef enum{
    _1_pix_ = 0,
    _2_pix_ = 1,
    _4_pix_ = 2,
}E_PIX;

/******************************************************************************
 * @enum ENUM_RAW_TG_SEL
 *
 * @Pixel mode: For reg read/write, reference to CAMCTL_RAW_TG_SEL
 *
 ******************************************************************************/
typedef enum{
    FROM_TG_A = 0,
    FROM_TG_B = 1,
    FROM_TG_C = 2,
}ENUM_RAW_TG_SEL;

#endif

