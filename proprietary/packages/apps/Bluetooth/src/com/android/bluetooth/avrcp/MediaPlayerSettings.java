/*
 * Copyright 2018 The Android Open Source Project
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

package com.android.bluetooth.avrcp;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.content.Context;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;

import com.android.bluetooth.Utils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/*
 * This class provides a way to set/get shuffle/repeat mode of music apk.
 * Interacts iMusic with broadcast. Fake response for other music apk.
 *
 * TODO : Interact with Google Play Music and other music apks with common api.
 */
public class MediaPlayerSettings {
    private static final String TAG = "NewAvrcpMediaPlayerSettings";
    private static final boolean DEBUG = SystemProperties
            .get("persist.vendor.bluetooth.hostloglevel","").equals("sqc");

    private static final String BLUETOOTH_ADMIN_PERM = android.Manifest.permission.BLUETOOTH_ADMIN;
    private static final String BLUETOOTH_PERM = android.Manifest.permission.BLUETOOTH;

    private final static int MESSAGE_PLAYERSETTINGS_TIMEOUT = 602;
    private static final int INTERNAL_ERROR = 0x03;
    private static final int OPERATION_SUCCESSFUL = 0x04;

    private Context mContext;
    private Looper mLooper;
    private Handler mHandler;
    private Map<Integer, Object> mCallbacks = new HashMap<Integer, Object>();

    private AvrcpTargetService.ListCallback mPlayerSettingsCb;

    //Intents for PlayerApplication Settings Feature
    private static final String PLAYERSETTINGS_REQUEST =
            "org.codeaurora.music.playersettingsrequest";
    private static final String PLAYERSETTINGS_RESPONSE =
           "org.codeaurora.music.playersettingsresponse";

    private class PlayerSettings {
        public byte attr;
        public byte [] attrIds;
        public String path;
    };

    private PlayerSettings mPlayerSettings = new PlayerSettings();
    private class localPlayerSettings {
        public byte eq_value = 0x01;
        public byte repeat_value = 0x01;
        public byte shuffle_value = 0x01;
        public byte scan_value = 0x01;
    };

    private static final String [] valTextRepeate =
            new String []{"Off", "Single track", "All tracks"};
    private static final String [] valTextShuffle = new String []{"Off", "All tracks"};
    private static final byte [] valTextRepeateIds = new byte []{1, 2, 3};
    private static final byte [] valTextShuffleIds = new byte []{1, 2};
    private localPlayerSettings settingValues = new localPlayerSettings();
    private static final String COMMAND = "command";
    private static final String CMDGET = "get";
    private static final String CMDSET = "set";
    private static final String EXTRA_GET_COMMAND = "commandExtra";
    private static final String EXTRA_GET_RESPONSE = "Response";

    private static final int GET_ATTRIBUTE_IDS = 0;
    private static final int GET_VALUE_IDS = 1;
    private static final int GET_ATTRIBUTE_TEXT = 2;
    private static final int GET_VALUE_TEXT     = 3;
    private static final int GET_ATTRIBUTE_VALUES = 4;
    private static final int NOTIFY_ATTRIBUTE_VALUES = 5;
    private static final int SET_ATTRIBUTE_VALUES  = 6;
    private static final int GET_INVALID = 0xff;

    private static final String EXTRA_ATTRIBUTE_ID = "Attribute";
    private static final String EXTRA_VALUE_STRING_ARRAY = "ValueStrings";
    private static final String EXTRA_ATTRIB_VALUE_PAIRS = "AttribValuePairs";
    private static final String EXTRA_ATTRIBUTE_STRING_ARRAY = "AttributeStrings";
    private static final String EXTRA_VALUE_ID_ARRAY = "Values";
    private static final String EXTRA_ATTIBUTE_ID_ARRAY = "Attributes";

    public static final int VALUE_SHUFFLEMODE_OFF = 1;
    public static final int VALUE_SHUFFLEMODE_ALL = 2;
    public static final int VALUE_REPEATMODE_OFF = 1;
    public static final int VALUE_REPEATMODE_SINGLE = 2;
    public static final int VALUE_REPEATMODE_ALL = 3;
    public static final int VALUE_INVALID = 0;
    public static final int ATTRIBUTE_NOTSUPPORTED = -1;

