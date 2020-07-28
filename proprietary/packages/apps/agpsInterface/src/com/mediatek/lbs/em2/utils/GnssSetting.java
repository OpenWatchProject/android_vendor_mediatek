package com.mediatek.lbs.em2.utils;

public class GnssSetting {
    
    public boolean sib8sib16Enable;
    
    public boolean gpsSatelliteEnable;
    public boolean glonassSatelliteEnable;
    public boolean beidouSatelliteEnable;
    public boolean galileoSatelliteEnable;
    public boolean aGpsSatelliteEnable;
    public boolean aGlonassSatelliteEnable;
    public boolean aBeidouSatelliteEnable;
    public boolean aGalileoSatelliteEnable;
    
    public boolean gpsSatelliteSupport;
    public boolean glonassSatelliteSupport;
    public boolean beidousSatelliteSupport;
    public boolean galileoSatelliteSupport;
    public boolean lppeSupport;

    public GnssSetting() {
        sib8sib16Enable = true;
        gpsSatelliteEnable = true;
        glonassSatelliteEnable = true;
        beidouSatelliteEnable = true;
        galileoSatelliteEnable = true;
        aGpsSatelliteEnable = true;
        aGlonassSatelliteEnable = true;
        aBeidouSatelliteEnable = false;
        aGalileoSatelliteEnable = false;
        gpsSatelliteSupport = true;
        glonassSatelliteSupport = true;
        beidousSatelliteSupport = true;
        galileoSatelliteSupport = true;
        lppeSupport = false;
    }
    
    public String toString() {
        String ret = "";
        ret += "sib8sib16Enable=[" + sib8sib16Enable + "] ";
        ret += "gpsSatelliteEnable=[" + gpsSatelliteEnable + "] ";
        ret += "glonassSatelliteEnable=[" + glonassSatelliteEnable + "] ";
        ret += "beidouSatelliteEnable=[" + beidouSatelliteEnable + "] ";
        ret += "galileoSatelliteEnable=[" + galileoSatelliteEnable + "] ";
        ret += "aGpsSatelliteEnable=[" + aGpsSatelliteEnable + "] ";
        ret += "aGlonassSatelliteEnable=[" + aGlonassSatelliteEnable + "] ";
        ret += "aBeidouSatelliteEnable=[" + aBeidouSatelliteEnable + "] ";
        ret += "aGalileoSatelliteEnable=[" + aGalileoSatelliteEnable + "] ";
        ret += "gpsSatelliteSupport=[" + gpsSatelliteSupport + "] ";
        ret += "glonassSatelliteSupport=[" + glonassSatelliteSupport + "] ";
        ret += "beidousSatelliteSupport=[" + beidousSatelliteSupport + "] ";
        ret += "galileoSatelliteSupport=[" + galileoSatelliteSupport + "] ";
        ret += "lppeSupport=[" + lppeSupport + "] ";
        return ret;
    }
}

