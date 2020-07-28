/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "PQDC_TUNING_JNI"

#define MTK_LOG_ENABLE 1
#include <jni.h>

#include <utils/threads.h>
#include <log/log.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include "cust_color.h"
#include "cust_tdshp.h"
#include "PQServiceCommon.h"
#include "PQAshmemProxy.h"
#include <vendor/mediatek/hardware/pq/2.0/IPictureQuality.h>

using namespace android;
using vendor::mediatek::hardware::pq::V2_0::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;

DISP_PQ_DC_PARAM dcparam;

int drvID = -1;

int PQDCOffsetMapping[40] = {0};
int isPQDCMappingTableInit = 0;

// ADL FW registers (Partial only for tuning)
struct ADLReg_Partial {
  // On & Off Switch
  uint32_t bALOnOff;
  uint32_t bBSOnOff;
  uint32_t bWSOnOff;
  uint32_t bStrongBSOffset;
  uint32_t bStrongWSOffset;
  uint32_t bBSAdaptiveLmt;  // Blending with Dynamic Ratio according to BS Index
  uint32_t bWSAdaptiveLmt;  // Blending with Dynamic Ratio according to WS Index

  // Full Range Setup
  uint32_t bLevelLowBnd;
  uint32_t bLevelHighBnd;

  // Scene Change Param
  uint32_t bADLSceneChange;
  uint32_t bForceScenceChangeEn;  // For debug
  uint32_t bForceScenceChange2En;  // To control scene change
  uint32_t bForceScenceChange2;  // Scene change type
  uint32_t bMaxDiffThreshold;
  uint32_t bTotalDiffThreshold;
  uint32_t bAPLDiffThreshold;

  // Histogram FIR Weight
  uint32_t bADLWeight1;
  uint32_t bADLWeight2;
  uint32_t bADLWeight3;

  // Mid Param
  uint32_t bMIDACGain;
  uint32_t bMIDDCGain;

  // BS Basic Param
  uint32_t bBSDCGain;
  uint32_t bBSACGain;
  uint32_t bBSLevel;
  uint32_t bBSRatio;

  // BS Offset Param
  uint32_t bBSOffsetThL;  // Input threshold
  uint32_t bBSOffsetThH;  // Input threshold
  uint32_t bBSOffsetL;  // Output threshold
  uint32_t bBSOffsetH;  // Output threshold

  // WS Basic Param
  uint32_t bWSDCGain;
  uint32_t bWSACGain;
  uint32_t bWSLevel;
  uint32_t bWSRatio;

  // WS Offset Param
  uint32_t bWSOffsetThL;  // Input threshold
  uint32_t bWSOffsetThH;  // Input threshold
  uint32_t bWSOffsetL;  // Output threshold
  uint32_t bWSOffsetH;  // Output threshold

  // YGamma
  uint32_t bYGammaIdx1;
  uint32_t bYGammaIdx2;

  // IIR
  uint32_t bIIRStrength;

  // Spike Fallback Protection
  uint32_t bSpikeWeightLowBnd;
  uint32_t bSpikeThLow;
  uint32_t bSpikeThHigh;
  uint32_t bSpikeBlendmethod;
  uint32_t bIIRStrengthSpike;

  // Skin processing
  uint32_t bSkinBlendmethod;
  uint32_t bIIRStrengthSkin;
  uint32_t bSkinWgtSlope;
};

#define DCRegOffset(field) (unsigned int)((char*)&( ((ADLReg_Partial*)(0))->field) - (char*)(0))

#define UNUSED(expr) do { (void)(expr); } while (0)

