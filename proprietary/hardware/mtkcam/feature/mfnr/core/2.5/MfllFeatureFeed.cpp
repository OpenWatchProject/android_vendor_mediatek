/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#define LOG_TAG "MfllCore/Feed"

#include "MfllFeatureFeed.h"
#include "MfllLog.h"
#include "MfllCore.h"

#include <string>

using namespace mfll;
using std::vector;
using std::string;


inline std::string
_parseFileName(
        IMfllImageBuffer*   pImg,
        const char*         name,
        int                 index = 0
)
{
    const std::string TOKEN_WIDTH       = MFLL_FEED_PATTERN_WIDTH;
    const std::string TOKEN_HEIGHT      = MFLL_FEED_PATTERN_HEIGHT;
    const std::string TOKEN_INDEX       = MFLL_FEED_PATTERN_INDEX;

    std::string szFilepath = name;

    // replace width and height
    if (szFilepath.find(TOKEN_WIDTH) != std::string::npos) {
        szFilepath.replace(
                szFilepath.find(TOKEN_WIDTH), TOKEN_WIDTH.length(), std::to_string(pImg->getAlignedWidth()));
    }
    if (szFilepath.find(TOKEN_HEIGHT) != std::string::npos) {
        szFilepath.replace(
                szFilepath.find(TOKEN_HEIGHT), TOKEN_HEIGHT.length(), std::to_string(pImg->getAlignedHeight()));
    }
    if (szFilepath.find(TOKEN_INDEX) != std::string::npos) {
        szFilepath.replace(
                szFilepath.find(TOKEN_INDEX), TOKEN_INDEX.length(), std::to_string(index));
    }

    return szFilepath;
}


inline void
_read_image(
        IMfllImageBuffer*   pImg,
        const char*         name,
        int                 index
)
{
    if ( pImg == nullptr ) {
        mfllLogE("Feed mechanism enabled, but input image buffer is null. "\
                "file name is: %s", name);
        return;
    }

    auto _szFilepath  = _parseFileName( pImg, name, index );
    auto _err = pImg->loadFile( _szFilepath.c_str() );
    if (_err != MfllErr_Ok) {
        mfllLogW("read file %s failed, ignored", _szFilepath.c_str());
    }
    else {
        mfllLogD("read ok: %s", _szFilepath.c_str());
        pImg->syncCache();
    }
}

static std::vector<enum EventType> EVENTS_TO_LISTEN_INITIALIZER(void)
{
    std::vector<enum EventType> v;
    #define LISTEN(x) v.push_back(x)
    LISTEN(EventType_CaptureRaw);
    LISTEN(EventType_CaptureRrzo);
    LISTEN(EventType_CaptureYuvQ);
    LISTEN(EventType_EncodeYuvBase);
    LISTEN(EventType_EncodeYuvGolden);
    LISTEN(EventType_Bss);
    LISTEN(EventType_MotionEstimation);
    LISTEN(EventType_MotionCompensation);
    LISTEN(EventType_Blending);
    LISTEN(EventType_Mixing);
    #undef LISTEN
    return v;
}

static vector<enum EventType> g_eventsToListen = EVENTS_TO_LISTEN_INITIALIZER();


MfllFeatureFeed::MfllFeatureFeed(void)
{
    ::memset(&m_feedFlag, 0, sizeof(m_feedFlag));
}

MfllFeatureFeed::~MfllFeatureFeed(void)
{
}

