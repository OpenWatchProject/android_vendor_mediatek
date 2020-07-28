package com.mediatek.lbs.em2.ui;

import android.location.Location;
import android.location.LocationManager;
//import android.os.Handler;
//import android.os.Message;
import android.util.Log;

import java.util.ArrayList;
import java.util.HashMap;


/**
 * Parser class for parsing NMEA sentences.
 */
public class NmeaParser {
//    private final static int SV_UPDATE        = 0;
    private static final String[] TALKER = {"$GP", "$GL", "$GA", "$BD"};
    private static final int[] TALKER_COLOR = {
        0xff00ffff, //cyan
        0xffffff00, //yellow
        0xffffffff, //white
        0xff0000ff //blue
        };
    private static final int TALKER_TYPE_NUM = 4;
    private static final int COLOR_RED = 0xffff0000;//red
    private static final boolean DEBUG = false;

    private static final String TALKER_NOT_SUPPORT = "none";
    private static NmeaParser sNmeaParser = null;
    private int mSatelliteCount;
    private HashMap<String, ArrayList> mSatelInfoList;
    private HashMap<String, SVExtraInfro> mExtraInfoList;
    private HashMap<String, Location> mLocRecord;
    private String mCurrentTalker = TALKER_NOT_SUPPORT;
    private final ArrayList<NmeaUpdateViewListener> mListener =
            new ArrayList<NmeaUpdateViewListener>();
    private HashMap<String, ArrayList<Integer>> mUsedFixIdMapList =
            new HashMap<String, ArrayList<Integer>>();
    private int mParseState = STATE_UNINIT;
    private static final int STATE_UNINIT = -1;
    private static final int STATE_PARSE_GGA = 1;
    private static final int STATE_PARSE_GSA = 2;
    private static final int STATE_PARSE_GSV = 3;
    private static final int STATE_PARSE_RMC = 4;
    private static final int STATE_PARSE_OTHERS = 5;

    /**
     * Get single instance of NmeaParser.
     * @return A instance of the NmeaParser
     */
    public static NmeaParser getNMEAParser() {
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
        this.mLocRecord = new HashMap<String, Location>();
        this.mSatelliteCount = 0;

        for (String s :TALKER) {
            ArrayList<SatelliteInfo> mSatlist = new ArrayList<SatelliteInfo>();
            mSatelInfoList.put(s, mSatlist);

            SVExtraInfro mextra = new SVExtraInfro();
            mExtraInfoList.put(s, mextra);

            Location loc = new Location(LocationManager.GPS_PROVIDER);
            mLocRecord.put(s, loc);
        }
    }

    /**
     * Get satellite count.
     * @return Total number of the satellites
     */
    public synchronized int getSatelliteCount() {
        return mSatelliteCount;
    }

    /**
     * Get Current All SV information.
     * @return A list of all satellites
     */
    public ArrayList<SatelliteInfo> getSatelliteList() {
        ArrayList<SatelliteInfo> mList = new ArrayList<SatelliteInfo>();
        for (ArrayList<SatelliteInfo> svlist : mSatelInfoList.values()) {
            mList.addAll(svlist);
        }
        return mList;
    }

