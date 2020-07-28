package com.mediatek.mms.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.graphics.Bitmap;
import android.net.Uri;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public class DefaultOpMmsPlayerActivityAdapterExt extends ContextWrapper implements
        IOpMmsPlayerActivityAdapterExt {

    public DefaultOpMmsPlayerActivityAdapterExt(Context base) {
        super(base);
    }

    @Override
    public void setExtendUrlSpan(TextView tv) {

    }

    @Override
    public void getView(Uri imageUri, String imageType,
            ImageView image, Bitmap t,
            String videoType, ImageView video, String audioName,
            String audioType, ImageView audioIcon, View audio,
            LinearLayout viewGroup) {

    }

}
