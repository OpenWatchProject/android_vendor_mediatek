package com.mediatek.contacts.aassne;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;

import com.android.internal.telephony.EncodeException;
import com.android.internal.telephony.GsmAlphabet;
import com.mediatek.contacts.simcontact.SimCardUtils;
import com.mediatek.contacts.simcontact.PhbInfoUtils;
import com.mediatek.contacts.simcontact.SubInfoUtils;
import com.mediatek.contacts.util.AccountTypeUtils;
import com.mediatek.contacts.util.Log;
import com.mediatek.internal.telephony.phb.AlphaTag;
import com.mediatek.internal.telephony.phb.IMtkIccPhoneBook;
import com.mediatek.provider.MtkContactsContract.Aas;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;

public class SimAasSneUtils {
    private static final String TAG = "SimAasSneUtils";
    private static final String MTK_SIMPHONEBOOK_SERVICE = "mtksimphonebook";
    public static final String IS_ADDITIONAL_NUMBER = "1";
    public static final String KEY_SUB_ID = "subId";
    private static final int ERROR = -1;

    /* M-TODO: better to refactor this member into
     * PhbInfoUtils.PhbInfoWrapper, and use PhbInfoWrapper's flow to get it.
     * But should take care: the methods related with this member are sync call,
     * while PhbInfoWrapper's flow is async call!!!
     */
    private static HashMap<Integer, List<AlphaTag>> sAasMap =
                new HashMap<Integer, List<AlphaTag>>(2);
    private static String sCurrentAccount = null;
    private static int sCurSubId = -1;

    public static void setCurrentSubId(int subId) {
        sCurSubId = subId;
        sCurrentAccount = AccountTypeUtils.getAccountTypeBySub(subId);
        Log.d(TAG, "[setCurrentSubId] sCurSubId=" + sCurSubId
                + " sCurrentAccount=" + Log.anonymize(sCurrentAccount));
    }

    public static String getCurAccount() {
        return sCurrentAccount;
    }

    public static int getCurSubId() {
        return sCurSubId;
    }

    /**
     * refresh local aas list. after you change the USim card aas info, please refresh local info.
     * @param slot
     * @return
     */
    public static boolean refreshAASList(int subId) {
        int slot = SubscriptionManager.getSlotIndex(subId);
        if (slot <= SubInfoUtils.getInvalidSlotId()) {
            Log.d(TAG, "[refreshAASList] slot=" + slot);
            return false;
        }

        try {
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            if (iIccPhb != null) {
                Log.d(TAG, "[refreshAASList] subId =" + subId);
                List<AlphaTag> atList = iIccPhb.getUsimAasList(subId);
                if (atList != null) {
                    Iterator<AlphaTag> iter = atList.iterator();
                    Log.d(TAG, "[refreshAASList] success");
                    while (iter.hasNext()) {
                        AlphaTag entry = iter.next();
                        String tag = entry.getAlphaTag();
                        if (TextUtils.isEmpty(tag)) {
                            iter.remove();
                        }
                        Log.d(TAG, "[refreshAASList] tag=" + Log.anonymize(tag));
                    }
                }
                sAasMap.put(slot, atList);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[refreshAASList] catched exception.");
            sAasMap.put(slot, null);
        }

        return true;
    }

    /**
     * get USim card aas info without null tag. It will return all aas info that can be used in
     * application.
     * @param slot
     * @return
     */
    public static List<AlphaTag> getAAS(int subId) {
        List<AlphaTag> atList = new ArrayList<AlphaTag>();
        int slot = SubscriptionManager.getSlotIndex(subId);
        if (slot <= SubInfoUtils.getInvalidSlotId()) {
            Log.e(TAG, "[getAAS] slot=" + slot);
            return atList;
        }
        // Here, force to refresh the list.
        Log.d(TAG, "[getAAS] refreshAASList");
        refreshAASList(subId);

        List<AlphaTag> list = sAasMap.get(slot);

        return list != null ? list : atList;
    }

    public static int getAasIndexFromIndicator(String indicator) {
        if(!TextUtils.isEmpty(indicator) && indicator.contains(Aas.ENCODE_SYMBOL)) {
            String[] keys = indicator.split(Aas.ENCODE_SYMBOL);
            return Integer.valueOf(keys[1]);
        }
        return 0;
    }

    public static String getAASByIndicator(String indicator) {
        if(TextUtils.isEmpty(indicator) || !indicator.contains(Aas.ENCODE_SYMBOL)) {
            return null;
        }
        String[] keys = indicator.split(Aas.ENCODE_SYMBOL);
        if (keys.length != 2) {
            return null;
        }
        return getAASById(Integer.valueOf(keys[0]), Integer.valueOf(keys[1]));
    }

    public static String getAASById(int subId, int index) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId <= SubInfoUtils.getInvalidSlotId() || index < 1) {
            return "";
        }
        String aas = "";
        try {
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            if (iIccPhb != null) {
                aas = iIccPhb.getUsimAasById(subId, index);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[getUSIMAASById] catched exception.");
        }
        if (aas == null) {
            aas = "";
        }
        Log.d(TAG, "[getUSIMAASById] aas=" + Log.anonymize(aas));
        return aas;
    }

