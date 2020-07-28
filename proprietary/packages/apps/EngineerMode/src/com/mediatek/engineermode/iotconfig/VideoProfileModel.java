package com.mediatek.engineermode.iotconfig;

public class VideoProfileModel {

    private String name;
    private String Profile;
    private String level;
    private String width;
    private String height;
    private String framerate;
    private String Iinterval;
    private String minBitRate;
    private String maxBitRate;
    private boolean isSelected;
    private String quality;

    public VideoProfileModel(String name, String profile, String level,
            String width, String height, String framerate, String iinterval,
            String minbitrate, String maxBitRate, boolean isSelected) {
        this.name = name;
        this.Profile = profile;
        this.level = level;
        this.width = width;
        this.height = height;
        this.framerate = framerate;
        this.Iinterval = iinterval;
        this.minBitRate = minbitrate;
        this.maxBitRate = maxBitRate;
        this.isSelected = isSelected;
        this.quality = "";
    }

    public VideoProfileModel(String name, String profile, String level,
            String width, String height, String framerate, String iinterval,
            String minbitrate, String maxBitRate) {
        this.name = name;
        this.Profile = profile;
        this.level = level;
        this.width = width;
        this.height = height;
        this.framerate = framerate;
        this.Iinterval = iinterval;
        this.minBitRate = minbitrate;
        this.maxBitRate = maxBitRate;
        this.isSelected = false;
        this.quality = "";
    }

    public VideoProfileModel() {
        this.isSelected = false;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getProfile() {
        return Profile;
    }

    public void setProfile(String profile) {
        Profile = profile;
    }

    public String getLevel() {
        return level;
    }

    public void setLevel(String level) {
        this.level = level;
    }

    public String getWidth() {
        return width;
    }

    public void setWidth(String width) {
        this.width = width;
    }

    public String getHeight() {
        return height;
    }

    public void setHeight(String height) {
        this.height = height;
    }

    public String getFramerate() {
        return framerate;
    }

    public void setFramerate(String framerate) {
        this.framerate = framerate;
    }

    public String getIinterval() {
        return Iinterval;
    }

    public void setIinterval(String iinterval) {
        Iinterval = iinterval;
    }

    public String getMinBitRate() {
        return minBitRate;
    }

    public void setMinBitRate(String minbitrate) {
        this.minBitRate = minbitrate;
    }

    public String getMaxBitRate() {
        return maxBitRate;
    }

    public void setMaxBitRate(String maxBitRate) {
        this.maxBitRate = maxBitRate;
    }

    public boolean isSelected() {
        return isSelected;
    }

    public void setSelected(boolean isSelected) {
        this.isSelected = isSelected;
    }

    public String getQuality() {
        // TODO Auto-generated method stub
        return this.quality;
    }
    public void setQuality(String quality) {
        // TODO Auto-generated method stub
        this.quality = quality;
    }

}
