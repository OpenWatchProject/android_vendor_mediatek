/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include <string.h>
#include "RfxSetDataProfileData.h"
#include "RfxLog.h"

#define RFX_LOG_TAG "RfxSetDataProfileData"

RFX_IMPLEMENT_DATA_CLASS(RfxSetDataProfileData);

RfxSetDataProfileData::RfxSetDataProfileData(void *data, int length) : RfxBaseData(data, length) {
    m_length = length;
    int num = m_length / sizeof(RIL_MtkDataProfileInfo *);
    if (num == 0 || data == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[%s] The number of DataProfile is %d (if num != 0, means data is null)",
                __FUNCTION__, num);
        return;
    }

    RIL_MtkDataProfileInfo **curPtr = (RIL_MtkDataProfileInfo **) data;
    RIL_MtkDataProfileInfo **tmpPtr = (RIL_MtkDataProfileInfo **) calloc(num, sizeof(RIL_MtkDataProfileInfo *));
    RFX_ASSERT(tmpPtr != NULL);

    for (int i = 0; i < num; i++ ) {
        RIL_MtkDataProfileInfo *dataPtr = (RIL_MtkDataProfileInfo *) calloc(1, sizeof(RIL_MtkDataProfileInfo));
        RFX_ASSERT(dataPtr != NULL);

        tmpPtr[i] = dataPtr;

        //profileId
        dataPtr->profileId = curPtr[i]->profileId;
        //apn
        if ((curPtr[i])->apn != NULL) {
            asprintf(&dataPtr->apn, "%s", curPtr[i]->apn);
        } else {
            asprintf(&dataPtr->apn, "");
        }
        //protocol
        if ((curPtr[i])->protocol != NULL) {
            asprintf(&dataPtr->protocol, "%s", curPtr[i]->protocol);
        } else {
            asprintf(&dataPtr->protocol, "");
        }
        //roamingProtocol
        if ((curPtr[i])->roamingProtocol != NULL) {
            asprintf(&dataPtr->roamingProtocol, "%s", curPtr[i]->roamingProtocol);
        } else {
            asprintf(&dataPtr->roamingProtocol, "");
        }
        //authType
        dataPtr->authType = curPtr[i]->authType;
        //user
        if ((curPtr[i])->user != NULL) {
            asprintf(&dataPtr->user, "%s", curPtr[i]->user);
        } else {
            asprintf(&dataPtr->user, "");
        }
        //password
        if ((curPtr[i])->password != NULL) {
            asprintf(&dataPtr->password, "%s", curPtr[i]->password);
        } else {
            asprintf(&dataPtr->password, "");
        }
        //type
        dataPtr->type = curPtr[i]->type;
        //maxConnsTime
        dataPtr->maxConnsTime = curPtr[i]->maxConnsTime;
        //maxConns
        dataPtr->maxConns = curPtr[i]->maxConns;
        //waitTime
        dataPtr->waitTime = curPtr[i]->waitTime;
        //enabled
        dataPtr->enabled = curPtr[i]->enabled;
        //supportedTypesBitmask
        dataPtr->supportedTypesBitmask = curPtr[i]->supportedTypesBitmask;
        //bearerBitmask
        dataPtr->bearerBitmask = curPtr[i]->bearerBitmask;
        //mtu
        dataPtr->mtu = curPtr[i]->mtu;
        //mvnoType
        if ((curPtr[i])->mvnoType != NULL) {
            asprintf(&dataPtr->mvnoType, "%s", curPtr[i]->mvnoType);
        } else {
            asprintf(&dataPtr->mvnoType, "");
        }
        //mvnoMatchData
        if ((curPtr[i])->mvnoMatchData != NULL) {
            asprintf(&dataPtr->mvnoMatchData, "%s", curPtr[i]->mvnoMatchData);
        } else {
            asprintf(&dataPtr->mvnoMatchData, "");
        }
        //inactiveTimer
        dataPtr->inactiveTimer = curPtr[i]->inactiveTimer;
    }
    m_data = tmpPtr;
}

RfxSetDataProfileData::~RfxSetDataProfileData() {
    RIL_MtkDataProfileInfo** tmpPtr = (RIL_MtkDataProfileInfo **)m_data;
    int num = m_length / sizeof(RIL_MtkDataProfileInfo *);

    if (tmpPtr != NULL) {
        // free memory
        for (int i = 0; i < num; i++) {
            RIL_MtkDataProfileInfo *dataPtr = (RIL_MtkDataProfileInfo *) tmpPtr[i];
            RFX_ASSERT(dataPtr != NULL);
            FREEIF(dataPtr->apn);
            FREEIF(dataPtr->protocol);
            FREEIF(dataPtr->roamingProtocol);
            FREEIF(dataPtr->user);
            FREEIF(dataPtr->password);
            FREEIF(dataPtr->mvnoType);
            FREEIF(dataPtr->mvnoMatchData);
            free(dataPtr);
        }
        free(tmpPtr);
        tmpPtr = NULL;
    }
}
