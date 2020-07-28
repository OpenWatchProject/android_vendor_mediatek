/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#define LOG_TAG "isp_debug"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <mtkcam/utils/std/Log.h>
#include <camera_custom_nvram.h>
#include <awb_feature.h>
#include <awb_param.h>
#include <ae_feature.h>
#include <ae_param.h>
#include "isp_mgr.h"
#include <drv/tuning_mgr.h>
#include <isp_tuning/isp_tuning_mgr.h>

#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/INormalPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#endif

using namespace NSIspTuningv3;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

/******************************************************************************
 *
 ******************************************************************************/
static INormalPipeModule* getNormalPipeModule()
{
    static auto pModule = INormalPipeModule::get();
    CAM_LOGE_IF(!pModule, "INormalPipeModule::get() fail");
    return pModule;
}


static MVOID* createDefaultNormalPipe(MUINT32 sensorIndex, char const* szCallerName)
{
    auto pModule = getNormalPipeModule();
    if  ( ! pModule ) {
        CAM_LOGE("getNormalPipeModule() fail");
        return NULL;
    }

    //  Select CamIO version
    size_t count = 0;
    MUINT32 const* version = NULL;
    int err = pModule->get_sub_module_api_version(&version, &count, sensorIndex);
    if  ( err < 0 || ! count || ! version ) {
        CAM_LOGE(
            "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
            sensorIndex, err, count, version
        );
        return NULL;
    }

    MUINT32 const selected_version = *(version + count - 1); //Select max. version
    CAM_LOGD("[%d] count:%zu Selected CamIO Version:%0#x", sensorIndex, count, selected_version);

    MVOID* pPipe = NULL;
    pModule->createSubModule(sensorIndex, szCallerName, selected_version, (MVOID**)&pPipe);
    return pPipe;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define _A_P1(reg)\
    ((MUINT32)offsetof(cam_reg_t, reg))

struct DUMP_INFO
{
    DUMP_INFO()
    {
    #define __I__(param) rRegs[E_DUMP_##param].Addr = _A_P1(param)

        __I__(CAM_CTL_EN         );
        __I__(CAM_CTL_DMA_EN     );
        __I__(CAM_SL2F_CEN       );
        __I__(CAM_SL2F_RR_CON0   );
        __I__(CAM_SL2F_RR_CON1   );
        __I__(CAM_SL2F_GAIN      );
        __I__(CAM_SL2F_RZ        );
        __I__(CAM_SL2F_XOFF      );
        __I__(CAM_SL2F_YOFF      );
        __I__(CAM_SL2F_SLP_CON0  );
        __I__(CAM_SL2F_SLP_CON1  );
        __I__(CAM_SL2F_SIZE      );
        __I__(CAM_DBS_SIGMA      );
        __I__(CAM_DBS_BSTBL_0    );
        __I__(CAM_DBS_BSTBL_1    );
        __I__(CAM_DBS_BSTBL_2    );
        __I__(CAM_DBS_BSTBL_3    );
        __I__(CAM_DBS_CTL        );
        __I__(CAM_DBS_SIGMA_2    );
        __I__(CAM_DBS_YGN        );
        __I__(CAM_DBS_SL_Y12     );
        __I__(CAM_DBS_SL_Y34     );
        __I__(CAM_DBS_SL_G12     );
        __I__(CAM_DBS_SL_G34     );
        __I__(CAM_OBC_OFFST0     );
        __I__(CAM_OBC_OFFST1     );
        __I__(CAM_OBC_OFFST2     );
        __I__(CAM_OBC_OFFST3     );
        __I__(CAM_OBC_GAIN0      );
        __I__(CAM_OBC_GAIN1      );
        __I__(CAM_OBC_GAIN2      );
        __I__(CAM_OBC_GAIN3      );
        __I__(CAM_BNR_BPC_CON    );
        __I__(CAM_BNR_BPC_TH1    );
        __I__(CAM_BNR_BPC_TH2    );
        __I__(CAM_BNR_BPC_TH3    );
        __I__(CAM_BNR_BPC_TH4    );
        __I__(CAM_BNR_BPC_DTC    );
        __I__(CAM_BNR_BPC_COR    );
        __I__(CAM_BNR_BPC_TBLI1  );
        __I__(CAM_BNR_BPC_TBLI2  );
        __I__(CAM_BNR_BPC_TH1_C  );
        __I__(CAM_BNR_BPC_TH2_C  );
        __I__(CAM_BNR_BPC_TH3_C  );
        __I__(CAM_BNR_NR1_CON    );
        __I__(CAM_BNR_NR1_CT_CON );
        __I__(CAM_BNR_PDC_CON    );
        __I__(CAM_BNR_PDC_GAIN_L0);
        __I__(CAM_BNR_PDC_GAIN_L1);
        __I__(CAM_BNR_PDC_GAIN_L2);
        __I__(CAM_BNR_PDC_GAIN_L3);
        __I__(CAM_BNR_PDC_GAIN_L4);
        __I__(CAM_BNR_PDC_GAIN_R0);
        __I__(CAM_BNR_PDC_GAIN_R1);
        __I__(CAM_BNR_PDC_GAIN_R2);
        __I__(CAM_BNR_PDC_GAIN_R3);
        __I__(CAM_BNR_PDC_GAIN_R4);
        __I__(CAM_BNR_PDC_TH_GB  );
        __I__(CAM_BNR_PDC_TH_IA  );
        __I__(CAM_BNR_PDC_TH_HD  );
        __I__(CAM_BNR_PDC_SL     );
        __I__(CAM_BNR_PDC_POS    );
        __I__(CAM_RMM_OSC        );
        __I__(CAM_RMM_MC         );
        __I__(CAM_RMM_REVG_1     );
        __I__(CAM_RMM_REVG_2     );
        __I__(CAM_RMM_LEOS       );
        __I__(CAM_RMM_MC2        );
        __I__(CAM_RMM_DIFF_LB    );
        __I__(CAM_RMM_MA         );
        __I__(CAM_RMM_TUNE       );
        //__I__(CAM_RMM_IDX        );
        __I__(CAM_LSC_CTL1       );
        __I__(CAM_LSC_CTL2       );
        __I__(CAM_LSC_CTL3       );
        __I__(CAM_LSC_LBLOCK     );
        __I__(CAM_LSC_RATIO_0    );
        __I__(CAM_LSC_RATIO_1    );
        __I__(CAM_RMG_HDR_CFG    );
        __I__(CAM_RMG_HDR_GAIN   );
        __I__(CAM_RMG_HDR_CFG2   );
        __I__(CAM_RPG_SATU_1     );
        __I__(CAM_RPG_SATU_2     );
        __I__(CAM_RPG_GAIN_1     );
        __I__(CAM_RPG_GAIN_2     );
        __I__(CAM_RPG_OFST_1     );
        __I__(CAM_RPG_OFST_2     );
        __I__(CAM_LCS25_FLR      );
        __I__(CAM_LCS25_SATU_1   );
        __I__(CAM_LCS25_SATU_2   );
        __I__(CAM_LCS25_GAIN_1   );
        __I__(CAM_LCS25_GAIN_2   );
        __I__(CAM_LCS25_OFST_1   );
        __I__(CAM_LCS25_OFST_2   );
        __I__(CAM_LCS25_G2G_CNV_1);
        __I__(CAM_LCS25_G2G_CNV_2);
        __I__(CAM_LCS25_G2G_CNV_3);
        __I__(CAM_LCS25_G2G_CNV_4);
        __I__(CAM_LCS25_G2G_CNV_5);
        __I__(CAM_RRZ_CTL           );
        __I__(CAM_RRZ_IN_IMG        );
        __I__(CAM_RRZ_OUT_IMG       );
        __I__(CAM_RRZ_HORI_STEP     );
        __I__(CAM_RRZ_VERT_STEP     );
        __I__(CAM_RRZ_HORI_INT_OFST );
        __I__(CAM_RRZ_HORI_SUB_OFST );
        __I__(CAM_RRZ_VERT_INT_OFST );
        __I__(CAM_RRZ_VERT_SUB_OFST );
        __I__(CAM_RRZ_MODE_TH       );
        __I__(CAM_RRZ_MODE_CTL      );
        __I__(CAM_BIN_CTL           );
        __I__(CAM_BIN_FTH           );
    }

    typedef enum
    {
        E_DUMP_CAM_CTL_EN           = 0,
        E_DUMP_CAM_CTL_DMA_EN       ,
        E_DUMP_CAM_SL2F_CEN         ,
        E_DUMP_CAM_SL2F_RR_CON0     ,
        E_DUMP_CAM_SL2F_RR_CON1     ,
        E_DUMP_CAM_SL2F_GAIN        ,
        E_DUMP_CAM_SL2F_RZ          ,
        E_DUMP_CAM_SL2F_XOFF        ,
        E_DUMP_CAM_SL2F_YOFF        ,
        E_DUMP_CAM_SL2F_SLP_CON0    ,
        E_DUMP_CAM_SL2F_SLP_CON1    ,
        E_DUMP_CAM_SL2F_SIZE        ,
        E_DUMP_CAM_DBS_SIGMA        ,
        E_DUMP_CAM_DBS_BSTBL_0      ,
        E_DUMP_CAM_DBS_BSTBL_1      ,
        E_DUMP_CAM_DBS_BSTBL_2      ,
        E_DUMP_CAM_DBS_BSTBL_3      ,
        E_DUMP_CAM_DBS_CTL          ,
        E_DUMP_CAM_DBS_SIGMA_2      ,
        E_DUMP_CAM_DBS_YGN          ,
        E_DUMP_CAM_DBS_SL_Y12       ,
        E_DUMP_CAM_DBS_SL_Y34       ,
        E_DUMP_CAM_DBS_SL_G12       ,
        E_DUMP_CAM_DBS_SL_G34       ,
        E_DUMP_CAM_OBC_OFFST0       ,
        E_DUMP_CAM_OBC_OFFST1       ,
        E_DUMP_CAM_OBC_OFFST2       ,
        E_DUMP_CAM_OBC_OFFST3       ,
        E_DUMP_CAM_OBC_GAIN0        ,
        E_DUMP_CAM_OBC_GAIN1        ,
        E_DUMP_CAM_OBC_GAIN2        ,
        E_DUMP_CAM_OBC_GAIN3        ,
        E_DUMP_CAM_BNR_BPC_CON      ,
        E_DUMP_CAM_BNR_BPC_TH1      ,
        E_DUMP_CAM_BNR_BPC_TH2      ,
        E_DUMP_CAM_BNR_BPC_TH3      ,
        E_DUMP_CAM_BNR_BPC_TH4      ,
        E_DUMP_CAM_BNR_BPC_DTC      ,
        E_DUMP_CAM_BNR_BPC_COR      ,
        E_DUMP_CAM_BNR_BPC_TBLI1    ,
        E_DUMP_CAM_BNR_BPC_TBLI2    ,
        E_DUMP_CAM_BNR_BPC_TH1_C    ,
        E_DUMP_CAM_BNR_BPC_TH2_C    ,
        E_DUMP_CAM_BNR_BPC_TH3_C    ,
        E_DUMP_CAM_BNR_NR1_CON      ,
        E_DUMP_CAM_BNR_NR1_CT_CON   ,
        E_DUMP_CAM_BNR_PDC_CON      ,
        E_DUMP_CAM_BNR_PDC_GAIN_L0  ,
        E_DUMP_CAM_BNR_PDC_GAIN_L1  ,
        E_DUMP_CAM_BNR_PDC_GAIN_L2  ,
        E_DUMP_CAM_BNR_PDC_GAIN_L3  ,
        E_DUMP_CAM_BNR_PDC_GAIN_L4  ,
        E_DUMP_CAM_BNR_PDC_GAIN_R0  ,
        E_DUMP_CAM_BNR_PDC_GAIN_R1  ,
        E_DUMP_CAM_BNR_PDC_GAIN_R2  ,
        E_DUMP_CAM_BNR_PDC_GAIN_R3  ,
        E_DUMP_CAM_BNR_PDC_GAIN_R4  ,
        E_DUMP_CAM_BNR_PDC_TH_GB    ,
        E_DUMP_CAM_BNR_PDC_TH_IA    ,
        E_DUMP_CAM_BNR_PDC_TH_HD    ,
        E_DUMP_CAM_BNR_PDC_SL       ,
        E_DUMP_CAM_BNR_PDC_POS      ,
        E_DUMP_CAM_RMM_OSC          ,
        E_DUMP_CAM_RMM_MC           ,
        E_DUMP_CAM_RMM_REVG_1       ,
        E_DUMP_CAM_RMM_REVG_2       ,
        E_DUMP_CAM_RMM_LEOS         ,
        E_DUMP_CAM_RMM_MC2          ,
        E_DUMP_CAM_RMM_DIFF_LB      ,
        E_DUMP_CAM_RMM_MA           ,
        E_DUMP_CAM_RMM_TUNE         ,
        //E_DUMP_CAM_RMM_IDX          ,
        E_DUMP_CAM_LSC_CTL1         ,
        E_DUMP_CAM_LSC_CTL2         ,
        E_DUMP_CAM_LSC_CTL3         ,
        E_DUMP_CAM_LSC_LBLOCK       ,
        E_DUMP_CAM_LSC_RATIO_0      ,
        E_DUMP_CAM_LSC_RATIO_1      ,
        E_DUMP_CAM_RMG_HDR_CFG      ,
        E_DUMP_CAM_RMG_HDR_GAIN     ,
        E_DUMP_CAM_RMG_HDR_CFG2     ,
        E_DUMP_CAM_RPG_SATU_1       ,
        E_DUMP_CAM_RPG_SATU_2       ,
        E_DUMP_CAM_RPG_GAIN_1       ,
        E_DUMP_CAM_RPG_GAIN_2       ,
        E_DUMP_CAM_RPG_OFST_1       ,
        E_DUMP_CAM_RPG_OFST_2       ,
        E_DUMP_CAM_LCS25_FLR        ,
        E_DUMP_CAM_LCS25_SATU_1     ,
        E_DUMP_CAM_LCS25_SATU_2     ,
        E_DUMP_CAM_LCS25_GAIN_1     ,
        E_DUMP_CAM_LCS25_GAIN_2     ,
        E_DUMP_CAM_LCS25_OFST_1     ,
        E_DUMP_CAM_LCS25_OFST_2     ,
        E_DUMP_CAM_LCS25_G2G_CNV_1  ,
        E_DUMP_CAM_LCS25_G2G_CNV_2  ,
        E_DUMP_CAM_LCS25_G2G_CNV_3  ,
        E_DUMP_CAM_LCS25_G2G_CNV_4  ,
        E_DUMP_CAM_LCS25_G2G_CNV_5  ,
        E_DUMP_CAM_RRZ_CTL          ,
        E_DUMP_CAM_RRZ_IN_IMG       ,
        E_DUMP_CAM_RRZ_OUT_IMG      ,
        E_DUMP_CAM_RRZ_HORI_STEP    ,
        E_DUMP_CAM_RRZ_VERT_STEP    ,
        E_DUMP_CAM_RRZ_HORI_INT_OFST,
        E_DUMP_CAM_RRZ_HORI_SUB_OFST,
        E_DUMP_CAM_RRZ_VERT_INT_OFST,
        E_DUMP_CAM_RRZ_VERT_SUB_OFST,
        E_DUMP_CAM_RRZ_MODE_TH      ,
        E_DUMP_CAM_RRZ_MODE_CTL     ,
        E_DUMP_CAM_BIN_CTL          ,
        E_DUMP_CAM_BIN_FTH          ,
        E_DUMP_NUM
    } E_DUMP_REG_T;
    IspP1RegIo rRegs[E_DUMP_NUM];
};

IspDebug::
IspDebug(MINT32 i4SensorIdx, MINT32 i4SensorDev)
    : m_i4SensorIdx(i4SensorIdx)
    , m_i4SensorDev(i4SensorDev)
{
}

MBOOL
IspDebug::
dumpRegs(MUINT32 u4MagicNum)
{
    MBOOL fgRet = MFALSE;
    char filename[512];

    INormalPipe* pPipe = (INormalPipe*)createDefaultNormalPipe(m_i4SensorIdx, LOG_TAG);
    if (!pPipe)
    {
        CAM_LOGE("Fail to create INormalPipe");
        return MFALSE;
    }

    sprintf(filename, "/sdcard/debug/p1dbg-%04d.log", u4MagicNum);
    FILE* fp = fopen(filename, "w");
    sprintf(filename, "/sdcard/debug/p1dbg-%04d.bin", u4MagicNum);
    FILE* fp_b = fopen(filename, "wb");

    if (fp && fp_b)
    {
        MUINT32 u4RegCnt = (_A_P1(CAM_SL2F_SIZE) / 4) + 1;
        MUINT32 u4RRZOfst = (_A_P1(CAM_RRZ_CTL) / 4);
        MUINT32* regData = new MUINT32[u4RegCnt];
        if (regData)
        {
            pPipe->getIspReg(0, u4RegCnt, regData);
            ::fwrite(regData, u4RegCnt*4, 1, fp_b);

            ISP_NVRAM_RRZ_T rrz;
            memcpy(&rrz, regData+u4RRZOfst, sizeof(ISP_NVRAM_RRZ_T));
            IspTuningMgr::getInstance().setRRZInfo(m_i4SensorDev, &rrz);
            delete [] regData;
        }

#define _DUMP(param) fprintf(fp, "%-40s:    0x%08X\n", #param, rDump.rRegs[DUMP_INFO::E_DUMP_##param].Data)

            DUMP_INFO rDump;
            pPipe->getIspReg(rDump.rRegs, DUMP_INFO::E_DUMP_NUM);

            _DUMP(CAM_CTL_EN         );
            _DUMP(CAM_CTL_DMA_EN     );
            _DUMP(CAM_SL2F_CEN       );
            _DUMP(CAM_SL2F_RR_CON0   );
            _DUMP(CAM_SL2F_RR_CON1   );
            _DUMP(CAM_SL2F_GAIN      );
            _DUMP(CAM_SL2F_RZ        );
            _DUMP(CAM_SL2F_XOFF      );
            _DUMP(CAM_SL2F_YOFF      );
            _DUMP(CAM_SL2F_SLP_CON0  );
            _DUMP(CAM_SL2F_SLP_CON1  );
            _DUMP(CAM_SL2F_SIZE      );
            _DUMP(CAM_DBS_SIGMA      );
            _DUMP(CAM_DBS_BSTBL_0    );
            _DUMP(CAM_DBS_BSTBL_1    );
            _DUMP(CAM_DBS_BSTBL_2    );
            _DUMP(CAM_DBS_BSTBL_3    );
            _DUMP(CAM_DBS_CTL        );
            _DUMP(CAM_DBS_SIGMA_2    );
            _DUMP(CAM_DBS_YGN        );
            _DUMP(CAM_DBS_SL_Y12     );
            _DUMP(CAM_DBS_SL_Y34     );
            _DUMP(CAM_DBS_SL_G12     );
            _DUMP(CAM_DBS_SL_G34     );
            _DUMP(CAM_OBC_OFFST0     );
            _DUMP(CAM_OBC_OFFST1     );
            _DUMP(CAM_OBC_OFFST2     );
            _DUMP(CAM_OBC_OFFST3     );
            _DUMP(CAM_OBC_GAIN0      );
            _DUMP(CAM_OBC_GAIN1      );
            _DUMP(CAM_OBC_GAIN2      );
            _DUMP(CAM_OBC_GAIN3      );
            _DUMP(CAM_BNR_BPC_CON    );
            _DUMP(CAM_BNR_BPC_TH1    );
            _DUMP(CAM_BNR_BPC_TH2    );
            _DUMP(CAM_BNR_BPC_TH3    );
            _DUMP(CAM_BNR_BPC_TH4    );
            _DUMP(CAM_BNR_BPC_DTC    );
            _DUMP(CAM_BNR_BPC_COR    );
            _DUMP(CAM_BNR_BPC_TBLI1  );
            _DUMP(CAM_BNR_BPC_TBLI2  );
            _DUMP(CAM_BNR_BPC_TH1_C  );
            _DUMP(CAM_BNR_BPC_TH2_C  );
            _DUMP(CAM_BNR_BPC_TH3_C  );
            _DUMP(CAM_BNR_NR1_CON    );
            _DUMP(CAM_BNR_NR1_CT_CON );
            _DUMP(CAM_BNR_PDC_CON    );
            _DUMP(CAM_BNR_PDC_GAIN_L0);
            _DUMP(CAM_BNR_PDC_GAIN_L1);
            _DUMP(CAM_BNR_PDC_GAIN_L2);
            _DUMP(CAM_BNR_PDC_GAIN_L3);
            _DUMP(CAM_BNR_PDC_GAIN_L4);
            _DUMP(CAM_BNR_PDC_GAIN_R0);
            _DUMP(CAM_BNR_PDC_GAIN_R1);
            _DUMP(CAM_BNR_PDC_GAIN_R2);
            _DUMP(CAM_BNR_PDC_GAIN_R3);
            _DUMP(CAM_BNR_PDC_GAIN_R4);
            _DUMP(CAM_BNR_PDC_TH_GB  );
            _DUMP(CAM_BNR_PDC_TH_IA  );
            _DUMP(CAM_BNR_PDC_TH_HD  );
            _DUMP(CAM_BNR_PDC_SL     );
            _DUMP(CAM_BNR_PDC_POS    );
            _DUMP(CAM_RMM_OSC        );
            _DUMP(CAM_RMM_MC         );
            _DUMP(CAM_RMM_REVG_1     );
            _DUMP(CAM_RMM_REVG_2     );
            _DUMP(CAM_RMM_LEOS       );
            _DUMP(CAM_RMM_MC2        );
            _DUMP(CAM_RMM_DIFF_LB    );
            _DUMP(CAM_RMM_MA         );
            _DUMP(CAM_RMM_TUNE       );
            //_DUMP(CAM_RMM_IDX        );
            _DUMP(CAM_LSC_CTL1       );
            _DUMP(CAM_LSC_CTL2       );
            _DUMP(CAM_LSC_CTL3       );
            _DUMP(CAM_LSC_LBLOCK     );
            _DUMP(CAM_LSC_RATIO_0    );
            _DUMP(CAM_LSC_RATIO_1    );
            _DUMP(CAM_RMG_HDR_CFG    );
            _DUMP(CAM_RMG_HDR_GAIN   );
            _DUMP(CAM_RMG_HDR_CFG2   );
            _DUMP(CAM_RPG_SATU_1     );
            _DUMP(CAM_RPG_SATU_2     );
            _DUMP(CAM_RPG_GAIN_1     );
            _DUMP(CAM_RPG_GAIN_2     );
            _DUMP(CAM_RPG_OFST_1     );
            _DUMP(CAM_RPG_OFST_2     );
            _DUMP(CAM_LCS25_FLR      );
            _DUMP(CAM_LCS25_SATU_1   );
            _DUMP(CAM_LCS25_SATU_2   );
            _DUMP(CAM_LCS25_GAIN_1   );
            _DUMP(CAM_LCS25_GAIN_2   );
            _DUMP(CAM_LCS25_OFST_1   );
            _DUMP(CAM_LCS25_OFST_2   );
            _DUMP(CAM_LCS25_G2G_CNV_1);
            _DUMP(CAM_LCS25_G2G_CNV_2);
            _DUMP(CAM_LCS25_G2G_CNV_3);
            _DUMP(CAM_LCS25_G2G_CNV_4);
            _DUMP(CAM_LCS25_G2G_CNV_5);
            _DUMP(CAM_RRZ_CTL           );
            _DUMP(CAM_RRZ_IN_IMG        );
            _DUMP(CAM_RRZ_OUT_IMG       );
            _DUMP(CAM_RRZ_HORI_STEP     );
            _DUMP(CAM_RRZ_VERT_STEP     );
            _DUMP(CAM_RRZ_HORI_INT_OFST );
            _DUMP(CAM_RRZ_HORI_SUB_OFST );
            _DUMP(CAM_RRZ_VERT_INT_OFST );
            _DUMP(CAM_RRZ_VERT_SUB_OFST );
            _DUMP(CAM_RRZ_MODE_TH       );
            _DUMP(CAM_RRZ_MODE_CTL      );
            _DUMP(CAM_BIN_CTL           );
            _DUMP(CAM_BIN_FTH           );

        fgRet = MTRUE;
    }
    pPipe->destroyInstance(LOG_TAG);

    if (fp)
        fclose(fp);
    if (fp_b)
        fclose(fp_b);

    return fgRet;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 1
    #define ISP_READ_BITS_NOPROTECT(IspDrvPtr,RegName,FieldName,...)    (0)
    #define ISP_READ_REG_NOPROTECT(IspDrvPtr,RegName,...)   (0)
#endif

#if 0
MVOID
IspDebug::
dumpIspDebugMessage_DMA()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.dma", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4DMAOption = atoi(value);

    // DMA status check: TG1
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, AA_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, AA_EN));
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_DMA, AAO_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_DMA, AAO_EN));
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, HBIN_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, HBIN_EN));

    if (i4DMAOption & 1) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_BASE_ADDR, BASE_ADDR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_BASE_ADDR, BASE_ADDR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_OFST_ADDR, OFFSET_ADDR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_OFST_ADDR, OFFSET_ADDR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_XSIZE, XSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_XSIZE, XSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_STRIDE, BUS_SIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_STRIDE, BUS_SIZE));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_AAO_CON));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_AAO_CON2));
    }

    // DMA status check: TG2
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, AA_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, AA_D_EN));
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_DMA_D, AAO_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_DMA_D, AAO_D_EN));

    if (i4DMAOption & 2) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_D_BASE_ADDR, BASE_ADDR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_D_BASE_ADDR, BASE_ADDR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_D_OFST_ADDR, OFFSET_ADDR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_D_OFST_ADDR, OFFSET_ADDR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_D_XSIZE, XSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_D_XSIZE, XSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_D_STRIDE, BUS_SIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AAO_D_STRIDE, BUS_SIZE));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_D_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_AAO_D_CON));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AAO_D_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_AAO_D_CON2));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_AWB()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.awb", value, "0"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4AWBOption = atoi(value);

    // AWB statistics: TG1
    if (i4AWBOption & 1) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_WIN_ORG, AWB_W_HORG) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_ORG, AWB_W_HORG));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_WIN_ORG, AWB_W_VORG) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_ORG, AWB_W_VORG));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_WIN_SIZE, AWB_W_HSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_SIZE, AWB_W_HSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_WIN_SIZE, AWB_W_VSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_SIZE, AWB_W_VSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_WIN_PIT, AWB_W_HPIT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_PIT, AWB_W_HPIT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_WIN_PIT, AWB_W_VPIT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_PIT, AWB_W_VPIT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_WIN_NUM, AWB_W_HNUM) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_NUM, AWB_W_HNUM));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_WIN_NUM, AWB_W_VNUM) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_WIN_NUM, AWB_W_VNUM));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_GAIN1_0, AWB_GAIN1_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_GAIN1_0, AWB_GAIN1_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_GAIN1_0, AWB_GAIN1_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_GAIN1_0, AWB_GAIN1_G));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_GAIN1_1, AWB_GAIN1_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_GAIN1_1, AWB_GAIN1_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_LMT1_0, AWB_LMT1_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LMT1_0, AWB_LMT1_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_LMT1_0, AWB_LMT1_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LMT1_0, AWB_LMT1_G));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_LMT1_1, AWB_LMT1_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LMT1_1, AWB_LMT1_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_LOW_THR, AWB_LOW_THR0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LOW_THR, AWB_LOW_THR0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_LOW_THR, AWB_LOW_THR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LOW_THR, AWB_LOW_THR1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_LOW_THR, AWB_LOW_THR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_LOW_THR, AWB_LOW_THR2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_HI_THR, AWB_HI_THR0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_HI_THR, AWB_HI_THR0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_HI_THR, AWB_HI_THR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_HI_THR, AWB_HI_THR1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_HI_THR, AWB_HI_THR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_HI_THR, AWB_HI_THR2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_PIXEL_CNT0, AWB_PIXEL_CNT0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_PIXEL_CNT0, AWB_PIXEL_CNT0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_PIXEL_CNT1, AWB_PIXEL_CNT1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_PIXEL_CNT1, AWB_PIXEL_CNT1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_PIXEL_CNT2, AWB_PIXEL_CNT2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_PIXEL_CNT2, AWB_PIXEL_CNT2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_ERR_THR, AWB_ERR_THR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_ERR_THR, AWB_ERR_THR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_ERR_THR, AWB_ERR_SFT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_ERR_THR, AWB_ERR_SFT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_ROT, AWB_C) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_ROT, AWB_C));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_ROT, AWB_S) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_ROT, AWB_S));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L0_X, AWB_L0_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L0_X, AWB_L0_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L0_X, AWB_L0_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L0_X, AWB_L0_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L0_Y, AWB_L0_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L0_Y, AWB_L0_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L0_Y, AWB_L0_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L0_Y, AWB_L0_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L1_X, AWB_L1_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L1_X, AWB_L1_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L1_X, AWB_L1_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L1_X, AWB_L1_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L1_Y, AWB_L1_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L1_Y, AWB_L1_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L1_Y, AWB_L1_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L1_Y, AWB_L1_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L2_X, AWB_L2_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L2_X, AWB_L2_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L2_X, AWB_L2_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L2_X, AWB_L2_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L2_Y, AWB_L2_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L2_Y, AWB_L2_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L2_Y, AWB_L2_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L2_Y, AWB_L2_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L3_X, AWB_L3_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L3_X, AWB_L3_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L3_X, AWB_L3_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L3_X, AWB_L3_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L3_Y, AWB_L3_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L3_Y, AWB_L3_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L3_Y, AWB_L3_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L3_Y, AWB_L3_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L4_X, AWB_L4_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L4_X, AWB_L4_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L4_X, AWB_L4_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L4_X, AWB_L4_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L4_Y, AWB_L4_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L4_Y, AWB_L4_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L4_Y, AWB_L4_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L4_Y, AWB_L4_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L5_X, AWB_L5_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L5_X, AWB_L5_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L5_X, AWB_L5_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L5_X, AWB_L5_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L5_Y, AWB_L5_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L5_Y, AWB_L5_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L5_Y, AWB_L5_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L5_Y, AWB_L5_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L6_X, AWB_L6_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L6_X, AWB_L6_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L6_X, AWB_L6_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L6_X, AWB_L6_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L6_Y, AWB_L6_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L6_Y, AWB_L6_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L6_Y, AWB_L6_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L6_Y, AWB_L6_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L7_X, AWB_L7_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L7_X, AWB_L7_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L7_X, AWB_L7_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L7_X, AWB_L7_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L7_Y, AWB_L7_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L7_Y, AWB_L7_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L7_Y, AWB_L7_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L7_Y, AWB_L7_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L8_X, AWB_L8_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L8_X, AWB_L8_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L8_X, AWB_L8_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L8_X, AWB_L8_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L8_Y, AWB_L8_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L8_Y, AWB_L8_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L8_Y, AWB_L8_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L8_Y, AWB_L8_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L9_X, AWB_L9_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L9_X, AWB_L9_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L9_X, AWB_L9_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L9_X, AWB_L9_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L9_Y, AWB_L9_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L9_Y, AWB_L9_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_L9_Y, AWB_L9_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_L9_Y, AWB_L9_Y_UP));
    }

    // AWB statistics: TG2
    if (i4AWBOption & 2) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_ORG, AWB_W_HORG) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_ORG, AWB_W_HORG));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_ORG, AWB_W_VORG) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_ORG, AWB_W_VORG));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_SIZE, AWB_W_HSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_SIZE, AWB_W_HSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_SIZE, AWB_W_HSIZE) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_SIZE, AWB_W_HSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_PIT, AWB_W_HPIT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_PIT, AWB_W_HPIT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_PIT, AWB_W_VPIT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_PIT, AWB_W_VPIT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_NUM, AWB_W_HNUM) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_NUM, AWB_W_HNUM));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_WIN_NUM, AWB_W_VNUM) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_WIN_NUM, AWB_W_VNUM));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_GAIN1_0, AWB_GAIN1_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_GAIN1_0, AWB_GAIN1_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_GAIN1_0, AWB_GAIN1_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_GAIN1_0, AWB_GAIN1_G));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_GAIN1_1, AWB_GAIN1_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_GAIN1_1, AWB_GAIN1_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_LMT1_0, AWB_LMT1_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LMT1_0, AWB_LMT1_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_LMT1_0, AWB_LMT1_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LMT1_0, AWB_LMT1_G));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_LMT1_1, AWB_LMT1_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LMT1_1, AWB_LMT1_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_LOW_THR, AWB_LOW_THR0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LOW_THR, AWB_LOW_THR0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_LOW_THR, AWB_LOW_THR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LOW_THR, AWB_LOW_THR1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_LOW_THR, AWB_LOW_THR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_LOW_THR, AWB_LOW_THR2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_HI_THR, AWB_HI_THR0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_HI_THR, AWB_HI_THR0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_HI_THR, AWB_HI_THR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_HI_THR, AWB_HI_THR1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_HI_THR, AWB_HI_THR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_HI_THR, AWB_HI_THR2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_PIXEL_CNT0, AWB_PIXEL_CNT0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_PIXEL_CNT0, AWB_PIXEL_CNT0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_PIXEL_CNT1, AWB_PIXEL_CNT1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_PIXEL_CNT1, AWB_PIXEL_CNT1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_PIXEL_CNT2, AWB_PIXEL_CNT2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_PIXEL_CNT2, AWB_PIXEL_CNT2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_ERR_THR, AWB_ERR_THR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_ERR_THR, AWB_ERR_THR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_ERR_THR, AWB_ERR_SFT) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_ERR_THR, AWB_ERR_SFT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_ROT, AWB_C) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_ROT, AWB_C));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_ROT, AWB_S) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_ROT, AWB_S));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L0_X, AWB_L0_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L0_X, AWB_L0_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L0_X, AWB_L0_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L0_X, AWB_L0_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L0_Y, AWB_L0_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L0_Y, AWB_L0_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L0_Y, AWB_L0_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L0_Y, AWB_L0_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L1_X, AWB_L1_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L1_X, AWB_L1_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L1_X, AWB_L1_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L1_X, AWB_L1_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L1_Y, AWB_L1_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L1_Y, AWB_L1_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L1_Y, AWB_L1_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L1_Y, AWB_L1_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L2_X, AWB_L2_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L2_X, AWB_L2_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L2_X, AWB_L2_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L2_X, AWB_L2_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L2_Y, AWB_L2_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L2_Y, AWB_L2_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L2_Y, AWB_L2_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L2_Y, AWB_L2_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L3_X, AWB_L3_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L3_X, AWB_L3_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L3_X, AWB_L3_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L3_X, AWB_L3_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L3_Y, AWB_L3_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L3_Y, AWB_L3_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L3_Y, AWB_L3_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L3_Y, AWB_L3_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L4_X, AWB_L4_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L4_X, AWB_L4_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L4_X, AWB_L4_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L4_X, AWB_L4_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L4_Y, AWB_L4_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L4_Y, AWB_L4_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L4_Y, AWB_L4_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L4_Y, AWB_L4_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L5_X, AWB_L5_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L5_X, AWB_L5_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L5_X, AWB_L5_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L5_X, AWB_L5_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L5_Y, AWB_L5_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L5_Y, AWB_L5_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L5_Y, AWB_L5_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L5_Y, AWB_L5_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L6_X, AWB_L6_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L6_X, AWB_L6_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L6_X, AWB_L6_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L6_X, AWB_L6_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L6_Y, AWB_L6_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L6_Y, AWB_L6_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L6_Y, AWB_L6_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L6_Y, AWB_L6_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L7_X, AWB_L7_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L7_X, AWB_L7_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L7_X, AWB_L7_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L7_X, AWB_L7_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L7_Y, AWB_L7_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L7_Y, AWB_L7_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L7_Y, AWB_L7_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L7_Y, AWB_L7_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L8_X, AWB_L8_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L8_X, AWB_L8_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L8_X, AWB_L8_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L8_X, AWB_L8_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L8_Y, AWB_L8_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L8_Y, AWB_L8_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L8_Y, AWB_L8_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L8_Y, AWB_L8_Y_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L9_X, AWB_L9_X_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L9_X, AWB_L9_X_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L9_X, AWB_L9_X_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L9_X, AWB_L9_X_UP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L9_Y, AWB_L9_Y_LOW) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L9_Y, AWB_L9_Y_LOW));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AWB_D_L9_Y, AWB_L9_Y_UP) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AWB_D_L9_Y, AWB_L9_Y_UP));
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_OBC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.obc", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4OBCOption = atoi(value);

    // OBC: TG1
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, OB_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, OB_EN));

    if (i4OBCOption & 1) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_OFFST0, OBC_OFST_B) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_OFFST0, OBC_OFST_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_OFFST1, OBC_OFST_GR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_OFFST1, OBC_OFST_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_OFFST2, OBC_OFST_GB) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_OFFST2, OBC_OFST_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_OFFST3, OBC_OFST_R) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_OFFST3, OBC_OFST_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_GAIN0, OBC_GAIN_B) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_GAIN0, OBC_GAIN_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_GAIN1, OBC_GAIN_GR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_GAIN1, OBC_GAIN_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_GAIN2, OBC_GAIN_GB) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_GAIN2, OBC_GAIN_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_GAIN3, OBC_GAIN_R) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_GAIN3, OBC_GAIN_R));
    }

    // OBC: TG2
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, OB_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, OB_D_EN));

    if (i4OBCOption & 2) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_D_OFFST0, OBC_D_OFST_B) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_OFFST0, OBC_D_OFST_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_D_OFFST1, OBC_D_OFST_GR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_OFFST1, OBC_D_OFST_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_D_OFFST2, OBC_D_OFST_GB) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_OFFST2, OBC_D_OFST_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_D_OFFST3, OBC_D_OFST_R) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_OFFST3, OBC_D_OFST_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_D_GAIN0, OBC_D_GAIN_B) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_GAIN0, OBC_D_GAIN_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_D_GAIN1, OBC_D_GAIN_GR) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_GAIN1, OBC_D_GAIN_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_D_GAIN2, OBC_D_GAIN_GB) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_GAIN2, OBC_D_GAIN_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_OBC_D_GAIN3, OBC_D_GAIN_R) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_OBC_D_GAIN3, OBC_D_GAIN_R));
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_BNR()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.bnr", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4BNROption = atoi(value);

    // BNR: TG1
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, BNR_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, BNR_EN));

    if (i4BNROption & 1) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_CON));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_TH1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_TH2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_TH3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_TH4) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH4));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_DTC) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_DTC));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_COR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_COR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_TBLI1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TBLI1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_TBLI2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TBLI2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_TH1_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH1_C));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_TH2_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH2_C));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_TH3_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_TH3_C));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_RMM1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM1));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_RMM2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM2));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_RMM_REVG_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM_REVG_1));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_RMM_REVG_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM_REVG_2));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_RMM_LEOS) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM_LEOS));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_RMM_GCNT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_RMM_GCNT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR1_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR1_CON));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR1_CT_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR1_CT_CON));
    }

    // BNR: TG2
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, BNR_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, BNR_D_EN));

    if (i4BNROption & 2) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_CON));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_TH1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_TH2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_TH3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_TH4) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH4));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_DTC) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_DTC));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_COR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_COR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_TBLI1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TBLI1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_TBLI2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TBLI2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_TH1_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH1_C));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_TH2_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH2_C));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_TH3_C) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_TH3_C));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_RMM1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM1));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_RMM2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM2));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_RMM_REVG_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM_REVG_1));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_RMM_REVG_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM_REVG_2));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_RMM_LEOS) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM_LEOS));