    public static String buildAASIndicator(String aas, int subId) {
        int index = getAasIndexByName(aas, subId);
        if (index == ERROR) {
            return "";
        } else {
            return "" + subId + Aas.ENCODE_SYMBOL + index;
        }
    }

    public static int getAasIndexByName(String aas, int subId) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId <= SubInfoUtils.getInvalidSlotId() || TextUtils.isEmpty(aas)) {
            Log.e(TAG, "[getAasIndexByName] error slotId=" + slotId + ",aas="
                    + Log.anonymize(aas));
            return ERROR;
        }
        // here, it only can compare type name
        Log.d(TAG, "[getAasIndexByName] aas=" + Log.anonymize(aas));
        List<AlphaTag> atList = getAAS(subId);
        Iterator<AlphaTag> iter = atList.iterator();
        while (iter.hasNext()) {
            AlphaTag entry = iter.next();
            String tag = entry.getAlphaTag();
            if (aas.equalsIgnoreCase(tag)) {
                Log.d(TAG, "[getAasIndexByName] tag=" + Log.anonymize(tag));
                return entry.getRecordIndex();
            }
        }
        return ERROR;
    }

    public static int insertUSIMAAS(int subId, String aasName) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId <= SubInfoUtils.getInvalidSlotId() || TextUtils.isEmpty(aasName)) {
            return ERROR;
        }
        int result = ERROR;
        try {
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            if (iIccPhb != null) {
                result = iIccPhb.insertUsimAas(subId, aasName);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[insertUSIMAAS] catched exception.");
        }

        return result;
    }

    public static boolean updateUSIMAAS(int subId, int index, int pbrIndex, String aasName) {
        boolean result = false;
        try {
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            if (iIccPhb != null) {
                result = iIccPhb.updateUsimAas(subId, index, pbrIndex, aasName);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[updateUSIMAAS] catched exception.");
        }
        Log.d(TAG, "[updateUSIMAAS] refreshAASList");
        refreshAASList(subId);

        return result;
    }

    public static boolean removeUsimAasById(int subId, int index, int pbrIndex) {
        boolean result = false;
        try {
            final IMtkIccPhoneBook iIccPhb = getIMtkIccPhoneBook();
            if (iIccPhb != null) {
                result = iIccPhb.removeUsimAasById(subId, index, pbrIndex);
            }
        } catch (RemoteException e) {
            Log.e(TAG, "[removeUsimAasById] catched exception.");
        }
        Log.d(TAG, "[removeUsimAasById] refreshAASList");
        refreshAASList(subId);

        return result;
    }

    public static boolean isAasTextValid(String text, int subId) {
        if (TextUtils.isEmpty(text)) {
            return false;
        }
        final int MAX = PhbInfoUtils.getUsimAasMaxNameLength(subId);
        try {
            GsmAlphabet.stringToGsm7BitPacked(text);
            if (text.length() > MAX) {
                return false;
            }
        } catch (EncodeException e) {
            if (text.length() > ((MAX - 1) >> 1)) {
                return false;
            }
        }
        return true;
    }

    private static IMtkIccPhoneBook getIMtkIccPhoneBook() {
        Log.d(TAG, "[getIMtkIccPhoneBook]");
        String serviceName = MTK_SIMPHONEBOOK_SERVICE;
        final IMtkIccPhoneBook iIccPhb = IMtkIccPhoneBook.Stub.asInterface(ServiceManager
                .getService(serviceName));
        return iIccPhb;
    }

    public static String getSuffix(int count) {
        if (count <= 0) {
            return "";
        } else {
            return String.valueOf(count);
        }
    }

}
