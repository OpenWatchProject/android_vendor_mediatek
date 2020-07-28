package com.mediatek.mms.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.net.Uri;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.VideoView;

public class DefaultOpSlideViewExt extends ContextWrapper implements IOpSlideViewExt {

    public DefaultOpSlideViewExt(Context base) {
        super(base);
    }

    @Override
    public void setText(Context context, TextView textView, boolean conformanceMode) {

    }

    @Override
    public void setVideo(VideoView videoView) {

    }

    @Override
    public void enableMMSConformanceMode(Context context, TextView textView,
            int textLeft, int textTop) {

    }

    @Override
    public void setImage(ImageView imageView, ViewGroup viewGroup) {
    }

    @Override
    public void initAudioInfoView(boolean conformanceMode, LinearLayout audioInfoView,
            LinearLayout viewPort, ViewGroup viewGroup) {
    }

    @Override
    public void displayAudioInfo() {
    }

    @Override
    public void hideAudioInfo() {
    }

    @Override
    public void setAudio(Uri audio) {
    }

    @Override
    public void videoViewMatchStatusbar(View view, ViewGroup viewGroup) {
    }

}
