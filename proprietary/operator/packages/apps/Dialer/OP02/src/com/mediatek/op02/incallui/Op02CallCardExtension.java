package com.mediatek.op02.incallui;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import com.mediatek.incallui.ext.DefaultCallCardExt;
import com.mediatek.op02.dialer.R;

public class Op02CallCardExtension extends DefaultCallCardExt {
    private static final String TAG = "Op02CallCardExtension";
    private static final String ID = "id";
    private static final String HD_ICON = "contactgrid_hdIcon";
    private Context mContext;
    public Op02CallCardExtension(Context context) {
        super();
        Log.d(TAG, "Op02CallCardExtension");
        mContext = context;
    }

    /**
      * Interface to modify the hd icon of OP02
      *
      * @param context the incallactivity context
      * @param view the callcard view
      */
    @Override
    public void onViewCreated(Context context, View view) {
        if (context == null) {
            Log.d(TAG, "onViewCreated, context is null.");
            return;
        }

        Resources resource = context.getResources();
        String packageName = context.getPackageName();

        View hdIcon =
                view.findViewById(
                        resource.getIdentifier(HD_ICON, ID, packageName));
        if (hdIcon != null) {
            ((ImageView) hdIcon).setImageDrawable(
                    mContext.getResources().getDrawable(R.drawable.ic_hd_audio));
        }
    }

}