package com.mediatek.campostalgo.functional;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.campostalgo.FeatureConfig;
import com.mediatek.campostalgo.FeatureParam;
import com.mediatek.campostalgo.FeatureResult;
import com.mediatek.campostalgo.FeaturePipeConfig;
import com.mediatek.campostalgo.StreamInfo;
import com.mediatek.campostalgo.ICamPostAlgoInterface;
import com.mediatek.campostalgo.ICamPostAlgoService;
import com.mediatek.campostalgo.ICamPostAlgoCallback;
import com.mediatek.campostalgo.Stream;


import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Paint;
import android.graphics.Rect;
import android.media.Image;
import android.media.ImageReader;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.test.InstrumentationTestCase;
import android.util.Log;
import android.view.Surface;
import android.support.test.InstrumentationRegistry;
import android.os.Handler;
import android.os.HandlerThread;
import android.graphics.PixelFormat;

import java.nio.ByteBuffer;
import java.util.Arrays;

import static org.mockito.Mockito.*;
//import static org.mockito.Mockito.verify;
//import static org.mockito.Mockito.mock;


public class CamPostAlgoApiTests extends InstrumentationTestCase {
    protected final static String TAG = CamPostAlgoApiTests.class.getSimpleName();
    private Context mContext;
    private HandlerThread mHandleThread;
    private ICamPostAlgoInterface mPostAlgoInterface;
    private ICamPostAlgoService mPostAlgoService;

    private static final String POSTALGO_SERVICE_BINDER_NAME = "mediatek.campostalgo";

    private static final int WIDTH = 640;
    private static final int HEIGHT = 480;

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        if (mHandleThread == null) {
            mHandleThread = new HandlerThread(TAG);
            mHandleThread.start();
        }

