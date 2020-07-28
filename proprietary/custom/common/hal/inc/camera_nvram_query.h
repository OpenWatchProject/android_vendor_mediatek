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
#ifndef _CAMERA_NVRAM_QUERY_H_
#define _CAMERA_NVRAM_QUERY_H_

#include <stdint.h>

enum class NvramFeatureID : uint8_t
{
    NONE,
    CLEARZOOM,
    CALTM,
    HFG,
    DSDN,
    MAX_NUM,
};

class NvramQueryHelper;
extern NvramQueryHelper* getNvramQueryHelperImpl() __attribute__((weak));

class NvramQueryHelper
{
public:
    static const uint32_t INVALID_INDEX = (uint32_t)-1;
    class QueryCfg
    {
    public:
        NvramFeatureID fID = NvramFeatureID::NONE;
        uint32_t index = INVALID_INDEX;
        void *featureNVRam = nullptr;

        QueryCfg(){}
        QueryCfg(NvramFeatureID _fID, uint32_t _index, void* _featureNVRam)
        : fID(_fID)
        , index(_index)
        , featureNVRam(_featureNVRam)

        {}
    };

    static NvramQueryHelper* getHelper()
    {
        static NvramQueryHelper sDefault;
        return (getNvramQueryHelperImpl != (NvramQueryHelper*(*)(void))0) ? getNvramQueryHelperImpl() : &sDefault;
    }

    NvramQueryHelper() {}
    virtual ~NvramQueryHelper() {}

    virtual bool queryIspModuleID(NvramFeatureID fID, uint8_t &outEnum)
    {
        (void)fID;
        (void)outEnum;
        return false;
    }

    virtual bool queryFeatureNvram(const QueryCfg &input, void* &out)
    {
        (void)input;
        out = nullptr;
        return false;
    }
};

#endif /* _CAMERA_NVRAM_QUERY_H_ */

