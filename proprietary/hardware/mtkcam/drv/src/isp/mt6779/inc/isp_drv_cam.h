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

#ifndef _ISP_DRV_CAM_H_
#define _ISP_DRV_CAM_H_
/**
    this module contains CAM_A, CAM_B in abstract method + UNI_A cq
    note:
        UNI_A phy is not included
*/
#include <isp_drv.h>
#include <imem_drv.h>
#include <isp_reg_def_cam.h>

/**
    patch for hw design prob:
    under twin mode, dmx_id/dmx_sel have no DB, need to use master cam's timing to ctrl slave cam's CQ.
    (setting is only valid at  v-blanking )
*/
#define TWIN_CQ_SW_WORKAROUND   1

/**
    descriptor range & dummy reg for descriptor
*/
#define CQ_DES_RANGE    0xFFFF
#define CQ_DUMMY_REG    0x43fc
#define CQ_NOP_REG      0x4400
///////////////////////////////////////////////////////////
/**
    CAM CQ descriptor
    its a abstract descriptor , cam_a & cam_b use the same enum
*/
typedef enum {
    //put slave cam's ctrl 1st, for pipe master cam & slave cam.
    TWIN_CQ0_BASEADDRESS = 0,    // 1    20208   //must be allocated before salve cam's descriptor which is for salve cam's cq-trig
    TWIN_CQ1_BASEADDRESS,        // 1    20214   //must be allocated before salve cam's descriptor which is for salve cam's cq-trig
    TWIN_CQ10_BASEADDRESS,       // 1    20280   //must be allocated before salve cam's descriptor which is for salve cam's cq-trig
#if TWIN_CQ_SW_WORKAROUND
    CAM_CQ0_EN_,                 // 1    10204/20204   //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    CAM_CQ0_TRIG_,               // 1    10054/20054
    CAM_CQ1_EN_,                 // 1    10210/20210   //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    CAM_CQ1_TRIG_,               // 1    10054/20054
    CAM_CQ10_EN_,                // 1    1027C/2027C   //this is for twin's master cam use only , due to hw design defect!!(dmx_sel/dmx_id have no db)
    CAM_CQ10_TRIG_,              // 1    10054/20054
#endif
    // CTL
    CAM_CTL_EN_,
    CAM_CTL_SEL_,     //10
    CAM_CTL_FMT_,
    CAM_CTL_MISC_,
    CAM_CTL_SW_CTL_,
    CAM_CTL_RAWI_TRIG_,
    CAM_CTL_DONE_SEL_,
    CAM_CTL_DONE_SEL2_,
    CAM_CTL_SW_DONE_SEL_,
    CAM_CTL_FBC_GROUP_,
    CAM_CTL_HLR_LKMSB_,
    CAM_CTL_DB_SUB_SEL_, //20
    CAM_CTL_INT_EN_,
    CAM_CTL_INT2_EN_,
    CAM_CTL_INT3_EN_,
    CAM_CTL_INT4_EN_,
    CAM_CTL_INT5_EN_,
    CAM_TWIN_INFO,
    CAM_CQ_COUNTER,
    // FBC
    CAM_FBC_IMGO_,
    CAM_FBC_RRZO_,
    CAM_FBC_UFEO_,   // 30
    CAM_FBC_LCSO_,   // LCSO == LCESO
    CAM_FBC_AFO_,
    CAM_FBC_AAO_,
    CAM_FBC_PDO_,
    CAM_FBC_FLKO_,
    CAM_FBC_LMVO_,
    CAM_FBC_RSSO_,
    CAM_FBC_UFGO_,
    CAM_FBC_TSFSO_,
    CAM_FBC_YUVO_, // 40
    CAM_FBC_YUVBO_,
    CAM_FBC_YUVCO_,
    CAM_FBC_CRZO_,
    CAM_FBC_CRZBO_,
    CAM_FBC_CRZO_R2_,
    CAM_FBC_CRZBO_R2_,
    CAM_FBC_RSSO_R2_,
    CAM_FBC_LTMSO_,
    CAM_CTL_DMA_V_FLIP_,
    CAM_CTL_SPECIAL_FUN_,
    //DMA CON1 & CON2 & CON3 can't be wriitten by CQ
    CAM_DMA_IMGO_BA,
    CAM_DMA_IMGO_BA_OFST,
    CAM_DMA_IMGO_X_,
    CAM_DMA_IMGO_Y_,
    CAM_DMA_IMGO_STRIDE_,
    CAM_DMA_IMGO_CROP_,
    CAM_DMA_RRZO_BA,
    CAM_DMA_RRZO_BA_OFST,
    CAM_DMA_RRZO_X_,
    CAM_DMA_RRZO_Y_,
    CAM_DMA_RRZO_STRIDE_,
    CAM_DMA_RRZO_CROP_,
    CAM_DMA_AAO_BA,
    CAM_DMA_AAO_,
    CAM_DMA_AFO_BA,
    CAM_DMA_AFO_BA_OFST,
    CAM_DMA_AFO_,
    CAM_DMA_LCSO_BA,      // LCSO == LCESO
    CAM_DMA_LCSO_BA_OFST, // LCSO == LCESO
    CAM_DMA_LCSO_,        // LCSO == LCESO
    CAM_DMA_UFEO_BA,
    CAM_DMA_UFEO_BA_OFST,
    CAM_DMA_UFEO_,
    CAM_DMA_PDO_BA,
    CAM_DMA_PDO_BA_OFST,
    CAM_DMA_PDO_,
    CAM_DMA_FLKO_BA,
    CAM_DMA_FLKO_BA_OFST,
    CAM_DMA_FLKO_,
    CAM_DMA_LMVO_BA,
    CAM_DMA_LMVO_BA_OFST,
    CAM_DMA_LMVO_,
    CAM_DMA_RSSO_A_BA,
    CAM_DMA_RSSO_A_BA_OFST,
    CAM_DMA_RSSO_A_,
    CAM_DMA_UFGO_BA,
    CAM_DMA_UFGO_BA_OFST,
    CAM_DMA_UFGO_,
    CAM_DMA_TSFSO_BA_,
    CAM_DMA_TSFSO_BA_OFST_,
    CAM_DMA_TSFSO_,
    CAM_DMA_LTMSO_BA_,
    CAM_DMA_LTMSO_BA_OFST_,
    CAM_DMA_LTMSO_,
    CAM_DMA_YUVO_BA_,
    CAM_DMA_YUVO_BA_OFST_,
    CAM_DMA_YUVO_,
    CAM_DMA_YUVO_CROP_,
    CAM_DMA_YUVBO_BA_,
    CAM_DMA_YUVBO_BA_OFST_,
    CAM_DMA_YUVBO_,
    CAM_DMA_YUVBO_CROP_,
    CAM_DMA_YUVCO_BA_,
    CAM_DMA_YUVCO_BA_OFST_,
    CAM_DMA_YUVCO_,
    CAM_DMA_YUVCO_CROP_,
    CAM_DMA_CRZO_BA_,
    CAM_DMA_CRZO_BA_OFST_,
    CAM_DMA_CRZO,
    CAM_DMA_CRZBO_BA_,
    CAM_DMA_CRZBO_BA_OFST_,
    CAM_DMA_CRZBO,
    CAM_DMA_CRZO_R2_BA_,
    CAM_DMA_CRZO_R2_BA_OFST_,
    CAM_DMA_CRZO_R2_,
    CAM_DMA_CRZBO_R2_BA_,
    CAM_DMA_CRZBO_R2_BA_OFST_,
    CAM_DMA_CRZBO_R2_,
    CAM_DMA_RSSO_R2_BA_,
    CAM_DMA_RSSO_R2_BA_OFST_,
    CAM_DMA_RSSO_R2_,
    CAM_DMA_RAWI_,
    CAM_DMA_BPCI_,
    CAM_DMA_BPCI_R2_,
    CAM_DMA_UFDI_R2_,
    CAM_DMA_LSCI_,
    CAM_DMA_PDI_,
    CAM_DMA_RSV1_,
    CAM_MAGIC_NUM_,
    CAM_DATA_PAT_,
    CAM_DMA_RSV6_,
    CAM_DMA_FH_EN_,
    CAM_DMA_FH_IMGO_,
    CAM_DMA_FH_IMGO_SPARE_,
    CAM_DMA_FH_IMGO_SPARE_CROP,
    CAM_DMA_FH_IMGO_SPARE_PA,
    CAM_DMA_FH_RRZO_,
    CAM_DMA_FH_RRZO_SPARE_,
    CAM_DMA_FH_RRZO_SPARE_CROP,
    CAM_DMA_FH_RRZO_SPARE_PA,
    CAM_DMA_FH_AAO_,
    CAM_DMA_FH_AAO_SPARE_CQ0_,
    CAM_DMA_FH_AAO_SPARE_,
    CAM_DMA_FH_AFO_,
    CAM_DMA_FH_AFO_SPARE_CQ0_,
    CAM_DMA_FH_AFO_SPARE_,
    CAM_DMA_FH_LCSO_,          // LCSO == LCESO
    CAM_DMA_FH_LCSO_SPARE_,    // LCSO == LCESO
    CAM_DMA_FH_LCSO_SPARE_PA,
    CAM_DMA_FH_UFEO_,
    CAM_DMA_FH_UFEO_SPARE_,
    CAM_DMA_FH_UFEO_SPARE_PA,
    CAM_DMA_FH_PDO_,
    CAM_DMA_FH_PDO_SPARE_CQ0_,
    CAM_DMA_FH_PDO_SPARE_,
    CAM_DMA_FH_FLKO_,
    CAM_DMA_FH_FLKO_SPARE_CQ0_,
    CAM_DMA_FH_FLKO_SPARE_,
    CAM_DMA_FH_LMVO_,
    CAM_DMA_FH_LMVO_SPARE_,
    CAM_DMA_FH_LMVO_SPARE_PA,
    CAM_DMA_FH_RSSO_A_,
    CAM_DMA_FH_RSSO_A_SPARE_,
    CAM_DMA_FH_RSSO_A_SPARE_PA,
    CAM_DMA_FH_UFGO_,
    CAM_DMA_FH_UFGO_SPARE_,
    CAM_DMA_FH_UFGO_SPARE_PA,
    CAM_DMA_FH_TSFSO_,
    CAM_DMA_FH_TSFSO_SPARE_CQ0_,
    CAM_DMA_FH_TSFSO_SPARE_,
    CAM_DMA_FH_LTMSO_,
    CAM_DMA_FH_LTMSO_SPARE_,
    CAM_DMA_FH_YUVO_,
    CAM_DMA_FH_YUVO_SPARE_,
    CAM_DMA_FH_YUVO_SPARE_PA,
    CAM_DMA_FH_YUVBO_,
    CAM_DMA_FH_YUVBO_SPARE_,
    CAM_DMA_FH_YUVBO_SPARE_PA,
    CAM_DMA_FH_YUVCO_,
    CAM_DMA_FH_YUVCO_SPARE_,
    CAM_DMA_FH_YUVCO_SPARE_PA,
    CAM_DMA_FH_CRZO_,
    CAM_DMA_FH_CRZO_SPARE_,
    CAM_DMA_FH_CRZO_SPARE_PA,
    CAM_DMA_FH_CRZBO_,
    CAM_DMA_FH_CRZBO_SPARE_,
    CAM_DMA_FH_CRZBO_SPARE_PA,
    CAM_DMA_FH_CRZO_R2_,
    CAM_DMA_FH_CRZO_R2_SPARE_,
    CAM_DMA_FH_CRZO_R2_SPARE_PA,
    CAM_DMA_FH_CRZBO_R2_,
    CAM_DMA_FH_CRZBO_R2_SPARE_,
    CAM_DMA_FH_CRZBO_R2_SPARE_PA,
    CAM_DMA_FH_RSSO_R2_,
    CAM_DMA_FH_RSSO_R2_SPARE_,
    CAM_CQ_THRE0_ADDR_,
    CAM_CQ_THRE0_SIZE_,
    CAM_CQ_THRE1_ADDR_,
    CAM_CQ_THRE1_SIZE_,
    CAM_CQ_THRE2_ADDR_,
    CAM_CQ_THRE2_SIZE_,
    CAM_CQ_THRE3_ADDR_,
    CAM_CQ_THRE3_SIZE_,
    CAM_CQ_THRE4_ADDR_,
    CAM_CQ_THRE4_SIZE_,
    CAM_CQ_THRE5_ADDR_,
    CAM_CQ_THRE5_SIZE_,
    CAM_CQ_THRE6_ADDR_,
    CAM_CQ_THRE6_SIZE_,
    CAM_CQ_THRE7_ADDR_,
    CAM_CQ_THRE7_SIZE_,
    CAM_CQ_THRE8_ADDR_,
    CAM_CQ_THRE8_SIZE_,
    CAM_CQ_THRE9_ADDR_,
    CAM_CQ_THRE9_SIZE_,
    CAM_CQ_THRE10_ADDR_,
    CAM_CQ_THRE10_SIZE_,
    CAM_CQ_THRE11_ADDR_,
    CAM_CQ_THRE11_SIZE_,
    CAM_CQ_THRE12_ADDR_,
    CAM_CQ_THRE12_SIZE_,
    CAM_CQ_THRE13_ADDR_,
    CAM_CQ_THRE13_SIZE_,
    CAM_CQ_THRE14_ADDR_,
    CAM_CQ_THRE14_SIZE_,
    CAM_CQ_THRE15_ADDR_,
    CAM_CQ_THRE15_SIZE_,
    CAM_CQ_THRE16_ADDR_,
    CAM_CQ_THRE16_SIZE_,
    CAM_CQ_THRE17_ADDR_,
    CAM_CQ_THRE17_SIZE_,
    CAM_CQ_THRE18_ADDR_,
    CAM_CQ_THRE18_SIZE_,
    CAM_CQ_THRE19_ADDR_,
    CAM_CQ_THRE19_SIZE_,
    CAM_CQ_THRE20_ADDR_,
    CAM_CQ_THRE20_SIZE_,
    CAM_CQ_THRE21_ADDR_,
    CAM_CQ_THRE21_SIZE_,
    CAM_CQ_THRE22_ADDR_,
    CAM_CQ_THRE22_SIZE_,
    CAM_CQ_THRE23_ADDR_,
    CAM_CQ_THRE23_SIZE_,
    CAM_CQ_THRE24_ADDR_,
    CAM_CQ_THRE24_SIZE_,
    CAM_CQ_THRE25_ADDR_,
    CAM_CQ_THRE25_SIZE_,
    CAM_CQ_THRE26_ADDR_,
    CAM_CQ_THRE26_SIZE_,
    CAM_CQ_THRE27_ADDR_,
    CAM_CQ_THRE27_SIZE_,
    CAM_CQ_THRE28_ADDR_,
    CAM_CQ_THRE28_SIZE_,
    CAM_ISP_PBN_,
    CAM_ISP_DBN_,
    CAM_ISP_BIN_,
    CAM_ISP_OBC_,
    CAM_ISP_LSC_,
    CAM_ISP_RRZ_,
    CAM_ISP_UFE_,
    CAM_ISP_QBIN_,
    CAM_ISP_PDE_,
    CAM_ISP_LCS_SIZE,
    CAM_ISP_LCS_LOG_,     // LCS == LCES
    CAM_ISP_LCS_LRZR,
    CAM_ISP_LCS_FLC0_,    // LCS == LCES
    CAM_ISP_AE_,
    CAM_ISP_AE2_,
    CAM_ISP_AWB_,
    CAM_ISP_AF_,
    CAM_ISP_FLK_A_,
    CAM_ISP_LMV_,
    CAM_ISP_HDS_,
    CAM_ISP_RSS_A_,
    CAM_ISP_SEP_,
    CAM_ISP_BPC_,
    CAM_ISP_DGN_,
    CAM_ISP_WB_,
    CAM_ISP_MRG_R7_,
    CAM_ISP_MRG_R5_,
    CAM_ISP_QBIN_R4_,
    CAM_ISP_LTMS_,
    CAM_ISP_LTM_SECTION1,
    CAM_ISP_LTM_MAX_DIV,
    CAM_ISP_LTM_SECTION2,
    CAM_ISP_LTMTC_CURVE_1,
    CAM_ISP_LTMTC_CURVE_2,
    CAM_ISP_LTM_PINGPONG_,
    CAM_ISP_HLR_,
    CAM_ISP_MRG_,
    CAM_ISP_AA_,
    CAM_ISP_CRP_R3_,
    CAM_ISP_PAK_,
    CAM_ISP_UNP_R2_,
    CAM_ISP_FBND_,
    CAM_ISP_BPC_R2_,
    CAM_ISP_OBC_R2_,
    CAM_ISP_UFG_,
    CAM_ISP_QBIN_R5_,
    CAM_ISP_GSE_,
    CAM_ISP_MRG_R8_,
    CAM_ISP_CRP_R1_,
    CAM_ISP_MRG_R2_,
    CAM_ISP_QBIN_R2_,
    CAM_ISP_TSFS_,
    CAM_ISP_CRP_R8_,
    CAM_ISP_SLK_CEN,
    CAM_ISP_SLK_XOFF,
    CAM_ISP_SLK_CON,
    CAM_ISP_SLK_SIZE,
    CAM_ISP_DM_,
    CAM_ISP_CCM_,
    CAM_ISP_GGM_,
    CAM_ISP_GGM_PINGPONG_,
    CAM_ISP_G2C_,
    CAM_ISP_C42_,
    CAM_ISP_CRSP_,
    CAM_ISP_PAK_R3_,
    CAM_ISP_PAK_R4_,
    CAM_ISP_MRG_R3_,
    CAM_ISP_MRG_R4_,
    CAM_ISP_MRG_R6_,
    CAM_ISP_QBIN_R3_,
    CAM_ISP_MRG_R10_,
    CAM_ISP_CRZ_,
    CAM_ISP_SLK_R2_CEN,
    CAM_ISP_SLK_R2_XOFF,
    CAM_ISP_SLK_R2_CON,
    CAM_ISP_SLK_R2_SIZE,
    CAM_ISP_MRG_R11_,
    CAM_ISP_MRG_R12_,
    CAM_ISP_UFD_R2_,
    CAM_ISP_CRP_R4_,
    CAM_ISP_MRG_R13_,
    CAM_ISP_MRG_R14_,
    CAM_ISP_BS_,
    CAM_ISP_BS_R2_,
    CAM_ISP_BS_R3_,
    CAM_ISP_PAK_R5_,
    CAM_ISP_G2C_R2_,
    CAM_ISP_C42_R2_,
    CAM_ISP_FLC_,
    CAM_ISP_CRSP_R2_,
    CAM_ISP_YNRS_,
    CAM_ISP_YNRS_PINGPONG_,
    CAM_ISP_GGM_R2_,
    CAM_ISP_GGM_R2_PINGPONG_,
    CAM_ISP_RSS_R2_,
    CAM_ISP_CRZ_R2_,
    CAM_ISP_MOBC_R2_,
    CAM_ISP_MOBC_R3_,
    SV_DMA_IMGO_BA_A_0,
    SV_DMA_IMGO_BA_A_1,
    SV_DMA_IMGO_BA_A_2,
    SV_DMA_IMGO_BA_A_3,
    SV_DMA_IMGO_BA_A_4,
    SV_DMA_IMGO_BA_A_5,
    SV_DMA_IMGO_BA_A_6,
    SV_DMA_IMGO_BA_A_7,
    SV_DMA_FH_IMGO_A_0,
    SV_DMA_FH_IMGO_A_1,
    SV_DMA_FH_IMGO_A_2,
    SV_DMA_FH_IMGO_A_3,
    SV_DMA_FH_IMGO_A_4,
    SV_DMA_FH_IMGO_A_5,
    SV_DMA_FH_IMGO_A_6,
    SV_DMA_FH_IMGO_A_7,
    SV_DMA_FH_IMGO_SPARE_A_0,
    SV_DMA_FH_IMGO_SPARE_A_1,
    SV_DMA_FH_IMGO_SPARE_A_2,
    SV_DMA_FH_IMGO_SPARE_A_3,
    SV_DMA_FH_IMGO_SPARE_A_4,
    SV_DMA_FH_IMGO_SPARE_A_5,
    SV_DMA_FH_IMGO_SPARE_A_6,
    SV_DMA_FH_IMGO_SPARE_A_7,
    SV_DMA_UFEO_BA_A_0,
    SV_DMA_UFEO_BA_A_1,
    SV_DMA_UFEO_BA_A_2,
    SV_DMA_UFEO_BA_A_3,
    SV_DMA_UFEO_BA_A_4,
    SV_DMA_UFEO_BA_A_5,
    SV_DMA_UFEO_BA_A_6,
    SV_DMA_UFEO_BA_A_7,
    SV_DMA_FH_UFEO_A_0,
    SV_DMA_FH_UFEO_A_1,
    SV_DMA_FH_UFEO_A_2,
    SV_DMA_FH_UFEO_A_3,
    SV_DMA_FH_UFEO_A_4,
    SV_DMA_FH_UFEO_A_5,
    SV_DMA_FH_UFEO_A_6,
    SV_DMA_FH_UFEO_A_7,
    SV_DMA_FH_UFEO_SPARE_A_0,
    SV_DMA_FH_UFEO_SPARE_A_1,
    SV_DMA_FH_UFEO_SPARE_A_2,
    SV_DMA_FH_UFEO_SPARE_A_3,
    SV_DMA_FH_UFEO_SPARE_A_4,
    SV_DMA_FH_UFEO_SPARE_A_5,
    SV_DMA_FH_UFEO_SPARE_A_6,
    SV_DMA_FH_UFEO_SPARE_A_7,
    SV_DMA_IMGO_BA_B_0,
    SV_DMA_IMGO_BA_B_1,
    SV_DMA_IMGO_BA_B_2,
    SV_DMA_IMGO_BA_B_3,
    SV_DMA_IMGO_BA_B_4,
    SV_DMA_IMGO_BA_B_5,
    SV_DMA_IMGO_BA_B_6,
    SV_DMA_IMGO_BA_B_7,
    SV_DMA_FH_IMGO_B_0,
    SV_DMA_FH_IMGO_B_1,
    SV_DMA_FH_IMGO_B_2,
    SV_DMA_FH_IMGO_B_3,
    SV_DMA_FH_IMGO_B_4,
    SV_DMA_FH_IMGO_B_5,
    SV_DMA_FH_IMGO_B_6,
    SV_DMA_FH_IMGO_B_7,
    SV_DMA_FH_IMGO_SPARE_B_0,
    SV_DMA_FH_IMGO_SPARE_B_1,
    SV_DMA_FH_IMGO_SPARE_B_2,
    SV_DMA_FH_IMGO_SPARE_B_3,
    SV_DMA_FH_IMGO_SPARE_B_4,
    SV_DMA_FH_IMGO_SPARE_B_5,
    SV_DMA_FH_IMGO_SPARE_B_6,
    SV_DMA_FH_IMGO_SPARE_B_7,
    SV_DMA_UFEO_BA_B_0,
    SV_DMA_UFEO_BA_B_1,
    SV_DMA_UFEO_BA_B_2,
    SV_DMA_UFEO_BA_B_3,
    SV_DMA_UFEO_BA_B_4,
    SV_DMA_UFEO_BA_B_5,
    SV_DMA_UFEO_BA_B_6,
    SV_DMA_UFEO_BA_B_7,
    SV_DMA_FH_UFEO_B_0,
    SV_DMA_FH_UFEO_B_1,
    SV_DMA_FH_UFEO_B_2,
    SV_DMA_FH_UFEO_B_3,
    SV_DMA_FH_UFEO_B_4,
    SV_DMA_FH_UFEO_B_5,
    SV_DMA_FH_UFEO_B_6,
    SV_DMA_FH_UFEO_B_7,
    SV_DMA_FH_UFEO_SPARE_B_0,
    SV_DMA_FH_UFEO_SPARE_B_1,
    SV_DMA_FH_UFEO_SPARE_B_2,
    SV_DMA_FH_UFEO_SPARE_B_3,
    SV_DMA_FH_UFEO_SPARE_B_4,
    SV_DMA_FH_UFEO_SPARE_B_5,
    SV_DMA_FH_UFEO_SPARE_B_6,
    SV_DMA_FH_UFEO_SPARE_B_7,
    SV_FBC_RCNT_INC_0,
    SV_FBC_RCNT_INC_1,
    SV_FBC_RCNT_INC_2,
    SV_FBC_RCNT_INC_3,
    SV_FBC_RCNT_INC_4,
    SV_FBC_RCNT_INC_5,
    SV_FBC_RCNT_INC_6,
    SV_FBC_RCNT_INC_7,
    SV_DMA_FH_IMGO_BASE_A_0,
    SV_DMA_FH_IMGO_BASE_A_1,
    SV_DMA_FH_IMGO_BASE_A_2,
    SV_DMA_FH_IMGO_BASE_A_3,
    SV_DMA_FH_IMGO_BASE_A_4,
    SV_DMA_FH_IMGO_BASE_A_5,
    SV_DMA_FH_IMGO_BASE_A_6,
    SV_DMA_FH_IMGO_BASE_A_7,
    SV_DMA_FH_IMGO_BASE_B_0,
    SV_DMA_FH_IMGO_BASE_B_1,
    SV_DMA_FH_IMGO_BASE_B_2,
    SV_DMA_FH_IMGO_BASE_B_3,
    SV_DMA_FH_IMGO_BASE_B_4,
    SV_DMA_FH_IMGO_BASE_B_5,
    SV_DMA_FH_IMGO_BASE_B_6,
    SV_DMA_FH_IMGO_BASE_B_7,
    SV_FBC_RCNT_INC_B_0,
    SV_FBC_RCNT_INC_B_1,
    SV_FBC_RCNT_INC_B_2,
    SV_FBC_RCNT_INC_B_3,
    SV_FBC_RCNT_INC_B_4,
    SV_FBC_RCNT_INC_B_5,
    SV_FBC_RCNT_INC_B_6,
    SV_FBC_RCNT_INC_B_7,

    SV_MODULE_EN_A_0,
    SV_MODULE_EN_A_1,
    SV_MODULE_EN_B_0,
    SV_MODULE_EN_B_1,
    SV_TG_SUB_PERIOD_A_0,
    SV_TG_SUB_PERIOD_A_1,
    SV_TG_SUB_PERIOD_B_0,
    SV_TG_SUB_PERIOD_B_1,
    SV_SPARE_0_A_0,
    SV_SPARE_0_A_1,
    SV_SPARE_0_B_0,
    SV_SPARE_0_B_1,
    SV_SPARE_1_A_0,
    SV_SPARE_1_A_1,
    SV_SPARE_1_B_0,
    SV_SPARE_1_B_1,
    SV_120FPS_CAMA_CHANGE,
    SV_120FPS_CAMB_CHANGE,
    CAM_DMA_EN_,

    CAM_DMA_CAMSV_STAGER_,
    CAM_DUMMY_,
    CAM_CTL_FBC_RCNT_INC_,       //must be allocated after all FBC_en descirptor
    CAM_CQ_EN_,
    CAM_NEXT_Thread_,            //must at the end of descriptor  , or apb will be crashed
    CAM_CAM_END_,                //must be kept at the end of descriptor
    CAM_CAM_MODULE_MAX
}E_CAM_MODULE;

