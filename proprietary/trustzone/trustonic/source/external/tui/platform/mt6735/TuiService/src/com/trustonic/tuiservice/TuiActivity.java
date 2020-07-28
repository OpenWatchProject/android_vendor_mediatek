/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.trustonic.tuiservice;

import com.trustonic.tuiapi.TUI_Event;
import com.trustonic.tuiapi.TUI_EventType;
import com.trustonic.util.tLog;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.drawable.BitmapDrawable;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.LinearLayout;

public class TuiActivity extends Activity {

    private static final String TAG = TuiActivity.class.getSimpleName();

    private LinearLayout mainView;
    private PerformActionInBackground mTuiHandler;
    // Use onWindowFocusChanged hasFocus as indicator
    // https://developer.android.com/reference/android/app/Activity.html#onWindowFocusChanged(boolean)
    private boolean isActityEverVisible = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        android.util.Log.i(TAG, BuildTag.BUILD_TAG);
        super.onCreate(savedInstanceState);
        tLog.i(TAG, "onCreate()");
        setContentView(R.layout.activity_tui);

        mTuiHandler = new PerformActionInBackground();
        TuiTlcWrapper.setHandler(mTuiHandler);

        try {
            mainView = (LinearLayout) findViewById(R.id.tuiLayout);
            mainView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
        } catch (Exception e) {
            e.printStackTrace();
        }
        TuiTlcWrapper.setIsActityAlive(true);

        // Remove below comment to test onCreate() -> onDestroy() case
        //finish();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if(hasFocus) {
            tLog.i(TAG, "Focus gained");
            TuiTlcWrapper.notifyStart();
        } else {
            tLog.i(TAG, "Focus lost");
//            tLog.d(TAG, "Focus lost");
//            synchronized(TuiTlcWrapper.getFinishSignal()){
//                TuiTlcWrapper.getFinishSignal().notify();
//            }
        }
        isActityEverVisible = true;
    }

    @Override
    protected void onResume() {
        tLog.i(TAG, "onResume()");
        super.onResume();
    }

    @Override
    protected void onPause() {
        tLog.i(TAG, "onPause()");
        super.onPause();
    }

    @Override
    protected void onStop() {
        tLog.i(TAG, "onStop()");
        super.onStop();
        // TuiActivity is not visible when onStop() called
        // which implied the window might never been created
        // and startTuiSession might still wait for StartSignal.
        tLog.i(TAG, "isActityEverVisible="+ isActityEverVisible + " isSessionOpened=" + TuiTlcWrapper.isSessionOpened());
        if (!isActityEverVisible && !TuiTlcWrapper.isSessionOpened()) {
            tLog.i(TAG, "notifyStartTuiSessionError()");
            TuiTlcWrapper.notifyStartTuiSessionError();
			finish();
        }
    }

    @Override
    protected void onDestroy() {
        tLog.i(TAG, "onDestroy()");
        super.onDestroy();
        TuiTlcWrapper.setIsActityAlive(false);
        TuiTlcWrapper.notifyFinish();
    }

    @Override
    public void onBackPressed() {
        tLog.i(TAG, "onBackPressed()");
        // Cancel the TUI session when the back key is pressed during a TUI
        // session
        final TUI_Event cancel = new TUI_Event(TUI_EventType.TUI_CANCEL_EVENT);
        if(!TuiTlcWrapper.notifyEvent(cancel.getType())) {
            tLog.e(TAG, "notifyEvent failed!");
        }
    }

    /*
     * Handler that receives messages from the TimerTask, to adapt the UI.
     */
    class PerformActionInBackground extends Handler {

        @Override
        public void handleMessage(Message msg) {
            switch(msg.what){
            case TuiTlcWrapper.CLOSE_SESSION:
                tLog.i(TAG, " handle message CLOSE_SESSION");
                // Call the finish method to close the activity
                finish();
                /* Remove the animation when the activity is finished */
                overridePendingTransition(0, 0);
                break;

            default:
                tLog.e(TAG, " handle unknown message");
                break;
            }
        }
    }
}
