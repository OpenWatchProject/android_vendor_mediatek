package com.mediatek.plugin;

import android.content.Context;

import com.mediatek.plugin.element.PluginDescriptor;
import com.mediatek.plugin.parallel.Future;
import com.mediatek.plugin.parallel.FutureListener;
import com.mediatek.plugin.parallel.ThreadPool;
import com.mediatek.plugin.parallel.ThreadPool.Job;
import com.mediatek.plugin.parallel.ThreadPool.JobContext;
import com.mediatek.plugin.preload.Preloader;
import com.mediatek.plugin.utils.FileUtils;
import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.ReflectUtils;
import com.mediatek.plugin.utils.TraceHelper;

import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Set;
import java.util.concurrent.CountDownLatch;

/**
 * PluginManager, load plugin entrance, and manage all plugins.
 */
public class PluginManager {
    private static final String TAG = "PluginManager/PluginManager";
    private static final String NATIVE_LIB_DIR = "plugin_manager_lib";

    private ArrayList<String> mArchiveDir = new ArrayList<String>();
    private String mNativeLibDir;

    private static PluginManager sPluginManager;
    private PluginRegistry mRegistry;
    private Context mContext;

    /**
     * Get instance of PluginManager.
     * @param context
     *            for create instance.
     * @return instance of PluginManager
     */
    public static PluginManager getInstance(Context context) {
        if (sPluginManager == null) {
            sPluginManager = new PluginManager(context);
        }
        return sPluginManager;
    }

    /**
     * When plugin preload successfully, call this callback to broadcast host.
     */
    public static interface PreloaderListener {
        public void onPreloadFinished();
    }

    /**
     * PluginManager constructor.
     * @param context
     *            host context
     */
    protected PluginManager(Context context) {
        mContext = context.getApplicationContext();
        mNativeLibDir =
                mContext.getDir(NATIVE_LIB_DIR, Context.MODE_PRIVATE).getAbsolutePath();
        mRegistry = new PluginRegistry();
    }

    /**
     * Add plugin apk or jar path.
     * @param path
     *            apk or jar path
     */
    public void addPluginDir(String path) {
        Log.d(TAG, "<addPluginDir> path = " + path);
        mArchiveDir.add(path);
    }

    /**
     * Pre-load all plugins, enable signature check and xml validate by default.
     * @param listener
     *            load finish callback listener
     */
    public void preloadAllPlugins(final PreloaderListener listener) {
        preloadAllPlugins(true, true, true, listener);
    }

