
package com.mediatek.settings.ext;

import android.content.Context;
import android.content.ContextWrapper;
import android.widget.TextView;
import android.view.View;
import android.util.Log;
import android.graphics.drawable.Drawable;
import android.view.ViewGroup;

//import com.android.settings.R;


public class DefaultAppListExt  extends ContextWrapper implements IAppListExt {
    private static final String TAG = "DefaultAppListExt";

    public DefaultAppListExt(Context context)   {
           super(context);
            Log.i(TAG, "constructor\n");
    }

    public View addLayoutAppView(View view, TextView textView, TextView defaultLabel,
                                 int position, Drawable image, ViewGroup parent) {
        return view;
    }
    public void setAppListItem(String packageName, int position){}
}
