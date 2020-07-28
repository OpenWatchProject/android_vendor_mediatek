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
#define LOG_TAG "isp_mgr_abf"

#include <aaa_log.h>
#include <nvbuf_util.h>
#include <isp_tuning.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <paramctrl.h>
#include <camera_custom_nvram.h>


using namespace NSIspTuning;

namespace NSIspTuningv3
{

class AbfTuningProxy
{
public:
    AbfTuningProxy(ESensorDev_T sensorDev) :
            mSensorDev(sensorDev), pIspTuningCustom(NULL)
    {
    }

    ~AbfTuningProxy() {
        if (pIspTuningCustom)
            pIspTuningCustom->destroyInstance();
    }

    int getTuningIndex(MINT32 openId, int inputIso) {
        if (pIspTuningCustom == NULL) {
            pIspTuningCustom = IspTuningCustom::createInstance(mSensorDev, openId);
        }

        int remappedIso = pIspTuningCustom->remap_ISO_value(inputIso);

        Paramctrl* pParamctrl = Paramctrl::getInstance(mSensorDev, openId);
        int tuningIndex = pParamctrl->map_ISO_value2index(remappedIso);
        pParamctrl->destroyInstance();

        if (tuningIndex < 0)
            tuningIndex = 0;
        else if (tuningIndex >= NVRAM_SWABF_TBL_NUM)
            tuningIndex = NVRAM_SWABF_TBL_NUM - 1;

        return tuningIndex;
    }

    void *getTuningData(MINT32 openId, int iso) {
        int tuningIndex = getTuningIndex(openId, iso);

        const NVRAM_CAMERA_FEATURE_STRUCT* pNvram = NULL;
        int err = NvBufUtil::getInstance().getBufAndRead(
                CAMERA_NVRAM_DATA_FEATURE, mSensorDev, (void*&)pNvram);

        if (err != 0) {
            MY_LOG("%s(%d, %d): getBufAndRead() failed: %d", __func__, openId, iso, err);
            return NULL;
        }

        return static_cast<void*>(
            &(const_cast<NVRAM_CAMERA_FEATURE_STRUCT*>(pNvram)->abf[tuningIndex])
        );
    }

private:
    ESensorDev_T        mSensorDev;
    IspTuningCustom*    pIspTuningCustom;
};


template <ESensorDev_T sensorDev>
class AbfTuningProxySinglton
{
private:
    static AbfTuningProxy *sInstance;

public:
    static AbfTuningProxy *get() {
        if (sInstance == NULL)
            sInstance = new AbfTuningProxy(sensorDev);
        return sInstance;
    }
};

template <ESensorDev_T sensorDev>
AbfTuningProxy* AbfTuningProxySinglton<sensorDev>::sInstance = NULL;


void *abfGetTuningData(ESensorDev_T sensorDev, MINT32 const openId, int iso)
{
    AbfTuningProxy *proxy = NULL;

    switch (sensorDev) {
    case ESensorDev_Main:
        proxy = AbfTuningProxySinglton<ESensorDev_Main>::get();
        break;
    case ESensorDev_MainSecond:
        proxy = AbfTuningProxySinglton<ESensorDev_MainSecond>::get();
        break;
    case ESensorDev_Sub:
        proxy = AbfTuningProxySinglton<ESensorDev_Sub>::get();
        break;
    case ESensorDev_SubSecond:
        proxy = AbfTuningProxySinglton<ESensorDev_SubSecond>::get();
        break;
    default:
        proxy = AbfTuningProxySinglton<ESensorDev_Main>::get();
        break;
    }

    return proxy->getTuningData(openId, iso);
}

};