    /**
     * Pre-load all plugins.
     * @param signatureCheckEnabled
     *            True, check if signature of plugin matches with host before preload, if not match,
     *            not preload this plugin. False, not check.
     * @param xmlValidateEnabled
     *            True, check if plugin.xml matches the rule before preload, if not match, not
     *            preload this plugin. False, not check.
     * @param preloadPluginClassEnabled
     *            True, preload all plugins's class. False, not preload.
     * @param listener
     *            load finish callback listener
     */
    public void preloadAllPlugins(final boolean signatureCheckEnabled,
                                  final boolean xmlValidateEnabled,
                                  final boolean preloadPluginClassEnabled,
                                  final PreloaderListener listener) {
        final ArrayList<String> archivePaths = getAllArchivePath();
        if (archivePaths == null || archivePaths.size() == 0) {
            Log.d(TAG, "<preloadAllPlugins> archivePaths empty, call onPreloadFinished directly");
            listener.onPreloadFinished();
            return;
        }
        int pluginCount = archivePaths.size();
        final CountDownLatch latch = new CountDownLatch(pluginCount);
        for (int i = 0; i < pluginCount; i++) {
            final int index = i;
            Job<Void> job = new Job<Void>() {
                @Override
                public Void run(JobContext jc) {
                    Log.d(TAG, "<preloadAllPlugins> plugin path " + archivePaths.get(index));
                    PluginDescriptor pluginDescriptor =
                            Preloader.getInstance().preloadPlugin(mContext,
                                    archivePaths.get(index), mNativeLibDir,
                                    signatureCheckEnabled, xmlValidateEnabled);
                    Log.d(TAG, "<preloadAllPlugins> pluginDescriptor " + pluginDescriptor);
                    if (pluginDescriptor != null) {
                        mRegistry.addPluginDescriptor(pluginDescriptor);
                    }
                    return null;
                }
            };
            ThreadPool.getInstance().submit(job, new FutureListener<Void>() {
                @Override
                public synchronized void onFutureDone(Future<Void> future) {
                    latch.countDown();
                    Log.d(TAG, "<preloadAllPlugins.onFutureDone> latch count " + latch.getCount());
                    if (latch.getCount() != 0) {
                        return;
                    }
                    mRegistry.generateRelationship();
                    listener.onPreloadFinished();
                    Log.d(TAG, "<preloadAllPlugins.onFutureDone> onPreloadFinished done!");
                    if (!preloadPluginClassEnabled) {
                        return;
                    }
                    Set<String> pluginsId = mRegistry.getAllPluginsId();
                    for (String pluginId : pluginsId) {
                        ThreadPool.getInstance().submit(new Job<Void>() {
                            @Override
                            public Void run(JobContext jc) {
                                getPlugin(pluginId);
                                return null;
                            }
                        });
                    }
                }
            });
        }
    }

    /**
     * Get instance of PluginRegistry.
     * @return instance of PluginRegistry
     */
    public PluginRegistry getRegistry() {
        return mRegistry;
    }

    /**
     * Get one speical plugin, and if not active, active it.
     * @param pluginId
     *            plugin id
     * @return actived plugin instance
     */
    public Plugin getPlugin(String pluginId) {
        TraceHelper.beginSection(">>>>PluginManager-getPlugin");
        Log.d(TAG, "<getPlugin> begin");
        PluginDescriptor pluginDescriptor = mRegistry.getPluginDescriptor(pluginId);
        if (pluginDescriptor == null) {
            Log.d(TAG, "<getPlugin> pluginDescriptor is null");
            TraceHelper.endSection();
            return null;
        }
        synchronized (pluginDescriptor) {
            Plugin plugin = mRegistry.getPlugin(pluginId);
            if (plugin != null) {
                Log.d(TAG, "<getPlugin> plugin != null");
                TraceHelper.endSection();
                return plugin;
            }
            // Active this plugin
            plugin = activePlugin(pluginId, pluginDescriptor);
            TraceHelper.endSection();
            return plugin;
        }
    }

    private Plugin activePlugin(String pluginId, PluginDescriptor pluginDescriptor) {
        TraceHelper.beginSection(">>>>PluginManager-activePlugin");
        Log.d(TAG, "<activePlugin> begin, pluginId " + pluginId);
        Plugin plugin = null;
        // Check required plugin is active, if not and requirePluginDes size is
        // 0, active it.
        HashMap<String, PluginDescriptor> requirePluginDes =
                pluginDescriptor.getRequirePluginDes();
        if (requirePluginDes == null || requirePluginDes.isEmpty()) {
            plugin = doActivePlugin(pluginId, pluginDescriptor);
            TraceHelper.endSection();
            Log.d(TAG,
                    "<activePlugin> end, requirePluginDes size is 0, doactive itsself! plugin id "
                            + pluginId);
            return plugin;
        }
        Plugin requirePlugin;
        String requirePluginId;
        for (HashMap.Entry<String, PluginDescriptor> requirePluginDesEntry : requirePluginDes
                .entrySet()) {
            if (requirePluginDesEntry.getValue() != null) {
                requirePluginId = requirePluginDesEntry.getValue().id;
                requirePlugin = mRegistry.getPlugin(requirePluginId);
                if (requirePlugin == null) {
                    requirePlugin =
                            activePlugin(requirePluginId, requirePluginDesEntry.getValue());
                    if (requirePlugin == null) {
                        throw new IllegalStateException("Active required plugin failed!");
                    } else {
                        plugin = doActivePlugin(pluginId, pluginDescriptor);
                        TraceHelper.endSection();
                        Log.d(TAG, "<activePlugin> end, doactive itsself! plugin id "
                                + pluginId);
                        return plugin;
                    }
                }
            }
        }
        Log.d(TAG, "<activePlugin> end, retun null");
        TraceHelper.endSection();
        return null;
    }

