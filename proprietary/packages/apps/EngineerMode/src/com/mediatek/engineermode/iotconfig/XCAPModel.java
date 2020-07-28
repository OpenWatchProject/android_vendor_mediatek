package com.mediatek.engineermode.iotconfig;

public class XCAPModel {

    private String name;
    private boolean selected = false;
    private boolean configed = false;
    private String value = "";
    private String type;

    public String getValue() {
        if(this.type.equals(IotConfigConstant.BOOLEANTYPE)){
            return this.selected ? "1" : "0";
        }
        return value;
    }

    public void setValue(String value) {
        this.value = value;
    }

    public void setName(String name) {
        this.name = name;
    }

    public XCAPModel(String name, String value, String type) {
        this.name = name;
        this.type = type;
        this.value = value;
    }

    public XCAPModel(String name, String type) {
        this.name = name;
        this.type = type;
        this.configed = false;
    }

    public XCAPModel(String name, boolean selected, String type) {
        this.name = name;
        this.type = type;
        this.selected = selected;
        this.configed = true;
    }
    public XCAPModel(String name, boolean isConfig, boolean selected, String type) {
        this.name = name;
        this.type = type;
        this.configed = isConfig;
        this.selected = selected;
    }

    public String getName() {
        return name;
    }

    public boolean isSelected() {
        return selected;
    }

    public void setSelected(boolean selected) {
        this.selected = selected;
        this.value = selected ? "1" : "0";
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getType() {
        return type;
    }

    public String toString() {
        return ("name=" + this.getName() + ";value=" + this.getValue()
                + ";type=" + this.getType());
    }

    public boolean isConfiged() {
        return configed;
    }

    public void setConfiged(boolean isConfig) {
        this.configed = isConfig;
    }

}