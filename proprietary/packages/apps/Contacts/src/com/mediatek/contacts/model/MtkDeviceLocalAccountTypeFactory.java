/** Copyright (C) 2014 MediaTek Inc.
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */

package com.mediatek.contacts.model;

import android.content.Context;

import com.android.contacts.model.account.AccountType;
import com.android.contacts.model.account.DeviceLocalAccountType;
import com.android.contacts.model.account.FallbackAccountType;
import com.android.contacts.util.DeviceLocalAccountTypeFactory;

import com.mediatek.contacts.model.account.GsimAccountType;
import com.mediatek.contacts.model.account.UsimAccountType;
import com.mediatek.contacts.model.account.CsimAccountType;
import com.mediatek.contacts.model.account.RuimAccountType;
import com.mediatek.contacts.model.account.LocalPhoneAccountType;
import com.mediatek.contacts.util.Log;

/**
 * Reports whether a value from RawContacts.ACCOUNT_TYPE should be considered a "Device"
 * account
 */

public class MtkDeviceLocalAccountTypeFactory implements DeviceLocalAccountTypeFactory {
    static final String TAG = "MtkDeviceLocalAccountTypeFactory";

    private Context mContext;

    public MtkDeviceLocalAccountTypeFactory(Context context) {
        mContext = context;
    }

    @Override
    public int classifyAccount(String accountType) {
        int type;
        if (null == accountType) {
            Log.e(TAG, "[classifyAccount] accountType should not be null !");
            return TYPE_OTHER;
        }

        switch (accountType) {
        case GsimAccountType.ACCOUNT_TYPE:
        case UsimAccountType.ACCOUNT_TYPE:
        case CsimAccountType.ACCOUNT_TYPE:
        case RuimAccountType.ACCOUNT_TYPE:
            type = TYPE_SIM;
            break;
        case LocalPhoneAccountType.ACCOUNT_TYPE:
            type = TYPE_DEVICE;
            break;
        default:
            type = TYPE_OTHER;
            break;
        }
        return type;
    }

    @Override
    public AccountType getAccountType(String accountType) {
        Log.i(TAG, "[getAccountType]accountType=" + accountType);
        if (null == accountType) {
            return new DeviceLocalAccountType(mContext); //for Fallback account
        }
        switch (accountType) {
        case GsimAccountType.ACCOUNT_TYPE:
            return new GsimAccountType(mContext, null);
        case UsimAccountType.ACCOUNT_TYPE:
            return new UsimAccountType(mContext, null);
        case CsimAccountType.ACCOUNT_TYPE:
            return new CsimAccountType(mContext, null);
        case RuimAccountType.ACCOUNT_TYPE:
            return new RuimAccountType(mContext, null);
        case LocalPhoneAccountType.ACCOUNT_TYPE:
            return new LocalPhoneAccountType(mContext, null);
        default:
            Log.i(TAG, accountType + " is not a device account type.");
            return new FallbackAccountType(mContext);
        }
    }
}
