package com.mediatek.mms.ext;

import android.graphics.Bitmap;
import android.net.Uri;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

public interface IOpMmsPlayerActivityAdapterExt {
    /**
     * @internal
     */
    void setExtendUrlSpan(TextView tv);

    /**
     * @internal
     */
    void getView(Uri imageUri, String imageType, ImageView image, Bitmap t, String videoType,
            ImageView video, String audioName, String audioType, ImageView audioIcon, View audio,
            LinearLayout viewGroup);
}