        mContext = InstrumentationRegistry.getTargetContext();
        getService();
        getPostAlgoInterface();

    }

    private class PostAlgoDeathRecipient implements IBinder.DeathRecipient {

        @Override
        public void binderDied() {
            Log.e(TAG, "Postalgo Service died!");
        }
    }

    private void getService() {
        if (mPostAlgoService == null) {
            Log.d(TAG, "Connect to postalgo service");
            Method method = null;
            try {
                method = Class.forName("android.os.ServiceManager").getMethod(
                        "getService", String.class);
                IBinder postAlgoBinder = (IBinder) method.invoke(null,
                        POSTALGO_SERVICE_BINDER_NAME);
                if (postAlgoBinder != null) {
                    try {
                        postAlgoBinder.linkToDeath(
                                new PostAlgoDeathRecipient(), 0);
                    } catch (RemoteException ex) {
                        ex.printStackTrace();
                        Log.e(TAG, "PostAlgo linkToDeath error");
                        return;
                    }
                    mPostAlgoService = ICamPostAlgoService.Stub
                            .asInterface(postAlgoBinder);
                } else {
                    Log.e(TAG, "Get postalgo service error binder is null");
                }

                // if(mPostAlgoService != null)
                // mIsAcquired = true;

            } catch (NoSuchMethodException e) {
                e.printStackTrace();
            } catch (ClassNotFoundException e) {
                e.printStackTrace();
            } catch (IllegalAccessException e) {
                e.printStackTrace();
            } catch (InvocationTargetException e) {
                e.printStackTrace();
            }

        } else {
            Log.d(TAG, "Service is already acquired");
        }
    }

    private ICamPostAlgoInterface getPostAlgoInterface() {
        ICamPostAlgoInterface postAlgoInterface = null;

        if (mPostAlgoService == null) {
            getService();
        }
        try {
            if (mPostAlgoInterface == null) {
                mPostAlgoInterface = mPostAlgoService.connect(
                        mContext.getPackageName(), -1);
                if (postAlgoInterface == null) {
                    Log.e(TAG, "Cannot get postAlgoInterface is null!");
                }
            }
        } catch (RemoteException ex) {
            ex.printStackTrace();
            Log.e(TAG, "connect algoservice error " + ex);
        }
        return mPostAlgoInterface;
    }

    public class PostaLgoImageAvaiableListener implements ImageReader.OnImageAvailableListener {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Log.d(TAG, "onImageAvailable!");
            Image i = null;
            try {
                i = reader.acquireNextImage();
                ByteBuffer buffer = i.getPlanes()[0].getBuffer();
                byte[] data = new byte[buffer.remaining()];
                buffer.get(data);
                buffer.rewind();
                Log.d(TAG, "get Image Bytes len: " + data.length);
            } finally {
                if (i != null) {
                    i.close();
                }
            }
            Log.d(TAG, "onImageAvailable finished!");
        }
    };

    private  PostaLgoImageAvaiableListener mReaderListener;
    private ImageReader getImageSurface(android.util.Size size, int format,
            int count) {
        ImageReader capture = ImageReader.newInstance(size.getWidth(),
                size.getHeight(), format, count);
        mReaderListener = spy(PostaLgoImageAvaiableListener.class);
        //mReaderListener = new PostaLgoImageAvaiableListener();
        capture.setOnImageAvailableListener(mReaderListener,
                new Handler(mHandleThread.getLooper()));
        return capture;
    }

    private class PostAlgoCallback extends ICamPostAlgoCallback.Stub {
        @Override
        public void processResult(FeatureParam params) {
            Log.d(TAG, "ICamPostAlgoCallback  processResult");
        }

        @Override
        public IBinder asBinder() {
            return this;
        }
    }

    public void testConfigPostAlgoFeature() {

        ImageReader input = getImageSurface(new android.util.Size(WIDTH, HEIGHT),
                PixelFormat.RGBA_8888, 1);
        FeatureConfig config = new FeatureConfig();
        List<Surface> surfaces = new ArrayList<Surface>();
        surfaces.add(input.getSurface());
        //config.addSurface(Array.asList(new Surface[]{input.getSurface()});
        config.addSurface(surfaces);
        StreamInfo si = new StreamInfo();
        si.addInfo(WIDTH, HEIGHT, ImageFormat.YUV_420_888, 0);
        config.addStreamInfo(Arrays.asList(si));
        FeaturePipeConfig pc = new FeaturePipeConfig();
        pc.addFeaturePipeConfig(FeaturePipeConfig.INDEX_CAPTURE
            , new int[]{FeaturePipeConfig.INDEX_NONE});
        config.addFeaturePipeConfig(pc);

        ICamPostAlgoInterface postAlgo = getPostAlgoInterface();
        // check connect
        assertNotNull(postAlgo);
        List<FeatureConfig> configs = new ArrayList<FeatureConfig>();
        configs.add(config);
        FeatureResult result = null;
        try {
            result = postAlgo.config(
                    configs.toArray(new FeatureConfig[configs.size()]),
                    new PostAlgoCallback());
        } catch (RemoteException ex) {
            ex.printStackTrace();
            Log.e(TAG, "Postalgo config feature failed.");
        }
        // check config
        assertNotNull(result);
        assertTrue(result.getStreams().size() > 0);
        Surface sf = result.getStreams().elementAt(0).getmSurface();

        Canvas canvas = sf.lockCanvas(null);
        assertNotNull(canvas);
        Log.d(TAG, "Paint Blue!");
        canvas.drawColor(Color.BLUE);
        sf.unlockCanvasAndPost(canvas);
        Log.d(TAG, "draw done!");
        try {
            Thread.sleep(2000);
        }catch(InterruptedException e ) {
            Log.d(TAG, "error sleep!");
        }
        if (postAlgo != null) {
            try {
                 postAlgo.disconnect();
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }
        }
        verify(mReaderListener).onImageAvailable(any(ImageReader.class));
    }

public void testFinalizeOutConfig() {

        ImageReader input = getImageSurface(new android.util.Size(WIDTH, HEIGHT),
                PixelFormat.RGBA_8888, 1);
        FeatureConfig config = new FeatureConfig();
        StreamInfo si = new StreamInfo();
        si.addInfo(WIDTH, HEIGHT, ImageFormat.YUV_420_888, 0);
        config.addStreamInfo(Arrays.asList(si));
        FeaturePipeConfig pc = new FeaturePipeConfig();
        pc.addFeaturePipeConfig(FeaturePipeConfig.INDEX_CAPTURE
            , new int[]{FeaturePipeConfig.INDEX_NONE});
        config.addFeaturePipeConfig(pc);

        ICamPostAlgoInterface postAlgo = getPostAlgoInterface();
        // check connect
        assertNotNull(postAlgo);
        List<FeatureConfig> configs = new ArrayList<FeatureConfig>();
        configs.add(config);
        FeatureResult result = null;
        try {
            result = postAlgo.config(
                    configs.toArray(new FeatureConfig[configs.size()]),
                    new PostAlgoCallback());
        } catch (RemoteException ex) {
            ex.printStackTrace();
            Log.e(TAG, "Postalgo config feature failed.");
        }
        // check config
        assertNotNull(result);
        assertTrue(result.getStreams().size() > 0);
        //check finalizeOutputConfig
        List<Stream> streames = new ArrayList<Stream>();
        streames.add(new Stream(FeaturePipeConfig.INDEX_CAPTURE, input.getSurface()));
        //config.addSurface(Array.asList(new Surface[]{input.getSurface()});
        //config.addSurface(surfaces);
        try {
            postAlgo.finalizeOutputConfig(
                    streames.toArray(new Stream[0]));
        } catch (RemoteException ex) {
            ex.printStackTrace();
            Log.e(TAG, "Postalgo finalizeOutputConfig failed.");
        }

        Surface sf = result.getStreams().elementAt(0).getmSurface();
        Canvas canvas = sf.lockCanvas(null);
        assertNotNull(canvas);
        Log.d(TAG, "Paint Blue!");
        canvas.drawColor(Color.BLUE);
        sf.unlockCanvasAndPost(canvas);
        Log.d(TAG, "draw done!");
        try {
            Thread.sleep(2000);
        }catch(InterruptedException e ) {
            Log.d(TAG, "error sleep!");
        }
        if (postAlgo != null) {
            try {
                 postAlgo.disconnect();
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }
        }
        verify(mReaderListener).onImageAvailable(any(ImageReader.class));
    }


