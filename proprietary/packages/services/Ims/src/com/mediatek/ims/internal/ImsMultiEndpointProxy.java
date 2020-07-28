
package com.mediatek.ims.internal;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.telephony.Rlog;

import android.telephony.ims.ImsExternalCallState;
import android.telephony.ims.stub.ImsMultiEndpointImplBase;
import com.android.ims.internal.IImsExternalCallStateListener;
import com.mediatek.ims.internal.DialogEventPackageReceiver.Listener;
import com.mediatek.ims.internal.DialogEventPackageReceiver.ListenerBase;

import java.util.List;

/**
 * OEM implementation of IImsMultiEndpoint aidl.
 */
public class ImsMultiEndpointProxy extends ImsMultiEndpointImplBase {
    private static final String TAG = "ImsMultiEndpointProxy";

    private DialogEventPackageReceiver mReceiver;
    private IImsExternalCallStateListener mListener = null;
    private Handler mHandler;
    private List<ImsExternalCallState> mExternalCallStateList;

    private static final int MSG_UPDATE = 1;

    /**
     * Creates the instance of ImsMultiEndpointProxy.
     * @param context application context
     */
    public ImsMultiEndpointProxy(Context context) {
        mReceiver = new DialogEventPackageReceiver(context, mDepListener);
        mHandler = new Handler(Looper.getMainLooper()) {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_UPDATE:
                        ImsMultiEndpointProxy.this.update(mExternalCallStateList);
                        break;
                    default:
                        break;
                }
            }
        };
        Rlog.d(TAG, "ImsMultiEndpointProxy");
    }

    private final Listener mDepListener = new ListenerBase() {
        @Override
        public void onStateChanged(List<ImsExternalCallState> list) {
            ImsMultiEndpointProxy.this.update(list);
        }
    };

    /**
     * Query api to get the latest Dialog Event Package information.
     * Should be invoked only after setListener is done.
     */
    @Override
    public void requestImsExternalCallStateInfo() {
        mHandler.sendEmptyMessage(MSG_UPDATE);
    }

    private void update(List<ImsExternalCallState> externalCallDialogs) {
        mExternalCallStateList = externalCallDialogs;
        onImsExternalCallStateUpdate(mExternalCallStateList);
    }
}
