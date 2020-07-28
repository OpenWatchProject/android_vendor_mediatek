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
#ifndef __MFLLMEMC_H__
#define __MFLLMEMC_H__

#include <mtkcam3/feature/mfnr/IMfllMemc.h>

// ALGORITHM
#include <MTKMfbll.h>

using android::sp;

namespace mfll {
class MfllMemc : public IMfllMemc {
public:
    MfllMemc();

public:
    enum MfllErr init(sp<IMfllNvram> &nvramProvider);
    enum MfllErr getAlgorithmWorkBufferSize(size_t *size);
    enum MfllErr motionEstimation();
    enum MfllErr motionCompensation();
    enum MfllErr setMfllCore(IMfllCore *c)
    {
        m_pCore = c;
        return MfllErr_Ok;
    }
    void setMotionVector(int x, int y)
    {
        m_globalMv.x = x;
        m_globalMv.y = y;
    }
    void setMotionEstimationResolution(const int &w, const int &h)
    {
        m_widthMe = w;
        m_heightMe = h;
        updateIsUsingFullMemc();
    }
    void setMotionCompensationResolution(const int &w, const int &h)
    {
        m_widthMc = w;
        m_heightMc = h;
        updateIsUsingFullMemc();
    }
    void setAlgorithmWorkingBuffer(sp<IMfllImageBuffer> &buffer)
    {
        m_imgWorkingBuffer = buffer;
    }
    void setMeBaseImage(sp<IMfllImageBuffer> &img)
    {
        m_imgSrcMe = img;
    }
    void setMeRefImage(sp<IMfllImageBuffer> &img)
    {
        m_imgRefMe = img;
    }
    void setMcRefImage(sp<IMfllImageBuffer> &img)
    {
        m_imgRefMc = img;
    }
    void setMcDstImage(sp<IMfllImageBuffer> &img)
    {
        m_imgDstMc = img;
    }
    void setMcThreadNum(const int &thread_num)
    {
        m_mcThreadNum = thread_num;
    }
    int isIgnoredMotionCompensation()
    {
        return m_isIgnoredMc;
    }
    void giveupMotionCompensation()
    {
        m_isIgnoredMc = 1;
    }

    void setMemcDump(int p, int r, int f)
    {
        m_dump_uniqueKey = p;
        m_dump_requestNum = r;
        m_dump_framNum = f;
    }

    /* To get the denominator value for Me size from algo define*/
    int32_t getMeDnRatio()
    {
        //defined in SRS
        if (m_pCore)
            return m_pCore->isMemcDownscale()?2:4;

        return PROC1_DN_RATIO;
    }

    /* To get the frame level confidence from ME */
    int32_t getMeFrameLevelConfidence()
    {
        return m_frameLevelConfidence;
    }

    /* To force Me setting as manual */
    bool getForceMe(void* param_addr, size_t param_size);

//
// New APIs
//
public:
    void setConfidenceMapImage(sp<IMfllImageBuffer>& map) override
    {
        m_imgConfidenceMap = map;
    }

    void setMotionCompensationMvImage(sp<IMfllImageBuffer>& mv) override
    {
        m_imgMotionCompensationMv = mv;
    }

    void setBlendType(int type) override
    {
        m_bldType = type;
    }

    void setCurrentIso(int iso) override
    {
        m_currIso = iso;
    }


private:
    inline void updateIsUsingFullMemc()
    {
        /* if resolution is different, try it as using full MC */
        m_isUsingFullMc = (m_widthMe == m_widthMc && m_heightMe == m_heightMc)
            ? 0 : 1;
    }

private:
    MTKMfbll* m_mfbll;

    int m_widthMe;
    int m_heightMe;
    int m_widthMc;
    int m_heightMc;
    int m_mcThreadNum;

    MfllMotionVector_t m_globalMv;

    volatile int m_isUsingFullMc;
    volatile int m_isIgnoredMc;
    volatile int m_frameLevelConfidence;
    volatile int m_bldType;
    volatile int m_currIso;
    volatile int m_dump_uniqueKey;
    volatile int m_dump_requestNum;
    volatile int m_dump_framNum;

    /* image buffer of confidence map */
    sp<IMfllImageBuffer> m_imgConfidenceMap;
    sp<IMfllImageBuffer> m_imgMotionCompensationMv;

    /* image buffers for algorithm working, decStrong after motionCompensation */
    sp<IMfllImageBuffer> m_imgWorkingBuffer;

    /* image buffers for ME, decStrong after motionEstimation() */
    sp<IMfllImageBuffer> m_imgSrcMe;
    sp<IMfllImageBuffer> m_imgRefMe;

    /* image buffers for MC, decStrong after motionCompensation */
    sp<IMfllImageBuffer> m_imgRefMc;
    sp<IMfllImageBuffer> m_imgDstMc;

    /* nvram provider */
    sp<IMfllNvram>       m_nvramProvider;

    IMfllCore            *m_pCore;

protected:
    virtual ~MfllMemc(void);
};
}; /* namespace mfll */

#endif//__MFLLMEMC_H__