#define RingBuf_MODULE_MAX   (4+1)  // +1 for cq inner baseaddr when high-speed mode

//special deifne
//write 2 cq inner address directly!!!!!
#define CAM_CQ_THRE0_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x18208           // for CQ access only
#define CAM_CQ_THRE1_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x18214           // for CQ access only
#define CAM_CQ_THRE2_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x18220           // for CQ access only
#define CAM_CQ_THRE3_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x1822c           // for CQ access only
#define CAM_CQ_THRE4_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x18238           // for CQ access only
#define CAM_CQ_THRE5_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x18244           // for CQ access only
#define CAM_CQ_THRE6_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x18250           // for CQ access only
#define CAM_CQ_THRE7_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x1825c           // for CQ access only
#define CAM_CQ_THRE8_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x18268           // for CQ access only
#define CAM_CQ_THRE9_ADDR_CQONLY    (CAM_NEXT_Thread_)  // 1    0x18274           // for CQ access only
#define CAM_CQ_THRE10_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x18280           // for CQ access only
#define CAM_CQ_THRE11_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x1828c           // for CQ access only
#define CAM_CQ_THRE12_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x18298           // for CQ access only
#define CAM_CQ_THRE13_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x182a4           // for CQ access only
#define CAM_CQ_THRE14_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x182b0           // for CQ access only
#define CAM_CQ_THRE15_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x182bc           // for CQ access only
#define CAM_CQ_THRE16_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x182c8           // for CQ access only
#define CAM_CQ_THRE17_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x182d4           // for CQ access only
#define CAM_CQ_THRE18_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x182e0           // for CQ access only
#define CAM_CQ_THRE19_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x182ec           // for CQ access only
#define CAM_CQ_THRE20_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x182f8           // for CQ access only
#define CAM_CQ_THRE21_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x18304           // for CQ access only
#define CAM_CQ_THRE22_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x18310           // for CQ access only
#define CAM_CQ_THRE23_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x1831c           // for CQ access only
#define CAM_CQ_THRE24_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x18328           // for CQ access only
#define CAM_CQ_THRE25_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x18334           // for CQ access only
#define CAM_CQ_THRE26_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x18340           // for CQ access only
#define CAM_CQ_THRE27_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x1834c           // for CQ access only
#define CAM_CQ_THRE28_ADDR_CQONLY   (CAM_NEXT_Thread_)  // 1    0x18358           // for CQ access only
///////////////////////////////////////////////////////////////////////////


