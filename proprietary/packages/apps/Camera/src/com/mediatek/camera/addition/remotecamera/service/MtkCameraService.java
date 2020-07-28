package com.mediatek.camera.addition.remotecamera.service;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.DecimalFormat;

import com.android.camera.CameraActivity;
import com.android.camera.CameraHolder;
import com.android.camera.ComboPreferences;
import android.util.Log;

import com.mediatek.camera.platform.Parameters;

import android.app.Instrumentation;
import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.text.format.Time;
import android.view.KeyEvent;

/**
 * This service is used to communicate with other clients.
 * it can receive commands from client and send these commands to camera ap
 * it also can send preview/capture data to client
 */
public class MtkCameraService extends Service {

    private final static String                            TAG = "MtkCameraService";
    public static final String SERVICE_LAUNCH = "android.camera.service.launch";
    public final static int                                MSG_PARAMETERS_READY = 100;
    public final static int                                MSG_PREVIEW_FRAME_DATA = 101;
    public final static int                                MSG_CAPTURE_DATA = 102;
    public final static int                                MSG_SERVER_EXIT = 103;
    public final static int                                MSG_ORIENTATION_CHANGED = 104;
    private RemoteCallbackList<ICameraClientCallback>      mRemoteClientCallback;
    private Handler                                        mServiceHandler;
    private int                                            mPreviewWidth = 1920;
    private int                                            mPreviewHeight = 1080;
    private int                                            mPictureWidth = 0;
    private int                                            mPictureHeight = 0;
    private int                                            mImageFormat = ImageFormat.NV21;
    private int                                            mTargetWidth = 240;
    private int                                            mTargetHeight = 240;
    private int                                            mOrientation = 0;
    private long                                           mRefreshInterval = 250;
    private long                                           mLastRefreshTime = 0;
    private int[]                                          mDataRGB8888;
    // capture
    private boolean                                        mIsDuringCapture = false;
    private long                                           mLastFrameTimeMs = 0;
    private long                                           mCurrentTimeMs = 0;
    private int                                            mFrameRate = 30;
    private boolean                                        mIsClientRequestExit = false;
    private boolean                                        mReleasCamera = false;
    // feature
    private static final String KEY_SUPPORTED_FEATURE = "supported-features=";
    private static final String                            CAPTURE = "Capture";
    private static final String                            CONTINUOUS_SHOT = "Continuous Shot";
    private String                                         mSupportedFeatures;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "onCreate()");
        mRemoteClientCallback = new RemoteCallbackList<ICameraClientCallback>();
        HandlerThread ht = new HandlerThread("camera service handler thread");
        ht.start();
        mServiceHandler = new ServiceHandler(ht.getLooper());
        mSupportedFeatures = KEY_SUPPORTED_FEATURE;
        // has capture feature
        mSupportedFeatures += (CAPTURE + ",");
    }

    @Override
    public void onStart(Intent intent, int startId) {
        Log.i(TAG, "startId :" + startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.i(TAG, "intent:" + intent.getAction());
        return new MtkCameraServiceImpl();
    }

    private void sendKeyCode(final int keyCode) {
        new Thread() {
            @Override
            public void run() {
                try {
                    Instrumentation instrumentation = new Instrumentation();
                    instrumentation.sendKeyDownUpSync(keyCode);
                } catch (Exception e) {

                }
            }
        }.start();
    }

    // Encode YUV to jpeg, and crop it
    private ByteArrayOutputStream cropFromYuvData(byte[] data) {
        Rect rect = new Rect(0, 0, mPreviewWidth, mPreviewHeight);
        YuvImage yuvImg = new YuvImage(data, mImageFormat, mPreviewWidth, mPreviewHeight, null);
        ByteArrayOutputStream outputstream = new ByteArrayOutputStream();
        yuvImg.compressToJpeg(rect, 100, outputstream);
        return outputstream;
    }

    // decode jpeg to bitmap
    private Bitmap decodeJpegToBitmap(ByteArrayOutputStream outputstream) {
        Bitmap bitmap = null;
        try {
            bitmap = BitmapFactory.decodeByteArray(outputstream.toByteArray(),
                    0, outputstream.size());
            outputstream.close();
            outputstream = null;
        } catch (Exception e) {
        }
        return bitmap;
    }

    // scale and rotate the cropped bitmap
    private byte[] cropScaleRotateJpegData(Bitmap bitmap, int rotation) {
        Matrix rotateMatrix = new Matrix();
        rotateMatrix.postRotate(rotation);
        double scaleRation;
        byte[] scaleData = null;
        int previewWidth = bitmap.getWidth();
        int previeHeight = bitmap.getHeight();
        int maxCropEdge = Math.min(previewWidth, previeHeight);
        boolean isLandScape = previewWidth > previeHeight;
        int cropX = isLandScape ? (previewWidth - previeHeight) / 2 : 0;
        int cropY = isLandScape ? 0 : (previeHeight - previewWidth) / 2;
        try {
            // crop and rotate
            bitmap = Bitmap.createBitmap(bitmap, cropX, cropY, maxCropEdge,
                    maxCropEdge, rotateMatrix, true);
            // scale
            int targetWidth = mTargetWidth;
            int targetHeight = mTargetHeight;
            if (mOrientation == 90 || mOrientation == 270) {
                targetHeight = targetWidth * (Math.min(mPreviewWidth, mPreviewHeight))
                        / (Math.max(mPreviewWidth, mPreviewHeight));
            } else {
                targetWidth = targetHeight * (Math.min(mPreviewWidth, mPreviewHeight))
                        / (Math.max(mPreviewWidth, mPreviewHeight));
            }
            bitmap = Bitmap.createScaledBitmap(bitmap, targetWidth, targetHeight, false);
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            bitmap.compress(Bitmap.CompressFormat.JPEG, 30, outputStream);
            //dumpBitmap(bitmap);
            bitmap.recycle();
            bitmap = null;
            scaleData = outputStream.toByteArray();
            outputStream.close();
            outputStream = null;
        } catch (Exception e) {

        }
        return scaleData;
    }

    private byte[] scaleCropJpegData(byte[] data) {
        Log.d(TAG, "scaleJpegData()");
        Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
        int pictureWidth = bitmap.getWidth();
        int pictureHeight = bitmap.getHeight();
        int maxCropEdge = Math.min(pictureWidth, pictureHeight);
        boolean isLandScape = pictureWidth > pictureHeight;
        int cropX = isLandScape ? (pictureWidth - pictureHeight) / 2 : 0;
        int cropY = isLandScape ? 0 : (pictureHeight - pictureWidth) / 2;
        bitmap = Bitmap.createBitmap(bitmap, cropX, cropY, maxCropEdge, maxCropEdge);
        double scaleRation;
        Matrix matrix = new Matrix();
        bitmap = Bitmap.createScaledBitmap(bitmap, mTargetWidth, mTargetHeight, false);
        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.JPEG, 30, outputStream);
        byte[] scaleData = outputStream.toByteArray();
        bitmap.recycle();
        return scaleData;
    }

    private void dumpBitmap(Bitmap bitmap) {
        Log.d(TAG, "dump()");
        Time t = new Time();
        t.setToNow();
        DecimalFormat df = new DecimalFormat("00");
        String date = String.valueOf(t.year) + df.format(t.month) + df.format(t.monthDay);
        String time =  String.valueOf(t.hour) + df.format(t.minute) + df.format(t.second);
        String pictureName = "IMG_" + date + "_" + time +  ".jpg";
        Log.i(TAG, "pictureName:" + pictureName);
        File sdCard = Environment.getExternalStorageDirectory();
        File directory = new File(sdCard.getAbsolutePath()
               + "/Photo");
        directory.mkdirs();

        File file = new File(directory, pictureName);
        try {
            FileOutputStream outputStream = new FileOutputStream(file);
            bitmap.compress(Bitmap.CompressFormat.JPEG, 100, outputStream);
            outputStream.flush();
            outputStream.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(Message msg) {
            long now = System.currentTimeMillis();
            byte[] data = null;
            byte[] dstData = null;
            int n = 0;
            Log.d(TAG, "Message.what:" + msg.what);
            switch(msg.what) {
            case MSG_PARAMETERS_READY:
                Parameters parameters = (Parameters) msg.obj;
                mPreviewWidth = parameters.getPreviewSize().width;
                mPreviewHeight = parameters.getPreviewSize().height;
                mPictureWidth = parameters.getPictureSize().width;
                mPictureHeight = parameters.getPictureSize().height;
                mImageFormat = parameters.getPreviewFormat();
                Log.d(TAG, "mPreviewWidth:" + mPreviewWidth + ", mPreviewHeight:" +
                        "" + mPreviewHeight + ", mPictureWidth:" + mPictureWidth + ", " +
                        "" + "mPictureHeight:" + mPictureHeight + ", " + "mImageFormat:" +
                        "" + mImageFormat + ", mReleasCamera:" + mReleasCamera);
                // some times mtk camera haven't opened, the watch have request release camera,
                // in this case, the mtk camera can not exit, so this force to exit camera.
                if (mReleasCamera) {
                    sendKeyCode(KeyEvent.KEYCODE_BACK);
                    mReleasCamera = false;
                }
                break;

            case MSG_PREVIEW_FRAME_DATA:
                Log.d(TAG, "preview frame comes~ orientation = " + msg.arg1);
                n = mRemoteClientCallback.beginBroadcast();
                data = (byte[]) msg.obj;
                if (data != null) {
                    // crop -> Bitmap -> scale
                    dstData = cropScaleRotateJpegData(decodeJpegToBitmap(cropFromYuvData(data)),
                            msg.arg1);
                    for (int i = 0; i < n; i++) {
                        try {
                            mRemoteClientCallback.getBroadcastItem(i).onPreviewFrame(dstData);
                            Log.d(TAG, "onPreviewFrame dstData = " + dstData);
                        } catch (RemoteException e) {
                        }
                    }
                }
                mRemoteClientCallback.finishBroadcast();
                break;
            case MSG_CAPTURE_DATA:
                n = mRemoteClientCallback.beginBroadcast();
                data = (byte[]) msg.obj;
                if (data != null) {
                    dstData = scaleCropJpegData(data);
                    for (int i = 0; i < n; i++) {
                        try {
                            mRemoteClientCallback.getBroadcastItem(i).onPictureTaken(dstData);
                        } catch (RemoteException e) {
                        }
                    }
                }
                mRemoteClientCallback.finishBroadcast();
                Log.d(TAG, "process capture frame consume time =" +
                        "" + (System.currentTimeMillis() - now));
                break;
            case MSG_SERVER_EXIT:
                n = mRemoteClientCallback.beginBroadcast();
                for (int i = 0; i < n; i++) {
                    try {
                        mRemoteClientCallback.getBroadcastItem(i).cameraServerApExit();
                    } catch (RemoteException e) {
                        Log.e(TAG, "cameraServerExit exception = " + e);
                    }
                }
                mRemoteClientCallback.finishBroadcast();
                break;
            case MSG_ORIENTATION_CHANGED:
                mOrientation = msg.arg1;
                int targetWidth = mTargetWidth;
                int targetHeight = mTargetHeight;
                if (mOrientation == 90 || mOrientation == 270) {
                    targetHeight = targetWidth * (Math.min(mPreviewWidth, mPreviewHeight))
                            / (Math.max(mPreviewWidth, mPreviewHeight));
                } else {
                    targetWidth = targetHeight * (Math.min(mPreviewWidth, mPreviewHeight))
                            / (Math.max(mPreviewWidth, mPreviewHeight));
                }
                Log.d(TAG, "onOrientationChanged, mOrientation:" + mOrientation + "," +
                        "targetHeight:" + targetHeight + ", targetWidth:" + targetWidth);
                break;
            default:
                break;
            }
            data = null;
            dstData = null;
        }
    }

    private Bitmap getBitmapFromNV21(byte[] data, int width, int height, int rotation) {
        Matrix rotateMatrix = new Matrix();
        rotateMatrix.postRotate(rotation);
        decodeYUV420SP(mDataRGB8888, data, width, height);
        Bitmap bitmap = Bitmap.createBitmap(mDataRGB8888, width, height, Bitmap.Config.ARGB_8888);
        Bitmap rotateBitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(),
                bitmap.getHeight(), rotateMatrix, true);
        bitmap.recycle();
        bitmap = null;
        return rotateBitmap;
    }

    private void decodeYUV420SP(int[] rgb, byte[] yuv420sp, int width, int height) {
        final int frameSize = width * height;
        for (int j = 0, yp = 0; j < height; j++) {
            int uvp = frameSize + (j >> 1) * width, u = 0, v = 0;
            for (int i = 0; i < width; i++, yp++) {
                int y = (0xff & ((int) yuv420sp[yp])) - 16;
                if (y < 0) y = 0;
                if ((i & 1) == 0) {
                    v = (0xff & yuv420sp[uvp++]) - 128;
                    u = (0xff & yuv420sp[uvp++]) - 128;
                }
                int y1192 = 1192 * y;
                int r = (y1192 + 1634 * v);
                int g = (y1192 - 833 * v - 400 * u);
                int b = (y1192 + 2066 * u);
                if (r < 0) {
                    r = 0;
                } else if (r > 262143) {
                    r = 262143;
                }
                if (g < 0) {
                    g = 0;
                } else if (g > 262143) {
                    g = 262143;
                }
                if (b < 0) {
                    b = 0;
                } else if (b > 262143) {
                    b = 262143;
                }
                rgb[yp] = 0xff000000 | ((r << 6) & 0xff0000)
                        | ((g >> 2) & 0xff00) | ((b >> 10) & 0xff);
            }
       }
    }

    public class MtkCameraServiceImpl extends IMtkCameraService.Stub {
        @Override
        public void openCamera() throws RemoteException {
            Log.d(TAG, "openCamera");
            // Try to get the camera hardware
            CameraHolder holder = CameraHolder.instance();
            ComboPreferences comboPref = new ComboPreferences(getApplicationContext());
            SharedPreferences globalPref = comboPref.getGlobal();

            int cameraId = Integer.parseInt(globalPref.getString("pref_camera_id_key", "0"));
            if (holder.tryOpen(cameraId) == null) {
                return;
            }
            // We are going to launch the camera, so hold the camera for later use
            holder.keep(3000, cameraId);
            holder.release(false);
            Intent i = new Intent(Intent.ACTION_MAIN);
            i.setClass(getApplicationContext(), CameraActivity.class);
            i.addCategory(Intent.CATEGORY_LAUNCHER);
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            i.putExtra(SERVICE_LAUNCH, false);
            getApplicationContext().startActivity(i);
            mLastFrameTimeMs = 0;
            mCurrentTimeMs = 0;
            mIsClientRequestExit = false;
        }

        public void setFrameRate(int frameRate) throws RemoteException {
            Log.d(TAG, "setFrameRate frameRate = " + frameRate);
            mFrameRate = frameRate;
        }

        @Override
        public void releaseCamera() throws RemoteException {
            Log.d(TAG, "releaseCamera mIsClientRequestExit = " + mIsClientRequestExit);
            if (!mIsClientRequestExit) {
                mReleasCamera = true;
                sendKeyCode(KeyEvent.KEYCODE_BACK);
                mIsClientRequestExit = true;
                if (mServiceHandler != null) {
                    mServiceHandler.removeCallbacksAndMessages(null);
                }
            }
        }

        @Override
        public String getSupportedFeatureList() throws RemoteException {
            Log.d(TAG, "getSupportedFeatureList = " + mSupportedFeatures);
            return mSupportedFeatures;
        }

        @Override
        public void capture() throws RemoteException {
            Log.d(TAG, "capture");
            mIsDuringCapture = true;
            if (mServiceHandler != null) {
                mServiceHandler.removeMessages(MSG_PREVIEW_FRAME_DATA);
            }
            sendKeyCode(KeyEvent.KEYCODE_CAMERA);
        }

        @Override
        public void sendMessage(Message msg) {
            if (msg.what == MSG_PARAMETERS_READY || msg.what == MSG_ORIENTATION_CHANGED) {
                if (mServiceHandler != null) {
                    mServiceHandler.sendMessage(msg);
                }
                return;
            }

            // when capturing is from remote camera, abandon.
            if (!mIsDuringCapture && msg.what == MSG_CAPTURE_DATA) {
                return;
            }

            // when take picture, wait for capture result comes
            if (mIsDuringCapture && (msg.what != MSG_CAPTURE_DATA)) {
                mCurrentTimeMs = System.currentTimeMillis();
                return;
            }
            mCurrentTimeMs = System.currentTimeMillis();
            long deltaTime = mCurrentTimeMs - mLastFrameTimeMs;
            if (mIsDuringCapture || (mLastFrameTimeMs == 0) || deltaTime > (1000 / mFrameRate)) {
                mLastFrameTimeMs = mCurrentTimeMs;
                if (mServiceHandler != null) {
                    mServiceHandler.sendMessage(msg);
                }
                mIsDuringCapture = false;
            }
        }

        @Override
        public void registerCallback(ICameraClientCallback cb) throws RemoteException {
            Log.d(TAG, "registerCallback");
            if (cb != null) {
                mRemoteClientCallback.register(cb);
            }
        }

        @Override
        public void unregisterCallback(ICameraClientCallback cb) throws RemoteException {
            Log.d(TAG, "unregisterCallback");
            if (cb != null) {
                mRemoteClientCallback.unregister(cb);
            }
        }

        @Override
        public void cameraServerExit() {
            Log.d(TAG, "cameraServerExit mIsClientRequestExit = " + mIsClientRequestExit);
            if (!mIsClientRequestExit) {
                mIsClientRequestExit = true;
                if (mServiceHandler != null) {
                    mServiceHandler.removeCallbacksAndMessages(null);
                    mServiceHandler.sendEmptyMessage(MSG_SERVER_EXIT);
                }
            }
        }
    }
}
