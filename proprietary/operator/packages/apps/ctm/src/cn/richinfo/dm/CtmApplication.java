package cn.richinfo.dm;

import android.app.Application;
import android.app.PendingIntent;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.os.Handler;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionManager;

import com.mediatek.provider.MtkSettingsExt;

import cn.richinfo.dm.DMSDK;
import cn.richinfo.dm.receiver.DMBroadCastReceiver;
import com.richinfo.dm.CtmReceiver;
import com.dmyk.android.telephony.DmykAbsTelephonyManager;
import com.dmyk.android.telephony.DmykTelephonyManager;
import com.dmyk.android.telephony.DmykTelephonyManager.MLog;


public class CtmApplication {
    private DmykTelephonyManager mDTM = null;
    private Application mApp;
    private ApnObserver[] mObservers = new ApnObserver[2];
    private static CtmApplication sInstance;

    private static final String ENHANCED_4G_MODE_ENABLED_SIM1 =
                                    Settings.Global.ENHANCED_4G_MODE_ENABLED;
    private static final String ENHANCED_4G_MODE_ENABLED_SIM2 = "volte_vt_enabled_sim2";
    public static final String TRANSIENT_SLOT_FLAG = "service.ctm.slot_flag";

    public CtmApplication(Application app) {
        mApp = app;
    }

    public void onCreate() {
        MLog.d("CtmApplication.onCreate()");
        DMSDK.init(mApp);
        if (MLog.isLogEnabled()) {
            DMSDK.setDebugMode(true);
        }
        // NOTE: this should be invoked after DMSDK.init(mApp), otherwise we cannot use the
        // the context specified by DMSDK. If DMSDK provides a non-null context, then use
        // application context instead.
        mDTM = (DmykTelephonyManager) DmykAbsTelephonyManager.getDefault(mApp);
        // DMSDK.setDmykAbsTelephonyManager(mDTM);

        registerNetworkCallback();
        // Observers for APN provider
        for (int slot = 0; slot < 2; ++slot) {
            Uri uri = mDTM.getAPNContentUri(slot);
            if (uri == null) {
                MLog.d("No URI for slot " + slot + ", probably there is no sim in the slot");
                mObservers[slot] = null;
            } else {
                mObservers[slot] = new ApnObserver(uri);
                getContentResolver().registerContentObserver(uri, true, mObservers[slot]);
            }
        }
        DMBroadCastReceiver DmReceiver = new DMBroadCastReceiver();
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction("cn.richinfo.dmyk.action.APPINFO_RETRY");
        intentFilter.addAction("cn.richinfo.dmyk.action.APPINFO");
        intentFilter.addAction("com.dmyk.android.telephony.action.SIM_STATE_CHANGED");
        intentFilter.addAction("cn.richinfo.dmyk.action.REGISITER");
        intentFilter.addAction("cn.richinfo.dmyk.action.REGISITER_RETRY");
        intentFilter.addAction("cn.richinfo.dmyk.action.HEARTBEAT");
        intentFilter.addAction("cn.richinfo.dmyk.action.HEARTBEAT_RETRY");
        intentFilter.addAction("cn.richinfo.dmyk.action.APN");
        intentFilter.addAction("cn.richinfo.dmyk.action.APN_RETRY");
        intentFilter.addAction("com.dmyk.android.telephony.action.VOLTE_STATE_CHANGE");
        intentFilter.addAction("com.dmyk.android.telephony.action.APN_STATE_CHANGE");
        intentFilter.addAction("cn.richinfo.dmyk.action.HEARTBEAT_SUCCESS");
        intentFilter.addAction("android.dmyk.net.conn.CONNECTIVITY_CHANGE");
        intentFilter.addAction("cn.richinfo.dmyk.action.THIRD_REQUEST");

        mApp.registerReceiver(DmReceiver,intentFilter);
        CtmReceiver ctmReceiver = new CtmReceiver();
        IntentFilter ctmIntentFilter = new IntentFilter();
        ctmIntentFilter.addAction("com.dmyk.android.telephony.action.VOLTE_STATE_SETTING");
        ctmIntentFilter.addAction("android.intent.action.SIM_STATE_CHANGED");
        mApp.registerReceiver(ctmReceiver,ctmIntentFilter);
        // Sync VoLTE status before registering observers
        syncVolteStatus();
        registerVolteObservers();
    }

