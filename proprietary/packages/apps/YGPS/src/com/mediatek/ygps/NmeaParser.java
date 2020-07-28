package com.mediatek.ygps;

import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;


/**
 * Parser class for parsing NMEA sentences.
 */
public class NmeaParser {
    private static final String TAG = "YGPS/NmeaParser";
    private static final String[] TALKER = {"$GP", "$GL", "$GA", "$BD"};
    private static final int[] TALKER_COLOR = {
        0xff00ffff, //cyan
        0xffffff00, //yellow
        0xffffffff, //white
        0xff0000ff //blue
        };
    private static final int TALKER_TYPE_NUM = 4;
    private static final int COLOR_RED = 0xffff0000; //red

    private static final String TALKER_NOT_SUPPORT = "none";
    private static NmeaParser sNmeaParser = null;
    private boolean mLogHidden = false;
    private int mSatelliteCount;
    private HashMap<String, ArrayList> mSatelInfoList;
    private HashMap<String, SVExtraInfro> mExtraInfoList;
    private String mCurrentTalker = TALKER_NOT_SUPPORT;
    private final ArrayList<NmeaUpdateViewListener> mListener =
            new ArrayList<NmeaUpdateViewListener>();
    private HashMap<String, ArrayList<Integer>> mUsedFixIdMapList =
            new HashMap<String, ArrayList<Integer>>();
    private int mParseState = STATE_UNINIT;
    private static final int STATE_UNINIT = -1;
    private static final int STATE_PARSE_GSA = 2;
    private static final int STATE_PARSE_GSV = 3;
    private static final int STATE_PARSE_OTHERS = 5;

    /**
     * Get single instance of NmeaParser.
     * @return A instance of the NmeaParser
     */
    static NmeaParser getNMEAParser() {
        if (sNmeaParser == null) {
            sNmeaParser = new NmeaParser();
        }
        return sNmeaParser;
    }

    private boolean isUsedInFix(int prn) {
        ArrayList<Integer> usedFixIdList = mUsedFixIdMapList.get(mCurrentTalker);
        if (usedFixIdList == null) {
            usedFixIdList = new ArrayList<Integer>();
            mUsedFixIdMapList.put(mCurrentTalker, usedFixIdList);
            return false;
        }
        for (Integer id : usedFixIdList) {
            if (prn == id) {
                return true;
            }
        }
        return false;
    }

    private void addUsedFixId(int prn) {
        ArrayList<Integer> usedFixIdList = mUsedFixIdMapList.get(mCurrentTalker);
        if (usedFixIdList == null) {
            usedFixIdList = new ArrayList<Integer>();
            mUsedFixIdMapList.put(mCurrentTalker, usedFixIdList);
        }
        usedFixIdList.add(prn);
    }

    private void clearUsedFixList() {
        ArrayList<Integer> usedFixIdList = mUsedFixIdMapList.get(mCurrentTalker);
        if (usedFixIdList == null) {
            usedFixIdList = new ArrayList<Integer>();
            mUsedFixIdMapList.put(mCurrentTalker, usedFixIdList);
            return;
        }
        usedFixIdList.clear();
    }

    private NmeaParser() {
        this.mSatelInfoList = new HashMap<String, ArrayList>();
        this.mExtraInfoList = new HashMap<String, SVExtraInfro>();
        this.mSatelliteCount = 0;
        mLogHidden = GpsMnlSetting.isLogHidden();

        for (String s :TALKER) {
            ArrayList<SatelliteInfo> mSatlist = new ArrayList<SatelliteInfo>();
            mSatelInfoList.put(s, mSatlist);

            SVExtraInfro mextra = new SVExtraInfro();
            mExtraInfoList.put(s, mextra);
        }
    }

    /**
     * Get satellite count.
     * @return Total number of the satellites
     */
    synchronized int getSatelliteCount() {
        return mSatelliteCount;
    }

    /**
     * Get Current All SV information.
     * @return A list of all satellites
     */
    ArrayList<SatelliteInfo> getSatelliteList() {
        ArrayList<SatelliteInfo> mList = new ArrayList<SatelliteInfo>();
        for (ArrayList<SatelliteInfo> svlist : mSatelInfoList.values()) {
            mList.addAll(svlist);
        }
        return mList;
    }

    /**
     * Clear All SV information.
     */
    void clearSatelliteList() {
        for (ArrayList<SatelliteInfo> svlist : mSatelInfoList.values()) {
            svlist.clear();
        }
    }

