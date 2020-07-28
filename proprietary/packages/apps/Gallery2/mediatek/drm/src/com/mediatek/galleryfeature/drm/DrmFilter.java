package com.mediatek.galleryfeature.drm;

import android.content.Intent;

import com.mediatek.gallerybasic.base.IFilter;
import com.mediatek.gallerybasic.base.MediaFilter;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.omadrm.OmaDrmStore;

public class DrmFilter implements IFilter {
    private final static String TAG = "DrmFilter";
    private static final String KEY_GET_CONTENT = "get-content";
    private static final int INVALID_DRM_LEVEL = -1;
    private static final int INCLUDE_DRM_FL = (1 << 1);
    private static final int INCLUDE_DRM_CD = (1 << 2);
    private static final int INCLUDE_DRM_SD = (1 << 3);
    private static final int INCLUDE_DRM_FLSD = (1 << 4);
    private static final int INCLUDE_DRM_ALL = INCLUDE_DRM_FL | INCLUDE_DRM_CD
            | INCLUDE_DRM_SD | INCLUDE_DRM_FLSD;

    public void setFlagFromIntent(Intent intent, MediaFilter filter) {
        if (!DrmHelper.sSupportDrm) {
            return;
        }
        if (intent == null) {
            return;
        }
        filter.setFlagDisable(INCLUDE_DRM_ALL);
        int drmLevel = intent.getIntExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL,
                INVALID_DRM_LEVEL);
        Log.d(TAG, "<setFlagFromIntent> drmLevel = " + drmLevel);
        switch (drmLevel) {
            case INVALID_DRM_LEVEL:
                // set LEVEL_ALL as default.
                intent.putExtra(OmaDrmStore.DrmIntentExtra.EXTRA_DRM_LEVEL, OmaDrmStore
                        .DrmIntentExtra.LEVEL_ALL);
                filter.setFlagEnable(INCLUDE_DRM_ALL);
                break;
            case OmaDrmStore.DrmIntentExtra.LEVEL_FL:
                filter.setFlagEnable(INCLUDE_DRM_FL);
                break;
            // For mms ACTION_GET_CONTENT action.
            case OmaDrmStore.DrmIntentExtra.LEVEL_SD:
                filter.setFlagEnable(INCLUDE_DRM_SD);
                break;
            case OmaDrmStore.DrmIntentExtra.LEVEL_ALL:
                filter.setFlagEnable(INCLUDE_DRM_ALL);
                break;
        }
        Log.d(TAG, " <setFlagFromIntent> = " + filter);
    }

    public void setDefaultFlag(MediaFilter filter) {
        if (!DrmHelper.sSupportDrm) {
            return;
        }
        filter.setFlagEnable(INCLUDE_DRM_ALL);
    }

    public String getWhereClauseForImage(int flag, int bucketID) {
        return getWhereClauseInternal(flag);
    }

    public String getWhereClauseForVideo(int flag, int bucketID) {
        return getWhereClauseInternal(flag);
    }

    public String getWhereClause(int flag, int bucketID) {
        return getWhereClauseInternal(flag);
    }

    public String getDeleteWhereClauseForImage(int flag, int bucketID) {
        return getWhereClauseInternal(flag);
    }

    public String getDeleteWhereClauseForVideo(int flag, int bucketID) {
        return getWhereClauseInternal(flag);
    }

    private String getWhereClauseInternal(int flag) {
        return null;
/* Comment DRM file based check as now we will rely on media type
        if (!DrmHelper.sSupportDrm) {
            return null;
        }
        String noDrmClause = DrmField.IS_DRM + "=0 OR " + DrmField.IS_DRM + " IS NULL";
        if ((flag & INCLUDE_DRM_ALL) == 0) {
            return noDrmClause;
        }
        String whereClause = null;
        if ((flag & INCLUDE_DRM_FL) != 0) {
            whereClause =
                    MediaFilter.OR(whereClause, DrmField.DRM_METHOD + "="
                            + OmaDrmStore.Method.FL);
        }
        if ((flag & INCLUDE_DRM_CD) != 0) {
            whereClause =
                    MediaFilter.OR(whereClause, DrmField.DRM_METHOD + "="
                            + OmaDrmStore.Method.CD);
        }
        if ((flag & INCLUDE_DRM_SD) != 0) {
            whereClause =
                    MediaFilter.OR(whereClause, DrmField.DRM_METHOD + "="
                            + OmaDrmStore.Method.SD);
        }
        if ((flag & INCLUDE_DRM_FLSD) != 0) {
            whereClause =
                    MediaFilter.OR(whereClause, DrmField.DRM_METHOD + "="
                            + OmaDrmStore.Method.FLSD);
        }
        if (whereClause != null) {
            whereClause = MediaFilter.AND(DrmField.IS_DRM + "=1", whereClause);
        }
        whereClause = MediaFilter.OR(noDrmClause, whereClause);
        return whereClause;
*/
    }

    public String convertFlagToString(int flag) {
        if (!DrmHelper.sSupportDrm) {
            return null;
        }
        StringBuilder sb = new StringBuilder();
        if ((flag & INCLUDE_DRM_FL) != 0) {
            sb.append("INCLUDE_DRM_FL, ");
        }
        if ((flag & INCLUDE_DRM_CD) != 0) {
            sb.append("INCLUDE_DRM_CD, ");
        }
        if ((flag & INCLUDE_DRM_SD) != 0) {
            sb.append("INCLUDE_DRM_SD, ");
        }
        if ((flag & INCLUDE_DRM_FLSD) != 0) {
            sb.append("INCLUDE_DRM_FLSD, ");
        }
        return sb.toString();
    }
}
