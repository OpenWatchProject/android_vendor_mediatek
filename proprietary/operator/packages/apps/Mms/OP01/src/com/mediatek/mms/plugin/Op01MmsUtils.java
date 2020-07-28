package com.mediatek.mms.plugin;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.preference.PreferenceManager;
import android.provider.Telephony;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.URLSpan;
import android.util.Log;
import android.widget.TextView;

import com.google.android.mms.ContentType;
import com.google.android.mms.pdu.PduPart;
import com.mediatek.mms.callback.IFileAttachmentModelCallback;
import com.mediatek.internal.telephony.IMtkSms;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.List;

/**
 * Op01MmsUtils.
 *
 */
public class Op01MmsUtils {
    private static final String TAG = "Op01MmsUtils";
    private static final String TEXT_SIZE = "message_font_size";
    private static final float DEFAULT_TEXT_SIZE = 18;
    private static final float MIN_TEXT_SIZE = 10;
    private static final float MAX_TEXT_SIZE = 32;
    private static final String MMS_APP_PACKAGE = "com.android.mms";

    private static final String MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY
            = "pref_key_mms_enable_to_send_delivery_reports";

    public static final String CONTENT_TYPE_APP_OCET_STREAM   = "application/octet-stream";

    // folder mode
    public static final int FOLDER_OPTION_INBOX    = 0;
    public static final int FOLDER_OPTION_OUTBOX   = 1;
    public static final int FOLDER_OPTION_DRAFTBOX = 2;
    public static final int FOLDER_OPTION_SENTBOX  = 3;

    /**
     * get Text size from preference.
     * @param context Context
     * @return text size
     */
    /* q0 migration, phase out text size adjust*/
    /*
    public static float getTextSize(Context context) {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        float size = sp.getFloat(TEXT_SIZE, DEFAULT_TEXT_SIZE);
        Log.v(TAG, "getTextSize = " + size);
        if (size < MIN_TEXT_SIZE) {
            size = MIN_TEXT_SIZE;
        } else if (size > MAX_TEXT_SIZE) {
            size = MAX_TEXT_SIZE;
        }
        return size;
    }
*/
    /**
     * setTextSize.
     * @param context Context
     * @param size float.
     */
    /* q0 migration, phase out text size adjust*/
    /*
    public static void setTextSize(Context context, float size) {
        float textSize;

        Log.v(TAG, "setTextSize = " + size);

        if (size < MIN_TEXT_SIZE) {
            textSize = MIN_TEXT_SIZE;
        } else if (size > MAX_TEXT_SIZE) {
            textSize = MAX_TEXT_SIZE;
        } else {
            textSize = size;
        }
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = sp.edit();
        editor.putFloat(TEXT_SIZE, textSize);
        editor.commit();
    }
*/
    /**
     * isSmsEnabled.
     * @param context Context
     * @return true if mms is default sms application.
     */
    public static boolean isSmsEnabled(Context context) {

        String defaultSmsApplication = Telephony.Sms.getDefaultSmsPackage(context);
        if (defaultSmsApplication != null && defaultSmsApplication.equals(MMS_APP_PACKAGE)) {
            return true;
        }
        return false;
    }

    /**
     * isSimInserted.
     * @param context Context
     * @return true if any sim inserted.
     */
    public static boolean isSimInserted(Context context) {
        Log.d(TAG, "isSimInserted");
        int subCount = getEnabledSubCount(context);
        if (subCount >= 0) {
            if (subCount == 0) {
                return false;
            } else {
                return true;
            }
        }
        List<SubscriptionInfo> listSimInfo = SubscriptionManager.from(context)
                                                .getActiveSubscriptionInfoList();
        if (listSimInfo == null || listSimInfo.isEmpty()) {
            return false;
        } else {
            return true;
        }
    }