/**
    RW marco, for coding convenience

    note1: cam + uni module only (uni is included because of cq programming range.

            if __offset over 0x1000, its a special design for CQ  baseaddr.
            target reg address is at current offset - 0x1000 + 0x8000. (0x1000 is for roll back, 0x8000 is mapping to set&clr domain.
    note2: caller here don't care, CAM_A or CAM_B r all acceptable!
*/

#define REG_NAME_CAT(Name,suffix) Name##suffix
//user no need to write reg address, drv will fill the address automatically
#define CAM_BURST_READ_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    ret=(IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->readRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,CAM_A));\
    ret;\
})

//user no need to write reg address, drv will fill the address automatically
#define CAM_BURST_WRITE_REGS(IspDrvPtr,RegName,RegStruct,RegCount,...)({\
    MBOOL __ret=0;\
    MUINT32 __i=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    do{\
        RegStruct[__i].Addr = __offset;\
        __offset+=4;\
    }while(++__i < RegCount);\
    __ret=(IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->writeRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,CAM_A));\
    __ret;\
})

//user no need to write reg address, drv will fill the address automatically
#define CAM_BURST_WRITE_REGS_DIVIDED_ARRAY(IspDrvPtr,RegName,RegStruct,RegCount,ArrayOffset,...)({\
        MBOOL __ret=0;\
        MUINT32 __i=0;\
        MUINT32 __offset;\
        __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                    (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
        __offset += ArrayOffset;\
        do{\
            RegStruct[__i].Addr = __offset;\
            __offset+=4;\
        }while(++__i < RegCount);\
        __ret=(IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->writeRegs((ISP_DRV_REG_IO_STRUCT*)RegStruct,RegCount,CAM_A));\
        __ret;\
})


