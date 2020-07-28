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
#ifndef __MFLLFEATUREFEED_H__
#define __MFLLFEATUREFEED_H__


#include <mtkcam3/feature/mfnr/IMfllCore.h>
#include <mtkcam3/feature/mfnr/IMfllEventListener.h>


/* Feed path */
#define MFLL_FEED_DEFAULT_PATH          "/sdcard/mfnr/feed/"

/* Feed pattern */
#define MFLL_FEED_PATTERN_WIDTH         "{width}"
#define MFLL_FEED_PATTERN_HEIGHT        "{height}"
#define MFLL_FEED_PATTERN_INDEX         "{##}"
#define MFLL_FEED_PATTERN_RESOLUTION    MFLL_FEED_PATTERN_WIDTH "x" MFLL_FEED_PATTERN_HEIGHT

/* Feed mechanism (MEMC stage) */
// {{{
// this string may be: /sdcard/mfnr/feed/memc_ref__{width}x{height}.y
#define MFLL_FEED_ME_IN_BASE    MFLL_FEED_DEFAULT_PATH "me_base__"     MFLL_FEED_PATTERN_RESOLUTION ".y"
#define MFLL_FEED_ME_IN_REF     MFLL_FEED_DEFAULT_PATH "me_ref__"      MFLL_FEED_PATTERN_RESOLUTION "_" MFLL_FEED_PATTERN_INDEX ".y"
#define MFLL_FEED_MC_IN_REF     MFLL_FEED_DEFAULT_PATH "mc_ref__"      MFLL_FEED_PATTERN_RESOLUTION "_" MFLL_FEED_PATTERN_INDEX ".i422"
// }}}

/* Feed mechanism (MFB stage) */
// {{{
// this string may be: /sdcard/mfnr/feed/mfb_base_in__{width}x{height}_{##}.yuy2
#define MFLL_FEED_MFB_IN_BASE   MFLL_FEED_DEFAULT_PATH "mfb_base_in__" MFLL_FEED_PATTERN_RESOLUTION "_" MFLL_FEED_PATTERN_INDEX ".yuy2"
#define MFLL_FEED_MFB_IN_REF    MFLL_FEED_DEFAULT_PATH "mfb_ref_in__"  MFLL_FEED_PATTERN_RESOLUTION "_" MFLL_FEED_PATTERN_INDEX ".yuy2"
#define MFLL_FEED_MFB_IN_COEF   MFLL_FEED_DEFAULT_PATH "mfb_conf__"    MFLL_FEED_PATTERN_RESOLUTION "_" MFLL_FEED_PATTERN_INDEX ".y"
#define MFLL_FEED_MFB_IN_MCMV   MFLL_FEED_DEFAULT_PATH "mfb_mcmv__"    MFLL_FEED_PATTERN_RESOLUTION "_" MFLL_FEED_PATTERN_INDEX ".sta32"
#define MFLL_FEED_MFB_IN_WEIGHT MFLL_FEED_DEFAULT_PATH "mfb_wt_in__"   MFLL_FEED_PATTERN_RESOLUTION "_" MFLL_FEED_PATTERN_INDEX ".y"
// }}}

/* Feed mechanism (MIX stage) */
// {{{
// this string may be: /sdcard/mfnr/feed/mix_blended_in__{width}x{height}.yuy2
#define MFLL_FEED_MIX_IN_BLD    MFLL_FEED_DEFAULT_PATH "mix_bld_in__"  MFLL_FEED_PATTERN_RESOLUTION ".yuy2"
#define MFLL_FEED_MIX_IN_REF    MFLL_FEED_DEFAULT_PATH "mix_ref_in__"  MFLL_FEED_PATTERN_RESOLUTION ".yuy2"
#define MFLL_FEED_MIX_IN_WEIGHT MFLL_FEED_DEFAULT_PATH "mix_wt_in__"   MFLL_FEED_PATTERN_RESOLUTION ".y"
#define MFLL_FEED_MIX_IN_DCESO  MFLL_FEED_DEFAULT_PATH "mix_dceso_in__"   MFLL_FEED_PATTERN_RESOLUTION ".sta32"

// }}}

/* Feed mechanism (SF stage) */
// {{{
#define MFLL_FEED_SF_IN         MFLL_FEED_DEFAULT_PATH "sf_in__"       MFLL_FEED_PATTERN_RESOLUTION ".yuy2"
// }}}

namespace mfll {

/**
 *  Feed Mechanism
 *
 *  This plug-in provides a mechanism to feed frame data for every stage.
 */
class MfllFeatureFeed : public IMfllEventListener {
public:
    typedef struct _FeedFlag {
        int bfbld;
        int bss;
        int memc;
        int sf;
        int mfb;
        int mix;
        int jpeg;
        _FeedFlag() : bfbld(0), bss(0), memc(0), sf(0), mfb(0), mix(0), jpeg(0) {}
        ~_FeedFlag() = default;
    } FeedFlag;

public:
    MfllFeatureFeed(void);


protected:
    virtual ~MfllFeatureFeed(void);


/* implementation */
public:
    virtual void onEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL);
    virtual void doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL);
    virtual vector<enum EventType> getListenedEventTypes(void);


/* interface */
public:
    inline void setFeedFlag(const FeedFlag &flags)
    { m_feedFlag = flags; }


/* attributes */
private:
    FeedFlag m_feedFlag;


}; /* class MfllFeatureFeed */
}; /* namespace mfll */
#endif//__MFLLFEATUREFEED_H__
