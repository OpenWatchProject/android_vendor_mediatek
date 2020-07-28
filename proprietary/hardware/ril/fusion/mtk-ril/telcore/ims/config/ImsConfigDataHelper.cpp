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

#include "ImsConfigDataHelper.h"
#include "RtcImsConfigDef.h"
#include "telephony/mtk_ims_config.h"

using std::vector;
using std::shared_ptr;
using std::make_shared;
using std::map;
using std::string;

map<int,ConfigSettings> ImsConfigDataHelper::sConfigData = {
#include "ImsConfigData.h"
};

vector<shared_ptr<ConfigValue>> ImsConfigDataHelper::getConfigData(int operatorId) {

    vector <ConfigValue> opData;
    if (operatorId == Operator::OP_06) {
        opData = {
        #include "op/Op06ImsConfigData.h"
        };
    } else if (operatorId == Operator::OP_08) {
        opData = {
        #include "op/Op08ImsConfigData.h"
        };
    } else if (operatorId == Operator::OP_12) {
        opData = {
        #include "op/Op12ImsConfigData.h"
        };
    } else if (operatorId == Operator::OP_236) {
        opData = {
        #include "op/Op236ImsConfigData.h"
        };
    }

    vector<shared_ptr<ConfigValue>> configData;
    map<int, ConfigSettings>::iterator iter;
    iter = sConfigData.begin();
    for (int i = 0; i < (int)sConfigData.size(); i++, iter++) {
        shared_ptr <ConfigValue> data = make_shared<ConfigValue>();
        data->configId = iter->first;
        data->unit = IMS_CONFIG_UNIT_NONE;
        data->provsionValue = IMS_CONFIG_VALUE_NO_DEFAULT;
        for (int j = 0; j < (int)opData.size(); j++) {
            if (data->configId == opData[j].configId) {
                data->unit = opData[j].unit;
                data->provsionValue = opData[j].provsionValue;
            }
        }
        configData.push_back(data);
    }
    return configData;
}

int ImsConfigDataHelper::getConfigDataType(int configId) {
    return sConfigData[configId].mimeType;
}

bool ImsConfigDataHelper::isProvisionToModem(int configId) {
    return !sConfigData[configId].provsionStr.empty();
}

string ImsConfigDataHelper::getConfigProvisionStr(int configId) {
    return sConfigData[configId].provsionStr;
}

int ImsConfigDataHelper::getConfigId(string provisionStr) {
    map<int, ConfigSettings>::iterator iter;
    iter = sConfigData.begin();
    for (int i = 0; i < (int)sConfigData.size(); i++, iter++) {
        if ((iter->second).provsionStr == provisionStr) {
            return iter->first;
        }
    }
    return -1;

};