    private ArrayList<SatelliteInfo> getSatelliteList(String talker) {
        ArrayList<SatelliteInfo> svlist = null;
        if (mSatelInfoList.containsKey(talker)) {
            svlist = mSatelInfoList.get(talker);
        } else {
            log("No this talker " + talker + " SV exist");
        }
        return svlist;
    }

    private void clearSatelliteList(String talker) {
        ArrayList<SatelliteInfo> svlist = getSatelliteList(talker);
        if (svlist != null) {
            for (SatelliteInfo sv : svlist) {
                sv.mUsedInFix = false;
            }
        }
    }

    private SVExtraInfro getTalkerExtra(String talker) {
        SVExtraInfro xtra = null;
        if (mExtraInfoList.containsKey(talker)) {
            xtra = mExtraInfoList.get(talker);
        } else {
            log("No this talker " + talker + " Extra exist");
        }
        return xtra;
    }

    private String checkTalker(String record) {
        String result = TALKER_NOT_SUPPORT;
        for (String s :TALKER) {
            if (record.contains(s)) {
                result = s;
                break;
            }
        }
        return result;
    }


    private int checkTalkerColor(String record) {
        for (int k = 0; k < TALKER_TYPE_NUM; k++) {
            if (TALKER[k].equals(record)) {
                return TALKER_COLOR[k];
            }
        }

        return COLOR_RED;
    }

    private String removeFirstZero(String record) {
        int ind = 0;
        int size = record.length();
        for (; ind < size ; ind++) {
            if (record.charAt(ind) != '0') {
                break;
            }
        }
        return (ind < size) ? record.substring(ind) : "0";
    }

    /**
     * Parse NMEA logs.
     * @param record The NMEA log sentence to parse
     */
    synchronized void parse(String record) {
        mCurrentTalker = checkTalker(record);
        /*
         * only parse all GSV sentences, then notify user
         */
        if (mParseState == STATE_PARSE_GSV && !record.contains("GSV")) {
//            sendMessage(SV_UPDATE);
            reportSVupdate();
        }
        if (record.contains("GSA")) {
            mParseState = STATE_PARSE_GSA;
            try {
                parseGSA(record);
            } catch (NumberFormatException e) {
                log("Exception in parseGSA()" + e);
            }
        } else if (record.contains("GSV")) {
            mParseState = STATE_PARSE_GSV;
            try {
                parseGSV(record);
            } catch (NumberFormatException e) {
                log("Exception in parseGSV()" + e);
            }
        } else {
            mParseState = STATE_PARSE_OTHERS;
            //log("undefined format");
        }

    }

    /**
     *
     * Spec please refer http://www.gpsinformation.org/dale/nmea.htm.
     *
     * @param record
     */
    private synchronized void parseGSA(String record) {
        seclog("parseGSA:" + record);
        String[] values = split(record);
        //String mode=values[1];
        SVExtraInfro mInfo = getTalkerExtra(mCurrentTalker);
        clearUsedFixList();
        if (mInfo != null && values.length >= 17) {
            if (values[2].equals("1")) {
                //no fix
        clearSatelliteList(mCurrentTalker);
                return;
            }
            mInfo.mFixType = values[2];
            getSatelliteList(mCurrentTalker);
            clearSatelliteList(mCurrentTalker);
            for (int i = 2; i < 15; i++) {
                int prn = parseInt(values[i]);
                if (prn > 0) {
                    addUsedFixId(prn);
                }
            }

            mInfo.mPdop = values[15];
            mInfo.mHdop = values[16];
            mInfo.mVdop = values[17];
        } else {
            // no fix
            clearSatelliteList(mCurrentTalker);
        }
    }


