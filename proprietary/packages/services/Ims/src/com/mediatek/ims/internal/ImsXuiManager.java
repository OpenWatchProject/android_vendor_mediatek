package com.mediatek.ims.internal;

import android.net.Uri;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

/**
 * ImsXuiManager class.
 * To manage XUI
 *
 */
public class ImsXuiManager {
    private static final String LOG_TAG = "ImsXuiManager";
    public String[] mXui;
    static ImsXuiManager sInstance;

    /**
     * Constructor.
     */
    private ImsXuiManager() {
        int numPhones = TelephonyManager.getDefault().getPhoneCount();
        mXui = new String[numPhones];
    }

    /**
     * Instance constructor.
     *
     * @return ImsXuiManager instance
     */
    static public ImsXuiManager getInstance() {
        if (sInstance == null) {
            sInstance = new ImsXuiManager();
            sInstance.loadXui();
        }
        return sInstance;
    }


    public String getXui() {
        return mXui[0];
    }

    public String getXui(int phoneId) {
        return mXui[phoneId];
    }


    /**
     * Clear XUI.
     * Should be called if SIM card changed
     *
     */
    public void clearStoredXui() {
        mXui = null;
        // Todo: Clear the NV storage that XUI belongs to.
    }

    /**
     * Update from IMSA.
     *
     * @param xui  XUI
     */
    public void setXui(String xui) {
        mXui[0] = xui;
        // Todo: Save XUI to a NV storage
    }

    /**
     * Update from IMSA.
     *
     * @param phoneId IMS Phone Id
     * @param xui  XUI
     */
    public void setXui(int phoneId, String xui) {
        mXui[phoneId] = xui;
        // Todo: Save XUI to a NV storage
    }

    private void loadXui() {
        // Todo: load XUI from a NV storage
    }

    public Uri[] getSelfIdentifyUri(int phoneId) {
        if (phoneId < 0 || phoneId >= mXui.length) {
            Rlog.d(LOG_TAG,"IMS: getSelfIdentifyUri() invalid phone Id: " + phoneId);
            return null;
        }
        String xui = mXui[phoneId];
        if (xui == null) {
            return null;
        }
        String[] ids = xui.split(",");
        int len = ids.length;
        Uri[] uris = new Uri[len];

        for (int i = 0; i < len; ++i) {
            Uri uri = Uri.parse(ids[i]);
            String number = uri.getSchemeSpecificPart();
            if (TextUtils.isEmpty(number)) {
                Rlog.d(LOG_TAG, "empty XUI");
                continue;
            }
            String numberParts[] = number.split("[@;:]");
            if (numberParts.length == 0) {
                Rlog.d(LOG_TAG, "no number in XUI handle");
                continue;
            }
            number = numberParts[0];
            uris[i] = Uri.parse(number);

            Rlog.d(LOG_TAG,"IMS: getSelfIdentifyUri() uri = " + Rlog.pii(LOG_TAG, uris[i]));
        }
        return uris;
    }
}
