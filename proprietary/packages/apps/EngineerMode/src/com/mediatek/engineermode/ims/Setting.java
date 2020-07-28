package com.mediatek.engineermode.ims;

import java.io.*;
import java.util.ArrayList;

public class Setting implements Serializable {
    public String label;
    public int type = 0;
    public ArrayList<String> entries = new ArrayList<String>();
    public ArrayList<Integer> values = new ArrayList<Integer>();
    public String defaultValue;
    public String suffix = "";

    public void setLabel(String label) {
        this.label = label;
    }

    public String getLabel() {
        return label;
    }

    public void setType(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }

    public void setEntries(ArrayList<String> entries) {
        this.entries = entries;
    }

    public ArrayList<String> getEntries() {
        return entries;
    }

    public void setValues(ArrayList<Integer> values) {
        this.values = values;
    }

    public ArrayList<Integer> getValues() {
        return values;
    }

    public void setDefaultValue(String defaultValue) {
        this.defaultValue = defaultValue;
    }

    public String getDefaultValue() {
        return defaultValue;
    }

    public void setSuffix(String suffix) {
        this.suffix = suffix;
    }

    public String getSuffix() {
        return suffix;
    }
}