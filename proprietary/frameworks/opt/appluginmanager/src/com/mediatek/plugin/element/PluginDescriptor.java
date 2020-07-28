package com.mediatek.plugin.element;

import android.content.pm.PackageInfo;
import android.content.res.AssetManager;
import android.content.res.Resources;

import java.util.HashMap;

/**
 * PluginDescriptor class represent for plugin root tag.
 */
public class PluginDescriptor extends Element {
    // Public attribute comes from xml
    public String name;
    public int version;
    public int requireMaxHostVersion = Integer.MAX_VALUE;
    public int requireMinHostVersion = Integer.MIN_VALUE;
    public String className;
    public String[] requiredPluginIds;

    // Attribute about relationship
    private HashMap<String, PluginDescriptor> mRequiredPluginDescriptor;

    // Attribute comes from other ways
    private String mArchivePath;
    private Resources mResources;
    private PackageInfo mInfo;
    private AssetManager mAssetManager;

    public HashMap<String, PluginDescriptor> getRequirePluginDes() {
        return mRequiredPluginDescriptor;
    }

    public HashMap<String, ExtensionPoint> getExtensionPoints() {
        return (HashMap) mChilds.get(ExtensionPoint.class);
    }

    public HashMap<String, Extension> getExtension() {
        return (HashMap) mChilds.get(Extension.class);
    }

    public String getArchivePath() {
        return mArchivePath;
    }

    public void setArchivePath(String archivePath) {
        mArchivePath = archivePath;
    }

    public void setPackageInfo(PackageInfo info) {
        mInfo = info;
    }

    public PackageInfo getPackageInfo() {
        return mInfo;
    }

    public void setResource(Resources res) {
        mResources = res;
    }

    public Resources getResources() {
        return mResources;
    }

    public void setAssetManager(AssetManager assetManager) {
        mAssetManager = assetManager;
    }

    public AssetManager getAssetManager() {
        return mAssetManager;
    }

    /**
     * Set the PluginDescriptor that extend current PluginDescriptor.
     * @param requiredPluginDescriptor
     *            The collection of PluginDescriptor
     */
    public void
            setRequirePluginDes(HashMap<String, PluginDescriptor> requiredPluginDescriptor) {
        mRequiredPluginDescriptor = requiredPluginDescriptor;
    }

    /**
     * Release.
     */
    public void release() {
        // TODO Auto-generated method stub
    }

    @Override
    public void printAllKeyValue(String prefix) {
        super.printAllKeyValue(prefix);
        printKeyValue(prefix, "name", String.valueOf(name));
        printKeyValue(prefix, "version", String.valueOf(version));
        printKeyValue(prefix, "requireMaxHostVersion", String.valueOf(requireMaxHostVersion));
        printKeyValue(prefix, "requireMinHostVersion", String.valueOf(requireMinHostVersion));
        printKeyValue(prefix, "className", String.valueOf(className));
        if (requiredPluginIds != null) {
            int requiredPluginLength = requiredPluginIds.length;
            if (requiredPluginLength == 0) {
                printKeyValue(prefix, "requiredPluginIds size ", String.valueOf(0));
            }
            for (int i = 0; i < requiredPluginLength; i++) {
                printKeyValue(prefix, "requiredPluginIds " + i + " ", String
                        .valueOf(requiredPluginIds[i]));
            }
        } else {
            printKeyValue(prefix, "requiredPluginIds size ", String.valueOf(0));
        }
        printKeyValue(prefix, "mArchivePath", String.valueOf(mArchivePath));
    }
}
