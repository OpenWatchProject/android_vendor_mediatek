package com.mediatek.op18.mms;

import android.content.Context;
import android.util.Log;

import com.mediatek.mms.callback.IMessageUtilsCallback;
import com.mediatek.mms.ext.DefaultOpMessagePluginExt;
import com.mediatek.mms.ext.IOpMessageListItemExt;
import com.mediatek.mms.ext.IOpMessageUtilsExt;
import com.mediatek.mms.ext.IOpMessagingNotificationExt;
import com.mediatek.mms.ext.IOpMmsMessageSenderExt;
import com.mediatek.mms.ext.IOpMmsPreferenceActivityExt;
import com.mediatek.mms.ext.IOpSettingListActivityExt;
import com.mediatek.mms.ext.IOpSmsPreferenceActivityExt;
import com.mediatek.mms.ext.IOpSmsSingleRecipientSenderExt;
import com.mediatek.mms.ext.IOpStatusBarSelectorCreatorExt;
import com.mediatek.mms.ext.IOpSubSelectActivityExt;
import com.mediatek.mms.ext.IOpComposeExt;
/**
 *  Op18MessagePluginExt. Used to get pluin instances.
 *
 */

public class  Op18MessagePluginExt extends DefaultOpMessagePluginExt {
    public static IMessageUtilsCallback sMessageUtilsCallback = null;
    private static final String TAG = "Mms/Op18MessagePluginExt";

    /**
     * Construction.
     * @param context Context
     */
    public  Op18MessagePluginExt(Context context) {
        super(context);
    }

    @Override
    public IOpSettingListActivityExt getOpSettingListActivityExt() {
        Log.d(TAG, "Op18MessagePluginExt getOpSettingListActivityExt actual instance");
        return new Op18SettingListActivityExt(mContext);
    }

    public IOpMessageUtilsExt getOpMessageUtilsExt() {
        Log.d(TAG, "getOpMessageUtilsExt");
        return new  Op18MessageUtilsExt(mContext);
    }


    public IOpMmsPreferenceActivityExt getOpMmsPreferenceActivityExt() {
        return new  Op18MmsPreferenceActivityExt(mContext);
    }


    public IOpSubSelectActivityExt getOpSubSelectActivityExt() {
        return new  Op18SubSelectActivityExt(mContext);
    }



    public IOpSmsPreferenceActivityExt getOpSmsPreferenceActivityExt() {
        return new  Op18SmsPreferenceActivityExt(mContext);
    }

    public IOpSmsSingleRecipientSenderExt getOpSmsSingleRecipientSenderExt() {
        return new  Op18SmsSingleRecipientSenderExt();
    }
    public IOpMmsMessageSenderExt getOpMmsMessageSenderExt() {
        return new  Op18MmsMessageSenderExt();
    }

    @Override
    public IOpMessageListItemExt getOpMessageListItemExt() {
        return new Op18MessageListItemExt(mContext);
    }

    @Override
    public IOpComposeExt getOpComposeExt() {
    Log.d("Op18MessagePluginExt", "call to getOpComposeExt ");
        return new Op18ComposeExt(mContext);
    }
}
