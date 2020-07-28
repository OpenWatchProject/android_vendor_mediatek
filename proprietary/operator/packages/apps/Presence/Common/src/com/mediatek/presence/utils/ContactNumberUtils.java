package com.mediatek.presence.utils;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.os.Build;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.utils.SimUtils;
import com.mediatek.presence.utils.logger.Logger;

import com.mediatek.internal.telephony.MtkPhoneNumberFormatUtil;
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
     * int result = mNumberUtils.validate(number);
     * if (ContactNumberUtils.NUMBER_VALID == result) {
     * }
     */

    /**
     * Contact nubmer error code.
     */
    public static int NUMBER_VALID = 0;
    public static int NUMBER_EMERGENCY = 1;
    public static int NUMBER_SHORT_CODE = 2;
    public static int NUMBER_PRELOADED_ENTRY = 3;
    public static int NUMBER_FREE_PHONE = 4;
    public static int NUMBER_INVALID = 5;
    public static int NUMBER_INTERNATIONAL = 6;

    /**
     * The logger related.
     */
    private static final boolean DEBUG = Build.IS_DEBUGGABLE;
    private static final String TAG = "ContactNumberUtils";
    private Logger logger = Logger.getLogger(TAG);

    /**
     * Contact number length.
     */
    public static final int NUMBER_LENGTH_NORMAL = 10;
    public static final int NUMBER_LENGTH_NO_AREA_CODE = 7;

    /**
     * Save the singleton instance.
     */
    private static ContactNumberUtils sInstance = null;

    private Context mContext = null;

    /**
     * Constructor
     */
    private ContactNumberUtils() {
        if (DEBUG) {
            logger.debug("ContactNumberUtils constructor");
        }
    }

    public static ContactNumberUtils getDefault() {
        if(sInstance == null) {
            sInstance = new ContactNumberUtils();
        }

        return sInstance;
    }

    public void setContext(Context context) {
        mContext = context;
    }

    /**
     * Check if it is a valid contact number for presence
     *
     * @param phoneNumber read from contact db.
     * @return contact number error code.
     */
    public boolean isValidNumber(final String phoneNumber) {
        int validateResult = validate(phoneNumber);
        logger.debug("validateResult = " + validateResult
                + "for phoneNumber: " + phoneNumber);
        return NUMBER_VALID == validateResult
                || NUMBER_INTERNATIONAL == validateResult;
    }

    /**
     * Format contact number to the common format
     *
     * @param phoneNumber read from contact db.
     * @return formatted contact number.
     */
    public String format(final String phoneNumber) {
        String number = phoneNumber;
        if (TextUtils.isEmpty(number)) {
            return null;
        }

        number = PhoneNumberUtils.stripSeparators(number);

        int len = number.length();
        if (len == NUMBER_LENGTH_NO_AREA_CODE) {
            if (!number.startsWith("1")) {
                number = "000" + number;
            }
        }

        number = PhoneNumberUtils.normalizeNumber(number);
        len = number.length();
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
        } else {
            if (len > NUMBER_LENGTH_NORMAL) {
                if (number.startsWith("011")) {
                    number = "+" + number.substring(3);
                } else {
                    if (!number.startsWith("1")) {
                        number = "+1" + number;
                    } else {
                        number = "+" + number;
                    }
                }
            }
        }
        logger.debug("orignal: " + phoneNumber + " after format: " + number);
        return number;
    }

    /**
     * Check if it is a valid contact number for presence
     *
     * @param phoneNumber read from contact db.
     * @return contact number error code.
     */
    public int validate(final String phoneNumber) {
        String number = phoneNumber;
        if (TextUtils.isEmpty(number)) {
            return NUMBER_INVALID;
        }

        logger.debug("validate, number: " + phoneNumber);

        if(number.startsWith("+")) {
            number = number.substring(1);
            // check international number
            // if is global international number return NUMBER_INTERNATIONAL
            int type = MtkPhoneNumberFormatUtil.getFormatTypeForLocale(null);
            if (type > 0
                    && !number.startsWith(MtkPhoneNumberFormatUtil.FORMAT_COUNTRY_CODES[type - 1])) {
                for (String contryCode : MtkPhoneNumberFormatUtil.FORMAT_COUNTRY_CODES) {
                    if (number.startsWith(contryCode)) {
                        return NUMBER_INTERNATIONAL;
                    }
                }
            }
        }
        if(number.contains("*")) {
            return NUMBER_PRELOADED_ENTRY;
        }

        number = PhoneNumberUtils.stripSeparators(number);
        if (!number.equals(PhoneNumberUtils.convertKeypadLettersToDigits(number))) {
            return NUMBER_INVALID;
        }

        if (PhoneNumberUtils.isEmergencyNumber(number)) {
            return NUMBER_EMERGENCY;
        // TODO: To handle short code
        //} else if ((mContext != null) && PhoneNumberUtils.isN11Number(mContext, number)) {
        //    return NUMBER_SHORT_CODE;
        } else if (number.startsWith("#")) {
            return NUMBER_PRELOADED_ENTRY;
        } else if (isInExcludedList(number)) {
            return NUMBER_FREE_PHONE;
        }

        int len = number.length();
        if (len > "+1-nnn-nnn-nnnn".length()) {
            return NUMBER_INVALID;
        } else if (len < NUMBER_LENGTH_NO_AREA_CODE) {
            return NUMBER_INVALID;
        }

        number = format(number);
        if (number.startsWith("+1")) {
            len = number.length();
            if (len == NUMBER_LENGTH_NORMAL + 2) {
                return NUMBER_VALID;
            }
            //Fix ALPS04844611 VzW Eab 3.01 test issue
            return NUMBER_VALID;
        }

        return NUMBER_INVALID;
    }

    /**
     * The excluded number list.
     */
    private static ArrayList<String> sExcludes = null;

    private boolean isInExcludedList(String number){
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
        if(TextUtils.isEmpty(tempNumber)) {
            return true; //exclude empty/null string.
        }

        if(tempNumber.startsWith("1")) {
            tempNumber = tempNumber.substring(1);
        } else if(tempNumber.startsWith("+1")) {
            tempNumber = tempNumber.substring(2);
        }

        if(TextUtils.isEmpty(tempNumber)) {
            return true; //exclude empty/null string.
        }

        for (String num : sExcludes) {
            if(tempNumber.startsWith(num)) {
                return true;
            }
        }

        return false;
    }
}

