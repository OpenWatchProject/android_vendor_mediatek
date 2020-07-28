package com.mediatek.emcamera;

public enum AeMode_e {
    Auto(0, "auto"), Bracket(1, "bracket");
    private final int id;
    private final String desc;

    AeMode_e(int id, String desc) {
        this.id = id;
        this.desc = desc;
    }

    public int getId() {
        return id;
    }

    public String getDesc() {
        return desc;
    }
}
