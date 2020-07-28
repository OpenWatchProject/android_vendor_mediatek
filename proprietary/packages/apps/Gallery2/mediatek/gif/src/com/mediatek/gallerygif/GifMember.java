package com.mediatek.gallerygif;

import android.content.Context;
import android.content.res.Resources;

import com.mediatek.gallerybasic.base.ExtItem;
import com.mediatek.gallerybasic.base.Layer;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.MediaMember;
import com.mediatek.gallerybasic.base.Player;
import com.mediatek.gallerybasic.base.ThumbType;
import com.mediatek.gallerybasic.gl.GLIdleExecuter;
import com.mediatek.gallerybasic.platform.PlatformHelper;
import com.mediatek.gallerybasic.util.Log;

public class GifMember extends MediaMember {
    private final static String TAG = "MtkGallery2/GifMember";
    private static final int PRIORITY = 20;
    private GLIdleExecuter mGlIdleExecuter;

    public GifMember(Context context) {
        super(context);
    }

    public GifMember(Context context, GLIdleExecuter exe) {
        super(context);
        mGlIdleExecuter = exe;
    }

    public GifMember(Context context, Resources res) {
        super(context, null, res);
    }

    public GifMember(Context context, GLIdleExecuter exe, Resources res) {
        super(context, exe, res);
    }

    @Override
    public boolean isMatching(MediaData md) {
        return md != null && "image/gif".equals(md.mimeType);
    }

    @Override
    public Player getPlayer(MediaData md, ThumbType type) {
        if (PlatformHelper.isOutOfDecodeSpec(md.fileSize, md.width,
                md.height, md.mimeType)) {
            Log.d(TAG, "<getPlayer>, outof decode spec, return null!");
            return null;
        }

        // [Feature reduction] Not support gif auto play back in thumbnail view, so disable it.
        if (type == ThumbType.FANCY || type == ThumbType.MICRO) {
            return null;
        }

        return new GifPlayer(mContext, md, Player.OutputType.TEXTURE, type, mGlIdleExecuter);
    }

    @Override
    public ExtItem getItem(MediaData md) {
        return new GifItem(md, mContext);
    }

    @Override
    public Layer getLayer() {
        return null;
    }

    @Override
    public int getPriority() {
        return PRIORITY;
    }
}