#define CAM_READ_REG(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    (IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->readReg(__offset,CAM_A));\
})

#define CAM_PTR_READ_REGS(IspDrvPtr,RegName,RegPtr,RegCount,...)({\
    MBOOL ret=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    ret=(IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->readPtrRegs(RegPtr, __offset, RegCount,CAM_A));\
    ret;\
})

#define CAM_PTR_READ_REGS_DIVIDED_ARRAY(IspDrvPtr,RegName,RegPtr,RegCount,ArrayOffset,...)({\
    MBOOL ret=0;\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    __offset += ArrayOffset;\
    ret=(IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->readPtrRegs(RegPtr, __offset, RegCount,CAM_A));\
    ret;\
})



#define CAM_READ_BITS(IspDrvPtr,RegName,FieldName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = (IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->readReg(__offset,CAM_A));\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName;\
})


#define CAM_WRITE_REG(IspDrvPtr,RegName,Value,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    (IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->writeReg(__offset, Value,CAM_A));\
}while(0);

#define CAM_WRITE_BITS(IspDrvPtr,RegName,FieldName,FieldValue,...)do{\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    android::Mutex::Autolock lock(IspDrv::RegRWMacro);\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw = (IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->readReg(__offset,CAM_A));\
    ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Bits.FieldName = FieldValue;\
    (IspDrvPtr REG_NAME_CAT(RegName,_GETPHY) ->writeReg(__offset, ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName.Raw,CAM_A));\
}while(0);

