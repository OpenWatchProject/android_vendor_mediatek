package com.mediatek.gallery3d.adapter;

import android.content.Context;
import android.os.Environment;
import android.util.Log;

import com.android.gallery3d.common.Utils;
import com.mediatek.gallerybasic.base.IActivityCallback;
import com.mediatek.gallerybasic.base.IDecodeOptionsProcessor;
import com.mediatek.gallerybasic.base.IFieldDefinition;
import com.mediatek.gallerybasic.base.IFilter;
import com.mediatek.gallerybasic.base.MediaMember;
import com.mediatek.plugin.PluginManager;
import com.mediatek.plugin.PluginManager.PreloaderListener;
import com.mediatek.plugin.PluginUtility;
import com.mediatek.plugin.element.Extension;

import java.lang.reflect.Array;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

/**
 * Initialize plugin environment, and interact with PluginManager.
 */
public class FeatureManager {
    private static final String TAG = "MtkGallery2/FeatureManager";
    private static final String PLUGIN_DIR_PATH1 = "/product/app/MtkGallery2/galleryfeature";
    private static FeatureManager sFeatureManager;

    private Context mContext;
    private PluginManager mPluginManager;
    private boolean mLoadingFinished = false;
    private Object mWaitLoading = new Object();

    private FeatureManager(Context context) {
        mContext = context;
    }

    /**
     * Initialize the running environment of FeatureManager:
     * 1. New instance
     * 2. Initialize: preload plugin
     *
     * @param context Current application context
     */
    public synchronized static void setup(Context context) {
        if (sFeatureManager == null) {
            Log.d(TAG, "<setup> new FeatureManager, initialize");
            sFeatureManager = new FeatureManager(context);
            sFeatureManager.initialize();
        } else {
            Log.d(TAG, "<setup> Has been set up, do nothing, return");
        }
    }

    /**
     * Get the instance of FeatureManager. Before call this, you must call
     * {@link FeatureManager#setup(Context)} at first, or else getInstance() will return null.
     *
     * @return The instance of FeatureManager.
     */
    public static FeatureManager getInstance() {
        return sFeatureManager;
    }

    private void initialize() {
        mPluginManager = PluginManager.getInstance(mContext);

        mPluginManager.addPluginDir(PLUGIN_DIR_PATH1);
        Log.d(TAG, "<initialize> preloadAllPlugins");
        mPluginManager.preloadAllPlugins(true, false, true, new PreloaderListener() {
            @Override
            public void onPreloadFinished() {
                mLoadingFinished = true;
                preloadImplements();
                synchronized (mWaitLoading) {
                    mWaitLoading.notifyAll();
                }
            }
        });
    }

    /**
     * Performance optimization for plugin flow, get the instance partly in service
     * when device boot completed, new flow can reduce first launch time.
     */
    private void preloadImplements() {
        FeatureManager.getInstance().getImplement(IFilter.class);
        FeatureManager.getInstance().getImplement(IActivityCallback.class);
        FeatureManager.getInstance().getImplement(IFieldDefinition.class);
        FeatureManager.getInstance().getImplement(MediaMember.class, mContext,
                null, mContext.getResources());
        FeatureManager.getInstance().getImplement(IDecodeOptionsProcessor.class);
    }

    /**
     * Get the instance list of specified extension point class.
     *
     * @param T The extension point class
     * @param args The parameter of constructor
     * @return The instance list
     */
    public <T> T[] getImplement(Class<?> T, Object... args) {
        waitToPreloadFinished();
        Map<String, Extension> extensions = PluginUtility.getExt(mPluginManager, T);
        if (extensions == null || extensions.size() == 0) {
            return (T[]) Array.newInstance(T, 0);
        }
        T[] res = (T[]) Array.newInstance(T, extensions.size());
        int index = 0;
        int nullObjNum = 0;
        Iterator<Entry<String, Extension>> itr = extensions.entrySet().iterator();
        while (itr.hasNext()) {
            Object obj = PluginUtility.createInstance(mPluginManager, itr.next().getValue(),
                    args);
            res[index++] = (T) obj;
            if (obj == null) {
                nullObjNum++;
            }
        }
        if (nullObjNum == 0) {
            return res;
        } else {
            index = 0;
            T[] newRes = (T[]) Array.newInstance(T, res.length - nullObjNum);
            for (T obj : res) {
                if (obj != null) {
                    newRes[index++] = obj;
                }
            }
            return newRes;
        }

    }

    private void waitToPreloadFinished() {
        while (!mLoadingFinished) {
            synchronized (mWaitLoading) {
                Log.d(TAG, "<waitToPreloadFinished> wait...");
                Utils.waitWithoutInterrupt(mWaitLoading);
                Log.d(TAG, "<waitToPreloadFinished> preload finished");
            }
        }

    }
}