    private Plugin doActivePlugin(String pluginId, PluginDescriptor pluginDescriptor) {
        TraceHelper.beginSection(">>>>PluginManager-doActivePlugin");
        Log.d(TAG, "<doActivePlugin> begin, pluginId " + pluginId);
        PluginClassLoader pluginClassLoader =
                new PluginClassLoader(pluginDescriptor.getArchivePath(),
                        mNativeLibDir, getClass().getClassLoader());
        pluginClassLoader.setRequiredClassLoader(getRequiredClassLoader(pluginDescriptor));
        try {
            Log.d(TAG, "<doActivePlugin> pluginDescriptor.className "
                    + pluginDescriptor.className);
            Class<?> pluginClass = pluginClassLoader.loadClass(pluginDescriptor.className);
            Constructor<?> pluginCons =
                    ReflectUtils.getConstructor(pluginClass, PluginDescriptor.class,
                            ClassLoader.class);
            Plugin plugin =
                    (Plugin) ReflectUtils.createInstance(pluginCons, pluginDescriptor,
                            pluginClassLoader);
            plugin.start();
            mRegistry.setPlugin(pluginId, plugin);
            TraceHelper.endSection();
            return plugin;
        } catch (ClassNotFoundException e) {
            Log.d(TAG, "<doActivePlugin> ClassNotFoundException, pluginId " + pluginId);
            e.printStackTrace();
            TraceHelper.endSection();
            return null;
        }
    }

    private ArrayList<ClassLoader> getRequiredClassLoader(PluginDescriptor pluginDescriptor) {
        TraceHelper.beginSection(">>>>PluginManager-getRequiredClassLoader");
        Log.d(TAG, "<getRequiredClassLoader> begin");
        ArrayList<ClassLoader> classLoders = new ArrayList<ClassLoader>();
        HashMap<String, PluginDescriptor> requirePluginDes =
                pluginDescriptor.getRequirePluginDes();
        if (requirePluginDes == null) {
            Log.d(TAG, "<getRequiredClassLoader> end, requirePluginDes == null");
            TraceHelper.endSection();
            return null;
        }
        Plugin requirePlugin = null;
        String requirepluginId;
        for (HashMap.Entry<String, PluginDescriptor> requirePluginDesEntry : requirePluginDes
                .entrySet()) {
            requirepluginId = requirePluginDesEntry.getValue().id;
            requirePlugin = mRegistry.getPlugin(requirepluginId);
            if (requirePlugin != null) {
                classLoders.add(requirePlugin.getClassLoader());
            }
        }
        Log.d(TAG, "<getRequiredClassLoader> end");
        TraceHelper.endSection();
        return classLoders;
    }

    private ArrayList<String> getAllArchivePath() {
        Log.d(TAG, "<getAllArchivePath> begin");
        ArrayList<String> archivePaths = new ArrayList<String>();
        for (String dir : mArchiveDir) {
            ArrayList<String> filePathList = FileUtils.getAllFile(dir);
            for (String path : filePathList) {
                if (path.endsWith(".jar") || path.endsWith(".apk")) {
                    archivePaths.add(path);
                    Log.d(TAG, "<getAllArchivePath> " + path);
                }
            }
        }
        Log.d(TAG, "<getAllArchivePath> end");
        return archivePaths;
    }
}
