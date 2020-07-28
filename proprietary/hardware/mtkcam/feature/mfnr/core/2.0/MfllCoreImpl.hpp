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

#ifndef __MFLLCORE_IMPL_INC__
#define __MFLLCORE_IMPL_INC__
namespace mfll {


enum MfllErr MfllCore::do_Init(const MfllConfig_t &cfg)
{
    MfllErr err = MfllErr_Ok;
    /* copy configuration */
    /* Update catprue frame number and blend frame number */
    m_iso = cfg.iso;
    m_exposure = cfg.exp;
    m_frameBlend = cfg.blend_num;
    m_frameCapture = cfg.capture_num;
    m_sensorId = cfg.sensor_id;
    m_shotMode = cfg.mfll_mode;
    m_rwbMode = cfg.rwb_mode;
    m_mrpMode = cfg.mrp_mode;
    m_memcMode = cfg.memc_mode;
    m_isFullSizeMc = cfg.full_size_mc != 0 ? 1 : 0;

    /**
     *  Using (number of blending - 1) as MEMC threads num
     *  or the default should be m_memcInstanceNum = MFLL_MEMC_THREADS_NUM;
     */
    m_memcInstanceNum = m_frameBlend - 1;

    /* update debug info */
    m_dbgInfoCore.frameCapture = m_frameCapture;
    m_dbgInfoCore.frameBlend = m_frameBlend;
    m_dbgInfoCore.iso = m_iso;
    m_dbgInfoCore.exp = m_exposure;
    m_dbgInfoCore.ori_iso = cfg.original_iso;
    m_dbgInfoCore.ori_exp = cfg.original_exp;
    if (isMfllMode(m_shotMode))
        m_dbgInfoCore.shot_mode = 1;
    else if (isAisMode(m_shotMode))
        m_dbgInfoCore.shot_mode = 2;
    else
        m_dbgInfoCore.shot_mode = 0;


    /* assign pointers to real buffer */
    m_ptrImgYuvBase = &(m_imgYuvBase);
    m_ptrImgYuvGolden = &(m_imgYuvGolden);
    m_ptrImgYuvBlended = &(m_imgYuvBlended);
    m_ptrImgWeightingIn = &(m_imgWeighting[0]);
    m_ptrImgWeightingOut = &(m_imgWeighting[1]);

    /* update m_qwidth and m_qheight to half of capture resolution */
    m_qwidth = m_width / 2;
    m_qheight = m_height / 2;

    /* init NVRAM provider */
    if (CC_UNLIKELY(m_spNvramProvider.get() == NULL)) {
        mfllLogW("%s: m_spNvramProvider has not been set, try to create one", __FUNCTION__);
        m_spNvramProvider = IMfllNvram::createInstance();
        if (CC_UNLIKELY(m_spNvramProvider.get() == NULL)) {
            err = MfllErr_UnexpectedError;
            mfllLogE("%s: create NVRAM provider failed", __FUNCTION__);
            goto lbExit;
        }
        IMfllNvram::ConfigParams nvramCfg;
        nvramCfg.iSensorId = cfg.sensor_id;
        nvramCfg.bFlashOn  = cfg.flash_on;
        m_spNvramProvider->init(nvramCfg);
    }

    /* create MEMC instance */
    for (size_t i = 0; i < getMemcInstanceNum(); i++) {
        m_spMemc[i] = IMfllMemc::createInstance();
        m_spMemc[i]->setMfllCore(this);
        m_spMemc[i]->setMotionEstimationResolution(m_qwidth, m_qheight);
        if (m_isFullSizeMc)
            m_spMemc[i]->setMotionCompensationResolution(m_width, m_height);
        else
            m_spMemc[i]->setMotionCompensationResolution(m_qwidth, m_qheight);

        /* set thread priority to algorithm threads */
        // {{{
        // Changes the current thread's priority, the algorithm threads will
        // inherits this value.
        int _priority = MfllProperty::readProperty(Property_AlgoThreadsPriority, MFLL_ALGO_THREADS_PRIORITY);
        int _oripriority = 0;
        // We give higher thread priority at the first MEMC thread because we
        // want the first MEMC finished ASAP.
        int _result = setThreadPriority(_priority - (i == 0 ? 1 : 0), _oripriority);
        if (CC_UNLIKELY( _result != 0 )) {
            mfllLogW("set algo threads priority failed(err=%d)", _result);
        }
        else {
            mfllLogD("set algo threads priority to %d", _priority);
        }
        // }}}

        // init MEMC
        if (CC_UNLIKELY(m_spMemc[i]->init(m_spNvramProvider) != MfllErr_Ok)) {
            mfllLogE("%s: init MfllMemc failed with code", __FUNCTION__);
        }

        // algorithm threads have been forked,
        // if priority set OK, reset it back to the original one
        if (CC_LIKELY( _result == 0 )) {
            _result = setThreadPriority( _oripriority, _oripriority );
            if (CC_UNLIKELY( _result != 0 )) {
                mfllLogE("set priority back failed, weird!");
            }
        }

    }

