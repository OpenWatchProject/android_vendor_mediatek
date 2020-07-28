package com.mediatek.systemui.ext;

import android.content.Context;
import android.content.ContextWrapper;

import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.LinearLayout;

/**
 * Default implementation of Plug-in definition of Status bar.
 */
public class DefaultStatusBarPlmnPlugin extends ContextWrapper implements IStatusBarPlmnPlugin {

    /**
     * Constructs a new DefaultStatusBarPlmnPlugin instance with Context.
     * @param context A Context object
     */
    public DefaultStatusBarPlmnPlugin(Context context) {
        super(context);
    }

    @Override
    public boolean supportCustomizeCarrierLabel() {
        return false;
    }

    @Override
    public View customizeCarrierLabel(ViewGroup parentView, View orgCarrierLabel) {
        return null;
    }

    @Override
    public void updateCarrierLabelVisibility(boolean force, boolean makeVisible) {
    }

    @Override
    public void updateCarrierLabel(int slotId, boolean isSimInserted, boolean isHasSimService,
            String[] networkNames) {
    }

    @Override
    public void addPlmn(LinearLayout statusBarContents, Context contx) {
    }

    @Override
    public void setPlmnVisibility(int visibility) {
    }
}
