package com.mediatek.plugin.element;

/**
 * Parameter class represent for parameter tag.
 */
public class Parameter extends Element {
    // Public attribute comes from xml
    public String value;

    @Override
    public void printAllKeyValue(String prefix) {
        super.printAllKeyValue(prefix);
        printKeyValue(prefix, "value", String.valueOf(value));
    }
}
