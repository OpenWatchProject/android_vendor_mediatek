package com.mediatek.engineermode;

import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.util.SparseArray;

import java.util.ArrayList;
import java.util.Arrays;

import vendor.mediatek.hardware.mtkradioex.V1_0.IMtkRadioEx;

public class EmRadioHidl {
    public static final String TAG = "EmRadioHidl";
    private static final String[] HIDL_SERVICE_NAME = {"mtkEm1", "mtkEm2", "mtkEm3", "mtkEm4"};
    public static SparseArray<Message> mRequestList = new SparseArray<Message>();
    public static Handler mHandler;
    public static int mWhat;
    public static int mRadioIndicationType = -1;
    private static MtkRadioResponseBase mRadioResponseMtk = null;
    private static MtkRadioIndication mRadioIndicationMtk = null;
    private static IMtkRadioEx[] mRadioProxyMtk = new IMtkRadioEx[4];
    private static boolean mIsRadioIndicationListen = false;

    static private IMtkRadioEx getRadioProxy(int phoneId, Message result) {
        //  Elog.d(TAG, "getRadioProxy,phoneId = " + phoneId);
        if (mRadioResponseMtk == null) {
            mRadioResponseMtk = new MtkRadioResponseBase(null);
        }

        if (mRadioIndicationMtk == null) {
            mRadioIndicationMtk = new MtkRadioIndication(null);
        }

        if (result != null) {
            mRequestList.append(result.what, result);
        }

        try {
            mRadioProxyMtk[phoneId] = IMtkRadioEx.getService(HIDL_SERVICE_NAME[phoneId], false);
            if (mRadioProxyMtk[phoneId] != null) {
                mRadioProxyMtk[phoneId].setResponseFunctionsEm(mRadioResponseMtk,
                        mIsRadioIndicationListen ? mRadioIndicationMtk : null);
               // Elog.d(TAG, "getRadioProxy succeed");
            } else {
                Elog.e(TAG, "getRadioProxy failed");
            }
        } catch (RemoteException | RuntimeException e) {
            mRadioProxyMtk[phoneId] = null;
            Elog.d(TAG, "RadioProxy getService: " + e);
        }
        return mRadioProxyMtk[phoneId];
    }

    public static int getSlotId(int phoneid) {
        int slotId;
        switch (phoneid) {
            case -1:
                slotId = ModemCategory.getCapabilitySim();
                break;
            case 0xff:
                slotId = ModemCategory.getCdmaPhoneId();
                break;
            default:
                slotId = phoneid;
                break;
        }
        return slotId;
    }

    public static void invokeOemRilRequestStringsEm(int phoneId, String strings[], Message
            result) {
        int slotId = getSlotId(phoneId);
        IMtkRadioEx radioProxy = getRadioProxy(slotId, result);
        if (radioProxy != null) {
            try {
                //  Elog.d(TAG, "send cmd = " + strings[0]);
                radioProxy.sendRequestStrings(result == null ? 0 : result.what,
                        new ArrayList<String>(Arrays.asList(strings)));
            } catch (RemoteException | RuntimeException e) {
                Elog.e(TAG, e.toString());
            }
        }
    }

    private static ArrayList<Byte> primitiveArrayToArrayList(byte[] arr) {
        ArrayList<Byte> arrayList = new ArrayList<>(arr.length);
        for (byte b : arr) {
            arrayList.add(b);
        }
        return arrayList;
    }

    public static void invokeOemRilRequestRawEm(int phoneId, byte[] data, Message
            result) {
        int slotId = getSlotId(phoneId);
        IMtkRadioEx radioProxy = getRadioProxy(slotId, result);
        if (radioProxy != null) {
            try {
                // Elog.d(TAG, "send cmd = " + data[0]);
                radioProxy.sendRequestRaw(result.what, primitiveArrayToArrayList(data));
            } catch (RemoteException | RuntimeException e) {
                Elog.e(TAG, e.toString());
            }
        } else {
            Elog.e(TAG, "radioProxy is null");
        }
    }

    public static void mSetRadioIndicationType(int type) {

    }

    public static void mSetRadioIndicationMtk(int phoneId, Handler handler, int what, boolean
            isRadioIndicationListen) {
        mIsRadioIndicationListen = isRadioIndicationListen;
        mHandler = handler;
        mWhat = what;
        IMtkRadioEx radioProxy = getRadioProxy(phoneId, null);
        if (radioProxy != null) {
            //Elog.d(TAG, "Set RadioIndicationMtk succeed");
        } else {
            Elog.e(TAG, "radioProxy is null");
        }
    }

    public static void rebootModemHidl() {
        IMtkRadioEx radioProxy = getRadioProxy(0, null);
        if (radioProxy != null) {
            try {
                radioProxy.setTrm(0, 2);
            } catch (RemoteException | RuntimeException e) {
                Elog.e(TAG, e.toString());
            }
        } else {
            Elog.e(TAG, "radioProxy is null");
        }
    }

    public static void reloadModemType(int modemType) {
        int slotId = getSlotId(-1);
        IMtkRadioEx radioProxy = getRadioProxy(slotId, null);
        if (radioProxy != null) {
            try {
                radioProxy.modifyModemType(0, 1, modemType);
            } catch (RemoteException | RuntimeException e) {
                Elog.e(TAG, e.toString());
            }
        } else {
            Elog.e(TAG, "radioProxy is null");
        }
    }

    public static void storeModemType(int modemType) {
        int slotId = getSlotId(-1);
        IMtkRadioEx radioProxy = getRadioProxy(slotId, null);
        if (radioProxy != null) {
            try {
                radioProxy.modifyModemType(0, 2, modemType);
            } catch (RemoteException | RuntimeException e) {
                Elog.e(TAG, e.toString());
            }
        } else {
            Elog.e(TAG, "radioProxy is null");
        }
    }
}