//        CAM_LOGD_IF(m_bDebugEnable,"(CAM_BPC_D_RMM_GCNT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_BPC_D_RMM_GCNT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR1_D_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR1_D_CON));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR1_D_CT_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR1_D_CT_CON));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_LSC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.lsc", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4LSCOption = atoi(value);

    // LSC: TG1
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, LSC_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, LSC_EN));

    if (i4LSCOption & 1) {
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_BA     = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_BASE_ADDR));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_XSIZE  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_XSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_YSIZE  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_YSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_STRIDE = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_STRIDE));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_CON    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_CON2   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON2));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_CTL1    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL1));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_CTL2    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL2));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_CTL3    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL3));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_LBLOCK  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_LBLOCK));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_RATIO   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_RATIO));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_GAIN_TH = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_GAIN_TH));
    }

    // LSC: TG2
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, LSC_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, LSC_D_EN));

    if (i4LSCOption & 2) {
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_D_BA     = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_BASE_ADDR));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_D_XSIZE  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_XSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_D_YSIZE  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_YSIZE));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_D_STRIDE = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_STRIDE));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_D_CON    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_CON));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSCI_D_CON2   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_D_CON2));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_D_CTL1    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_CTL1));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_D_CTL2    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_CTL2));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_D_CTL3    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_CTL3));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_D_LBLOCK  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_LBLOCK));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_D_RATIO   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_RATIO));
        CAM_LOGD_IF(m_bDebugEnable,"CAM_LSC_D_GAIN_TH = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_D_GAIN_TH));
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_RPG()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.rpg", value, "3"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4RPGOption = atoi(value);

    // RPG: TG1
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, RPG_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, RPG_EN));

    if (i4RPGOption & 1) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_SATU_1, RPG_SATU_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_SATU_1, RPG_SATU_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_SATU_1, RPG_SATU_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_SATU_1, RPG_SATU_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_SATU_2, RPG_SATU_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_SATU_2, RPG_SATU_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_SATU_2, RPG_SATU_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_SATU_2, RPG_SATU_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_GAIN_1, RPG_GAIN_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_GAIN_1, RPG_GAIN_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_GAIN_1, RPG_GAIN_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_GAIN_1, RPG_GAIN_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_GAIN_2, RPG_GAIN_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_GAIN_2, RPG_GAIN_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_GAIN_2, RPG_GAIN_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_GAIN_2, RPG_GAIN_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_OFST_1, RPG_OFST_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_OFST_1, RPG_OFST_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_OFST_1, RPG_OFST_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_OFST_1, RPG_OFST_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_OFST_2, RPG_OFST_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_OFST_2, RPG_OFST_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_OFST_2, RPG_OFST_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_OFST_2, RPG_OFST_R));
    }

    // RPG: TG2
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, RPG_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, RPG_D_EN));

    if (i4RPGOption & 2) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_SATU_1, RPG_SATU_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_SATU_1, RPG_SATU_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_SATU_1, RPG_SATU_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_SATU_1, RPG_SATU_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_SATU_2, RPG_SATU_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_SATU_2, RPG_SATU_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_SATU_2, RPG_SATU_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_SATU_2, RPG_SATU_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_GAIN_1, RPG_GAIN_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_GAIN_1, RPG_GAIN_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_GAIN_1, RPG_GAIN_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_GAIN_1, RPG_GAIN_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_GAIN_2, RPG_GAIN_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_GAIN_2, RPG_GAIN_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_GAIN_2, RPG_GAIN_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_GAIN_2, RPG_GAIN_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_OFST_1, RPG_OFST_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_OFST_1, RPG_OFST_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_OFST_1, RPG_OFST_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_OFST_1, RPG_OFST_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_OFST_2, RPG_OFST_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_OFST_2, RPG_OFST_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_RPG_D_OFST_2, RPG_OFST_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_RPG_D_OFST_2, RPG_OFST_R));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_PGN()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.pgn", value, "1"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, PGN_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, PGN_EN));

    if (bDebugEnable) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_SATU_1, PGN_SATU_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_SATU_1, PGN_SATU_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_SATU_1, PGN_SATU_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_SATU_1, PGN_SATU_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_SATU_2, PGN_SATU_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_SATU_2, PGN_SATU_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_SATU_2, PGN_SATU_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_SATU_2, PGN_SATU_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_GAIN_1, PGN_GAIN_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_GAIN_1, PGN_GAIN_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_GAIN_1, PGN_GAIN_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_GAIN_1, PGN_GAIN_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_GAIN_2, PGN_GAIN_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_GAIN_2, PGN_GAIN_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_GAIN_2, PGN_GAIN_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_GAIN_2, PGN_GAIN_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_OFST_1, PGN_OFST_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_OFST_1, PGN_OFST_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_OFST_1, PGN_OFST_GB) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_OFST_1, PGN_OFST_GB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_OFST_2, PGN_OFST_GR) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_OFST_2, PGN_OFST_GR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PGN_OFST_2, PGN_OFST_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_PGN_OFST_2, PGN_OFST_R));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_CFA()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.cfa", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, CFA_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, CFA_EN));

    if (bDebugEnable) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_BYP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_BYP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_ED_FLAT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_ED_FLAT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_ED_NYQ) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_ED_NYQ));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_ED_STEP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_ED_STEP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_RGB_HF) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_RGB_HF));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_DOT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_DOT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_F1_ACT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F1_ACT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_F2_ACT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F2_ACT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_F3_ACT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F3_ACT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_F4_ACT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F4_ACT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_F1_L) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F1_L));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_F2_L) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F2_L));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_F3_L) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F3_L));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_F4_L) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_F4_L));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_HF_RB) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_HF_RB));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_HF_GAIN) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_HF_GAIN));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_HF_COMP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_HF_COMP));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_HF_CORIN_TH) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_HF_CORIN_TH));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_ACT_LUT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_ACT_LUT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_SPARE) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_SPARE));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_DM_O_BB) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_DM_O_BB));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_CCM()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.ccm", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, G2G_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, G2G_EN));

    if (bDebugEnable) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CNV_1, G2G_CNV_00) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_1, G2G_CNV_00));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CNV_1, G2G_CNV_01) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_1, G2G_CNV_01));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CNV_2, G2G_CNV_02) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_2, G2G_CNV_02));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CNV_3, G2G_CNV_10) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_3, G2G_CNV_10));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CNV_3, G2G_CNV_11) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_3, G2G_CNV_11));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CNV_4, G2G_CNV_12) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_4, G2G_CNV_12));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CNV_5, G2G_CNV_20) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_5, G2G_CNV_20));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CNV_5, G2G_CNV_21) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_5, G2G_CNV_21));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CNV_6, G2G_CNV_22) = 0x%8x\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CNV_6, G2G_CNV_22));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2G_CTRL, G2G_ACC) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_G2G_CTRL, G2G_ACC));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_GGM()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.ggm", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, GGM_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, GGM_EN));
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_GGM_CTRL, GAMMA_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_GGM_CTRL, GGM_EN));

    if (bDebugEnable) {
        for (MINT32 i = 0; i < 144; i++) {
            CAM_LOGD_IF(m_bDebugEnable,"(CAM_GGM_LUT_RB[%d]) = 0x%8x\n", i, ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_GGM_LUT_RB[i]));
            CAM_LOGD_IF(m_bDebugEnable,"(CAM_GGM_LUT_G[%d]) = 0x%8x\n", i, ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_GGM_LUT_G[i]));
        }
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_G2C()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.g2c", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, G2C_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, G2C_EN));

    if (bDebugEnable) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_CONV_0A) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_0A));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_CONV_0B) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_0B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_CONV_1A) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_1A));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_CONV_1B) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_1B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_CONV_2A) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_2A));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_CONV_2B) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_CONV_2B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_SHADE_CON_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_CON_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_SHADE_CON_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_CON_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_SHADE_CON_3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_CON_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_SHADE_TAR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_TAR));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_G2C_SHADE_SP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_G2C_SHADE_SP));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_NBC()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.nbc", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, NBC_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, NBC_EN));

    if (bDebugEnable) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_CON1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_CON2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_CON3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_YAD1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_YAD1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_YAD2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_YAD2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_4LUT1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_4LUT1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_4LUT2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_4LUT2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_4LUT3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_4LUT3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_PTY) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_PTY));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_CAD) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_CAD));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_PTC) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_PTC));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_LCE1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_LCE1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_LCE2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_LCE2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_HP1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_HP1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_HP2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_HP2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_HP3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_HP3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_ACTY) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_ACTY));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_ANR_ACTC) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_ANR_ACTC));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_CCR_CON) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_CON));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_CCR_YLUT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_YLUT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_CCR_UVLUT) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_UVLUT));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_CCR_YLUT2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_YLUT2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_CCR_SAT_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_SAT_CTRL));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_CCR_UVLUT_SP) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_CCR_UVLUT_SP));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_PCA()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.pca", value, "0"); // 0: disable, 1: 180BIN, 2: 360BIN
    MINT32 i4PCAOption = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, PCA_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, PCA_EN));

    if (i4PCAOption & 1) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PCA_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_CON1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PCA_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_CON2));

        for (MINT32 i = 0; i < 180; i++) {
            CAM_LOGD_IF(m_bDebugEnable,"(CAM_PCA_TBL[%d]) = 0x%8x\n", i, ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_TBL[i]));
        }
    }

    if (i4PCAOption & 2) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PCA_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_CON1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_PCA_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_CON2));

        for (MINT32 i = 0; i < 360; i++) {
            CAM_LOGD_IF(m_bDebugEnable,"(CAM_PCA_TBL[%d]) = 0x%8x\n", i, ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_PCA_TBL[i]));
        }
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_SEEE()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.seee", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, SEEE_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, SEEE_EN));

    if (bDebugEnable) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_SRK_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SRK_CTRL));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_CLIP_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_CLIP_CTRL));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_FLT_CTRL_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_FLT_CTRL_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_FLT_CTRL_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_FLT_CTRL_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_01) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_01));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_02) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_02));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_03) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_03));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_04) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_04));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_05) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_05));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_06) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_06));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_EDTR_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_EDTR_CTRL));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_07) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_07));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_08) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_08));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_09) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_09));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_10) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_10));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_GLUT_CTRL_11) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_GLUT_CTRL_11));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_OUT_EDGE_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_OUT_EDGE_CTRL));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_SE_Y_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_Y_CTRL));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_SE_EDGE_CTRL_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_EDGE_CTRL_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_SE_EDGE_CTRL_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_EDGE_CTRL_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_SE_EDGE_CTRL_3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_EDGE_CTRL_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_SE_SPECL_CTRL) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_SPECL_CTRL));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_SE_CORE_CTRL_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_CORE_CTRL_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_SEEE_SE_CORE_CTRL_2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SEEE_SE_CORE_CTRL_2));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_AE()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.ae", value, "0"); // 0: disable, 1: TG1, 2: TG2, 3: TG1/TG2
    MINT32 i4AEOption = atoi(value);

    // AE statistics: TG1
    if (i4AEOption & 1) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_CTL, AE_HST0_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_CTL, AE_HST0_EN));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_CTL, AE_HST1_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_CTL, AE_HST1_EN));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_CTL, AE_HST2_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_CTL, AE_HST2_EN));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_CTL, AE_HST3_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_CTL, AE_HST3_EN));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_GAIN2_0, AE_GAIN2_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_GAIN2_0, AE_GAIN2_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_GAIN2_0, AE_GAIN2_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_GAIN2_0, AE_GAIN2_G));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_GAIN2_1, AE_GAIN2_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_GAIN2_1, AE_GAIN2_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_LMT2_0, AE_LMT2_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_LMT2_0, AE_LMT2_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_LMT2_0, AE_LMT2_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_LMT2_0, AE_LMT2_G));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_LMT2_1, AE_LMT2_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_LMT2_1, AE_LMT2_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_0, AE_RC_CNV00) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_0, AE_RC_CNV00));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_0, AE_RC_CNV01) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_0, AE_RC_CNV01));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_1, AE_RC_CNV02) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_1, AE_RC_CNV02));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_1, AE_RC_CNV10) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_1, AE_RC_CNV10));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_2, AE_RC_CNV11) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_2, AE_RC_CNV11));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_2, AE_RC_CNV12) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_2, AE_RC_CNV12));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_3, AE_RC_CNV20) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_3, AE_RC_CNV20));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_3, AE_RC_CNV21) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_3, AE_RC_CNV21));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_4, AE_RC_CNV22) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_4, AE_RC_CNV22));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_RC_CNV_4, AE_RC_ACC) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_RC_CNV_4, AE_RC_ACC));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_0, Y_GMR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_0, Y_GMR1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_0, Y_GMR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_0, Y_GMR2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_0, Y_GMR3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_0, Y_GMR3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_0, Y_GMR4) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_0, Y_GMR4));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_YGAMMA_1, Y_GMR5) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_YGAMMA_1, Y_GMR5));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_BIN_MODE_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_BIN_MODE_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_BIN_MODE_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_BIN_MODE_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_BIN_MODE_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_BIN_MODE_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_BIN_MODE_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_BIN_MODE_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_COLOR_MODE_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_COLOR_MODE_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_COLOR_MODE_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_COLOR_MODE_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_COLOR_MODE_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_COLOR_MODE_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST_SET, AE_COLOR_MODE_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST_SET, AE_COLOR_MODE_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST0_RNG, AE_X_LOW_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST0_RNG, AE_X_LOW_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST0_RNG, AE_X_HI_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST0_RNG, AE_X_HI_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST0_RNG, AE_Y_LOW_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST0_RNG, AE_Y_LOW_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST0_RNG, AE_Y_HI_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST0_RNG, AE_Y_HI_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST1_RNG, AE_X_LOW_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST1_RNG, AE_X_LOW_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST1_RNG, AE_X_HI_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST1_RNG, AE_X_HI_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST1_RNG, AE_Y_LOW_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST1_RNG, AE_Y_LOW_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST1_RNG, AE_Y_HI_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST1_RNG, AE_Y_HI_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST2_RNG, AE_X_LOW_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST2_RNG, AE_X_LOW_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST2_RNG, AE_X_HI_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST2_RNG, AE_X_HI_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST2_RNG, AE_Y_LOW_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST2_RNG, AE_Y_LOW_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST2_RNG, AE_Y_HI_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST2_RNG, AE_Y_HI_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST3_RNG, AE_X_LOW_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST3_RNG, AE_X_LOW_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST3_RNG, AE_X_HI_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST3_RNG, AE_X_HI_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST3_RNG, AE_Y_LOW_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST3_RNG, AE_Y_LOW_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_HST3_RNG, AE_Y_HI_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_HST3_RNG, AE_Y_HI_3));
     }

    // AWB statistics: TG2
    if (i4AEOption & 2) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_CTL, AE_HST0_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_CTL, AE_HST0_EN));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_CTL, AE_HST1_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_CTL, AE_HST1_EN));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_CTL, AE_HST2_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_CTL, AE_HST2_EN));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_CTL, AE_HST3_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_CTL, AE_HST3_EN));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_GAIN2_0, AE_GAIN2_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_GAIN2_0, AE_GAIN2_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_GAIN2_0, AE_GAIN2_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_GAIN2_0, AE_GAIN2_G));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_GAIN2_1, AE_GAIN2_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_GAIN2_1, AE_GAIN2_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_LMT2_0, AE_LMT2_R) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_LMT2_0, AE_LMT2_R));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_LMT2_0, AE_LMT2_G) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_LMT2_0, AE_LMT2_G));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_LMT2_1, AE_LMT2_B) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_LMT2_1, AE_LMT2_B));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_0, AE_RC_CNV00) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_0, AE_RC_CNV00));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_0, AE_RC_CNV01) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_0, AE_RC_CNV01));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_1, AE_RC_CNV02) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_1, AE_RC_CNV02));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_1, AE_RC_CNV10) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_1, AE_RC_CNV10));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_2, AE_RC_CNV11) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_2, AE_RC_CNV11));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_2, AE_RC_CNV12) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_2, AE_RC_CNV12));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_3, AE_RC_CNV20) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_3, AE_RC_CNV20));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_3, AE_RC_CNV21) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_3, AE_RC_CNV21));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_4, AE_RC_CNV22) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_4, AE_RC_CNV22));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_RC_CNV_4, AE_RC_ACC) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_RC_CNV_4, AE_RC_ACC));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_0, Y_GMR1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_0, Y_GMR1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_0, Y_GMR2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_0, Y_GMR2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_0, Y_GMR3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_0, Y_GMR3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_0, Y_GMR4) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_0, Y_GMR4));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_YGAMMA_1, Y_GMR5) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_YGAMMA_1, Y_GMR5));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_BIN_MODE_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_BIN_MODE_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_BIN_MODE_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_BIN_MODE_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_BIN_MODE_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_BIN_MODE_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_BIN_MODE_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_BIN_MODE_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_COLOR_MODE_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_COLOR_MODE_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_COLOR_MODE_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_COLOR_MODE_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_COLOR_MODE_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_COLOR_MODE_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST_SET, AE_COLOR_MODE_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST_SET, AE_COLOR_MODE_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST0_RNG, AE_X_LOW_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST0_RNG, AE_X_LOW_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST0_RNG, AE_X_HI_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST0_RNG, AE_X_HI_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST0_RNG, AE_Y_LOW_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST0_RNG, AE_Y_LOW_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST0_RNG, AE_Y_HI_0) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST0_RNG, AE_Y_HI_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST1_RNG, AE_X_LOW_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST1_RNG, AE_X_LOW_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST1_RNG, AE_X_HI_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST1_RNG, AE_X_HI_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST1_RNG, AE_Y_LOW_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST1_RNG, AE_Y_LOW_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST1_RNG, AE_Y_HI_1) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST1_RNG, AE_Y_HI_1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST2_RNG, AE_X_LOW_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST2_RNG, AE_X_LOW_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST2_RNG, AE_X_HI_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST2_RNG, AE_X_HI_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST2_RNG, AE_Y_LOW_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST2_RNG, AE_Y_LOW_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST2_RNG, AE_Y_HI_2) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST2_RNG, AE_Y_HI_2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST3_RNG, AE_X_LOW_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST3_RNG, AE_X_LOW_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST3_RNG, AE_X_HI_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST3_RNG, AE_X_HI_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST3_RNG, AE_Y_LOW_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST3_RNG, AE_Y_LOW_3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_AE_D_HST3_RNG, AE_Y_HI_3) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_AE_D_HST3_RNG, AE_Y_HI_3));
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_NR3D()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.nr3d", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, NR3D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, NR3D_EN));

    if (bDebugEnable) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_BLEND) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_BLEND));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_CPX) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_CPX));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_Y_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_Y_CON1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_Y_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_Y_CON2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_Y_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_Y_CON3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_U_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_U_CON1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_U_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_U_CON2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_U_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_U_CON3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_V_CON1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_V_CON1));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_V_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_V_CON2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_NR3D_LMT_V_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_NR3D_LMT_V_CON3));
    }

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_MFB()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.mfb", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, MFB_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, MFB_EN));

    if (bDebugEnable) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_MFB_LL_CON2) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON2));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_MFB_LL_CON3) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON3));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_MFB_LL_CON4) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON4));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_MFB_LL_CON5) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON5));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_MFB_LL_CON6) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MFB_LL_CON6));
    }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
