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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.feature.mode.vsdof.photo;

import android.os.AsyncTask;

import com.mediatek.accessor.StereoInfoAccessor;
import com.mediatek.accessor.data.StereoCaptureInfo;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.feature.mode.vsdof.photo.device.ISdofPhotoDeviceController;

import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * A stereo data synth by stereo Jps/Clear image/
 * Mask/Ldc/DepthMap.
 */
public class StereoDataSynth {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(StereoDataSynth.class.getSimpleName());
    private static final int STEREO_CAPTURE_TIME = 0;
    private static final int STEREO_ORIGINAL_JPEG = 1;
    private static final int STEREO_JPS = 2;
    private static final int STEREO_MASK = 3;
    private static final int STEREO_DEPTH = 4;
    private static final int STEREO_CLEAR_IMAGE = 5;
    private static final int STEREO_LDC = 6;
    private static final int STEREO_N3D = 7;
    private static final int STEREO_DEPTH_WRAPPER = 8;
    private static final String CLEAR_IMAGE = "ci";
    private static final String BOKEH_IMAGE = "bi";
    private static final String MTK_BOKEH_DEPTH = "mbd";
    private static final String MTK_DEBUG_BUFFER = "mdb";
    private static final String MTK_BOKEH_META = "mbm";
    private static final String MTK_DEPTH_WRAPPER = "mdw";
    private static final String LDC = "ldc";

    private ConcurrentLinkedQueue<CaptureTime> mCaptureTimeQueue = new ConcurrentLinkedQueue<>();
    private ConcurrentLinkedQueue<OriginalJpegData> mOriginalJpegQueue
            = new ConcurrentLinkedQueue<>();
    private ConcurrentLinkedQueue<JpsData> mJpsQueue = new ConcurrentLinkedQueue<>();
    private ConcurrentLinkedQueue<MaskAndConfigData> mMaskQueue = new ConcurrentLinkedQueue<>();
    private ConcurrentLinkedQueue<DepthData> mDepthQueue = new ConcurrentLinkedQueue<>();
    private ConcurrentLinkedQueue<ClearImageData> mClearImageQueue = new ConcurrentLinkedQueue<>();
    private ConcurrentLinkedQueue<LdcData> mLdcQueue = new ConcurrentLinkedQueue<>();
    private ConcurrentLinkedQueue<DepthWrapper> mDepthWrapperQueue = new ConcurrentLinkedQueue<>();
    private ConcurrentLinkedQueue<N3dData> mN3dQueue = new ConcurrentLinkedQueue<>();
    private StereoInfoAccessor mAccessor;
    private XmpCompoundTask mXmpCompoundTask;
    private CompoundJpegListener mCompoundJpegListener;
    private AllCallbackReadyListener mAllCallbackReadyListener;
    private String mCallbackString;

    /**
     *
     */
    public StereoDataSynth() {
        mAccessor = new StereoInfoAccessor();
    }

    /**
     * the interface notify when jpeg compound completed.
     */
    public interface CompoundJpegListener {
        /**
         * notified others when compound completed.
         *
         * @param xmpJpeg     The compound jpeg.
         * @param captureTime the capture time.
         */
        void onXmpCompoundJpeg(byte[] xmpJpeg, long captureTime);
    }

    /**
     * add jpeg compound listener mode jpeg is compound.
     *
     * @param listener the use listener.
     */
    public void addCompoundJpegListener(CompoundJpegListener listener) {
        mCompoundJpegListener = listener;
    }

    /**
     * the interface notify when all stereo data callback.
     */
    public interface AllCallbackReadyListener {
        /**
         * notified all callbacks ready.
         */
        void onAllCallbackReady();
    }

    /**
     * add all callback ready listener.
     *
     * @param listener the use listener.
     */
    public void addAllCbReadyListener(AllCallbackReadyListener listener) {
        mAllCallbackReadyListener = listener;
    }

    /**
     * set device ctrl for receive ex stereo data.
     *
     * @param deviceCtrl the use listener.
     */
    public void setDeviceCtrl(ISdofPhotoDeviceController deviceCtrl) {
        if (deviceCtrl != null) {
            deviceCtrl.setExStereoDataCallback(mExStereoDataCallback);
            deviceCtrl.setStereoDataSynth(this);
        }
    }

