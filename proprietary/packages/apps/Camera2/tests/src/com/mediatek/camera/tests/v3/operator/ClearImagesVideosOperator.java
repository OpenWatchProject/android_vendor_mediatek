package com.mediatek.camera.tests.v3.operator;

import android.provider.MediaStore;

import com.mediatek.camera.tests.v3.arch.OperatorOne;
import com.mediatek.camera.tests.v3.arch.Page;
import com.mediatek.camera.tests.v3.util.Utils;

public class ClearImagesVideosOperator extends OperatorOne {
    @Override
    protected void doOperate() {
        Utils.getTargetContext().getContentResolver().delete(
                MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null);
        Utils.getTargetContext().getContentResolver().delete(
                MediaStore.Video.Media.EXTERNAL_CONTENT_URI, null, null);
    }

    @Override
    public Page getPageBeforeOperate() {
        return null;
    }

    @Override
    public Page getPageAfterOperate() {
        return null;
    }

    @Override
    public String getDescription() {
        return "Delete all images and videos in storage";
    }
}