    public static final int ATTRIBUTE_EQUALIZER = 1;
    public static final int ATTRIBUTE_REPEATMODE = 2;
    public static final int ATTRIBUTE_SHUFFLEMODE = 3;
    public static final int ATTRIBUTE_SCANMODE = 4;
    public static final int NUMPLAYER_ATTRIBUTE = 2;

    private byte [] def_attrib = new byte [] {ATTRIBUTE_REPEATMODE, ATTRIBUTE_SHUFFLEMODE};
    private byte [] value_repmode = new byte [] { VALUE_REPEATMODE_OFF,
                                                  VALUE_REPEATMODE_SINGLE,
                                                  VALUE_REPEATMODE_ALL};

    private byte [] value_shufmode = new byte [] { VALUE_SHUFFLEMODE_OFF,
                                                  VALUE_SHUFFLEMODE_ALL};
    private byte [] value_default = new byte [] {0};
    private final String UPDATE_ATTRIBUTES = "UpdateSupportedAttributes";
    private final String UPDATE_VALUES = "UpdateSupportedValues";
    private final String UPDATE_ATTRIB_VALUE = "UpdateCurrentValues";
    private final String UPDATE_ATTRIB_TEXT = "UpdateAttributesText";
    private final String UPDATE_VALUE_TEXT = "UpdateValuesText";
    private ArrayList <Integer> mPendingCmds;
    private ArrayList <Integer> mPendingSetAttributes;

    /** M: Callback for player app settings Interface @{ */
    // Send player app settings update
    interface PlayerSettingsUpdateCallback {
        void run(boolean settingChanged);
    }

    // PDU ID 0x11
    interface GetListPlayerAttributeCallback {
        void run(byte attr, byte[] attrIds);
    }

    //PDU ID 0x12
    interface GetListPlayerAttributeValuesCallback {
        void run(byte numberAttr, byte[]values);
    }

    //PDU ID 0x13
    interface GetPlayerAttributeValueCallback {
        void run(byte numberAttr, byte[]attr);
    }

    //PDU ID 0x14
    interface SetPlayerAppSettingCallback {
        void run(int status);
    }

    //PDU ID 0x15
    interface GetPlayerAttributeTextCallback {
        void run(int numAttr, byte[] attr, int length, String[]text);
    }

    //PDU ID 0x16
    interface GetPlayerAttributeValueTextCallback {
        void run(int numAttr, byte[] attr, int length, String[]text);
    }

    //Get app setting change
    interface GetAppSettingChangeCallback {
        void run(byte numberAttr, byte[]attr);
    }
    /** @} */


    MediaPlayerSettings(Looper looper, Context context) {
        Log.v(TAG, "Create MediaPlayerSettings");

        mLooper = looper;
        mContext = context;

        // Register for intents where music players might be not response for request
        mPendingCmds = new ArrayList<Integer>();
        mPendingSetAttributes = new ArrayList<Integer>();
        IntentFilter playerSettingsFilter = new IntentFilter();
        playerSettingsFilter.addAction(PLAYERSETTINGS_RESPONSE);
        try {
            context.registerReceiver(mPlayerSettingsReceiver, playerSettingsFilter);
        }catch (Exception e) {
            Log.e(TAG,"Unable to register Avrcp player app settings receiver", e);
        }

        mHandler = new PlayerSettingsMessageHandler(looper);
    }

    void init(AvrcpTargetService.ListCallback callback) {
        Log.v(TAG, "Initializing MediaPlayerSettings");
        mPlayerSettingsCb = callback;
    }

    private final class PlayerSettingsMessageHandler extends Handler {
        private final static int MESSAGE_PLAYERSETTINGS_TIMEOUT = 602;

        PlayerSettingsMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg.what != MESSAGE_PLAYERSETTINGS_TIMEOUT) {
                Log.wtf(TAG, "Unknown message on timeout handler: " + msg.what);
                return;
            }

