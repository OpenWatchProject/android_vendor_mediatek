package com.mediatek.op08.dialer.setting;

import android.content.Context;
import android.support.v7.widget.RecyclerView.ViewHolder;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Switch;
import android.widget.TextView;
import android.util.Log;

import com.mediatek.op08.dialer.R;

class SettingHeaderViewHolder extends ViewHolder {
    private static final String TAG = "SettingHeaderViewHolder";

    private final int mDisableColor;
    private final int mTextEnableColor;

    private final TextView mContentView;
    private final Switch mSwitch;
    private Context mContext;
    private boolean mMasterStatus;


    SettingHeaderViewHolder(View container, Context context) {
        super(container);
        Log.d(TAG, "SettingHeaderViewHolder");
        mContext = context;

        mContentView = (TextView) container.findViewById(R.id.content);
        mContentView.setText(mContext.getString(R.string.rcs_enrich_call_status));
        mSwitch = (Switch) container.findViewById(R.id.item_switch);

        mDisableColor = mContext.getColor(R.color.setting_disabled_color);
        mTextEnableColor = mContext.getColor(R.color.text_enabled_color);
    }

    void setData(boolean masterStatus) {
        Log.d(TAG, "setData masterStatus: " + masterStatus);
        mSwitch.setChecked(masterStatus);
        mContentView.setClickable(false);
        mSwitch.setClickable(false);

        mMasterStatus = masterStatus;
        if (mMasterStatus) {
            mContentView.setTextColor(mTextEnableColor);
        } else {
            mContentView.setTextColor(mDisableColor);
        }
    }
}
