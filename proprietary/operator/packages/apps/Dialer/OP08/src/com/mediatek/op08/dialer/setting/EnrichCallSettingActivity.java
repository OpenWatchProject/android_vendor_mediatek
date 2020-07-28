package com.mediatek.op08.dialer.setting;

import android.os.Bundle;
import android.app.Activity;

import android.content.Context;
import android.os.AsyncTask;
import android.app.ActionBar;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;

import android.util.Log;
import android.view.MenuItem;

import com.mediatek.op08.dialer.R;

public class EnrichCallSettingActivity extends Activity {
    private static final String TAG = "EnrichCallSettingActivity";

    private EnrichCallSettingAdapter mAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");
        super.onCreate(savedInstanceState);

        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayShowTitleEnabled(true);
        setContentView(R.layout.setting_activity_layout);
        Log.d(TAG, "get Data begin");

        mAdapter = new EnrichCallSettingAdapter(this /* context */);
        RecyclerView recyclerView = findViewById(R.id.recycler_view);
        recyclerView.setLayoutManager(new LinearLayoutManager(this));
        recyclerView.setAdapter(mAdapter);
        mAdapter.refreshData();//TBD prehaps need do in other thread.
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onCreate");
        super.onResume();

    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();

  }

    @Override
    public void onBackPressed() {
        Log.d(TAG, "onBackPressed");
        super.onBackPressed();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.d(TAG, "onOptionsItemSelected, click home");
        finish();
        return super.onOptionsItemSelected(item);
    }
}
