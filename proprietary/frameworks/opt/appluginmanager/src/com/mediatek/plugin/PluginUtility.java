package com.mediatek.plugin;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.plugin.element.Extension;
import com.mediatek.plugin.element.ExtensionPoint;
import com.mediatek.plugin.element.PluginDescriptor;
import com.mediatek.plugin.utils.Log;
import com.mediatek.plugin.utils.ReflectUtils;
import com.mediatek.plugin.utils.TraceHelper;

import java.lang.reflect.Constructor;
import java.util.HashMap;
import java.util.Map;

/**
 * Provide convenient methods to the user of PluginManager, make user easy to get info he wanted.
 */
public class PluginUtility {
    private static final String TAG = "PluginManager/PluginUtility";

    /**
     * Get all names of plugins.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @return The array of plugin name
     */
    public static String[] getAllPluginsName(PluginManager pm) {
        return pm.getRegistry().getAllPluginsName();
    }

    /**
     * Load the extension class of specified class name.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param className
     *            The class name of one extension
     * @return The corresponded Class of class name
     */
    public static Class<?> loadExtClass(PluginManager pm, String className) {
        String pluginId = pm.getRegistry().getPluginIdByExtensionClass(className);
        if (pluginId == null) {
            Log.d(TAG, "<loadExtClass> pluginId is null, valid!");
            return null;
        }
        Plugin plugin = pm.getPlugin(pluginId);
        if (plugin == null) {
            Log.d(TAG, "<loadExtClass> plugin is null!");
            return null;
        }
        try {
            return plugin.getClassLoader().loadClass(className);
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
            Log.d(TAG, "<loadExtClass> ClassNotFoundException! className: " + className);
            return null;
        }
    }

    /**
     * Get all class name of extension that extends from specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClassName
     *            The class name of extension point
     * @return The array of class name
     */
    public static String[] getExtClassByClassName(PluginManager pm, String pointClassName) {
        ExtensionPoint extentionPoint =
                pm.getRegistry().getExtensionPointByClass(pointClassName);
        if (extentionPoint == null) {
            Log.d(TAG, "<getExtClassByClassName> extentionPoint is null!");
            return new String[] {};
        }
        HashMap<String, Extension> connectedExtentions =
                extentionPoint.getConnectedExtensions();
        if (connectedExtentions == null) {
            Log.d(TAG, "<getExtClassByClassName> connectedExtentions is null!");
            return new String[] {};
        }

        String[] className = new String[connectedExtentions.size()];
        int i = 0;
        for (HashMap.Entry<String, Extension> connectedExtention : connectedExtentions
                .entrySet()) {
            Extension extension = connectedExtention.getValue();
            if (extension != null) {
                className[i] = extension.className;
                Log.d(TAG, "<getExtClassByClassName> className[" + i + "] " + className[i]);
                i++;
            }
        }
        Log.d(TAG, "<getExtClassByClassName> class name is " + className);
        return className;
    }

    /**
     * Search in the specified plugin, and get all class name of extension that extends from
     * specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClassName
     *            The class name of extension point
     * @param pluginId
     *            The id of plugin that want to search
     * @return The array of class name
     */
    public static String[] getExtClassByClassName(PluginManager pm, String pointClassName,
                                                  String pluginId) {
        ExtensionPoint extentionPoint =
                pm.getRegistry().getExtensionPointByClass(pointClassName);
        if (extentionPoint == null) {
            Log.d(TAG, "<getExtClassByClassName> extentionPoint is null!");
            return new String[] {};
        }
        HashMap<String, Extension> connectedExtentions =
                extentionPoint.getConnectedExtensions();
        if (connectedExtentions == null) {
            Log.d(TAG, "<getExtClassByClassName> connectedExtentions is null!");
            return new String[] {};
        }

        String[] className = new String[connectedExtentions.size()];
        int i = 0;
        for (HashMap.Entry<String, Extension> connectedExtention : connectedExtentions
                .entrySet()) {
            Extension extension = connectedExtention.getValue();
            if (extension != null && (pluginId).equals(extension.pluginId)) {
                className[i] = extension.className;
                Log.d(TAG, "<getExtClassByClassName> className[" + i + "] " + className[i]);
                i++;
            }
        }
        Log.d(TAG, "<getExtClassByClassName> class name is " + className);
        return className;
    }