IspDebug::
dumpIspDebugMessage_MIX3()
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.debug.mix3", value, "0"); // 0: disable, 1: enable
    MBOOL bDebugEnable = atoi(value);

    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, MIX3_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, MIX3_EN));

    if (bDebugEnable) {
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_MIX3_CTRL_0) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MIX3_CTRL_0));
        CAM_LOGD_IF(m_bDebugEnable,"(CAM_MIX3_CTRL_1) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_MIX3_CTRL_1));
    }

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
IspDebug::
dumpIspDebugMessage()
{
    //CAM_LOGD_IF("%s()\n", __FUNCTION__);

    dumpIspDebugMessage_DMA();
    dumpIspDebugMessage_AWB();
    dumpIspDebugMessage_OBC();
    dumpIspDebugMessage_LSC();
    dumpIspDebugMessage_BNR();
    dumpIspDebugMessage_RPG();
    dumpIspDebugMessage_PGN();
    dumpIspDebugMessage_CFA();
    dumpIspDebugMessage_CCM();
    dumpIspDebugMessage_GGM();
    dumpIspDebugMessage_G2C();
    dumpIspDebugMessage_NBC();
    dumpIspDebugMessage_PCA();
    dumpIspDebugMessage_SEEE();
    dumpIspDebugMessage_NR3D();
    dumpIspDebugMessage_AE();
    dumpIspDebugMessage_MFB();
    dumpIspDebugMessage_MIX3();

    // LSC
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1, LSC_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1, LSC_EN));
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P1_D, LSC_D_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P1_D, LSC_D_EN));
    #if 0
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_BA)    = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_BASE_ADDR));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_XSIZE) = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_XSIZE));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_YSIZE) = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_YSIZE));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_STRIDE)= 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_STRIDE));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON)   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON2)  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSCI_CON2));

    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL1)   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL1));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL2)   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL2));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL3)   = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_CTL3));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_LBLOCK) = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_LBLOCK));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_RATIO)  = 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_RATIO));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_GAIN_TH)= 0x%08x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_LSC_GAIN_TH));
    #endif

    // SL2
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, SL2_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, SL2_EN));
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, SL2B_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, SL2B_EN));
    CAM_LOGD_IF(m_bDebugEnable,"(CAM_CTL_EN_P2, SL2C_EN) = %d\n", ISP_READ_BITS_NOPROTECT(m_pIspDrv , CAM_CTL_EN_P2, SL2C_EN));
    #if 0
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_CEN) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_CEN));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX0_RR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX0_RR));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX1_RR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX1_RR));
    CAM_LOGD_IF(m_bDebugEnable,"ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX2_RR) = 0x%8x\n", ISP_READ_REG_NOPROTECT(m_pIspDrv , CAM_SL2_MAX2_RR));
    #endif

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IspDebug::
readLsciAddr(MUINT32 u4TgInfo)
{
    MUINT32 u4Addr;
    switch (u4TgInfo)
    {
    case ESensorTG_1:
        u4Addr = ISP_READ_REG_NOPROTECT(m_pIspDrv, CAM_LSCI_BASE_ADDR);
        break;
    case ESensorTG_2:
        u4Addr = ISP_READ_REG_NOPROTECT(m_pIspDrv, CAM_LSCI_D_BASE_ADDR);
        break;
    }
    return u4Addr;
}
#endif