    /**
     *
     * Spec please refer http://www.gpsinformation.org/dale/nmea.htm.
     *
     * @param record
     *
     */
    private synchronized void parseGSV(String record) {
        seclog("parseGSV:" + record);
        String[] values = split(record);
        ArrayList<SatelliteInfo> svList = getSatelliteList(mCurrentTalker);

        if (svList == null) {
            log("parseGSV get SVlist Error" + svList + " Current Talker:" + mCurrentTalker);
            return;
        }

        int mTotalNum = parseInt(values[1]);
        int mMsgInd = parseInt(values[2]);

        if (mTotalNum > 0 && mMsgInd == 1) {
            //clear all SV record
            svList.clear();
            mSatelInfoList.put(mCurrentTalker, svList);
        }

        int index = 4;
        while (index + 3 < values.length) {
            int satelliteNumber = parseInt(values[index++]);
            float elevation = parseFloat(values[index++]);
            float azimuth = parseFloat(values[index++]);
            float satelliteSnr = 0;
            if (values[index].contains("*")) {
                String[] mStrl = values[index].split("\\*");
                satelliteSnr = parseFloat(mStrl[0]);
                index++;
            } else {
                satelliteSnr = parseFloat(values[index++]);
            }

            if (satelliteNumber > 0) {
                SatelliteInfo sat = new SatelliteInfo(satelliteNumber,
                        checkTalkerColor(mCurrentTalker));
                sat.mSnr = satelliteSnr;
                sat.mElevation = elevation;
                sat.mAzimuth = azimuth;
                if (isUsedInFix(satelliteNumber)) {
                    sat.mUsedInFix = true;
                }
                svList.add(sat);
            }
        }

        if (values[1].equals(values[2])) {
            //report location update
            log("mSatelInfoList add svlist : " + mCurrentTalker + " size:" + svList.size());
            mSatelInfoList.put(mCurrentTalker, svList);
        }
    }

    private void log(String msg) {
        Log.i(TAG, msg);
    }

    private void seclog(String msg) {
        if (!mLogHidden) {
            Log.i(TAG, msg);
        }
    }


    private String[] split(String str) {
        String[] result = null;
        String delims = "[,]";
        result = str.split(delims);
        return result;
    }



    private float parseFloat(String str) {
        float d = 0;
        if (str.equals("")) {
            return d;
        }
        String mStr = removeFirstZero(str);
        try {
            d = Float.parseFloat(mStr);
        } catch (NumberFormatException e) {
            Log.e(TAG, "parseFloat:" + e);
        }
        return d;
    }


    private double parseDouble(String str) {
        double d = 0;
        if (str.equals("")) {
            return d;
        }
        String mStr = removeFirstZero(str);
        try {
            d = Double.parseDouble(mStr);
        } catch (NumberFormatException e) {
            Log.e(TAG, "parseDouble:" + e);
        }
        return d;
    }

    private int parseInt(String str) {
        int d = 0;
        if (str.equals("")) {
            return d;
        }
        String mStr = removeFirstZero(str);
        try {
            d = Integer.valueOf(mStr);
        } catch (NumberFormatException  e) {
            Log.e(TAG, "parseDouble:" + e);
        }
        return d;
    }

    private void reportSVupdate() {
        synchronized (mListener) {
            int size = mListener.size();
            for (int i = 0; i < size; i++) {
                NmeaUpdateViewListener listener = mListener.get(i);
                listener.onViewupdateNotify();
            }
        }
    }

    /**
     * Add listener to update satellites information.
     * @param l The listener to add to the list
     */
    void addSVUpdateListener(NmeaUpdateViewListener l) {
        synchronized (mListener) {
            mListener.add(l);
        }
    }

    /**
     * Remove listener to update satellites information.
     * @param l The listener to remove from list
     */
    void removeSVUpdateListener(NmeaUpdateViewListener l) {
        synchronized (mListener) {
            mListener.remove(l);
        }
    }


    /**
     * Interface for satellites information update notification.
     *
     */
    public interface NmeaUpdateViewListener {
        /**
         * Callback for satellites information update notification.
         */
        void onViewupdateNotify();
    }

    /**
     * Class to hold data for satellites extra information.
     *
     */
    private class SVExtraInfro {

        String mPdop = "";
        String mHdop = "";
        String mVdop = "";
        String mFixType = "";

        private SVExtraInfro() {
        }
    }


    /**
     * Class to hold data for satellites basic information.
     *
     */
    class SatelliteInfo {

        int mPrn;
        float mSnr = 0;
        float mElevation = 0;
        float mAzimuth = 0;
        boolean mUsedInFix = false;
        int mColor = 0xffffffff; // white default

        SatelliteInfo(int prn, int color) {
            mPrn = prn;
            mColor = color;
        }

        public String toString() {
            StringBuilder builder = new StringBuilder();
            builder.append("[").append(mPrn).append(", ");
            builder.append(mSnr).append(", ");
            builder.append(mElevation).append(", ");
            builder.append(mAzimuth).append(", ");
            builder.append(mUsedInFix).append("]");
            return builder.toString();
        }
    }
}
