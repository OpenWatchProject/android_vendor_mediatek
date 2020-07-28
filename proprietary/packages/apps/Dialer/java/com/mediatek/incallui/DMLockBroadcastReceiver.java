package com.mediatek.incallui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.RemoteException;
import android.widget.Toast;

import com.android.incallui.call.DialerCall;
import com.android.incallui.call.CallList;
import com.android.incallui.call.state.DialerCallState;
import com.android.incallui.InCallPresenter;
import com.android.incallui.Log;
import com.android.incallui.R;

import java.util.NoSuchElementException;

import vendor.mediatek.hardware.pplagent.V1_0.IPplAgent;

public class DMLockBroadcastReceiver extends BroadcastReceiver {
    public static DMLockBroadcastReceiver sDMLockBroadcastReceiver;

    private DMLockBroadcastReceiver(Context context) {
        statusBarHelper = new StatusBarHelper(context);
    }

    // Only one this receiver can be registered/unregisted
    public synchronized static DMLockBroadcastReceiver getInstance(Context context) {
        if (sDMLockBroadcastReceiver == null && context != null) {
            sDMLockBroadcastReceiver = new DMLockBroadcastReceiver(context.getApplicationContext());
        }

        return sDMLockBroadcastReceiver;
    }

    /// privacy protect @{
    private final static String NOTIFY_LOCKED = "com.mediatek.ppl.NOTIFY_LOCK";
    private final static String NOTIFY_UNLOCK = "com.mediatek.ppl.NOTIFY_UNLOCK";
    /// @}

    private StatusBarHelper statusBarHelper;

    public void register(Context context) {
        IntentFilter lockFilter = new IntentFilter(NOTIFY_LOCKED);
        lockFilter.addAction(NOTIFY_UNLOCK);
        Log.d(this, "register ppl lock message");

        if (isPplLock()) {
            statusBarHelper.enableSystemBarNavigation(false);
        }

        context.registerReceiver(this, lockFilter);
        Log.d(this, "finish register ppl lock message");
    }

    private boolean isPplLock() {
        boolean isPplLocked = false;
        try {
            IPplAgent agent = IPplAgent.getService();
            if (agent != null) {
                isPplLocked = agent.needLock() == 1;
            }
        } catch (RemoteException e) {
            Log.d(this, "isPplLock, RemoteException");
        } catch (NoSuchElementException e) {
            Log.d(this, "isPplLock, NoSuchElementException");
        }
        return isPplLocked;
    }

    public void unregister(Context context) {
        // reset privacyProtectEnable
        Log.d(this, "Disable privacy protect and enable system bar in onDestroy()");
        /// M: For ALPS1561354. The on-going call would be hung up as NOTIFY_LOCK arrives.
        // At this point, this activity destroyed and unexpectly unregister the broadcast
        // receiver which is suppose to receive the subsequent NOTIFY_UNLOCK. This change
        // insure to do the reverse operation for the NOTIFY_LOCK in this case
        statusBarHelper.enableSystemBarNavigation(true);
        context.unregisterReceiver(this);
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.d(this, "action: " + action);
        /// if privacy protect open,should disable NavigationBar. For ALPS01414144 @
        if (action.equals(NOTIFY_LOCKED)) {
            statusBarHelper.enableSystemBarNavigation(false);
        } else if (action.equals(NOTIFY_UNLOCK)) {
            statusBarHelper.enableSystemBarNavigation(true);
        }
        /// @}
        DialerCall call = CallList.getInstance().getActiveOrBackgroundCall();
        if (call == null || !DialerCallState.isConnectingOrConnected(call.getState())) {
            Log.d(this, "onReceive , return");
            return;
        }
        if (action.equals(NOTIFY_LOCKED)) {
            int msg = R.string.dm_lock;
            if (call.getState() == DialerCallState.IDLE) {
                return;
            } else {
                Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
            }
        } else if (action.equals(NOTIFY_UNLOCK)) {
            int msg = R.string.dm_unlock;
            if (call.getState() == DialerCallState.IDLE) {
                return;
            } else {
                Toast.makeText(context, msg, Toast.LENGTH_LONG).show();
            }
        }

        InCallPresenter.getInstance().onCallListChange(CallList.getInstance());
    }
}