    /**
     * Search in the specified plugin, and get all class name of extension that extends from
     * specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClass
     *            The Class of extension point
     * @return The array of class name
     */
    public static String[] getExtClass(PluginManager pm, Class<?> pointClass) {
        return getExtClassByClassName(pm, pointClass.getName());
    }

    /**
     * Get all class name of extension that extends from specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClass
     *            The Class of extension point
     * @param pluginId
     *            The id of plugin that want to search
     * @return The array of class name
     */
    public static String[] getExtClass(PluginManager pm, Class<?> pointClass, String pluginId) {
        return getExtClassByClassName(pm, pointClass.getName(), pluginId);
    }

    /**
     * Get all class name of extension that extends from specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointId
     *            The id of extension point
     * @return The array of class name
     */
    public static String[] getExtClassByPointId(PluginManager pm, String pointId) {
        ExtensionPoint extensionPoint = pm.getRegistry().getExtensionPointById(pointId);
        return getExtClassByClassName(pm, extensionPoint.className);
    }

    /**
     * Search in the specified plugin, and get all class name of extension that extends from
     * specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointId
     *            The id of extension point
     * @param pluginId
     *            The id of plugin that want to search
     * @return The array of class name
     */
    public static String[] getExtClassByPointId(PluginManager pm, String pointId,
                                                String pluginId) {
        ExtensionPoint extensionPoint = pm.getRegistry().getExtensionPointById(pointId);
        return getExtClassByClassName(pm, extensionPoint.className, pluginId);
    }

    /**
     * Get all extension name of extension that extends from specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClassName
     *            The class name of extension point
     * @return The array of extension name
     */
    public static String[] getExtNameByClassName(PluginManager pm, String pointClassName) {
        ExtensionPoint extentionPoint =
                pm.getRegistry().getExtensionPointByClass(pointClassName);
        if (extentionPoint == null) {
            Log.d(TAG, "<getExtNameByClassName> extentionPoint is null!");
            return new String[] {};
        }
        HashMap<String, Extension> connectedExtentions =
                extentionPoint.getConnectedExtensions();
        if (connectedExtentions == null) {
            Log.d(TAG, "<getExtNameByClassName> connectedExtentions is null!");
            return new String[] {};
        }

        String[] extName = new String[connectedExtentions.size()];
        int i = 0;
        for (HashMap.Entry<String, Extension> connectedExtention : connectedExtentions
                .entrySet()) {
            Extension extension = connectedExtention.getValue();
            if (extension != null) {
                extName[i] = extension.name;
                Log.d(TAG, "<getExtNameByClassName> extName[" + i + "] " + extName[i]);
                i++;
            }
        }
        Log.d(TAG, "<getExtNameByClassName> class name is " + extName);
        return extName;
    }

    /**
     * Search in the specified plugin, and get all extension name of extension that extends from
     * specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClassName
     *            The class name of extension point
     * @param pluginId
     *            The id of plugin that want to search
     * @return The array of extension name
     */
    public static String[] getExtNameByClassName(PluginManager pm, String pointClassName,
                                                 String pluginId) {
        ExtensionPoint extentionPoint =
                pm.getRegistry().getExtensionPointByClass(pointClassName);
        if (extentionPoint == null) {
            Log.d(TAG, "<getExtNameByClassName> extentionPoint is null!");
            return new String[] {};
        }
        HashMap<String, Extension> connectedExtentions =
                extentionPoint.getConnectedExtensions();
        if (connectedExtentions == null) {
            Log.d(TAG, "<getExtNameByClassName> connectedExtentions is null!");
            return new String[] {};
        }

        String[] extName = new String[connectedExtentions.size()];
        int i = 0;
        for (HashMap.Entry<String, Extension> connectedExtention : connectedExtentions
                .entrySet()) {
            Extension extension = connectedExtention.getValue();
            if (extension != null && (pluginId).equals(extension.pluginId)) {
                extName[i] = extension.name;
                Log.d(TAG, "<getExtNameByClassName> extName[" + i + "] " + extName[i]);
                i++;
            }
        }
        Log.d(TAG, "<getExtNameByClassName> class name is " + extName);
        return extName;
    }