static int getPQDCOffset(PQ_DC_index_t a_eIndex)
{
    if (isPQDCMappingTableInit == 0)
    {
        PQDCOffsetMapping[BlackEffectEnable] = DCRegOffset(bBSOnOff);
        PQDCOffsetMapping[WhiteEffectEnable] = DCRegOffset(bWSOnOff);
        PQDCOffsetMapping[StrongBlackEffect] = DCRegOffset(bStrongBSOffset);
        PQDCOffsetMapping[StrongWhiteEffect] = DCRegOffset(bStrongWSOffset);
        PQDCOffsetMapping[AdaptiveBlackEffect] = DCRegOffset(bBSAdaptiveLmt);
        PQDCOffsetMapping[AdaptiveWhiteEffect] = DCRegOffset(bWSAdaptiveLmt);
        PQDCOffsetMapping[ScenceChangeOnceEn] = DCRegOffset(bForceScenceChangeEn);
        PQDCOffsetMapping[ScenceChangeControlEn] = DCRegOffset(bForceScenceChange2En);
        PQDCOffsetMapping[ScenceChangeControl] = DCRegOffset(bForceScenceChange2);
        PQDCOffsetMapping[ScenceChangeTh1] = DCRegOffset(bMaxDiffThreshold);
        PQDCOffsetMapping[ScenceChangeTh2] = DCRegOffset(bTotalDiffThreshold);
        PQDCOffsetMapping[ScenceChangeTh3] =  DCRegOffset(bAPLDiffThreshold);
        PQDCOffsetMapping[ContentSmooth1] = DCRegOffset(bADLWeight1);
        PQDCOffsetMapping[ContentSmooth2] = DCRegOffset(bADLWeight2);
        PQDCOffsetMapping[ContentSmooth3] = DCRegOffset(bADLWeight3);
        PQDCOffsetMapping[MiddleRegionGain1] = DCRegOffset(bMIDDCGain);
        PQDCOffsetMapping[MiddleRegionGain2] = DCRegOffset(bMIDACGain);
        PQDCOffsetMapping[BlackRegionGain1] = DCRegOffset(bBSDCGain);
        PQDCOffsetMapping[BlackRegionGain2] = DCRegOffset(bBSACGain);
        PQDCOffsetMapping[BlackRegionRange] = DCRegOffset(bBSLevel);
        PQDCOffsetMapping[BlackEffectLevel] = DCRegOffset(bBSRatio);
        PQDCOffsetMapping[BlackEffectParam1] = DCRegOffset(bBSOffsetThL);
        PQDCOffsetMapping[BlackEffectParam2] = DCRegOffset(bBSOffsetThH);
        PQDCOffsetMapping[BlackEffectParam3] = DCRegOffset(bBSOffsetL);
        PQDCOffsetMapping[BlackEffectParam4] = DCRegOffset(bBSOffsetH);
        PQDCOffsetMapping[WhiteRegionGain1] = DCRegOffset(bWSDCGain);
        PQDCOffsetMapping[WhiteRegionGain2] = DCRegOffset(bWSACGain);
        PQDCOffsetMapping[WhiteRegionRange] = DCRegOffset(bWSLevel);
        PQDCOffsetMapping[WhiteEffectLevel] = DCRegOffset(bWSRatio);
        PQDCOffsetMapping[WhiteEffectParam1] = DCRegOffset(bWSOffsetThL);
        PQDCOffsetMapping[WhiteEffectParam2] = DCRegOffset(bWSOffsetThH);
        PQDCOffsetMapping[WhiteEffectParam3] = DCRegOffset(bWSOffsetL);
        PQDCOffsetMapping[WhiteEffectParam4] = DCRegOffset(bWSOffsetH);
        PQDCOffsetMapping[ContrastAdjust1] = DCRegOffset(bYGammaIdx1);
        PQDCOffsetMapping[ContrastAdjust2] = DCRegOffset(bYGammaIdx2);
        PQDCOffsetMapping[DCChangeSpeedLevel] = DCRegOffset(bIIRStrength);
        PQDCOffsetMapping[ProtectRegionEffect] = DCRegOffset(bSkinBlendmethod);
        PQDCOffsetMapping[DCChangeSpeedLevel2] = DCRegOffset(bIIRStrengthSkin);
        PQDCOffsetMapping[ProtectRegionWeight] = DCRegOffset(bSkinWgtSlope);
        PQDCOffsetMapping[DCEnable] = DCRegOffset(bALOnOff);

        isPQDCMappingTableInit = 1;
    }

    return PQDCOffsetMapping[a_eIndex];
}

static int getIndex(PQ_DC_index_t a_eIndex)
{
    int32_t value = 0;

    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGE("[PQDC_JNI] failed to get HW service");
        return 0;
    }

    android::hardware::Return<Result> ret = service->setTuningField(MOD_DC_SWREG, 0xffff, PROXY_TUNING_READING);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("IPictureQuality::setTuningField failed!");
    }
    android::hardware::Return<void> ret_2 = service->getTuningField(MOD_DC_SWREG, getPQDCOffset(a_eIndex),
        [&] (Result retval, int32_t retIndex) {
        if (retval == Result::OK) {
            value = retIndex;
        }
    });
    if (!ret_2.isOk()) {
        ALOGE("IPictureQuality::getTuningField failed!");
    }

    return value;
}

