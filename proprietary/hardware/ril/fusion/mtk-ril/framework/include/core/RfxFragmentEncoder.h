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

#ifndef __RFX_FRAGMENT_ENCODER_H__
#define __RFX_FRAGMENT_ENCODER_H__

#include "RfxLog.h"
#include "rfx_properties.h"
#include "RfxDefs.h"
#include <telephony/mtk_ril.h>
#include "utils/String8.h"

using ::android::String8;

class RfxFragmentData {
    public:
        RfxFragmentData() :
                mVersion(1),
                mClientId(-1),
                mConfig(0),
                mLength(0) {
        }

        RfxFragmentData(int version, int clientId,
                int config, size_t length) :
                mVersion(version),
                mClientId(clientId),
                mConfig(config),
                mLength(length) {
        }

        int getClientId() const {
            return mClientId;
        }

        int getVersion() const {
            return mVersion;
        }

        int getConfig() const {
            return mConfig;
        }

        size_t getDataLength() {
            return mLength;
        }

        String8 toString() const;

    private:
        int mVersion;
        int mClientId;
        int mConfig;
        size_t mLength;
};

class RfxFragmentEncoder {

    public:
        static const unsigned char VERSION= 1;
        static const size_t HEADER_SIZE = 8;
        static const size_t MAX_FRAME_SIZE = 2048;

    private:

        static const unsigned char FRAME_START_FLAG = 0x7F;
        static const unsigned char FRAME_END_FLAG = 0x7F;
        static const size_t FRAME_START_FLAG_SIZE = 2;
        static const size_t FRAME_CLIENT_ID_SIZE = 5;

    private:

        RfxFragmentEncoder();

    public:

        static void init();
        static RfxFragmentData decodeHeader(unsigned char *header);
        static unsigned char* encodeHeader(RfxFragmentData data);

    private:
        static RfxFragmentEncoder *sSelf;
};
#endif
