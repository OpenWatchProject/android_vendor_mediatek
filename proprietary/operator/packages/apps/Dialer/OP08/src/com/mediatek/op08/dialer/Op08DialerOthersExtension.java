package com.mediatek.op08.dialer;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.mediatek.op08.dialer.setting.Op08DialerSettingExt;
import com.mediatek.op08.dialer.setting.SettingConstant;
import com.mediatek.dialer.ext.DefaultDialerOthersExtension;
import com.mediatek.dialer.ext.IDialerSettingExt;
import java.util.HashMap;

import com.mediatek.ims.internal.MtkImsManager;
import com.android.ims.ImsManager;
import android.net.Uri;
import android.telecom.PhoneAccount;
import android.telephony.SubscriptionManager;
import com.android.ims.ImsException;




public class Op08DialerOthersExtension extends DefaultDialerOthersExtension {
    private static final String TAG = "Op08DialerOthersExtension";

    public static Context mPluginContext;

    private static final int CALL_MODE_RCS = 3;
    public Op08DialerOthersExtension(Context context) {
        mPluginContext = context;
    }

    @Override
    public IDialerSettingExt getDialerSettingExt(Context hostContext) {
        Log.d(TAG, "getDialerSettingExt Op08DialerSettingExt");
        return new Op08DialerSettingExt(mPluginContext, hostContext);
    }

    //For OP08 enrich call
    @Override
    public void setPrecallInfo(Context context, Intent intent) {
        Log.i(TAG, "enrichcall, setPrecallInfo " + intent);
        HashMap<String, String> info = SettingConstant.getEnrichCallSettingInfo(context);
        Uri uri = intent.getData();
        String address = null;
        if (uri != null && PhoneAccount.SCHEME_TEL.equals(uri.getScheme())) {
           address = uri.getSchemeSpecificPart();     //get phone number for address
        }
        setPrecallInfoToIms(context,info,address);
        return;
    }

    private void setPrecallInfoToIms(Context context, HashMap<String, String> info, String address) {
        Log.i(TAG, "enrichcall, setPrecallInfoToIms enter ");
        if (info == null) {
            Log.i(TAG, "info == null, normal call ! ");
            return;
        }
        String priority = info.get(SettingConstant.HASHMAP_PRIORITY_KEY);
        String subject = info.get(SettingConstant.HASHMAP_SUBJECT_KEY);
        String picture = info.get(SettingConstant.HASHMAP_CELLINFO_KEY);
        String location = info.get(SettingConstant.HASHMAP_LOCATION_KEY);
        Log.i(TAG, "precall info =  priority: " + priority + " subject: " + subject
                  + " picture: " + picture + " location: " + location + " address = " + address);
        int phoneId = SubscriptionManager.getDefaultVoicePhoneId();
        MtkImsManager imsManager = (MtkImsManager)ImsManager.getInstance(context,phoneId);
        HashMap<String, String>  extraHeader = new HashMap<String, String>();
        if (priority == null && subject == null && picture == null) {
            Log.i(TAG, "priority, subject, picture are all null");
            extraHeader = null;
        }
        if (priority != null) {
            extraHeader.put(SettingConstant.HASHMAP_PRIORITY_KEY, priority);
        }
        if (subject != null) {
            extraHeader.put(SettingConstant.HASHMAP_SUBJECT_KEY, subject);
        }
        if (picture != null) {
            extraHeader.put(SettingConstant.HASHMAP_CELLINFO_KEY, picture);
        }
        String fromUri = null;
        String[] loc = null;
        if (location != null) {
            loc = location.split(",");
        }
        try {
            imsManager.setImsPreCallInfo(CALL_MODE_RCS, address, fromUri, extraHeader, loc);
        } catch (ImsException e) {
            Log.e(TAG, "setImsPreCallInfo fail, throws ImsException !");
        }
    }
}