public void testFilterFeature() {

        ImageReader input = getImageSurface(new android.util.Size(WIDTH, HEIGHT),
                PixelFormat.RGBA_8888, 1);
        FeatureConfig config = new FeatureConfig();
        List<Surface> surfaces = new ArrayList<Surface>();
        surfaces.add(input.getSurface());
        //config.addSurface(Array.asList(new Surface[]{input.getSurface()});
        config.addSurface(surfaces);
        StreamInfo si = new StreamInfo();
        si.addInfo(WIDTH, HEIGHT, ImageFormat.YUV_420_888, 0);
        config.addStreamInfo(Arrays.asList(si));
        FeaturePipeConfig pc = new FeaturePipeConfig();
        pc.addFeaturePipeConfig(FeaturePipeConfig.INDEX_CAPTURE
            , new int[]{FeaturePipeConfig.INDEX_FILTER_CAPTURE});
        config.addFeaturePipeConfig(pc);

        ICamPostAlgoInterface postAlgo = getPostAlgoInterface();
        // check connect
        assertNotNull(postAlgo);
        List<FeatureConfig> configs = new ArrayList<FeatureConfig>();
        configs.add(config);
        FeatureResult result = null;
        try {
            result = postAlgo.config(
                    configs.toArray(new FeatureConfig[configs.size()]),
                    new PostAlgoCallback());
        } catch (RemoteException ex) {
            ex.printStackTrace();
            Log.e(TAG, "Postalgo config feature failed.");
        }
        // check config
        assertNotNull(result);
        assertTrue(result.getStreams().size() > 0);
        Surface sf = result.getStreams().elementAt(0).getmSurface();

        Canvas canvas = sf.lockCanvas(null);
        assertNotNull(canvas);
        Log.d(TAG, "Paint Blue!");
        canvas.drawColor(Color.BLUE);
        sf.unlockCanvasAndPost(canvas);
        Log.d(TAG, "draw done!");
        try {
            Thread.sleep(2000);
        }catch(InterruptedException e ) {
            Log.d(TAG, "error sleep!");
        }
        if (postAlgo != null) {
            try {
                 postAlgo.disconnect();
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }
        }
        verify(mReaderListener).onImageAvailable(any(ImageReader.class));
    }

    public void testFBFeature() {

        ImageReader input = getImageSurface(new android.util.Size(WIDTH, HEIGHT),
                PixelFormat.RGBA_8888, 1);
        FeatureConfig config = new FeatureConfig();
        List<Surface> surfaces = new ArrayList<Surface>();
        surfaces.add(input.getSurface());
        //config.addSurface(Array.asList(new Surface[]{input.getSurface()});
        config.addSurface(surfaces);
        StreamInfo si = new StreamInfo();
        si.addInfo(WIDTH, HEIGHT, ImageFormat.YUV_420_888, 0);
        config.addStreamInfo(Arrays.asList(si));
        FeaturePipeConfig pc = new FeaturePipeConfig();
        pc.addFeaturePipeConfig(FeaturePipeConfig.INDEX_CAPTURE
            , new int[]{FeaturePipeConfig.INDEX_FB});
        config.addFeaturePipeConfig(pc);

        ICamPostAlgoInterface postAlgo = getPostAlgoInterface();
        // check connect
        assertNotNull(postAlgo);
        List<FeatureConfig> configs = new ArrayList<FeatureConfig>();
        configs.add(config);
        FeatureResult result = null;
        try {
            result = postAlgo.config(
                    configs.toArray(new FeatureConfig[configs.size()]),
                    new PostAlgoCallback());
        } catch (RemoteException ex) {
            ex.printStackTrace();
            Log.e(TAG, "Postalgo config feature failed.");
        }
        // check config
        assertNotNull(result);
        assertTrue(result.getStreams().size() > 0);
        Surface sf = result.getStreams().elementAt(0).getmSurface();

        Canvas canvas = sf.lockCanvas(null);
        assertNotNull(canvas);
        Log.d(TAG, "Paint Blue!");
        canvas.drawColor(Color.BLUE);
        sf.unlockCanvasAndPost(canvas);
        Log.d(TAG, "draw done!");
        try {
            Thread.sleep(2000);
        }catch(InterruptedException e ) {
            Log.d(TAG, "error sleep!");
        }
        if (postAlgo != null) {
            try {
                 postAlgo.disconnect();
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }
        }
        verify(mReaderListener).onImageAvailable(any(ImageReader.class));
    }

