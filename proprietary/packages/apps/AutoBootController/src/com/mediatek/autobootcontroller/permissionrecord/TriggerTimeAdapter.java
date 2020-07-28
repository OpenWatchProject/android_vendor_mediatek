package com.mediatek.autobootcontroller;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Locale;

import android.content.Context;
import android.view.ContextThemeWrapper;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

public class TriggerTimeAdapter extends BaseAdapter {
    private List<Long> mTriggerTimes;
    private LayoutInflater mInflater;
    private SimpleDateFormat mDateFormat;
    private Context mContext;

    public TriggerTimeAdapter(Context context, List<Long> times) {
        mTriggerTimes = times;
        ContextThemeWrapper themeWrapper = new ContextThemeWrapper(context,
                com.android.internal.R.style.Theme_Material_Settings);
        mInflater = (LayoutInflater) themeWrapper
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        mContext = context;
        setTimeFormat();
    }

    public void setTimeListAndUpdate(List<Long> times) {
        setTimeFormat();

        mTriggerTimes = times;
        notifyDataSetChanged();
    }

    private void setTimeFormat() {
        String type = android.provider.Settings.System.getString(
                mContext.getContentResolver(),
                android.provider.Settings.System.TIME_12_24);
        String format = "yyyy.MM.dd  hh:mm:ss";
        if ("24".equals(type)) {
            format = "yyyy.MM.dd  HH:mm:ss";
        }
        mDateFormat = new SimpleDateFormat(format, Locale.getDefault());
    }

    @Override
    public int getCount() {
        return mTriggerTimes.size();
    }

    @Override
    public Long getItem(int position) {
        return mTriggerTimes.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        TextView view = (TextView) convertView;
        if (view == null) {
            view = (TextView) mInflater.inflate(
                    android.R.layout.simple_expandable_list_item_1, parent,
                    false);
        }

        Date date = new Date(getItem(position));
        view.setText(mDateFormat.format(date));
        return view;
    }
}