#define CAM_REG_ADDR(IspDrvPtr,RegName,...)({\
    MUINT32 __offset;\
    __offset = (MUINT8*)(&( ((cam_reg_t*)(IspDrvPtr->m_pIspRegMap))->RegName)) - \
                (MUINT8*)(IspDrvPtr->m_pIspRegMap);\
    __offset;\
})

///////////////////////////////////////////////////////////
/**
    CAM hw CQ
*/
typedef enum
{
    ISP_DRV_CQ_THRE0 = 0,
    ISP_DRV_CQ_THRE1,
    ISP_DRV_CQ_THRE2,
    ISP_DRV_CQ_THRE3,
    ISP_DRV_CQ_THRE4,
    ISP_DRV_CQ_THRE5,
    ISP_DRV_CQ_THRE6,
    ISP_DRV_CQ_THRE7,
    ISP_DRV_CQ_THRE8,
    ISP_DRV_CQ_THRE9,
    ISP_DRV_CQ_THRE10,
    ISP_DRV_CQ_THRE11,
    ISP_DRV_CQ_THRE12,
    ISP_DRV_CQ_THRE13,
    ISP_DRV_CQ_THRE14,
    ISP_DRV_CQ_THRE15,
    ISP_DRV_CQ_THRE16,
    ISP_DRV_CQ_THRE17,
    ISP_DRV_CQ_THRE18,
    ISP_DRV_CQ_THRE19,
    ISP_DRV_CQ_THRE20,
    ISP_DRV_CQ_THRE21,
    ISP_DRV_CQ_THRE22,
    ISP_DRV_CQ_THRE23,
    ISP_DRV_CQ_THRE24,
    ISP_DRV_CQ_THRE25,
    ISP_DRV_CQ_THRE26,
    ISP_DRV_CQ_THRE27,
    ISP_DRV_CQ_THRE28,
    ISP_DRV_CAM_BASIC_CQ_NUM,    //baisc set, pass1 cqs
    ISP_DRV_CQ_NONE = ISP_DRV_CAM_BASIC_CQ_NUM
}E_ISP_CAM_CQ;

