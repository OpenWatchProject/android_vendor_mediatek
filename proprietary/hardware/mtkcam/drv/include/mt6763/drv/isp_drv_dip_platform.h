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

//#include <mtkcam/drv/isp_drv.h>
#include "isp_drv.h"


//////////////////////////////////////////////////////////////////
#define MAX_BURST_QUE_NUM   10
#define MAX_DUP_CQ_NUM      2


/**
DIP cq module info
*/
#define ISP_CQ_APB_INST             0x0
#define ISP_CQ_NOP_INST             0x1
#define ISP_DRV_CQ_END_TOKEN        0x1C000000
#define ISP_DRV_CQ_DUMMY_TOKEN      ((ISP_CQ_NOP_INST<<26)|0x4184)
#define ISP_CQ_DUMMY_PA             0x88100000

#define ISP_DIP_CQ_DUMMY_WR_TOKEN   0x27000

#define CMDQ_HW_BUG         0

///////////////////////////////////////////////////////////
/**
DIP CQ descriptor
*/
#if CMDQ_HW_BUG
typedef enum {
    DIP_A_CTL_BUG_1 = 0,    //0x15022004
    DIP_A_CTL,              //0x15022004~0x1502201c
    DIP_A_CTL_BUG_2,        //0x1502201c
    DIP_A_CTL_TDR_BUG_1,    //0x15022050
    DIP_A_CTL_TDR,          //0x15022050~0x1502205c
    DIP_A_CTL_TDR_BUG_2,    //0x1502205c
    DIP_A_CTL_DONE,         //0x1502207c
    DIP_A_TDRI_BUG_1,       //0x15022204
    DIP_A_TDRI,             //0x15022204~0x1502220c
    DIP_A_TDRI_BUG_2,       //0x1502220c
    DIP_A_VECTICAL_FLIP,    //0x15022210
    DIP_A_SPECIAL_FUN,      //0x1502221c
    DIP_A_IMG2O_BUG_1,      //0x15022230
    DIP_A_IMG2O,            //0x15022230~0x15022248
    DIP_A_IMG2O_BUG_2,      //0x15022248
    DIP_A_IMG2O_CRSP,       //0x15022258
    DIP_A_IMG2BO_BUG_1,     //0x15022260
    DIP_A_IMG2BO,           //0x15022260~0x15022278
    DIP_A_IMG2BO_BUG_2,     //0x15022278
    DIP_A_IMG2BO_CRSP,      //0x15022288
    DIP_A_IMG3O_BUG_1,      //0x15022290
    DIP_A_IMG3O,            //0x15022290~0x150222a8
    DIP_A_IMG3O_BUG_2,      //0x150222a8
    DIP_A_IMG3O_CRSP,       //0x150222b8
    DIP_A_IMG3BO_BUG_1,     //0x150222c0
    DIP_A_IMG3BO,           //0x150222c0~0x150222d8
    DIP_A_IMG3BO_BUG_2,     //0x150222d8
    DIP_A_IMG3BO_CRSP,      //0x150222e8
    DIP_A_IMG3CO_BUG_1,     //0x150222f0
    DIP_A_IMG3CO,           //0x150222f0~0x15022308
    DIP_A_IMG3CO_BUG_2,     //0x15022308
    DIP_A_IMG3CO_CRSP,      //0x15022318
    DIP_A_FEO_BUG_1,        //0x15022320
    DIP_A_FEO,              //0x15022320~0x15022338
    DIP_A_FEO_BUG_2,        //0x15022338
    DIP_A_MFBO_BUG_1,       //0x15022350
    DIP_A_MFBO,             //0x15022350~0x15022368
    DIP_A_MFBO_BUG_2,       //0x15022368
    DIP_A_MFBO_CROP,        //0x15022378
    DIP_A_IMGI_BUG_1,       //0x15022400
    DIP_A_IMGI,             //0x15022400~0x15022418
    DIP_A_IMGI_BUG_2,       //0x15022418
    DIP_A_IMGBI_BUG_1,      //0x15022430
    DIP_A_IMGBI,            //0x15022430~0x15022448
    DIP_A_IMGBI_BUG_2,      //0x15022448
    DIP_A_IMGCI_BUG_1,      //0x15022460
    DIP_A_IMGCI,            //0x15022460~0x15022478
    DIP_A_IMGCI_BUG_2,      //0x15022478
    DIP_A_VIPI_BUG_1,       //0x15022490
    DIP_A_VIPI,             //0x15022490~0x150224a8
    DIP_A_VIPI_BUG_2,       //0x150224a8
    DIP_A_VIP2I_BUG_1,      //0x150224c0
    DIP_A_VIP2I,            //0x150224c0~0x150224d8
    DIP_A_VIP2I_BUG_2,      //0x150224d8
    DIP_A_VIP3I_BUG_1,      //0x150224f0
    DIP_A_VIP3I,            //0x150224f0~0x15022508
    DIP_A_VIP3I_BUG_2,      //0x15022508
    DIP_A_DMGI_BUG_1,       //0x15022520
    DIP_A_DMGI,             //0x15022520~0x15022538
    DIP_A_DMGI_BUG_2,       //0x15022538
    DIP_A_DEPI_BUG_1,       //0x15022550
    DIP_A_DEPI,             //0x15022550~0x15022568
    DIP_A_DEPI_BUG_2,       //0x15022568
    DIP_A_LCEI_BUG_1,       //0x15022580
    DIP_A_LCEI,             //0x15022580~0x15022598
    DIP_A_LCEI_BUG_2,       //0x15022598
    DIP_A_UFDI_BUG_1,       //0x150225b0
    DIP_A_UFDI,             //0x150225b0~0x150225c8
    DIP_A_UFDI_BUG_2,       //0x150225c8
    DIP_A_UFD_BUG_1,        //0x15022700
    DIP_A_UFD,              //0x15022700~0x15022710
    DIP_A_UFD_BUG_2,        //0x15022710
    DIP_A_PGN_BUG_1,        //0x15022720
    DIP_A_PGN,              //0x15022720~0x15022734
    DIP_A_PGN_BUG_2,        //0x15022734
    DIP_A_SL2_BUG_1,        //0x15022740
    DIP_A_SL2,              //0x15022740~0x1502276C
    DIP_A_SL2_BUG_2,        //0x1502276c
    DIP_A_UDM_BUG_1,        //0x15022780
    DIP_A_UDM,              //0x15022780~0x150227c8
    DIP_A_UDM_BUG_2,        //0x150227c8
    DIP_A_G2G_BUG_1,        //0x150227f0
    DIP_A_G2G,              //0x150227f0~0x1502280c
    DIP_A_G2G_BUG_2,        //0x1502280c
    DIP_A_MFB_BUG_1,        //0x15022f60
    DIP_A_MFB,              //0x15022f60 ~ 0x15022fb8
    DIP_A_MFB_BUG_2,        //0x15022fb8
    DIP_A_FLC_BUG_1,        //0x15022FC0
    DIP_A_FLC,              //0x15022FC0~0x15022FD8
    DIP_A_FLC_BUG_2,        //0x15022FD8
    DIP_A_FLC2_BUG_1,       //0x15022FE0
    DIP_A_FLC2,             //0x15022FE0~0x15022FF8
    DIP_A_FLC2_BUG_2,       //0x15022FF8
    DIP_A_G2C_BUG_1,        //0x15022840
    DIP_A_G2C,              //0x15022840~0x15022870
    DIP_A_G2C_BUG_2,        //0x15022870
    DIP_A_C42,              //0x15022880
    DIP_A_SRZ3_BUG_1,       //0x15022890
    DIP_A_SRZ3,             //0x15022890~0x150228b0
    DIP_A_SRZ3_BUG_2,       //0x150228b0
    DIP_A_SRZ4_BUG_1,       //0x150228c0
    DIP_A_SRZ4,             //0x150228c0~0x150228e0
    DIP_A_SRZ4_BUG_2,       //0x150228e0
    DIP_A_SRZ5_BUG_1,       //0x150228F0
    DIP_A_SRZ5,             //0x150228F0~0x15022910
    DIP_A_SRZ5_BUG_2,       //0x15022910
    DIP_A_SEEE_BUG_1,       //0x15022990
    DIP_A_SEEE,             //0x15022990~0x150229f8
    DIP_A_SEEE_BUG_2,       //0x150229f8
    DIP_A_NDG2_BUG_1,       //0x15022a10
    DIP_A_NDG2,             //0x15022a10~0x15022a24
    DIP_A_NDG2_BUG_2,       //0x15022a24
    DIP_A_NR3D_BUG_1,       //0x15022a30
    DIP_A_NR3D,             //0x15022a30~0x15022ac8
    DIP_A_NR3D_BUG_2,       //0x15022ac8
    DIP_A_SL2B_BUG_1,       //0x15022b30
    DIP_A_SL2B,             //0x15022b30~0x15022b5C
    DIP_A_SL2B_BUG_2,       //0x15022b5c
    DIP_A_SL2C_BUG_1,       //0x15022b70
    DIP_A_SL2C,             //0x15022b70~0x15022b9c
    DIP_A_SL2C_BUG_2,       //0x15022b9c
    DIP_A_SRZ1_BUG_1,       //0x15022bb0
    DIP_A_SRZ1,             //0x15022bb0~0x15022bd0
    DIP_A_SRZ1_BUG_2,       //0x15022bd0
    DIP_A_SRZ2_BUG_1,       //0x15022be0
    DIP_A_SRZ2,             //0x15022be0~0x15022c00
    DIP_A_SRZ2_BUG_2,       //0x15022c00
    DIP_A_CRZ_BUG_1,        //0x15022c10
    DIP_A_CRZ,              //0x15022c10~0x15022c44
    DIP_A_CRZ_BUG_2,        //0x15022c44
    DIP_A_MIX1_BUG_1,       //0x15022c70
    DIP_A_MIX1,             //0x15022c70~0x15022c74
    DIP_A_MIX1_BUG_2,       //0x15022c74
    DIP_A_MIX2_BUG_1,       //0x15022c80
    DIP_A_MIX2,             //0x15022c80~0x15022c84
    DIP_A_MIX2_BUG_2,       //0x15022c84
    DIP_A_MIX3_BUG_1,       //0x15022c90
    DIP_A_MIX3,             //0x15022c90~0x15022c94
    DIP_A_MIX3_BUG_2,       //0x15022c94
    DIP_A_SL2D_BUG_1,       //0x15022ca0
    DIP_A_SL2D,             //0x15022ca0~0x15022ccc
    DIP_A_SL2D_BUG_2,       //0x15022ccc
    DIP_A_SL2E_BUG_1,       //0x15022ce0
    DIP_A_SL2E,             //0x15022ce0~0x15022d0c
    DIP_A_SL2E_BUG_2,       //0x15022d0c
    DIP_A_MDP_BUG_1,        //0x15022d20
    DIP_A_MDP,              //0x15022d20~0x15022d24
    DIP_A_MDP_BUG_2,        //0x15022d24
    DIP_A_ANR2_BUG_1,       //0x15022d30
    DIP_A_ANR2,             //0x15022d30~0x15022da4
    DIP_A_ANR2_BUG_2,       //0x15022da4
    DIP_A_CCR_BUG_1,        //0x15022db0
    DIP_A_CCR,              //0x15022db0~0x15022ddc
    DIP_A_CCR_BUG_2,        //0x15022ddc
    DIP_A_BOK_BUG_1,        //0x15022de0
    DIP_A_BOK,              //0x15022de0~0x15022de8
    DIP_A_BOK_BUG_2,        //0x15022de8
    DIP_A_ABF_BUG_1,        //0x15022df0
    DIP_A_ABF,              //0x15022df0~0x15022e18
    DIP_A_ABF_BUG_2,        //0x15022e18
    DIP_A_UNP,              //0x15022e30
    DIP_A_C02_BUG_1,        //0x15022e40
    DIP_A_C02,              //0x15022e40~0x15022e48
    DIP_A_C02_BUG_2,        //0x15022e48
    DIP_A_FE_BUG_1,         //0x15022e50
    DIP_A_FE,               //0x15022e50~0x15022e60
    DIP_A_FE_BUG_2,         //0x15022e60
    DIP_A_CRSP_BUG_1,       //0x15022e70
    DIP_A_CRSP,             //0x15022e70~0x15022e84
    DIP_A_CRSP_BUG_2,       //0x15022e84
    DIP_A_C02B_BUG_1,       //0x15022e90
    DIP_A_C02B,             //0x15022e90~0x15022e98
    DIP_A_C02B_BUG_2,       //0x15022e98
    DIP_A_C24,              //0x15022ea0
    DIP_A_C24B,             //0x15022eb0
    DIP_A_LCE_BUG_1,        //0x15022ec0
    DIP_A_LCE,              //0x15022ec0~0x15022ef8
    DIP_A_LCE_BUG_2,        //0x15022ef8
    DIP_A_FM_BUG_1,         //0x15022f40
    DIP_A_FM,               //0x15022f40~0x15022f48
    DIP_A_FM_BUG_2,         //0x15022f48
    DIP_A_GGM_BUG_1,        //0x15023000
    DIP_A_GGM,              //0x15023000~0x15023480
    DIP_A_GGM_BUG_2,        //0x15023480
    DIP_A_PCA_BUG_1,        //0x15023800
    DIP_A_PCA,              //0x15023800~0x15023e18
    DIP_A_PCA_BUG_2,        //0x15023e18
    DIP_A_OBC2_BUG_1,       //0x15023e40
    DIP_A_OBC2,             //0x15023e40~0x15023e5c
    DIP_A_OBC2_BUG_2,       //0x15023e5c
    DIP_A_BNR2_BUG_1,       //0x15023e70
    DIP_A_BNR2,             //0x15023e70~0x15023eec
    DIP_A_BNR2_BUG_2,       //0x15023eec
    DIP_A_RMG2_BUG_1,       //0x15023ef0
    DIP_A_RMG2,             //0x15023ef0~0x15023ef8
    DIP_A_RMG2_BUG_2,       //0x15023ef8
    DIP_A_RMM2_BUG_1,       //0x15023f10
    DIP_A_RMM2,             //0x15023f10~0x15023f30
    DIP_A_RMM2_BUG_2,       //0x15023f30
    DIP_A_RCP2_BUG_1,       //0x15023f60
    DIP_A_RCP2,             //0x15023f60~0x15023f64
    DIP_A_RCP2_BUG_2,       //0x15023f64
    DIP_A_LSC2_BUG_1,       //0x15023f70
    DIP_A_LSC2,             //0x15023f70~0x15023f90
    DIP_A_LSC2_BUG_2,       //0x15023f90
    DIP_A_DBS2_BUG_1,       //0x15023fa0
    DIP_A_DBS2,             //0x15023fa0~0x15023fd0
    DIP_A_DBS2_BUG_2,       //0x15023fd0
    DIP_A_ANR_BUG_1,        //0x15024000
    DIP_A_ANR,              //0x15024000~0x150244b0
    DIP_A_ANR_BUG_2,        //0x150244b0
    DIP_A_SL2G_BUG_1,       //0x15024500
    DIP_A_SL2G,             //0x15024500~0x1502452C
    DIP_A_SL2G_BUG_2,       //0x1502452C
    DIP_A_SL2H_BUG_1,       //0x15024540
    DIP_A_SL2H,             //0x15024540~0x1502456C
    DIP_A_SL2H_BUG_2,       //0x1502456C
    DIP_A_HLR2_BUG_1,       //0x15024580
    DIP_A_HLR2,             //0x15024580~0x15024580
    DIP_A_HLR2_BUG_2,       //0x15024580
    DIP_A_HLR2_1_BUG_1,     //0x15024588
    DIP_A_HLR2_1,           //0x15024588~0x150245ac
    DIP_A_HLR2_1_BUG_2,     //0x150245ac
    DIP_A_RNR_BUG_1,        //0x150245b0
    DIP_A_RNR,              //0x150245b0~0x150245ec
    DIP_A_RNR_BUG_2,        //0x150245ec
    DIP_A_HFG_BUG_1,        //0x15024630
    DIP_A_HFG,              //0x15024630~0x15024664
    DIP_A_HFG_BUG_2,        //0x15024664
    DIP_A_SL2I_BUG_1,       //0x15024670
    DIP_A_SL2I,             //0x15024670~0x1502469C
    DIP_A_SL2I_BUG_2,       //0x1502469c
    DIP_A_SL2K_BUG_1,       //0x150246B0
    DIP_A_SL2K,             //0x150246B0 ~ 0x150246DC
    DIP_A_SL2K_BUG_2,       //0x150246DC
    DIP_A_COLOR_BUG_1,      //0x150246F0
    DIP_A_COLOR,            //0x150246F0 ~ 0x15024958
    DIP_A_COLOR_BUG_2,      //0x15024958
    DIP_A_NDG_BUG_1,        //0x15024960
    DIP_A_NDG,              //0x15024960~0x15024974
    DIP_A_NDG_BUG_2,        //0x15024974
    DIP_A_END_,             //must be kept at the end of descriptor
    //CQ ==> 0x15022100~0x150221c0 (set them via physical address directly)
    DIP_A_MODULE_MAX,
    DIP_A_DUMMY = 0xff
}DIP_A_MODULE_ENUM;
#else
typedef enum {
    DIP_A_CTL = 0,          //0x15022004~0x1502201c
    DIP_A_CTL_TDR,          //0x15022050~0x1502205c
    DIP_A_CTL_DONE,         //0x1502207c
    DIP_A_TDRI,             //0x15022204~0x1502220c
    DIP_A_VECTICAL_FLIP,    //0x15022210
    DIP_A_SPECIAL_FUN,      //0x1502221c
    DIP_A_IMG2O,            //0x15022230~0x15022248
    DIP_A_IMG2O_CRSP,       //0x15022230~0x15022258
    DIP_A_IMG2BO,           //0x15022260~0x15022278
    DIP_A_IMG2BO_CRSP,      //0x15022260~0x15022288
    DIP_A_IMG3O,            //0x15022290~0x150222a8
    DIP_A_IMG3O_CRSP,       //0x15022290~0x150222b8
    DIP_A_IMG3BO,           //0x150222c0~0x150222d8
    DIP_A_IMG3BO_CRSP,      //0x150222c0~0x150222e8
    DIP_A_IMG3CO,           //0x150222f0~0x15022308
    DIP_A_IMG3CO_CRSP,      //0x150222f0~0x15022318
    DIP_A_FEO,              //0x15022320~0x15022338
    DIP_A_MFBO,             //0x15022350~0x15022368
    DIP_A_MFBO_CROP,        //0x15022350~0x15022378
    DIP_A_IMGI,             //0x15022400~0x15022418
    DIP_A_IMGBI,            //0x15022430~0x15022444
    DIP_A_IMGCI,            //0x15022460~0x15022478
    DIP_A_VIPI,             //0x15022490~0x150224a8
    DIP_A_VIP2I,            //0x150224c0~0x150224d8
    DIP_A_VIP3I,            //0x150224f0~0x15022508
    DIP_A_DMGI,             //0x15022520~0x15022538
    DIP_A_DEPI,             //0x15022550~0x15022568
    DIP_A_LCEI,             //0x15022580~0x15022598
    DIP_A_UFDI,             //0x150225b0~0x150225c8
    DIP_A_UFD,              //0x15022700~0x15022710
    DIP_A_PGN,              //0x15022720~0x15022734
    DIP_A_SL2,              //0x15022740~0x1502276C
    DIP_A_UDM,              //0x15022780~0x150227c8
    DIP_A_G2G,              //0x150227f0~0x1502280c
    DIP_A_MFB,              //0x15022f60~0x15022fb8
    DIP_A_FLC,              //0x15022FC0~0x15022FD8
    DIP_A_FLC2,             //0x15022FE0~0x15022FF8
    DIP_A_G2C,              //0x15022840~0x15022870
    DIP_A_C42,              //0x15022880
    DIP_A_SRZ3,             //0x15022890~0x150228b0
    DIP_A_SRZ4,             //0x150228c0~0x150228e0
    DIP_A_SRZ5,             //0x150228F0~0x15022910
    DIP_A_SEEE,             //0x15022990~0x150229f8
    DIP_A_NDG2,             //0x15022a10~0x15022a24
    DIP_A_NR3D,             //0x15022a30~0x15022ac8
    DIP_A_SL2B,             //0x15022b30~0x15022b5C
    DIP_A_SL2C,             //0x15022b70~0x15022b9C
    DIP_A_SRZ1,             //0x15022bb0~0x15022bd0
    DIP_A_SRZ2,             //0x15022be0~0x15022c00
    DIP_A_CRZ,              //0x15022c10~0x15022c44
    DIP_A_MIX1,             //0x15022c70~0x15022c74
    DIP_A_MIX2,             //0x15022c80~0x15022c88
    DIP_A_MIX3,             //0x15022c90~0x15022c90
    DIP_A_SL2D,             //0x15022ca0~0x15022ccc
    DIP_A_SL2E,             //0x15022ce0~0x15022d0c
    DIP_A_MDP,              //0x15022d20~0x15022d24
    DIP_A_ANR2,             //0x15022d30~0x15022da4
    DIP_A_CCR,              //0x15022db0~0x15022ddc
    DIP_A_BOK,              //0x15022de0~0x15022de8
    DIP_A_ABF,              //0x15022df0~0x15022e18
    DIP_A_UNP,              //0x15022e30
    DIP_A_C02,              //0x15022e40~0x15022e48
    DIP_A_FE,               //0x15022e50~0x15022e60
    DIP_A_CRSP,             //0x15022e70~0x15022e84
    DIP_A_C02B,             //0x15022e90~0x15022e98
    DIP_A_C24,              //0x15022ea0
    DIP_A_C24B,             //0x15022eb0
    DIP_A_LCE,              //0x15022ec0~0x15022ef8
    DIP_A_FM,               //0x15022f40~0x15022f48
    DIP_A_GGM,              //0x15023000~0x15023480
    DIP_A_PCA,              //0x15023800~0x15023e18
    DIP_A_OBC2,             //0x15023e40~0x15023e5c
    DIP_A_BNR2,             //0x15023e70~0x15023eec
    DIP_A_RMG2,             //0x15023ef0~0x15023ef8
    DIP_A_RMM2,             //0x15023f10~0x15023f30
    DIP_A_RCP2,             //0x15023f60~0x15023f64
    DIP_A_LSC2,             //0x15023f70~0x15023f90
    DIP_A_DBS2,             //0x15023fa0~0x15023fd0
    DIP_A_ANR,              //0x15024000~0x150244b0
    DIP_A_SL2G,             //0x15024500~0x1502452C
    DIP_A_SL2H,             //0x15024540~0x1502456C
    DIP_A_HLR2,             //0x15024580~0x15024580
    DIP_A_HLR2_1,           //0x15024588~0x150245ac
    DIP_A_RNR,              //0x150245b0~0x150245ec
    DIP_A_HFG,              //0x15024630~0x15024664
    DIP_A_SL2I,             //0x15024670~0x1502469C
    DIP_A_SL2K,             //0x150246B0 ~ 0x150246DC
    DIP_A_COLOR,            //0x150246F0 ~ 0x15024958
    DIP_A_NDG,              //0x15024960~0x15024974
    DIP_A_END_,             //must be kept at the end of descriptor
    //CQ ==> 0x15022100~0x150221c0 (set them via physical address directly)
    DIP_A_MODULE_MAX,
    DIP_A_DUMMY = 0xff
}DIP_A_MODULE_ENUM;
#endif

