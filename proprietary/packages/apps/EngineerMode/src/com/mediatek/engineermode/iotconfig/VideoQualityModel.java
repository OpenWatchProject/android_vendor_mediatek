package com.mediatek.engineermode.iotconfig;

public class VideoQualityModel {

    private String name;
    private String format;
    private String profile;
    private String level;

    public VideoQualityModel(String name, String format, String profile,
            String level) {
        this.format = format;
        this.level = level;
        this.profile = profile;
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getFormat() {
        return format;
    }

    public void setFormat(String format) {
        this.format = format;
    }

    public String getLevel() {
        return level;
    }

    public void setLevel(String level) {
        this.level = level;
    }

    public String getProfile() {
        return profile;
    }

    public void setProfile(String profile) {
        this.profile = profile;
    }

}
