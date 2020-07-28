package com.mediatek.internal.telephony.ppl;

import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;

import mediatek.telephony.MtkSmsMessage;
import vendor.mediatek.hardware.pplagent.V1_0.IPplAgent;

import java.util.ArrayList;
import java.util.List;

/**
 *@hide
 */
public class PplSmsFilterExtension extends ContextWrapper implements IPplSmsFilter {
    private static final String TAG = "PPL/PplSmsFilterExtension";
    public static final boolean USER_LOAD = TextUtils.equals(Build.TYPE, "user");

    public static final String INTENT_REMOTE_INSTRUCTION_RECEIVED =
            "com.mediatek.ppl.REMOTE_INSTRUCTION_RECEIVED";
    public static final String INSTRUCTION_KEY_TYPE = "Type";
    public static final String INSTRUCTION_KEY_FROM = "From";
    public static final String INSTRUCTION_KEY_TO = "To";
    public static final String INSTRUCTION_KEY_SIM_ID = "SimId";

    private final PplMessageManager mMessageManager;
    private final boolean mEnabled;

    private IPplAgent mAgent;

    public PplSmsFilterExtension(Context context) {
        super(context);
        Log.d(TAG, "PplSmsFilterExtension enter");
        if (!"1".equals(SystemProperties.get("ro.vendor.mtk_privacy_protection_lock"))) {
            mAgent = null;
            mMessageManager = null;
            mEnabled = false;
            return;
        }

        try {
            mAgent = IPplAgent.getService();
        } catch (Exception e) {
            Log.e(TAG, "Failed to get PPLAgent", e);
        }

        if (mAgent == null) {
            Log.e(TAG, "mAgent is null!");
            mMessageManager = null;
            mEnabled = false;
            return;
        }

        mMessageManager = new PplMessageManager(context);
        mEnabled = true;
        Log.d(TAG, "PplSmsFilterExtension exit");
    }

    private void convertArrayListToByteArray(ArrayList<Byte> in, byte[] out) {
        for(int i = 0; i < in.size() && i < out.length; i++) {
            out[i] = in.get(i);
        }
    }

    public byte[] readControlData() {
        if (mAgent == null) {
            Log.e(TAG, "[writeControlData] mAgent is null !!!");
            return null;
        }
        try {
            ArrayList<Byte> data = mAgent.readControlData();
            byte[] buff = new byte[data.size()];
            convertArrayListToByteArray(data, buff);
            return buff;
        } catch (RemoteException e) {
            e.printStackTrace();
        }
        return null;
    }

