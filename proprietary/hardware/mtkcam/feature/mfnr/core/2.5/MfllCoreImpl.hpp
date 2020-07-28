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

        int32_t _MeDnRatio = m_spMemc[i]->getMeDnRatio();
        mfllLogD3("MeDnRatio:%d", _MeDnRatio);
        /* update m_qwidth and m_qheight to quater of capture resolution */
        // TODO: integrate new MEMC algo for downscale config.
        m_qwidth = m_imgoWidth / _MeDnRatio;
        m_qheight = m_imgoHeight / _MeDnRatio;
        mfllLogD3("(m_qwidth x m_qheight) = (%d x %d)", m_qwidth, m_qheight);

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
            mfllLogD3("set algo threads priority to %d", _priority);
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
            mfllLogD3("%s: create MfllCapturer", __FUNCTION__);
            m_spCapturer = IMfllCapturer::createInstance();
            if (m_spCapturer.get() == NULL) {
                mfllLogE("%s: create MfllCapturer instance", __FUNCTION__);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
        }

        std::vector< sp<IMfllImageBuffer>* > raws;
        std::vector< sp<IMfllImageBuffer>* > rrzos;
        std::vector<MfllMotionVector_t*>     gmvs;
        std::vector< int >                   rStatus;

        /* resize first */
        raws    .resize(getCaptureFrameNum());
        rrzos   .resize(getCaptureFrameNum());
        gmvs    .resize(getCaptureFrameNum());
        rStatus .resize(getCaptureFrameNum());

        /* prepare output buffer */
        for (int i = 0; i < (int)getCaptureFrameNum(); i++) {
            raws[i] = &m_imgRaws[i].imgbuf;
            rrzos[i] = &m_imgRrzos[i].imgbuf;
            gmvs[i] = &m_globalMv[i];
            rStatus[i] = 0;
        }

        /* register event dispatcher and set MfllCore instance */
        m_spCapturer->setMfllCore((IMfllCore*)this);
        err = m_spCapturer->registerEventDispatcher(m_event);

        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: MfllCapture::registerEventDispatcher failed with code %d", __FUNCTION__, err);
            goto lbExit;
        }

        /* Catpure frames */
        mfllLogD3("Capture frames!");

        err = m_spCapturer->captureFrames(getCaptureFrameNum(), raws, rrzos, gmvs, rStatus);
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
            std::vector<ImageBufferPack>     z; // rrzo buffer;
            std::vector<MfllMotionVector_t>  m; // motion

            r.resize(getCaptureFrameNum());
            z.resize(getCaptureFrameNum());
            m.resize(getCaptureFrameNum());

            size_t okCount = 0; // counting ok frame numbers

            /* check the status from Capturer, if status is 0 means ok */
            for (size_t i = 0, j = 0; i < (size_t)getCaptureFrameNum(); i++) {
                /* If not failed, save to stack */
                if (rStatus[i] == 0) {
                    r[j] = m_imgRaws[i];
                    z[j] = m_imgRrzos[i];
                    m[j] = m_globalMv[i];
                    okCount++;
                    j++;
                }
            }

            m_dbgInfoCore.frameCapture = okCount;
            m_dbgInfoCore.frameMaxCapture = getCaptureFrameNum();
            m_dbgInfoCore.frameMaxBlend = getBlendFrameNum();

            mfllLogD3("capture done, ok count=%zu, num of capture = %u",
                    okCount, getCaptureFrameNum());

            m_caputredCount = okCount;

            /* if not equals, something wrong */
            if (okCount != (size_t)getCaptureFrameNum()) {
                /* sort available buffers continuously */
                for (size_t i = 0; i < okCount; i++) {
                    m_imgRaws[i] = r[i];
                    m_imgRrzos[i] = z[i];
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
                        m_bypass.bypassEncodeQYuv[i] = 1;
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

        bss->setUniqueKey(m_uniqueKey);

        std::vector< sp<IMfllImageBuffer> > imgs;
        std::vector< sp<IMfllImageBuffer> > rrzos;
        std::vector< MfllMotionVector_t >   mvs;
        std::vector< int64_t > tss = m_timestampSync;

        for (int i = 0; i < bssFrameCount; i++) {
            imgs.push_back(m_imgRaws[i].imgbuf);
            rrzos.push_back(m_imgRrzos[i].imgbuf);
            mvs.push_back(m_globalMv[i]);
        }

        tss.resize(imgs.size(), -1);

        mfllLogD3("%s: RAW domain BSS", __FUNCTION__);
        std::vector<int> newIndex = bss->bss(rrzos, mvs, tss);
        if (newIndex.size() <= 0) {
            mfllLogE("%s: do bss failed", __FUNCTION__);
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }

        // sort items
        std::deque<ImageBufferPack> newRaws;
        std::deque<ImageBufferPack> newRrzos;
        for (int i = 0; i < bssFrameCount; i++) {
            int index = newIndex[i]; // new index
            newRaws.push_back(m_imgRaws[index]);
            newRrzos.push_back(m_imgRrzos[index]);
            m_bssIndex[i] = index;
            mfllLogD3("%s: new index (%u)-->(%d)", __FUNCTION__, i, index);
            /**
             *  mvs will be sorted by Bss, we don't need to re-sort it again,
             *  just update it
             */
            m_globalMv[i] = mvs[i];
        }
        for (int i = 0; i < bssFrameCount; i++) {
            m_imgRaws[i] = newRaws[i];
            m_imgRrzos[i] = newRrzos[i];
        }

        // check frame to skip
        size_t frameNumToSkip = bss->getSkipFrameCount();

        // due to adb force drop
        int forceDropNum   = MfllProperty::getDropNum();
        if (CC_UNLIKELY(forceDropNum > -1)) {
            mfllLogD("%s: force drop frame count = %d, original bss drop frame = %zu",
                    __FUNCTION__, forceDropNum, frameNumToSkip);

            frameNumToSkip = static_cast<size_t>(forceDropNum);
        }

        // due to capture M and blend N
        int frameNumToSkipBase = static_cast<int>(getCaptureFrameNum()) - static_cast<int>(getBlendFrameNum());
        if (CC_UNLIKELY(frameNumToSkip < frameNumToSkipBase)) {
            mfllLogD("%s: update drop frame count = %d, original drop frame = %zu",
                    __FUNCTION__, frameNumToSkipBase, frameNumToSkip);

            frameNumToSkip = static_cast<size_t>(frameNumToSkipBase);
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
                idxForImages = 0;
            }
            mfllLogD("%s: skip frame count = %zu", __FUNCTION__, frameNumToSkip);

            if (idxForOperation <= 0) { // ignore all blending
                mfllLogW("%s: BSS drops all frames, using single capture", __FUNCTION__);
                // ignore encoding YUV base due to no need to blend
                m_bypass.bypassEncodeYuvBase = 1;
                m_bypass.bypassMixing = 1;
                m_bAsSingleFrame = 1;
                if (m_downscaleRatio > 0) { // if support downscale
                    if (m_bDoDownscale == false) {
                        m_width = m_width/m_downscaleRatio;
                        m_height = m_height/m_downscaleRatio;

                        /* update debug info */
                        m_dbgInfoCore.blend_yuv_width = m_width;
                        m_dbgInfoCore.blend_yuv_height = m_height;
                    }
                    m_bDoDownscale = true;
                    mfllLogI("%s: use downscale yuv size(%zu, %zu) to blend for single capture", __FUNCTION__, m_width, m_height);
                }
            }

            for (size_t i = static_cast<size_t>(idxForOperation); i < MFLL_MAX_FRAMES; i++) {
                if (i >= static_cast<size_t>(idxForOperation)) {
                    mfllLogD("%s: bypass MEMC/BLD (index = %zu)", __FUNCTION__, i);
                    m_bypass.bypassMotionEstimation[i] = 1;
                    m_bypass.bypassMotionCompensation[i] = 1;
                    m_bypass.bypassBlending[i] = 1;
                }
            }

            for (size_t i = static_cast<size_t>(idxForImages); i < MFLL_MAX_FRAMES; i++) {
                if (i >= static_cast<size_t>(idxForImages)) {
                    mfllLogD("%s: bypass encodeRawToYuv (index = %zu)", __FUNCTION__, i);
                    m_bypass.bypassEncodeQYuv[i] = 1;
                }
            }
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_EncodeQYuv(void *void_index)
{
    enum MfllErr err = MfllErr_Ok;

    const unsigned int index = (unsigned int)(long)void_index;

    if (CC_UNLIKELY(index >= MFLL_MAX_FRAMES)){
        mfllLogE("index:%d is out of frames index:%d, ignore it", index, MFLL_MAX_FRAMES);
        return MfllErr_BadArgument;
    }
    else {
        /* handle RAW to YUV frames */
        if ( m_imgRaws[index].imgbuf.get() ) {
            mfllLogD3("Convert RAW to YUV");

            // check if full yuv exists
            bool bFullYuv = m_imgYuvs[index].imgbuf.get() ? true : false;

            /* capture YUV */
            /* no raw or ignored */
            err = doAllocQyuvBuffer((void*)(long)index);

            // check if buffers are ready to use && qyuvs[index] exists
            bool bBuffersReady = ( (err == MfllErr_Ok) && m_imgQYuvs[index].imgbuf.get() );

            if (CC_UNLIKELY( ! bBuffersReady )) {
                mfllLogE("%s: alloc QYUV buffer %zu failed", __FUNCTION__, index);
                goto lbExit;
            }
            else {
                if (CC_UNLIKELY(m_spMfb.get() == NULL)) {
                    mfllLogD3("%s: create IMfllMfb instance", __FUNCTION__);
                    m_spMfb = IMfllMfb::createInstance();
                    if (m_spMfb.get() == NULL) {
                        mfllLogE("%s: m_spMfb is NULL", __FUNCTION__);
                        err = MfllErr_CreateInstanceFailed;
                        goto lbExit;
                    }
                }

                /* tell MfllMfb what shot mode and post NR type is using */
                err = m_spMfb->init(m_sensorId);
                m_spMfb->setShotMode(m_shotMode);
                m_spMfb->setPostNrType(m_postNrType);

                if (bFullYuv) {
                    mfllLogD3("%s: RAW to 2 YUVs (index = %d), %dx%d, %dx%d.", __FUNCTION__, index,
                            m_imgYuvs[index].imgbuf->getWidth(), m_imgYuvs[index].imgbuf->getHeight(),
                            m_imgQYuvs[index].imgbuf->getWidth(), m_imgQYuvs[index].imgbuf->getHeight());

                    /**
                     *  if we've already known that it's single capture, we don't need
                     *  stage BFBLD because we use stage SF to generate the output.
                     */
                    if ( getCaptureFrameNum() > 1) {
                        if (retrieveBuffer(MfllBuffer_PostviewYuv).get()) {
                            if (m_bDoDownscale && m_downscaleRatio > 0) {
                                err = m_spMfb->encodeRawToYuv(
                                    m_imgRaws[index].imgbuf.get(),  // source
                                    m_imgYuvs[index].imgbuf.get(),  // output
                                    m_imgQYuvs[index].imgbuf.get(), // output2
                                    MfllRect_t(), // crop for output, no need cropping
                                    MfllRect_t(), // crop for output2, no need cropping
                                    YuvStage_RawToYv16
                                    );

                                MfllRect_t postviewCropRgn = m_postviewCropRgn;
                                mfllLogD("%s: enable downscale, crop region before downscale(x:%zu, y:%zu, w:%zu, h:%zu)",
                                        __FUNCTION__, m_postviewCropRgn.x, m_postviewCropRgn.y, m_postviewCropRgn.w, m_postviewCropRgn.h);
                                postviewCropRgn.x = m_postviewCropRgn.x/m_downscaleRatio;
                                postviewCropRgn.y = m_postviewCropRgn.y/m_downscaleRatio;
                                postviewCropRgn.w = m_postviewCropRgn.w/m_downscaleRatio;
                                postviewCropRgn.h = m_postviewCropRgn.h/m_downscaleRatio;
                                mfllLogD("%s: enable downscale, crop region after downscale(x:%zu, y:%zu, w:%zu, h:%zu)",
                                        __FUNCTION__, postviewCropRgn.x, postviewCropRgn.y, postviewCropRgn.w, postviewCropRgn.h);

                                /* generate postview by MDP */
                                err = m_spMfb->convertYuvFormatByMdp(
                                    m_imgYuvs[index].imgbuf.get(),  // input
                                    retrieveBuffer(MfllBuffer_PostviewYuv).get(), // output1
                                    NULL, // output2
                                    postviewCropRgn, // crop for output1
                                    MfllRect_t(), // crop for output2, no need cropping
                                    YuvStage_Unknown
                                    );
                            }
                            else {
                                err = m_spMfb->encodeRawToYuv(
                                    m_imgRaws[index].imgbuf.get(),  // source
                                    m_imgYuvs[index].imgbuf.get(),  // output
                                    m_imgQYuvs[index].imgbuf.get(), // output2
                                    retrieveBuffer(MfllBuffer_PostviewYuv).get(), // output3
                                    MfllRect_t(), // crop for output2, no need cropping
                                    m_postviewCropRgn, // crop for output3
                                    YuvStage_RawToYv16
                                    );
                            }
                        }
                        else {
                            err = m_spMfb->encodeRawToYuv(
                                m_imgRaws[index].imgbuf.get(),
                                m_imgYuvs[index].imgbuf.get(),
                                m_imgQYuvs[index].imgbuf.get(),
                                MfllRect_t(), // no need cropping
                                MfllRect_t(), // no need cropping
                                YuvStage_RawToYv16
                                );
                        }
                    }

                    if (CC_UNLIKELY(err != MfllErr_Ok)) {
                        mfllLogE("%s: Encode RAW to YUV fail", __FUNCTION__);
                        goto lbExit;
                    }
                }
                else {
                    /**
                     *  if we've already known that it's single capture, we don't need
                     *  stage BFBLD because we use stage SF to generate the output.
                     */
                    if ( getCaptureFrameNum() > 1) {
                        err = m_spMfb->encodeRawToYuv(
                            m_imgRaws[index].imgbuf.get(),
                            m_imgQYuvs[index].imgbuf.get(),
                            retrieveBuffer(MfllBuffer_PostviewYuv).get(),
                            MfllRect_t(),
                            m_postviewCropRgn,
                            YuvStage_RawToYv16);
                    }

                    if (CC_UNLIKELY(err != MfllErr_Ok)) {
                        mfllLogE("%s: Encode RAW to YUV fail", __FUNCTION__);
                        goto lbExit;
                    }
                }
            }
        }
    }

lbExit:
    if (CC_UNLIKELY(err != MfllErr_Ok)) {
        mfllLogD("%s: bypass MEMC/BLD (index = %zu)", __FUNCTION__, index);
        m_bypass.bypassMotionEstimation[index] = 1;
        m_bypass.bypassMotionCompensation[index] = 1;
        m_bypass.bypassBlending[index] = 1;
    }

    // clear image
    if (index != 0) {
        mfllLogD3("%s: clear images (index = %zu)", __FUNCTION__, index);

        // release RAW image buffer
        m_imgRrzos[index].releaseBufLocked();
        m_imgRaws[index].releaseBufLocked();
    }
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
        mfllLogD3("%s: encode base raw to yuv", __FUNCTION__);

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
                mfllLogD3("%s: convertYuvFormatByMdp returns fail, try to use P2", __FUNCTION__);
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

enum MfllErr MfllCore::do_EncodeYuvGoldenDownscale(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;

    {
        err = doAllocQyuvBuffer((void*)(long)0);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: allocate YUV for downscale golden failed", __FUNCTION__);
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

        /* handle RAW to YUV frames */
        if ( m_imgRaws[0].imgbuf.get() == nullptr ||
             m_imgYuvs[0].imgbuf.get() == nullptr ) {
            mfllLogE("%s: in-raw(%p)/out-yuv(%p) buffer is NULL, cannot encode",
                    __FUNCTION__, m_imgRaws[0].imgbuf.get(), m_imgYuvs[0].imgbuf.get());
            err = MfllErr_UnexpectedError;
            goto lbExit;
        }
        else{
            mfllLogD3("%s: Convert RAW to YUV, use single frame profile", __FUNCTION__);
            // update shot mode to Zsd+SingleFrame
            m_spMfb->setShotMode(static_cast<enum MfllMode>(m_shotMode | (1 << MfllMode_Bit_SingleFrame)));

            {
                mfllLogD("%s: convert downscale golden YUV src size(%d, %d)", __FUNCTION__, m_width, m_height);
                auto pImgDownscaleGoldenYuv = m_imgYuvs[0].imgbuf.get();
                pImgDownscaleGoldenYuv->setResolution(m_width, m_height);
                pImgDownscaleGoldenYuv->setAligned(2, 2);
                pImgDownscaleGoldenYuv->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_FULL);
            }

            err = m_spMfb->encodeRawToYuv(
                    m_imgRaws[0].imgbuf.get(),  // use RAW as input for single frame
                    m_imgYuvs[0].imgbuf.get(),
                    YuvStage_GoldenYuy2
                    );

            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: Encode RAW to YUV fail", __FUNCTION__);
                goto lbExit;
            }
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
        MfllRect_t postviewCropRgn = m_postviewCropRgn;
        if (m_bypass.bypassMixing && !m_bDoDownscale) {
            // update shot mode to Zsd+SingleFrame
            mfllLogD3("%s: use single frame profile", __FUNCTION__);
            m_spMfb->setShotMode(static_cast<enum MfllMode>(m_shotMode | (1 << MfllMode_Bit_SingleFrame)));
            imgSrc = m_imgRaws[0].imgbuf;
        }
        // If using downscale denoise flow and single frame capture,
        // we have to convert output buffer size to full image resolution
        else if (m_bypass.bypassMixing && m_bDoDownscale) {
            imgSrc = m_imgYuvs[0].imgbuf;
            // convert output buffer image size to full image resolution
            auto ig = m_imgYuvGolden.imgbuf.get();
            ig->setResolution(m_imgoWidth, m_imgoHeight);
            ig->setAligned(2, 2);
            ig->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_GOLDEN);
        }
        else {
            imgSrc = m_imgYuvs[0].imgbuf;
        }

        if (m_bDoDownscale && m_downscaleRatio > 0) {
            mfllLogD("%s: enable downscale, crop region before downscale(x:%zu, y:%zu, w:%zu, h:%zu)",
                    __FUNCTION__, m_postviewCropRgn.x, m_postviewCropRgn.y, m_postviewCropRgn.w, m_postviewCropRgn.h);

            postviewCropRgn.x = m_postviewCropRgn.x/m_downscaleRatio;
            postviewCropRgn.y = m_postviewCropRgn.y/m_downscaleRatio;
            postviewCropRgn.w = m_postviewCropRgn.w/m_downscaleRatio;
            postviewCropRgn.h = m_postviewCropRgn.h/m_downscaleRatio;
        }
        mfllLogD("%s: postview crop region(x:%zu, y:%zu, w:%zu, h:%zu)",
                __FUNCTION__, postviewCropRgn.x, postviewCropRgn.y, postviewCropRgn.w, postviewCropRgn.h);

        err = m_spMfb->encodeRawToYuv(
                imgSrc.get(),
                m_imgYuvGolden.imgbuf.get(),
                m_imgYuvPostview.imgbuf.get(),
                MfllRect_t(),
                postviewCropRgn,
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
            mfllLogD3("%s: ignored motion compensation & blending", __FUNCTION__);
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
            /* convert image format without alignment for blending or mixing */
            pMcRef->imgbuf->setAligned(2, 2);
            pMcRef->imgbuf->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_MC_WORKING);
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

        err = doAllocYuvBase(NULL);
        if (CC_UNLIKELY(err != MfllErr_Ok)) {
            mfllLogE("%s: allocate YUV base buffer failed", __FUNCTION__);
            goto lbExit;
        }
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
            mfllLogD3("%s: create IMfllMfb instance", __FUNCTION__);
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
        mfllLogD3("%s: do blending now", __FUNCTION__);

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
        mfllLogD3("%s: do mixing", __FUNCTION__);
        if ( CC_LIKELY(m_ptrImgYuvThumbnail != nullptr) &&
             CC_LIKELY(m_ptrImgYuvThumbnail->imgbuf.get() != nullptr) ){

            if (m_bDoDownscale && m_downscaleRatio > 1) {
                mfllLogD("%s: enable downscale, crop region before downscale(x:%zu, y:%zu, w:%zu, h:%zu)",
                        __FUNCTION__, m_thumbnailCropRgn.x, m_thumbnailCropRgn.y, m_thumbnailCropRgn.w, m_thumbnailCropRgn.h);

                m_thumbnailCropRgn.x = m_thumbnailCropRgn.x/m_downscaleRatio;
                m_thumbnailCropRgn.y = m_thumbnailCropRgn.y/m_downscaleRatio;
                m_thumbnailCropRgn.w = m_thumbnailCropRgn.w/m_downscaleRatio;
                m_thumbnailCropRgn.h = m_thumbnailCropRgn.h/m_downscaleRatio;
            }
            mfllLogD("%s: thumbnail crop region(x:%zu, y:%zu, w:%zu, h:%zu)",
                    __FUNCTION__, m_thumbnailCropRgn.x, m_thumbnailCropRgn.y, m_thumbnailCropRgn.w, m_thumbnailCropRgn.h);

            err = m_spMfb->mix(
                m_ptrImgYuvBlended->imgbuf.get(),
                m_ptrImgYuvGolden->imgbuf.get(),
                m_ptrImgYuvMixed->imgbuf.get(),
                m_ptrImgYuvThumbnail->imgbuf.get(),
                m_ptrImgWeightingFinal->imgbuf.get(),
                m_thumbnailCropRgn
            );
            mfllLogD("%s: thumbnail has been generated during mixing stage", __FUNCTION__);
            m_bThumbnailDone = true;
        }
        else {
            mfllLogW("%s: no thumbnail yuv", __FUNCTION__);
            err = m_spMfb->mix(
                m_ptrImgYuvBlended->imgbuf.get(),
                m_ptrImgYuvGolden->imgbuf.get(),
                m_ptrImgYuvMixed->imgbuf.get(),
                m_ptrImgWeightingFinal->imgbuf.get()
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
            pImg->setImageFormat(MFLL_IMAGE_FORMAT_RAW);
            pImg->setResolution(m_imgoWidth, m_imgoHeight);
            err = pImg->initBuffer();

            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init raw buffer(%d) failed", __FUNCTION__, index);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
        }
        else {
            mfllLogD3("%s: raw buffer %d is inited, ignore here", __FUNCTION__, index);
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocRrzoBuffer(void *void_index)
{
    const unsigned int index = (unsigned int)(long)void_index;
    enum MfllErr err = MfllErr_Ok;

    {
        std::lock_guard<std::mutex> __l(m_imgRrzos[index].locker);

        /* create IMfllImageBuffer instances */
        sp<IMfllImageBuffer> pImg = m_imgRrzos[index].imgbuf;
        if (pImg == NULL) {
            pImg = IMfllImageBuffer::createInstance(BUFFER_NAME("rrzo_", index));
            if (CC_UNLIKELY(pImg == NULL)) {
                mfllLogE("%s: create IMfllImageBuffer(%d) failed", __FUNCTION__, index);
                err = MfllErr_CreateInstanceFailed;
                goto lbExit;
            }
            m_imgRrzos[index].imgbuf = pImg;
        }

        /* if not init, init it */
        if (!pImg->isInited()) {
            pImg->setImageFormat(MFLL_IMAGE_FORMAT_RAW);
            pImg->setResolution(m_rrzoWidth, m_rrzoHeight);
            err = pImg->initBuffer();

            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init raw buffer(%d) failed", __FUNCTION__, index);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
        }
        else {
            mfllLogD3("%s: raw buffer %d is inited, ignore here", __FUNCTION__, index);
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

        /* check we need resized image (m_qwidth by m_qheight) or not */
        bool bNeedResizedImg = _isNeedResizedYBuffer();

        sp<IMfllImageBuffer> pImg = m_imgQYuvs[index].imgbuf;
        sp<IMfllImageBuffer> pFullSizeImg = m_imgYuvs[index].imgbuf;

        if (bNeedResizedImg && (pImg == NULL)) {
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

        /* Resized Y8 images */
        if (bNeedResizedImg) {
            if (!pImg->isInited()) {
                pImg->setImageFormat(MFLL_IMAGE_FORMAT_YUV_Q);
                pImg->setAligned(16, 16);
                pImg->setResolution(m_qwidth, m_qheight);
                err = pImg->initBuffer();
                if (CC_UNLIKELY(err != MfllErr_Ok)) {
                    mfllLogE("%s: init QYUV buffer(%d) failed", __FUNCTION__, index);
                    goto lbExit;
                }
            }
            else {
                mfllLogD3("%s: QYUV buffer %d re-init(%dx%d)", __FUNCTION__, index, m_width, m_height);
                // only half size needs to convert image
                pImg->setAligned(16, 16);
                pImg->setResolution(m_qwidth, m_qheight);
                pImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_Q);
            }
        }

        /* full size YUV */
        if (m_isFullSizeMc && pFullSizeImg->isInited() == false) {
            pFullSizeImg->setImageFormat(MFLL_IMAGE_FORMAT_YUV_FULL);
            pFullSizeImg->setAligned(16, 16);
            pFullSizeImg->setResolution(m_imgoWidth, m_imgoHeight);
            err = pFullSizeImg->initBuffer();
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init full size YUV buffer (%d) failed", __FUNCTION__, index);
                goto lbExit;
            }
            pFullSizeImg->setResolution(m_width, m_height);
            pFullSizeImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_FULL);
        }
        /* full size YUV, and it's been inited */
        else if (m_isFullSizeMc){
            mfllLogD3("%s: full size YUV buffer %d re-init(%dx%d)", __FUNCTION__, index, m_width, m_height);
            pFullSizeImg->setAligned(16, 16);
            pFullSizeImg->setResolution(m_width, m_height);
            pFullSizeImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_FULL);
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
            pImg->setImageFormat(MFLL_IMAGE_FORMAT_YUV_BASE);
            pImg->setResolution(m_imgoWidth, m_imgoHeight); // always crate sensor size image
            pImg->setAligned(16, 16); // always uses 16 pixels aligned
            err = pImg->initBuffer();

            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init YUV base buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_BASE);
        }
        else {
            mfllLogD3("%s: YUV base buffer re-init(%dx%d)", __FUNCTION__, m_width, m_height);
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_BASE);
        }
    }

lbExit:
    return err;
}


enum MfllErr MfllCore::do_AllocYuvGolden(JOB_VOID)
{
    enum MfllErr err = MfllErr_Ok;

    // if capture downscale single frame image,
    // we use the golden stage as last full yuv output.
    bool is_full_size_golden = (m_bAsSingleFrame && m_bDoDownscale && (m_downscaleRatio > 0));

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
            pImg->setImageFormat(MFLL_IMAGE_FORMAT_YUV_GOLDEN);
            pImg->setResolution(m_imgoWidth, m_imgoHeight);
            pImg->setAligned(16, 16);
            err = pImg->initBuffer();
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init YUV golden buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            /* re-map the IImageBuffers due to aligned changed */
            pImg->setAligned(2, 2);
            pImg->setResolution(
                    is_full_size_golden ? m_imgoWidth  : m_width,
                    is_full_size_golden ? m_imgoHeight : m_height
                    );
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_GOLDEN);
        }
        else {
            unsigned int reinit_width = (is_full_size_golden) ? m_imgoWidth : m_width;
            unsigned int reinit_height = (is_full_size_golden) ? m_imgoHeight : m_height;
            mfllLogD3("%s: YUV golden buffer re-init(%dx%d)", __FUNCTION__, reinit_width, reinit_height);
            pImg->setAligned(2, 2);
            pImg->setResolution(reinit_width, reinit_height);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_GOLDEN);
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
            pImg->setImageFormat(MFLL_IMAGE_FORMAT_YUV_WORKING);
            pImg->setResolution(m_imgoWidth, m_imgoHeight);
            pImg->setAligned(16, 16);
            err = pImg->initBuffer();
            if (err != MfllErr_Ok) {
                mfllLogE("%s: init YUV blended buffer failed", __FUNCTION__);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_WORKING);
        }
        else {
            mfllLogD3("%s: YUV blended buffer re-init(%dx%d)", __FUNCTION__, m_width, m_height);
            pImg->setResolution(m_width, m_height);
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_WORKING);
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
            pImg->setImageFormat(MFLL_IMAGE_FORMAT_YUV_MC_WORKING);
            if (m_isFullSizeMc) {
                pImg->setResolution(m_imgoWidth, m_imgoHeight);
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
            /* convert image buffer to real size */
            pImg->setResolution(
                    (m_isFullSizeMc ? m_width  : m_qwidth),
                    (m_isFullSizeMc ? m_height : m_qheight)
                    );
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_MC_WORKING);
        }
        else {
            mfllLogD3("%s: YUV MC working buffer re-init(%dx%d)", __FUNCTION__, m_width, m_height);
            if (m_isFullSizeMc) {
                pImg->setResolution(m_width, m_height);
            }
            else {
                pImg->setResolution(m_qwidth, m_qheight);
            }
            pImg->setAligned(2, 2);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_YUV_MC_WORKING);
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
            pImg->setImageFormat(MFLL_IMAGE_FORMAT_WEIGHTING);
            pImg->setResolution(m_imgoWidth, m_imgoHeight);
            err = pImg->initBuffer();
            if (CC_UNLIKELY(err != MfllErr_Ok)) {
                mfllLogE("%s: init weighting table(%d) buffer failed", __FUNCTION__, index);
                err = MfllErr_UnexpectedError;
                goto lbExit;
            }
            /* convert image size to real width and height */
            pImg->setResolution(m_width, m_height);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_WEIGHTING);
        }
        else {
            mfllLogD3("%s: YUV weighting table(%d) buffer re-init(%dx%d)", __FUNCTION__, index, m_width, m_height);
            pImg->setResolution(m_width, m_height);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_WEIGHTING);
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

            pImg->setImageFormat(MFLL_IMAGE_FORMAT_MEMC_WORKING);
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
                mfllLogE("%s: get algorithm working(%d) buffer size fail", __FUNCTION__, bufferIndex);
                goto lbExit;
            }
            mfllLogD3("%s: MEMC working buffer %d re-init(%dx%d)", __FUNCTION__, bufferIndex, bufferSize/2, 2);
            pImg->setAligned(2, 2); // always using 2 bytes align
            pImg->setResolution(bufferSize/2, 2);
            pImg->convertImageFormat(MFLL_IMAGE_FORMAT_MEMC_WORKING);
        }
    }

lbExit:
    return err;
}


}; // namespace mfll
#endif//__MFLLCORE_IMPL_INC__
