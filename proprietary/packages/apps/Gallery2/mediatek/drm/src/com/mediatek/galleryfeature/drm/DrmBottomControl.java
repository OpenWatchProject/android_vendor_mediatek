package com.mediatek.galleryfeature.drm;


import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.view.ViewGroup;

import com.mediatek.gallerybasic.base.BackwardBottomController;
import com.mediatek.gallerybasic.base.IBottomControl;
import com.mediatek.gallerybasic.base.MediaData;
import com.mediatek.gallerybasic.util.Log;

public class DrmBottomControl implements IBottomControl {
    private final static String TAG = "DrmBottomControl";

    public DrmBottomControl(Context context, Resources res) {
    }

    @Override
    public void onBottomControlCreated() {

    }

    @Override
    public boolean onUpPressed() {
        return false;
    }

    @Override
    public boolean onBackPressed() {
        return false;
    }

    @Override
    public boolean onBottomControlButtonClicked(int id, MediaData data) {
        return false;
    }

    @Override
    public int canDisplayBottomControls() {
        return DISPLAY_IGNORE;
    }

    @Override
    public int canDisplayBottomControlButton(int id, MediaData data) {
        Log.d(TAG, "<canDisplayBottomControlButton> data = " + data);
        if (data != null && data.extFileds != null
                && null != data.extFileds.getImageField(DrmField.IS_DRM)) {
            if (1 == (int) data.extFileds.getImageField(DrmField.IS_DRM)) {
                return DISPLAY_FALSE;
            }
        }
        return DISPLAY_IGNORE;
    }

    @Override
    public void init(ViewGroup viewRoot, BackwardBottomController controller) {

    }

    @Override
    public boolean onActivityResult(int requestCode, int resultCode, Intent data) {
        return false;
    }
}