            synchronized(mPendingCmds) {
                Integer val = new Integer(msg.arg1);
                if (!mPendingCmds.contains(val)) {
                    return;
                }
                mPendingCmds.remove(val);
            }

            d("handle Message MESSAGE_PLAYERSETTINGS_TIMEOUT");
            switch (msg.arg1) {
                case GET_ATTRIBUTE_IDS:
                    d("GET_ATTRIBUTE_IDS timeout");
                    getListPlayerAttributeRsp((byte)def_attrib.length, def_attrib);
                break;
                case GET_VALUE_IDS:
                    d("GET_VALUE_IDS timeout");
                    switch (mPlayerSettings.attr) {
                        case ATTRIBUTE_REPEATMODE:
                            getListPlayerAttributeValuesRsp((byte)value_repmode.length,
                                    value_repmode);
                            break;
                        case ATTRIBUTE_SHUFFLEMODE:
                            getListPlayerAttributeValuesRsp((byte)value_shufmode.length,
                                    value_shufmode);
                            break;
                        default:
                            getListPlayerAttributeValuesRsp((byte)value_default.length,
                                    value_default);
                            break;
                    }
                break;
                case GET_ATTRIBUTE_VALUES:
                    d("GET_ATTRIBUTE_VALUES timeout");
                    int j = 0;
                    byte [] retVal = new byte [mPlayerSettings.attrIds.length*2];
                    for (int i = 0; i < mPlayerSettings.attrIds.length; i++) {
                        retVal[j++] = mPlayerSettings.attrIds[i];
                        if (mPlayerSettings.attrIds[i] == ATTRIBUTE_REPEATMODE) {
                            retVal[j++] = settingValues.repeat_value;
                        } else if (mPlayerSettings.attrIds[i] == ATTRIBUTE_SHUFFLEMODE) {
                            retVal[j++] = settingValues.shuffle_value;
                        } else {
                            retVal[j++] = 0x0;
                        }
                    }
                    getPlayerAttributeValueRsp((byte)retVal.length, retVal);
                    break;
                case SET_ATTRIBUTE_VALUES:
                    d("SET_ATTRIBUTE_VALUES timeout");
                    sendPlayerAppSettingsUpdate(true);
                    setPlayerAppSettingRsp(OPERATION_SUCCESSFUL);
                    break;
                case GET_ATTRIBUTE_TEXT:
                    d("GET_ATTRIBUTE_TEXT timeout");
                    String [] values = new String [mPlayerSettings.attrIds.length];
                    for (int i = 0; i < mPlayerSettings.attrIds.length; i++) {
                        switch (mPlayerSettings.attrIds[i]) {
                            case ATTRIBUTE_REPEATMODE:
                                values[i] = "Repeat";
                            break;
                            case ATTRIBUTE_SHUFFLEMODE:
                                values[i] = "Shuffle";
                            break;
                        }
                    }
                    getPlayerAttributeTextRsp(mPlayerSettings.attrIds.length,
                            mPlayerSettings.attrIds, values.length,values);
                    break;
                case GET_VALUE_TEXT:
                    d("GET_VALUE_TEXT timeout;msg.arg2="+msg.arg2);
                    switch (msg.arg2) {
                        case ATTRIBUTE_REPEATMODE:
                            getPlayerAttributeTextValueRsp(valTextRepeateIds.length,
                                    valTextRepeateIds, valTextRepeate.length, valTextRepeate);
                        break;
                        case ATTRIBUTE_SHUFFLEMODE:
                            getPlayerAttributeTextValueRsp(valTextShuffleIds.length,
                                    valTextShuffleIds, valTextShuffle.length, valTextShuffle);
                        break;
                    }
                    break;
                default :
                    break;
            }
        }
    }

    void cleanup() {
        d("cleanup");
        try {
            mContext.unregisterReceiver(mPlayerSettingsReceiver);
        } catch (Exception e) {
            Log.e(TAG,"Unable to unregister Avrcp receiver", e);
        }
        // clean up callback object
        mCallbacks.clear();
    }


