/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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


#include <camera_nvram_query.h>
#include <camera_custom_nvram.h>

class NvramQueryHelperImpl : public NvramQueryHelper
{
public:

    NvramQueryHelperImpl() {}
    virtual ~NvramQueryHelperImpl() {}

    virtual bool queryIspModuleID(NvramFeatureID fID, uint8_t &outEnum);

    virtual bool queryFeatureNvram(const QueryCfg &input, void* &out);
};

/* Define for getHelper function used */
NvramQueryHelper* getNvramQueryHelperImpl()
{
    static NvramQueryHelperImpl sInst;
    return &sInst;
}

bool NvramQueryHelperImpl::queryIspModuleID(NvramFeatureID fID, uint8_t &outEnum)
{
    switch(fID)
    {
        case NvramFeatureID::CLEARZOOM:
            outEnum = (uint8_t) NSIspTuning::EModule_ClearZoom;
            return true;
        case NvramFeatureID::CALTM:
            outEnum = (uint8_t) NSIspTuning::EModule_CA_LTM;
            return true;
        default:
            return false;
    }
}

bool NvramQueryHelperImpl::queryFeatureNvram(const QueryCfg &input, void* &out)
{
    out = nullptr;
    bool ret = false;
    if(input.featureNVRam == NULL || input.index == NvramQueryHelper::INVALID_INDEX)
        return ret;

    switch(input.fID)
    {
        case NvramFeatureID::CLEARZOOM:
        {
            out = &(((NVRAM_CAMERA_FEATURE_STRUCT*)input.featureNVRam)->ClearZoom[input.index]);
            ret = true;
            break;
        }
        case NvramFeatureID::CALTM:
        {
            out = &(((NVRAM_CAMERA_FEATURE_STRUCT*)input.featureNVRam)->CA_LTM[input.index]);
            ret = true;
            break;
        }
        default:
            break;
    }
    return ret;
}





