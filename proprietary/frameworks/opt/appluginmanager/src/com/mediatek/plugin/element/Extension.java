package com.mediatek.plugin.element;

import android.graphics.drawable.Drawable;

import com.mediatek.plugin.element.ParameterDef.ParameterType;
import com.mediatek.plugin.utils.Log;

import java.util.HashMap;

/**
 * Extension class for extension tag.
 */
public class Extension extends Element {
    private static final String TAG = "PluginManager/Extension";

    // Public attribute comes from xml
    public String pluginId;
    public String extensionPointId;
    public String name;
    public Drawable drawable;
    public String className;

    // Attribute about relationship
    private ExtensionPoint mExtensionPoint;

    public void setExtensionPoint(ExtensionPoint extensionPoint) {
        mExtensionPoint = extensionPoint;
    }

    /**
     * Get Parameter object, that has the key value .
     * @param key
     *            the value of Parameter.
     * @return Parameter object.
     */
    public Element getParameter(String key) {
        HashMap<String, Element> parameters = mChilds.get(Parameter.class);
        return parameters.get(key);
    }

    /**
     * Get the int value of the parameter, that has the key.
     * @param key
     *            The parameter key.
     * @param defaultValue
     *            return the defaultValue, while can not find the key value.
     * @return The value of the key.
     */
    public int getInt(String key, int defaultValue) {
        ParameterType type = mExtensionPoint.getParameterType(key);
        if (type == ParameterType.INT) {
            Parameter parameter = (Parameter) getParameter(key);
            if (parameter != null && parameter.value != null) {
                try {
                    return Integer.valueOf(parameter.value);
                } catch (NumberFormatException e) {
                    Log.e(TAG, "<getInt>", e);
                }
            }
            return defaultValue;
        } else {
            throw new RuntimeException("Please Check thes Type !");
        }
    }

    /**
     * Get the double value of the parameter, that has the key.
     * @param key
     *            The parameter key.
     * @param defaultValue
     *            return the defaultValue, while can not find the key value.
     * @return The value of the key.
     */
    public double getDouble(String key, double defaultValue) {
        ParameterType type = mExtensionPoint.getParameterType(key);
        if (type == ParameterType.DOUBLE) {
            Parameter parameter = (Parameter) getParameter(key);
            if (parameter != null && parameter.value != null) {
                try {
                    return Double.valueOf(parameter.value);
                } catch (NumberFormatException e) {
                    Log.e(TAG, "<getDouble>", e);

                }
            }
            return defaultValue;
        } else {
            throw new RuntimeException("Please Check the Type !");
        }
    }

    /**
     * Get the float value of the parameter, that has the key.
     * @param key
     *            The parameter key.
     * @param defaultValue
     *            return the defaultValue, while can not find the key value.
     * @return The value of the key.
     */
    public float getFloat(String key, float defaultValue) {
        ParameterType type = mExtensionPoint.getParameterType(key);
        if (type == ParameterType.FLOAT) {
            Parameter parameter = (Parameter) getParameter(key);
            if (parameter != null && parameter.value != null) {
                try {
                    return Float.valueOf(parameter.value);
                } catch (NumberFormatException e) {
                    Log.e(TAG, "<getFloat>", e);
                }
            }
            return defaultValue;
        } else {
            throw new RuntimeException("Please Check the Type !");
        }
    }

    /**
     * Get the float String of the parameter, that has the key.
     * @param key
     *            The parameter key.
     * @param defaultValue
     *            return the defaultValue, while can not find the key value.
     * @return The value of the key.
     */
    public String getString(String key, String defaultValue) {
        ParameterType type = mExtensionPoint.getParameterType(key);
        if (type == ParameterType.STRING) {
            Parameter parameter = (Parameter) getParameter(key);
            if (parameter != null && parameter.value != null) {
                return parameter.value;
            } else {
                return defaultValue;
            }
        } else {
            throw new RuntimeException("Please Check the Type !");
        }
    }

    /**
     * Get the float boolean of the parameter, that has the key.
     * @param key
     *            The parameter key.
     * @param defaultValue
     *            return the defaultValue, while can not find the key value.
     * @return The value of the key.
     */
    public boolean getBoolean(String key, boolean defaultValue) {
        ParameterType type = mExtensionPoint.getParameterType(key);
        if (type == ParameterType.BOOLEAN) {
            Parameter parameter = (Parameter) getParameter(key);
            if (parameter != null && parameter.value != null) {
                return Boolean.parseBoolean(parameter.value);
            } else {
                return defaultValue;
            }
        } else {
            throw new RuntimeException("Please Check the Type !");
        }
    }

    /**
     * Get the float long of the parameter, that has the key.
     * @param key
     *            The parameter key.
     * @param defaultValue
     *            return the defaultValue, while can not find the key value.
     * @return The value of the key.
     */
    public long getLong(String key, long defaultValue) {
        ParameterType type = mExtensionPoint.getParameterType(key);
        if (type == ParameterType.LONG) {
            Parameter parameter = (Parameter) getParameter(key);
            if (parameter != null && parameter.value != null) {
                try {
                    return Long.valueOf(parameter.value);
                } catch (NumberFormatException e) {
                    Log.e(TAG, "<getLong>", e);
                }
            }
            return defaultValue;
        } else {
            throw new RuntimeException("Please Check the Type !");
        }
    }

    @Override
    public void printAllKeyValue(String prefix) {
        super.printAllKeyValue(prefix);
        printKeyValue(prefix, "name", String.valueOf(name));
        printKeyValue(prefix, "pluginId", String.valueOf(pluginId));
        printKeyValue(prefix, "extensionPointId", String.valueOf(extensionPointId));
        printKeyValue(prefix, "drawable", String.valueOf(drawable));
        printKeyValue(prefix, "className", String.valueOf(className));
    }
}