    /**
     * set callback info.
     *
     * @param callbackInfo the stereo capture callback info,
     *                     ex, "ci,bi,mdb,mbd,mbm,ldc"
     *                     ci = clean image
     *                     bi = bokeh image
     *                     mdb = mtk debug buffer
     *                     mbd = mtk bokeh depth
     *                     mbm = mtk bokeh metadata
     *                     ldc = ldc
     */
    public void setCallbackInfo(String callbackInfo) {
        mCallbackString = callbackInfo;
    }

    private void addStereoDatas(int dataType, byte[] data, long time) {
        switch (dataType) {
            case STEREO_CAPTURE_TIME:
                CaptureTime captureTime = new CaptureTime(time);
                synchronized (mCaptureTimeQueue) {
                    mCaptureTimeQueue.add(captureTime);
                }
                break;
            case STEREO_ORIGINAL_JPEG:
                OriginalJpegData originalJpegData = new OriginalJpegData(data);
                synchronized (mOriginalJpegQueue) {
                    mOriginalJpegQueue.add(originalJpegData);
                }
                break;
            case STEREO_JPS:
                JpsData jpsData = new JpsData(data);
                synchronized (mJpsQueue) {
                    mJpsQueue.add(jpsData);
                }
                break;
            case STEREO_MASK:
                MaskAndConfigData maskAndConfigData = new MaskAndConfigData(data);
                synchronized (mMaskQueue) {
                    mMaskQueue.add(maskAndConfigData);
                }
                break;
            case STEREO_DEPTH:
                DepthData depthData = new DepthData(data);
                synchronized (mDepthQueue) {
                    mDepthQueue.add(depthData);
                }
                break;
            case STEREO_CLEAR_IMAGE:
                ClearImageData clearImageData = new ClearImageData(data);
                synchronized (mClearImageQueue) {
                    mClearImageQueue.add(clearImageData);
                }
                break;
            case STEREO_LDC:
                LdcData ldcData = new LdcData(data);
                synchronized (mLdcQueue) {
                    mLdcQueue.add(ldcData);
                }
                break;
            case STEREO_N3D:
                N3dData n3dData = new N3dData(data);
                synchronized (mN3dQueue) {
                    mN3dQueue.add(n3dData);
                }
                break;
            case STEREO_DEPTH_WRAPPER:
                DepthWrapper depthWrapper = new DepthWrapper(data);
                synchronized (mDepthWrapperQueue) {
                    mDepthWrapperQueue.add(depthWrapper);
                }
                break;
            default:
        }
        if (isQueueFull() && mXmpCompoundTask == null) {
            addXmpRequest();
            if (mAllCallbackReadyListener != null) {
                mAllCallbackReadyListener.onAllCallbackReady();
            }
        }
    }

    private boolean isQueueFull() {
        boolean isFull = true;
        isFull = isFull && !mCaptureTimeQueue.isEmpty();
        if (mCallbackString.contains(CLEAR_IMAGE)) {
            isFull = isFull && !mClearImageQueue.isEmpty();
        }
        if (mCallbackString.contains(BOKEH_IMAGE)) {
            isFull = isFull && !mOriginalJpegQueue.isEmpty();
        }
        if (mCallbackString.contains(MTK_BOKEH_DEPTH)) {
            isFull = isFull && !mDepthQueue.isEmpty();
        }
        if (mCallbackString.contains(MTK_BOKEH_META)) {
            isFull = isFull && !mMaskQueue.isEmpty();
        }
        if (mCallbackString.contains(MTK_DEBUG_BUFFER)) {
            isFull = isFull && !mN3dQueue.isEmpty();
        }
        if (mCallbackString.contains(LDC)) {
            isFull = isFull && !mLdcQueue.isEmpty();
        }
        if (mCallbackString.contains(MTK_DEPTH_WRAPPER)) {
            isFull = isFull && !mDepthWrapperQueue.isEmpty();
        }
        LogHelper.i(TAG, "isQueueFull :" + isFull);
        return isFull;

    }

