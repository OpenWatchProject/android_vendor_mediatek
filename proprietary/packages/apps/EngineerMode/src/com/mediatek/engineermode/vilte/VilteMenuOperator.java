package com.mediatek.engineermode.vilte;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;


public class VilteMenuOperator extends Activity implements AdapterView.OnItemClickListener {
    private final String TAG = this.getClass().getSimpleName();
    ArrayList<String> items = null;
    private MenuListAdapter mAdapter;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.vilte_menu_select);

        ListView simTypeListView = (ListView) findViewById(R.id.vilte_main_menu_select);

        items = new ArrayList<String>();
        items.add(getString(R.string.vilte_menu_operator_ims_framework));
        items.add(getString(R.string.vilte_menu_operator_vt_service));
        items.add(getString(R.string.vilte_menu_operator_media));
        items.add(getString(R.string.vilte_menu_operator_codec));
        items.add(getString(R.string.vilte_menu_operator_modem));

        mAdapter = new MenuListAdapter(this);
        simTypeListView.setAdapter(mAdapter);

        simTypeListView.setOnItemClickListener(this);
        updateListView();
    }

    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        Intent intent = new Intent();
        Elog.d(TAG, "Click item = " + items.get(position));
        if (items.get(position).equals(getString(R.string.vilte_menu_operator_ims_framework))) {
            intent.setClass(this, VilteImsFramework.class);
        } else if (items.get(position).equals(getString(R.string.vilte_menu_operator_vt_service))) {
            intent.setClass(this, ViLTEVtService.class);
        } else if (items.get(position).equals(getString(R.string.vilte_menu_operator_media))) {
            intent.setClass(this, VilteMenuMedia.class);
        } else if (items.get(position).equals(getString(R.string.vilte_menu_operator_codec))) {
            intent.setClass(this, VilteMenuCodec.class);
        } else if (items.get(position).equals(getString(R.string.vilte_menu_operator_modem))) {
            return;
        }
        this.startActivity(intent);
    }

    @TargetApi(Build.VERSION_CODES.HONEYCOMB)
    private void updateListView() {
        mAdapter.clear();
        mAdapter.addAll(items);
        mAdapter.notifyDataSetChanged();
    }

    private class MenuListAdapter extends ArrayAdapter<String> {
        MenuListAdapter(Context activity) {
            super(activity, 0);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            LayoutInflater inflater = VilteMenuOperator.this.getLayoutInflater();
            if (convertView == null) {
                convertView = inflater.inflate(R.layout.vilte_menu_operator_entry, null);
                holder = new ViewHolder();
                holder.label = (TextView) convertView.findViewById(R.id.column1);
                convertView.setTag(holder);
            } else {
                holder = (ViewHolder) convertView.getTag();
            }
            String testItem = getItem(position);
            holder.label.setText(testItem.toString());
            holder.label.setTextColor(Color.LTGRAY);
            if (testItem.equals(getString(R.string.vilte_menu_operator_modem))) {
                convertView.setEnabled(false);
                holder.label.setTextColor(Color.GRAY);
            }else{
                holder.label.setTextColor(Color.BLACK);
            }
            return convertView;
        }

        private class ViewHolder {
            public TextView label;
        }
    }
}