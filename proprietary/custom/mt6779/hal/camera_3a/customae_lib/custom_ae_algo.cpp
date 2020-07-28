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

#define LOG_TAG "CustomAEalgo"

#include "algorithm/ae_algo_ctrl.h"
#include "isp_tuning.h"
#include <cutils/log.h>
#include <aaa/ae_param.h>
#include "custom_ae_algo_if.h"
#include "camera_custom_ae_tuning.h"

using namespace NSIspTuning;
using namespace NS3Av3;

template <ESensorDev_T const eSensorDev>
class CustomAeAlgoDev : public ICustomAeAlgo
{
    public:
        static ICustomAeAlgo* getInstance()
        {
            static CustomAeAlgoDev<eSensorDev> singleton;
            return &singleton;
        }

        CustomAeAlgoDev(): ICustomAeAlgo(eSensorDev) {}
        virtual ~CustomAeAlgoDev() {}
};

ICustomAeAlgo* ICustomAeAlgo::getInstance(ESensorDev_T sensor)
{
    switch (sensor)
    {
        default:                    // default use Main Sensor
        case ESensorDev_Main:       // Main Sensor
            return  CustomAeAlgoDev<ESensorDev_Main>::getInstance();
        case ESensorDev_MainSecond: // Main Second Sensor
            return  CustomAeAlgoDev<ESensorDev_MainSecond>::getInstance();
        case ESensorDev_Sub:        // Sub Sensor
            return  CustomAeAlgoDev<ESensorDev_Sub>::getInstance();
        case ESensorDev_SubSecond:  // Sub Second Sensor
            return  CustomAeAlgoDev<ESensorDev_SubSecond>::getInstance();
    }
}

ICustomAeAlgo::ICustomAeAlgo(ESensorDev_T eSensorDev)
{}

ICustomAeAlgo::~ICustomAeAlgo()
{}

void ICustomAeAlgo::CUS_AE_Core_Init(AE_CORE_INIT *in)
{
    ALOGD("[CUS_AE_Core_Init]");
}

static unsigned int g_frm_cnt = 0;
void ICustomAeAlgo::CUS_AE_Core_Ctrl(AE_CTRL_ENUM id, void *ctrl_in, void *ctrl_out)
{
    AE_CORE_CTRL_CCU_VSYNC_INFO *vsync_info = (AE_CORE_CTRL_CCU_VSYNC_INFO *)ctrl_in;
    AE_CORE_CTRL_RUN_TIME_INFO *run_time_info = (AE_CORE_CTRL_RUN_TIME_INFO *)ctrl_in;
    AE_CORE_CTRL_GET_EXIF_PTR *exif_ptr = (AE_CORE_CTRL_GET_EXIF_PTR *)ctrl_out;
    strAECustomParam *CustomAEParam = (strAECustomParam *)(run_time_info->pCusAEParam);

    ALOGD("[CUS_AE_Core_Ctrl] + ENUM id: %d", id);
    switch (id)
    {
        case AE_CTRL_SET_VSYNC_INFO:
            // vsync_info
            g_frm_cnt = vsync_info->u4MagicNo;
            ALOGD( "[CUS_AE_Core_Ctrl] i4AEMaxFps:%d \n", vsync_info->i4AEMaxFps, g_frm_cnt); // for test
            break;
        case AE_CTRL_SET_RUN_TIME_INFO:
            // run_time_info

            // verify Custom AE Tuning param
            // TO-DO -> currently CustomAEParam is nullptr
            //ALOGD("[CUS_AE_Core_Ctrl] Input Custom Tuning Param u4AECustomVersion:%d\n", CustomAEParam->CustomAEInfo.u4AECustomVersion);
            break;
        case AE_CTRL_GET_EXIF_PTR:
            // exif_ptr
            break;
        default:
            break;
    }
    ALOGD("[CUS_AE_Core_Ctrl] -");
}

