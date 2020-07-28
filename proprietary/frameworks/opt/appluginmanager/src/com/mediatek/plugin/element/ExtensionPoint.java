package com.mediatek.plugin.element;

import com.mediatek.plugin.element.ParameterDef.ParameterType;

import java.util.HashMap;
import java.util.Map.Entry;

/**
 * ExtensionPoint class represent for extension-point tag.
 */
public class ExtensionPoint extends Element {
    // Public attribute comes from xml
    public String className;

    // Attribute about relationship
    private HashMap<String, Extension> mConnectedExtensions;

    public HashMap<String, Extension> getConnectedExtensions() {
        return mConnectedExtensions;
    }

    /**
     * Set the extensions that extend current extension point.
     * @param connectedExtensions
     *            The collection of extension
     * @return
     */
    public void setConnectedExtensions(HashMap<String, Extension> connectedExtensions) {
        mConnectedExtensions = connectedExtensions;
    }

    /**
     * Get the type value of ParameterDef object that has the key.
     * @param key
     *            The key value of ParameterDef object.
     * @return the type value.
     */
    public ParameterType getParameterType(String key) {
        HashMap<String, Element> value = mChilds.get(ParameterDef.class);
        if (value != null) {
            ParameterDef def = (ParameterDef) value.get(key);
            return def.type;
        }
        return null;
    }

    @Override
    public void printAllKeyValue(String prefix) {
        super.printAllKeyValue(prefix);
        printKeyValue(prefix, "className", String.valueOf(className));
        if (mConnectedExtensions != null) {
            printKeyValue(prefix, "mConnectedExtensions size ", String
                    .valueOf(mConnectedExtensions.size()));
            for (Entry<String, Extension> entry : mConnectedExtensions.entrySet()) {
                prefix += "|    ";
                printValue(prefix, "id = " + entry.getKey() + " extension = "
                        + entry.getValue().toString());
            }
        } else {
            printKeyValue(prefix, "mConnectedExtensions size ", String.valueOf(0));
        }
    }

}
