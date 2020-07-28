package com.mediatek.contacts.plugin;

import android.content.Context;
import android.os.Build;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;

/**
 * @hide
 */
public class ContactNumberUtils {

    /**
     * Sample code:
     *
     * ContactNumberUtils mNumberUtils = ContactNumberUtils.getDefault();
     * mNumberUtils.setContext(this);
     *
     * number = mNumberUtils.format(number);
     * FormatNumberResult result = mNumberUtils.validate(number);
     * if (ContactNumberUtils.NUMBER_VALID == result.getResultCode) {
     * }
     */

    /**
     * Contact nubmer error code.
     */
    public static final int NUMBER_VALID = 0;
    public static final int NUMBER_EMERGENCY = 1;
    public static final int NUMBER_SHORT_CODE = 2;
    public static final int NUMBER_PRELOADED_ENTRY = 3;
    public static final int NUMBER_FREE_PHONE = 4;
    public static final int NUMBER_INVALID = 5;
    public static final int NUMBER_INTERNATIONAL = 6;

    /**
     * The logger related.
     */
    private static final boolean DEBUG = Build.IS_DEBUGGABLE;
    private static final String TAG = "PHB/ContactNumberUtils";

    /**
     * Contact number length.
     */
    private static final int NUMBER_LENGTH_NORMAL = 10;
    private static final int NUMBER_LENGTH_NO_AREA_CODE = 7;

    /**
     * Save the singleton instance.
     */
    private static ContactNumberUtils sInstance = null;

    private Context mContext = null;

    /**
     * Constructor.
     */
    private ContactNumberUtils() {
        if (DEBUG) {
            Log.d(TAG, "ContactNumberUtils constructor");
        }
    }

    /**
     * Singe instance.
     * @return ContactNumberUtils.
     */
    public static ContactNumberUtils getDefault() {
        if (sInstance == null) {
            sInstance = new ContactNumberUtils();
        }

        return sInstance;
    }

    public void setContext(Context context) {
        mContext = context;
    }

    /**
     * Format contact number to the common format.
     *
     * @param phoneNumber read from contact, dialer or call log app.
     * @return formatted contact number.
     */
    public String format(final String phoneNumber) {
        String number = phoneNumber;
        if (TextUtils.isEmpty(number)) {
            return null;
        }

        number = PhoneNumberUtils.normalizeNumber(number);
        int len = number.length();
        if (len == NUMBER_LENGTH_NORMAL) {
            if (!number.startsWith("+1") && !number.startsWith("1")) {
                number = "+1" + number;
            }
        } else if (len == NUMBER_LENGTH_NORMAL + 1) {
            if (number.startsWith("1")) {
                number = "+" + number;
            }
        } else if (len >= NUMBER_LENGTH_NORMAL + 4) {
            if (number.startsWith("011")) {
                number = "+" + number.substring(3);
            }
        }

        return number;
    }

    /**
     * Check if it is a valid contact number for presence.
     *
     * @param phoneNumber read from contact, dialer or call log app.
     * @return contact number error code.
     */
    public boolean isValidNumber(final String phoneNumber) {
        FormatNumberResult result = validate(phoneNumber);
        return result.isValidNumber();
    }

    /**
     * Get format number for presence.
     *
     * @param phoneNumber read from contact, dialer or call log app.
     * @return format number, default is null.
     */
    public String getFormatNumber(final String phoneNumber) {
        String formatNumber = null;
        FormatNumberResult formatResult = validate(phoneNumber);
        LogUtils.d(TAG, "formatNumber: " + LogUtils.anonymize(formatResult.getFormatNumber())
                + " result: " + formatResult.getResultCode());
        if (formatResult.isValidNumber()) {
            formatNumber = formatResult.getFormatNumber();
        }
        return formatNumber;
    }