static jboolean setIndex(PQ_DC_index_t a_eIndex , unsigned int a_u4Index)
{
    ALOGD("[PQDC_JNI] setIndex[%d], value[%u]", (int)a_eIndex, a_u4Index);
    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGE("[PQDC_JNI] failed to get HW service");
        return 0;
    }

    android::hardware::Return<Result> ret = service->setTuningField(MOD_DC_SWREG, 0xffff, PROXY_TUNING_OVERWRITTEN);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("IPictureQuality::setTuningField failed!");
    }
    ret = service->setTuningField(MOD_DC_SWREG, getPQDCOffset(a_eIndex), a_u4Index);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("IPictureQuality::setTuningField failed!");
    }
    ret = service->setTuningField(MOD_DC_SWREG, 0xfffe, 0);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("IPictureQuality::setTuningField failed!");
    }

    return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
static jboolean setTuningMode(JNIEnv *env, jobject thiz, int mode)
{
    ALOGD("[PQDC_JNI] setTuningMode[%d]", mode);

    sp<IPictureQuality> service = IPictureQuality::getService();
    if (service == nullptr) {
        ALOGE("[PQDC_JNI] failed to get HW service");
        return 0;
    }

    if (mode)
    {
        android::hardware::Return<Result> ret =
            service->setTuningField(MOD_DC_SWREG, 0xffff, PROXY_TUNING_READING);
        if (!ret.isOk()) {
            ALOGE("IPictureQuality::setTuningField failed!");
        }
    }
    else
    {
        android::hardware::Return<Result> ret =
            service->setTuningField(MOD_DC_SWREG, 0xffff, PROXY_TUNING_NORMAL);
        if (!ret.isOk()) {
            ALOGE("IPictureQuality::setTuningField failed!");
        }
    }
    UNUSED(env);
    UNUSED(thiz);

    return JNI_TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
static jint getBlackEffectEnableRange(JNIEnv *env, jobject thiz)
{
    //return 2;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getBlackEffectEnableIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(BlackEffectEnable);
}

static jboolean setBlackEffectEnableIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(BlackEffectEnable, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getWhiteEffectEnableRange(JNIEnv *env, jobject thiz)
{
    //return 2;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getWhiteEffectEnableIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(WhiteEffectEnable);
}

static jboolean setWhiteEffectEnableIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(WhiteEffectEnable, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getStrongBlackEffectRange(JNIEnv *env, jobject thiz)
{
    //return 2;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getStrongBlackEffectIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(StrongBlackEffect);
}

static jboolean setStrongBlackEffectIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(StrongBlackEffect, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getStrongWhiteEffectRange(JNIEnv *env, jobject thiz)
{
    //return 2;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getStrongWhiteEffectIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(StrongWhiteEffect);
}

static jboolean setStrongWhiteEffectIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(StrongWhiteEffect, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getAdaptiveBlackEffectRange(JNIEnv *env, jobject thiz)
{
    //return 2;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getAdaptiveBlackEffectIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(AdaptiveBlackEffect);
}

static jboolean setAdaptiveBlackEffectIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(AdaptiveBlackEffect, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getAdaptiveWhiteEffectRange(JNIEnv *env, jobject thiz)
{
    //return 2;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getAdaptiveWhiteEffectIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(AdaptiveWhiteEffect);
}

static jboolean setAdaptiveWhiteEffectIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(AdaptiveWhiteEffect, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getScenceChangeOnceEnRange(JNIEnv *env, jobject thiz)
{
    //return 2;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getScenceChangeOnceEnIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ScenceChangeOnceEn);
}

static jboolean setScenceChangeOnceEnIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ScenceChangeOnceEn, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getScenceChangeControlEnRange(JNIEnv *env, jobject thiz)
{
    //return 2;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getScenceChangeControlEnIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ScenceChangeControlEn);
}

static jboolean setScenceChangeControlEnIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ScenceChangeControlEn, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getScenceChangeControlRange(JNIEnv *env, jobject thiz)
{
    //return 2;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getScenceChangeControlIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ScenceChangeControl);
}

static jboolean setScenceChangeControlIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ScenceChangeControl, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getScenceChangeTh1Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getScenceChangeTh1Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ScenceChangeTh1);
}

static jboolean setScenceChangeTh1Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ScenceChangeTh1, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getScenceChangeTh2Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getScenceChangeTh2Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ScenceChangeTh2);
}

static jboolean setScenceChangeTh2Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ScenceChangeTh2, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getScenceChangeTh3Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getScenceChangeTh3Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ScenceChangeTh3);
}

static jboolean setScenceChangeTh3Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ScenceChangeTh3, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getContentSmooth1Range(JNIEnv *env, jobject thiz)
{
    //return 65;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getContentSmooth1Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ContentSmooth1);
}

static jboolean setContentSmooth1Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ContentSmooth1, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getContentSmooth2Range(JNIEnv *env, jobject thiz)
{
    //return 65;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getContentSmooth2Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ContentSmooth2);
}

static jboolean setContentSmooth2Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ContentSmooth2, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getContentSmooth3Range(JNIEnv *env, jobject thiz)
{
    //return 65;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getContentSmooth3Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ContentSmooth3);
}

static jboolean setContentSmooth3Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ContentSmooth3, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getMiddleRegionGain1Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getMiddleRegionGain1Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(MiddleRegionGain1);
}

