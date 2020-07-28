package com.orangelabs.rcs.service;

import com.gsma.services.rcs.Intents;

import com.android.internal.telephony.PhoneConstants;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.logger.Logger;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncTask;

/**
 * Device boot event receiver: automatically starts the RCS service
 *
 * @author jexa7410
 */
public class DataSimChanged extends BroadcastReceiver {
    private static BroadcastReceiver coreServiceStateChangeListener = null;
    private final static int RCS_CORE_STOPPED = 3;
    private static Logger logger = Logger.getLogger(DataSimChanged.class.getSimpleName());
    private final static int RCS_CORE_NOT_LOADED = 10;

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (action == null
                || !action.equals("android.intent.action.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED"))
            return;

        LauncherUtils.stopRcsService(context);

        if (logger.isActivated())
            logger.debug("Data Sim Changed "+ RcsCoreService.CURRENT_STATE);

        //RcsUaAdapter uaAdapter = RcsUaAdapter.getInstance();
        //uaAdapter.sendSimOperator();

        if (RcsCoreService.CURRENT_STATE != RCS_CORE_NOT_LOADED
                && RcsCoreService.CURRENT_STATE != RCS_CORE_STOPPED) {
            AsyncTask.execute(new Runnable() {
                public void run() {
                    registerSimModuleStateListener(AndroidFactory.getApplicationContext());
                }
            });
        } else {
            LauncherUtils.launchRcsService(AndroidFactory.getApplicationContext(), true, false);
        }
    }

    /**
     * Register the broadcast receiver for SIM state
     */
    protected static void registerSimModuleStateListener(Context context) {

        // Check if network state listener is already registered
        if (coreServiceStateChangeListener != null) {
            if (logger.isActivated()) {
                logger.debug(" Core Service State change listener already registered");
            }
            return;
        }

        if (logger.isActivated()) {
            logger.debug("Registering Core service State listener");
        }

        // Instantiate the network state listener
        coreServiceStateChangeListener = new BroadcastReceiver() {
            @Override
            public void onReceive(final Context context, final Intent intent) {
                int labelEnum = intent.getIntExtra("label_enum", 0);
                if (RCS_CORE_STOPPED==labelEnum) {
                  //this.simState=IccCard.State.ABSENT;
                    Thread t = new Thread() {
                        public void run() {
                            if (logger.isActivated()) {
                                logger.debug("Core Service is Stopped - Received broadcast: "
                                        + intent.toString());
                            }
                            LauncherUtils.launchRcsService(AndroidFactory.getApplicationContext(), true, false);
                           // AndroidFactory.getApplicationContext().connectionEvent(ConnectivityManager.CONNECTIVITY_ACTION);

                        }
                    };
                    t.start();
                    unregisterSimModuleStateListener(AndroidFactory.getApplicationContext());
                }
               // IccCardContants.INTENT_KEY_ICC_STATE = IccCardConstants.INTENT_VALUE_ICC_LOADED

            }
        };

        // Register network state listener
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(Intents.Client.ACTION_VIEW_SETTINGS);
        context.getApplicationContext().registerReceiver(coreServiceStateChangeListener, intentFilter);
    }

    /**
     * Unregister the broadcast receiver for network state
     */
    public static void unregisterSimModuleStateListener(Context context) {
        if (coreServiceStateChangeListener != null) {
            if (logger.isActivated()) {
                logger.debug("Unregistering Core Service State listener");
            }

            try {
                context.getApplicationContext().unregisterReceiver(coreServiceStateChangeListener);
            } catch (IllegalArgumentException e) {
                // Nothing to do
            }
            coreServiceStateChangeListener = null;
        }
    }
}
