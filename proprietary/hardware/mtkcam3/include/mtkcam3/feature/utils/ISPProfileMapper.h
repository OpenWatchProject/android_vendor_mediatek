/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

 /**
 * @file ISPProfileMapping.h
 * @brief Manage the ISP profile mapping from feature/sensor combination/stages
 *
 */

#ifndef _ISP_PROFILE_MAPPER_HEADER_H_
#define _ISP_PROFILE_MAPPER_HEADER_H_

// Standard C header file
#include <tuple>
#include <map>
#include <unordered_set>
// Android system/core header file

// mtkcam custom header file
#include <camera_custom_isp_profile_mapping.h>
// mtkcam global header file
#include <mtkcam/utils/metadata/IMetadata.h>
// Module header file

// Local header file

using std::tuple;
using std::map;
using NSIspTuning::EIspProfile_T;
using NSCam::IMetadata;

enum EMappingItemKeyType
{
    eMappingKey_NonValid,
    eMappingKey_Proprietary,
    eMappingKey_ISPHIDL
};

struct ProfileMapperKey
{
    EMappingItemKeyType type;
    ProfileMappingItemKeys itemKey;
};

/**
 * @brief ISP Profile Mapper - for each feature & scenario
 */
class ISPProfileMapper
{
public:
    static ISPProfileMapper* getInstance();
    virtual ~ISPProfileMapper() {};
private:
    ISPProfileMapper();
    void _init();
public:

    /**
     * @brief query the current mapping key of profile mapper
     * @param [in] pAppMeta app meta
     * @param [in] pInHalMeta hal meta
     * @param [in] scenario query scenario
     * @param [in] isDualMode dual cam mode or not
     * @param [in] sensorCom sensor combination
     * @return
     * - mapping key of isp profile mapper
     */
    const ProfileMapperKey queryMappingKey(
            IMetadata* pAppMeta,
            IMetadata* pInHalMeta,
            MBOOL isPhysical,
            EProfileMappinScenario scenario,
            ESensorCombination sensorCom = eSensorComb_Single
        );
     /**
     * @brief remove the feature after query
     * @param [in] featureID feature id to remove
     * @param [in/out] rMappingKey mapping key for isp profile mapper
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure, not found any profile
     */
    MBOOL removeFeature(MUINT64 featureID, ProfileMapperKey& rMappingKey);
    /**
     * @brief perform 3dnr hal to generate tuning data
     * @param [in] key query key item
     * @param [in] stage mapping stage
     * @param [out] query profile
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure, not found any profile
     */
    bool mappingProfile(
            const ProfileMapperKey& key,
            const EProfileMappingStages& stage,
            EIspProfile_T& profile
        );
    /**
     * @brief Check this platform is suporrted ISPProfileMapper or not
     * @return
     * - MTRUE indicates support.
     * - MFALSE indicates not-support
     */
    bool isSupported();

private:
    typedef tuple<MUINT64, ESensorCombination, EProfileMappinScenario> MappingKey;
    typedef map<EProfileMappingStages, EIspProfile_T> StageProfileMap;
    std::map<MappingKey, StageProfileMap> mMappingItems;
    std::map<MappingKey, StageProfileMap> mMappingItems_ISPHIDL;
    MUINT64 miKeyFeatures = 0;
};

#endif