//Listen to intents from MediaPlayer and Audio Manager and update data structures
    private BroadcastReceiver mPlayerSettingsReceiver = new BroadcastReceiver() {
    @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(PLAYERSETTINGS_RESPONSE)) {
                int getResponse = intent.getIntExtra(EXTRA_GET_RESPONSE, GET_INVALID);
                byte [] data;
                String [] text;
                boolean isSetAttrValRsp = false;
                d("PLAYERSETTINGS_RESPONSE");

                synchronized (mPendingCmds) {
                    Integer val = new Integer(getResponse);
                    if (mPendingCmds.contains(val)) {
                        if (getResponse == SET_ATTRIBUTE_VALUES) {
                            isSetAttrValRsp = true;
                            d("Response received for SET_ATTRIBUTE_VALUES");
                        }
                        mHandler.removeMessages(MESSAGE_PLAYERSETTINGS_TIMEOUT);
                        mPendingCmds.remove(val);
                    }
                }
                d("getResponse " + getResponse);
                byte numAttr = 0x00;
                switch (getResponse) {
                    case GET_ATTRIBUTE_IDS:
                        data = intent.getByteArrayExtra(EXTRA_ATTIBUTE_ID_ARRAY);
                        numAttr = (byte) data.length;
                        d("GET_ATTRIBUTE_IDS " + numAttr);
                        getListPlayerAttributeRsp(numAttr, data);
                        break;
                    case GET_VALUE_IDS:
                        data = intent.getByteArrayExtra(EXTRA_VALUE_ID_ARRAY);
                        numAttr = (byte) data.length;
                        d("GET_VALUE_IDS " + numAttr);
                        getListPlayerAttributeValuesRsp(numAttr, data);
                        break;
                    case GET_ATTRIBUTE_VALUES:
                        data = intent.getByteArrayExtra(EXTRA_ATTRIB_VALUE_PAIRS);
                        updateLocalPlayerSettings(data);
                        numAttr = (byte) data.length;
                        d("GET_ATTRIBUTE_VALUES " + numAttr);
                        int j = 0;
                        byte [] retVal = new byte [mPlayerSettings.attrIds.length*2];
                        for (int i = 0; i < mPlayerSettings.attrIds.length; i++) {
                            retVal[j++] = mPlayerSettings.attrIds[i];
                            switch (mPlayerSettings.attrIds[i]) {
                                case ATTRIBUTE_EQUALIZER:
                                    retVal[j++] = settingValues.eq_value;
                                    break;
                                case ATTRIBUTE_REPEATMODE:
                                    retVal[j++] = settingValues.repeat_value;
                                    break;
                                case ATTRIBUTE_SHUFFLEMODE:
                                    retVal[j++] = settingValues.shuffle_value;
                                    break;
                                case ATTRIBUTE_SCANMODE:
                                    retVal[j++] = settingValues.scan_value;
                                    break;
                                default:
                                    retVal[j++] = 0x00;
                                    break;
                            }
                        }
                        getPlayerAttributeValueRsp((byte)retVal.length, retVal);
                        break;
                    case SET_ATTRIBUTE_VALUES:
                        data = intent.getByteArrayExtra(EXTRA_ATTRIB_VALUE_PAIRS);
                        updateLocalPlayerSettings(data);
                        Log.v(TAG,"SET_ATTRIBUTE_VALUES: " + isSetAttrValRsp);
                        if (isSetAttrValRsp){
                            isSetAttrValRsp = false;
                            Log.v(TAG,"Respond to SET_ATTRIBUTE_VALUES request");
                            if (checkPlayerAttributeResponse(data)) {
                               setPlayerAppSettingRsp(OPERATION_SUCCESSFUL);
                            } else {
                               setPlayerAppSettingRsp(INTERNAL_ERROR);
                            }
                        }
                        d("Send Player app attribute changed update");
                        sendPlayerAppSettingsUpdate(true);
                        break;
                    case GET_ATTRIBUTE_TEXT:
                        d("GET_ATTRIBUTE_TEXT");
                        text = intent.getStringArrayExtra(EXTRA_ATTRIBUTE_STRING_ARRAY);
                        getPlayerAttributeTextRsp(mPlayerSettings.attrIds.length,
                                mPlayerSettings.attrIds, text.length, text);
                        d("mPlayerSettings.attrIds " + mPlayerSettings.attrIds.length);
                        break;
                    case GET_VALUE_TEXT:
                        d("GET_VALUE_TEXT");
                        text = intent.getStringArrayExtra(EXTRA_VALUE_STRING_ARRAY);
                        getPlayerAttributeTextValueRsp(mPlayerSettings.attrIds.length,
                                mPlayerSettings.attrIds, text.length, text);
                        break;
                    default :
                        break;
                }
            }
        }
    };

    private void sendPlayerAppSettingsUpdate(boolean playerSettings) {
        if (mPlayerSettingsCb== null) {
            return;
        }
        mPlayerSettingsCb.run(true);
        d("sendPlayerAppSettingsUpdate");
    }

    private void updateLocalPlayerSettings(byte[] data) {
        d("updateLocalPlayerSettings");
        for (int i = 0; i < data.length; i += 2) {
            d("ID: " + data[i] + " Value: " + data[i+1]);
                switch (data[i]) {
                case ATTRIBUTE_EQUALIZER:
                    settingValues.eq_value = data[i+1];
                    break;
                case ATTRIBUTE_REPEATMODE:
                    settingValues.repeat_value = data[i+1];
                    break;
                case ATTRIBUTE_SHUFFLEMODE:
                    settingValues.shuffle_value = data[i+1];
                    break;
                case ATTRIBUTE_SCANMODE:
                    settingValues.scan_value = data[i+1];
                    break;
                default :
                    break;
            }
        }
    }

    private boolean checkPlayerAttributeResponse( byte[] data) {
        boolean ret = false;
        d("checkPlayerAttributeResponse");
        for (int i = 0; i < data.length; i += 2) {
            d("ID: " + data[i] + " Value: " + data[i+1]);
            switch (data[i]) {
                case ATTRIBUTE_EQUALIZER:
                    if (mPendingSetAttributes.contains(new Integer(ATTRIBUTE_EQUALIZER))) {
                        if(data[i+1] == ATTRIBUTE_NOTSUPPORTED) {
                            ret = false;
                        } else {
                            ret = true;
                        }
                    }
                    break;
                case ATTRIBUTE_REPEATMODE:
                    if (mPendingSetAttributes.contains(new Integer(ATTRIBUTE_REPEATMODE))) {
                        if(data[i+1] == ATTRIBUTE_NOTSUPPORTED) {
                            ret = false;
                        } else {
                            ret = true;
                        }
                    }
                    break;
                case ATTRIBUTE_SHUFFLEMODE:
                    if (mPendingSetAttributes.contains(new Integer(ATTRIBUTE_SHUFFLEMODE))) {
                        if(data[i+1] == ATTRIBUTE_NOTSUPPORTED) {
                            ret = false;
                        } else {
                            ret = true;
                        }
                    }
                    break;
                default :
                    break;
            }
        }
        mPendingSetAttributes.clear();
        return ret;
    }