    /**
     * Clear All SV infomation.
     */
    public void clearSatelliteList() {
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

    private Location getTalkerLocation(String talker) {
        Location loc = null;
        if (mLocRecord.containsKey(talker)) {
            loc = mLocRecord.get(talker);
        } else {
            log("No this talker " + talker + " Loc exist");
        }
        return loc;
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
        for (int k=0; k<TALKER_TYPE_NUM; k++) {
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

    public synchronized void parse(String record) {
        String[] values = splitLines(record);
        log("splitLines line size = " + values.length);
        for (int i = 0; i < values.length; i++) {
            parseOneSentence(values[i]);
        }
    }

    /**
     * Parse NMEA logs.
     * @param record The NMEA log sentence to parse
     */
    public synchronized void parseOneSentence(String record) {
        log("parseOneSentence: " + record);
        mCurrentTalker = checkTalker(record);
        /*
         * only parse all GSV sentences, then notify user
         */
        if (mParseState == STATE_PARSE_GSV && !record.contains("GSV")) {
//            sendMessage(SV_UPDATE);
            reportSVupdate();
        }
        if (record.contains("RMC")) {

            mParseState = STATE_PARSE_RMC;
            try {
                parseRMC(record);
            } catch (NumberFormatException e) {
                log("Exception in parseRMC()");
            }
        } else if (record.contains("GSA")) {
            mParseState = STATE_PARSE_GSA;
            try {
                parseGSA(record);
            } catch (NumberFormatException e) {
                log("Exception in parseGSA()" + e);
            }
        } else if (record.contains("GGA")) {
            mParseState = STATE_PARSE_GGA;
            try {
                parseGGA(record);
            } catch (NumberFormatException e) {
                log("Exception in parseGGA()" + e);
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

    double getLongitude(String[] values) {
        double dLongitude = 0.0;
        final String longitude = values[5];
        final String longitudeDirection = values[6];

        if (longitude.length() > 0) {
            dLongitude = parseDouble(longitude);
            if (!longitudeDirection.equals("E")) {
                dLongitude *= -1;
            }
        } else {
            log("Error with longitude");
        }
        return dLongitude;
    }

    double getLatitude(String[] values) {
        double dLatitude = 0.0;
        final String latitude = values[3];
        final String latitudeDirection = values[4];

        if (latitude.length() > 0) {
            dLatitude = parseDouble(latitude);
            if (!latitudeDirection.equals("N")) {
                dLatitude *= -1;
            }
        } else {
            log("Error with latatitude");
        }
        return dLatitude;
    }

    int getCourse(String[] values) {
        int course = 0;
        final String courseString = values[8];
        if (courseString.length() > 0) {
            try {
                course = (int) parseDouble(courseString);
            } catch (NumberFormatException e) {
                course = 180;
            }
        }
        return course;
    }

    double getSpeed(String[] values) {
        double speed = 0.0;
        final double groundSpeed = parseDouble(values[7]);
        if (groundSpeed > 0) {
            speed = ((groundSpeed) * 1.852);
        }
        if (speed < 0) {
            speed = 0;
        }
        return speed;
    }

    boolean isStatusActive(String[] values) {
        final String status = values[2];
        if ("A".equals(status)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Spec please refer http://www.gpsinformation.org/dale/nmea.htm
     *
     * @param record
     */
    private synchronized void parseRMC(String record) {
        log("parseRMC:" + record);

        String[] values = split(record);
        double dLatitude = getLatitude(values);
        double dLongitude = getLongitude(values);
        int course = getCourse(values);
        double dSpeed = getSpeed(values);

        if (isStatusActive(values)) {
            Location loc = getTalkerLocation(mCurrentTalker);
            if (loc != null) {
                loc.setLatitude(dLatitude);
                loc.setLongitude(dLongitude);
                loc.setBearing(course);
                loc.setSpeed((float) dSpeed * 1000);
                mLocRecord.put(mCurrentTalker, loc);
            }
        }
    }

    /**
     *
     * Spec please refer http://www.gpsinformation.org/dale/nmea.htm
     *
     * @param record
     */
    private synchronized void parseGSA(String record) {
        log("parseGSA:" + record);
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
            mInfo.mfixtype = values[2];
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
     * Spec please refer http://www.gpsinformation.org/dale/nmea.htm
     *
     * @param record
     */
    private synchronized void parseGGA(String record) {
        log("parseGGA:" + record);
        String[] values = split(record);
        clearSatelliteList();
        Location mInfo = getTalkerLocation(mCurrentTalker);
        long utcTime = (Double.valueOf(parseDouble(values[1]))).longValue();
        double lat = parseDouble(values[2]);
        double longt = parseDouble(values[4]);
        double alti = parseDouble(values[9]);
        if (!values[3].equals("N")) {
            lat = -lat;
        }

        if (!values[5].equals("E")) {
            longt = -longt;
        }

        if (mInfo != null) {
            mInfo.setTime(utcTime);
            mInfo.setLatitude(lat);
            mInfo.setLongitude(longt);
            mInfo.setAltitude(alti);
        }
    }

    /**
     *
     * Spec please refer http://www.gpsinformation.org/dale/nmea.htm
     *
     * @param record
     *
     */
    private synchronized void parseGSV(String record) {
        log("parseGSV:" + record);
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
        if (DEBUG) {
            Log.d("nmeaParser", msg);
        }
    }


    private String[] split(String str) {
        String[] result = null;
        String delims = "[,]";
        result = str.split(delims);
        return result;
    }


    private String[] splitLines(String str) {
        String[] result = null;
        String delims = "[\n]";
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
            Log.d("nmeaParser", "parseFloat:" + e);
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
            Log.d("nmeaParser", "parseDouble:" + e);
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
            Log.d("nmeaParser", "parseDouble:" + e);
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
    public void addSVUpdateListener(NmeaUpdateViewListener l) {
        synchronized (mListener) {
            mListener.add(l);
        }
    }

    /**
     * Remove listener to update satellites information.
     * @param l The listener to remove from list
     */
    public void removeSVUpdateListener(NmeaUpdateViewListener l) {
        synchronized (mListener) {
            mListener.remove(l);
        }
    }

/*    private void sendMessage(int what) {
        Message m = new Message();
        m.what = what;
        mHandler.sendMessage(m);
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case SV_UPDATE: //timer update
                reportSVupdate();
                break;
            default:
                log("WARNING: unknown handle event recv!!");
                break;
            }
        }
    };*/


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
    class SVExtraInfro {

        public String mPdop = "";
        public String mHdop = "";
        public String mVdop = "";
        public String mfixtype = "";

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
