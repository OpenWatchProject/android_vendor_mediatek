package com.mediatek.mms.util;

import android.util.Log;

import java.util.Hashtable;
import java.util.Iterator;

public class DrmCacheManager {

    private static final String TAG = "DrmCacheManager";
    private static Hashtable<String, Object> sCacheList = new Hashtable<String, Object>();
    private static int sMaxCacheSize = 20;

    private DrmCacheManager() {
    }

    public synchronized static void add(String key, String src, boolean isDrm, boolean hasRight) {
        DrmCacheItem item = new DrmCacheItem(key, src, isDrm, hasRight);
        sCacheList.put(key, item);
    }

    public synchronized static DrmCacheItem get(String key) {
        Object obj = sCacheList.get(key);
        if (obj == null) {
            return null;
        }
        DrmCacheItem item = (DrmCacheItem) obj;
        item.visit();
        clearExpiredCache();
        return item;
    }

    public synchronized static void remove(String key) {
        Object obj = sCacheList.get(key);
        if (obj != null) {
            obj = null;
        }
        sCacheList.remove(key);
    }

    public synchronized static void clear() {

        for (String s : sCacheList.keySet()) {
            sCacheList.put(s, null);
        }
        sCacheList.clear();
    }

    private synchronized static void clearExpiredCache() {
        if (sCacheList.size() < sMaxCacheSize) {
            return;
        }

        for (Iterator it = sCacheList.keySet().iterator(); it.hasNext(); ) {
            String key = (String) it.next();
            DrmCacheItem item = (DrmCacheItem) sCacheList.get(key);
            if (item.isExpired()) {
                it.remove();
                Log.d(TAG, "clearExpiredCache: " + key);
            }
        }
    }
}
