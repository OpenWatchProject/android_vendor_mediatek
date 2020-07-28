package com.mediatek.engineermode.cfu;

public class CfuSimInfoData {
    String name;
    int EF_id;
    String[] EF_Path;
    int Family;
    String type;

    public CfuSimInfoData(String name, int EF_id, String[] EF_Path, int Family, String type) {
        this.name = name;
        this.EF_id = EF_id;
        this.EF_Path = EF_Path;
        this.Family = Family;
        this.type = type;
    }
}