#if CMDQ_HW_BUG
static ISP_DRV_CQ_MODULE_INFO_STRUCT mIspDipCQModuleInfo[DIP_A_MODULE_MAX]
   =  {{DIP_A_CTL_BUG_1,        0x22004, 0x0004, 1  },  //0x15022004
       {DIP_A_CTL,              0x22004, 0x0004, 7  },  //0x15022004 ~ 0x1502201c
       {DIP_A_CTL_BUG_2,        0x2201c, 0x001c, 1  },  //0x1502201c
       {DIP_A_CTL_TDR_BUG_1,    0x22050, 0x0050, 1  },  //0x15022050
       {DIP_A_CTL_TDR,          0x22050, 0x0050, 4  },  //0x15022050 ~ 0x1502205c
       {DIP_A_CTL_TDR_BUG_2,    0x2205c, 0x005c, 1  },  //0x1502205c
       {DIP_A_CTL_DONE,         0x2207c, 0x007c, 1  },  //0x1502207c
       {DIP_A_TDRI_BUG_1,       0x22204, 0x0204, 1  },  //0x15022204
       {DIP_A_TDRI,             0x22204, 0x0204, 3  },  //0x15022204 ~ 0x1502220c
       {DIP_A_TDRI_BUG_2,       0x2220c, 0x020c, 1  },  //0x1502220c
       {DIP_A_VECTICAL_FLIP,    0x22210, 0x0210, 1  },  //0x15022210
       {DIP_A_SPECIAL_FUN,      0x2221c, 0x021c, 1  },  //0x1502221c
       {DIP_A_IMG2O_BUG_1,      0x22230, 0x0230, 1  },  //0x15022230
       {DIP_A_IMG2O,            0x22230, 0x0230, 7  },  //0x15022230 ~ 0x15022248
       {DIP_A_IMG2O_BUG_2,      0x22248, 0x0248, 1  },  //0x15022248
       {DIP_A_IMG2O_CRSP,       0x22258, 0x0258, 1  },  //0x15022258
       {DIP_A_IMG2BO_BUG_1,     0x22260, 0x0260, 1  },  //0x15022260
       {DIP_A_IMG2BO,           0x22260, 0x0260, 7  },  //0x15022260 ~ 0x15022278
       {DIP_A_IMG2BO_BUG_2,     0x22278, 0x0278, 1  },  //0x15022278
       {DIP_A_IMG2BO_CRSP,      0x22288, 0x0288, 1  },  //0x15022288
       {DIP_A_IMG3O_BUG_1,      0x22290, 0x0290, 1  },  //0x15022290
       {DIP_A_IMG3O,            0x22290, 0x0290, 7  },  //0x15022290 ~ 0x150222a8
       {DIP_A_IMG3O_BUG_2,      0x222a8, 0x02a8, 1  },  //0x150222a8
       {DIP_A_IMG3O_CRSP,       0x222b8, 0x02b8, 1  },  //0x150222b8
       {DIP_A_IMG3BO_BUG_1,     0x222c0, 0x02c0, 1  },  //0x150222c0
       {DIP_A_IMG3BO,           0x222c0, 0x02c0, 7  },  //0x150222c0 ~ 0x150222d8
       {DIP_A_IMG3BO_BUG_2,     0x222d8, 0x02d8, 1  },  //0x150222d8
       {DIP_A_IMG3BO_CRSP,      0x222e8, 0x02e8, 1  },  //0x150222e8
       {DIP_A_IMG3CO_BUG_1,     0x222f0, 0x02f0, 1  },  //0x150222f0
       {DIP_A_IMG3CO,           0x222f0, 0x02f0, 7  },  //0x150222f0 ~ 0x15022308
       {DIP_A_IMG3CO_BUG_2,     0x22308, 0x0308, 1  },  //0x15022308
       {DIP_A_IMG3CO_CRSP,      0x22318, 0x0318, 1  },  //0x15022318
       {DIP_A_FEO_BUG_1,        0x22320, 0x0320, 1  },  //0x15022320
       {DIP_A_FEO,              0x22320, 0x0320, 7  },  //0x15022320 ~ 0x15022338
       {DIP_A_FEO_BUG_2,        0x22338, 0x0338, 1  },  //0x15022338
       {DIP_A_MFBO_BUG_1,       0x22350, 0x0350, 1  },  //0x15022350
       {DIP_A_MFBO,             0x22350, 0x0350, 7  },  //0x15022350 ~ 0x15022368
       {DIP_A_MFBO_BUG_2,       0x22368, 0x0368, 1  },  //0x15022368
       {DIP_A_MFBO_CROP,        0x22378, 0x0378, 1  },  //0x15022378
       {DIP_A_IMGI_BUG_1,       0x22400, 0x0400, 1  },  //0x15022400
       {DIP_A_IMGI,             0x22400, 0x0400, 7  },  //0x15022400 ~ 0x15022418
       {DIP_A_IMGI_BUG_2,       0x22418, 0x0418, 1  },  //0x15022418
       {DIP_A_IMGBI_BUG_1,      0x22430, 0x0430, 1  },  //0x15022430
       {DIP_A_IMGBI,            0x22430, 0x0430, 7  },  //0x15022430 ~ 0x15022448
       {DIP_A_IMGBI_BUG_2,      0x22448, 0x0448, 1  },  //0x15022448
       {DIP_A_IMGCI_BUG_1,      0x22460, 0x0460, 1  },  //0x15022460
       {DIP_A_IMGCI,            0x22460, 0x0460, 7  },  //0x15022460 ~ 0x15022478
       {DIP_A_IMGCI_BUG_2,      0x22478, 0x0478, 1  },  //0x15022478
       {DIP_A_VIPI_BUG_1,       0x22490, 0x0490, 1  },  //0x15022490
       {DIP_A_VIPI,             0x22490, 0x0490, 7  },  //0x15022490 ~ 0x150224a8
       {DIP_A_VIPI_BUG_2,       0x224a8, 0x04a8, 1  },  //0x150224a8
       {DIP_A_VIP2I_BUG_1,      0x224c0, 0x04c0, 1  },  //0x150224c0
       {DIP_A_VIP2I,            0x224c0, 0x04c0, 7  },  //0x150224c0 ~ 0x150224d8
       {DIP_A_VIP2I_BUG_2,      0x224d8, 0x04d8, 1  },  //0x150224d8
       {DIP_A_VIP3I_BUG_1,      0x224f0, 0x04f0, 1  },  //0x150224f0
       {DIP_A_VIP3I,            0x224f0, 0x04f0, 7  },  //0x150224f0 ~ 0x15022508
       {DIP_A_VIP3I_BUG_2,      0x22508, 0x0508, 1  },  //0x15022508
       {DIP_A_DMGI_BUG_1,       0x22520, 0x0520, 1  },  //0x15022520
       {DIP_A_DMGI,             0x22520, 0x0520, 7  },  //0x15022520 ~ 0x15022538
       {DIP_A_DMGI_BUG_2,       0x22538, 0x0538, 1  },  //0x15022538
       {DIP_A_DEPI_BUG_1,       0x22550, 0x0550, 1  },  //0x15022550
       {DIP_A_DEPI,             0x22550, 0x0550, 7  },  //0x15022550 ~ 0x15022568
       {DIP_A_DEPI_BUG_2,       0x22568, 0x0568, 1  },  //0x15022568
       {DIP_A_LCEI_BUG_1,       0x22580, 0x0580, 1  },  //0x15022580
       {DIP_A_LCEI,             0x22580, 0x0580, 7  },  //0x15022580 ~ 0x15022598
       {DIP_A_LCEI_BUG_2,       0x22598, 0x0598, 1  },  //0x15022598
       {DIP_A_UFDI_BUG_1,       0x225b0, 0x05b0, 1  },  //0x150225b0
       {DIP_A_UFDI,             0x225b0, 0x05b0, 7  },  //0x150225b0 ~ 0x150225c8
       {DIP_A_UFDI_BUG_2,       0x225c8, 0x05c8, 1  },  //0x150225c8
       {DIP_A_UFD_BUG_1,        0x22700, 0x0700, 1  },  //0x15022700
       {DIP_A_UFD,              0x22700, 0x0700, 5  },  //0x15022700 ~ 0x15022710
       {DIP_A_UFD_BUG_2,        0x22710, 0x0710, 1  },  //0x15022710
       {DIP_A_PGN_BUG_1,        0x22720, 0x0720, 1  },  //0x15022720
       {DIP_A_PGN,              0x22720, 0x0720, 6  },  //0x15022720 ~ 0x15022734
       {DIP_A_PGN_BUG_2,        0x22734, 0x0734, 1  },  //0x15022734
       {DIP_A_SL2_BUG_1,        0x22740, 0x0740, 1  },  //0x15022740
       {DIP_A_SL2,              0x22740, 0x0740, 12 },  //0x15022740 ~ 0x1502276c
       {DIP_A_SL2_BUG_2,        0x2276c, 0x076c, 1  },  //0x1502276c
       {DIP_A_UDM_BUG_1,        0x22780, 0x0780, 1  },  //0x15022780
       {DIP_A_UDM,              0x22780, 0x0780, 19 },  //0x15022780 ~ 0x150227c8
       {DIP_A_UDM_BUG_2,        0x227c8, 0x07c8, 1  },  //0x150227c8
       {DIP_A_G2G_BUG_1,        0x227f0, 0x07f0, 1  },  //0x150227f0
       {DIP_A_G2G,              0x227f0, 0x07f0, 8  },  //0x150227f0 ~ 0x1502280c
       {DIP_A_G2G_BUG_2,        0x2280c, 0x080c, 1  },  //0x1502280c
       {DIP_A_MFB_BUG_1,        0x22f60, 0x0f60, 1  },  //0x15022f60
       {DIP_A_MFB,              0x22f60, 0x0f60, 23 },  //0x15022f60 ~ 0x15022fb8
       {DIP_A_MFB_BUG_2,        0x22fb8, 0x0fb8, 1  },  //0x15022fb8
       {DIP_A_FLC_BUG_1,        0x22fc0, 0x0fc0, 1  },  //0x15022FC0
       {DIP_A_FLC,              0x22fc0, 0x0fc0, 7  },  //0x15022FC0 ~ 0x15022FD8
       {DIP_A_FLC_BUG_2,        0x22fd8, 0x0fd8, 1  },  //0x15022FD8
       {DIP_A_FLC2_BUG_1,       0x22fe0, 0x0fe0, 1  },  //0x15022FE0
       {DIP_A_FLC2,             0x22fe0, 0x0fe0, 7  },  //0x15022FE0 ~ 0x15022FF8
       {DIP_A_FLC2_BUG_2,       0x22ff8, 0x0ff8, 1  },  //0x15022FF8
       {DIP_A_G2C_BUG_1,        0x22840, 0x0840, 1  },  //0x15022840
       {DIP_A_G2C,              0x22840, 0x0840, 13 },  //0x15022840 ~ 0x15022870
       {DIP_A_G2C_BUG_2,        0x22870, 0x0870, 1  },  //0x15022870
       {DIP_A_C42,              0x22880, 0x0880, 1  },  //0x15022880
       {DIP_A_SRZ3_BUG_1,       0x22890, 0x0890, 1  },  //0x15022890
       {DIP_A_SRZ3,             0x22890, 0x0890, 9  },  //0x15022890 ~ 0x150228b0
       {DIP_A_SRZ3_BUG_2,       0x228b0, 0x08b0, 1  },  //0x150228b0
       {DIP_A_SRZ4_BUG_1,       0x228c0, 0x08c0, 1  },  //0x150228c0
       {DIP_A_SRZ4,             0x228c0, 0x08c0, 9  },  //0x150228c0 ~ 0x150228e0
       {DIP_A_SRZ4_BUG_2,       0x228e0, 0x08e0, 1  },  //0x150228e0
       {DIP_A_SRZ5_BUG_1,       0x228f0, 0x08f0, 1  },  //0x150228F0
       {DIP_A_SRZ5,             0x228f0, 0x08f0, 9  },  //0x150228F0~0x15022910
       {DIP_A_SRZ5_BUG_2,       0x22910, 0x0910, 1  },  //0x15022910
       {DIP_A_SEEE_BUG_1,       0x22990, 0x0990, 1  },  //0x15022990
       {DIP_A_SEEE,             0x22990, 0x0990, 27 },  //0x15022990 ~ 0x150229f8
       {DIP_A_SEEE_BUG_2,       0x229f8, 0x09f8, 1  },  //0x150229f8
       {DIP_A_NDG2_BUG_1,       0x22a10, 0x0a10, 1  },  //0x15022a10
       {DIP_A_NDG2,             0x22a10, 0x0a10, 6  },  //0x15022a10~0x15022a24
       {DIP_A_NDG2_BUG_2,       0x22a24, 0x0a24, 1  },  //0x15022a24
       {DIP_A_NR3D_BUG_1,       0x22a30, 0x0a30, 1  },  //0x15022a30
       {DIP_A_NR3D,             0x22a30, 0x0a30, 39 },  //0x15022a30 ~ 0x15022ac8
       {DIP_A_NR3D_BUG_2,       0x22ac8, 0x0ac8, 1  },  //0x15022ac8
       {DIP_A_SL2B_BUG_1,       0x22b30, 0x0b30, 1  },  //0x15022b30
       {DIP_A_SL2B,             0x22b30, 0x0b30, 12 },  //0x15022b30 ~ 0x15022b5c
       {DIP_A_SL2B_BUG_2,       0x22b5c, 0x0b5c, 1  },  //0x15022b5c
       {DIP_A_SL2C_BUG_1,       0x22b70, 0x0b70, 1  },  //0x15022b70
       {DIP_A_SL2C,             0x22b70, 0x0b70, 12 },  //0x15022b70 ~ 0x15022b9C
       {DIP_A_SL2C_BUG_2,       0x22b9c, 0x0b9c, 1  },  //0x15022b9c
       {DIP_A_SRZ1_BUG_1,       0x22bb0, 0x0bb0, 1  },  //0x15022bb0
       {DIP_A_SRZ1,             0x22bb0, 0x0bb0, 9  },  //0x15022bb0 ~ 0x15022bd0
       {DIP_A_SRZ1_BUG_2,       0x22bd0, 0x0bd0, 1  },  //0x15022bd0
       {DIP_A_SRZ2_BUG_1,       0x22be0, 0x0be0, 1  },  //0x15022be0
       {DIP_A_SRZ2,             0x22be0, 0x0be0, 9  },  //0x15022be0 ~ 0x15022c00
       {DIP_A_SRZ2_BUG_2,       0x22c00, 0x0c00, 1  },  //0x15022c00
       {DIP_A_CRZ_BUG_1,        0x22c10, 0x0c10, 1  },  //0x15022c10
       {DIP_A_CRZ,              0x22c10, 0x0c10, 14 },  //0x15022c10 ~ 0x15022c44
       {DIP_A_CRZ_BUG_2,        0x22c44, 0x0c44, 1  },  //0x15022c44
       {DIP_A_MIX1_BUG_1,       0x22c70, 0x0c70, 1  },  //0x15022c70
       {DIP_A_MIX1,             0x22c70, 0x0c70, 2  },  //0x15022c70 ~ 0x15022c74
       {DIP_A_MIX1_BUG_2,       0x22c74, 0x0c74, 1  },  //0x15022c74
       {DIP_A_MIX2_BUG_1,       0x22c80, 0x0c80, 1  },  //0x15022c80
       {DIP_A_MIX2,             0x22c80, 0x0c80, 2  },  //0x15022c80 ~ 0x15022c84
       {DIP_A_MIX2_BUG_2,       0x22c84, 0x0c84, 1  },  //0x15022c84
       {DIP_A_MIX3_BUG_1,       0x22c90, 0x0c90, 1  },  //0x15022c90
       {DIP_A_MIX3,             0x22c90, 0x0c90, 2  },  //0x15022c90 ~ 0x15022c94
       {DIP_A_MIX3_BUG_2,       0x22c94, 0x0c94, 1  },  //0x15022c94
       {DIP_A_SL2D_BUG_1,       0x22ca0, 0x0ca0, 1  },  //0x15022ca0
       {DIP_A_SL2D,             0x22ca0, 0x0ca0, 12 },  //0x15022ca0 ~ 0x15022ccc
       {DIP_A_SL2D_BUG_2,       0x22ccc, 0x0ccc, 1  },  //0x15022ccc
       {DIP_A_SL2E_BUG_1,       0x22ce0, 0x0ce0, 1  },  //0x15022ce0
       {DIP_A_SL2E,             0x22ce0, 0x0ce0, 12 },  //0x15022ce0 ~ 0x15022d0c
       {DIP_A_SL2E_BUG_2,       0x22d0C, 0x0d0C, 1  },  //0x15022d0c
       {DIP_A_MDP_BUG_1,        0x22d20, 0x0d20, 1  },  //0x15022d20
       {DIP_A_MDP,              0x22d20, 0x0d20, 2  },  //0x15022d20 ~ 0x15022d24
       {DIP_A_MDP_BUG_2,        0x22d24, 0x0d24, 1  },  //0x15022d24
       {DIP_A_ANR2_BUG_1,       0x22d30, 0x0d30, 1  },  //0x15022d30
       {DIP_A_ANR2,             0x22d30, 0x0d30, 30 },  //0x15022d30 ~ 0x15022da4
       {DIP_A_ANR2_BUG_2,       0x22da4, 0x0da4, 1  },  //0x15022da4
       {DIP_A_CCR_BUG_1,        0x22db0, 0x0db0, 1  },  //0x15022db0
       {DIP_A_CCR,              0x22db0, 0x0db0, 12 },  //0x15022db0 ~ 0x15022ddc
       {DIP_A_CCR_BUG_2,        0x22ddc, 0x0ddc, 1  },  //0x15022ddc
       {DIP_A_BOK_BUG_1,        0x22de0, 0x0de0, 1  },  //0x15022de0
       {DIP_A_BOK,              0x22de0, 0x0de0, 3  },  //0x15022de0 ~ 0x15022de8
       {DIP_A_BOK_BUG_2,        0x22de8, 0x0de8, 1  },  //0x15022de8
       {DIP_A_ABF_BUG_1,        0x22df0, 0x0df0, 1  },  //0x15022df0
       {DIP_A_ABF,              0x22df0, 0x0df0, 11 },  //0x15022df0 ~ 0x15022e18
       {DIP_A_ABF_BUG_2,        0x22e18, 0x0e18, 1  },  //0x15022e18
       {DIP_A_UNP,              0x22e30, 0x0e30, 1  },  //0x15022e30
       {DIP_A_C02_BUG_1,        0x22e40, 0x0e40, 1  },  //0x15022e40
       {DIP_A_C02,              0x22e40, 0x0e40, 3  },  //0x15022e40 ~ 0x15022e48
       {DIP_A_C02_BUG_2,        0x22e48, 0x0e48, 1  },  //0x15022e48
       {DIP_A_FE_BUG_1,         0x22e50, 0x0e50, 1  },  //0x15022e50
       {DIP_A_FE,               0x22e50, 0x0e50, 5  },  //0x15022e50 ~ 0x15022e60
       {DIP_A_FE_BUG_2,         0x22e60, 0x0e60, 1  },  //0x15022e60
       {DIP_A_CRSP_BUG_1,       0x22e70, 0x0e70, 1  },  //0x15022e70
       {DIP_A_CRSP,             0x22e70, 0x0e70, 6  },  //0x15022e70 ~ 0x15022e84
       {DIP_A_CRSP_BUG_2,       0x22e84, 0x0e84, 1  },  //0x15022e84
       {DIP_A_C02B_BUG_1,       0x22e90, 0x0e90, 1  },  //0x15022e90
       {DIP_A_C02B,             0x22e90, 0x0e90, 3  },  //0x15022e90 ~ 0x15022e98
       {DIP_A_C02B_BUG_2,       0x22e98, 0x0e98, 1  },  //0x15022e98
       {DIP_A_C24,              0x22ea0, 0x0ea0, 1  },  //0x15022ea0
       {DIP_A_C24B,             0x22eb0, 0x0eb0, 1  },  //0x15022eb0
       {DIP_A_LCE_BUG_1,        0x22ec0, 0x0ec0, 1  },  //0x15022ec0
       {DIP_A_LCE,              0x22ec0, 0x0ec0, 15 },  //0x15022ec0 ~ 0x15022ef8
       {DIP_A_LCE_BUG_2,        0x22ef8, 0x0ef8, 1  },  //0x15022ef8
       {DIP_A_FM_BUG_1,         0x22f40, 0x0f40, 1  },  //0x15022f40
       {DIP_A_FM,               0x22f40, 0x0f40, 3  },  //0x15022f40 ~ 0x15022f48
       {DIP_A_FM_BUG_2,         0x22f48, 0x0f48, 1  },  //0x15022f48
       {DIP_A_GGM_BUG_1,        0x23000, 0x1000, 1  },  //0x15023000
       {DIP_A_GGM,              0x23000, 0x1000, 289},  //0x15023000 ~ 0x15023480
       {DIP_A_GGM_BUG_2,        0x23480, 0x1480, 1  },  //0x15023480
       {DIP_A_PCA_BUG_1,        0x23800, 0x1800, 1  },  //0x15023800
       {DIP_A_PCA,              0x23800, 0x1800, 391},  //0x15023800 ~ 0x15023e18
       {DIP_A_PCA_BUG_2,        0x23e18, 0x1e18, 1  },  //0x15023e18
       {DIP_A_OBC2_BUG_1,       0x23e40, 0x1e40, 1  },  //0x15023e40
       {DIP_A_OBC2,             0x23e40, 0x1e40, 8  },  //0x15023e40 ~ 0x15023e5c
       {DIP_A_OBC2_BUG_2,       0x23e5c, 0x1e5c, 1  },  //0x15023e5c
       {DIP_A_BNR2_BUG_1,       0x23e70, 0x1e70, 1  },  //0x15023e70
       {DIP_A_BNR2,             0x23e70, 0x1e70, 32 },  //0x15023e70 ~ 0x15023eec
       {DIP_A_BNR2_BUG_2,       0x23eec, 0x1eec, 1  },  //0x15023eec
       {DIP_A_RMG2_BUG_1,       0x23ef0, 0x1ef0, 1  },  //0x15023ef0
       {DIP_A_RMG2,             0x23ef0, 0x1ef0, 3  },  //0x15023ef0 ~ 0x15023ef8
       {DIP_A_RMG2_BUG_2,       0x23ef8, 0x1ef8, 1  },  //0x15023ef8
       {DIP_A_RMM2_BUG_1,       0x23f10, 0x1f10, 1  },  //0x15023f10
       {DIP_A_RMM2,             0x23f10, 0x1f10, 9  },  //0x15023f10 ~ 0x15023f30
       {DIP_A_RMM2_BUG_2,       0x23f30, 0x1f30, 1  },  //0x15023f30
       {DIP_A_RCP2_BUG_1,       0x23f60, 0x1f60, 1  },  //0x15023f60
       {DIP_A_RCP2,             0x23f60, 0x1f60, 2  },  //0x15023f60 ~ 0x15023f64
       {DIP_A_RCP2_BUG_2,       0x23f64, 0x1f64, 1  },  //0x15023f64
       {DIP_A_LSC2_BUG_1,       0x23f70, 0x1f70, 1  },  //0x15023f70
       {DIP_A_LSC2,             0x23f70, 0x1f70, 9  },  //0x15023f70 ~ 0x15023f90
       {DIP_A_LSC2_BUG_2,       0x23f90, 0x1f90, 1  },  //0x15023f90
       {DIP_A_DBS2_BUG_1,       0x23fa0, 0x1fa0, 1  },  //0x15023fa0
       {DIP_A_DBS2,             0x23fa0, 0x1fa0, 13 },  //0x15023fa0 ~ 0x15023fd0
       {DIP_A_DBS2_BUG_2,       0x23fd0, 0x1fd0, 1  },  //0x15023fd0
       {DIP_A_ANR_BUG_1,        0x24000, 0x2000, 1  },  //0x15024000
       {DIP_A_ANR,              0x24000, 0x2000, 301},  //0x15024000 ~ 0x150244b0
       {DIP_A_ANR_BUG_2,        0x244b0, 0x24b0, 1  },  //0x150244b0
       {DIP_A_SL2G_BUG_1,       0x24500, 0x2500, 1  },  //0x15024500
       {DIP_A_SL2G,             0x24500, 0x2500, 12 },  //0x15024500 ~ 0x1502452C
       {DIP_A_SL2G_BUG_2,       0x24524, 0x252C, 1  },  //0x1502452C
       {DIP_A_SL2H_BUG_1,       0x24540, 0x2540, 1  },  //0x15024540
       {DIP_A_SL2H,             0x24540, 0x2540, 12 },  //0x15024540 ~ 0x1502456C
       {DIP_A_SL2H_BUG_2,       0x24564, 0x256C, 1  },  //0x1502456C
       {DIP_A_HLR2_BUG_1,       0x24580, 0x2580, 1  },  //0x15024580
       {DIP_A_HLR2,             0x24580, 0x2580, 1  },  //0x15024580 ~ 0x15024580
       {DIP_A_HLR2_BUG_2,       0x24580, 0x2580, 1  },  //0x15024580
       {DIP_A_HLR2_1_BUG_1,     0x24588, 0x2588, 1  },  //0x15024588
       {DIP_A_HLR2_1,           0x24588, 0x2588, 10 },  //0x15024588 ~ 0x150245Ac
       {DIP_A_HLR2_1_BUG_2,     0x245ac, 0x25ac, 1  },  //0x150245Ac
       {DIP_A_RNR_BUG_1,        0x245b0, 0x25b0, 1  },  //0x150245b0
       {DIP_A_RNR,              0x245b0, 0x25b0, 16 },  //0x150245b0 ~ 0x150245ec
       {DIP_A_RNR_BUG_2,        0x245ec, 0x25ec, 1  },  //0x150245ec
       {DIP_A_HFG_BUG_1,        0x24630, 0x2630, 1  },  //0x15024630
       {DIP_A_HFG,              0x24630, 0x2630, 14 },  //0x15024630 ~ 0x15024664
       {DIP_A_HFG_BUG_2,        0x24664, 0x2664, 1  },  //0x15024664
       {DIP_A_SL2I_BUG_1,       0x24670, 0x2670, 1  },  //0x15024670
       {DIP_A_SL2I,             0x24670, 0x2670, 12 },  //0x15024670 ~ 0x1502469C
       {DIP_A_SL2I_BUG_2,       0x24694, 0x269C, 1  },  //0x1502469C
       {DIP_A_SL2K_BUG_1,       0x246B0, 0x26B0, 1  },  //0x150246B0
       {DIP_A_SL2K,             0x246B0, 0x26B0, 12 },  //0x150246B0 ~ 0x150246DC
       {DIP_A_SL2K_BUG_2,       0x246DC, 0x26DC, 1  },  //0x150246DC
       {DIP_A_COLOR_BUG_1,      0x246F0, 0x26F0, 1  },  //0x150246F0
       {DIP_A_COLOR,            0x246F0, 0x26F0, 155},  //0x150246F0 ~ 0x15024958
       {DIP_A_COLOR_BUG_2,      0x24958, 0x2958, 1  },  //0x15024958
       {DIP_A_NDG_BUG_1,        0x24960, 0x2960, 1  },  //0x15024960
       {DIP_A_NDG,              0x24960, 0x2960, 6  },  //0x15024960~0x15024974
       {DIP_A_NDG_BUG_2,        0x24974, 0x2974, 1  },  //0x15024974
       {DIP_A_END_,             0x00000, 0x0000, 1}
   };