void MfllFeatureFeed::onEvent(
        enum EventType      t           __attribute__((unused)) ,
        MfllEventStatus_t&  status      __attribute__((unused)) ,
        void*               mfllCore    __attribute__((unused)) ,
        void*               param1      __attribute__((unused)) ,
        void*               param2      __attribute__((unused))
)
{
    auto err = MfllErr_Ok;
    auto index = (int)(long) param1;
    IMfllCore* c = static_cast<IMfllCore*>(mfllCore);


    switch (t) {
    // Stage SF {{{
    case EventType_EncodeYuvGolden:
        if ( m_feedFlag.sf && ( !status.ignore )) {
            IMfllImageBuffer* pImg = nullptr;

            // [in] full size yuv or full raw
            pImg = c->retrieveBuffer(MfllBuffer_BaseYuv).get();
            _read_image(pImg, MFLL_FEED_SF_IN, index);

        }
        break;
    // }}}

    // Stage MFB {{{
    case EventType_Blending:
        if ( m_feedFlag.mfb && (!status.ignore) ) {
            IMfllImageBuffer* pImg = nullptr;

            // [in] base
            pImg = c->retrieveBuffer(MfllBuffer_BaseYuv).get();
            _read_image(pImg, MFLL_FEED_MFB_IN_BASE, index);

            // [in] ref
            pImg = c->retrieveBuffer(MfllBuffer_ReferenceYuv).get();
            _read_image(pImg, MFLL_FEED_MFB_IN_REF, index);

            // [in] coef
            pImg = c->retrieveBuffer(MfllBuffer_ConfidenceMap, index).get();
            _read_image(pImg, MFLL_FEED_MFB_IN_COEF, index);

            // [in] weighting
            if (index > 0) {
                pImg = c->retrieveBuffer(MfllBuffer_WeightingIn).get();
                _read_image(pImg, MFLL_FEED_MFB_IN_WEIGHT, index);
            }

        }
        break;
    // }}}

    // Stage AFBLD (MIX) {{{
    case EventType_Mixing:
        if ( m_feedFlag.mix && (!status.ignore) ) {
            IMfllImageBuffer* pImg = nullptr;

            // [in] blended
            pImg = c->retrieveBuffer(MfllBuffer_BlendedYuv).get();
            _read_image(pImg, MFLL_FEED_MIX_IN_BLD, index);

            // [in] ref
            pImg = c->retrieveBuffer(MfllBuffer_GoldenYuv).get();
            _read_image(pImg, MFLL_FEED_MIX_IN_REF, index);

            // [in] weighting
            pImg = c->retrieveBuffer(MfllBuffer_WeightingOut).get();
            _read_image(pImg, MFLL_FEED_MIX_IN_WEIGHT, index);
        }
        break;
    // }}}

    default: break;
    } // switch
}



void
MfllFeatureFeed::
doneEvent(
        enum EventType      t           __attribute__((unused)) ,
        MfllEventStatus_t&  status      __attribute__((unused)) ,
        void*               mfllCore    __attribute__((unused)) ,
        void*               param1      __attribute__((unused)) ,
        void*               param2      __attribute__((unused))
)
{
    MfllErr err = MfllErr_Ok;
    int index = (int)(long) param1;
    IMfllCore* c = static_cast<IMfllCore*>(mfllCore);


    switch (t) {
    // Stage MEMC {{{
    //
    // MEMC stage is a stage with several parallel operations, we need feed All
    // data before triggering it
    case EventType_Bss:
        if (m_feedFlag.memc) {
            IMfllImageBuffer* pImg = nullptr;
            // get blending number first
            int blend_num = c->getBlendFrameNum();

            // [in] the best frame for ME (Y8)
            pImg = c->retrieveBuffer(MfllBuffer_QYuv, 0).get();
            _read_image(pImg, MFLL_FEED_ME_IN_BASE, 0);

            // read excluding the first one (index 0)
            for (int i = 1; i < blend_num; i++) {
                // [in] the following frames for ME (Y8)
                pImg = c->retrieveBuffer(MfllBuffer_QYuv, i).get();
                _read_image(pImg, MFLL_FEED_ME_IN_REF, (i - 1));

                // [in] the following frames for MC (I422)
                pImg = c->retrieveBuffer(MfllBuffer_FullSizeYuv, i).get();
                _read_image(pImg, MFLL_FEED_MC_IN_REF, (i - 1));
            }
        }
        break;
    // }}}

    default:break;
    }
}


vector<enum EventType>
MfllFeatureFeed::
getListenedEventTypes(void)
{
    return g_eventsToListen;
}
