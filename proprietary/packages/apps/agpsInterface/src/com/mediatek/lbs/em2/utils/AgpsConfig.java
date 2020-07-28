package com.mediatek.lbs.em2.utils;

import java.util.ArrayList;

public class AgpsConfig {

    public ArrayList<SuplProfile> suplProfiles;
    public SuplProfile curSuplProfile;
    public CdmaProfile cdmaProfile;
    public AgpsSetting agpsSetting;
    public CpSetting cpSetting;
    public UpSetting upSetting;
    public GnssSetting gnssSetting;

    public AgpsConfig() {
        suplProfiles = new ArrayList<SuplProfile>();
        curSuplProfile = new SuplProfile();
        cdmaProfile = new CdmaProfile();
        agpsSetting = new AgpsSetting();
        cpSetting = new CpSetting();
        upSetting = new UpSetting();
        gnssSetting = new GnssSetting();
    }

    public ArrayList<SuplProfile> getSuplProfiles() {
        return suplProfiles;
    }

    public SuplProfile getCurSuplProfile() {
        return curSuplProfile;
    }

    public CdmaProfile getCdmaProfile() {
        return cdmaProfile;
    }

    public AgpsSetting getAgpsSetting() {
        return agpsSetting;
    }

    public CpSetting getCpSetting() {
        return cpSetting;
    }

    public UpSetting getUpSetting() {
        return upSetting;
    }

    public GnssSetting getGnssSetting() {
        return gnssSetting;
    }

    public String toString() {
        String ret = "";
        ret += "### SuplProfiles ###\n";
        for (SuplProfile p : suplProfiles) {
            ret += p + "\n";
        }
        ret += "### SuplProfile ###\n";
        ret += curSuplProfile + "\n";
        ret += "### CdmaProfile ###\n";
        ret += cdmaProfile + "\n";
        ret += "### AgpsSetting ###\n";
        ret += agpsSetting + "\n";
        ret += "### CpSetting ###\n";
        ret += cpSetting + "\n";
        ret += "### UpSetting ###\n";
        ret += upSetting + "\n";
        ret += "### GnssSetting ###\n";
        ret += gnssSetting + "\n";
        return ret;
    }
}
