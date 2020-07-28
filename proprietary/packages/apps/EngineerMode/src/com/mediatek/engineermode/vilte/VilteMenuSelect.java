package com.mediatek.engineermode.vilte;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;


public class VilteMenuSelect extends Activity implements AdapterView.OnItemClickListener {
    private final String TAG = "Vilte/MenuSelect";
    ArrayList<String> items = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.vilte_menu_select);

        ListView simTypeListView = (ListView) findViewById(R.id.vilte_main_menu_select);

        items = new ArrayList<String>();
        items.add(getString(R.string.vilte_menu_common));
        items.add(getString(R.string.vilte_menu_operator));

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, items);
        simTypeListView.setAdapter(adapter);
        simTypeListView.setOnItemClickListener(this);
    }

    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        Intent intent = new Intent();
        Elog.d(TAG, "Click item = " + items.get(position));
        if (items.get(position).equals(getString(R.string.vilte_menu_common))) {
            intent.setClass(this, VilteMenuCommon.class);
        } else if (items.get(position).equals(getString(R.string.vilte_menu_operator))) {
            intent.setClass(this, VilteMenuOperator.class);
        }
        this.startActivity(intent);
    }
}