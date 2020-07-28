package com.mediatek.op08.dialer.setting;

import android.content.Context;
import android.net.Uri;
import android.database.Cursor;
import android.util.Log;

import java.util.HashMap;

public class SettingConstant{
    private static final String TAG = "SettingConstant";

    public static final String AUTHORITY = "enrich_call_setting";
    public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY);

    public static final int TYPE_MASTER_SWITCH = 0;
    public static final int TYPE_IMPORTANT_INDICATOR = 1;
    public static final int TYPE_SUBJECT = 2;
    public static final int TYPE_IMAGE = 3;
    public static final int TYPE_LOCATION = 4;

    public static final String RCS_CALL_PREF_NAME = "rcs_enrich_setting";

    //hashMap key for dialer ui
    public static final String HASHMAP_PRIORITY_KEY = "Priority";
    public static final String HASHMAP_SUBJECT_KEY = "Subject";
    public static final String HASHMAP_CELLINFO_KEY = "Call-info";
    public static final String HASHMAP_LOCATION_KEY = "location";

    private static final String PRIORITY_STANDARD = "standard";
    private static final String PRIORITY_URGENT = "urgent";

    //preference_key, for looking up content
    public static final String KEY_MASTER_SWITCH_CONTENT = "mastercontent";
    public static final String KEY_INDICATOR_CONTENT = "importantContent";
    public static final String KEY_SUBJECT_CONTENT = "subjectContent";
    public static final String KEY_IMAGE_CONTENT = "imageContent";
    public static final String KEY_LOCATION_CONTENT = "locationContent";
    //preference_key, for looking up switchStatus
    public static final String KEY_MASTER_SWITCH_STATUS = "masterStatus";
    public static final String KEY_INDICATOR_STATUS = "importantStatus";
    public static final String KEY_SUBJECT_STATUS = "subjectStatus";
    public static final String KEY_IMAGE_STATUS = "imageStatus";
    public static final String KEY_LOCATION_STATUS = "locationStatus";

    public static String getContentKeyByType(int type) {
        switch (type) {
        case TYPE_SUBJECT:
            return KEY_SUBJECT_CONTENT;
        case TYPE_IMAGE:
            return KEY_IMAGE_CONTENT;
        case TYPE_LOCATION:
            return KEY_LOCATION_CONTENT;
        case TYPE_IMPORTANT_INDICATOR:
            return KEY_INDICATOR_CONTENT;
        default:
            return null;
        }
    }

    public static String getStatusKeyByType(int type) {
        switch (type) {
        case TYPE_SUBJECT:
            return KEY_SUBJECT_STATUS;
        case TYPE_IMAGE:
            return KEY_IMAGE_STATUS;
        case TYPE_LOCATION:
            return KEY_LOCATION_STATUS;
        case TYPE_IMPORTANT_INDICATOR:
            return KEY_INDICATOR_STATUS;
        default:
            return null;
        }
    }

    static String getRandomLocationLon() {
        double lon = Math.random()*180;
        return String.format("%.2f", lon);
    }

    static String getRandomLocationLat() {
        double lat = Math.random()*90;
        return String.format("%.2f", lat);
    }

    /*
     * Interface for dialer AP to get enrich call setting info from plugin AP.
     */
    public static HashMap<String, String> getEnrichCallSettingInfo(Context context) {
        Log.d(TAG, "getEnrichCallSettingInfo context: " + context);
        Cursor cursor = context.getContentResolver()
                .query(SettingConstant.CONTENT_URI, null, null, null, null);
        if (cursor == null) {
            Log.d(TAG, "getEnrichCallSettingInfo perhaps no setting data here, return null");
            return null;
        }
        if (cursor.getCount() <= 0) {
            cursor.close();
            return null;
        }

        HashMap<String, String> ret = new HashMap<String, String>();
        cursor.moveToFirst();
        Log.d(TAG, "getEnrichCallSettingInfo count: " + cursor.getCount());
        while (!cursor.isAfterLast()) {
            int type = cursor.getInt(1);
            int status = cursor.getInt(2);
            String content = cursor.getString(3);
            Log.d(TAG, "getDataFromProvider type: " + type + " status: " + status
                    + " content: " + content);

            if (type == TYPE_IMPORTANT_INDICATOR) {
                if (status == 1) {
                    content = PRIORITY_URGENT;
                } else {
                    content = PRIORITY_STANDARD;
                }
                Log.d(TAG, "getDataFromProvider put value to ret: " + HASHMAP_PRIORITY_KEY +
                        " : " + " content: " + content);
                ret.put(HASHMAP_PRIORITY_KEY, content);
            } else if (type == TYPE_LOCATION) {
                if (content != null) {
                    String location;
                    String[] values = content.split(",");
                    if (values != null && values.length >= 2) {
                        values[0].trim();
                        int start = new String("Longitude:").length();
                        String longitude = values[0].substring(start).trim();

                        values[1].trim();
                        start = new String(" Latitude:").length();
                        String latitude = values[1].substring(start).trim();

                        location = latitude + "," + longitude + "," + "0.0001";
                        Log.d(TAG, "getDataFromProvider location: " + location);

                        Log.d(TAG, "getDataFromProvider put value to ret: " + HASHMAP_LOCATION_KEY
                                + " : " + " content: " + location);
                        ret.put(HASHMAP_LOCATION_KEY, location);
                    }
                }
            } else {
                if (status == 1) {
                    String key = getHashmapKeyByType(type);
                    if (key != null && content != null) {
                        Log.d(TAG, "getDataFromProvider put value to ret: " + key + " : " +
                                " content: " + content);
                        ret.put(key, content);
                    }
                }
            }
            cursor.moveToNext();
        }

        if (cursor != null) {
            cursor.close();
        }
        return ret;
    }

    private static String getHashmapKeyByType(int type) {
        switch(type) {
            case TYPE_IMPORTANT_INDICATOR:
                return HASHMAP_PRIORITY_KEY;
            case TYPE_SUBJECT:
                return HASHMAP_SUBJECT_KEY;
            case TYPE_IMAGE:
                return HASHMAP_CELLINFO_KEY;
            case TYPE_LOCATION:
                return HASHMAP_LOCATION_KEY;
            default:
                return null;
        }
    }
}