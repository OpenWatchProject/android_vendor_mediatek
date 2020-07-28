package com.mediatek.op08.deviceswitch;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;


import com.mediatek.op08.dialer.R;

import java.util.List;

public class DeviceSwitchListAdapter extends ArrayAdapter<DeviceSwitchInfo> {
    private static final String TAG = "DeviceSwitchListAdapter";
    private final Context mContext;
    private final List<DeviceSwitchInfo> mValues;
    private int mSelectedItemPosition = 0;
    private String mCallid;

    public DeviceSwitchListAdapter(Context context, List<DeviceSwitchInfo> values, String callId) {
        super(context, R.layout.mtk_device_switch_list_item, values);
        this.mContext = context;
        this.mValues = values;
        this.mCallid = callId;
        mSelectedItemPosition = -1;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        LayoutInflater inflater = (LayoutInflater) mContext
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        convertView = inflater.inflate(R.layout.mtk_device_switch_list_item, parent, false);
        Log.d(TAG, "getView:" + convertView + ", position:" + position);
        return new DeviceSwitchListItem(mContext, convertView, mValues, position, mCallid);
    }

    public void setSelectedItemId(int id) {
        if (id < getCount()) {
            mSelectedItemPosition = id;
        } else {
            mSelectedItemPosition = -1;
        }
    }

    public int getSelectedItemId() {
        return mSelectedItemPosition;
    }
}