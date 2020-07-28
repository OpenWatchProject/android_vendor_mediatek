package com.mediatek.galleryraw;

import android.app.Activity;
import android.content.res.Resources;
import android.content.res.XmlResourceParser;
import android.os.Build;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.mediatek.gallerybasic.base.Layer;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.base.Player;
import com.mediatek.gallerybasic.gl.MGLView;
import com.mediatek.gallerybasic.util.Log;

/**
 * When current photo is raw format, use this class to show raw indicator at
 * the right-bottom corner.
 */
public class RawLayer extends Layer {
    private static final String TAG = "MtkGallery2/RawLayer";
    private Resources mResources;
    private ViewGroup mRawViewGroup;
    private ImageView mRawImageView;
    private boolean mIsFilmMode;

    public RawLayer(Resources res) {
        mResources = res;
    }

    @Override
    public void onCreate(Activity activity, ViewGroup root) {
        LayoutInflater flater = LayoutInflater.from(activity);
        XmlResourceParser parser = mResources.getLayout(R.layout.raw);
        mRawViewGroup = (ViewGroup) flater.inflate(parser, null);
        mRawViewGroup.setVisibility(View.INVISIBLE);
        mRawImageView = (ImageView) (mRawViewGroup.findViewById(R.id.raw_indicator));
        mRawImageView.setContentDescription(mResources.getString(R.string.indicator_description));
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            mRawImageView.setImageDrawable(mResources.getDrawable(R.drawable.raw_indicator, null));
        } else {
            mRawImageView.setImageDrawable(mResources.getDrawable(R.drawable.raw_indicator));
        }
    }

    @Override
    public void onResume(boolean isFilmMode) {
        Log.d(TAG, "<onResume>");
        mIsFilmMode = isFilmMode;
        updateIndicatorVisibility();
    }

    @Override
    public void onPause() {
        Log.d(TAG, "<onPause>");
        mRawViewGroup.setVisibility(View.INVISIBLE);
    }

    public void onDestroy() {
        Log.d(TAG, "<onDestroy>");
    }

    @Override
    public void setData(MediaData data) {
    }

    @Override
    public void setPlayer(Player player) {
    }

    @Override
    public View getView() {
        return mRawViewGroup;
    }

    @Override
    public MGLView getMGLView() {
        return null;
    }

    @Override
    public void onChange(Player player, int what, int arg, Object obj) {
    }

    @Override
    public void onFilmModeChange(boolean isFilmMode) {
        mIsFilmMode = isFilmMode;
        updateIndicatorVisibility();
    }

    private void updateIndicatorVisibility() {
        if (mIsFilmMode) {
            mRawViewGroup.setVisibility(View.INVISIBLE);
            Log.d(TAG, "<updateIndicatorVisibility> INVISIBLE");
        } else {
            mRawViewGroup.setVisibility(View.VISIBLE);
            Log.d(TAG, "<updateIndicatorVisibility> VISIBLE");
        }
    }
}