    if (CC_LIKELY(m_mrpMode == MrpMode_BestPerformance)) {
        updateImageBufferQueueLimit();
        tellsFutureAllocateMemory();
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_Capture(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;

    {
        /**
         *  N O T I C E
         *
         *  Aquire buffers should be invoked by MfllCapturer
         *  1. IMfllCore::doAllocRawBuffer
         *  2. IMfllCore::doAllocQyuvBuffer
         */
        /* check if IMfllCapturer has been assigned */
        if (CC_UNLIKELY(m_spCapturer.get() == NULL)) {
            mfllLogD("%s: create MfllCapturer", __FUNCTION__);
            m_spCapturer = IMfllCapturer::createInstance();
            if (m_spCapturer.get() == NULL) {
                mfllLogE("%s: create MfllCapturer instance", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
        }

        std::vector< sp<IMfllImageBuffer>* > raws;
        std::vector< sp<IMfllImageBuffer>* > rrzos;
        std::vector< sp<IMfllImageBuffer>* > yuvs;
        std::vector< sp<IMfllImageBuffer>* > qyuvs;
        std::vector<MfllMotionVector_t*>     gmvs;
        std::vector< int >                   rStatus;

        /* resize first */
        raws    .resize(getCaptureFrameNum());
        yuvs    .resize(getCaptureFrameNum());
        qyuvs   .resize(getCaptureFrameNum());
        gmvs    .resize(getCaptureFrameNum());
        rStatus .resize(getCaptureFrameNum());

        /* prepare output buffer */
        for (int i = 0; i < (int)getCaptureFrameNum(); i++) {
            raws[i] = &m_imgRaws[i].imgbuf;
            qyuvs[i] = &m_imgQYuvs[i].imgbuf;
            gmvs[i] = &m_globalMv[i];
            rStatus[i] = 0;

            if (m_isFullSizeMc)
                yuvs[i] = &m_imgYuvs[i].imgbuf;
        }

        /* register event dispatcher and set MfllCore instance */
        m_spCapturer->setMfllCore((IMfllCore*)this);
        err = m_spCapturer->registerEventDispatcher(m_event);

        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: MfllCapture::registerEventDispatcher failed with code %d", __FUNCTION__, err);
            goto lbExit;
        }

        /* Catpure frames */
        mfllLogD("Capture frames!");

        if (m_isFullSizeMc)
            err = m_spCapturer->captureFrames(getCaptureFrameNum(), raws, rrzos, yuvs, qyuvs, gmvs, rStatus);
        else
            err = m_spCapturer->captureFrames(getCaptureFrameNum(), raws, rrzos, qyuvs, gmvs, rStatus);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: MfllCapture::captureFrames failed with code %d", __FUNCTION__, err);
            goto lbExit;
        }
        /* check if force set GMV to zero */
        if (CC_UNLIKELY(m_spProperty->getForceGmvZero() > 0)) {
            for (int i = 0; i < MFLL_MAX_FRAMES; i++) {
                m_globalMv[i].x = 0;
                m_globalMv[i].y = 0;
            }
        }
        else if (m_spProperty->getForceGmv(m_globalMv)) {
            mfllLogI("%s: force set Gmv as manual setting", __FUNCTION__);
            for (int i = 0; i < MFLL_MAX_FRAMES; i++) {
                mfllLogI("%s: m_globalMv[%d](x,y) = (%d, %d)", __FUNCTION__, i, m_globalMv[i].x, m_globalMv[i].y);
            }
        }


        // {{{ checks captured buffers
        /* check result, resort buffers, and update frame capture number and blend number if need */
        {
            std::vector<ImageBufferPack>     r; // raw buffer;
            std::vector<ImageBufferPack>     q; // qyuv buffer
            std::vector<ImageBufferPack>     y; // full size yuv buffer
            std::vector<MfllMotionVector_t>  m; // motion

            r.resize(getCaptureFrameNum());
            q.resize(getCaptureFrameNum());
            y.resize(getCaptureFrameNum());
            m.resize(getCaptureFrameNum());

            size_t okCount = 0; // counting ok frame numbers

            /* check the status from Capturer, if status is 0 means ok */
            for (size_t i = 0, j = 0; i < (size_t)getCaptureFrameNum(); i++) {
                /* If not failed, save to stack */
                if (rStatus[i] == 0) {
                    r[j] = m_imgRaws[i];
                    q[j] = m_imgQYuvs[i];
                    y[j] = m_imgYuvs[i];
                    m[j] = m_globalMv[i];
                    okCount++;
                    j++;
                }
            }

            m_dbgInfoCore.frameCapture = okCount;
            mfllLogD("capture done, ok count=%zu, num of capture = %u",
                    okCount, getCaptureFrameNum());

            m_caputredCount = okCount;

            /* if not equals, something wrong */
            if (okCount != (size_t)getCaptureFrameNum()) {
                /* sort available buffers continuously */
                for (size_t i = 0; i < okCount; i++) {
                    m_imgRaws[i] = r[i];
                    m_imgYuvs[i] = y[i];
                    m_imgQYuvs[i] = q[i];
                    m_globalMv[i] = m[i];
                }

                /* boundary blending frame number */
                if (getBlendFrameNum() > okCount) {
                    m_dbgInfoCore.frameBlend = okCount;
                    /* by pass un-necessary operation due to no buffer, included the last frame */
                    for (size_t i = (okCount <= 0 ? 0 : okCount - 1); i < getBlendFrameNum(); i++) {
                        m_bypass.bypassMotionEstimation[i] = 1;
                        m_bypass.bypassMotionCompensation[i] = 1;
                        m_bypass.bypassBlending[i] = 1;
                    }
                }

                /* if okCount <= 0, it means no source image can be used, ignore all operations */
                if (CC_UNLIKELY( okCount <= 0 )) {
                    mfllLogE("%s: okCount <= 0, no source image can be used, " \
                             "ignore all operations.(SF, BASEYUV, MIX)", __FUNCTION__);
                    m_bypass.bypassEncodeYuvBase = 1;
                    m_bypass.bypassEncodeYuvGolden = 1;
                    m_bypass.bypassMixing = 1;
                }
            }
        } // }}}
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_Bss(intptr_t intptrframeCount)
{
    enum MfllErr err = MfllErr_Ok;
    int bssFrameCount = intptrframeCount;

    {
        sp<IMfllBss> bss = IMfllBss::createInstance();
        if (CC_UNLIKELY(bss.get() == NULL)) {
            mfllLogE("%s: create IMfllBss instance fail", __FUNCTION__);
            m_dbgInfoCore.bss_enable = 0;
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
        bss->setMfllCore(this);

        err = bss->init(m_spNvramProvider);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: init BSS failed, ignore BSS", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            m_dbgInfoCore.bss_enable = 0;
            goto lbExit;
        }

        std::vector< sp<IMfllImageBuffer> > imgs;
        std::vector< MfllMotionVector_t >   mvs;
        std::vector< int64_t > tss;

        for (int i = 0; i < bssFrameCount; i++) {
            imgs.push_back(m_imgQYuvs[i].imgbuf);
            mvs.push_back(m_globalMv[i]);
        }

        std::vector<int> newIndex = bss->bss(imgs, mvs, tss);
        if (newIndex.size() <= 0) {
            mfllLogE("%s: do bss failed", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        // sort items
        std::deque<ImageBufferPack> newRaws;
        std::deque<ImageBufferPack> newQYuvs;
        std::deque<ImageBufferPack> newYuvs;
        for (int i = 0; i < bssFrameCount; i++) {
            int index = newIndex[i]; // new index
            newRaws.push_back(m_imgRaws[index]);
            newQYuvs.push_back(m_imgQYuvs[index]);
            newYuvs.push_back(m_imgYuvs[index]);
            m_bssIndex[i] = index;
            mfllLogD("%s: new index (%u)-->(%d)", __FUNCTION__, i, index);
            /**
             *  mvs will be sorted by Bss, we don't need to re-sort it again,
             *  just update it
             */
            m_globalMv[i] = mvs[i];
        }
        for (int i = 0; i < bssFrameCount; i++) {
            m_imgRaws[i] = newRaws[i];
            m_imgQYuvs[i] = newQYuvs[i];
            m_imgYuvs[i] = newYuvs[i];
        }

        // check frame to skip
        size_t frameNumToSkip = bss->getSkipFrameCount();

        int forceDropNum   = MfllProperty::getDropNum();
        if (CC_UNLIKELY(forceDropNum > -1)) {
            mfllLogD("%s: force drop frame count = %d, original bss drop frame = %zu",
                    __FUNCTION__, forceDropNum, frameNumToSkip);

            frameNumToSkip = static_cast<size_t>(forceDropNum);
        }

#if 0
        // test skip all frames
        frameNumToSkip = bssFrameCount - 1;
        // test out-of-range
        frameNumToSkip = 100;
#endif

        if (frameNumToSkip > 0) {
            // get the index of image buffers for releasing due to ignored
            int idxForImages =
                static_cast<int>(getCaptureFrameNum()) - static_cast<int>(frameNumToSkip);

            // get the infex of operations (including MEMC, blending) to ignore
            int idxForOperation = idxForImages - 1;
            if (idxForOperation < 0) {
                mfllLogE("%s: drop frame number(%zu) is more than capture number(%u), ignore all frames",
                        __FUNCTION__,
                        frameNumToSkip,
                        getCaptureFrameNum());
                idxForOperation = 0;
                idxForImages = idxForOperation + 1;
            }
            mfllLogD("%s: skip frame count = %zu", __FUNCTION__, frameNumToSkip);

            if (idxForOperation <= 0) { // ignore all blending
                mfllLogW("%s: BSS drops all frames, using single capture", __FUNCTION__);
                // ignore encoding YUV base due to no need to blend
                m_bypass.bypassEncodeYuvBase = 1;
            }

            for (size_t i = static_cast<size_t>(idxForOperation); i < MFLL_MAX_FRAMES; i++) {
                // clear image
                if (i >= static_cast<size_t>(idxForImages)) {
                    mfllLogD("%s: clear images (index = %zu)", __FUNCTION__, i);

                    // release RAW, YUV and QYUV image buffer
                    m_imgRaws[i].releaseBufLocked();
                    m_imgYuvs[i].releaseBufLocked();
                    m_imgQYuvs[i].releaseBufLocked();
                }
                if (i >= static_cast<size_t>(idxForOperation)) {
                    mfllLogD("%s: bypass MEMC/BLD (index = %zu)", __FUNCTION__, i);
                    m_bypass.bypassMotionEstimation[i] = 1;
                    m_bypass.bypassMotionCompensation[i] = 1;
                    m_bypass.bypassBlending[i] = 1;
                }
            }
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_EncodeYuvBase(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;

    {
        err = doAllocYuvBase(NULL);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: allocate YUV base failed", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        if (CC_UNLIKELY(m_spMfb.get() == NULL)) {
            mfllLogE("%s: MfllMfb is NULL, cannot encode", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        m_spMfb->init(m_sensorId);
        m_spMfb->setShotMode(m_shotMode);
        m_spMfb->setPostNrType(m_postNrType);
        mfllLogD("%s: encode base raw to yuv", __FUNCTION__);

        // if using full size MC,
        if (m_isFullSizeMc) {
            mfllLogD("%s: convert YV16 to YUV422 as base Yuv using MDP", __FUNCTION__);
            err = m_spMfb->convertYuvFormatByMdp(
                    m_imgYuvs[0].imgbuf.get(),
                    m_imgYuvBase.imgbuf.get(),
                    nullptr,
                    MfllRect_t(
                        0, 0,
                        m_imgYuvs[0].imgbuf->getWidth(), m_imgYuvs[0].imgbuf->getHeight()
                        ),
                    MfllRect_t(),
                    YuvStage_BaseYuy2);

            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogD("%s: convertYuvFormatByMdp returns fail, try to use P2", __FUNCTION__);
                err = m_spMfb->encodeRawToYuv(
                        m_imgRaws[0].imgbuf.get(),      // source RAW
                        m_imgYuvBase.imgbuf.get(),      // output 1
                        YuvStage_BaseYuy2);
            }
        }
        else {
            err = m_spMfb->encodeRawToYuv(
                    m_imgRaws[0].imgbuf.get(),      // source RAW
                    m_imgYuvBase.imgbuf.get(),      // output 1
                    YuvStage_BaseYuy2);
        }
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: Encode RAW to YUV fail", __FUNCTION__);
            goto lbExit;
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_EncodeYuvGolden(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;

    {
        err = doAllocYuvGolden(NULL);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: allocate YUV golden failed", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        if (CC_UNLIKELY(m_spMfb.get() == NULL)) {
            mfllLogE("%s: MfllMfb is NULL, cannot encode", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        m_spMfb->init(m_sensorId);
        m_spMfb->setShotMode(m_shotMode);
        m_spMfb->setPostNrType(m_postNrType);

        // switch source image, if ignore encoding base YUV, use RAW as input
        sp<IMfllImageBuffer> imgSrc = nullptr;
        if (m_bypass.bypassEncodeYuvBase) {
            // update shot mode to Zsd+SingleFrame
            mfllLogD("%s: use single frame profile", __FUNCTION__);
            m_spMfb->setShotMode(static_cast<enum MfllMode>(m_shotMode | (1 << MfllMode_Bit_SingleFrame)));
            imgSrc = m_imgRaws[0].imgbuf;
        }
        else
            imgSrc = m_imgYuvBase.imgbuf;

        err = m_spMfb->encodeRawToYuv(
                imgSrc.get(),
                m_imgYuvGolden.imgbuf.get(),
                m_imgYuvPostview.imgbuf.get(),
                MfllRect_t(),
                m_postviewCropRgn,
                YuvStage_GoldenYuy2
                );

        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: Encode RAW to YUV fail", __FUNCTION__);
            goto lbExit;
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_MotionEstimation(void *void_index)
{
    unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;
    std::string _log = std::string("start ME") + to_char(index);

    {
        mfllAutoLog(_log.c_str());
        unsigned int memcIndex = index % getMemcInstanceNum();
        sp<IMfllMemc> memc = m_spMemc[memcIndex];
        if (CC_UNLIKELY(memc.get() == NULL)) {
            mfllLogE("%s: MfllMemc is necessary to be created first (index=%d)", __FUNCTION__, index);
            err = MfllErr_NullPointer;
            goto lbExit;
        }

        err = doAllocMemcWorking((void*)(long)memcIndex);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: allocate MEMC working buffer(%d) failed", __FUNCTION__, memcIndex);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
        /* set motion vector */
        memc->setMotionVector(m_globalMv[index + 1].x, m_globalMv[index + 1].y);
        memc->setAlgorithmWorkingBuffer(m_imgMemc[memcIndex].imgbuf);
        memc->setMeBaseImage(m_imgQYuvs[0].imgbuf);
        memc->setMeRefImage(m_imgQYuvs[index + 1].imgbuf);

        err = memc->motionEstimation();
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            memc->giveupMotionCompensation();
            mfllLogE("%s: IMfllMemc::motionEstimation failed, returns %d", __FUNCTION__, (int)err);
            goto lbExit;
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_MotionCompensation(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;

    /* using full size YUV if is using full size MC */
    ImageBufferPack* pMcRef = (m_isFullSizeMc == 0)
        ? &m_imgQYuvs[index + 1]
        : &m_imgYuvs[index + 1]
        ;

    {
        unsigned int memcIndex = index % getMemcInstanceNum();
        sp<IMfllMemc> memc = m_spMemc[memcIndex];

        if (CC_UNLIKELY(memc.get() == NULL)) {
            mfllLogE("%s: MfllMemc is necessary to be created first (index=%d)", __FUNCTION__, index);
            err = MfllErr_NullPointer;
            goto lbExit;
        }

        /* check if we need to do MC or not */
        if (memc->isIgnoredMotionCompensation()) {
            mfllLogD("%s: ignored motion compensation & blending", __FUNCTION__);
            m_bypass.bypassBlending[index] = 1;
            goto lbExit;
        }

        /* allocate YUV MC working buffer if necessary */
        err = doAllocYuvMcWorking(NULL);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: allocate YUV MC working buffer failed", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        /* setting up mc */
        memc->setMcRefImage(pMcRef->imgbuf);
        memc->setMcDstImage(m_imgYuvMcWorking.imgbuf);

        err = memc->motionCompensation();
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: IMfllMemc::motionCompensation failed, returns %d", __FUNCTION__, (int)err);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        /* exchange buffer from Dst->Src */
        {
            ImageBufferPack::swap(pMcRef, &m_imgYuvMcWorking);
            /* convert image format to Yuy2 without alignment for blending or mixing */
            pMcRef->imgbuf->setAligned(2, 2);
            pMcRef->imgbuf->convertImageFormat(ImageFormat_Yuy2);
        }

        /* sync CPU cache to HW */
        pMcRef->imgbuf->syncCache(); // CPU->HW
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_Blending(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;

   {
        MfllOperationSync::getInstance()->addJob(MfllOperationSync::JOB_MFB);

        err = doAllocYuvWorking(NULL);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: allocate YUV working buffer failed", __FUNCTION__);
            goto lbExit;
        }

        err = doAllocWeighting((void*)(long)0);
        err = doAllocWeighting((void*)(long)1);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: allocate weighting buffer 0 or 1 failed", __FUNCTION__);
            goto lbExit;
        }

        if (CC_UNLIKELY(m_spMfb.get() == NULL)) {
            mfllLogD("%s: create IMfllMfb instance", __FUNCTION__);
            m_spMfb = IMfllMfb::createInstance();
            if (m_spMfb.get() == NULL) {
                mfllLogE("%s: m_spMfb is NULL", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
        }

        err = m_spMfb->init(m_sensorId);
        m_spMfb->setShotMode(m_shotMode);
        m_spMfb->setPostNrType(m_postNrType);

        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: m_spMfb init failed with code %d", __FUNCTION__, (int)err);
            goto lbExit;
        }

        /* do blending */
        mfllLogD("%s: do blending now", __FUNCTION__);

        /**
         * while index == 0, which means the first time to blend, the input weighting
         * table should be sent
         */
        err = m_spMfb->blend(
            m_ptrImgYuvBase->imgbuf.get(),
            m_ptrImgYuvRef->imgbuf.get(),
            m_ptrImgYuvBlended->imgbuf.get(),
            (index == 0) ? NULL : m_ptrImgWeightingIn->imgbuf.get(),
            m_ptrImgWeightingOut->imgbuf.get()
        );

        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: Mfb failed with code %d", __FUNCTION__, (int)err);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
        else {
            /* save for mixing */
            m_ptrImgWeightingFinal = m_ptrImgWeightingOut;
            m_blendedCount++; // count blended frame
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_Mixing(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;

    {
        if (CC_UNLIKELY(m_spMfb.get() == NULL)) {
            m_spMfb = IMfllMfb::createInstance();
            if (m_spMfb.get() == NULL) {
                mfllLogE("%s: create MFB instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
        }

        /* set sensor ID and private data for sync */
        err = m_spMfb->init(m_sensorId);
        m_spMfb->setShotMode(m_shotMode);
        m_spMfb->setPostNrType(m_postNrType);

        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: init MFB instance failed with code %d", __FUNCTION__, (int)err);
            goto lbExit;
        }

        /* mixing */
        mfllLogD("%s: do mixing", __FUNCTION__);
        if (CC_UNLIKELY(m_imgYuvThumbnail == nullptr)) {
            mfllLogW("%s: no thumbnail yuv", __FUNCTION__);
            err = m_spMfb->mix(
                m_ptrImgYuvBlended->imgbuf.get(),
                m_ptrImgYuvGolden->imgbuf.get(),
                m_ptrImgYuvMixed->imgbuf.get(),
                m_ptrImgWeightingFinal->imgbuf.get()
            );
        }
        else {
            err = m_spMfb->mix(
                m_ptrImgYuvBlended->imgbuf.get(),
                m_ptrImgYuvGolden->imgbuf.get(),
                m_ptrImgYuvMixed->imgbuf.get(),
                m_ptrImgYuvThumbnail->imgbuf.get(),
                m_ptrImgWeightingFinal->imgbuf.get(),
                m_thumbnailCropRgn
            );
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocRawBuffer(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;

    {
        std::lock_guard<std::mutex> __l(m_imgRaws[index].locker);

        /* create IMfllImageBuffer instances */
        sp<IMfllImageBuffer> pImg = m_imgRaws[index].imgbuf;
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("raw_", index));
            if (CC_UNLIKELY(pImg == NULL)) {
                mfllLogE("%s: create IMfllImageBuffer(%d) failed", __FUNCTION__, index);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgRaws[index].imgbuf = pImg;
        }

        /* if not init, init it */
        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Raw10);
            pImg->setResolution(m_width, m_height);
            err = pImg->initBuffer();

            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init raw buffer(%d) failed", __FUNCTION__, index);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
        }
        else {
            mfllLogD("%s: raw buffer %d is inited, ignore here", __FUNCTION__, index);
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocQyuvBuffer(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;

    {
        // notice that, we cannot invoke any locked method of ImageBufferPack here
        std::lock(m_imgYuvs[index].locker, m_imgQYuvs[index].locker);
        std::lock_guard<std::mutex> __l1(m_imgYuvs[index].locker, std::adopt_lock);
        std::lock_guard<std::mutex> __l2(m_imgQYuvs[index].locker, std::adopt_lock);

        sp<IMfllImageBuffer> pImg = m_imgQYuvs[index].imgbuf;
        sp<IMfllImageBuffer> pFullSizeImg = m_imgYuvs[index].imgbuf;

        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("qyuv_", index), Flag_QuarterSize);
            if (CC_UNLIKELY(pImg == NULL)) {
                mfllLogE("%s: create QYUV buffer instance (%d) failed", __FUNCTION__, index);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgQYuvs[index].imgbuf = pImg;
        }

        /* full size YUV (if necessary) */
        if (m_isFullSizeMc && pFullSizeImg == NULL) {
            pFullSizeImg = IMfllImageBuffer::createInstance(BUFFER_NAME("fyuv_", index), Flag_FullSize);
            if (CC_UNLIKELY(pFullSizeImg == NULL)) {
                mfllLogE("%s: create full size YUV buffer instance (%d) failed", __FUNCTION__, index);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvs[index].imgbuf = pFullSizeImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(m_isFullSizeMc ? ImageFormat_Y8 : ImageFormat_I422);
            pImg->setAligned(16, 16);
            pImg->setResolution(m_qwidth, m_qheight);
            err = pImg->initBuffer();
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init QYUV buffer(%d) failed", __FUNCTION__, index);
                goto lbExit;
            }
        }
        else {
            mfllLogD("%s: QYuv %d has been inited, ignored here", __FUNCTION__, index);
            // only half size needs to convert image
            if (!m_isFullSizeMc) {
                pImg->setAligned(16, 16);
                pImg->convertImageFormat(ImageFormat_I422);
            }
        }

        /* full size YUV */
        if (m_isFullSizeMc && pFullSizeImg->isInited() == false) {
            pFullSizeImg->setImageFormat(ImageFormat_I422);
            pFullSizeImg->setAligned(16, 16);
            pFullSizeImg->setResolution(m_width, m_height);
            err = pFullSizeImg->initBuffer();
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init full size YUV buffer (%d) failed", __FUNCTION__, index);
                goto lbExit;
            }
        }
        /* full size YUV, and it's been inited */
        else if (m_isFullSizeMc){
            mfllLogD("%s: full yuv %d has been inited, ignored here", __FUNCTION__, index);
            pFullSizeImg->setAligned(16, 16);
            pFullSizeImg->convertImageFormat(ImageFormat_I422);
        }
        else {
            /* not full size, don't care */
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocYuvBase(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;
    {
        std::lock_guard<std::mutex> __l(m_imgYuvBase.locker);

        sp<IMfllImageBuffer> pImg = m_imgYuvBase.imgbuf;
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("yuv_base", 0), Flag_FullSize);
            if (CC_UNLIKELY(pImg == NULL)) {
                mfllLogE("%s: create YUV base buffer instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvBase.imgbuf = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Yuy2);
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(16, 16); // always uses 16 pixels aligned
            err = pImg->initBuffer();

            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init YUV base buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(ImageFormat_Yuy2);
        }
        else {
            if (pImg->getImageFormat() != ImageFormat_Yuy2) {
                pImg->setAligned(2, 2);
                pImg->convertImageFormat(ImageFormat_Yuy2);
            }
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocYuvGolden(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;

    {
        std::lock_guard<std::mutex> __l(m_imgYuvGolden.locker);

        sp<IMfllImageBuffer> pImg = m_imgYuvGolden.imgbuf;
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("yuv_golden", 0), Flag_FullSize);
            if (CC_UNLIKELY(pImg == NULL)) {
                mfllLogE("%s: create YUV golden instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvGolden.imgbuf = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Yuy2);
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(16, 16);
            err = pImg->initBuffer();
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init YUV golden buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            /* re-map the IImageBuffers due to aligned changed */
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(ImageFormat_Yuy2);
        }
        else {
            if (pImg->getImageFormat() != ImageFormat_Yuy2) {
                pImg->setAligned(2, 2);
                pImg->convertImageFormat(ImageFormat_Yuy2);
            }
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocYuvWorking(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;

    {
        std::lock_guard<std::mutex> __l(m_imgYuvBlended.locker);

        sp<IMfllImageBuffer> pImg = m_imgYuvBlended.imgbuf;
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("yuv_blended",0), Flag_FullSize);
            if (pImg == NULL) {
                mfllLogE("%s: create YUV blended buffer instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvBlended.imgbuf = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Yuy2);
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(16, 16);
            err = pImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init YUV blended buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(ImageFormat_Yuy2);
        }
        else {
            if (pImg->getImageFormat() != ImageFormat_Yuy2) {
                pImg->setAligned(2, 2);
                pImg->convertImageFormat(ImageFormat_Yuy2);
            }
            mfllLogD("%s: yuv blending(working) is inited, ignored here", __FUNCTION__);
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocYuvMcWorking(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;

    {
        std::lock_guard<std::mutex> __l(m_imgYuvMcWorking.locker);

        sp<IMfllImageBuffer> pImg = m_imgYuvMcWorking.imgbuf;

        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance("yuv_mc_working", m_isFullSizeMc ? Flag_FullSize : Flag_QuarterSize);
            if (pImg == NULL) {
                mfllLogE("%s: create YUV MC working buffer instance failed", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgYuvMcWorking.imgbuf = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Yuy2);
            if (m_isFullSizeMc) {
                pImg->setResolution(m_width, m_height);
            }
            else
                pImg->setResolution(m_qwidth, m_qheight);
            pImg->setAligned(16, 16);
            err = pImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init YUV MC working buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(ImageFormat_Yuy2);
        }
        else {
            if (pImg->getImageFormat() != ImageFormat_Yuy2) {
                pImg->setAligned(2, 2);
                pImg->convertImageFormat(ImageFormat_Yuy2);
            }
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocWeighting(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;

    {
        std::lock_guard<std::mutex> __l(m_imgWeighting[index].locker);
        sp<IMfllImageBuffer> pImg = m_imgWeighting[index].imgbuf;
        if (CC_UNLIKELY(pImg == NULL)) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("wt_", index), Flag_WeightingTable);
            if (pImg == NULL) {
                mfllLogE("%s: create weighting table(%d) buffer instance failed", __FUNCTION__, index);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgWeighting[index].imgbuf = pImg;
        }

        if (!pImg->isInited()) {
            pImg->setImageFormat(ImageFormat_Raw8);
            pImg->setResolution(m_width, m_height);
            err = pImg->initBuffer();
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init weighting table(%d) buffer failed", __FUNCTION__, index);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
        }
        else {
            mfllLogD("%s: weighting table %d is inited, ignored here", __FUNCTION__, index);
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocMemcWorking(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;

    {
        int bufferIndex = index % getMemcInstanceNum();

        std::lock_guard<std::mutex> __l(m_imgMemc[bufferIndex].locker);

        sp<IMfllImageBuffer> pImg = m_imgMemc[bufferIndex].imgbuf;

        if (CC_UNLIKELY(pImg == NULL)) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("memc_", bufferIndex), Flag_Algorithm);
            if (pImg == NULL) {
                mfllLogE("%s: create MEMC working buffer(%d) instance failed", __FUNCTION__, bufferIndex);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgMemc[bufferIndex].imgbuf = pImg;
        }

        if (!pImg->isInited()) {
            sp<IMfllMemc> memc = m_spMemc[bufferIndex];
            if (CC_UNLIKELY(memc.get() == NULL)) {
                mfllLogE("%s: memc instance(index %d) is NULL", __FUNCTION__, bufferIndex);
                err = MfllErr_NullPointer;
                goto lbExit;
            }
            size_t bufferSize = 0;
            err = memc->getAlgorithmWorkBufferSize(&bufferSize);
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: get algorithm working buffer size fail", __FUNCTION__);
                goto lbExit;
            }

            /* check if 2 bytes alignment */
            if (bufferSize % 2 != 0) {
                mfllLogW("%s: algorithm working buffer size is not 2 bytes alignment, make it is", __FUNCTION__);
                bufferSize += 1;
            }

            pImg->setImageFormat(ImageFormat_Raw8);
            pImg->setAligned(2, 2); // always using 2 bytes align
            pImg->setResolution(bufferSize/2, 2);
            err = pImg->initBuffer();
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init MEMC working buffer(%d) failed", __FUNCTION__, bufferIndex);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
        }
        else {
            sp<IMfllMemc> memc = m_spMemc[bufferIndex];
            if (CC_UNLIKELY(memc.get() == NULL)) {
                mfllLogE("%s: memc instance(index %d) is NULL", __FUNCTION__, bufferIndex);
                err = MfllErr_NullPointer;
                goto lbExit;
            }
            size_t bufferSize = 0;
            err = memc->getAlgorithmWorkBufferSize(&bufferSize);
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: get algorithm working buffer size fail", __FUNCTION__);
                goto lbExit;
            }
            mfllLogD("%s: memc buffer %d is inited, ignored here", __FUNCTION__, bufferIndex);
        }
    }

lbExit:
    return err;
}


}; // namespace mfll
#endif//__MFLLCORE_IMPL_INC__