//PDU ID 0x11
    public void getListPlayerAttribute(GetListPlayerAttributeCallback cb) {
        d("getListPlayerAttribute");
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, GET_ATTRIBUTE_IDS);
        intent.setPackage("com.android.bbkmusic");
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);

        Message msg = mHandler.obtainMessage(MESSAGE_PLAYERSETTINGS_TIMEOUT,
                GET_ATTRIBUTE_IDS, 0);
        mPendingCmds.add(new Integer(msg.arg1));
        mCallbacks.put(GET_ATTRIBUTE_IDS, cb);
        mHandler.sendMessageDelayed(msg, 50);
    }

    private void getListPlayerAttributeRsp(byte attr, byte[] attrIds) {
        GetListPlayerAttributeCallback cb
                = (GetListPlayerAttributeCallback)mCallbacks.get(GET_ATTRIBUTE_IDS);
        if (cb != null) {
            cb.run(attr, attrIds);
            d("getListPlayerAttributeRsp");
        }
    }

//PDU ID 0x12
    public void getListPlayerAttributeValues(byte attr, GetListPlayerAttributeValuesCallback cb) {
        d("getListPlayerAttributeValues");
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, GET_VALUE_IDS);
        intent.putExtra(EXTRA_ATTRIBUTE_ID, attr);
        intent.setPackage("com.android.bbkmusic");
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        mPlayerSettings.attr = attr;
        Message msg = mHandler.obtainMessage();
        msg.what = MESSAGE_PLAYERSETTINGS_TIMEOUT;
        msg.arg1 = GET_VALUE_IDS;
        mPendingCmds.add(new Integer(msg.arg1));
        mCallbacks.put(GET_VALUE_IDS, cb);
        mHandler.sendMessageDelayed(msg, 50);
    }

    private void getListPlayerAttributeValuesRsp(byte numberAttr, byte[] values) {
        GetListPlayerAttributeValuesCallback cb
                = (GetListPlayerAttributeValuesCallback)mCallbacks.get(GET_VALUE_IDS);
        if (cb != null) {
            cb.run(numberAttr, values);
            d("getListPlayerAttributeValuesRsp");
        }
    }