#else
static ISP_DRV_CQ_MODULE_INFO_STRUCT mIspDipCQModuleInfo[DIP_A_MODULE_MAX]
   =  {{DIP_A_CTL,              0x22004, 0x0004, 7  },  //0x15022004 ~ 0x1502201c
       {DIP_A_CTL_TDR,          0x22050, 0x0050, 4  },  //0x15022050 ~ 0x1502205c
       {DIP_A_CTL_DONE,         0x2207c, 0x007c, 1  },  //0x1502207c
       {DIP_A_TDRI,             0x22204, 0x0204, 3  },  //0x15022204 ~ 0x1502220c
       {DIP_A_VECTICAL_FLIP,    0x22210, 0x0210, 1  },  //0x15022210
       {DIP_A_SPECIAL_FUN,      0x2221c, 0x021c, 1  },  //0x1502221c
       {DIP_A_IMG2O,            0x22230, 0x0230, 7  },  //0x15022230 ~ 0x15022248
       {DIP_A_IMG2O_CRSP,       0x22258, 0x0258, 1  },  //0x15022258
       {DIP_A_IMG2BO,           0x22260, 0x0260, 7  },  //0x15022260 ~ 0x15022278
       {DIP_A_IMG2BO_CRSP,      0x22288, 0x0288, 1  },  //0x15022288
       {DIP_A_IMG3O,            0x22290, 0x0290, 7  },  //0x15022290 ~ 0x150222a8
       {DIP_A_IMG3O_CRSP,       0x222b8, 0x02b8, 1  },  //0x150222b8
       {DIP_A_IMG3BO,           0x222c0, 0x02c0, 7  },  //0x150222c0 ~ 0x150222d8
       {DIP_A_IMG3BO_CRSP,      0x222e8, 0x02e8, 1  },  //0x150222e8
       {DIP_A_IMG3CO,           0x222f0, 0x02f0, 7  },  //0x150222f0 ~ 0x15022308
       {DIP_A_IMG3CO_CRSP,      0x22318, 0x0318, 1  },  //0x15022318
       {DIP_A_FEO,              0x22320, 0x0320, 7  },  //0x15022320 ~ 0x15022338
       {DIP_A_MFBO,             0x22350, 0x0350, 7  },  //0x15022350 ~ 0x15022368
       {DIP_A_MFBO_CROP,        0x22378, 0x0378, 1  },  //0x15022378
       {DIP_A_IMGI,             0x22400, 0x0400, 7  },  //0x15022400 ~ 0x15022418     
       {DIP_A_IMGBI,            0x22430, 0x0430, 7  },  //0x15022430 ~ 0x15022448
       {DIP_A_IMGCI,            0x22460, 0x0460, 7  },  //0x15022460 ~ 0x15022478
       {DIP_A_VIPI,             0x22490, 0x0490, 7  },  //0x15022490 ~ 0x150224a8
       {DIP_A_VIP2I,            0x224c0, 0x04c0, 7  },  //0x150224c0 ~ 0x150224d8
       {DIP_A_VIP3I,            0x224f0, 0x04f0, 7  },  //0x150224f0 ~ 0x15022508
       {DIP_A_DMGI,             0x22520, 0x0520, 7  },  //0x15022520 ~ 0x15022538
       {DIP_A_DEPI,             0x22550, 0x0550, 7  },  //0x15022550 ~ 0x15022568
       {DIP_A_LCEI,             0x22580, 0x0580, 7  },  //0x15022580 ~ 0x15022598
       {DIP_A_UFDI,             0x225b0, 0x05b0, 7  },  //0x150225b0 ~ 0x150225c8
       {DIP_A_UFD,              0x22700, 0x0700, 5  },  //0x15022700 ~ 0x15022710
       {DIP_A_PGN,              0x22720, 0x0720, 6  },  //0x15022720 ~ 0x15022734
       {DIP_A_SL2,              0x22740, 0x0740, 12 },  //0x15022740 ~ 0x1502276c
       {DIP_A_UDM,              0x22780, 0x0780, 19 },  //0x15022780 ~ 0x150227c8
       {DIP_A_G2G,              0x227f0, 0x07f0, 8  },  //0x150227f0 ~ 0x1502280c
       {DIP_A_MFB,              0x22f60, 0x0f60, 23 },  //0x15022f60 ~ 0x15022fb8
       {DIP_A_FLC,              0x22fc0, 0x0fc0, 7  },  //0x15022FC0 ~ 0x15022FD8
       {DIP_A_FLC2,             0x22fe0, 0x0fe0, 7  },  //0x15022FE0 ~ 0x15022FF8
       {DIP_A_G2C,              0x22840, 0x0840, 13 },  //0x15022840 ~ 0x15022870
       {DIP_A_C42,              0x22880, 0x0880, 1  },  //0x15022880
       {DIP_A_SRZ3,             0x22890, 0x0890, 9  },  //0x15022890 ~ 0x150228b0
       {DIP_A_SRZ4,             0x228c0, 0x08c0, 9  },  //0x150228c0 ~ 0x150228e0
       {DIP_A_SRZ5,             0x228f0, 0x08f0, 9  },  //0x150228F0~0x15022910
       {DIP_A_SEEE,             0x22990, 0x0990, 27 },  //0x15022990 ~ 0x150229f8
       {DIP_A_NDG2,             0x22a10, 0x0a10, 6  },  //0x15022a10~0x15022a24
       {DIP_A_NR3D,             0x22a30, 0x0a30, 39 },  //0x15022a30 ~ 0x15022ac8
       {DIP_A_SL2B,             0x22b30, 0x0b30, 12 },  //0x15022b30 ~ 0x15022b5c
       {DIP_A_SL2C,             0x22b70, 0x0b70, 12 },  //0x15022b70 ~ 0x15022b9C
       {DIP_A_SRZ1,             0x22bb0, 0x0bb0, 9  },  //0x15022bb0 ~ 0x15022bd0
       {DIP_A_SRZ2,             0x22be0, 0x0be0, 9  },  //0x15022be0 ~ 0x15022c00
       {DIP_A_CRZ,              0x22c10, 0x0c10, 14 },  //0x15022c10 ~ 0x15022c44
       {DIP_A_MIX1,             0x22c70, 0x0c70, 2  },  //0x15022c70 ~ 0x15022c74
       {DIP_A_MIX2,             0x22c80, 0x0c80, 2  },  //0x15022c80 ~ 0x15022c84
       {DIP_A_MIX3,             0x22c90, 0x0c90, 2  },  //0x15022c90 ~ 0x15022c94
       {DIP_A_SL2D,             0x22ca0, 0x0ca0, 12 },  //0x15022ca0 ~ 0x15022ccc
       {DIP_A_SL2E,             0x22ce0, 0x0ce0, 12 },  //0x15022ce0 ~ 0x15022d0c
       {DIP_A_MDP,              0x22d20, 0x0d20, 2  },  //0x15022d20 ~ 0x15022d24
       {DIP_A_ANR2,             0x22d30, 0x0d30, 30 },  //0x15022d30 ~ 0x15022da4
       {DIP_A_CCR,              0x22db0, 0x0db0, 12 },  //0x15022db0 ~ 0x15022ddc
       {DIP_A_BOK,              0x22de0, 0x0de0, 3  },  //0x15022de0 ~ 0x15022de8
       {DIP_A_ABF,              0x22df0, 0x0df0, 11 },  //0x15022df0 ~ 0x15022e18
       {DIP_A_UNP,              0x22e30, 0x0e30, 1  },  //0x15022e30
       {DIP_A_C02,              0x22e40, 0x0e40, 3  },  //0x15022e40 ~ 0x15022e48
       {DIP_A_FE,               0x22e50, 0x0e50, 5  },  //0x15022e50 ~ 0x15022e60
       {DIP_A_CRSP,             0x22e70, 0x0e70, 6  },  //0x15022e70 ~ 0x15022e84
       {DIP_A_C02B,             0x22e90, 0x0e90, 3  },  //0x15022e90 ~ 0x15022e98
       {DIP_A_C24,              0x22ea0, 0x0ea0, 1  },  //0x15022ea0
       {DIP_A_C24B,             0x22eb0, 0x0eb0, 1  },  //0x15022eb0
       {DIP_A_LCE,              0x22ec0, 0x0ec0, 15 },  //0x15022ec0 ~ 0x15022ef8
       {DIP_A_FM,               0x22f40, 0x0f40, 3  },  //0x15022f40 ~ 0x15022f48
       {DIP_A_GGM,              0x23000, 0x1000, 289},  //0x15023000 ~ 0x15023480
       {DIP_A_PCA,              0x23800, 0x1800, 391},  //0x15023800 ~ 0x15023e18
       {DIP_A_OBC2,             0x23e40, 0x1e40, 8  },  //0x15023e40 ~ 0x15023e5c
       {DIP_A_BNR2,             0x23e70, 0x1e70, 32 },  //0x15023e70 ~ 0x15023eec
       {DIP_A_RMG2,             0x23ef0, 0x1ef0, 3  },  //0x15023ef0 ~ 0x15023ef8
       {DIP_A_RMM2,             0x23f10, 0x1f10, 9  },  //0x15023f10 ~ 0x15023f30
       {DIP_A_RCP2,             0x23f60, 0x1f60, 2  },  //0x15023f60 ~ 0x15023f64
       {DIP_A_LSC2,             0x23f70, 0x1f70, 9  },  //0x15023f70 ~ 0x15023f90
       {DIP_A_DBS2,             0x23fa0, 0x1fa0, 13 },  //0x15023fa0 ~ 0x15023fd0
       {DIP_A_ANR,              0x24000, 0x2000, 301},  //0x15024000 ~ 0x150244b0
       {DIP_A_SL2G,             0x24500, 0x2500, 12 },  //0x15024500 ~ 0x1502452C
       {DIP_A_SL2H,             0x24540, 0x2540, 12 },  //0x15024540 ~ 0x1502456C
       {DIP_A_HLR2,             0x24580, 0x2580, 1  },  //0x15024580 ~ 0x15024580
       {DIP_A_HLR2_1,           0x24588, 0x2588, 10 },  //0x15024588 ~ 0x150245Ac
       {DIP_A_RNR,              0x245b0, 0x25b0, 16 },  //0x150245b0 ~ 0x150245ec
       {DIP_A_HFG,              0x24630, 0x2630, 14 },  //0x15024630 ~ 0x15024664
       {DIP_A_SL2I,             0x24670, 0x2670, 12 },  //0x15024670 ~ 0x1502469C
       {DIP_A_SL2K,             0x246B0, 0x26B0, 12 },  //0x150246B0 ~ 0x150246DC
       {DIP_A_COLOR,            0x246F0, 0x26F0, 155},  //0x150246F0 ~ 0x15024958
       {DIP_A_NDG,              0x24960, 0x2960, 6  },  //0x15024960~0x15024974
       {DIP_A_END_,             0x00000, 0x0000, 1}
   };
