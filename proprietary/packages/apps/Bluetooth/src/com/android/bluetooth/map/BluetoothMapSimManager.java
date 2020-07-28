
package com.android.bluetooth.map;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.PhoneNumberUtils;

import com.android.internal.telephony.TelephonyIntents;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;
//import android.telephony.SubInfoRecord;
import android.telephony.TelephonyManager;

import java.util.List;

public class BluetoothMapSimManager {

    private static final String TAG = "[MAP]BluetoothMapSimManager";

    private Context mContext;

    private int mSubCount;

    public final static long INVALID_SUBID = SubscriptionManager.INVALID_SUBSCRIPTION_ID;

    private List<SubscriptionInfo> mSubInfoList;

    private static TelephonyManager sTelephonyManager;
    private static SubscriptionManager sSubscriptionManager;

    public void init(Context context) {
        mContext = context;
        sTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        sSubscriptionManager = SubscriptionManager.from(mContext);
        mSubInfoList = sSubscriptionManager.getActiveSubscriptionInfoList();
        mSubCount = (mSubInfoList == null || mSubInfoList.isEmpty()) ? 0 : mSubInfoList.size();

        IntentFilter intentFilter =
          new IntentFilter(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        mContext.registerReceiver(mSubReceiver, intentFilter);
    }

    public void unregisterReceiver() {
        mContext.unregisterReceiver(mSubReceiver);
    }

    private BroadcastReceiver mSubReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                mSubInfoList = sSubscriptionManager.getActiveSubscriptionInfoList();
                mSubCount = (mSubInfoList == null || mSubInfoList.isEmpty()) ?
                  0 : mSubInfoList.size();
            }
        }
    };

    public int getSubCount() {
        return mSubCount;
    }

    public long getSingleSubId() {
        if (mSubInfoList != null && mSubInfoList.size() == 1) {
            return (long) (mSubInfoList.get(0).getSubscriptionId());
        }
        return INVALID_SUBID;
    }

    public List<SubscriptionInfo> getSimList() {
        return mSubInfoList;
    }

    public long getSimIdFromOriginator(String origNumber) {
        if (mSubCount < 2) {
            return getSingleSubId();
        } else {
            for (int i = 0; i < mSubInfoList.size(); i++) {
                if (PhoneNumberUtils.compareLoosely(mSubInfoList.get(i).getNumber(), origNumber)) {
                    return (long) (mSubInfoList.get(i).getSubscriptionId());
                }
            }
            return INVALID_SUBID;
        }
    }

    public static int getSubInfoNumber() {
        if (sSubscriptionManager != null) {
            List<SubscriptionInfo> activeSubInfoList =
              sSubscriptionManager.getActiveSubscriptionInfoList();
            if (activeSubInfoList != null) {
                return activeSubInfoList.size();
            }
        }
        return 0;
    }

    public static long getFristSubID() {
        if (sSubscriptionManager != null) {
            List<SubscriptionInfo> activeSubInfoList =
              sSubscriptionManager.getActiveSubscriptionInfoList();
            if (activeSubInfoList != null && activeSubInfoList.size() > 0) {
                return (long) (activeSubInfoList.get(0).getSubscriptionId());
                //return activeSubInfoList.get(0).subId;
            }
        }
        return 0;
    }

    public static String getNumberBySubID(long subId) {
        if (sTelephonyManager != null) {
            return sTelephonyManager.getLine1Number((int) subId);
        }
        return "";
    }

    public static boolean isValidSubId(long subId) {
        boolean isValid = false;
        if (sSubscriptionManager != null) {
            List<SubscriptionInfo> activeSubInfoList =
              sSubscriptionManager.getActiveSubscriptionInfoList();
            if (activeSubInfoList != null) {
                for (SubscriptionInfo subInfoRecord : activeSubInfoList) {
                    //if (subInfoRecord.subId == subId) {
                    if ((long) (subInfoRecord.getSubscriptionId()) == subId) {
                        isValid = true;
                        break;
                    }
                }
            }
        }
        return isValid;
    }
}