static jboolean setMiddleRegionGain1Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(MiddleRegionGain1, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getMiddleRegionGain2Range(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return 256;
}

static jint getMiddleRegionGain2Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(MiddleRegionGain2);
}

static jboolean setMiddleRegionGain2Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(MiddleRegionGain2, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getBlackRegionGain1Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getBlackRegionGain1Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(BlackRegionGain1);
}

static jboolean setBlackRegionGain1Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(BlackRegionGain1, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getBlackRegionGain2Range(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return 256;
}

static jint getBlackRegionGain2Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(BlackRegionGain2);
}

static jboolean setBlackRegionGain2Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(BlackRegionGain2, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getBlackRegionRangeRange(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return 17;
}

static jint getBlackRegionRangeIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(BlackRegionRange);
}

static jboolean setBlackRegionRangeIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(BlackRegionRange, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getBlackEffectLevelRange(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return 256;
}

static jint getBlackEffectLevelIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(BlackEffectLevel);
}

static jboolean setBlackEffectLevelIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(BlackEffectLevel, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getBlackEffectParam1Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getBlackEffectParam1Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(BlackEffectParam1);
}

static jboolean setBlackEffectParam1Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(BlackEffectParam1, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getBlackEffectParam2Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getBlackEffectParam2Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(BlackEffectParam2);
}

static jboolean setBlackEffectParam2Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(BlackEffectParam2, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getBlackEffectParam3Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getBlackEffectParam3Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(BlackEffectParam3);
}

static jboolean setBlackEffectParam3Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(BlackEffectParam3, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getBlackEffectParam4Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getBlackEffectParam4Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(BlackEffectParam4);
}

static jboolean setBlackEffectParam4Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(BlackEffectParam4, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getWhiteRegionGain1Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getWhiteRegionGain1Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(WhiteRegionGain1);
}

static jboolean setWhiteRegionGain1Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(WhiteRegionGain1, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getWhiteRegionGain2Range(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return 256;
}

static jint getWhiteRegionGain2Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(WhiteRegionGain2);
}

static jboolean setWhiteRegionGain2Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(WhiteRegionGain2, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getWhiteRegionRangeRange(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return 17;
}

static jint getWhiteRegionRangeIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(WhiteRegionRange);
}

static jboolean setWhiteRegionRangeIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(WhiteRegionRange, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getWhiteEffectLevelRange(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return 256;
}

static jint getWhiteEffectLevelIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(WhiteEffectLevel);
}

static jboolean setWhiteEffectLevelIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(WhiteEffectLevel, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getWhiteEffectParam1Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getWhiteEffectParam1Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(WhiteEffectParam1);
}

static jboolean setWhiteEffectParam1Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(WhiteEffectParam1, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getWhiteEffectParam2Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getWhiteEffectParam2Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(WhiteEffectParam2);
}

static jboolean setWhiteEffectParam2Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(WhiteEffectParam2, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getWhiteEffectParam3Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getWhiteEffectParam3Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(WhiteEffectParam3);
}