#include <fstream>
#include <string>
#include <cutils/properties.h>
void ICustomAeAlgo::CUS_AE_Core_Main(AE_CORE_MAIN_IN *in, AE_CORE_MAIN_OUT *out)
{
    AE_CORE_MAIN_IN* pCustData = (AE_CORE_MAIN_IN*)in;
    CCU_AE_STAT* pCustStatData = (CCU_AE_STAT*)in->pCustStatData;
    /*
    // verify 120x90 statistics
    for (int i = 0; i < 120*90; i+=120)
    {
        // sample for test
        ALOGD("[CUS_AE_Core_Main] statistics%5d~%5d:%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n", i, i+15,
           pCustStatData->y[i], pCustStatData->y[i+1], pCustStatData->y[i+2], pCustStatData->y[i+3],
           pCustStatData->y[i+4], pCustStatData->y[i+5], pCustStatData->y[i+6], pCustStatData->y[i+7],
           pCustStatData->y[i+8], pCustStatData->y[i+9], pCustStatData->y[i+10], pCustStatData->y[i+11],
           pCustStatData->y[i+12], pCustStatData->y[i+13], pCustStatData->y[i+14], pCustStatData->y[i+15]);
    }
    // verify 256bin histogram
    for (int i = 0; i < 256; i+=16)
    {
        ALOGD("[CUS_AE_Core_Main] Input Histogram%3d~%3d:%5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d %5d\n", i, i+15,
           pCustStatData->FullYHist[i], pCustStatData->FullYHist[i+1], pCustStatData->FullYHist[i+2], pCustStatData->FullYHist[i+3],
           pCustStatData->FullYHist[i+4], pCustStatData->FullYHist[i+5], pCustStatData->FullYHist[i+6], pCustStatData->FullYHist[i+7],
           pCustStatData->FullYHist[i+8], pCustStatData->FullYHist[i+9], pCustStatData->FullYHist[i+10], pCustStatData->FullYHist[i+11],
           pCustStatData->FullYHist[i+12], pCustStatData->FullYHist[i+13], pCustStatData->FullYHist[i+14], pCustStatData->FullYHist[i+15]);
    }
    */

    ALOGD("[%s()] u4CurHDRRatio:%d g_frm_cnt:%d\n", __FUNCTION__, pCustData->u4CurHDRRatio, g_frm_cnt);

    MINT32 i4DebugEnable = property_get_int32("vendor.debug.cust.ae.enable", 0);
    if (i4DebugEnable)
    {
        std::ofstream file;
        std::string filename;
        filename = "/data/vendor/camera_dump/statR" + std::to_string(g_frm_cnt) + "_" + std::to_string(pCustStatData->stat_width) + "x" + std::to_string(pCustStatData->stat_height) + ".raw";

        file.open(filename.c_str(), std::ios::binary);
        if (file)
        {
            ALOGD("[%s()] Save to %s\n", __FUNCTION__, filename.c_str());        
            file.write((const char*)pCustStatData->r, pCustStatData->stat_width * pCustStatData->stat_height * sizeof(MUINT16));
            file.close();
        }
        else
        {
            ALOGD("[%s()] Fail to open %s\n", __FUNCTION__, filename.c_str());
        }

        filename = "/data/vendor/camera_dump/statY" + std::to_string(g_frm_cnt) + "_" + std::to_string(pCustStatData->stat_width) + "x" + std::to_string(pCustStatData->stat_height) + ".raw";
        file.open(filename.c_str(), std::ios::binary);
        if (file)
        {
            ALOGD("[%s()] Save to %s\n", __FUNCTION__, filename.c_str());        
            file.write((const char*)pCustStatData->y, pCustStatData->stat_width * pCustStatData->stat_height * sizeof(MUINT16));
            file.close();
        }
        else
        {
            ALOGD("[%s()] Fail to open %s\n", __FUNCTION__, filename.c_str());
        }
    }

    // temp for test
    out->u4CWValue = 102;
    out->RealOutput.EvSetting.u4Eposuretime = 30000;
    out->RealOutput.EvSetting.u4AfeGain = 2048;
    out->RealOutput.EvSetting.u4IspGain = 4096;
    out->RealOutput.u4ISO = 200;
    out->RealOutput.u4Index = 100;
    out->RealOutput.u4IndexF = 300;
    ALOGD("[CUS_AE_Core_Main]");
}

void ICustomAeAlgo::CUS_AE_Core_Reset(void)
{
    ALOGD("[CUS_AE_Core_Reset]");
}

void ICustomAeAlgo::CUS_AE_Core_Uninit(void)
{
    ALOGD("[CUS_AE_Core_Uninit]");
}

void ICustomAeAlgo::CUS_AE_Core_ISPInfo(AE_INFO_T &ae_info)
{
    ALOGD("[CUS_AE_Core_ISPInfo]");
}

void ICustomAeAlgo::CUS_AE_Core_DebugInfo(MVOID* exif_info)
{
    AE_DEBUG_DATA_CUST_T *debug_info = (AE_DEBUG_DATA_CUST_T *)exif_info;    
    ALOGD("[CUS_AE_Core_DebugInfo] Size:%d", debug_info->u4Size);
}