/**
    CQ trig mode
*/
typedef enum
{
    CQ_SINGLE_IMMEDIATE_TRIGGER = 0,
    CQ_SINGLE_EVENT_TRIGGER,
    CQ_CONTINUOUS_EVENT_TRIGGER,
    CQ_TRIGGER_MODE_NUM
}E_ISP_CAM_CQ_TRIGGER_MODE;


/**
    CQ trig source
*/
typedef enum
{
    CQ_TRIG_BY_START = 0,
    CQ_TRIG_BY_PASS1_DONE,
    CQ_TRIG_BY_IMGO_DONE,
    CQ_TRIG_BY_RRZO_DONE,
    CQ_TRIG_SRC_NUM,
    CQ_TRIG_BY_NONE = CQ_TRIG_SRC_NUM
}E_ISP_CAM_CQ_TRIGGER_SOURCE;


///////////////////////////////////////////////////////////
/**
    CAM bufctrl CmdQ special descriptor, and ob special descriptor
*/
typedef enum{
    BUF_CTRL_ENQUE  = 0,
    BUF_CTRL_ENQUE_HIGH_SPEED = 1,
    BUF_CTRL_CLEAR,
    SET_FH_SPARE,
    GET_RING_DEPTH,
}E_BUF_CTRL;

typedef struct{
    E_BUF_CTRL      ctrl;
    MUINT32         dma_PA;
    MUINT32         dma_FH_PA;
    MUINT32*        pDma_fh_spare;
}CQ_RingBuf_ST;

