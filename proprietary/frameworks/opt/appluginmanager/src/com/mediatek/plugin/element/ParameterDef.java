package com.mediatek.plugin.element;

/**
 * ParameterDef class represent for parameter-def tag.
 */
public class ParameterDef extends Element {
    /**
     * Parameter value type.
     */
    public static enum ParameterType {
        STRING, INT, LONG, FLOAT, DOUBLE, BOOLEAN
    }

    // Public attribute comes from xml
    public ParameterType type;

    @Override
    public void printAllKeyValue(String prefix) {
        super.printAllKeyValue(prefix);
        printKeyValue(prefix, "type", String.valueOf(type));
    }

}