    /**
     * isAirplaneOn.
     * @param context Context
     * @return true if air mode on.
     */
    public static boolean isAirplaneOn(Context context) {
        boolean airplaneOn = Settings.System.getInt(context.getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, 0) == 1;
        if (airplaneOn) {
            Log.d(TAG, "airplane is On");
            return true;
        }
        return false;
    }

    /**
     * isSmsReady.
     * @param context Context
     * @return true if ready.
     */
    public static boolean isSmsReady(Context context) {
        Log.d(TAG, "isSmsReady");
        IMtkSms smsManager = IMtkSms.Stub.asInterface(ServiceManager.getService("imtksms"));
        if (smsManager == null) {
            Log.d(TAG, "smsManager is null");
            return false;
        }

        boolean smsReady = false;
        List<SubscriptionInfo> subInfoList = SubscriptionManager.from(context).getActiveSubscriptionInfoList();

        for (SubscriptionInfo subInfoRecord : subInfoList) {
            try {
                Log.d(TAG, "subId=" + subInfoRecord.getSubscriptionId());
                smsReady = smsManager.isSmsReadyForSubscriber(subInfoRecord.getSubscriptionId());
                if (smsReady) {
                    break;
                }
            } catch (RemoteException e) {
                Log.d(TAG, "isSmsReady failed to get sms state for sub "
                        + subInfoRecord.getSubscriptionId());
                smsReady = false;
            }
        }

        Log.d(TAG, "smsReady" + smsReady);
        return smsReady;
    }

    /**
     * unescapeXML.
     * @param str String
     * @return String
     */
    public static String unescapeXML(String str) {
        return str.replaceAll("&lt;", "<")
                .replaceAll("&gt;", ">")
                .replaceAll("&quot;", "\"")
                .replaceAll("&apos;", "'")
                .replaceAll("&amp;", "&");
    }

    /**
     * getAllAttachSize.
     * @param files ArrayList
     * @return totle size of all all attachment.
     */
    public static int getAllAttachSize(ArrayList files) {
        if (files == null) {
            return 0;
        }

        int attachSize = 0;
        for (int i = 0; i < files.size(); i++) {
            attachSize += ((IFileAttachmentModelCallback) files.get(i)).getAttachSizeCallback();
        }
        return attachSize;
    }

    /**
     * Whether the type is supported.
     * @param contentType String
     * @return true if supported
     */
    public static boolean isSupportedFile(final String contentType) {
        if (TextUtils.isEmpty(contentType)) {
            return false;
        }

        return contentType.equalsIgnoreCase(ContentType.TEXT_VCARD)
                || contentType.equalsIgnoreCase(ContentType.TEXT_VCALENDAR)
                || contentType.equalsIgnoreCase(CONTENT_TYPE_APP_OCET_STREAM)
                // for support any attachment expect text/plain and text/html
                || ContentType.isImageType(contentType)
                || ContentType.isVideoType(contentType)
                || ContentType.isAudioType(contentType)
                || isAnyAttachment(contentType);
    }

    /**
     * Whether this pdu part is a attachment.
     * @param part Pdupart
     * @return return true if the part is a attachment.
     */
    public static boolean isOtherAttachment(final PduPart part) {
        String filename = null;
        final String type = new String(part.getContentType());
        if (isAnyAttachment(type)) {
            return true;
        } else {
            return false;
        }
    }

    private static boolean isAnyAttachment(String str) {
        if (!str.equals("application/smil") && !str.equals("text/plain")
                && !str.equals("text/html")) {
            Log.d(TAG, "is isAnyAttachment type");
            return true;
        } else {
            return false;
        }
    }

