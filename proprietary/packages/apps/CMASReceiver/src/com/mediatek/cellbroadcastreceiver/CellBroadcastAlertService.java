/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.cellbroadcastreceiver;

import android.app.KeyguardManager;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.telephony.CellBroadcastMessage;
import android.telephony.SmsCbCmasInfo;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbMessage;
import android.telephony.SmsManager;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.cellbroadcastreceiver.CellBroadcastContentProvider.CMASCellBroadcasts;
import com.mediatek.cmas.ext.ICmasDuplicateMessageExt;
import com.mediatek.cmas.ext.ICmasMainSettingsExt;
import com.mediatek.cmas.ext.ICmasMessageInitiationExt;
import com.mediatek.internal.telephony.gsm.cbutil.Shape;
import com.mediatek.internal.telephony.gsm.cbutil.Vertex;
import com.mediatek.internal.telephony.gsm.cbutil.WhamTuple;
import com.mediatek.internal.telephony.gsm.cbutil.Polygon;
import com.mediatek.internal.telephony.gsm.cbutil.Circle;
import com.mediatek.internal.telephony.MtkSmsCbMessage;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

import mediatek.telephony.MtkSmsManager;
import mediatek.telephony.MtkCellBroadcastMessage;


/**
 * This service manages the display and animation of broadcast messages.
 * Emergency messages display with a flashing animated exclamation mark icon,
 * and an alert tone is played when the alert is first shown to the user
 * (but not when the user views a previously received broadcast).
 */
public class CellBroadcastAlertService extends Service {
    private static final String TAG = "[CMAS]CellBroadcastAlertService";

    /** Intent action to display alert dialog/notification, after verifying the alert is new. */
    static final String SHOW_NEW_ALERT_ACTION = "cellbroadcastreceiver.SHOW_NEW_ALERT";

    public static final String CB_ROWID = "msg_rowid";

    private static final String PREF_NAME = "com.mediatek.cellbroadcastreceiver_preferences";

    /** Use the same notification ID for non-emergency alerts. */
    static final int NOTIFICATION_ID = 1;

    private static Comparer sCompareList = new Comparer();

    private static final int MSG_OVER_12HOURS = 301;
    private static final int MSG_HANDLE_WHAM_MESSAGE = 701;

//    final private InternalHandler mTimeoutHandler;
//    static HandlerThread handlerThread = null;

    @Override
    public void onCreate() {
        // Temporarily removed for this duplicate message track down.
        Log.v(TAG, "onCreate");
        // Start up the thread running the service.  Note that we create a
        // separate thread because the service normally runs in the process's
        // main thread, which we don't want to block.
//        if (handlerThread == null) {
//            handlerThread = new HandlerThread(TAG, Process.THREAD_PRIORITY_BACKGROUND);
//            handlerThread.start();
//        }
//        //mServiceLooper = thread.getLooper();
//        mTimeoutHandler = new InternalHandler(handlerThread.getLooper());
        mLocationManager = (LocationManager) this.getSystemService(Context.LOCATION_SERVICE);
    }