#endif


///////////////////////////////////////////////////////////////////////////

//DIP_X_CTL_YUV_EN, reg:0x15022004
#define DIP_X_REG_CTL_YUV_EN_MFB                   (1L<<0)
#define DIP_X_REG_CTL_YUV_EN_C02B                  (1L<<1)
#define DIP_X_REG_CTL_YUV_EN_C24                   (1L<<2)
#define DIP_X_REG_CTL_YUV_EN_G2C                   (1L<<3)
#define DIP_X_REG_CTL_YUV_EN_C42                   (1L<<4)
#define DIP_X_REG_CTL_YUV_EN_NBC                   (1L<<5)
#define DIP_X_REG_CTL_YUV_EN_NBC2                  (1L<<6)
#define DIP_X_REG_CTL_YUV_EN_PCA                   (1L<<7)
#define DIP_X_REG_CTL_YUV_EN_SEEE                  (1L<<8)
#define DIP_X_REG_CTL_YUV_EN_rsv_9                 (1L<<9)
#define DIP_X_REG_CTL_YUV_EN_NR3D                  (1L<<10)
#define DIP_X_REG_CTL_YUV_EN_SL2B                  (1L<<11)
#define DIP_X_REG_CTL_YUV_EN_SL2C                  (1L<<12)
#define DIP_X_REG_CTL_YUV_EN_SL2D                  (1L<<13)
#define DIP_X_REG_CTL_YUV_EN_SL2E                  (1L<<14)
#define DIP_X_REG_CTL_YUV_EN_SRZ1                  (1L<<15)
#define DIP_X_REG_CTL_YUV_EN_SRZ2                  (1L<<16)
#define DIP_X_REG_CTL_YUV_EN_CRZ                   (1L<<17)
#define DIP_X_REG_CTL_YUV_EN_MIX1                  (1L<<18)
#define DIP_X_REG_CTL_YUV_EN_MIX2                  (1L<<19)
#define DIP_X_REG_CTL_YUV_EN_MIX3                  (1L<<20)
#define DIP_X_REG_CTL_YUV_EN_CRSP                  (1L<<21)
#define DIP_X_REG_CTL_YUV_EN_C24B                  (1L<<22)
#define DIP_X_REG_CTL_YUV_EN_MDPCROP               (1L<<23)
#define DIP_X_REG_CTL_YUV_EN_C02                   (1L<<24)
#define DIP_X_REG_CTL_YUV_EN_FE                    (1L<<25)
#define DIP_X_REG_CTL_YUV_EN_MFBW                  (1L<<26)
#define DIP_X_REG_CTL_YUV_EN_rsv_27                (1L<<27)
#define DIP_X_REG_CTL_YUV_EN_PLNW1                 (1L<<28)
#define DIP_X_REG_CTL_YUV_EN_PLNR1                 (1L<<29)
#define DIP_X_REG_CTL_YUV_EN_PLNW2                 (1L<<30)
#define DIP_X_REG_CTL_YUV_EN_PLNR2                 (1L<<31)