    private void addXmpRequest() {
        mXmpCompoundTask = new XmpCompoundTask();
        mXmpCompoundTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    /**
     * the AsyncTask to handle all request to xmp compound files.
     */
    private class XmpCompoundTask extends AsyncTask<Void, Void, Void> {

        public XmpCompoundTask() {
        }

        @Override
        protected void onPreExecute() {
        }

        @Override
        protected Void doInBackground(Void... v) {
            LogHelper.d(TAG, "[XmpCompoundTask]doInBackground+");

            StereoCaptureInfo captureInfo = new StereoCaptureInfo();
            long captureTime = mCaptureTimeQueue.poll().getCaptureTime();
            configCaptureInfo(captureInfo, captureTime);
            byte [] xmpJpegData = mAccessor.writeStereoCaptureInfo(captureInfo);
            if (mCompoundJpegListener != null) {
                mCompoundJpegListener.onXmpCompoundJpeg(xmpJpegData, captureTime);
            }

            mXmpCompoundTask = null;
            LogHelper.d(TAG, "[XmpCompoundTask] doInBackground-");
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
        }
    }

    private void configCaptureInfo(StereoCaptureInfo captureInfo, long captureTime) {
        captureInfo.debugDir = SdofPhotoHelper.getFileName(captureTime);

        synchronized (mJpsQueue) {
            if (!mJpsQueue.isEmpty()) {
                captureInfo.jpsBuffer = mJpsQueue.poll().getJpsData();
            }
        }

        synchronized (mOriginalJpegQueue) {
            if (!mOriginalJpegQueue.isEmpty()) {
                captureInfo.jpgBuffer = mOriginalJpegQueue.poll().getOriginalJpegData();
            }
        }

        synchronized (mMaskQueue) {
            if (!mMaskQueue.isEmpty()) {
                captureInfo.configBuffer = mMaskQueue.poll().getMaskAndConfigData();
            }
        }

        synchronized (mClearImageQueue) {
            if (!mClearImageQueue.isEmpty()) {
                captureInfo.clearImage = mClearImageQueue.poll().getClearImage();
            }
        }

        synchronized (mDepthQueue) {
            if (!mDepthQueue.isEmpty()) {
                captureInfo.depthMap = mDepthQueue.poll().getDepthData();
            }
        }

        synchronized (mLdcQueue) {
            if (!mLdcQueue.isEmpty()) {
                captureInfo.ldc = mLdcQueue.poll().getLdcData();
            }
        }

        synchronized (mN3dQueue) {
            if (!mN3dQueue.isEmpty()) {
                captureInfo.debugBuffer = mN3dQueue.poll().getN3dData();
            }
        }

        synchronized (mDepthWrapperQueue) {
            if (!mDepthWrapperQueue.isEmpty()) {
                captureInfo.depthBuffer = mDepthWrapperQueue.poll().getDepthWrapperData();
            }
        }
    }

    private ISdofPhotoDeviceController.ExStereoDataCallback mExStereoDataCallback
            = new ISdofPhotoDeviceController.ExStereoDataCallback() {
        @Override
        public void onCaptureTime(long time) {
            addStereoDatas(STEREO_CAPTURE_TIME, null, time);
        }

        @Override
        public void onOriginalJpeg(byte[] jpeg) {
            addStereoDatas(STEREO_ORIGINAL_JPEG, jpeg, 0);
        }

        @Override
        public void onJpsCapture(byte[] jpsData) {
            addStereoDatas(STEREO_JPS, jpsData, 0);
        }

        @Override
        public void onMaskCapture(byte[] maskData) {
            addStereoDatas(STEREO_MASK, maskData, 0);
        }

        @Override
        public void onDepthMapCapture(byte[] depthMapData) {
            addStereoDatas(STEREO_DEPTH, depthMapData, 0);
        }

        @Override
        public void onClearImageCapture(byte[] clearImageData) {
            addStereoDatas(STEREO_CLEAR_IMAGE, clearImageData, 0);
        }

        @Override
        public void onLdcCapture(byte[] ldcData) {
            addStereoDatas(STEREO_LDC, ldcData, 0);
        }

        @Override
        public void onN3dCapture(byte[] n3dData) {
            addStereoDatas(STEREO_N3D, n3dData, 0);
        }

        @Override
        public void onDepthWrapperCapture(byte[] depthWrapper) {
            addStereoDatas(STEREO_DEPTH_WRAPPER, depthWrapper, 0);
        }

        @Override
        public void onClearQueue() {
            if (!mCaptureTimeQueue.isEmpty()) {
                mCaptureTimeQueue.clear();
            }
            if (!mJpsQueue.isEmpty()) {
                mJpsQueue.clear();
            }
            if (!mOriginalJpegQueue.isEmpty()) {
                mOriginalJpegQueue.clear();
            }
            if (!mClearImageQueue.isEmpty()) {
                mClearImageQueue.clear();
            }
            if (!mDepthQueue.isEmpty()) {
                mDepthQueue.clear();
            }
            if (!mN3dQueue.isEmpty()) {
                mN3dQueue.clear();
            }
            if (!mLdcQueue.isEmpty()) {
                mLdcQueue.clear();
            }
            if (!mDepthWrapperQueue.isEmpty()) {
                mDepthWrapperQueue.clear();
            }
        }
    };

    /**
     * The capture time class.
     */
    class CaptureTime {
        private long mCaptureTime;

        /**
         * Get capture time.
         *
         * @return the capture time.
         */
        public long getCaptureTime() {
            return mCaptureTime;
        }

        /**
         * Set capture time.
         *
         * @param captureTime capture time
         */
        public CaptureTime(long captureTime) {
            mCaptureTime = captureTime;
        }
    }

    /**
     * The Jps data class.
     */
    class JpsData {
        private byte[] mJpsData;

        /**
         * Get jps data.
         *
         * @return jps data
         */
        public byte[] getJpsData() {
            return mJpsData;
        }

        /**
         * Set jps data.
         *
         * @param jpsData jps data
         */
        public JpsData(byte[] jpsData) {
            mJpsData = jpsData;
        }
    }

    /**
     * The Original jpeg class.
     */
    class OriginalJpegData {
        private byte[] mOriginalJpegData;

        /**
         * Get Original data.
         *
         * @return Original data
         */
        public byte[] getOriginalJpegData() {
            return mOriginalJpegData;
        }

        /**
         * Set jpeg data.
         *
         * @param jpegData jpeg data
         */
        public OriginalJpegData(byte[] jpegData) {
            mOriginalJpegData = jpegData;
        }
    }

    /**
     * The mask class.
     */
    class MaskAndConfigData {
        private byte[] mMaskAndConfigData;

        /**
         * Get mask data.
         *
         * @return mask data
         */
        public byte[] getMaskAndConfigData() {
            return mMaskAndConfigData;
        }

        /**
         * Set mask data.
         *
         * @param maskAndConfigData mask data
         */
        public MaskAndConfigData(byte[] maskAndConfigData) {
            mMaskAndConfigData = maskAndConfigData;
        }
    }

    /**
     * The depth data class.
     */
    class DepthData {
        private byte[] mDepthData;

        /**
         * Get depth data.
         *
         * @return depth data
         */
        public byte[] getDepthData() {
            return mDepthData;
        }

        /**
         * Set depth data.
         *
         * @param depthData depth data
         */
        public DepthData(byte[] depthData) {
            mDepthData = depthData;
        }
    }

    /**
     * The clear image class.
     */
    class ClearImageData {
        private byte[] mClearImage;

        /**
         * Get clear image data.
         *
         * @return clear iamge data
         */
        public byte[] getClearImage() {
            return mClearImage;
        }

        /**
         * Set clear image data.
         *
         * @param clearImage the clear image data
         */
        public ClearImageData(byte[] clearImage) {
            mClearImage = clearImage;
        }
    }

    /**
     * The ldc class.
     */
    class LdcData {
        private byte[] mLdcData;

        /**
         * Get mask data.
         *
         * @return mask data
         */
        public byte[] getLdcData() {
            return mLdcData;
        }

        /**
         * Set jpeg data.
         *
         * @param ldcData jpeg data
         */
        public LdcData(byte[] ldcData) {
            mLdcData = ldcData;
        }
    }

    /**
     * The n3d class.
     */
    class N3dData {
        private byte[] mN3dData;

        /**
         * Get n3d data.
         *
         * @return n3d data
         */
        public byte[] getN3dData() {
            return mN3dData;
        }

        /**
         * Create n3d data.
         *
         * @param n3dData n3d data
         */
        public N3dData(byte[] n3dData) {
            mN3dData = n3dData;
        }
    }

    /**
     * The depth wrapper class.
     */
    class DepthWrapper {
        private byte[] mDepthWrapper;

        /**
         * Get depth wrapper data.
         *
         * @return the depth wrapper data
         */
        public byte[] getDepthWrapperData() {
            return mDepthWrapper;
        }

        /**
         * create depth wrapper.
         *
         * @param depthWrapper the depth wrapper data
         */
        public DepthWrapper(byte[] depthWrapper) {
            mDepthWrapper = depthWrapper;
        }
    }
}