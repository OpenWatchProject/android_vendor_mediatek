package com.mediatek.gnssdebugreport;

import android.os.Bundle;
import android.util.Log;

import com.mediatek.gnssdebugreport.DebugDataReport;
import com.mediatek.gnssdebugreport.DebugDataReport.DebugData840;
import com.mediatek.gnssdebugreport.DebugDataReport.DebugData841;

import org.json.JSONException;
import org.json.JSONObject;

public abstract class DebugDataParser {

    protected static final String DEBUG_DATA_SPLIT = ",";
    protected static final String JSON_TYPE = "type";
    abstract Bundle parseData(String data);
}

class DebugData839Parser extends DebugDataParser {
    private static final int DEBUG_DATA_TYPE = 839;
    private static final int DEBUG_DATA_NUMBER = 19;

    private static final String TAG = "GnssDebug/Data839Parser";

    @Override
    Bundle parseData(String data) {
        // TODO Auto-generated method stub
        //Parse data
        String[] debugData = data.split(DEBUG_DATA_SPLIT);
        if ((debugData == null) || (debugData.length != DEBUG_DATA_NUMBER)) {
            return null;
        }
        double CB = Double.valueOf(debugData[0]).doubleValue();
        double CompCB = Double.valueOf(debugData[1]).doubleValue();
        double ClkTemp = Double.valueOf(debugData[2]).doubleValue();
        int Saturation = Integer.valueOf(debugData[3]).intValue();
        int Pga = Integer.valueOf(debugData[4]).intValue();
        long Ttff = Long.valueOf(debugData[5]).longValue();
        int Svnum = Integer.valueOf(debugData[6]).intValue();
        long TT4SV = Long.valueOf(debugData[7]).longValue();
        float Top4CNR = Float.valueOf(debugData[8]).floatValue();
        double InitLlhLongi = Double.valueOf(debugData[9]).doubleValue();
        double InitLlhLati = Double.valueOf(debugData[10]).doubleValue();
        double InitLlhHeight = Double.valueOf(debugData[11]).doubleValue();
        int InitSrc = Integer.valueOf(debugData[12]).intValue();
        float InitPacc = Float.valueOf(debugData[13]).floatValue();
        int HaveEPO = Integer.valueOf(debugData[14]).intValue();
        int EPOage = Integer.valueOf(debugData[15]).intValue();
        float SensorHACC = Float.valueOf(debugData[16]).floatValue();
        int MPEvalid = Integer.valueOf(debugData[17]).intValue();
        int Lsvalid = Integer.valueOf(debugData[18]).intValue();
        DebugDataReport debugDataReport = new DebugDataReport(CB, CompCB, ClkTemp, Saturation, Pga, Ttff,
                Svnum, TT4SV, Top4CNR, InitLlhLongi, InitLlhLati, InitLlhHeight, InitSrc,
                InitPacc, HaveEPO, EPOage, SensorHACC, MPEvalid, Lsvalid);
        Log.i(TAG, "debugDataReport:" + debugDataReport.toString());
        Bundle bundle = new Bundle();
        bundle.putParcelable(DebugDataReport.DATA_KEY, debugDataReport);
        return bundle;
    }
    
}

class DebugData840Parser extends DebugDataParser {
    private static final int DEBUG_DATA_TYPE = 840;
    private static final int DEBUG_DATA_NUMBER = 11;
    private static final String TAG = "GnssDebug/Data840Parser";

    @Override
    Bundle parseData(String data) {
        // TODO Auto-generated method stub
        //Parse data
        String[] debugData = data.split(DEBUG_DATA_SPLIT);
        if ((debugData == null) || (debugData.length != DEBUG_DATA_NUMBER)) {
            return null;
        }

        double version = Double.valueOf(debugData[0]).doubleValue();
        long supl = Long.valueOf(debugData[1]).longValue();
        long epo = Long.valueOf(debugData[2]).longValue();
        int epoAge = Integer.valueOf(debugData[3]).intValue();
        int qepo = Integer.valueOf(debugData[4]).intValue();
        int nlp = Integer.valueOf(debugData[5]).intValue();
        double aidLat = Double.valueOf(debugData[6]).doubleValue();
        double aidLong = Double.valueOf(debugData[7]).doubleValue();
        double aidHeight = Double.valueOf(debugData[8]).doubleValue();
        int nv = Integer.valueOf(debugData[9]).intValue();
        long summary = Long.valueOf(debugData[10]).longValue();
        JSONObject json = new JSONObject();
        try {
            json.put(JSON_TYPE, DEBUG_DATA_TYPE);
            json.put(DebugData840.KEY_VER, version);
            json.put(DebugData840.KEY_SUPL_INJECT, supl);
            json.put(DebugData840.KEY_EPO, epo);
            json.put(DebugData840.KEY_EPO_AGE, epoAge);
            json.put(DebugData840.KEY_QEPO, qepo);
            json.put(DebugData840.KEY_NLP, nlp);
            json.put(DebugData840.KEY_AID_LAT, aidLat);
            json.put(DebugData840.KEY_AID_LON, aidLong);
            json.put(DebugData840.KEY_AID_HEIGHT, aidHeight);
            json.put(DebugData840.KEY_NV, nv);
            json.put(DebugData840.KEY_AID_SUMMARY, summary);
        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }
        Bundle bundle = new Bundle();
        bundle.putString(DebugDataReport.DATA_KEY_TYPE1, json.toString());
        return bundle;
    }
    
}

class DebugData841Parser extends DebugDataParser {

    private static final int DEBUG_DATA_TYPE = 841;
    private static final int DEBUG_DATA_NUMBER = 10;
    private static final String TAG = "GnssDebug/Data841Parser";

    @Override
    Bundle parseData(String data) {
        // TODO Auto-generated method stub
        //Parse data
        String[] debugData = data.split(DEBUG_DATA_SPLIT);
        if ((debugData == null) || (debugData.length < DEBUG_DATA_NUMBER)) {
            return null;
        }

        double version = Double.valueOf(debugData[0]).doubleValue();
        double clkD = Double.valueOf(debugData[1]).doubleValue();
        double temper = Double.valueOf(debugData[2]).doubleValue();
        int pgaGain = Integer.valueOf(debugData[3]).intValue();
        int noiseFloor = Integer.valueOf(debugData[4]).intValue();
        int digiI = Integer.valueOf(debugData[5]).intValue();
        int digiQ = Integer.valueOf(debugData[6]).intValue();
        long sensor = Long.valueOf(debugData[7]).longValue();
        long summary = Long.valueOf(debugData[8]).longValue();
        long blanking = Long.valueOf(debugData[9]).longValue();

        JSONObject json = new JSONObject();
        try {
            json.put(JSON_TYPE, DEBUG_DATA_TYPE);
            json.put(DebugData841.KEY_VER, version);
            json.put(DebugData841.KEY_CLKD, clkD);
            json.put(DebugData841.KEY_XO_TEMPER, temper);
            json.put(DebugData841.KEY_PGA_GAIN, pgaGain);
            json.put(DebugData841.KEY_NOISE_FLOOR, noiseFloor);
            json.put(DebugData841.KEY_DIGI_I, digiI);
            json.put(DebugData841.KEY_DIGI_Q, digiQ);
            json.put(DebugData841.KEY_SENSOR, sensor);
            json.put(DebugData841.KEY_CHIP_SUMMARY, summary);
            json.put(DebugData841.KEY_BLANKING, blanking);
        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }
        Bundle bundle = new Bundle();
        bundle.putString(DebugDataReport.DATA_KEY_TYPE1, json.toString());
        return bundle;
    }
    

}