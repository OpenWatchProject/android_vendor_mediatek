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
import com.trustonic.samplepinpad.BuildTag;

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

    public boolean isActivityVisible(){
        return mActivityVisible.get();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "Application build tag : " + BuildTag.BUILD_TAG);
        ctxt = this;
        getActionBar().setTitle(R.string.home_actionbar_title);
        getActionBar().setSubtitle(R.string.home_actionbar_subtitle);
        getActionBar().setIcon(R.drawable.t_logo);
        setContentView(R.layout.activity_start_tui_pinpad);
        launcher = (Button) findViewById(R.id.home_main_button);
        launcher.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                pinpadTLCTask = new PinpadTLCTask(ctxt);
                pinpadTLCTask.execute();
            }
        });
        PinpadTLCWrapper.ctxt = getApplicationContext();
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
        Log.d(TAG,"onResume!!");
        // Pinpad activity becomes visible
        mActivityVisible.set(true);
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();
        Log.d(TAG,"onPause!!");
        // Pinpad activity is no more visible
        mActivityVisible.set(false);
    }
}
