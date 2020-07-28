package com.mediatek.engineermode.siminfo;

public class SimInfoData {
    String num;
    String name;
    int EF_id;
    String[] EF_Path;
    int Family;
    String type;
    boolean need_to_check;

    public SimInfoData() {
    }

    public SimInfoData(String num, String name, int EF_id, String[] EF_Path, int Family,
            String type, boolean need_to_check) {
        this.num = num;
        this.name = name;
        this.EF_id = EF_id;
        this.EF_Path = EF_Path;
        this.Family = Family;
        this.type = type;
        this.need_to_check = need_to_check;
    }
}