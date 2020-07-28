package com.mediatek.plugin;

import com.mediatek.plugin.element.Extension;
import com.mediatek.plugin.element.ExtensionPoint;
import com.mediatek.plugin.element.PluginDescriptor;
import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.TraceHelper;

import java.util.HashMap;
import java.util.Set;

/**
 * PluginRegistry, manager all plugins, and provide extension and extension point helper methods,
 * and generate some relationship.
 */
public class PluginRegistry {
    private String TAG = "PluginManager/PluginRegistry";

    private HashMap<String, PluginEntry> mAllPlugins = new HashMap<String, PluginEntry>();

    /**
     * Get the plugin descriptor of specified plugin id.
     * @param pluginId
     *            The id of plugin that want to search
     * @return instance of specified plugin descriptor
     */
    public PluginDescriptor getPluginDescriptor(String pluginId) {
        Log.d(TAG, "<getPluginDescriptor> pluginId is " + pluginId);
        if (mAllPlugins.isEmpty()) {
            Log.d(TAG, "<getPluginDescriptor> mAllPlugins is empty!");
            return null;
        }
        if (mAllPlugins.containsKey(pluginId)) {
            return mAllPlugins.get(pluginId).getDescriptor();
        }
        return null;
    }

    /**
     * Get the extension point of specified point id.
     * @param pointId
     *            the id of extension point
     * @return instance of specified extension point
     */
    public ExtensionPoint getExtensionPointById(String pointId) {
        Log.d(TAG, "<getExtensionPointById> pointId is " + pointId);
        if (mAllPlugins.isEmpty()) {
            Log.d(TAG, "<getExtensionPointById> mAllPlugins is empty!");
            return null;
        }
        ExtensionPoint extentionPoint = null;
        for (HashMap.Entry<String, PluginEntry> pluginEntry : mAllPlugins.entrySet()) {
            PluginDescriptor descriptor = pluginEntry.getValue().getDescriptor();
            if (descriptor == null) {
                Log.d(TAG, "<getExtensionPointById> descriptor is null");
                continue;
            }
            if (descriptor.getExtensionPoints() == null) {
                Log.d(TAG, "<getExtensionPointById> descriptor.getExtensionPoints() is null");
                continue;
            }
            extentionPoint = descriptor.getExtensionPoints().get(pointId);
            if (extentionPoint != null) {
                Log.d(TAG, "<getExtensionPointById> extentionPoint is not null, id " + pointId);
                return extentionPoint;
            }
        }
        return extentionPoint;
    }

    /**
     * Get the extension point of specified point class name.
     * @param pointClassName
     *            the class name of extension point
     * @return instance of specified extension point
     */
    public ExtensionPoint getExtensionPointByClass(String pointClassName) {
        Log.d(TAG, "<getExtensionPointByClass> pointClassName is " + pointClassName);
        if (mAllPlugins.isEmpty()) {
            Log.d(TAG, "<getExtensionPointByClass> mAllPlugins is empty!");
            return null;
        }
        for (HashMap.Entry<String, PluginEntry> pluginEntry : mAllPlugins.entrySet()) {
            HashMap<String, ExtensionPoint> mExtensionPoints =
                    pluginEntry.getValue().getDescriptor().getExtensionPoints();
            if (mExtensionPoints == null || mExtensionPoints.isEmpty()) {
                continue;
            }
            for (HashMap.Entry<String, ExtensionPoint> extentionPointEntry : mExtensionPoints
                    .entrySet()) {
                if (extentionPointEntry.getValue().className.equals(pointClassName)) {
                    return extentionPointEntry.getValue();
                }
            }
        }
        return null;
    }

    /**
     * Search in all plugins, and get id of plugin that from include specified extension.
     * @param extensionClassName
     *            specified extension name
     * @return plugin id
     */
    public String getPluginIdByExtensionClass(String extensionClassName) {
        Log.d(TAG, "<getPluginIdByExtensionClass> extensionClassName is " + extensionClassName);
        if (mAllPlugins.isEmpty()) {
            Log.d(TAG, "<getPluginIdByExtensionClass> mAllPlugins is empty!");
            return null;
        }
        for (HashMap.Entry<String, PluginEntry> pluginEntry : mAllPlugins.entrySet()) {
            HashMap<String, Extension> mExtensions =
                    pluginEntry.getValue().getDescriptor().getExtension();
            if (mExtensions == null || mExtensions.isEmpty()) {
                continue;
            }
            for (HashMap.Entry<String, Extension> extentionEntry : mExtensions.entrySet()) {
                if (extentionEntry.getValue().className.equals(extensionClassName)) {
                    return pluginEntry.getKey();
                }
            }
        }
        return null;
    }