    /**
     * Get all extension name of extension that extends from specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClass
     *            The Class of extension point
     * @return The array of extension name
     */
    public static String[] getExtName(PluginManager pm, Class<?> pointClass) {
        return getExtNameByClassName(pm, pointClass.getName());
    }

    /**
     * Search in the specified plugin, and get all extension name of extension that extends from
     * specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClass
     *            The Class of extension point
     * @param pluginId
     *            The id of plugin that want to search
     * @return The array of extension name
     */
    public static String[] getExtName(PluginManager pm, Class<?> pointClass, String pluginId) {
        return getExtNameByClassName(pm, pointClass.getName(), pluginId);
    }

    /**
     * Get all extension name of extension that extends from specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointId
     *            The id of extension point
     * @return The array of extension name
     */
    public static String[] getExtNameByPointId(PluginManager pm, String pointId) {
        ExtensionPoint extensionPoint = pm.getRegistry().getExtensionPointById(pointId);
        return getExtNameByClassName(pm, extensionPoint.className);
    }

    /**
     * Search in the specified plugin, get all extension name of extension that extends from
     * specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointId
     *            The id of extension point
     * @param pluginId
     *            The id of plugin that want to search
     * @return The array of extension name
     */
    public static String[] getExtNameByPointId(PluginManager pm, String pointId,
                                               String pluginId) {
        ExtensionPoint extensionPoint = pm.getRegistry().getExtensionPointById(pointId);
        return getExtNameByClassName(pm, extensionPoint.className, pluginId);
    }

    /**
     * Get the map of Extension that extends from specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClass
     *            The Class of extension point
     * @return The Map of extension id and extension
     */
    public static Map<String, Extension> getExt(PluginManager pm, Class<?> pointClass) {
        TraceHelper.beginSection(">>>>PluginUtility-getExt");
        ExtensionPoint extentionPoint =
                pm.getRegistry().getExtensionPointByClass(pointClass.getName());
        if (extentionPoint == null) {
            TraceHelper.endSection();
            return new HashMap<String, Extension>();
        }
        TraceHelper.endSection();
        return extentionPoint.getConnectedExtensions();
    }

    /**
     * Search in the specified plugin, and return the map of Extension that extends from specified
     * extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClass
     *            The Class of extension point
     * @param pluginId
     *            The id of plugin that want to search
     * @return The Map of extension id and extension
     */
    public static Map<String, Extension> getExt(PluginManager pm, Class<?> pointClass,
                                                String pluginId) {
        ExtensionPoint extensionPoint =
                pm.getRegistry().getExtensionPointByClass(pointClass.getName());
        return getExt(extensionPoint, pluginId);
    }

    /**
     * Get the map of Extension that extends from specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointId
     *            The id of extension point
     * @return The Map of extension id and extension
     */
    public static Map<String, Extension> getExtByPointId(PluginManager pm, String pointId) {
        ExtensionPoint extensionPoint = pm.getRegistry().getExtensionPointById(pointId);
        if (extensionPoint == null) {
            Log.d(TAG, "<loadExtClass> extensionPoint is null!");
            return new HashMap<String, Extension>();
        }
        return extensionPoint.getConnectedExtensions();
    }

    /**
     * Search in the specified plugin, and return the map of Extension that extends from specified
     * extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointId
     *            The id of extension point
     * @param pluginId
     *            The id of plugin that want to search
     * @return The Map of extension id and extension
     */
    public static Map<String, Extension> getExtByPointId(PluginManager pm, String pointId,
                                                         String pluginId) {
        ExtensionPoint extensionPoint = pm.getRegistry().getExtensionPointById(pointId);
        return getExt(extensionPoint, pluginId);
    }

    /**
     * Get the map of Extension that extends from specified extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClassName
     *            The class name of extension point
     * @return The Map of extension id and extension
     */
    public static Map<String, Extension> getExtByClassName(PluginManager pm,
                                                           String pointClassName) {
        ExtensionPoint extensionPoint =
                pm.getRegistry().getExtensionPointByClass(pointClassName);
        if (extensionPoint == null) {
            Log.d(TAG, "<loadExtClass> extensionPoint is null!");
            return new HashMap<String, Extension>();
        }
        return extensionPoint.getConnectedExtensions();
    }

