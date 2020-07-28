package com.mediatek.mms.plugin;

import android.content.Context;
import android.widget.TextView;
import android.widget.VideoView;

import com.mediatek.mms.ext.DefaultOpSlideViewExt;

/**
 * Op01SlideViewExt.
 *
 */
public class Op01SlideViewExt extends DefaultOpSlideViewExt {

    /**
     * Construction.
     * @param context Context
     */
    public Op01SlideViewExt(Context context) {
        super(context);
    }

    @Override
    public void setText(Context context, TextView textView, boolean conformanceMode) {
        /* q0 migration, phase out text size adjust*/
        /*
        if (!conformanceMode) {
            float size =  Op01MmsUtils.getTextSize(context);
            textView.setTextSize(size);
        }
        */
        /* q0 migration, phase out open url dialog*/
//        Op01MmsUtils.setExtendUrlSpan(textView);
    }

    @Override
    public void setVideo(VideoView videoView) {
        videoView.seekTo(1);
    }

    /* q0 migration, phase out text size adjust*/
    /*
    @Override
    public void enableMMSConformanceMode(Context context, TextView textView,
            int textLeft, int textTop) {
        if (textLeft >= 0 && textTop >= 0) {
            float size =  Op01MmsUtils.getTextSize(context);
            textView.setTextSize(size);
        }
    }
    */
}
