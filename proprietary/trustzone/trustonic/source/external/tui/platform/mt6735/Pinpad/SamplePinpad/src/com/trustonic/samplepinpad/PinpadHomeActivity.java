/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

package com.trustonic.samplepinpad;


import java.util.concurrent.atomic.AtomicBoolean;
import com.trustonic.util.tLog;

import android.os.Bundle;
import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;

public class PinpadHomeActivity extends Activity {
    private static final String TAG = PinpadHomeActivity.class.getSimpleName();

    private Button              launcher;
    private PinpadTLCTask       pinpadTLCTask;
    public Context              ctxt;
    private static AtomicBoolean mActivityVisible = new AtomicBoolean();
    public static int           taskRunning;
    
    public boolean isActivityVisible(){
        return mActivityVisible.get();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        tLog.i(TAG, "onCreate() taskRunning=" + taskRunning);
        ctxt = this;
        taskRunning = 0;
        getActionBar().setTitle(R.string.home_actionbar_title);
        getActionBar().setSubtitle(R.string.home_actionbar_subtitle);
        getActionBar().setIcon(R.drawable.t_logo);
        setContentView(R.layout.activity_start_tui_pinpad);
        launcher = (Button) findViewById(R.id.home_main_button);
        launcher.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                tLog.i(TAG, "onClick() taskRunning=" + taskRunning);
                if (pinpadTLCTask != null) {
                    tLog.i(TAG, " pinpadTLCTask.isCancelled=" + pinpadTLCTask.isCancelled());
                    tLog.i(TAG, " pinpadTLCTask.getStatus()=" + pinpadTLCTask.getStatus());
                }
                if (taskRunning != 0)
                    return;
                taskRunning = 1;
                pinpadTLCTask = new PinpadTLCTask(ctxt);
                pinpadTLCTask.execute();
            }
        });
    }
    
    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        tLog.i(TAG, "onResume() taskRunning=" + taskRunning);
        if (pinpadTLCTask != null) {
            Log.i(TAG, " pinpadTLCTask.isCancelled=" + pinpadTLCTask.isCancelled());
            Log.i(TAG, " pinpadTLCTask.getStatus()=" + pinpadTLCTask.getStatus());
        }
        // Pinpad activity becomes visible
        mActivityVisible.set(true);
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        tLog.i(TAG, "onPause() taskRunning=" + taskRunning);
        if (pinpadTLCTask != null) {
            Log.i(TAG, " pinpadTLCTask.isCancelled=" + pinpadTLCTask.isCancelled());
            Log.i(TAG, " pinpadTLCTask.getStatus()=" + pinpadTLCTask.getStatus());
        }
        // Pinpad activity is no more visible
        mActivityVisible.set(false);
    }
}
