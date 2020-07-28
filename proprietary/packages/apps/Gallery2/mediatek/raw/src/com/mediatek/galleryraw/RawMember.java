package com.mediatek.galleryraw;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.gallerybasic.base.ExtItem;
import com.mediatek.gallerybasic.base.Layer;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.MediaMember;
import com.mediatek.gallerybasic.gl.GLIdleExecuter;

/**
 * One type of MediaMember special for raw. 1. Check if one MediaData is raw 2. Return the special
 * layer for raw
 */
public class RawMember extends MediaMember {
    private static final String TAG = "MtkGallery2/RawMember";
    public static int sType;
    private static final int PRIORITY = 10;
    public static final String[] RAW_MIME_TYPE = new String[] {
            "image/x-adobe-dng", "image/x-canon-cr2", "image/x-nikon-nef",
            "image/x-nikon-nrw", "image/x-sony-arw", "image/x-panasonic-rw2",
            "image/x-olympus-orf", "image/x-fuji-raf", "image/x-pentax-pef",
            "image/x-samsung-srw"
    };

    private Layer mLayer;

    /**
     * Constructor for RawMember, no special operation, but same as parent.
     * @param context
     *            The context of current application environment
     */
    public RawMember(Context context) {
        super(context);
    }

    public RawMember(Context context, Resources res) {
        super(context, null, res);
    }

    public RawMember(Context context, GLIdleExecuter exe, Resources res) {
        super(context, exe, res);
    }

    @Override
    public boolean isMatching(MediaData md) {
        if (md == null || md.mimeType == null) {
            return false;
        }
        for (String mimetype : RAW_MIME_TYPE) {
            if (mimetype.equals(md.mimeType)) {
                return true;
            }
        }
        return false;
    }

    @Override
    public ExtItem getItem(MediaData md) {
        return new RawItem(md);
    }

    @Override
    public Layer getLayer() {
        if (mLayer == null) {
            mLayer = new RawLayer(mResources);
        }
        return mLayer;
    }

    @Override
    public int getPriority() {
        return PRIORITY;
    }

    @Override
    protected void onTypeObtained(int type) {
        sType = type;
    }
}