    /**
     * Check if it is a valid contact number for presence.
     *
     * @param phoneNumber read from contact, dialer or call log app.
     * @return contact number error code.
     */
    public FormatNumberResult validate(final String phoneNumber) {
        String number = phoneNumber;
        FormatNumberResult result = new FormatNumberResult(phoneNumber);

        if (TextUtils.isEmpty(number)) {
            return result;
        }

        if (number.contains("*")) {
            result.setResultCode(NUMBER_PRELOADED_ENTRY);
            return result;
        }

        number = PhoneNumberUtils.stripSeparators(number);
        number = number.replaceAll("[\\s\\-()]", "");
        if (!number.equals(PhoneNumberUtils.convertKeypadLettersToDigits(number))) {
            result.setResultCode(NUMBER_INVALID);
            return result;
        }

        if (PhoneNumberUtils.isEmergencyNumber(number)) {
            result.setResultCode(NUMBER_EMERGENCY);
            return result;
        } else if (number.startsWith("#")) {
            result.setResultCode(NUMBER_PRELOADED_ENTRY);
            return result;
        } else if (isInExcludedList(number)) {
            result.setResultCode(NUMBER_FREE_PHONE);
            return result;
        }
        int len = number.length();
        if (len > "+1-nnn-nnn-nnnn".length() || len < NUMBER_LENGTH_NORMAL) {
            result.setResultCode(NUMBER_INVALID);
            return result;
        }

        number = format(number);
        if (number.startsWith("+1")) {
            len = number.length();
            if (len == NUMBER_LENGTH_NORMAL + 2) {
                result.setResultCode(NUMBER_VALID);
                result.setFormatNumber(number);
                return result;
            }
        }

        return result;
    }

    /**
     * The excluded number list.
     */
    private static ArrayList<String> sExcludes = null;

    private boolean isInExcludedList(String number) {
        if (sExcludes == null) {
            sExcludes = new ArrayList<String>();
            sExcludes.add("800");
            sExcludes.add("822");
            sExcludes.add("833");
            sExcludes.add("844");
            sExcludes.add("855");
            sExcludes.add("866");
            sExcludes.add("877");
            sExcludes.add("880882");
            sExcludes.add("888");
            sExcludes.add("900");
            sExcludes.add("911");
            // for AT&T
            sExcludes.add("411");
        }

        String tempNumber = number;
        if (TextUtils.isEmpty(tempNumber)) {
            return true; //exclude empty/null string.
        }

        if (tempNumber.startsWith("1")) {
            tempNumber = tempNumber.substring(1);
        } else if (tempNumber.startsWith("+1")) {
            tempNumber = tempNumber.substring(2);
        }

        if (TextUtils.isEmpty(tempNumber)) {
            return true; //exclude empty/null string.
        }

        for (String num : sExcludes) {
            if (tempNumber.startsWith(num)) {
                return true;
            }
        }

        return false;
    }

    /**
     * Format Number Result.
     */
    public class FormatNumberResult {

        private int mResultCode;
        private String mFormatNumber;

        /**
        * Constructor.
        * @param resultCode result code.
        * @param formatNumber format number.
        */
        public FormatNumberResult(int resultCode, String formatNumber) {
            mResultCode = resultCode;
            mFormatNumber = formatNumber;
        }

        /**
        * Constructor.
        * @param formatNumber format number.
        */
        public FormatNumberResult(String formatNumber) {
            mResultCode = NUMBER_INVALID;
            mFormatNumber = formatNumber;
        }

        public int getResultCode() {
            return mResultCode;
        }

        public String getFormatNumber() {
            return mFormatNumber;
        }

        public void setResultCode(int resultCode) {
            mResultCode = resultCode;
        }

        public void setFormatNumber(String formatNumber) {
            mFormatNumber = formatNumber;
        }

        /**
        * isValidNumber.
        * @return true or false.
        */
        public boolean isValidNumber() {
            boolean result = false;
            if (NUMBER_VALID == mResultCode) {
                result = true;
            }
            return result;
        }
    }

}

