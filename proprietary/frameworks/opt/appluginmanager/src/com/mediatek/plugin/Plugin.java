package com.mediatek.plugin;

import com.mediatek.plugin.element.PluginDescriptor;

/**
 * Default Plugin class.
 */
public class Plugin {
    private ClassLoader mClassLoader;
    private PluginDescriptor mDescriptor;

    /**
     * Plugin constructor.
     * @param des
     *            instance of PluginDescriptor
     * @param loader
     *            this plugin class loader
     */
    public Plugin(PluginDescriptor des, ClassLoader loader) {
        mDescriptor = des;
        mClassLoader = loader;
    }

    /**
     * Plugin start.
     */
    public void start() {
        doStart();
    }

    /**
     * Plugin stop.
     */
    public void stop() {
        doStop();
    }

    /**
     * Get plugin class loader.
     * @return instance of ClassLoader
     */
    public ClassLoader getClassLoader() {
        return mClassLoader;
    }

    /**
     * Get plugin descriptor.
     * @return instance of PluginDescriptor
     */
    public PluginDescriptor getDescriptor() {
        return mDescriptor;
    }

    /**
     * In child class, override this function to do some initialize operation when start.
     */
    protected void doStart() {

    }

    /**
     * In child class, override this function to do some initialize operation when stop.
     */
    protected void doStop() {

    }
}