    /**
     * Search in the specified plugin, and return the map of Extension that extends from specified
     * extension point.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pointClassName
     *            The class name of extension point
     * @param pluginId
     *            The id of plugin that want to search
     * @return The Map of extension id and extension
     */
    public static Map<String, Extension> getExtByClassName(PluginManager pm,
                                                           String pointClassName,
                                                           String pluginId) {
        ExtensionPoint extensionPoint =
                pm.getRegistry().getExtensionPointByClass(pointClassName);
        return getExt(extensionPoint, pluginId);
    }

    /**
     * Create the instance of {@link Extension#className}.
     * @param pm
     *            Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param e
     *            The extension
     * @param args
     *            The parameters of {@link Extension#className} constructor
     * @return The instance of {@link Extension#className}
     */
    public static Object createInstance(PluginManager pm, Extension e, Object... args) {
        TraceHelper.beginSection(">>>>PluginUtility-createInstance");
        Log.d(TAG, "<createInstance> Extension class name " + e.className);
        Plugin plugin = pm.getPlugin(e.getParent().id);
        Object instance = null;
        if (plugin == null) {
            Log.d(TAG, "<createInstance> plugin is null!");
            TraceHelper.endSection();
            return null;
        }

        try {
            Class<?> clazz = plugin.getClassLoader().loadClass(e.className);
            Log.d(TAG, "<createInstance> clazz " + clazz);
            int argLength = args.length;
            Constructor<?> cons = null;
            if (argLength == 0) {
                cons = ReflectUtils.getConstructor(clazz);
                instance = ReflectUtils.createInstance(cons);
                TraceHelper.endSection();
                return instance;
            }

            // Hand with parameter
            Class<?>[] parameterTypes = new Class<?>[argLength];
            for (int i = 0; i < argLength; i++) {
                if (args[i] == null) {
                    continue;
                }
                parameterTypes[i] = args[i].getClass();
                if (args[i] instanceof Context) {
                    parameterTypes[i] = Context.class;
                }
                if (args[i] instanceof Resources) {
                    args[i] = plugin.getDescriptor().getResources();
                    parameterTypes[i] = Resources.class;
                }
                Log.d(TAG, "<createInstance> parameterTypes " + i + ":" + parameterTypes[i]);
            }
            cons = ReflectUtils.getConstructor(clazz, parameterTypes);
            Log.d(TAG, "<createInstance> cons " + cons);
            if (cons == null) {
                TraceHelper.endSection();
                return null;
            }
            instance = ReflectUtils.createInstance(cons, args);
            TraceHelper.endSection();
            return instance;
        } catch (ClassNotFoundException ex) {
            ex.printStackTrace();
            TraceHelper.endSection();
            return null;
        }
    }

    /**
     * Get the Resources of one plugin.
     * @param pm Current PluginManager, the PluginManager itself is single instance, see
     *            {@link PluginManager#getInstance(android.content.Context)}
     * @param pluginId The plugin id
     * @return The Resources of specified plugin
     */
    public static Resources getResources(PluginManager pm, String pluginId) {
        Plugin plugin = pm.getPlugin(pluginId);
        if (plugin == null) {
            return null;
        }
        PluginDescriptor descriptor = plugin.getDescriptor();
        if (descriptor == null) {
            return null;
        }
        return descriptor.getResources();
    }

    private static Map<String, Extension>
            getExt(ExtensionPoint extensionPoint, String pluginId) {
        if (extensionPoint == null) {
            Log.d(TAG, "<getExt> extensionPoint is null!");
            return new HashMap<String, Extension>();
        }
        HashMap<String, Extension> connectedExtentions =
                extensionPoint.getConnectedExtensions();
        if (connectedExtentions == null || connectedExtentions.isEmpty()) {
            Log.d(TAG, "<getExt> connectedExtentions is null or connectedExtentions is empty!");
            return new HashMap<String, Extension>();
        }
        HashMap<String, Extension> extentions = new HashMap<String, Extension>();
        for (HashMap.Entry<String, Extension> connectedExtention : connectedExtentions
                .entrySet()) {
            Extension extension = connectedExtention.getValue();
            if (extension != null && (pluginId.equals(extension.pluginId))) {
                extentions.put(connectedExtention.getKey(), connectedExtention.getValue());
            }
        }
        return extentions;
    }
}