    /**
     * isEnableSendDeliveryReport.
     * @param context Context
     * @param subId int
     * @return return true if allow to send delivery report.
     */
    public static boolean isEnableSendDeliveryReport(Context context, int subId) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        if (isMtkGemini()) {
            return prefs.getBoolean(Integer.toString(subId) + "_" +
                    MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY,
                    false);
        } else {
            return prefs.getBoolean(MMS_ENABLE_TO_SEND_DELIVERY_REPORT_KEY, false);
        }
    }

    /**
     * Is genini config.
     * @return return true if the phone is configured to gemini, else return false;
     */
    public static boolean isMtkGemini() {
        return TelephonyManager.getDefault().getPhoneCount() == 2;
    }

    public static int getEnabledSubCount(Context context) {
        Log.d(TAG, "getEnabledSubCount");
        /* q0 migration*/
        boolean simOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        Log.d(TAG, "simOnOffEnabled1 = " + simOnOffEnabled);
        if (simOnOffEnabled == false) {
            return -1;
        }

        List<SubscriptionInfo> listSimInfo = SubscriptionManager.from(context)
                                                .getActiveSubscriptionInfoList();
        if (listSimInfo == null) {
            return 0;
        }
        int subCount = listSimInfo.size();
        int enabledSubCount = 0;

        for (int i = 0; i < subCount; i++) {
            //int subId = (int) mSubInfoList.get(i).getSubscriptionId();
            SubscriptionInfo subInfo = listSimInfo.get(i);
            if (subInfo != null) {
                int slotId = subInfo.getSimSlotIndex();
                Log.d(TAG, "slotId = " + slotId);
                /* q0 migration*/
                int slotState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
                Log.d(TAG, "slotState = " + slotState);
                if (slotState == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                    enabledSubCount++;
                }
            }
        }
        Log.d(TAG, "enabledSubCount = " + enabledSubCount);
        return enabledSubCount;
    }

    public static boolean isSubEnabled(Context context, int subId) {
        Log.d(TAG, "isSubEnabled subId = " + subId);
        if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            return false;
        }
        boolean simOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        Log.d(TAG, "simOnOffEnabled1 = " + simOnOffEnabled);
        if (simOnOffEnabled == false) {
            return true;
        }
        int slotId = SubscriptionManager.from(context)
                .getActiveSubscriptionInfo(subId).getSimSlotIndex();
        /* q0 migration*/
        int slotState = MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId);
        Log.d(TAG, "slotState = " + slotState);
        if (slotState == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
            return true;
        } else {
            return false;
        }
    }


    /// M: ALPS00527989, Extend TextView URL handling @ {
    /**
     * setExtendUrlSpan.
     * @param textView TextView
     */
    /* q0 migration, phase out open url dialog*/
    /*
    public static void setExtendUrlSpan(TextView textView) {
        Log.d(TAG, "setExtendUrlSpan");

        CharSequence text = textView.getText();

        Spanned spanned = ((Spanned) text);
        URLSpan[] urlSpanArray = textView.getUrls();
        for (int i = 0; i < urlSpanArray.length; i++) {
            String url = urlSpanArray[i].getURL();
            Log.d(TAG, "find url:" + url);
            if (isWebUrl(url)) {
                URLSpan newurlSpan = new ExtendURLSpan(url);
                int spanStart = spanned.getSpanStart(urlSpanArray[i]);
                int spanEnd = spanned.getSpanEnd(urlSpanArray[i]);
                Spannable sp = (SpannableString) (text);
                ((SpannableString) (text)).removeSpan(urlSpanArray[i]);
                ((SpannableString) (text)).setSpan(newurlSpan, spanStart,
                                        spanEnd, Spanned.SPAN_INCLUSIVE_INCLUSIVE);
            }
        }
    }

    private static boolean isWebUrl(String urlString) {
        boolean isWebURL = false;
        Uri uri = Uri.parse(urlString);

        String scheme = uri.getScheme();
        if (scheme != null) {
            isWebURL = scheme.equalsIgnoreCase("http") || scheme.equalsIgnoreCase("https")
                    || scheme.equalsIgnoreCase("rtsp");
        }
        return isWebURL;
    }
    */
    /// @}
}