//DIP_X_CTL_YUV2_EN, reg:0x15022008
#define DIP_X_REG_CTL_YUV2_EN_FM                   (1L<<0)
#define DIP_X_REG_CTL_YUV2_EN_SRZ3                 (1L<<1)
#define DIP_X_REG_CTL_YUV2_EN_SRZ4                 (1L<<2)
#define DIP_X_REG_CTL_YUV2_EN_SRZ5                 (1L<<3)
#define DIP_X_REG_CTL_YUV2_EN_HFG                  (1L<<4)
#define DIP_X_REG_CTL_YUV2_EN_SL2I                 (1L<<5)
#define DIP_X_REG_CTL_YUV2_EN_NDG                  (1L<<6)
#define DIP_X_REG_CTL_YUV2_EN_NDG2                 (1L<<7)
#define DIP_X_REG_CTL_YUV2_EN_SET_ISO              (1L<<31)  /* NOTICE: this bit is not real HW register */


//DIP_X_CTL_RGB_EN, reg:0x1502200C
#define DIP_X_REG_CTL_RGB_EN_UNP                   (1L<<0)
#define DIP_X_REG_CTL_RGB_EN_UFD                   (1L<<1)
#define DIP_X_REG_CTL_RGB_EN_PGN                   (1L<<2)
#define DIP_X_REG_CTL_RGB_EN_SL2                   (1L<<3)
#define DIP_X_REG_CTL_RGB_EN_UDM                   (1L<<4)
#define DIP_X_REG_CTL_RGB_EN_LCE                   (1L<<5)
#define DIP_X_REG_CTL_RGB_EN_G2G                   (1L<<6)
#define DIP_X_REG_CTL_RGB_EN_GGM                   (1L<<7)
#define DIP_X_REG_CTL_RGB_EN_GDR1                  (1L<<8)
#define DIP_X_REG_CTL_RGB_EN_GDR2                  (1L<<9)
#define DIP_X_REG_CTL_RGB_EN_DBS2                  (1L<<10)
#define DIP_X_REG_CTL_RGB_EN_OBC2                  (1L<<11)
#define DIP_X_REG_CTL_RGB_EN_RMG2                  (1L<<12)
#define DIP_X_REG_CTL_RGB_EN_BNR2                  (1L<<13)
#define DIP_X_REG_CTL_RGB_EN_RMM2                  (1L<<14)
#define DIP_X_REG_CTL_RGB_EN_LSC2                  (1L<<15)
#define DIP_X_REG_CTL_RGB_EN_RCP2                  (1L<<16)
#define DIP_X_REG_CTL_RGB_EN_PAK2                  (1L<<17)
#define DIP_X_REG_CTL_RGB_EN_RNR                   (1L<<18)
#define DIP_X_REG_CTL_RGB_EN_SL2G                  (1L<<19)
#define DIP_X_REG_CTL_RGB_EN_SL2H                  (1L<<20)
#define DIP_X_REG_CTL_RGB_EN_HLR2                  (1L<<21)
#define DIP_X_REG_CTL_RGB_EN_SL2K                  (1L<<22)
#define DIP_X_REG_CTL_RGB_EN_FLC                   (1L<<23)
#define DIP_X_REG_CTL_RGB_EN_FLC2                  (1L<<24)