    /**
     * Search in plugin resistry, get the plugin of specified plugin id.
     * @param pluginId
     *            plugin id
     * @return actived plugin instance
     */
    public Plugin getPlugin(String pluginId) {
        Log.d(TAG, "<getPlugin> pluginId is " + pluginId);
        if (mAllPlugins.isEmpty()) {
            Log.d(TAG, "<getPlugin> mAllPlugins is empty!");
            return null;
        }
        if (mAllPlugins.containsKey(pluginId)) {
            return mAllPlugins.get(pluginId).getPlugin();
        }
        return null;
    }

    /**
     * Add one plugin descriptor to plugin registry.
     * @param pluginDescriptor
     *            specified plugin descriptor
     */
    public synchronized void addPluginDescriptor(PluginDescriptor pluginDescriptor) {
        if (pluginDescriptor == null) {
            Log.d(TAG, "<addPluginDescriptor> pluginDescriptor is null!");
            return;
        }
        if (!mAllPlugins.isEmpty() && mAllPlugins.containsKey(pluginDescriptor.id)) {
            PluginDescriptor descriptor = mAllPlugins.get(pluginDescriptor.id).getDescriptor();
            if (descriptor != null && descriptor.version >= pluginDescriptor.version) {
                Log.d(TAG, "<addPluginDescriptor> already contains this pluginDescriptor!");
                return;
            } else {
                if (descriptor != null) {
                    descriptor.release();
                }
                mAllPlugins.remove(pluginDescriptor.id);
            }
        }
        PluginEntry pluginEntry = new PluginEntry(pluginDescriptor);
        mAllPlugins.put(pluginDescriptor.id, pluginEntry);
    }

    /**
     * Add specified plugin to plugin registry.
     * @param pluginDescriptorId
     *            plugin id
     * @param plugin
     *            instance of plugin
     */
    public void setPlugin(String pluginDescriptorId, Plugin plugin) {
        Log.d(TAG, "<setPlugin> pluginDescriptorId is " + pluginDescriptorId + ", plugin is "
                + plugin);
        if (mAllPlugins.isEmpty()) {
            Log.d(TAG, "<setPlugin> mAllPlugins is empty!");
            return;
        }
        PluginEntry pluginEntry = mAllPlugins.get(pluginDescriptorId);
        pluginEntry.setPlugin(plugin);
    }

    /**
     * Get all names of plugins.
     * @return all names of plugins
     */
    public String[] getAllPluginsName() {
        if (mAllPlugins.isEmpty()) {
            Log.d(TAG, "<getAllPluginsName> mAllPlugins is empty!");
            return null;
        }
        String[] pluginsName = new String[mAllPlugins.size()];
        int i = 0;
        for (HashMap.Entry<String, PluginEntry> pluginEntry : mAllPlugins.entrySet()) {
            PluginDescriptor descriptor = pluginEntry.getValue().getDescriptor();
            if (descriptor != null) {
                pluginsName[i] = descriptor.name;
                Log.d(TAG, "<getAllPluginsName> pluginsName[" + i + "] , " + pluginsName[i]);
                i++;
            }
        }
        return pluginsName;
    }

    /**
     * Get all id of plugins.
     * @return all id of plugins
     */
    public Set<String> getAllPluginsId() {
        return mAllPlugins.keySet();
    }

    /**
     * Generate all extensionPoints connected all extensions.
     */
    public void generateRelationship() {
        TraceHelper.beginSection(">>>>PluginRegistry-generateRelationship");
        if (mAllPlugins.isEmpty()) {
            Log.d(TAG, "<generateRelationship> mAllPlugins is empty!");
            TraceHelper.endSection();
            return;
        }

        // Get all extension and all extension point
        HashMap<String, Extension> allExtensions = new HashMap<String, Extension>();
        HashMap<String, ExtensionPoint> allExtensionPoints =
                new HashMap<String, ExtensionPoint>();
        getAllExtensionsAndPoints(allExtensions, allExtensionPoints);
        if (allExtensions.isEmpty() && allExtensionPoints.isEmpty()) {
            TraceHelper.endSection();
            return;
        }

        // Find extension and extension point relationship
        setConnectedExtensions(allExtensions, allExtensionPoints);

        // print all PluginDescriptor info
        /*for (Entry<String, PluginEntry> entry : mAllPlugins.entrySet()) {
            entry.getValue().getDescriptor().printf();
        }*/
        TraceHelper.endSection();
    }