    public static synchronized CtmApplication getInstance(Application app) {
        if (sInstance == null) {
            CtmApplication instance = new CtmApplication(app);
            // this method should set sInstance
            instance.onCreate();
            sInstance = instance;
        }
        return sInstance;
    }

    public int getSlotFlag() {
        String valueStr = SystemProperties.get(TRANSIENT_SLOT_FLAG, Integer.toString(0x00));
        int value = Integer.parseInt(valueStr);
        return value;
    }

    /**
     * 0x00 - neither slot is determined
     * 0x01 - slot 1 is determined and slot 2 is not determined
     * 0x02 - slot 2 is determined and slot 1 is not determined
     * 0x03 - both slots are determined, boot phase passed
     */
    public void setSlotFlag(int value) {
        SystemProperties.set(TRANSIENT_SLOT_FLAG, Integer.toString(value));
    }


    private void sendConnnectivityChanged(){
        Intent intent = new Intent(DmykAbsTelephonyManager.ACTION_CONNECTIVITY_CHANGE);
        intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
        mApp.sendBroadcast(intent);
    }

    private void registerNetworkCallback() {
        final ConnectivityManager connectivityMgr = ConnectivityManager.from(mApp);
        final NetworkRequest wifiRequest = new NetworkRequest.Builder()
            .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
            .build();
        final NetworkRequest dataRequest = new NetworkRequest.Builder()
            .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
            .build();
        connectivityMgr.registerNetworkCallback(
            wifiRequest,
            new ConnectivityManager.NetworkCallback() {
                /**
                 * @param network
                 */
                @Override
                public void onAvailable(Network network) {
                    MLog.d("WifiCallback.onAvailable, network=" + network);
                    sendConnnectivityChanged();
                }

                /**
                 * @param network
                 */
                @Override
                public void onLost(Network network) {
                    MLog.d("WifiCallback.onLost, network=" + network);
                    sendConnnectivityChanged();
                }
            });

        connectivityMgr.registerNetworkCallback(
            dataRequest,
            new ConnectivityManager.NetworkCallback() {
                /**
                 * @param network
                 */
                @Override
                public void onAvailable(Network network) {
                    MLog.d("MobileCallback.onAvailable, network=" + network);
                    sendConnnectivityChanged();
                }
                /**
                 * @param network
                 */
                @Override
                public void onLost(Network network) {
                    MLog.d("MobileCallback.onLost, network=" + network);
                    sendConnnectivityChanged();
                }
            });
    }
    private class ApnObserver extends ContentObserver {
        public final Uri uri;
        public ApnObserver(Uri uri) {
            super(new Handler());
            this.uri = uri;
        }

        @Override
        public void onChange(boolean selfChange) {
            onChange(selfChange, null);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            Intent intent = new Intent(DmykAbsTelephonyManager.ACTION_APN_STATE_CHANGE);
            intent.setData(uri);
            mApp.sendBroadcast(intent);
        }

    }

    private class VolteStatusObserver extends ContentObserver {
        private int mSlotId;
        private String mKey;
        public VolteStatusObserver(int slotId, String key) {
            super(new Handler());
            mSlotId = slotId;
            mKey = key;
        }

        @Override
        public void onChange(boolean selfChange) {
            onChange(selfChange, null);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            MLog.d("VolteStatusObserver.onChange(" + selfChange + ", " + uri + ")");
            int internalStatus = mDTM.getVoLTEState(mSlotId);
            MLog.d("VolteStatusObserver.onChange(): internalStatus=" + internalStatus);
            int status = Settings.System.getInt(
                getContentResolver(),
                mKey,
                DmykAbsTelephonyManager.VOLTE_STATE_UNKNOWN);
            MLog.d("VolteStatusObserver.onChange(): status=" + status);
            if (internalStatus != DmykAbsTelephonyManager.VOLTE_STATE_UNKNOWN &&
                status != DmykAbsTelephonyManager.VOLTE_STATE_UNKNOWN) {
                if (internalStatus != status) {
                    mDTM.setVoLTEState(mSlotId, status);
                }

                Intent intent = new Intent(DmykAbsTelephonyManager.ACTION_VOLTE_STATE_CHANGE);
                mApp.sendBroadcast(intent);
            }
        }
    }