//PDU ID 0x13
    public void getPlayerAttributeValue(byte attr,int[] arr, GetPlayerAttributeValueCallback cb) {
        d("getPlayerAttributeValue attr " + attr);
        int i ;
        byte[] barray = new byte[attr];
        for(i =0 ; i<attr ; ++i) {
            barray[i] = (byte)arr[i];
        }
        mPlayerSettings.attrIds = new byte [attr];
        for ( i = 0; i < attr; i++) {
            mPlayerSettings.attrIds[i] = barray[i];
        }
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, GET_ATTRIBUTE_VALUES);
        intent.putExtra(EXTRA_ATTIBUTE_ID_ARRAY, barray);
        intent.setPackage("com.android.bbkmusic");
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        Message msg = mHandler.obtainMessage();
        msg.what = MESSAGE_PLAYERSETTINGS_TIMEOUT;
        msg.arg1 = GET_ATTRIBUTE_VALUES;
        mPendingCmds.add(new Integer(msg.arg1));
        mCallbacks.put(GET_ATTRIBUTE_VALUES, cb);
        mHandler.sendMessageDelayed(msg, 50);
    }

    private void getPlayerAttributeValueRsp(byte numberAttr, byte[] attr) {
        GetPlayerAttributeValueCallback cb
                = (GetPlayerAttributeValueCallback)mCallbacks.get(GET_ATTRIBUTE_VALUES);
        if (cb != null) {
            cb.run(numberAttr, attr);
            d("getPlayerAttributeValueRsp");
        }
    }

//PDU 0x14
    public void setPlayerAppSetting( byte num, byte [] attrId, byte [] attrVal,
            SetPlayerAppSettingCallback cb) {
        d("setPlayerAppSetting num " + num);
        byte[] array = new byte[num*2];
        for ( int i = 0; i < num; i++) {
            array[i] = attrId[i];
            array[i+1] = attrVal[i];
            mPendingSetAttributes.add(new Integer(attrId[i]));
        }
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        intent.putExtra(COMMAND, CMDSET);
        intent.putExtra(EXTRA_ATTRIB_VALUE_PAIRS, array);
        intent.setPackage("com.android.bbkmusic");
        updateLocalPlayerSettings(array);
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        Message msg = mHandler.obtainMessage();
        msg.what = MESSAGE_PLAYERSETTINGS_TIMEOUT;
        msg.arg1 = SET_ATTRIBUTE_VALUES;
        mPendingCmds.add(new Integer(msg.arg1));
        mCallbacks.put(SET_ATTRIBUTE_VALUES, cb);
        mHandler.sendMessageDelayed(msg, 50);
    }

    private void setPlayerAppSettingRsp(int status) {
        SetPlayerAppSettingCallback cb
                = (SetPlayerAppSettingCallback)mCallbacks.get(SET_ATTRIBUTE_VALUES);
        if (cb != null) {
            cb.run(status);
            d("setPlayerAppSettingRsp");
        }
    }