    private void getAllExtensionsAndPoints(HashMap<String, Extension> allExtensions,
                                           HashMap<String, ExtensionPoint> allExtensionPoints) {
        TraceHelper.beginSection(">>>>PluginRegistry-getAllExtensionsAndPoints");
        for (HashMap.Entry<String, PluginEntry> pluginEntry : mAllPlugins.entrySet()) {
            PluginDescriptor descriptor = pluginEntry.getValue().getDescriptor();
            if (descriptor == null) {
                TraceHelper.endSection();
                return;
            }
            setRequirePluginDes(descriptor);
            boolean extensionCollectionIsEmpty =
                    (descriptor.getExtension() == null || descriptor.getExtension().isEmpty());
            if (!extensionCollectionIsEmpty) {
                allExtensions.putAll(descriptor.getExtension());
            }
            boolean extensionPointsCollectionIsEmpty =
                    (descriptor.getExtensionPoints() == null || descriptor
                            .getExtensionPoints().isEmpty());
            if (!extensionPointsCollectionIsEmpty) {
                allExtensionPoints.putAll(descriptor.getExtensionPoints());
            }
        }
        TraceHelper.endSection();
    }

    private void setConnectedExtensions(HashMap<String, Extension> allExtensions,
                                        HashMap<String, ExtensionPoint> allExtensionPoints) {
        TraceHelper.beginSection(">>>>PluginRegistry-setConnectedExtensions");
        for (HashMap.Entry<String, ExtensionPoint> pointEntry : allExtensionPoints.entrySet()) {
            HashMap<String, Extension> connectedExtensions = new HashMap<String, Extension>();
            ExtensionPoint extensionPoint = pointEntry.getValue();
            for (HashMap.Entry<String, Extension> extentionEntry : allExtensions.entrySet()) {
                if (pointEntry.getValue().id
                        .equals(extentionEntry.getValue().extensionPointId)) {
                    Log.d(TAG, "<generateRelationship> Extension " + pointEntry.getValue().id);
                    connectedExtensions
                            .put(extentionEntry.getKey(), extentionEntry.getValue());
                }
            }
            if (connectedExtensions.size() != 0 && extensionPoint != null) {
                Log.d(TAG, "<generateRelationship> extensionPoint " + extensionPoint.id);
                extensionPoint.setConnectedExtensions(connectedExtensions);
            }
        }
        TraceHelper.endSection();
    }

    private void setRequirePluginDes(PluginDescriptor descriptor) {
        TraceHelper.beginSection(">>>>PluginRegistry-setRequirePluginDes");
        String[] requiredPluginIds = descriptor.requiredPluginIds;
        if (requiredPluginIds != null && requiredPluginIds.length > 0) {
            HashMap<String, PluginDescriptor> requiredPluginDescriptors =
                    new HashMap<String, PluginDescriptor>();
            PluginDescriptor requiredPluginDes = null;
            int requiredPluginLength = requiredPluginIds.length;
            for (int i = 0; i < requiredPluginLength; i++) {
                requiredPluginDes = getPluginDescriptor(requiredPluginIds[i]);
                if (requiredPluginDes != null) {
                    requiredPluginDescriptors.put(requiredPluginIds[i], requiredPluginDes);
                }
            }
            descriptor.setRequirePluginDes(requiredPluginDescriptors);
        }
        TraceHelper.endSection();
    }

    private class PluginEntry {
        private PluginDescriptor mDescriptor;
        private Plugin mPlugin;

        public PluginEntry(PluginDescriptor des) {
            mDescriptor = des;
        }

        public void setPlugin(Plugin plugin) {
            mPlugin = plugin;
        }

        public Plugin getPlugin() {
            return mPlugin;
        }

        public PluginDescriptor getDescriptor() {
            return mDescriptor;
        }
    }
}
