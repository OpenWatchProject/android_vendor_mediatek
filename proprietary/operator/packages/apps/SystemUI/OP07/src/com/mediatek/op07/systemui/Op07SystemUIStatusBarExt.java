package com.mediatek.op07.systemui;

import android.content.Context;
import android.util.Log;
import com.mediatek.systemui.ext.DefaultSystemUIStatusBarExt;

public class Op07SystemUIStatusBarExt extends DefaultSystemUIStatusBarExt {
    static final String TAG = "Op07StatusBarPlmnPluginExt";
    private Context mContext;

    /**
     * Constructs a new Op07SystemUIStatusBarExt instance with Context.
     * @param context The Context object
     */
    public Op07SystemUIStatusBarExt(Context context) {
        super(context);
        mContext = context;
    }

    //@Override
    public boolean needShowRoamingIcons(boolean isInRoaming) {
        Log.d(TAG, "needShowRoamingIcons");
        //for stocked devices no need to show roaming icons.
        return false;
    }

   //@Override
    public void setCustomizedPlmnTextTint(int tint) {
        Log.d(TAG, "setCustomizedPlmnTextTint");
        Op07StatusBarPlmnPluginExt.setTint(tint);
    }
}