//PDU 0x15
    public void getPlayerAttributeText(byte attr, byte [] attrIds,
            GetPlayerAttributeTextCallback cb) {
        d("getplayerattribute_text" + attr +"attrIDsNum" + attrIds.length);
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        Message msg = mHandler.obtainMessage();
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, GET_ATTRIBUTE_TEXT);
        intent.putExtra(EXTRA_ATTIBUTE_ID_ARRAY, attrIds);
        intent.setPackage("com.android.bbkmusic");
        mPlayerSettings.attrIds = new byte [attr];
        for (int i = 0; i < attr; i++) {
            mPlayerSettings.attrIds[i] = attrIds[i];
        }
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        msg.what = MESSAGE_PLAYERSETTINGS_TIMEOUT;
        msg.arg1 = GET_ATTRIBUTE_TEXT;
        mPendingCmds.add(new Integer(msg.arg1));
        mCallbacks.put(GET_ATTRIBUTE_TEXT, cb);
        mHandler.sendMessageDelayed(msg, 50);
    }

    private void getPlayerAttributeTextRsp(int num_attr, byte[] attr, int length, String[]text) {
        GetPlayerAttributeTextCallback cb
                = (GetPlayerAttributeTextCallback)mCallbacks.get(GET_ATTRIBUTE_TEXT);
        if (cb != null) {
            cb.run(num_attr, attr, length, text);
            d("getPlayerAttributeTextRsp");
        }
    }

//PDU 0x16
     public void getPlayerAttributeTextValue(byte attr_id, byte numValue, byte [] value,
            GetPlayerAttributeValueTextCallback cb) {
        d("getplayervalue_text id" + attr_id +"numValue" + numValue
                +"value.lenght" + value.length);
        Intent intent = new Intent(PLAYERSETTINGS_REQUEST);
        Message msg = mHandler.obtainMessage();
        intent.putExtra(COMMAND, CMDGET);
        intent.putExtra(EXTRA_GET_COMMAND, GET_VALUE_TEXT);
        intent.putExtra(EXTRA_ATTRIBUTE_ID, attr_id);
        intent.putExtra(EXTRA_VALUE_ID_ARRAY, value);
        intent.setPackage("com.android.bbkmusic");
        mPlayerSettings.attrIds = new byte [numValue];
        for (int i = 0; i < numValue; i++) {
            mPlayerSettings.attrIds[i] = value[i];
        }
        mContext.sendBroadcast(intent, BLUETOOTH_PERM);
        msg.what = MESSAGE_PLAYERSETTINGS_TIMEOUT;
        msg.arg1 = GET_VALUE_TEXT;
        msg.arg2 = attr_id;
        mPendingCmds.add(new Integer(msg.arg1));
        mCallbacks.put(GET_VALUE_TEXT, cb);
        mHandler.sendMessageDelayed(msg, 50);
    }

    private void getPlayerAttributeTextValueRsp(int num_attr, byte[] attr, int length,
            String[]text) {
        GetPlayerAttributeValueTextCallback cb
                = (GetPlayerAttributeValueTextCallback)mCallbacks.get(GET_VALUE_TEXT);
        if (cb != null) {
            cb.run(num_attr, attr, length, text);
            d("getPlayerAttributeTextValueRsp");
        } else
        d("getPlayerAttributeTextValueRsp cb null");
    }

//Get app setting change
    public void getAppSettingChange(GetAppSettingChangeCallback cb) {
        d("getAppSettingChange");
        int j = 0;
        byte i = NUMPLAYER_ATTRIBUTE*2;
        byte [] retVal = new byte [i];
        retVal[j++] = ATTRIBUTE_REPEATMODE;
        retVal[j++] = settingValues.repeat_value;
        retVal[j++] = ATTRIBUTE_SHUFFLEMODE;
        retVal[j++] = settingValues.shuffle_value;
        cb.run(i, retVal);
    }

    private static void d(String message) {
        if (DEBUG) {
            Log.d(TAG, message);
        }
    }
}