    /**
     * internal handler for events.
     */
    private Handler mTimeoutHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_OVER_12HOURS:
                CellBroadcastMessage cbmsg = (CellBroadcastMessage) msg.obj;
                Log.d(TAG, "Before delete message, msgid = " + cbmsg.getServiceCategory() +
                    " sn = " + cbmsg.getSerialNumber() + " subid = " + cbmsg.getSubId());
                MtkSmsManager smsManager = MtkSmsManager
                        .getSmsManagerForSubscriptionId(cbmsg.getSubId());
                smsManager.removeCellBroadcastMsg(cbmsg.getServiceCategory(),
                    cbmsg.getSerialNumber());
                break;
            case MSG_HANDLE_WHAM_MESSAGE:
                Log.d(TAG, "handle wham message");
                Intent triggerMessageIntent = (Intent) msg.obj;
                handleCellBroadcast(triggerMessageIntent);
                break;
            case MSG_LOCATION_TIMEOUT:// cancel location and show message
                Log.d(TAG, "handle get location time out");
                cancelNetworkGeoLocationRequest();
                if (mGeofencingPendingIntent.size() <= 0) {
                    Log.d(TAG, "Geofencing PendingIntent is empty, return");
                    return;
                }
                for (Intent intent : mGeofencingPendingIntent) {
                    //need mark checked
                    intent.putExtra("checklocationflag", true);
                    handleCellBroadcast(intent);//update this message
                }
                mGeofencingPendingIntent.clear();
                break;
            case EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE:
                Location location = (Location) msg.obj;
                Log.d(TAG, "handle get location response location: " + location);
                if (mGeofencingPendingIntent.size() <= 0) {
                    Log.d(TAG, "Geofencing PendingIntent is empty, return");
                    return;
                }
                for (Intent intent : mGeofencingPendingIntent) {
                    Bundle extras = intent.getExtras();
                    if (extras == null) {
                        Log.e(TAG, "handle location response with no extras!");
                        return;
                    }
                    MtkCellBroadcastMessage cbMsg  =
                        (MtkCellBroadcastMessage) extras.get("triggermessage");
                    if (cbMsg == null) {
                        SmsCbMessage message = (SmsCbMessage) extras.get("message");
                        if (message == null) {
                            Log.e(TAG, "HANDLE  LOCATION RESPONSE with no message extra");
                            return;
                        }
                        cbMsg = new MtkCellBroadcastMessage(message);
                    } else {
                        Log.d(TAG, "handle location response with triggermessage extra!");
                    }

                    final MtkCellBroadcastMessage cbm = cbMsg;

                    ArrayList<Shape> shapeList = cbm.getWacResult();
                    if (location == null || geofencingCheck(location, shapeList)) {
                        //need mark checked
                        intent.putExtra("checklocationflag", true);
                        handleCellBroadcast(intent);//show this message
                    } else {
                        new CellBroadcastContentProvider
                          .AsyncCellBroadcastTask(getContentResolver())
                        .execute(new CellBroadcastContentProvider.CellBroadcastOperation() {
                                @Override
                            public boolean execute(CellBroadcastContentProvider provider) {
                                long rowId = provider.addMessageAsNotDisplayed(cbm);
//                                store a time to delete not display message for boot complete.
                                Message msg = mTimeoutHandler.
                                    obtainMessage(MSG_REMOVE_NOT_DISPLAYED_MESSAGE);
                                msg.arg1 = (int) rowId;
                                mTimeoutHandler.sendMessageDelayed(msg, 86400000);//86400000:24hours
                                return true;
                            }
                        });
                    }
                }
                mGeofencingPendingIntent.clear();
                break;
            case MSG_REMOVE_NOT_DISPLAYED_MESSAGE: //remove 24hours timeout not displayed message
                Log.d(TAG, "handle remove not displayed message");
                final long rowId = msg.arg1;
                new CellBroadcastContentProvider.AsyncCellBroadcastTask(getContentResolver())
                .execute(new CellBroadcastContentProvider.CellBroadcastOperation() {
                    @Override
                    public boolean execute(CellBroadcastContentProvider provider) {
                        provider.deleteBroadcast(rowId);
                        return true;
                    }
                });
                break;
            default:
                break;
            }
        }
    };

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String action = intent.getAction();
        if (Telephony.Sms.Intents.SMS_EMERGENCY_CB_RECEIVED_ACTION.equals(action) ||
                Telephony.Sms.Intents.SMS_CB_RECEIVED_ACTION.equals(action)) {
            handleCellBroadcast(intent);
        } else if (Intent.ACTION_AIRPLANE_MODE_CHANGED.equals(action)) {
            sCompareList.clear();
        } else {
            Log.e(TAG, "Unrecognized intent action: " + action);
        }
        return START_NOT_STICKY;
    }

    private void handleGeofencingTrigger(CellBroadcastMessage triggerCbm) {
        MtkCellBroadcastMessage triggerMessage = null;
        if (triggerCbm instanceof MtkCellBroadcastMessage) {
            triggerMessage = (MtkCellBroadcastMessage) triggerCbm;
        } else {
            Log.d(TAG, " handleGeofencingTrigger not instanceof MtkCellBroadcastMessage");
            return;
        }
        final ArrayList<ArrayList<WhamTuple>> whamList = triggerMessage.getWHAMTupleList();
        new CellBroadcastContentProvider.AsyncCellBroadcastTask(getContentResolver())
        .execute(new CellBroadcastContentProvider.CellBroadcastOperation() {
            @Override
            public boolean execute(CellBroadcastContentProvider provider) {
                Set<Integer> messageIds = new HashSet<Integer>();
                Set<Integer> serialNumbers = new HashSet<Integer>();

                for (int i = 0; i < whamList.size(); i++) {
                    ArrayList<WhamTuple> whamTupleList = whamList.get(i);
                    for (WhamTuple whamTuple : whamTupleList) {
                        messageIds.add(whamTuple.mMsgId);
                        serialNumbers.add(whamTuple.mSerialNumber);
                    }
                }

                String messageIdIn = queryMessageAndFormatIn(messageIds);
                String serialNumberIn = queryMessageAndFormatIn(serialNumbers);

                String queryStr = Telephony.CellBroadcasts.SERVICE_CATEGORY + messageIdIn
                    + " AND " + Telephony.CellBroadcasts.SERIAL_NUMBER + serialNumberIn
                    + " AND " + CMASCellBroadcasts.CMAS_DISPLAYED + "=" + 2;

                Cursor triggerMessageCursor = provider.
                            getCellBroadcastCursorByTriggerMessage(queryStr);
                if (triggerMessageCursor != null) {
                    try {
                        if (triggerMessageCursor.moveToFirst()) {
                            do {
                                MtkCellBroadcastMessage cbmsg = MtkCellBroadcastMessage.
                                    createFromCursor(triggerMessageCursor);
                                Intent intent = new Intent();
                                intent.putExtra("triggermessage", cbmsg);
                                mTimeoutHandler.obtainMessage(MSG_HANDLE_WHAM_MESSAGE,
                                    intent).sendToTarget();
                            } while(triggerMessageCursor.moveToNext());
                        }
                    } finally {
                        triggerMessageCursor.close();
                    }
                }
                return true;
            }
        });
    }

    private String queryMessageAndFormatIn(Set<Integer> ids){
        String in = " IN ";
        in += ids.toString();
        in = in.replace('[', '(');
        in = in.replace(']', ')');
        Log.d(TAG, "queryMessageAndFormatIn, In = " + in);
        return in;
    }

    private void handleCellBroadcast(Intent intent) {
        Log.d(TAG, "handleCellBroadcast " + intent);
        Bundle extras = intent.getExtras();
        if (extras == null) {
            Log.e(TAG, "received SMS_CB_RECEIVED_ACTION with no extras!");
            return;
        }
        CellBroadcastMessage cbMessage  = (MtkCellBroadcastMessage) extras.get("triggermessage");
        if (cbMessage == null) {
            SmsCbMessage message = (SmsCbMessage) extras.get("message");
            if (message == null) {
                Log.e(TAG, "received SMS_CB_RECEIVED_ACTION with no message extra");
                return;
            }
            if (message instanceof MtkSmsCbMessage) {
                cbMessage = new MtkCellBroadcastMessage(message);
            } else {
                cbMessage = new CellBroadcastMessage(message);
            }
        }

        final CellBroadcastMessage cbm = cbMessage;
        if (cbm.getServiceCategory() == 4400) {
            handleGeofencingTrigger(cbm);
            return;
        }

        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, 1);
        Log.d(TAG, "handleCellBroadcast subId = " + subId);
        if (SubscriptionManager.isValidSubscriptionId(subId)) {
            cbm.setSubId(subId);
        } else {
            Log.e(TAG, "Invalid subscription id");
        }
        if (!isMessageEnabledByUser(cbm)) {
            Log.d(TAG, "ignoring alert of type " + cbm.getServiceCategory() +
                                 " by user preference");
            return;
        }

        ICmasMainSettingsExt optrMainSetting = (ICmasMainSettingsExt)
                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS);

        if (optrMainSetting.needBlockMessageInEcbm()){
            Log.d(TAG, "ignoring EmergencyCallbackMode of type " + cbm.getServiceCategory());
            return;
        }

        boolean spanishAlerts = optrMainSetting.isSpanishAlert(this, cbm.getLanguageCode(),
            cbm.getServiceCategory(), subId);
        if (!spanishAlerts) {
            Log.d(TAG, "ignoring spanish alert of type " + cbm.getServiceCategory() +
            " by user preference");
            return;
        }

        //geofencing check
        if (getGeofencingCheckResult(cbm, intent)) {
            return;
        }

        final Intent alertIntent = new Intent(SHOW_NEW_ALERT_ACTION);
        alertIntent.setClass(this, CMASPresentationService.class);
        alertIntent.putExtra("message", cbm);

        // write to database on a background thread
        new CellBroadcastContentProvider.AsyncCellBroadcastTask(getContentResolver())
                .execute(new CellBroadcastContentProvider.CellBroadcastOperation() {
                    @Override
                    public boolean execute(CellBroadcastContentProvider provider) {
                        int res = ICmasDuplicateMessageExt.NEW_CMAS_PROCESS;

                        ICmasDuplicateMessageExt optDetection = (ICmasDuplicateMessageExt)
                             CellBroadcastPluginManager.getCellBroadcastPluginObject(
                             CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_DUPLICATE_MESSAGE);
                        // Log.d(TAG, "execute, get ICmasDuplicateMessageExt " + optDetection);
                        if (optDetection != null && !cbm.isEtwsMessage()) {
                            res = optDetection.handleDuplicateMessage(provider, cbm);
                            Log.d(TAG, "res = " + res);
                        }

                        if (!sCompareList.add(cbm) &&
                            res != ICmasDuplicateMessageExt.OVER_12_HOURS_IDENTIFY_MSG &&
                            res != ICmasDuplicateMessageExt.PRESENT_CMAS_PROCESS) {
                            // reject the duplicate cmas in non power-off & non off-airplane
                            Log.d(TAG, "ignoring duplicate alert with " + cbm);
                            return false;
                        }

                        switch (res) {
                        case ICmasDuplicateMessageExt.OVER_12_HOURS_IDENTIFY_MSG:
                            //update db and show msg to user
                            long rowNewId = provider.getRowId(cbm);
                            Log.i(TAG, "rowId = " + rowNewId);

                            if (rowNewId > -1) {
                                provider.updateOldBroadcast(cbm, rowNewId);
                                alertIntent.putExtra(CB_ROWID, rowNewId);
                                startService(alertIntent);
                                //43200000:12 hours
                                mTimeoutHandler.sendMessageDelayed(mTimeoutHandler.obtainMessage(
                                        MSG_OVER_12HOURS, cbm), Comparer.REGARD_AS_NEW_TIMEOUT);
                            }

                            return true;
                        case ICmasDuplicateMessageExt.NEW_CMAS_PROCESS: // NewCMASProcess
                            // handle update msg
                            if (!handleUpdatedCB(provider, cbm)) {
                                return true;
                            }
                            Log.d(TAG, "before insertNewBroadcast, sn " + cbm.getSerialNumber());
                            //if (provider.insertNewBroadcast(cbm)) {
                            ICmasMessageInitiationExt msgInit = (ICmasMessageInitiationExt)
                            CellBroadcastPluginManager.getCellBroadcastPluginObject(
                            CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION);
                            if (msgInit != null) {
                                msgInit.setExpirationTime(CellBroadcastAlertService.this, cbm);
                            }
                            long rowId = provider.addNewBroadcast(cbm, subId);
                            if (rowId > -1) {
                                alertIntent.putExtra(CB_ROWID, rowId);
                                startService(alertIntent);
                                ICmasDuplicateMessageExt optDetect = (ICmasDuplicateMessageExt)
                                CellBroadcastPluginManager.getCellBroadcastPluginObject(
                                    CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_DUPLICATE_MESSAGE);
                                Log.d(TAG, "Delete message to receive again host: " +
                                        Comparer.REGARD_AS_NEW_TIMEOUT);
                                optDetect.sendDelayedMsgToDelete(mTimeoutHandler, cbm, MSG_OVER_12HOURS);

                                /*if it OP08, send delay message to delete the msg
                                if ((optDetect != null) && (optDetect.sendDelayedMsgToDelete())) {
                                    mTimeoutHandler.sendMessageDelayed(mTimeoutHandler.obtainMessage(
                                            MSG_OVER_12HOURS, cbm.getServiceCategory(), cbm.getSerialNumber()), Comparer.REGARD_AS_NEW_TIMEOUT); //12 hours
                                }
                                */

                                return true;
                            }
                            return false;
                        case ICmasDuplicateMessageExt.PRESENT_CMAS_PROCESS: // PresentCMASProcess
                            long id = provider.getRowId(cbm);
                            Log.d(TAG, "id = " + id);
                            if (id > -1) {
                                alertIntent.putExtra(CB_ROWID, id);
                            }
                            startService(alertIntent);
                            return true;
                        case ICmasDuplicateMessageExt.DISCARD_CMAS_PROCESS: // DiscardCMASProcess
                        default:
                            return true;
                        }
                    }
                });
    }

    private boolean handleUpdatedCB(CellBroadcastContentProvider provider
                                                  , CellBroadcastMessage cbm) {
        Cursor c = provider.getAllCellBroadcastCursor();
        if (c != null
           && c.getCount() > 0) {

        Log.d(TAG, "handleUpdatedCB Cursor not null");
        //Comparer oldList = Comparer.createFromCursor(c);

        String oldIDstr = null;
        if (!c.isClosed()) {
        Log.d(TAG, "handleUpdatedCB isClosed");
          oldIDstr = sCompareList.searchUpdatedCBFromCache(c, cbm);
        }
        if (oldIDstr != null) {
            Log.d(TAG, "Checking oldIDstr" + oldIDstr);
            if (oldIDstr.equals(Comparer.INVALID_UPDATE_CB)) {
                Log.d(TAG, "handleUpdatedCB " + oldIDstr);
                return false;
            }

            if (!provider.deleteBroadcast(Integer.valueOf(oldIDstr).intValue())) {
                Log.d(TAG, "error handleUpdateCB,failed to delete ID " + oldIDstr);
            }
        }
       }

        return true;
    }

    /**
     * Filter out broadcasts on the test channels that the user has not enabled,
     * and types of notifications that the user is not interested in receiving.
     * This allows us to enable an entire range of message identifiers in the
     * radio and not have to explicitly disable the message identifiers for
     * test broadcasts. In the unlikely event that the default shared preference
     * values were not initialized in CellBroadcastReceiverApp, the second parameter
     * to the getBoolean() calls match the default values in res/xml/preferences.xml.
     *
     * @param message the message to check
     * @return true if the user has enabled this message type; false otherwise
     */
    private boolean isMessageEnabledByUser(CellBroadcastMessage message) {
        Log.d(TAG, "isMessageEnabledByUser " + message.getServiceCategory());
        if (message.isEtwsMessage()) {
            return true;
        }
        int slotIndex = SubscriptionManager.getSlotIndex(message.getSubId());
        String sKey = CMASUtils.getSlotKey(slotIndex);
        if (message.isCmasMessage()) {
            Log.d(TAG, "in isMessageEnabledByUser , CMASMessageClass " +
                message.getCmasMessageClass() + " sKey = " + sKey);
            boolean retValue = true;
            switch (message.getCmasMessageClass()) {
                case SmsCbCmasInfo.CMAS_CLASS_PRESIDENTIAL_LEVEL_ALERT:
                    Log.d(TAG, "in isMessageEnabledByUser , president level");
                    if (CmasConfigManager.isTwProfile() &&
                            (message.getServiceCategory() == 919 ||
                             message.getServiceCategory() == 911)) {
                        retValue = PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
                                CheckBoxAndSettingsPreference.KEY_ENABLE_ALERT_MESSAGE, true);
                        Log.d(TAG, "isMessageEnabledByUser: value of 919/911 = " + retValue);
                    }
                    break;
                case SmsCbCmasInfo.CMAS_CLASS_EXTREME_THREAT:
                    retValue = CmasConfigManager.isExtremEnable(getApplicationContext(), sKey);
                    Log.d(TAG, "in isMessageEnabledByUser , extreme setting " + retValue);
                    break;
                case SmsCbCmasInfo.CMAS_CLASS_SEVERE_THREAT:
                    retValue = CmasConfigManager.isSevereEnable(getApplicationContext(), sKey);
                    Log.d(TAG, "in isMessageEnabledByUser , server threat setting " + retValue);
                    break;
                case SmsCbCmasInfo.CMAS_CLASS_CHILD_ABDUCTION_EMERGENCY:
                    retValue = CmasConfigManager.isAmberEnable(getApplicationContext(), sKey);
                    Log.d(TAG, "in isMessageEnabledByUser , child abduction setting " + retValue);
                    break;
                case SmsCbCmasInfo.CMAS_CLASS_REQUIRED_MONTHLY_TEST:
                    retValue = CmasConfigManager.isRmtEnable(getApplicationContext(), sKey);
                    Log.d(TAG, "in isMessageEnabledByUser , RMT setting " + retValue);
                    break;
                case SmsCbCmasInfo.CMAS_CLASS_CMAS_EXERCISE:
                    retValue = CmasConfigManager.isExerciseEnable(getApplicationContext(), sKey);
                    Log.d(TAG, "in isMessageEnabledByUser , EXER setting " + retValue);
                    break;
                case SmsCbCmasInfo.CMAS_CLASS_OPERATOR_DEFINED_USE:
                    retValue = PreferenceManager.getDefaultSharedPreferences(this).getBoolean(
                        sKey + CellBroadcastConfigService.ENABLE_CMAS_OPERATOR_CHOICE_SUPPORT,
                        false);
                    Log.d(TAG, "in isMessageEnabledByUser,OPERATOR_DEFINED setting " + retValue);
                    break;
                default:
                    break;
            }
            ICmasMainSettingsExt mainSettingPlugin = (ICmasMainSettingsExt)
            CellBroadcastPluginManager.getCellBroadcastPluginObject(
            CellBroadcastPluginManager.CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS);
            retValue = mainSettingPlugin.isMessageEnabledByUser(message.getCmasMessageClass(),
                                retValue, sKey);
            return retValue;
        }

        return false;    // other broadcast messages are always enabled
    }

    static Intent createDisplayMessageIntent(Context context, Class intentClass,
            ArrayList<CellBroadcastMessage> messageList) {
        // Trigger the list activity to fire up a dialog that shows the received messages
        Intent intent = new Intent(context, intentClass);
        intent.putParcelableArrayListExtra(CellBroadcastMessage.SMS_CB_MESSAGE_EXTRA, messageList);
        return intent;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;    // clients can't bind to this service
    }

    // geofencing

    private LocationManager mLocationManager;
    private static final int NETWORK_LOCATION_UPDATE_TIME = 1000;
    private static final int GPS_LOCATION_UPDATE_TIME = 1000;

    private static final int MSG_LOCATION_TIMEOUT = 501;
    private static final int MSG_REMOVE_NOT_DISPLAYED_MESSAGE = 601;
    private ArrayList <Intent> mGeofencingPendingIntent = new ArrayList<Intent>();
    private int requestGeoLocationFlag = 0;//0,idle;1,doing request

    /* Event Message definition for handler */
    private static final int EVENT_MSG_REQUEST_NETWORK_LOCATION = 1;
    private static final int EVENT_MSG_REQUEST_LAST_KNOW_LOCATION = 2;
    private static final int EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE = 3;

    private boolean getGeofencingCheckResult(CellBroadcastMessage cbMessage, Intent intent) {
        MtkCellBroadcastMessage cbm = null;
        if (cbMessage instanceof MtkCellBroadcastMessage) {
            cbm = (MtkCellBroadcastMessage) cbMessage;
        } else {
            Log.d(TAG, " getGeofencingCheckResult not instanceof MtkCellBroadcastMessage");
            return false;
        }
        if (!mLocationManager.isLocationEnabled()) {
             Log.d(TAG, " getGeofencingCheckResult isLocationEnabled=false ");
            return false;
        }
        //true means already checked
        boolean checkLocationFlag = intent.getBooleanExtra("checklocationflag", false);
        ArrayList<Shape> shapeList = cbm.getWacResult();
        if (shapeList == null || shapeList.size() <= 0 || checkLocationFlag) {
            return false;
        }
        long waitTimer = cbm.getMaxWaitTime();
        if (waitTimer == 0) { //get cached location
            Location location = getLastKnownLocation();
            if (location != null) {
                if (geofencingCheck(location, shapeList)) {
                    return false;//inside
                } else {
                  //out side and store this message as not displayed
                     // write to database on a background thread
                    final MtkCellBroadcastMessage cbMsg = cbm;
                    new CellBroadcastContentProvider.AsyncCellBroadcastTask(getContentResolver())
                    .execute(new CellBroadcastContentProvider.CellBroadcastOperation() {
                            @Override
                        public boolean execute(CellBroadcastContentProvider provider) {
                            long rowId = provider.addMessageAsNotDisplayed(cbMsg);

                            Message msg = mTimeoutHandler.obtainMessage(
                                                  MSG_REMOVE_NOT_DISPLAYED_MESSAGE);
                            msg.arg1 = (int) rowId;
                            mTimeoutHandler.sendMessageDelayed(msg, 86400000);//86400000:24 hours
                            return true;
                        }
                    });
                    return true;
                }
            } else {//location = null need show message
                return false;
            }
        } else {// use timer to get device location
            mGeofencingPendingIntent.add(intent);
            //start location timer
            if (requestGeoLocationFlag == 0) {//0,idle;1,doing request,do not need to request again

                requestGeoLocationFlag = 1;
                mTimeoutHandler.sendMessageDelayed(mTimeoutHandler.obtainMessage(
                    MSG_LOCATION_TIMEOUT), waitTimer * 1000);
                requestGeoLocationFromGpsLocation();
                requestGeoLocationFromNetworkLocation();
            }
            return true;
        }
    }

    //    public static final int TYPE_CIRCLE = 1;
    //    public static final int TYPE_POLYGON = 2;
    private boolean geofencingCheck(Location location, ArrayList<Shape> shapeList) {
        Vertex vertexCoordinate = new Vertex(location.getLatitude(), location.getLongitude());
        for (Shape shape : shapeList) {
            if (shape.mType == shape.TYPE_CIRCLE) {
                //unit：KM
                Circle circle = (Circle)shape;
                double distance = CMASUtils.getDistance(vertexCoordinate.mLati,
                    vertexCoordinate.mLongi, circle.mVertex.mLati, circle.mVertex.mLongi);
                if (distance <= circle.mRadius) {
                    return true;
                }
            } else if (shape.mType == shape.TYPE_POLYGON) {
                Polygon polygon = (Polygon)shape;
                if (CMASUtils.PtInPolygon(vertexCoordinate, polygon.mVertices)) {
                    return true;
                }
            }
        }
        return false;
    }

    private LocationListener mLocationListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {
            Log.d(TAG, "onLocationChanged: " + location);

            cancelNetworkGeoLocationRequest();
            //get call back info and remove location timer
            mTimeoutHandler.removeMessages(MSG_LOCATION_TIMEOUT);
            mTimeoutHandler.obtainMessage(EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE,
                location).sendToTarget();
        }

        @Override
        public void onProviderDisabled(String provider) {
            Log.d(TAG, "onProviderDisabled: " + provider);
        }

        @Override
        public void onProviderEnabled(String provider) {
            Log.d(TAG, "onProviderEnabled: " + provider);
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            Log.d(TAG, "onStatusChanged: " + provider + ", status=" + status);
        }
    };

    private Location getLastKnownLocation() {
        Log.d(TAG, "getLastKnownLocation");

        if (mLocationManager == null) {
            Log.d(TAG, "getLastKnownLocation: empty locationManager, return");
            return null;
        }

        // Get cached GPS location
        Location gpsLocation =
            mLocationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);

        if (gpsLocation != null) {
            Log.d(TAG, "GPS location: " + gpsLocation);
            return gpsLocation;
        }

        // Get cached network location
        Location networkLocation =
            mLocationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);

        if (networkLocation != null) {
            Log.d(TAG, "Network location: " + networkLocation);
            return networkLocation;
        }

        Log.d(TAG, "getLastKnownLocation: no last known location");
        return null;
    }

    private boolean requestDeviceLocation() {
        Log.d(TAG, "requestDeviceLocation");
        if (mLocationManager == null) {
            Log.d(TAG, "requestDeviceLocation: empty locationManager, return");
            return false;
        }
        Criteria localCriteria = new Criteria();
        localCriteria.setPowerRequirement(3);
        localCriteria.setAccuracy(1);

        mLocationManager.requestLocationUpdates(1000L, 0.0F, localCriteria,
                                mLocationListener, Looper.myLooper());
        Log.d(TAG, "requestDeviceLocation: request Location update");
        return true;
    }

    private boolean requestGeoLocationFromGpsLocation() {
        Log.d(TAG, "requestGeoLocationFromGpsLocation");
        if (mLocationManager == null) {
            Log.d(TAG, "requestGeoLocationFromGpsLocation: empty locationManager, return");
            return false;
        }

        // Avoid "provider doesn't exist: network" JE
        if (mLocationManager.getProvider(LocationManager.GPS_PROVIDER) == null) {
            Log.d(TAG, "GeoLocationFromGpsLocation: provider doesn't exist or not ready yet");
            return false;
        }

        LocationRequest request = LocationRequest.createFromDeprecatedProvider(
                LocationManager.GPS_PROVIDER, GPS_LOCATION_UPDATE_TIME /*minTime*/,
                0 /*minDistance*/, false/*oneShot*/);
        request.setHideFromAppOps(true);
        mLocationManager.requestLocationUpdates(request, mLocationListener, Looper.myLooper());
        Log.d(TAG, "requestGeoLocationFromGpsLocation: request Location update");

        return true;
    }

    private boolean requestGeoLocationFromNetworkLocation() {
        Log.d(TAG, "requestGeoLocationFromNetworkLocation");
        if (mLocationManager == null) {
            Log.d(TAG, "getGeoLocationFromNetworkLocation: empty locationManager, return");
            return false;
        }

        // Avoid "provider doesn't exist: network" JE
        if (mLocationManager.getProvider(LocationManager.NETWORK_PROVIDER) == null) {
            Log.d(TAG, "GeoLocationFromNetworkLocation: provider doesn't exist or not ready yet");
            return false;
        }

        LocationRequest request = LocationRequest.createFromDeprecatedProvider(
                LocationManager.NETWORK_PROVIDER, NETWORK_LOCATION_UPDATE_TIME /*minTime*/,
                0 /*minDistance*/, false/*oneShot*/);
        request.setHideFromAppOps(true);
        mLocationManager.requestLocationUpdates(request, mLocationListener, Looper.myLooper());
        Log.d(TAG, "requestGeoLocationFromNetworkLocation: request networkLocation update");

        return true;
    }

    private void cancelNetworkGeoLocationRequest() {
        Log.d(TAG, "cancelNetworkGeoLocationRequest");
        requestGeoLocationFlag = 0;
        if (mLocationManager == null) {
            Log.d(TAG, "cancelNetworkGeoLocationRequest: empty locationManager, return");
            return;
        }
        mLocationManager.removeUpdates(mLocationListener);
    }

}