//DIP_X_CTL_DMA_EN, reg:0x15022010
#define DIP_X_REG_CTL_DMA_EN_NONE                  (0)
#define DIP_X_REG_CTL_DMA_EN_IMGI                  (1L<<0)
#define DIP_X_REG_CTL_DMA_EN_IMGBI                 (1L<<1)
#define DIP_X_REG_CTL_DMA_EN_IMGCI                 (1L<<2)
#define DIP_X_REG_CTL_DMA_EN_UFDI                  (1L<<3)
#define DIP_X_REG_CTL_DMA_EN_VIPI                  (1L<<4)
#define DIP_X_REG_CTL_DMA_EN_VIP2I                 (1L<<5)
#define DIP_X_REG_CTL_DMA_EN_VIP3I                 (1L<<6)
#define DIP_X_REG_CTL_DMA_EN_LCEI                  (1L<<7)
#define DIP_X_REG_CTL_DMA_EN_DEPI                  (1L<<8)
#define DIP_X_REG_CTL_DMA_EN_DMGI                  (1L<<9)
#define DIP_X_REG_CTL_DMA_EN_MFBO                  (1L<<10)
#define DIP_X_REG_CTL_DMA_EN_IMG2O                 (1L<<11)
#define DIP_X_REG_CTL_DMA_EN_IMG2BO                (1L<<12)
#define DIP_X_REG_CTL_DMA_EN_IMG3O                 (1L<<13)
#define DIP_X_REG_CTL_DMA_EN_IMG3BO                (1L<<14)
#define DIP_X_REG_CTL_DMA_EN_IMG3CO                (1L<<15)
#define DIP_X_REG_CTL_DMA_EN_FEO                   (1L<<16)
#define DIP_X_REG_CTL_DMA_EN_ADL2                  (1L<<17)
#define DIP_X_REG_CTL_DMA_EN_WROTO                 (1L<<20)
#define DIP_X_REG_CTL_DMA_EN_WDMAO                 (1L<<21)
#define DIP_X_REG_CTL_DMA_EN_JPEGO                 (1L<<22)
#define DIP_X_REG_CTL_DMA_EN_VENCO                 (1L<<23)

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
#define DIP_X_REG_CTL_TDR_TCM_EN_rsv_0             (1L<<0)
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
#define DIP_X_REG_CTL_TDR_MFBO_TCM_EN              (1L<<17)
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
#define DIP_X_REG_CTL_TDR_rsv_28                   (1L<<28)
#define DIP_X_REG_CTL_TDR_SRZ1_TCM_EN              (1L<<29)
#define DIP_X_REG_CTL_TDR_SRZ2_TCM_EN              (1L<<30)
#define DIP_X_REG_CTL_TDR_LCE_TCM_EN               (1L<<31)


