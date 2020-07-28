package com.mediatek.hdmi;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.UEventObserver;
import android.util.Log;

public class HdmiObserver extends UEventObserver {
    private static final String TAG = "HdmiObserver";

    private static final String HDMI_UEVENT_MATCH = "DEVPATH=/devices/virtual/switch/hdmi";
    private static final String HDMI_STATE_PATH = "/sys/class/switch/hdmi/state";
    private static final String HDMI_NAME_PATH = "/sys/class/switch/hdmi/name";

    // Monitor OTG and notify HDMI
    private static final int MSG_HDMI = 1;

    private static HdmiObserver mHdmiObserver;

    private Handler mHandler;

    public static HdmiObserver getHdmiObserver() {
        if (mHdmiObserver == null) {
            mHdmiObserver = new HdmiObserver();
        }
        return mHdmiObserver;
    }

    HdmiObserver() {
        init();
    }

    /**
     * start observing hdmi cable plugged event
     */
    public void startObserve() {
        startObserving(HDMI_UEVENT_MATCH);
    }

    /**
     * stop observing hdmi cable plugged event
     */
    public void stopObserve() {
        stopObserving();
    }

    @Override
    public void onUEvent(UEventObserver.UEvent event) {
        Log.i(TAG, "HdmiObserver: onUEvent: " + event.toString());
        int state = 0;
        try {
            state = Integer.parseInt(event.get("SWITCH_STATE"));
        } catch (NumberFormatException e) {
            Log.w(TAG, "HdmiObserver: Could not parse switch state from event " + event);
        }
        if (mHandler != null) {
            Log.i(TAG, "HdmiObserver: onUEventstate : " + state);
            mHandler.sendMessage(mHandler.obtainMessage(MSG_HDMI, state, 0));
        }
    }

    private void init() {

        int newState = 0;
        try {
            newState = Integer.parseInt(getContentFromFile(HDMI_STATE_PATH));
        } catch (NumberFormatException e) {
            Log.w(TAG, "HDMI state fail");
        }
        if (mHandler != null) {
            Log.i(TAG, "HdmiObserver: init state : " + newState);
            mHandler.sendMessage(mHandler.obtainMessage(MSG_HDMI, newState));
        }
    }

    private String getContentFromFile(String filePath) {
        char[] buffer = new char[1024];
        FileReader reader = null;
        String content = null;
        try {
            reader = new FileReader(filePath);
            int len = reader.read(buffer, 0, buffer.length);
            content = String.valueOf(buffer, 0, len).trim();
            Log.i(TAG, filePath + " content is " + content);
        } catch (FileNotFoundException e) {
            Log.w(TAG, "can't find file " + filePath);
        } catch (IOException e) {
            Log.w(TAG, "IO exception when read file " + filePath);
        } catch (IndexOutOfBoundsException e) {
            Log.w(TAG, "index exception: " + e.getMessage());
        } finally {
            if (null != reader) {
                try {
                    reader.close();
                } catch (IOException e) {
                    Log.w(TAG, "close reader fail: " + e.getMessage());
                }
            }
        }
        return content;
    }

    public void setHandler(Handler handler) {
        mHandler = handler;
    }
}
