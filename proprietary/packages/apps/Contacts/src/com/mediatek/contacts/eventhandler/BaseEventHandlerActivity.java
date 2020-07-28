package com.mediatek.contacts.eventhandler;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

import com.android.contacts.activities.TransactionSafeActivity;

import com.mediatek.contacts.util.Log;

/**
 * this class used to handle base event for Activity.such as phb state change,
 * plug out/sd card.offer the default action for response base event.
 */
public class BaseEventHandlerActivity extends TransactionSafeActivity implements GeneralEventHandler.Listener {
    private static String TAG = "BaseEventHandleActivity";

    @Override
    protected void onCreate(Bundle savedState) {
        super.onCreate(savedState);
        Log.i(TAG, "[onCreate]");
        GeneralEventHandler.getInstance(this).register(this);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "[onDestroy]");
        GeneralEventHandler.getInstance(this).unRegister(this);
    }

    @Override
    public void onReceiveEvent(String eventType, Intent extraData) {
        Log.d(TAG, "[onReceiveEvent] eventType: " + eventType + ", extraData: " +
                extraData.toString());
        if ((GeneralEventHandler.EventType.PHB_STATE_CHANGE_EVENT.equals(eventType)
                || GeneralEventHandler.EventType.SD_STATE_CHANGE_EVENT.equals(eventType))
                && (!isFinishing())) {
            Log.i(TAG, "[onReceiveEvent] Phb and sd event,default action is finish!");
            finish();
        }
    }
}
