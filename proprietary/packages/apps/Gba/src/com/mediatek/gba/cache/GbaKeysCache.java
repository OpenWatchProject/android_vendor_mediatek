package com.mediatek.gba.cache;

import android.util.Log;

import com.mediatek.gba.NafSessionKey;
import com.mediatek.gba.element.NafId;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.Map;
import java.util.TimeZone;

/**
 * implementation for GbaKeysCache.
 *
 * @hide
 */
public class GbaKeysCache {
    private static final String TAG = "GbaKeysCache";

    private static final String UTC_PATTERN_TIMEZONE = "yyyy-MM-dd'T'HH:mm:ss'Z'";
    private static final String UTC_PATTERN = "yyyy-MM-dd'T'HH:mm:ss";

    private Map<GbaKeysCacheEntryKey, NafSessionKey> mMap =
            new HashMap<GbaKeysCacheEntryKey, NafSessionKey>();

    /**
     * Utility function to check the NAF_Ks_ext key is expired or not.
     *
     * @param nafId the id of NAF server.
     * @param subId subscription id.
     * @return indicate the NAF_Ks_ext key is expired not or.
     */
    public boolean isExpiredKey(NafId nafId, int subId) {
        boolean isExpired = true;
        GbaKeysCacheEntryKey key = new GbaKeysCacheEntryKey(nafId, subId);

        boolean res = mMap.containsKey(key);
        Log.i(TAG, "   containsKey=" + res);

        if (res) {
            NafSessionKey nafSessionKey = mMap.get(key);
            //Check time is expired or not

            //Get current UTC time
            Calendar calenar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
            long cTime = calenar.getTimeInMillis();
            Log.d(TAG, "now: " + calenar.getTime() + ", cTime=" + cTime);

            Date expiredDate = getExpiredDate(nafSessionKey.getKeylifetime());

            if (expiredDate == null) {
                Log.e(TAG, "Can't get expired date");
                return true;
            }

            calenar.setTime(expiredDate);
            long eTime = calenar.getTimeInMillis();
            Log.d(TAG, "exp: " + expiredDate + ", eTime=" + eTime);
            Log.d(TAG, "isExpiredKey: " + (eTime < cTime)
                    + ", subId=" + subId + ", nafId=" + nafId);
            return (eTime < cTime);
        }

        return isExpired;
    }

    /**
     * to check the cache has NAF_Ks_ext key is expired or not.
     *
     * @param nafId the id of NAF server.
     * @param subId subscription id.
     * @return indicate the cache has NAF_Ks_ext key is expired or not.
     */
    public boolean hasKey(NafId nafId, int subId) {
        GbaKeysCacheEntryKey key = new GbaKeysCacheEntryKey(nafId, subId);

        return mMap.containsKey(key);
    }

    /**
     * to get NAF_Ks_ext key from cache.
     *
     * @param nafId the id of NAF server.
     * @param subId subscription id.
     * @return the NAF_Ks_ext object whichi is stored in NafSessionKey.
     */
    public synchronized NafSessionKey getKeys(NafId nafId, int subId) {
        GbaKeysCacheEntryKey key = new GbaKeysCacheEntryKey(nafId, subId);

        return mMap.get(key);
    }

    /**
     * to put NAF_Ks_ext key into cache.
     *
     * @param nafId the id of NAF server.
     * @param subId subscription id.
     * @param sessionKey the NAF_Ks_ext key value
     */
    public synchronized void putKeys(NafId nafId, int subId, NafSessionKey sessionKey) {
        if (sessionKey == null) {
            throw new IllegalArgumentException("The sessionKey mus be not null.");
        }

        GbaKeysCacheEntryKey key = new GbaKeysCacheEntryKey(nafId, subId);
        mMap.put(key, sessionKey);
    }

    private Date getExpiredDate(String utcDate) {
        if (utcDate == null) {
            return null;
        }
        SimpleDateFormat sDateFormat = null;
        Date expiredDate = null;

        try {
            Log.i(TAG, "Expired date:" + utcDate);

            if (utcDate.indexOf("Z") != -1) {
                sDateFormat = new SimpleDateFormat(UTC_PATTERN_TIMEZONE);
                sDateFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
            } else {
                sDateFormat = new SimpleDateFormat(UTC_PATTERN);
            }

            expiredDate = sDateFormat.parse(utcDate);
        } catch (ParseException e) {
            e.printStackTrace();
        }

        return expiredDate;
    }
}