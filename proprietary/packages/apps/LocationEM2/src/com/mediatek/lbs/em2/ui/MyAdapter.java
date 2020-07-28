package com.mediatek.lbs.em2.ui;

import java.io.File;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.List;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.TextView;

public class MyAdapter extends BaseAdapter {
    private LayoutInflater mInflater;
    private Bitmap mIcon1;
    private Bitmap mIcon2;
    private Bitmap mIcon3;
    private Bitmap mIcon4;
    private List<String> items;
    private List<String> paths;

    public MyAdapter(Context context, List<String> it, List<String> pa) {
        mInflater = LayoutInflater.from(context);
        items = it;
        paths = pa;
        mIcon1 = BitmapFactory.decodeResource(context.getResources(), R.drawable.home);
        mIcon2 = BitmapFactory.decodeResource(context.getResources(), R.drawable.back2);
        mIcon3 = BitmapFactory.decodeResource(context.getResources(), R.drawable.folder2);
        mIcon4 = BitmapFactory.decodeResource(context.getResources(), R.drawable.doc2);
    }

    public int getCount() {
        return items.size();
    }

    public Object getItem(int position) {
        return items.get(position);
    }

    public long getItemId(int position) {
        return position;
    }

    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder;
        if (convertView == null) {
            convertView = mInflater.inflate(R.layout.file_row, null);
            holder = new ViewHolder();
            holder.text = (TextView) convertView.findViewById(R.id.TextView_fileRow);
              holder.text2 = (TextView) convertView.findViewById(R.id.TextView_fileRow2);
              holder.icon = (ImageView) convertView.findViewById(R.id.ImageView_fileRow);

              convertView.setTag(holder);
        } else {
            holder = (ViewHolder) convertView.getTag();
        }

        File f = new File(paths.get(position).toString());
        if (items.get(position).toString().equals("Back to /")) {
            holder.text.setText("Back to /");
            holder.text2.setText("");
            holder.icon.setImageBitmap(mIcon1);
        } else if (items.get(position).toString().equals("Back to ../")) {
            holder.text.setText("Back to ../");
            holder.text2.setText("");
            holder.icon.setImageBitmap(mIcon2);
        } else {
            Calendar cal = new GregorianCalendar();
            cal.setTimeInMillis(f.lastModified());

            holder.text.setText(f.getName());
            holder.text2.setText("[" + (f.canRead() ? "R" : "") + (f.canWrite() ? "W" : "") + "] " + (f.isFile() ? (f.length() + "bytes ") : " ") +
                    cal.get(Calendar.YEAR) + "/" + cal.get(Calendar.MONTH) + "/" + cal.get(Calendar.DAY_OF_MONTH) + " " +
                    cal.get(Calendar.HOUR_OF_DAY) + ":" + cal.get(Calendar.MINUTE) + ":" + cal.get(Calendar.SECOND));
            if (f.canWrite())
                holder.text2.setTextColor(0xff00ff00);
            else if (f.canRead())
                holder.text2.setTextColor(0xffffff00);
            else
                holder.text2.setTextColor(0xffff0000);

            if (f.isDirectory()) {
                holder.icon.setImageBitmap(mIcon3);
            } else {
                holder.icon.setImageBitmap(mIcon4);
            }
        }
        return convertView;
    }

    private class ViewHolder {
        TextView text;
        TextView text2;
        ImageView icon;
    }
}