public void testAutoramaFeature() {
        ImageReader input = getImageSurface(new android.util.Size(1280, 720),
                PixelFormat.RGBA_8888, 1);
        FeatureConfig config = new FeatureConfig();
        List<Surface> surfaces = new ArrayList<Surface>();
        surfaces.add(input.getSurface());
        //config.addSurface(Array.asList(new Surface[]{input.getSurface()});
        config.addSurface(surfaces);
        StreamInfo si = new StreamInfo();
        si.addInfo(1280, 720, ImageFormat.NV21, 0);
        config.addStreamInfo(Arrays.asList(si));
        FeaturePipeConfig pc = new FeaturePipeConfig();
        pc.addFeaturePipeConfig(FeaturePipeConfig.INDEX_PREVIEW
            , new int[]{FeaturePipeConfig.INDEX_AUTOPANORAMA});
        config.addFeaturePipeConfig(pc);

        ICamPostAlgoInterface postAlgo = getPostAlgoInterface();
        // check connect
        assertNotNull(postAlgo);
        List<FeatureConfig> configs = new ArrayList<FeatureConfig>();
        configs.add(config);
        FeatureResult result = null;
        try {
            result = postAlgo.config(
                    configs.toArray(new FeatureConfig[configs.size()]),
                    new PostAlgoCallback(){
                        @Override
                        public void processResult(FeatureParam params) {
                            Log.d(TAG, "ICamPostAlgoCallback processResult " + params);
                            int[] paramArray = new int[4];
                            params.getIntArray("postalgo.autorama.motion.data", paramArray);
                            Log.d(TAG, "processResult data size " + paramArray.length);
                            for(int i:paramArray) {
                                Log.d(TAG, "array is " + i);
                            }

                        }
                    });
        } catch (RemoteException ex) {
            ex.printStackTrace();
            Log.e(TAG, "Postalgo config feature failed.");
        }
        // check config
        assertNotNull(result);
        assertTrue(result.getStreams().size() > 0);
        Surface sf = result.getStreams().elementAt(0).getmSurface();

        FeatureParam startCmd = new FeatureParam();
        startCmd.appendInt("postalgo.autorama.cmd", 3);//AUTORAMA_START
        try {
            postAlgo.configParams(FeaturePipeConfig.INDEX_PREVIEW, startCmd);
            Log.d(TAG, "startCmd done!");
        } catch (RemoteException ex) {
            ex.printStackTrace();
            Log.e(TAG, "startCmd config feature failed.");
        }

        Canvas canvas = null;
        for(int i= 0; i < 4; i++) {
            canvas = sf.lockCanvas(null);
            assertNotNull(canvas);
            Log.d(TAG, "Paint Blue!");
            canvas.drawColor(Color.BLUE);
            sf.unlockCanvasAndPost(canvas);
        }
        FeatureParam mergeCmd = new FeatureParam();
        mergeCmd.appendInt("postalgo.autorama.cmd", 1);//AUTORAMA_MERGE
        try {
            postAlgo.configParams(FeaturePipeConfig.INDEX_PREVIEW, mergeCmd);
            Log.d(TAG, "mergeCmd done!");
        } catch (RemoteException ex) {
            ex.printStackTrace();
            Log.e(TAG, "mergeCmd config feature failed.");
        }
        //trigger complete
        canvas = sf.lockCanvas(null);
        assertNotNull(canvas);
        canvas.drawColor(Color.BLUE);
        sf.unlockCanvasAndPost(canvas);
        try {
            Thread.sleep(2000);
        } catch (InterruptedException e) {
            Log.d(TAG, "error sleep!");
        }
        if (postAlgo != null) {
            try {
                 postAlgo.disconnect();
            } catch (RemoteException ex) {
                ex.printStackTrace();
            }
        }
        verify(mReaderListener, atLeast(1)).onImageAvailable(any(ImageReader.class));
    }

}