    @Override
    public boolean pplFilter(Bundle params) {
        if (!mEnabled) {
            Log.d(TAG, "pplFilter returns false: feature not enabled");
            return false;
        }

        String format = params.getString(KEY_FORMAT);
        boolean isMO = (params.getInt(KEY_SMS_TYPE) == 1);

        int subId = params.getInt(KEY_SUB_ID);
        int simId = SubscriptionManager.getSlotIndex(subId);
        Log.d(TAG, "pplFilter: subId = " + subId + ". simId = " + simId);

        Object[] messages = (Object[]) params.getSerializable(KEY_PDUS);
        String dst = null;
        String src = null;
        String content = null;

        if (messages == null) {
            content = params.getString(KEY_MSG_CONTENT);
            src = params.getString(KEY_SRC_ADDR);
            dst = params.getString(KEY_DST_ADDR);
            Log.d(TAG, "pplFilter: Read msg directly:" + stringForSecureSms(content));
        } else {
            byte[][] pdus = new byte[messages.length][];
            for (int i = 0; i < messages.length; i++) {
                pdus[i] = (byte[]) messages[i];
            }
            int pduCount = pdus.length;
            if (pduCount > 1) {
                Log.d(TAG, "pplFilter return false: ppl sms is short msg, count should <= 1 ");
                return false;
            }
            MtkSmsMessage[] msgs = new MtkSmsMessage[pduCount];
            for (int i = 0; i < pduCount; i++) {
                msgs[i] = MtkSmsMessage.createFromPdu(pdus[i], format);
            }

            Log.d(TAG, "pplFilter: pdus length " + pdus.length);
            if (msgs[0] == null) {
                Log.d(TAG, "pplFilter returns false: message is null");
                return false;
            }
            content = msgs[0].getMessageBody();
            Log.d(TAG, "pplFilter: message content is " + stringForSecureSms(content));

            src = msgs[0].getOriginatingAddress();
            dst = msgs[0].getDestinationAddress();
        }

        if (content == null) {
            Log.d(TAG, "pplFilter returns false: content is null");
            return false;
        }

        PplControlData controlData = PplControlData.buildControlData(readControlData());

        if (controlData == null || !controlData.isEnabled()) {
            Log.d(TAG, "pplFilter returns false: control data is null or ppl is not enabled");
            return false;
        }

        if (isMO) {
            Log.d(TAG, "pplFilter: dst is " + stringForSecureNumber(dst));
            if (!matchNumber(dst, controlData.TrustedNumberList)) {
                Log.d(TAG, "pplFilter returns false: MO number does not match");
                return false;
            }
        } else {
            Log.d(TAG, "pplFilter: src is " + stringForSecureNumber(src));
            if (!matchNumber(src, controlData.TrustedNumberList)) {
                Log.d(TAG, "pplFilter returns false: MT number does not match");
                return false;
            }
        }

        byte instruction = mMessageManager.getMessageType(content);
        if (instruction == PplMessageManager.Type.INVALID) {
            Log.d(TAG, "pplFilter returns false: message is not matched");
            return false;
        }
        if (isMO) {
            if (instruction == PplMessageManager.Type.LOCK_REQUEST ||
                instruction == PplMessageManager.Type.UNLOCK_REQUEST ||
                instruction == PplMessageManager.Type.RESET_PW_REQUEST ||
                instruction == PplMessageManager.Type.WIPE_REQUEST) {
                Log.d(TAG, "pplFilter returns false: ignore MO command: " + instruction);
                return false;
            }
        } else {
            if (instruction == PplMessageManager.Type.SIM_CHANGED ||
                instruction == PplMessageManager.Type.LOCK_RESPONSE ||
                instruction == PplMessageManager.Type.UNLOCK_RESPONSE ||
                instruction == PplMessageManager.Type.RESET_PW_RESPONSE ||
                instruction == PplMessageManager.Type.WIPE_STARTED ||
                instruction == PplMessageManager.Type.WIPE_COMPLETED ||
                instruction == PplMessageManager.Type.INSTRUCTION_DESCRIPTION ||
                instruction == PplMessageManager.Type.INSTRUCTION_DESCRIPTION2) {
                Log.d(TAG, "pplFilter returns false: ignore MT command: " + instruction);
                return false;
            }
        }

        Intent intent = new Intent(INTENT_REMOTE_INSTRUCTION_RECEIVED);
        intent.setClassName("com.mediatek.ppl", "com.mediatek.ppl.PplService");
        intent.putExtra(INSTRUCTION_KEY_TYPE, instruction);
        intent.putExtra(INSTRUCTION_KEY_SIM_ID, simId);

        if (isMO) {
            intent.putExtra(INSTRUCTION_KEY_TO, dst);
        } else {
            intent.putExtra(INSTRUCTION_KEY_FROM, src);
        }
        Log.d(TAG, "start PPL Service");
        startService(intent);

        return true;
    }

    private boolean matchNumber(String number, List<String> numbers) {
        if (number != null && numbers != null) {
            for (String s : numbers) {
                if (PhoneNumberUtils.compare(s, number)) {
                    return true;
                }
            }
        }
        return false;
    }

    private String stringForSecureNumber(String data) {
        String result = "";
        if (!USER_LOAD) {
            if (!TextUtils.isEmpty(data)) {
                int length = data.length();
                if (data.length() >= 11) {
                    result = "*******" + data.substring(7,length);
                }
                else if (length <= 4) {
                    result = data.replaceAll("\\w", "*");
                } else {
                    result = "***" + data.substring(3,length);
                }
            }
        }
        return result;

    }

    public static String stringForSecureSms(String data) {
        String result = "";
        if (!USER_LOAD) {
            if (!TextUtils.isEmpty(data)) {
                int length = data.length();
                if (length >= 6) {
                    result = data.substring(0,5) + "......";
                }
                else {
                    result = data.replaceAll("\\w", "*");
                }
            }
        }
        return result;
    }
}