static jboolean setWhiteEffectParam3Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(WhiteEffectParam3, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getWhiteEffectParam4Range(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getWhiteEffectParam4Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(WhiteEffectParam4);
}

static jboolean setWhiteEffectParam4Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(WhiteEffectParam4, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getContrastAdjust1Range(JNIEnv *env, jobject thiz)
{
    //return 11;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getContrastAdjust1Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ContrastAdjust1);
}

static jboolean setContrastAdjust1Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ContrastAdjust1, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getContrastAdjust2Range(JNIEnv *env, jobject thiz)
{
    //return 13;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getContrastAdjust2Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ContrastAdjust2);
}

static jboolean setContrastAdjust2Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ContrastAdjust2, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getDCChangeSpeedLevelRange(JNIEnv *env, jobject thiz)
{
    //return 4;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getDCChangeSpeedLevelIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(DCChangeSpeedLevel);
}

static jboolean setDCChangeSpeedLevelIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(DCChangeSpeedLevel, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getProtectRegionEffectRange(JNIEnv *env, jobject thiz)
{
    //return 3;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getProtectRegionEffectIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ProtectRegionEffect);
}

static jboolean setProtectRegionEffectIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ProtectRegionEffect, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getDCChangeSpeedLevel2Range(JNIEnv *env, jobject thiz)
{
    //return 5;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getDCChangeSpeedLevel2Index(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(DCChangeSpeedLevel2);
}

static jboolean setDCChangeSpeedLevel2Index(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(DCChangeSpeedLevel2, index);
}

/////////////////////////////////////////////////////////////////////////////////
static jint getProtectRegionWeightRange(JNIEnv *env, jobject thiz)
{
    //return 256;
    UNUSED(env);
    UNUSED(thiz);
    return 0;
}

static jint getProtectRegionWeightIndex(JNIEnv *env, jobject thiz)
{
    UNUSED(env);
    UNUSED(thiz);
    return getIndex(ProtectRegionWeight);
}

static jboolean setProtectRegionWeightIndex(JNIEnv *env, jobject thiz , int index)
{
    UNUSED(env);
    UNUSED(thiz);
    return setIndex(ProtectRegionWeight, index);
}

/////////////////////////////////////////////////////////////////////////////////
//JNI register
////////////////////////////////////////////////////////////////
static const char *classPathName = "com/mediatek/galleryfeature/pq/dcfilter/DCFilter";

static JNINativeMethod g_methods[] = {

  {"nativeSetTuningMode",  "(I)Z", (void*)setTuningMode },

  {"nativeGetBlackEffectEnableRange",  "()I",  (void*)getBlackEffectEnableRange },
  {"nativeGetBlackEffectEnableIndex",  "()I",  (void*)getBlackEffectEnableIndex },
  {"nativeSetBlackEffectEnableIndex",  "(I)Z", (void*)setBlackEffectEnableIndex },

  {"nativeGetWhiteEffectEnableRange",  "()I",  (void*)getWhiteEffectEnableRange },
  {"nativeGetWhiteEffectEnableIndex",  "()I",  (void*)getWhiteEffectEnableIndex },
  {"nativeSetWhiteEffectEnableIndex",  "(I)Z", (void*)setWhiteEffectEnableIndex },

  {"nativeGetStrongBlackEffectRange",  "()I",  (void*)getStrongBlackEffectRange },
  {"nativeGetStrongBlackEffectIndex",  "()I",  (void*)getStrongBlackEffectIndex },
  {"nativeSetStrongBlackEffectIndex",  "(I)Z", (void*)setStrongBlackEffectIndex },

  {"nativeGetStrongWhiteEffectRange",  "()I",  (void*)getStrongWhiteEffectRange },
  {"nativeGetStrongWhiteEffectIndex",  "()I",  (void*)getStrongWhiteEffectIndex },
  {"nativeSetStrongWhiteEffectIndex",  "(I)Z", (void*)setStrongWhiteEffectIndex },

  {"nativeGetAdaptiveBlackEffectRange",  "()I",  (void*)getAdaptiveBlackEffectRange },
  {"nativeGetAdaptiveBlackEffectIndex",  "()I",  (void*)getAdaptiveBlackEffectIndex },
  {"nativeSetAdaptiveBlackEffectIndex",  "(I)Z", (void*)setAdaptiveBlackEffectIndex },

  {"nativeGetAdaptiveWhiteEffectRange",  "()I",  (void*)getAdaptiveWhiteEffectRange },
  {"nativeGetAdaptiveWhiteEffectIndex",  "()I",  (void*)getAdaptiveWhiteEffectIndex },
  {"nativeSetAdaptiveWhiteEffectIndex",  "(I)Z", (void*)setAdaptiveWhiteEffectIndex },

  {"nativeGetScenceChangeOnceEnRange",  "()I",  (void*)getScenceChangeOnceEnRange },
  {"nativeGetScenceChangeOnceEnIndex",  "()I",  (void*)getScenceChangeOnceEnIndex },
  {"nativeSetScenceChangeOnceEnIndex",  "(I)Z", (void*)setScenceChangeOnceEnIndex },

  {"nativeGetScenceChangeControlEnRange",  "()I",  (void*)getScenceChangeControlEnRange },
  {"nativeGetScenceChangeControlEnIndex",  "()I",  (void*)getScenceChangeControlEnIndex },
  {"nativeSetScenceChangeControlEnIndex",  "(I)Z", (void*)setScenceChangeControlEnIndex },

  {"nativeGetScenceChangeControlRange",  "()I",  (void*)getScenceChangeControlRange },
  {"nativeGetScenceChangeControlIndex",  "()I",  (void*)getScenceChangeControlIndex },
  {"nativeSetScenceChangeControlIndex",  "(I)Z", (void*)setScenceChangeControlIndex },

  {"nativeGetScenceChangeTh1Range",  "()I",  (void*)getScenceChangeTh1Range },
  {"nativeGetScenceChangeTh1Index",  "()I",  (void*)getScenceChangeTh1Index },
  {"nativeSetScenceChangeTh1Index",  "(I)Z", (void*)setScenceChangeTh1Index },

  {"nativeGetScenceChangeTh2Range",  "()I",  (void*)getScenceChangeTh2Range },
  {"nativeGetScenceChangeTh2Index",  "()I",  (void*)getScenceChangeTh2Index },
  {"nativeSetScenceChangeTh2Index",  "(I)Z", (void*)setScenceChangeTh2Index },

  {"nativeGetScenceChangeTh3Range",  "()I",  (void*)getScenceChangeTh3Range },
  {"nativeGetScenceChangeTh3Index",  "()I",  (void*)getScenceChangeTh3Index },
  {"nativeSetScenceChangeTh3Index",  "(I)Z", (void*)setScenceChangeTh3Index },

  {"nativeGetContentSmooth1Range",  "()I",  (void*)getContentSmooth1Range },
  {"nativeGetContentSmooth1Index",  "()I",  (void*)getContentSmooth1Index },
  {"nativeSetContentSmooth1Index",  "(I)Z", (void*)setContentSmooth1Index },

  {"nativeGetContentSmooth2Range",  "()I",  (void*)getContentSmooth2Range },
  {"nativeGetContentSmooth2Index",  "()I",  (void*)getContentSmooth2Index },
  {"nativeSetContentSmooth2Index",  "(I)Z", (void*)setContentSmooth2Index },

  {"nativeGetContentSmooth3Range",  "()I",  (void*)getContentSmooth3Range },
  {"nativeGetContentSmooth3Index",  "()I",  (void*)getContentSmooth3Index },
  {"nativeSetContentSmooth3Index",  "(I)Z", (void*)setContentSmooth3Index },

  {"nativeGetMiddleRegionGain1Range",  "()I",  (void*)getMiddleRegionGain1Range },
  {"nativeGetMiddleRegionGain1Index",  "()I",  (void*)getMiddleRegionGain1Index },
  {"nativeSetMiddleRegionGain1Index",  "(I)Z", (void*)setMiddleRegionGain1Index },

  {"nativeGetMiddleRegionGain2Range",  "()I",  (void*)getMiddleRegionGain2Range },
  {"nativeGetMiddleRegionGain2Index",  "()I",  (void*)getMiddleRegionGain2Index },
  {"nativeSetMiddleRegionGain2Index",  "(I)Z", (void*)setMiddleRegionGain2Index },

  {"nativeGetBlackRegionGain1Range",  "()I",  (void*)getBlackRegionGain1Range },
  {"nativeGetBlackRegionGain1Index",  "()I",  (void*)getBlackRegionGain1Index },
  {"nativeSetBlackRegionGain1Index",  "(I)Z", (void*)setBlackRegionGain1Index },

  {"nativeGetBlackRegionGain2Range",  "()I",  (void*)getBlackRegionGain2Range },
  {"nativeGetBlackRegionGain2Index",  "()I",  (void*)getBlackRegionGain2Index },
  {"nativeSetBlackRegionGain2Index",  "(I)Z", (void*)setBlackRegionGain2Index },

  {"nativeGetBlackRegionRangeRange",  "()I",  (void*)getBlackRegionRangeRange },
  {"nativeGetBlackRegionRangeIndex",  "()I",  (void*)getBlackRegionRangeIndex },
  {"nativeSetBlackRegionRangeIndex",  "(I)Z", (void*)setBlackRegionRangeIndex },

  {"nativeGetBlackEffectLevelRange",  "()I",  (void*)getBlackEffectLevelRange },
  {"nativeGetBlackEffectLevelIndex",  "()I",  (void*)getBlackEffectLevelIndex },
  {"nativeSetBlackEffectLevelIndex",  "(I)Z", (void*)setBlackEffectLevelIndex },

  {"nativeGetBlackEffectParam1Range",  "()I",  (void*)getBlackEffectParam1Range },
  {"nativeGetBlackEffectParam1Index",  "()I",  (void*)getBlackEffectParam1Index },
  {"nativeSetBlackEffectParam1Index",  "(I)Z", (void*)setBlackEffectParam1Index },

  {"nativeGetBlackEffectParam2Range",  "()I",  (void*)getBlackEffectParam2Range },
  {"nativeGetBlackEffectParam2Index",  "()I",  (void*)getBlackEffectParam2Index },
  {"nativeSetBlackEffectParam2Index",  "(I)Z", (void*)setBlackEffectParam2Index },

  {"nativeGetBlackEffectParam3Range",  "()I",  (void*)getBlackEffectParam3Range },
  {"nativeGetBlackEffectParam3Index",  "()I",  (void*)getBlackEffectParam3Index },
  {"nativeSetBlackEffectParam3Index",  "(I)Z", (void*)setBlackEffectParam3Index },

  {"nativeGetBlackEffectParam4Range",  "()I",  (void*)getBlackEffectParam4Range },
  {"nativeGetBlackEffectParam4Index",  "()I",  (void*)getBlackEffectParam4Index },
  {"nativeSetBlackEffectParam4Index",  "(I)Z", (void*)setBlackEffectParam4Index },

  {"nativeGetWhiteRegionGain1Range",  "()I",  (void*)getWhiteRegionGain1Range },
  {"nativeGetWhiteRegionGain1Index",  "()I",  (void*)getWhiteRegionGain1Index },
  {"nativeSetWhiteRegionGain1Index",  "(I)Z", (void*)setWhiteRegionGain1Index },

  {"nativeGetWhiteRegionGain2Range",  "()I",  (void*)getWhiteRegionGain2Range },
  {"nativeGetWhiteRegionGain2Index",  "()I",  (void*)getWhiteRegionGain2Index },
  {"nativeSetWhiteRegionGain2Index",  "(I)Z", (void*)setWhiteRegionGain2Index },

  {"nativeGetWhiteRegionRangeRange",  "()I",  (void*)getWhiteRegionRangeRange },
  {"nativeGetWhiteRegionRangeIndex",  "()I",  (void*)getWhiteRegionRangeIndex },
  {"nativeSetWhiteRegionRangeIndex",  "(I)Z", (void*)setWhiteRegionRangeIndex },

  {"nativeGetWhiteEffectLevelRange",  "()I",  (void*)getWhiteEffectLevelRange },
  {"nativeGetWhiteEffectLevelIndex",  "()I",  (void*)getWhiteEffectLevelIndex },
  {"nativeSetWhiteEffectLevelIndex",  "(I)Z", (void*)setWhiteEffectLevelIndex },

  {"nativeGetWhiteEffectParam1Range",  "()I",  (void*)getWhiteEffectParam1Range },
  {"nativeGetWhiteEffectParam1Index",  "()I",  (void*)getWhiteEffectParam1Index },
  {"nativeSetWhiteEffectParam1Index",  "(I)Z", (void*)setWhiteEffectParam1Index },

  {"nativeGetWhiteEffectParam2Range",  "()I",  (void*)getWhiteEffectParam2Range },
  {"nativeGetWhiteEffectParam2Index",  "()I",  (void*)getWhiteEffectParam2Index },
  {"nativeSetWhiteEffectParam2Index",  "(I)Z", (void*)setWhiteEffectParam2Index },

  {"nativeGetWhiteEffectParam3Range",  "()I",  (void*)getWhiteEffectParam3Range },
  {"nativeGetWhiteEffectParam3Index",  "()I",  (void*)getWhiteEffectParam3Index },
  {"nativeSetWhiteEffectParam3Index",  "(I)Z", (void*)setWhiteEffectParam3Index },

  {"nativeGetWhiteEffectParam4Range",  "()I",  (void*)getWhiteEffectParam4Range },
  {"nativeGetWhiteEffectParam4Index",  "()I",  (void*)getWhiteEffectParam4Index },
  {"nativeSetWhiteEffectParam4Index",  "(I)Z", (void*)setWhiteEffectParam4Index },

  {"nativeGetContrastAdjust1Range",  "()I",  (void*)getContrastAdjust1Range },
  {"nativeGetContrastAdjust1Index",  "()I",  (void*)getContrastAdjust1Index },
  {"nativeSetContrastAdjust1Index",  "(I)Z", (void*)setContrastAdjust1Index },

  {"nativeGetContrastAdjust2Range",  "()I",  (void*)getContrastAdjust2Range },
  {"nativeGetContrastAdjust2Index",  "()I",  (void*)getContrastAdjust2Index },
  {"nativeSetContrastAdjust2Index",  "(I)Z", (void*)setContrastAdjust2Index },

  {"nativeGetDCChangeSpeedLevelRange",  "()I",  (void*)getDCChangeSpeedLevelRange },
  {"nativeGetDCChangeSpeedLevelIndex",  "()I",  (void*)getDCChangeSpeedLevelIndex },
  {"nativeSetDCChangeSpeedLevelIndex",  "(I)Z", (void*)setDCChangeSpeedLevelIndex },

  {"nativeGetProtectRegionEffectRange",  "()I",  (void*)getProtectRegionEffectRange },
  {"nativeGetProtectRegionEffectIndex",  "()I",  (void*)getProtectRegionEffectIndex },
  {"nativeSetProtectRegionEffectIndex",  "(I)Z", (void*)setProtectRegionEffectIndex },

  {"nativeGetDCChangeSpeedLevel2Range",  "()I",  (void*)getDCChangeSpeedLevel2Range },
  {"nativeGetDCChangeSpeedLevel2Index",  "()I",  (void*)getDCChangeSpeedLevel2Index },
  {"nativeSetDCChangeSpeedLevel2Index",  "(I)Z", (void*)setDCChangeSpeedLevel2Index },

  {"nativeGetProtectRegionWeightRange",  "()I",  (void*)getProtectRegionWeightRange },
  {"nativeGetProtectRegionWeightIndex",  "()I",  (void*)getProtectRegionWeightIndex },
  {"nativeSetProtectRegionWeightIndex",  "(I)Z", (void*)setProtectRegionWeightIndex }

};

/*
 * Register several native methods for one class.
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* gMethods, int numMethods)
{
    jclass clazz;

    clazz = env->FindClass(className);
    if (clazz == NULL) {
        ALOGE("Native registration unable to find class '%s'", className);
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        ALOGE("RegisterNatives failed for '%s'", className);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

// ----------------------------------------------------------------------------

/*
 * This is called by the VM when the shared library is first loaded.
 */

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;
    UNUSED(reserved);
    ALOGI("JNI_OnLoad");

    if (JNI_OK != vm->GetEnv((void **)&env, JNI_VERSION_1_4)) {
        ALOGE("ERROR: GetEnv failed");
        goto bail;
    }

    if (!registerNativeMethods(env, classPathName, g_methods, sizeof(g_methods) / sizeof(g_methods[0]))) {
        ALOGE("ERROR: registerNatives failed");
        goto bail;
    }

    result = JNI_VERSION_1_4;

bail:
    return result;
}

