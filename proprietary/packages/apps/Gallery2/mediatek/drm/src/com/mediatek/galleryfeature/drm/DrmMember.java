package com.mediatek.galleryfeature.drm;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Context;
import android.content.res.Resources;

import com.mediatek.gallerybasic.base.*;
import com.mediatek.gallerybasic.gl.GLIdleExecuter;
import com.mediatek.gallerybasic.util.Log;
import com.mediatek.omadrm.OmaDrmUtils;

import java.lang.SecurityException;

/**
 * Drm Member.
 */
public class DrmMember extends MediaMember {
    private static final String TAG = "MtkGallery2/DrmMember";
    public static int sType;

    /**
     * Constructor.
     *
     * @param context The context for Drm.
     */
    public DrmMember(Context context) {
        super(context);
    }

    /**
     * Constructor.
     * @param context The context for Drm.
     * @param res The res of Drm feature.
     */
    public DrmMember(Context context, Resources res) {
        super(context, null, res);
    }

    /**
     * Constructor.
     * @param context The context for Drm.
     * @param exe GLIdleExecuter for GLThread operation.
     * @param res The res of Drm feature.
     */
    public DrmMember(Context context, GLIdleExecuter exe, Resources res) {
        super(context, exe, res);
    }

    @Override
    public boolean isMatching(MediaData md) {
        if (!DrmHelper.sSupportDrm || md == null) {
            return false;
        }
        boolean isDrm = false;
        if (md.extFileds != null) {
            Object drmImageField = md.extFileds.getImageField(DrmField.IS_DRM);
            Object drmVideoField = md.extFileds.getVideoField(DrmField.IS_DRM);
            if (drmImageField != null) {
                isDrm = (1 == (int) drmImageField);
            }
            if (drmVideoField != null) {
                isDrm = (1 == (int) drmVideoField);
            }
        }
        // all DRM file name is end with .dcf,should not modify '.dcf'
        if (isDrm
                && !(DrmHelper.isDataProtectionFile(md.filePath) || DrmHelper.isDrmFile(md
                .filePath))) {
            Log.d(TAG, "<isMatching> DRM fileName = " + md.filePath);
            return false;
        }
        // FilePath is null, means Gallery is started by third part apk.
        if (!isDrm && (md.uri != null) && (md.filePath == null || md.filePath.equals(""))) {
            if (!ContentResolver.SCHEME_FILE.equals(md.uri.getScheme())) {
                try {
                    isDrm =
                            OmaDrmUtils.isDrm(DrmHelper.getOmaDrmClient(mContext), md.uri);
                    md.filePath = DrmHelper.convertUriToPath(mContext, md.uri);
                } catch (IllegalArgumentException e) {
                    Log.d(TAG, "<isMatching> IllegalArgumentException", e);
                } catch (SecurityException e) {
                    Log.d(TAG, "<isMatching> SecurityException", e);
                    ((Activity)mContext).finish();
                }
                //Add for CTA image.
                if ((!isDrm) && DrmHelper.isDataProtectionFile(md.filePath) && DrmHelper
                        .sSupportCTA) {
                    isDrm = true;
                }
            } else {
                md.filePath = md.uri.getPath();
            }
            Log.d(TAG, "<isMatching> filePath = " + md.filePath + "  md.uri = " + md
                    .uri + " isDrm = " + isDrm);
        }
        return isDrm;
    }

    @Override
    public Player getPlayer(MediaData md, ThumbType type) {
        if (type == ThumbType.MIDDLE) {
            return new DrmPlayer(mContext, md, Player.OutputType.TEXTURE, mMediaCenter, mResources);
        }
        return null;
    }

    public Layer getLayer() {
        return new DrmLayer(mMediaCenter);
    }

    @Override
    public ExtItem getItem(MediaData md) {
        return new DrmItem(mContext, md, mMediaCenter);
    }

    @Override
    protected void onTypeObtained(int type) {
        sType = type;
    }

    @Override

    public int getPriority() {
        return Integer.MAX_VALUE;
    }

}
