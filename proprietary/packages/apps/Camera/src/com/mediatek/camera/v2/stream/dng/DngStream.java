package com.mediatek.camera.v2.stream.dng;

import android.graphics.ImageFormat;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.Surface;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.stream.ICaptureStream;
import com.mediatek.camera.v2.stream.ImageInfo;
import com.mediatek.camera.v2.util.Utils;

import java.util.Map;

public class DngStream implements IDngStream {
    private static final Tag TAG = new Tag(DngStream.class.getSimpleName());
    private static int                            MAX_RAW_CAPTURE_IMAGES = 1;
    private ImageReader                           mRawImageReader;
    private CaptureStreamCallback                 mCallback;
    private CameraCharacteristics                 mCharacteristics;
    private HandlerThread                         mRawCaptureHandlerThread;
    private Handler                               mRawCaptureHandler;
    private Surface                               mRawCaptureSurface;
    private ICaptureStream                        mCaptureStream;
    private Size                                  mRawSize;
    private int                                   mRawCaptureWidth;
    private int                                   mRawCaptureHeight;


    public DngStream(ICaptureStream captureStreamController) {
        mCaptureStream = captureStreamController;
    }

    @Override
    public void setCaptureStreamCallback(CaptureStreamCallback callback) {
        mCallback = callback;
        if (mCaptureStream != null) {
            mCaptureStream.setCaptureStreamCallback(callback);
        }
    }

    private ImageReader.OnImageAvailableListener  mRawCaptureImageListener =
        new ImageReader.OnImageAvailableListener() {
            @Override
            public void onImageAvailable(ImageReader reader) {
                LogHelper.d(TAG, "mRawCaptureImageListener mCallback = " + mCallback);
                if (mCallback == null) {
                    return;
                }
                Image image = reader.acquireLatestImage();
                int width = image.getWidth();
                int height = image.getHeight();
                int imageFormat = image.getFormat();
                byte[] data = Utils.acquireRawBytesAndClose(image);
                ImageInfo info = new ImageInfo(data, width, height, imageFormat);
                synchronized (mCallback) {
                    mCallback.onCaptureCompleted(info);
                }
            }
        };

    @Override
    public boolean updateCaptureSize(Size pictureSize, int pictureFormat) {
        boolean capture_updated = mCaptureStream.updateCaptureSize(pictureSize, pictureFormat);

        if (mRawCaptureHandler == null) {
            mRawCaptureHandlerThread = new HandlerThread("ImageReaderStream.RawCaptureThread");
            mRawCaptureHandlerThread.start();
            mRawCaptureHandler = new Handler(mRawCaptureHandlerThread.getLooper());
        }

        if (mRawImageReader != null && mRawCaptureWidth == mRawSize.getWidth()
                && mRawCaptureHeight == mRawSize.getHeight()) {
            LogHelper.d(TAG, "[updateCaptureSize]- configure the same size, skip : " + "" +
                    " width  = " + mRawCaptureWidth +
                    " height = " + mRawCaptureHeight);
            return capture_updated;
        }

        mRawCaptureWidth = mRawSize.getWidth();
        mRawCaptureHeight = mRawSize.getHeight();

        if (mRawImageReader != null) {
            mRawImageReader.close();
            mRawImageReader = null;
        }

        //TODO: the raw size is queried form setting?
        LogHelper.d(TAG, "[updateCaptureSize]-raw size:" +
                mRawSize.getWidth() + "x" + mRawSize.getHeight());
        mRawImageReader = mRawImageReader.newInstance(mRawSize.getWidth(),
                mRawSize.getHeight(), ImageFormat.RAW_SENSOR, MAX_RAW_CAPTURE_IMAGES);
        mRawImageReader.setOnImageAvailableListener(mRawCaptureImageListener, mRawCaptureHandler);
        mRawCaptureSurface = mRawImageReader.getSurface();
        LogHelper.d(TAG, "[updateCaptureSize]-Raw reader:" + mRawImageReader);

        return true;
    }


    @Override
    public Map<String, Surface> getCaptureInputSurface() {
        Map<String, Surface> surfaceMap = mCaptureStream.getCaptureInputSurface();
        if (mRawCaptureSurface == null) {
            throw new IllegalStateException("You should call" +
                    " CaptureStream.updateCaptureSize firstly, " +
                    "when get input capture surface");
        }

        LogHelper.d(TAG, "getCaptureInputSurface:" + mRawCaptureSurface);
        surfaceMap.put(CAPUTRE_RAW_SURFACE_KEY, mRawCaptureSurface);
        return surfaceMap;
    }


    @Override
    public void releaseCaptureStream() {

        if (mRawImageReader != null) {
            mRawImageReader.close();
            mRawImageReader = null;
            mRawCaptureSurface = null;
        }

        if (mRawCaptureHandlerThread != null) {
            mRawCaptureHandlerThread.quitSafely();
            mRawCaptureHandler = null;
        }
    }

    public void updateCameraCharacteristics(CameraCharacteristics csdata) {
        mCharacteristics = csdata;
        StreamConfigurationMap config = mCharacteristics.get(
                CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        Size[] rawSizes = config.getOutputSizes(ImageFormat.RAW_SENSOR);
        for (int i = 0; i < rawSizes.length; i++) {
            LogHelper.d(TAG, "raw supported size:" + rawSizes[i]);
        }
        mRawSize = rawSizes[0];
    }
}
