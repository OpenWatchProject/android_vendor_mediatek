package com.mediatek.gnssdebugreport;

import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import com.mediatek.gnssdebugreport.DebugDataReport;
import com.mediatek.gnssdebugreport.IDebugReportCallback;
import com.mediatek.gnssdebugreport.IGnssDebugReportService;

public class GnssDebugReportServiceStub extends IGnssDebugReportService.Stub implements
IGnssDebugReportService, MnldConn.OnResponseListener {

    private static final String TAG = "GnssDebugReportServiceStub";
    private static final String TAG_TYPE = "type";
    private static final String TAG_TYPE_DEBUG = "debuginfo";
    private static final String TAG_TYPE_STATE = "stateinfo";
    private static final String DEBUG_DATA_HEAD = "$PMTK";
    private static final int DEBUG_HEAD_PRE_LENGTH = 5;
    private static final String DEBUG_DATA_HEAD1 = "$PMTK840";
    private static final String DEBUG_DATA_HEAD2 = "$PMTK841";
    private static final int DEBUG_DATA_HEAD_LENGTH = 9;
    private static final int DEBUG_DATA_ACK_LENGTH = 4;
    private static final int DEBUG_DATA_ID_839 = 839;
    private static final int DEBUG_DATA_ID_840 = 840;
    private static final int DEBUG_DATA_ID_841 = 841;
    private static final String DEBUG_DATA_TAIL = "*";
    private static final String DEBUG_DATA_SPLIT = ",";
    private DebugDataReport mDebugDataReport = null;
    private MnldConn mMnldConn = new MnldConn(this);
    private final RemoteCallbackList<IDebugReportCallback> mCbList = new RemoteCallbackList<IDebugReportCallback>();
    enum DebugDataType {
        Type_None,
        Type_839,
        Type_840,
        Type_841
    };

    @Override
    public IBinder asBinder() {
        return this;
    }

    @Override
    public synchronized boolean startDebug(IDebugReportCallback callback) {
        if (mCbList != null) {
            int oriRegisterNum = mCbList.getRegisteredCallbackCount();
            registerDebugReportCallback(callback);
            int newRigsteNum = mCbList.getRegisteredCallbackCount();
            Log.i(TAG, "startDebug ori:" + oriRegisterNum + " new:" + newRigsteNum);
            if ((oriRegisterNum == 0) && (newRigsteNum == 1)) {
                Log.i(TAG, "startDebug");
                mMnldConn.startDebug();
            }
            return true;
        } else {
            return false;
        }
    }

    @Override
    public synchronized void addListener(IDebugReportCallback callback) {
        registerDebugReportCallback(callback);
    }

    private boolean registerDebugReportCallback(IDebugReportCallback callback) {
        if (callback != null) {
            return mCbList.register(callback);
        } else {
            return false;
        }


    }

    private boolean unregisterDebugReportCallback(IDebugReportCallback callback) {
        if (callback != null) {
            return mCbList.unregister(callback);
        } else {
            return false;
        }
    }

    @Override
    public synchronized boolean stopDebug(IDebugReportCallback callback) {
        if (mCbList != null) {
            int oriRegisterNum = mCbList.getRegisteredCallbackCount();
            unregisterDebugReportCallback(callback);
            int newRigsteNum = mCbList.getRegisteredCallbackCount();
            Log.i(TAG, "stopDebug ori:" + oriRegisterNum + " new:" + newRigsteNum);
            if ((oriRegisterNum == 1) && (newRigsteNum == 0)) {
                Log.i(TAG, "stopDebug");
                mMnldConn.stopDebug();
            }
            return true;
        } else {
            return false;
        }

    }

    @Override
    public synchronized void removeListener(IDebugReportCallback callback) {
        unregisterDebugReportCallback(callback);
    }

    private Bundle parseDebugData(String response) {
        //Extract data body
        DebugDataType debugDataType = DebugDataType.Type_None;
        int startPos = response.indexOf(DEBUG_DATA_HEAD);
        if (startPos == -1) {
            return null;
        }
        response = response.substring(startPos);
        DebugDataParser debugDataParser = null;
        if (response.length() >= DEBUG_DATA_HEAD_LENGTH) {
            String strHead = response.substring(DEBUG_HEAD_PRE_LENGTH, DEBUG_DATA_HEAD_LENGTH - 1);
            try {
                int headID = Integer.parseInt(strHead);
                switch (headID) {
                case DEBUG_DATA_ID_839: {
                    debugDataParser = new DebugData839Parser();
                    break;
                }
                case DEBUG_DATA_ID_840: {
                    debugDataParser = new DebugData840Parser();
                    break;
                }
                case DEBUG_DATA_ID_841: {
                    debugDataParser = new DebugData841Parser();
                    break;
                }
                default:
                    break;
                }
            } catch (NumberFormatException e) {
                return null;
            }

            int endPos = response.indexOf(DEBUG_DATA_TAIL);
            if (endPos != -1) {
                response = response.substring(DEBUG_DATA_HEAD_LENGTH + DEBUG_DATA_ACK_LENGTH, endPos);
                if ((response == null) || (response.isEmpty())) {
                    return null;
                }
                if (debugDataParser != null) {
                    Bundle bundle = debugDataParser.parseData(response);
                    if (bundle != null) {
                        return bundle;
                    }
                }
            }

        }


    
        return null;

    }

    public void onDebugInfoResp(String response){
        Bundle bundle = parseDebugData(response);
        if (bundle == null) {
            return;
        }
        int number = mCbList.beginBroadcast();
        for (int k = 0; k < number; k++) {
            try {
                mCbList.getBroadcastItem(k).onDebugReportAvailable(bundle);
            } catch (RemoteException e) {
                e.printStackTrace();
            }

        }
        mCbList.finishBroadcast();
    }



}