typedef struct _cq_cmd_st_CAM_
{
    unsigned int descriptor;
    unsigned int data;
}CQ_CMD_ST_CAM;

typedef struct _cq_info_rtbc_st_CAM_
{
    CQ_CMD_ST_CAM   dma;
    CQ_CMD_ST_CAM   dma_fh;
    CQ_CMD_ST_CAM   fh_spare;
    CQ_CMD_ST_CAM   next_Thread_addr;
    CQ_CMD_ST_CAM   next_Thread_addr_inner;
    CQ_CMD_ST_CAM   end;
    MUINT32         dma_base_pa;
    MUINT32         dma_fh_base_pa;
    MUINT32         dma_fh_spare[CAM_IMAGE_HEADER];
}CQ_INFO_RTBC_ST_CAM;

typedef struct _cq_ring_cmd_st_CAM_
{
    CQ_INFO_RTBC_ST_CAM cq_rtbc;
    unsigned long next_pa;
    unsigned int align_space[2]; //descp. table 16bytes alignment
    struct _cq_ring_cmd_st_CAM_ *pNext;
}CQ_RING_CMD_ST_CAM;

typedef struct _cq_rtbc_ring_st_CAM_
{
    CQ_RING_CMD_ST_CAM rtbc_ring[MAX_RING_SIZE];   //ring depth: sw maximum: 16 [hw maximus is depended on hw bits]
    unsigned int   dma_ring_size;
}CQ_RTBC_RING_ST_CAM;

//CQ0B for AE smoothing, set obc_gain0~3
typedef struct _cq0b_info_rtbc_st_CAM_
{
    CQ_CMD_ST_CAM ob;
    CQ_CMD_ST_CAM end;
}CQ0B_INFO_RTBC_ST_CAM;

typedef struct _cq0b_ring_cmd_st_CAM_
{
    CQ0B_INFO_RTBC_ST_CAM cq0b_rtbc;
    unsigned long next_pa;
    struct _cq0b_ring_cmd_st_CAM_ *pNext;
}CQ0B_RING_CMD_ST_CAM;

typedef struct _cq0b_rtbc_ring_st_CAM_
{
    CQ0B_RING_CMD_ST_CAM rtbc_ring;
}CQ0B_RTBC_RING_ST_CAM;

//////////////////////////////////////////////////////////////////
/**
    vir cq definition
*/
#define CmdQ_Cache  0   //CmdQ using by Cache is 1, Non-cache is 0
#define Max_PageNum 128  //e.g. nBurst * nDup * 2(normal/dummy) <= Max_PageNum

/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
//control range: CAM + UNI_VIR only
class ISP_DRV_CAM : public IspDrvVir, public IspDrv
{
    public:
        ~ISP_DRV_CAM(){};
        ISP_DRV_CAM(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx);
    public:

        static  IspDrvVir*  createInstance(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx,const char* userName);

        virtual void    destroyInstance(void);


        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   start(void);
        virtual MBOOL   stop(void);

        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   waitIrq(ISP_WAIT_IRQ_STRUCT* pWaitIrq);
        virtual MBOOL   clearIrq(ISP_CLEAR_IRQ_ST* pClearIrq);
        virtual MBOOL   registerIrq(ISP_REGISTER_USERKEY_STRUCT* pRegIrq);
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_ST* pWaitIrq);
        virtual MBOOL   signalIrq(ISP_WAIT_IRQ_STRUCT* pWaitIrq);

        virtual MBOOL   readRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MUINT32 readInnerReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);

        virtual MBOOL   getDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);
        virtual MBOOL   setDeviceInfo(E_DEVICE_INFO eCmd,MUINT8* pData);

        virtual MBOOL   cqAddModule(MUINT32 moduleId);
        virtual MBOOL   cqAddModule(MUINT32 moduleId, ISP_HW_MODULE slave_cam);
        virtual MBOOL   cqAddModule(MUINT32 moduleId, MUINT32 extModuleId, ISP_HW_MODULE slave_cam=CAM_MAX);
        virtual MBOOL   cqDelModule(MUINT32 moduleId);
        virtual MBOOL   cqNopModule(MUINT32 moduleId);
        virtual MBOOL   cqApbModule(MUINT32 moduleId);
        virtual MBOOL   dumpCQRingTable(void);
        virtual MBOOL   dumpCQTable(void);


        virtual MUINT32*    getCQDescBufPhyAddr(void);
        virtual MUINT32*    getCQDescBufVirAddr(void);
        virtual MUINT32*    getIspVirRegPhyAddr(void);
        virtual MUINT32*    getIspVirRegVirAddr(void);
        virtual MUINT32*    getHwRegAddr(void);

        virtual MBOOL       getCQModuleInfo(MUINT32 moduleId, MUINT32 &addrOffset, MUINT32 &moduleSize);
        virtual MBOOL       flushCmdQ(void);

        IspDrv*             getPhyObj(void);

        MBOOL               getCurObjInfo(ISP_HW_MODULE* p_module,E_ISP_CAM_CQ* p_cq,MUINT32* p_page);
        MBOOL               DumpReg(MBOOL bPhy);

        //for CQ those descriptor is static, not programable
        MUINT32             cqRingBuf(CQ_RingBuf_ST *pBuf_ctrl);

        //set cq in fixed format in which each reg have it's own descriptor with data length:1 only.  e.g.:CQ11...
        MBOOL               CQ_SetContent(ISP_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count);

        //read registers according to offset
        MBOOL               readPtrRegs(MUINT32 **ptr, MUINT32 offset, MUINT32 size,MINT32 caller=0);

        MBOOL               updateSVModule(E_CAM_MODULE moduleId,
                                           vector<MUINT32> reg_values,
                                           MUINT32 CamsvDCIFsubsample,
                                           ISP_HW_MODULE camsvModuleId);

        MBOOL               updateSVModule_120fps(E_CAM_MODULE moduleId,
                                            vector<MUINT32> reg_values,
                                            ISP_HW_MODULE camsvModuleId);

        //For secure camera 2nd DAPC register control
        MBOOL               setDapcReg(vector<MUINT32> reg_list,ISP_HW_MODULE module);
        vector<MUINT32>     getDapcReg(ISP_HW_MODULE module);

    private:
                MBOOL   FSM_CHK(MUINT32 op,const char* caller);
                //
                MBOOL   CQ_Allocate_method1(MUINT32 step);
                MBOOL   CQ_Allocate_method2(MUINT32 step);
                //
                MBOOL   updateEnqCqRingBuf(CQ_RingBuf_ST *pBuf_ctrl);
                MUINT32*    update_FH_Spare(CQ_RingBuf_ST *pBuf_ctrl);

                MUINT32 VirReg_OFFSET(MUINT32 hwModule);
                MBOOL   VirReg_ValidRW(void);
                MUINT32 twinPath_CQ_Ctrl(MUINT32 moduleId, ISP_HW_MODULE slave_cam); //this is for master cam to control salve cam under twin mode
    public:
        ISP_HW_MODULE   m_HWmasterModule; //To get system time of master when run twin.

    private:
        volatile MINT32         m_UserCnt;
        mutable android::Mutex           IspRegMutex;
        //
        MUINT32*        m_pIspVirRegAddr_va;
        MUINT32*        m_pIspVirRegAddr_pa;

        //
        ISP_DRV_CQ_CMD_DESC_STRUCT*         m_pIspDescript_vir;
        MUINT32*                            m_pIspDescript_phy;

        //
        MUINT32         m_FSM;
        //imem
        IMemDrv*        m_pMemDrv;
        IMEM_BUF_INFO   m_ispVirRegBufInfo;
        IMEM_BUF_INFO   m_ispCQDescBufInfo;

        E_ISP_CAM_CQ    m_CQ;
        ISP_HW_MODULE   m_HWModule;
        ISP_HW_MODULE   m_Module;           //phy + virtual hw module
        MUINT32         m_pageIdx;
        char            m_useName[32];
        MUINT32         m_currentDCIFsumsampleRecode;

        IspDrv*         m_pIspDrvImp;       //ptr for access CAM_A/CAM_B ...etc. phy
        vector<string>  m_useNameInfo;
        //For secure camera 2nd DAPC register control
        static vector<MUINT32> m_DapcIdx;
        static MBOOL m_SecOn;
        static vector<MUINT32> m_DapcReg[PHY_CAM];
};





#endif //_ISP_DRV_CAM_H_
