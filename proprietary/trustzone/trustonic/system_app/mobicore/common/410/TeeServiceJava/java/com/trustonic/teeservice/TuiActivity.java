/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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

package com.trustonic.teeservice;

import com.trustonic.tuiapi.TUI_Event;
import com.trustonic.tuiapi.TUI_EventType;

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
import android.util.Log;

import vendor.trustonic.tee.tui.V1_0.ITui;

public class TuiActivity extends Activity {

    private static final String TAG = TuiActivity.class.getSimpleName();

    private LinearLayout mainView;
    private PerformActionInBackground mTuiHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (!TuiCallback.isActivityCreationAllowed()) {
            finish();
        }
        Log.d(TAG, "onCreate()");
        setContentView(R.layout.activity_tui);

        mTuiHandler = new PerformActionInBackground();
        TuiCallback.setHandler(mTuiHandler);

        try {
            mainView = (LinearLayout) findViewById(R.id.tuiLayout);
            mainView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
        } catch (Exception e) {
            e.printStackTrace();
        }
        TuiCallback.setIsActivityCreated(true);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if(hasFocus) {
            Log.d(TAG, "Focus gained");
            synchronized(TuiCallback.getStartSignal()){
                TuiCallback.setIsActityAlive(true);
                TuiCallback.getStartSignal().notify();
            }
        } else {
            Log.d(TAG, "Focus lost");
            synchronized(TuiCallback.getFinishSignal()){
                TuiCallback.getFinishSignal().notify();
            }
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy()");
        super.onDestroy();

        //TuiCallback.setIsActivityCreated(false);
        TuiCallback.setIsActityAlive(false);
        synchronized(TuiCallback.getFinishSignal()){
            TuiCallback.getFinishSignal().notify();
        }
    }

    @Override
    public void onBackPressed() {
        // Cancel the TUI session when the back key is pressed during a TUI
        // session
        final TUI_Event cancel = new TUI_Event(TUI_EventType.TUI_CANCEL_EVENT);
        try {
            if(0 != ITui.getService(true).notifyReeEvent(cancel.getType())) {
                Log.e(TAG, "notifyReeEvent failed!");
            }
        } catch (android.os.RemoteException e) {
            Log.e(TAG, String.format("Exception %s", e.toString()));
            Log.e(TAG, "Cannot notify vendor server.  Ignoring failure, but TUI session won't be cancel");
        } catch (NullPointerException e) {
            // `mTeeService` may be null, if the activity failed to
            // retrieve it in the onCreate method. In this case, it
            // is not possible to notify it of a Ree Event. Just
            // ignore the error
        }
    }

    /*
     * Handler that receives messages from the TimerTask, to adapt the UI.
     */
    class PerformActionInBackground extends Handler {

        @Override
        public void handleMessage(Message msg) {
            switch(msg.what){
            case TuiCallback.CLOSE_SESSION:
                Log.d(TAG, " handle message CLOSE_SESSION");
                // Call the finish method to close the activity
                finish();
                /* Remove the animation when the activity is finished */
                overridePendingTransition(0, 0);
                break;

            default:
                Log.d(TAG, " handle unknown message");
                break;
            }
        }
    }
}
