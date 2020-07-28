package com.mediatek.cellbroadcastreceiver;

import android.util.Log;
import java.util.Arrays;
import java.util.List;
import com.mediatek.internal.telephony.gsm.cbutil.Vertex;

public class CMASUtils {

    private static final String TAG = "[CMAS]CMASUtils";

    public static final int PRIORITY_PRESIDENTIAL = 12;
    public static final int PRIORITY_NONE_PRESIDENTIAL = 10;
    public static final int PRIORITY_EXTREAM = 8;
    public static final int PRIORITY_SEVERE = 4;
    public static final int PRIORITY_AMBER = 0;

    private static final Integer[] presidential_msgId = {4370};
    /*private static final Integer[] extream_msgId = {4371, 4372};
    private static final Integer[] severe_msgId = {4373, 4374, 4375, 4376, 4377, 4378};
    private static final Integer[] amber_msgId = {4379};
    private static final Integer[] none_presidential_msgId = {4371, 4372, 4373, 4374, 4375, 4376, 4377, 4378, 4379}; */
    private static final List<Integer> PRESIDENTIAL_MSGID = Arrays.asList(presidential_msgId);


    public static int getMsgPriority(int msgId) {
        Log.d(TAG, "getMsgPriority:: msgId = " + msgId);

        if (PRESIDENTIAL_MSGID.contains(msgId)) {
            return PRIORITY_PRESIDENTIAL;
        } else {
            return PRIORITY_NONE_PRESIDENTIAL;
        }
    }

    public static String convertMsgId2Str(int msgId) {
          if (CmasConfigManager.isChileProfile()) {
            if (msgId == 919 || msgId == 921 || (msgId > 4370 && msgId < 4399) || msgId == 4411) {
                if (msgId == 919 || msgId == 921) {
                    return "MsgId" + String.valueOf(msgId - 919 + 1);
                } else {
                    return "MsgId" + String.valueOf(msgId - 4370 + 1);
                }
            } else {
                Log.d(TAG, "convertMsgId2Str msgId = " + msgId + ", error");
                return null;
            }
        } else if (msgId < 4370 || msgId > 4399) {
            Log.d(TAG, "convertMsgId2Str msgId = " + msgId + ", error");
            return null;
        }

        return "MsgId" + String.valueOf(msgId - 4370 + 1);
    }

    public static String getSlotKey(int slotId) {
        String slotKey = "";
        if (CmasConfigManager.getGeminiSupport()) {
            slotKey = Long.toString(slotId) + "_";
        }
        return slotKey;
    }

    public static boolean PtInPolygon(Vertex point, List<Vertex> APoints) {
        int nCross = 0;
        for (int i = 0; i < APoints.size(); i++) {
            Vertex p1 = APoints.get(i);
            Vertex p2 = APoints.get((i + 1) % APoints.size());
            if ( p1.mLongi == p2.mLongi)
                continue;
            if ( point.mLongi <  Math.min(p1.mLongi, p2.mLongi))
                continue;
            if ( point.mLongi >= Math.max(p1.mLongi, p2.mLongi))
                continue;
            double x = (double)(point.mLongi - p1.mLongi) *
                (double)(p2.mLati - p1.mLati) / (double)(p2.mLongi - p1.mLongi) + p1.mLati;
            if ( x > point.mLati )
                nCross++;
        }
        return (nCross % 2 == 1);
    }

    /**
     * Earth Radius
     */
    private static double EarthRadius = 6378.137;

    private static double rad(double d) {
        return d * Math.PI / 180.0;
    }

    public static double getDistance(double firstLatitude, double firstLongitude,
                                     double secondLatitude, double secondLongitude) {
        double firstRadLat = rad(firstLatitude);
        double firstRadLng = rad(firstLongitude);
        double secondRadLat = rad(secondLatitude);
        double secondRadLng = rad(secondLongitude);

        double a = firstRadLat - secondRadLat;
        double b = firstRadLng - secondRadLng;
        double cal = 2 * Math.asin(Math.sqrt(Math.pow(Math.sin(a / 2), 2) + Math.cos(firstRadLat)
                * Math.cos(secondRadLat) * Math.pow(Math.sin(b / 2), 2))) * EarthRadius;
        double result = Math.round(cal * 10000d) / 10000d;
        return result;
    }

    public static double GetPointDistance(String firstPoint, String secondPoint) {
        String[] firstArray = firstPoint.split(",");
        String[] secondArray = secondPoint.split(",");
        double firstLatitude = Double.valueOf(firstArray[0].trim());
        double firstLongitude = Double.valueOf(firstArray[1].trim());
        double secondLatitude = Double.valueOf(secondArray[0].trim());
        double secondLongitude = Double.valueOf(secondArray[1].trim());
        return getDistance(firstLatitude, firstLongitude, secondLatitude, secondLongitude);
    }
}
