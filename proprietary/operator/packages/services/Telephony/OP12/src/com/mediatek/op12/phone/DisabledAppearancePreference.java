package com.mediatek.op12.phone;

import android.content.Context;
import android.preference.CheckBoxPreference;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

public class DisabledAppearancePreference extends CheckBoxPreference {

    protected boolean mEnabledAppearance = false;

    public DisabledAppearancePreference(Context context,
            AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        boolean viewEnabled = mEnabledAppearance;
        enableView(view, viewEnabled);
    }

   protected void enableView(View view, boolean enabled) {
    view.setEnabled(enabled);
    if (view instanceof ViewGroup) {
        ViewGroup grp = (ViewGroup) view;
        for (int index = 0; index < grp.getChildCount(); index++)
            enableView(grp.getChildAt(index), enabled);
    }
   }

   public void setEnabledAppearance(boolean enabled) {
      mEnabledAppearance = enabled;
      notifyChanged();
   }
}