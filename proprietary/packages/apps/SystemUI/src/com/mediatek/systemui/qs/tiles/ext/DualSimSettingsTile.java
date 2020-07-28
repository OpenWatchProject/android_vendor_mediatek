package com.mediatek.systemui.qs.tiles.ext;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.systemui.Dependency;
import com.android.systemui.plugins.ActivityStarter;
import com.android.systemui.plugins.qs.QSTile.BooleanState;
import com.android.systemui.qs.QSHost;
import com.android.systemui.qs.tileimpl.QSTileImpl;

import com.mediatek.systemui.ext.IQuickSettingsPlugin;
import com.mediatek.systemui.ext.OpSystemUICustomizationFactoryBase;
import com.mediatek.systemui.statusbar.extcb.IconIdWrapper;

/**
 * M: Dual SIM settings tile Customization for operator .
 */
public class DualSimSettingsTile extends QSTileImpl<BooleanState> {
    private static final String TAG = "DualSimSettingsTile";
    private static final Intent DUAL_SIM_SETTINGS = new Intent().setComponent(new ComponentName(
            "com.android.settings", "com.android.settings.Settings$SimSettingsActivity"));

    private CharSequence mTileLabel;
    private final IconIdWrapper mEnableIconIdWrapper = new IconIdWrapper();
    private final IconIdWrapper mDisableIconIdWrapper = new IconIdWrapper();

    /**
     * Constructor.
     * @param host The QSTileHost.
     */
    public DualSimSettingsTile(QSHost host) {
        super(host);
        registerSimStateReceiver();
   }

    @Override
    public BooleanState newTileState() {
        return new BooleanState();
    }

    @Override
    public void handleSetListening(boolean listening) {
    }

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.QS_PANEL;
    }

    @Override
    public CharSequence getTileLabel() {
        mTileLabel = OpSystemUICustomizationFactoryBase
                .getOpFactory(mContext).makeQuickSettings(mContext)
                .getTileLabel("dulsimsettings");
        return mTileLabel;
    }

    @Override
    public Intent getLongClickIntent() {
        return null;
    }

    @Override
    protected void handleLongClick() {
        handleClick();
    }

    @Override
    protected void handleClick() {
        final long subId = SubscriptionManager.getDefaultDataSubscriptionId();
        Log.d(TAG, "handleClick, " + DUAL_SIM_SETTINGS);
        DUAL_SIM_SETTINGS.putExtra(PhoneConstants.SUBSCRIPTION_KEY, subId);
        /* mHost.startActivityDismissingKeyguard(DUAL_SIM_SETTINGS); */
        Dependency.get(ActivityStarter.class).postStartActivityDismissingKeyguard(
                DUAL_SIM_SETTINGS, 0);
        refreshState();
    }

    @Override
    protected void handleUpdateState(BooleanState state, Object arg) {
        Boolean simInserted = (Boolean) arg;
        Log.d(TAG, "handleUpdateState, " + " simInserted=" + simInserted);
        IQuickSettingsPlugin quickSettingsPlugin = OpSystemUICustomizationFactoryBase
                .getOpFactory(mContext).makeQuickSettings(mContext);

        if (simInserted != null && simInserted) {
            state.label = quickSettingsPlugin.customizeDualSimSettingsTile(
                    false, mDisableIconIdWrapper, "");
            state.icon = QsIconWrapper.get(
                    mDisableIconIdWrapper.getIconId(), mDisableIconIdWrapper);
        } else {
            state.label = quickSettingsPlugin.customizeDualSimSettingsTile(
                    true, mEnableIconIdWrapper, "");
            state.icon = QsIconWrapper.get(mEnableIconIdWrapper.getIconId(), mEnableIconIdWrapper);
        }
        mTileLabel = state.label;
    }

    private void registerSimStateReceiver() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mContext.registerReceiver(mSimStateIntentReceiver, filter);
     }

    // Broadcast receive to determine if SIM is absent or not.
    private BroadcastReceiver mSimStateIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "onReceive action is " + action);
            if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                Log.d(TAG, "onReceive action is " + action + " stateExtra=" + stateExtra);
                if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(stateExtra)) {
                    handleRefreshState(false);
                } else {
                    handleRefreshState(true);
                }
            }
        }
    };
}
