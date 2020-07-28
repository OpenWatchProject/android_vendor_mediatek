package com.mediatek.plugin.element;

import com.mediatek.plugin.utils.Log;

import java.util.HashMap;
import java.util.Map.Entry;

/**
 * All sub class must inherit Element class, which correspond for plugin xml tag.
 */
public class Element {
    private static final String TAG = "PluginManager/Element";

    public String id;

    protected HashMap<Class<? extends Element>, HashMap<String, Element>> mChilds =
            new HashMap<Class<? extends Element>, HashMap<String, Element>>();

    protected Element mParent = null;

    /**
     * After has finish init the child object, save the child object in HashMap.
     * @param child
     *            the child of this Element.
     */
    public void addChild(Element child) {
        if (child == null) {
            return;
        }
        HashMap<String, Element> value = mChilds.get(child.getClass());
        if (value == null) {
            value = new HashMap<String, Element>();
            mChilds.put(child.getClass(), value);
        }
        value.put(child.id, child);
        Log.d(TAG, "<addChild> this = " + this + ", child class = " + child.getClass()
                + ", child id = " + child.id);
    }

    public void setParent(Element parent) {
        mParent = parent;
    }

    public Element getParent() {
        return mParent;
    }

    /**
     * Print the info of element and its children.
     */
    public final void printf() {
        printf("", true);
    }

    private void printf(String prefix, boolean rootPrint) {
        if (rootPrint) {
            Log.d(TAG, prefix + "class" + " = " + toString());
        } else {
            printKeyValue(prefix, "child class", toString());
            prefix += "|    ";
        }
        printAllKeyValue(prefix);
        for (Entry<Class<? extends Element>, HashMap<String, Element>> entry1 : mChilds
                .entrySet()) {
            for (Entry<String, Element> entry2 : entry1.getValue().entrySet()) {
                entry2.getValue().printf(prefix, false);
            }
        }
    }

    protected final void printKeyValue(String prefix, String key, String value) {
        Log.d(TAG, prefix + "|");
        Log.d(TAG, prefix + "+----" + key + " = " + value);
    }

    protected final void printValue(String prefix, String value) {
        Log.d(TAG, prefix + "|");
        Log.d(TAG, prefix + "+----" + value);
    }

    protected void printAllKeyValue(String prefix) {
        printKeyValue(prefix, "id", id);
    }
}