    private int getVolteStatus(int slot){
        int status = DmykAbsTelephonyManager.VOLTE_STATE_UNKNOWN;
        if (slot == 0) {
            status = Settings.System.getInt(
                getContentResolver(),
                DmykAbsTelephonyManager.VOLTE_DMYK_STATE_0,
                DmykAbsTelephonyManager.VOLTE_STATE_UNKNOWN);
        } else {
            status = Settings.System.getInt(
                getContentResolver(),
                DmykAbsTelephonyManager.VOLTE_DMYK_STATE_1,
                DmykAbsTelephonyManager.VOLTE_STATE_UNKNOWN);
        }
        return status;
    }
    private void putVolteStatus(int slot,int status){
        if (slot == 0) {
            Settings.System.putInt(
                getContentResolver(),
                DmykAbsTelephonyManager.VOLTE_DMYK_STATE_0,
                status);
        } else {
            Settings.System.putInt(
                getContentResolver(),
                DmykAbsTelephonyManager.VOLTE_DMYK_STATE_1,
                status);
        }
    }
    private class InternalVolteStatusObserver extends ContentObserver {
        public InternalVolteStatusObserver() {
            super(new Handler());
        }

        @Override
        public void onChange(boolean selfChange) {
            onChange(selfChange, null);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            MLog.d("InternalVolteStatusObserver.onChange(" + selfChange + ", " + uri + ")");
            int internalStatus = DmykAbsTelephonyManager.VOLTE_STATE_UNKNOWN;
            int status = DmykAbsTelephonyManager.VOLTE_STATE_UNKNOWN;
            for (int i = 0; i < 2; i++) {
                internalStatus = mDTM.getVoLTEState(i);
                status = getVolteStatus(i);
                MLog.d("InternalVolteStatusObserver.onChange():" + "slot" + i + " status=" + status
                       +"internalStatus" +internalStatus );
                if (internalStatus != DmykAbsTelephonyManager.VOLTE_STATE_UNKNOWN) {
                    if (internalStatus != status) {
                        MLog.d("InternalVolteStatusObserver.onChange(): set status, slot="
                               + i + ", status=" + internalStatus);
                        putVolteStatus(i,internalStatus);
                    }
                }
            }
        }
    }

    public void updateAPNObservers() {
        MLog.d("Update APNObserver list");
        for (int slot = 0; slot < 2; ++slot) {
            Uri uri = mDTM.getAPNContentUri(slot);
            if (uri == null) {
                MLog.d("No URI for slot " + slot + ", probably there is no sim in the slot");
                if (mObservers[slot] != null) {
                    getContentResolver().unregisterContentObserver(mObservers[slot]);
                    mObservers[slot] = null;
                }
            } else {
                if (mObservers[slot] != null) {
                    if (mObservers[slot].uri.compareTo(uri) != 0) {
                        getContentResolver().unregisterContentObserver(mObservers[slot]);
                        mObservers[slot] = new ApnObserver(uri);
                        getContentResolver().registerContentObserver(uri, true, mObservers[slot]);
                    }
                } else {
                    mObservers[slot] = new ApnObserver(uri);
                    getContentResolver().registerContentObserver(uri, true, mObservers[slot]);
                }
            }
        }

    }

    public void syncVolteStatus() {
        MLog.d("syncVolteStatus");
        ContentResolver cr = getContentResolver();
        Settings.Global.putInt(
            cr,
            DmykAbsTelephonyManager.VOLTE_DMYK_STATE_0,
            mDTM.getVoLTEState(0));
        Settings.Global.putInt(
            cr,
            DmykAbsTelephonyManager.VOLTE_DMYK_STATE_1,
            mDTM.getVoLTEState(1));
    }

    private void registerVolteObservers() {
        MLog.d("registerVolteObservers");
        ContentResolver cr = getContentResolver();
        cr.registerContentObserver(
            Settings.System.getUriFor(MtkSettingsExt.System.VOLTE_DMYK_STATE_0),
            true,
            new VolteStatusObserver(0, MtkSettingsExt.System.VOLTE_DMYK_STATE_0));
        cr.registerContentObserver(
            Settings.System.getUriFor(MtkSettingsExt.System.VOLTE_DMYK_STATE_1),
            true,
            new VolteStatusObserver(1, MtkSettingsExt.System.VOLTE_DMYK_STATE_1));
        cr.registerContentObserver(
            SubscriptionManager.CONTENT_URI,
            true,
            new InternalVolteStatusObserver());
    }

    public ContentResolver getContentResolver() {
        return mApp.getContentResolver();
    }
}
