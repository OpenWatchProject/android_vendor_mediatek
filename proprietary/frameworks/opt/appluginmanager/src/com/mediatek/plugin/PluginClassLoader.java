package com.mediatek.plugin;

import dalvik.system.PathClassLoader;

import java.util.ArrayList;

/**
 * Plugin class loader, for load plugin related class.
 */
class PluginClassLoader extends PathClassLoader {
    private ArrayList<ClassLoader> mRequiredLoaders;

    /**
     * PluginClassLoader constructor.
     * @param dexPath
     * @param libraryPath
     * @param parent
     */
    public PluginClassLoader(String dexPath, String libraryPath, ClassLoader parent) {
        super(dexPath, libraryPath, parent);
    }

    @Override
    public Class<?> loadClass(String className) throws ClassNotFoundException {
        Class<?> clazz = null;
        // 1. load class from parent
        try {
            clazz = getParent().loadClass(className);
        } catch (ClassNotFoundException e) {
            // skip
        }
        if (clazz != null) {
            return clazz;
        }

        // 2. if not find, load class from self
        try {
            clazz = super.loadClass(className);
        } catch (ClassNotFoundException e) {
            // skip
        }
        if (clazz != null) {
            return clazz;
        }

        // 3. if not find, load class from required ClassLoader
        if (mRequiredLoaders == null) {
            return null;
        }
        int classLoaderSize = mRequiredLoaders.size();
        if (classLoaderSize <= 0) {
            return null;
        }
        for (int i = 0; i < classLoaderSize; i++) {
            try {
                clazz = mRequiredLoaders.get(i).loadClass(className);
            } catch (ClassNotFoundException e) {
                // skip
            }
            if (clazz != null) {
                return clazz;
            }
        }
        throw new ClassNotFoundException("Cannot find " + className, new Throwable());
    }

    /**
     * Set required class loader, be used to load class.
     * @param classLoders
     *            required class loader collection
     */
    public void setRequiredClassLoader(ArrayList<ClassLoader> classLoders) {
        mRequiredLoaders = classLoders;
    }
}
