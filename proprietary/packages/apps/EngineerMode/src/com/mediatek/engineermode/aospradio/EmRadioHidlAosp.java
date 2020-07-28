package com.mediatek.engineermode.aospradio;

import android.hardware.radio.V1_0.IRadio;
import android.os.Handler;
import android.os.HwBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.SparseArray;

import com.mediatek.engineermode.Elog;

public class EmRadioHidlAosp {
    public static final String TAG = "EmRadioHidlAosp";
    private static final String[] HIDL_SERVICE_NAME = {"em1", "em2", "em3", "em4"};
    private static final int EVENT_RADIO_PROXY_DEAD = 1;
    public static SparseArray<Message> mRequestList = new SparseArray<Message>();
    public static Handler mHandler;
    public static int mWhat;
    public static int mRadioIndicationType = -1;
    private static RadioIndication mRadioIndicationMtk = null;
    private static IRadio[] mRadioProxyMtk = new IRadio[4];
    private static boolean mIsRadioIndicationListen = false;
    private static final int IRADIO_GET_SERVICE_DELAY_MILLIS = 2 * 1000;
    private static CapRadioProxyDeathRecipient mCapRadioProxyDeathRecipient = null;

    private static Handler mHandlerRadio = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_RADIO_PROXY_DEAD:
                    Elog.d(TAG, "RadioProxy dead");
                    getRadioProxy(0,null);
                    break;
                default:
                    break;
            }
        }
    };

    static private IRadio getRadioProxy(int phoneId, Message result) {
        //  Elog.d(TAG, "getRadioProxy,phoneId = " + phoneId);
        if (mCapRadioProxyDeathRecipient == null) {
            mCapRadioProxyDeathRecipient = new CapRadioProxyDeathRecipient();
        }
        if (mRadioIndicationMtk == null) {
            mRadioIndicationMtk = new RadioIndication(null);
        }

        if (result != null) {
            mRequestList.append(result.what, result);
        }

        try {
            mRadioProxyMtk[phoneId] = IRadio.getService(HIDL_SERVICE_NAME[phoneId], false);
            if (mRadioProxyMtk[phoneId] != null) {
                mRadioProxyMtk[phoneId].setResponseFunctions(null,
                        mIsRadioIndicationListen ? mRadioIndicationMtk : null);
                mRadioProxyMtk[phoneId].linkToDeath(mCapRadioProxyDeathRecipient, 0);
               // Elog.d(TAG, "getAospRadioProxy succeed");
            } else {
                Elog.e(TAG, "getAospRadioProxy failed");
            }
        } catch (RemoteException | RuntimeException e) {
            mRadioProxyMtk[phoneId] = null;
            Elog.d(TAG, "RadioProxyAosp getService: " + e);
            mHandlerRadio.sendMessageDelayed(mHandlerRadio.obtainMessage(EVENT_RADIO_PROXY_DEAD),
                    IRADIO_GET_SERVICE_DELAY_MILLIS);
        }
        return mRadioProxyMtk[phoneId];
    }

    public static void mSetRadioIndicationAosp(int phoneId, Handler handler, int what, boolean
            isRadioIndicationListen) {
        mIsRadioIndicationListen = isRadioIndicationListen;
        mHandler = handler;
        mWhat = what;
        IRadio radioProxy = getRadioProxy(phoneId, null);
        if (radioProxy != null) {
            // Elog.d(TAG, "Set RadioIndicationAosp succeed");
        } else {
            Elog.e(TAG, "radioProxyAosp is null");
        }
    }

    final static class CapRadioProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            mHandlerRadio.sendMessage(mHandlerRadio.obtainMessage(EVENT_RADIO_PROXY_DEAD, cookie));
        }
    }
}
