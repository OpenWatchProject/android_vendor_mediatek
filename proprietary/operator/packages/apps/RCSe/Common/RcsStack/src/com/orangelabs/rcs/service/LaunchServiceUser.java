package com.orangelabs.rcs.service;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;
import com.orangelabs.rcs.utils.logger.Logger;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
 * Device boot event receiver: automatically starts the RCS service
 *
 * @author jexa7410
 */
public class LaunchServiceUser extends BroadcastReceiver {
    private static Logger logger = Logger.getLogger(LaunchServiceUser.class.getSimpleName());

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (!"com.mediatek.intent.rcs.stack.LaunchService".equals(action)) {
            if (logger.isActivated())
                logger.debug("not support action:" + action);
            return;
        }

        if (logger.isActivated())
            logger.debug("Rcs started by user");

        if (RcsSettings.getInstance() == null) {
            RcsSettings.createInstance(context);
        }

        RcsSettings.getInstance().setServiceActivationState(true);
        UaServiceManager.getInstance().publishRcsCap();
        //RcsUaAdapter.getInstance().sendRcsActivation();
        //RcsUaAdapter.getInstance().sendRegAddCapabilties();
        //RcsUaAdapter.getInstance().sendRegDeregister();
        LauncherUtils.launchRcsService(context, false, true);
    }
}
