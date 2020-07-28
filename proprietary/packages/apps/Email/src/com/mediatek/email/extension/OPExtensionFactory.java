package com.mediatek.email.extension;

import android.content.Context;

import com.android.emailcommon.Configuration;
import com.android.emailcommon.Logging;
import com.mediatek.email.ext.DefaultServerProviderExt;
import com.mediatek.email.ext.IServerProviderExt;
import com.mediatek.email.ext.DefaultSendNotification;
import com.mediatek.email.ext.ISendNotification;
import com.mediatek.email.ext.OpEmailCustomizationFactoryBase;


/**
 * Factory for producing each operator extensions
 */
public class OPExtensionFactory {
    private static final String TAG = "OPExtensionFactory";

    private static ISendNotification sSendNotification;
    private static IServerProviderExt sProviderExtension;

    /**
     * Clear all plugin objects which instance has already existed.
     * Because the context of plugin objects will updated in some case, such as
     * locale configuration changed,etc.
     */
    public static void resetAllPluginObject(Context context) {
        sSendNotification = null;
        sProviderExtension = null;
        getSendingNotifyExtension(context);
        getProviderExtension(context);
    }

    /**
     * The "Sending Notification" Extension is an Single instance. it would hold the ApplicationContext, and
     * alive within the whole Application.
     * @param context PluginManager use it to retrieve the plug-in object
     * @return the single instance of "Sending Notification" Extension
     */
    public synchronized static ISendNotification getSendingNotifyExtension(Context context) {
        if (Configuration.isTest()) {
            sSendNotification = new DefaultSendNotification();
        } else {
            sSendNotification = OpEmailCustomizationFactoryBase.getOpFactory(context)
                                    .makeSendNotification(context);
            Logging.d(TAG, "use SendNotification plugin");
            if (sSendNotification == null) {
                Logging.d(TAG, "get plugin failed, use default");
                sSendNotification = new DefaultSendNotification();
            }
        }
        return sSendNotification;
    }

    /**
     * The Provider Extension is an Single instance. it would hold the ApplicationContext, and
     * alive within the whole Application.
     * @param context PluginManager use it to retrieve the plug-in object
     * @return the single instance of Lunar Extension
     */
    public synchronized static IServerProviderExt getProviderExtension(Context context) {
        // if is running test, just return DefaultServerProviderExtension to
        // avoid test fail,
        // because this extension feature will change set up flow of email.
        if (Configuration.isTest()) {
            sProviderExtension = new DefaultServerProviderExt();
        } else {
            // if is not running test, return extension operator object.
            sProviderExtension = OpEmailCustomizationFactoryBase.getOpFactory(context)
                                    .makeServerProvider(context);
            Logging.d(TAG, "use email esp plugin");
            if (sProviderExtension == null) {
                Logging.d(TAG, "get plugin failed, use default");
                sProviderExtension = new DefaultServerProviderExt();
            }
        }
        return sProviderExtension;
    }
}
