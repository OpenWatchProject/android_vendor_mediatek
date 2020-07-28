package com.mediatek.op08.dialer.setting;

import android.content.Context;
import android.support.v7.widget.RecyclerView.ViewHolder;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;
import android.widget.TextView;
import android.util.Log;

import com.mediatek.op08.dialer.R;

class SettingEntryViewHolder extends ViewHolder implements OnCheckedChangeListener,
        OnClickListener {
    private static final String TAG = "SettingEntryViewHolder";
    private static final String IMAGE_PRI = "http://";
    private final int mTextEnableColor;
    private final int mDisableColor;

    private final TextView mContentView;
    private final TextView mTypeView;
    private final Switch mSwitch;
    private Context mContext;
    private boolean mItemStatus;
    private SubSwitchChangeListener mSubSwitchChangeListener;

    private String mContent;
    private int mType;

    SettingEntryViewHolder(View container, SubSwitchChangeListener subSwitchChangeListener,
            Context context) {
        super(container);
        Log.d(TAG, "SettingEntryViewHolder");
        mContext = context;
        mSubSwitchChangeListener = subSwitchChangeListener;

        mContentView = (TextView) container.findViewById(R.id.content);
        mContentView.setOnClickListener(this);
        mTypeView = (TextView) container.findViewById(R.id.type);
        mTypeView.setOnClickListener(this);
        mSwitch = (Switch) container.findViewById(R.id.item_switch);
        mSwitch.setOnCheckedChangeListener(this);
        mContentView.setText(mContext.getString(R.string.default_content));

        mDisableColor = mContext.getColor(R.color.setting_disabled_color);
        mTextEnableColor = mContext.getColor(R.color.text_enabled_color);
    }

    void setEntryData(int type, boolean itemStatus, String content, boolean masterStatus) {
        Log.d(TAG, "setEntryData type: " + type + " content: " + content
                + " itemStatus: " + itemStatus + " masterStatus: " + masterStatus);
        mType = type;
        mItemStatus = itemStatus;
        mTypeView.setText(getItemTitleByType(type));
        mSwitch.setChecked(mItemStatus);
        updateContentView(itemStatus, content);

        if (mType == SettingConstant.TYPE_IMPORTANT_INDICATOR) {
            mContentView.setVisibility(View.GONE);
        }
    }

    private void updateContentView(boolean itemStatus, String content) {
        Log.d(TAG, "updateContentView itemStatus:" + itemStatus + " content:" + content);
        if (itemStatus && (content != null) && !content.isEmpty()) {
            mContent = content;
            mContentView.setText(content);
            mContentView.setTextColor(mTextEnableColor);
        } else {
            mContentView.setText(mContext.getString(R.string.default_content));
            mContentView.setTextColor(mDisableColor);
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        Log.d(TAG, "onCheckedChanged isChecked:" + isChecked);
        if (mItemStatus != isChecked) {
            mItemStatus = isChecked;
            String key = SettingConstant.getStatusKeyByType(mType);
            Log.d(TAG, "onCheckedChanged set key: " + key + " mItemStatus: " + mItemStatus);
            SharedPreferencesHelper.setBooleanToPref(mContext, key, mItemStatus);

            if (mType == SettingConstant.TYPE_LOCATION) {
                Log.d(TAG, "onCheckedChanged location change:" + mItemStatus);
                updateLocationInfo(mItemStatus);
            }

            updateContentView(mItemStatus, mContent);
            mSubSwitchChangeListener.onSubSwitchChanged(mType, mItemStatus);
        }
    }

    private void updateLocationInfo(boolean isEnable) {
        if (isEnable) {
           String lon = SettingConstant.getRandomLocationLon();
           String lat = SettingConstant.getRandomLocationLat();
           mContent = mContext.getString(R.string.location_content, lon, lat);
        } else {
            mContent = null;
        }
        Log.d(TAG, "updateLocationInfo isEnable : " + isEnable + " content: " + mContent);
        SharedPreferencesHelper.setStringToPref(mContext,
                SettingConstant.KEY_LOCATION_CONTENT, mContent);
    }

    private String getItemTitleByType(int type) {
        switch(type) {
            case SettingConstant.TYPE_SUBJECT:
                return mContext.getString(R.string.subject);
            case SettingConstant.TYPE_IMAGE:
                return mContext.getString(R.string.image);
            case SettingConstant.TYPE_LOCATION:
                return mContext.getString(R.string.location);
            case SettingConstant.TYPE_IMPORTANT_INDICATOR:
                return mContext.getString(R.string.indicator_deschri);
            default:
                return null;
        }
    }

    @Override
    public void onClick(View view) {
        Log.d(TAG, "onClick");
        if (!mItemStatus) {
            Log.d(TAG, "onClick mItemStatus false, do nothing and return");
            return;
        }

        if (mType == SettingConstant.TYPE_IMPORTANT_INDICATOR) {
            return;
        }

        if (mType == SettingConstant.TYPE_LOCATION) {
            return;
        }

        if (view == mContentView || view == mTypeView) {
            String deschription = getDialogDeschrip();
            SettingDialogFragment.show(getItemTitleByType(mType), deschription,
                    ((EnrichCallSettingActivity)mContext).getFragmentManager(),
                    result -> {
                         if (mType == SettingConstant.TYPE_IMAGE
                                 && (result.length() <= IMAGE_PRI.length())
                                 && result.startsWith(IMAGE_PRI)) {
                            result = null;
                        }

                        String key = SettingConstant.getContentKeyByType(mType);
                        Log.d(TAG, "enterContentDone key: " + key + " result: " + result);
                        SharedPreferencesHelper.setStringToPref(mContext, key, result);
                        mContent = result;
                        if (mContent != null && !mContent.isEmpty()) {
                            mContentView.setText(mContent);
                            mContentView.setTextColor(mTextEnableColor);
                        } else {
                            mContentView.setText(mContext.getString(R.string.default_content));
                            mContentView.setTextColor(mDisableColor);
                        }
                    });
        }
    }

    private String getDialogDeschrip() {
        String deschription = null;
        if (mType == SettingConstant.TYPE_IMAGE && (mContent == null || mContent.isEmpty())) {
            deschription = IMAGE_PRI;
        } else {
            deschription = mContent;
        }
        return deschription;
    }

    public interface SubSwitchChangeListener {
        void onSubSwitchChanged(int tyle, boolean isChecked);
    }
}
