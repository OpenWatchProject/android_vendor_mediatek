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

#include "RfxFragmentEncoder.h"
#include "RfxRilUtils.h"

#define RFX_LOG_TAG "RfxFragEnc"

String8 RfxFragmentData::toString() const {
    return String8::format("mVersion: %d, mClientId: %d, mConfig: %d, \
            mLength: %zu", mVersion, mClientId, mConfig, mLength);
}

RfxFragmentEncoder *RfxFragmentEncoder::sSelf = NULL;

RfxFragmentEncoder::RfxFragmentEncoder() {
}

void RfxFragmentEncoder::init() {
    sSelf = new RfxFragmentEncoder();
}

unsigned char* RfxFragmentEncoder::encodeHeader(RfxFragmentData data) {
    unsigned char* header;
    header = (unsigned char*)calloc(HEADER_SIZE+1, sizeof(char));
    if (header == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "OOM");
        return NULL;
    }
    char log[100] = {0};
    char *tmpLog = NULL;

    int tmpClientId;
    // start flag
    header[0] =  FRAME_START_FLAG;
    header[1] =  FRAME_START_FLAG;

    // version
    header[2] =  VERSION;

    // config
    header[3] =  (char)data.getClientId();
    header[4] =  (char)data.getConfig();

    // data length
    RFX_LOG_D(RFX_LOG_TAG, " encodeHeader len: %zu", data.getDataLength());
    header[5] =  data.getDataLength() & 0x00FF;
    header[6] = (char)((data.getDataLength() & 0xFF00) >> 8);

    // end flag
    header[7] =  FRAME_END_FLAG;
    header[8] =  '\0';

    strncpy(log, "header:", 7);
    for (size_t i=0; i<HEADER_SIZE; i++) {
        asprintf(&tmpLog, "%02X", header[i]);
        strncat(log, tmpLog, strlen(tmpLog));
        free(tmpLog);
        tmpLog = NULL;
    }

    RFX_LOG_D(RFX_LOG_TAG, "%s", log);
    return header;
}

RfxFragmentData RfxFragmentEncoder::decodeHeader(unsigned char *header)
{
    unsigned char *local_readp = header;

    size_t startCount = 0;
    while (startCount < FRAME_START_FLAG_SIZE) {
        if (*(local_readp) == FRAME_START_FLAG) {
            RFX_LOG_D(RFX_LOG_TAG, "find start flag");
            local_readp++;
            startCount++;
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "ERROR: can not find start flag");
            //RFX_ASSERT(0);
            return RfxFragmentData();
        }
    }

    unsigned char version = *(local_readp++);
    unsigned char clientId = *(local_readp++);
    unsigned char config = *(local_readp++);
    unsigned char length = *(local_readp++);
    unsigned char lengthExt = *(local_readp++);
    size_t dataLength = (size_t)length + ((size_t)lengthExt << 8);
    unsigned char endFlag = *(local_readp++);
    if (endFlag == FRAME_END_FLAG) {
        RFX_LOG_D(RFX_LOG_TAG, "find end flag");
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "ERROR: can not find end flag");
        //RFX_ASSERT(0);
        return RfxFragmentData();
    }

    RFX_LOG_D(RFX_LOG_TAG, " decodeHeader version: %d, clientId:%d, config:%d, length: %zu",
            version, clientId, config, dataLength);
    RfxFragmentData data((int)version, clientId, config, dataLength);
    return data;
}
