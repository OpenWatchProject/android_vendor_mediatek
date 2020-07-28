package com.mediatek.mms.appservice;

import android.content.Context;
import android.util.Log;

import com.mediatek.mmsappservice.ext.DefaultMmsAppServiceSmsReceiverServiceExt;
import com.mediatek.mmsappservice.ext.IMmsAppServiceSmsReceiverServiceExt;
import com.mediatek.mmsappservice.ext.OpMmsAppServiceCustomizationFactoryBase;
import com.mediatek.mmsappservice.ext.OpMmsAppServiceCustomizationUtils;

/* jhz plugin*/
public class MmsAppServicePluginManager {
    public static final int PLUGIN_TYPE_SMS_RECEIVER_SERVICE = 0x0001;
    private static final String TAG = "MmsAppServicePluginManager";

    private static IMmsAppServiceSmsReceiverServiceExt sSmsReceiverServiceExt = null;

    public static void initPlugins(Context context) {
        OpMmsAppServiceCustomizationFactoryBase opFactory;
        opFactory = OpMmsAppServiceCustomizationUtils.getOpFactory(context);

        sSmsReceiverServiceExt = opFactory.makeOpMmsAppServiceSmsReceiverServicePluginExt();
        if (sSmsReceiverServiceExt == null) {
            Log.d(TAG, "default sMmsServiceCancelDownloadExt =" + sSmsReceiverServiceExt);
            sSmsReceiverServiceExt = new DefaultMmsAppServiceSmsReceiverServiceExt(context);
        }
    }

    public static Object getMmsPluginObject(int type) {
        Object obj = null;
        Log.d(TAG, "getMmsPlugin, type = " + type);
        switch (type) {
            case PLUGIN_TYPE_SMS_RECEIVER_SERVICE:
                obj = sSmsReceiverServiceExt;
                break;

            default:
                Log.e(TAG, "mms plugin type = " + type + " don't exist.");
                break;

        }
        return obj;
    }
}
