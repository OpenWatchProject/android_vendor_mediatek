package com.mediatek.engineermode.misc;

import com.mediatek.engineermode.R;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.List;

public class PresenceActivity extends Activity implements OnItemClickListener {
    private static final String TAG = "Presence/Activity";
    private static final String PACKAGE_NAME = "com.mediatek.engineermode.misc";
    private static final String ITEM_STRINGS[] = {"Presence Setting", "AOSP Presence Test"};
    private static final String ITEM_INTENT_STRING[] = {"PresenceSet", "AospPresenceTestActivity"};
    private ListView mMenuListView;
    private List<String> mListData;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.presence);
        mMenuListView = (ListView) findViewById(R.id.ListViewPresence);
        if (mMenuListView == null) {
            Log.e(TAG, "Resource could not be allocated");
        }
        mMenuListView.setOnItemClickListener(this);
        Log.i(TAG, "onCreate in dsActivity");
    }

    @Override
    protected void onResume() {
        super.onResume();
        mListData = getData();
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1, mListData);
        mMenuListView.setAdapter(adapter);
    }

    public void onItemClick(AdapterView<?> arg0, View view, int menuId, long arg3) {
        int i = 0;
        Intent intent = new Intent();
        for (i = 0 ; i < ITEM_STRINGS.length; i++) {
            if (ITEM_STRINGS[i] == mListData.get(menuId)) {
                intent.setClassName(this, PACKAGE_NAME + "." + ITEM_INTENT_STRING[i]);
                Log.i(TAG, "Start activity:" +
                    ITEM_STRINGS[i] + " inent:" + ITEM_INTENT_STRING[i]);
                break;
            }
        }
        this.startActivity(intent);
    }
    private List<String> getData() {
        List<String> items = new ArrayList<String>();
        int i = 0;
        for (; i < ITEM_STRINGS.length; i++) {
            items.add(ITEM_STRINGS[i]);
        }
        return items;
    }
}
