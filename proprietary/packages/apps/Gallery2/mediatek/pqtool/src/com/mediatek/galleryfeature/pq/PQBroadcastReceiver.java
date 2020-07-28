package com.mediatek.galleryfeature.pq;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

/**
 *Receive broadcast for PQ effect.
 */
public class PQBroadcastReceiver extends BroadcastReceiver {
    private final static String PQACTION = "com.mediatek.gallery.action.ReloadImage";
    private static PQBroadcastReceiver sReceiver;
    private PQListener mListener = null;
    /**
     *Implement by PhotoPage for add PQ effect.
     */
    public interface PQListener {
        /**
         * Call back after modify PQ effect.
         */
        public void onPQEffect();
    }

    /**
     * create PQBroadcastReceiver object.
     * @return PQBroadcastReceiver object for register and set listener.
     */
    public static PQBroadcastReceiver getReceiver() {
        if (sReceiver == null) {
            sReceiver = new PQBroadcastReceiver();
        }
        return sReceiver;
    }

    /**
     * Register PQ Receiver.
     * @param context for register PQ Receiver.
     */
    public static void registerReceiver(Context context) {
        if (getReceiver() != null) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(PQACTION);
            context.registerReceiver(getReceiver(), filter);
        }
    }

    /**
     * Register PQ Receiver.
     * @param context  for unregister PQ Receiver.
     */
    public static void unregisterReceiver(Context context) {
        if (getReceiver() != null) {
            context.unregisterReceiver(getReceiver());
        }
    }

    /**
     * add listener for receive PQ broadcast.
     * @param listener for PQ effect.
     */
    public static void setListener(PQListener listener) {
        if (getReceiver() != null) {
            getReceiver().mListener = listener;
        }
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (PQACTION.equalsIgnoreCase(action)) {
            if (mListener != null) {
                mListener.onPQEffect();
            }
        }
    }
}