//DIP_X_REG_CTL_TDR_TCM2_EN,  0x1502205C
#define DIP_X_REG_CTL_TDR_CRZ_TCM_EN               (1L<<0)
#define DIP_X_REG_CTL_TDR_FE_TCM_EN                (1L<<1)
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
#define DIP_X_REG_CTL_TDR_HLR2_TCM_EN              (1L<<28)
#define DIP_X_REG_CTL_TDR_SL2K_TCM_EN              (1L<<29)
#define DIP_X_REG_CTL_TDR_ADL2_TCM_EN              (1L<<30)
#define DIP_X_REG_CTL_TDR_FLC_TCM_EN               (1L<<31)

///////////////////////////////////////////////////////////
/**
dip initial setting
*/
#define ISP_DIP_INIT_SETTING_COUNT  90//82
static ISP_DRV_REG_IO_STRUCT mIspDipInitReg[ISP_DIP_INIT_SETTING_COUNT]
    =  {{DIP_A, 0x0024, 0xffffffff},   //DIP_X_REG_CTL_CQ_INT_EN
        {DIP_A, 0x0104, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR0_CTL
        {DIP_A, 0x0110, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR1_CTL
        {DIP_A, 0x011C, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR2_CTL
        {DIP_A, 0x0128, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR3_CTL
        {DIP_A, 0x0134, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR4_CTL
        {DIP_A, 0x0140, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR5_CTL
        {DIP_A, 0x014C, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR6_CTL
        {DIP_A, 0x0158, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR7_CTL
        {DIP_A, 0x0164, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR8_CTL
        {DIP_A, 0x0170, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR9_CTL
        {DIP_A, 0x017C, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR10_CTL
        {DIP_A, 0x0188, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR11_CTL
        {DIP_A, 0x0194, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR12_CTL
        {DIP_A, 0x01A0, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR13_CTL
        {DIP_A, 0x01AC, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR14_CTL
        {DIP_A, 0x01B8, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR15_CTL
        {DIP_A, 0x01C4, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},	//DIP_X_CQ_THR16_CTL
        {DIP_A, 0x01D0, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR17_CTL
        {DIP_A, 0x01DC, (DIP_X_CQ_THRX_CTL_EN|DIP_X_CQ_THRX_CTL_THRX_MODE)},   //DIP_X_CQ_THR18_CTL
        {DIP_A, 0x010C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR0_BASEADDR
        {DIP_A, 0x0118, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR1_BASEADDR
        {DIP_A, 0x0124, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR2_BASEADDR
        {DIP_A, 0x0130, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR3_BASEADDR
        {DIP_A, 0x013C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR4_BASEADDR
        {DIP_A, 0x0148, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR5_BASEADDR
        {DIP_A, 0x0154, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR6_BASEADDR
        {DIP_A, 0x0160, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR7_BASEADDR
        {DIP_A, 0x016C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR8_BASEADDR
        {DIP_A, 0x0178, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR9_BASEADDR
        {DIP_A, 0x0184, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR10_BASEADDR
        {DIP_A, 0x0190, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR11_BASEADDR
        {DIP_A, 0x019C, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR12_BASEADDR
        {DIP_A, 0x01A8, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR13_BASEADDR
        {DIP_A, 0x01B4, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR14_BASEADDR
        {DIP_A, 0x01C0, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR15_BASEADDR
        {DIP_A, 0x01CC, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR16_BASEADDR
        {DIP_A, 0x01D8, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR17_BASEADDR
        {DIP_A, 0x01E4, (DIP_A_END_+1)*sizeof(ISP_DRV_CQ_CMD_DESC_STRUCT)},    //DIP_X_REG_CQ_THR18_BASEADDR
        {DIP_A, 0x41C, 0x80000080},    //DIP_X_IMGI_CON
        {DIP_A, 0x420, 0x00800080},    //DIP_X_IMGI_CON2
        {DIP_A, 0x424, 0x00300030},    //DIP_X_IMGI_CON3
        {DIP_A, 0x44C, 0x80000040},    //DIP_X_IMGBI_CON
        {DIP_A, 0x450, 0x00400040},    //DIP_X_IMGBI_CON2
        {DIP_A, 0x454, 0x00150015},    //DIP_X_IMGBI_CON3
        {DIP_A, 0x47C, 0x80000040},    //DIP_X_IMGCI_CON
        {DIP_A, 0x480, 0x00400040},    //DIP_X_IMGCI_CON2
        {DIP_A, 0x484, 0x00150015},    //DIP_X_IMGCI_CON3
        {DIP_A, 0x5CC, 0x80000020},    //DIP_X_UFDI_CON
        {DIP_A, 0x5D0, 0x00200020},    //DIP_X_UFDI_CON2
        {DIP_A, 0x5D4, 0x000A000A},    //DIP_X_UFDI_CON3
        {DIP_A, 0x59C, 0x80000020},    //DIP_X_LCEI_CON
        {DIP_A, 0x5A0, 0x00200020},    //DIP_X_LCEI_CON2
        {DIP_A, 0x5A4, 0x000A000A},    //DIP_X_LCEI_CON3
        {DIP_A, 0x4AC, 0x80000080},    //DIP_X_VIPI_CON
        {DIP_A, 0x4B0, 0x00800080},    //DIP_X_VIPI_CON2
        {DIP_A, 0x4B4, 0x00300030},    //DIP_X_VIPI_CON3
        {DIP_A, 0x4DC, 0x80000040},    //DIP_X_VIP2I_CON
        {DIP_A, 0x4E0, 0x00400040},    //DIP_X_VIP2I_CON2
        {DIP_A, 0x4E4, 0x00140014},    //DIP_X_VIP2I_CON3
        {DIP_A, 0x50C, 0x80000040},    //DIP_X_VIP3I_CON
        {DIP_A, 0x510, 0x00400040},    //DIP_X_VIP3I_CON2
        {DIP_A, 0x514, 0x00140014},    //DIP_X_VIP3I_CON3
        {DIP_A, 0x53C, 0x80000020},    //DIP_X_DMGI_CON
        {DIP_A, 0x540, 0x00200020},    //DIP_X_DMGI_CON2
        {DIP_A, 0x544, 0x000A000A},    //DIP_X_DMGI_CON3
        {DIP_A, 0x56C, 0x80000020},    //DIP_X_DEPI_CON
        {DIP_A, 0x570, 0x00200020},    //DIP_X_DEPI_CON2
        {DIP_A, 0x574, 0x000A000A},    //DIP_X_DEPI_CON3
        {DIP_A, 0x36C, 0x80000040},    //DIP_X_MFBO_CON
        {DIP_A, 0x370, 0x00400040},    //DIP_X_MFBO_CON2
        {DIP_A, 0x374, 0x00150015},    //DIP_X_MFBO_CON3
        {DIP_A, 0x2AC, 0x80000040},    //DIP_X_IMG3O_CON
        {DIP_A, 0x2B0, 0x00400040},    //DIP_X_IMG3O_CON2
        {DIP_A, 0x2B4, 0x00140014},    //DIP_X_IMG3O_CON3
        {DIP_A, 0x2DC, 0x80000020},    //DIP_X_IMG3BO_CON
        {DIP_A, 0x2E0, 0x00200020},    //DIP_X_IMG3BO_CON2
        {DIP_A, 0x2E4, 0x000A000A},    //DIP_X_IMG3BO_CON3
        {DIP_A, 0x30C, 0x80000020},    //DIP_X_IMG3CO_CON
        {DIP_A, 0x310, 0x00200020},    //DIP_X_IMG3CO_CON2
        {DIP_A, 0x314, 0x000A000A},    //DIP_X_IMG3CO_CON3
        {DIP_A, 0x33C, 0x80000040},    //DIP_X_FEO_CON
        {DIP_A, 0x340, 0x00400040},    //DIP_X_FEO_CON2
        {DIP_A, 0x344, 0x00150015},    //DIP_X_FEO_CON3
        {DIP_A, 0x24C, 0x80000040},    //DIP_X_IMG2O_CON
        {DIP_A, 0x250, 0x00400040},    //DIP_X_IMG2O_CON2
        {DIP_A, 0x254, 0x00140014},    //DIP_X_IMG2O_CON3
        {DIP_A, 0x27C, 0x80000040},    //DIP_X_IMG2BO_CON
        {DIP_A, 0x280, 0x00400040},    //DIP_X_IMG2bO_CON2
        {DIP_A, 0x284, 0x00150015}     //DIP_X_IMG2bO_CON3
       };


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


/**
DIP hw module
*/
typedef enum{
    ISP_DIP_MODULE_IDX_DIP_A = 0,
    //ISP_DIP_MODULE_IDX_DIP_B ,  //not supported in everest
    ISP_DIP_MODULE_IDX_MAX
}ISP_DIP_MODULE_IDX;


#endif //_ISP_DRV_DIP_PLATFORM_H_

