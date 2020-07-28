package com.mediatek.optin.parser;

import android.content.Context;
import android.location.CountryDetector;
import android.os.Build;
import android.os.SystemProperties;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.i18n.phonenumbers.NumberParseException;
import com.android.i18n.phonenumbers.PhoneNumberUtil;
import com.android.i18n.phonenumbers.PhoneNumberUtil.PhoneNumberFormat;
import com.android.i18n.phonenumbers.Phonenumber.PhoneNumber;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

import android.net.Uri;
import android.database.Cursor;

/** Class to make xml request body.
 */
public class XmlConstructor {

    private static final String TAG = "OP12XmlConstructor";

    private static final String APP_TOKEN_PATH = "content://com.verizon.loginclient/token";
    private static final int APP_TOKEN_COULMN_INDEX = 0;
    private static final Uri APP_TOKEN_URI = Uri.parse(APP_TOKEN_PATH);
    private static final String PROP_APP_TOKEN_REQUIRED = "persist.vendor.apptoken.required";

    // TODO: device name, os type
    private static final String DEVICE_NAME = "MediaTek";
    private static final String OS_TYPE = "Android";
    private static final String OS_VERSION = Build.VERSION.RELEASE;
    /* Doc: Reqs-Voice overWiFi, section#2(Opt-in and State management), point#17 */
    private static final String IMEI_STRING = "VOWIFI";

    private static Context sContext;

    /** Makes xml request body as per request.
     * @param context context
     * @param reqId Add/query/partial Validation
     * @param req userAddress
     * @return xml request body
     */
    public static String makeXml(Context context, int reqId, RequestResponse.Address req) {
        sContext = context;
        String requestXml = null;
        switch (reqId) {
            case RequestResponse.QUERY_REQUEST:
                requestXml = makeQueryXml(reqId);
                break;
            case RequestResponse.ADD_REQUEST:
                requestXml = makeAddXml(reqId, req, false);
                break;
            case RequestResponse.PARTIAL_VALIDATION_REQUEST:
                requestXml = makeAddXml(reqId, req, true);
                break;
            case RequestResponse.DELETE_REQUEST:
         //       requestXml = makeDeleteRequestXml();
                break;
            default:
                Log.d(TAG, "Invalid requestId: " + reqId);
                break;
        }
        return requestXml;
    }

    private static String makeQueryXml(int reqId) {
        StringBuilder sb = new StringBuilder();
        sb.append(addXmlHeader().toString());
        sb.append("<E911LocationManagementSvc xmlns:ssf=\"http://ssf.vzw.com/common.xsd\""
                + " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                + " xsi:noNamespaceSchemaLocation=\"Address.xsd\">");
            sb.append(makeSvcHdr(reqId).toString());
            sb.append(makeSvcBdy(reqId, null, false).toString());
        sb.append("</E911LocationManagementSvc>");

        return sb.toString();
    }

    private static String makeAddXml(int reqId, RequestResponse.Address req,
            boolean isPartialValidation) {
        StringBuilder sb = new StringBuilder();
        sb.append(addXmlHeader().toString());
        sb.append("<E911LocationManagementSvc xmlns:ssf=\"http://ssf.vzw.com/common.xsd\""
                + " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
                + " xsi:noNamespaceSchemaLocation=\"Address.xsd\">");
            sb.append(makeSvcHdr(reqId).toString());
            sb.append(makeSvcBdy(reqId, req, isPartialValidation).toString());
        sb.append("</E911LocationManagementSvc>");

        return sb.toString();
    }

    private static StringBuilder addXmlHeader() {
        StringBuilder sb = new StringBuilder();
        sb.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        return sb;
    }

    private static StringBuilder makeSvcHdr(int reqId) {
        StringBuilder sb = new StringBuilder();
        sb.append("<ssf:SvcHdr>");
            sb.append(makeSvcInfo(reqId).toString());
            sb.append(makeTransInfo().toString());
            sb.append(makeDvcInfo().toString());
        sb.append("</ssf:SvcHdr>");
        return sb;
    }

    private static StringBuilder makeSvcInfo(int reqId) {
        StringBuilder sb = new StringBuilder();
        sb.append("<ssf:SvcInfo>");
            sb.append("<ssf:svcNm>E911Address</ssf:svcNm>");
            if (reqId == RequestResponse.QUERY_REQUEST) {
                sb.append("<ssf:subSvcNm>retrieveAddresses</ssf:subSvcNm>");
            } else if (reqId == RequestResponse.ADD_REQUEST) {
                sb.append("<ssf:subSvcNm>addAddressesW91</ssf:subSvcNm>");
            }
        sb.append("</ssf:SvcInfo>");
        return sb;
    }

    private static StringBuilder makeTransInfo() {
        StringBuilder sb = new StringBuilder();
        sb.append("<ssf:TransInfo>");
            sb.append("<ssf:timeStamp>" + getTimeStamp() + "</ssf:timeStamp>");
        sb.append("</ssf:TransInfo>");
        return sb;
    }

    private static StringBuilder makeDvcInfo() {
        StringBuilder sb = new StringBuilder();
        sb.append("<ssf:DvcInfo>");
            sb.append("<ssf:dvcName>" + getDeviceName() + "</ssf:dvcName>");
            sb.append("<ssf:osType>" + getOsType() + "</ssf:osType>");
            sb.append("<ssf:osVersion>" + getOsversion() + "</ssf:osVersion>");
        sb.append("</ssf:DvcInfo>");
        return sb;
    }

    private static StringBuilder makeSvcBdy(int reqId, RequestResponse.Address address,
            boolean isPartialValidation) {
        StringBuilder sb = new StringBuilder();
        sb.append("<SvcBdy>");
            sb.append(makeSvcReq(reqId, address, isPartialValidation).toString());
        sb.append("</SvcBdy>");
        return sb;
    }

    private static StringBuilder makeSvcReq(int reqId, RequestResponse.Address address,
            boolean isPartialValidation) {
        StringBuilder sb = new StringBuilder();
        sb.append("<SvcReq>");
            sb.append("<requestId>" + getRequestId(reqId) + "</requestId>");
            sb.append("<appToken>" + getAppToken() + "</appToken>");
            sb.append(makeUserDetailList(reqId, address, isPartialValidation).toString());
        sb.append("</SvcReq>");
        return sb;
    }

    private static StringBuilder makeUserDetailList(int reqId, RequestResponse.Address address,
            boolean isPartialValidation) {
        StringBuilder sb = new StringBuilder();
        sb.append("<userDetailList>");
            sb.append(makeUserDetail(reqId, address, isPartialValidation).toString());
        sb.append("</userDetailList>");
        return sb;
    }


    private static StringBuilder makeUserDetail(int reqId, RequestResponse.Address address,
            boolean isPartialValidation) {
        StringBuilder sb = new StringBuilder();
        sb.append("<UserDetail>");
            sb.append("<reqType>" + getReqString(reqId) + "</reqType>");
            sb.append("<isPartialValidation>" + (isPartialValidation ? "Y" : "N")
                    + "</isPartialValidation>");
            sb.append("<mdn>" + getMdn() + "</mdn>");
            sb.append("<imei>" + getImei() + "</imei>");
            /* uuid not reuired for SP */
            //sb.append("<uuid>" + getUuid() + "</uuid>");
            /* address will be null when query request is being made */
            if (address != null) {
                sb.append(makeAddress(address).toString());
            }
        sb.append("</UserDetail>");
        return sb;
    }

    private static StringBuilder makeAddress(RequestResponse.Address address) {
        StringBuilder sb = new StringBuilder();
        sb.append("<Address>");
            sb.append("<houseNumber>" + address.houseNumber + "</houseNumber>");
            sb.append("<road>" + address.road + "</road>");
            sb.append("<city>" + address.city + "</city>");
            sb.append("<state>" + address.state + "</state>");
            sb.append("<zip>" + address.zip + "</zip>");
            sb.append("<location>" + address.location + "</location>");
            sb.append("<country>" + address.country + "</country>");
        sb.append("</Address>");
        return sb;
    }

    private static String getRequestId(int reqId) {
        Log.i(TAG, "getRequestId: " + reqId);
        switch (reqId) {
        case RequestResponse.QUERY_REQUEST:
            return "1";
        case RequestResponse.ADD_REQUEST:
            return "2";
        default:
            Log.e(TAG, "Invalid requestId: " + reqId);
        return "1";
    }
    }

    private static String getReqString(int reqId) {
        switch (reqId) {
            case RequestResponse.QUERY_REQUEST:
                return "QUERY";
            case RequestResponse.ADD_REQUEST:
                return "ADD";
            case RequestResponse.DELETE_REQUEST:
                return "DELETE";
            default:
                Log.e(TAG, "Invalid requestId: " + reqId);
                return null;
        }
    }

    private static String getDeviceName() {
        return DEVICE_NAME;
    }

    private static String getOsType() {
        return OS_TYPE;
    }

    private static String getOsversion() {
        return OS_VERSION;
    }

    private static String getTimeStamp() {
        DateFormat df = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'");
        Date dateobj = new Date();
        String ts = df.format(dateobj);
        return ts;
    }

    private static String getAppToken() {
        Cursor cursor = sContext.getContentResolver().query(APP_TOKEN_URI, null, null, null, null);
        String appToken = "";
        if (cursor == null) {
            Log.e(TAG, "Cursor is null causing getAppToken() fail");
        } else {
            appToken = cursor.getString(APP_TOKEN_COULMN_INDEX);
            Log.d(TAG, "App token is : " + appToken);
        }
        if (!isAppTokenRequired()) {
            appToken = "";
        }
        return appToken;
    }

    private static String getMdn() {
        TelephonyManager telephonyManager = TelephonyManager.from(sContext);
        //return telephonyManager.getCdmaMdn(SubscriptionManager.getDefaultVoiceSubscriptionId());
        String mdn = telephonyManager.getMsisdn(SubscriptionManager.getDefaultVoiceSubscriptionId());
        Log.d(TAG, "getMdn entry: " + mdn);
        String countryIso;
        CountryDetector detector = (CountryDetector) sContext.getSystemService(
                Context.COUNTRY_DETECTOR);
        if (detector != null && detector.detectCountry() != null) {
            countryIso = detector.detectCountry().getCountryIso();
        } else {
            Log.d(TAG, "getMdn countrydetector is not found");
            Locale locale = sContext.getResources().getConfiguration().locale;
            countryIso = locale.getCountry();
        }
        PhoneNumberUtil util = PhoneNumberUtil.getInstance();
        String result = null;
        try {
            PhoneNumber pn = util.parseAndKeepRawInput(mdn, countryIso);
            result = util.format(pn, PhoneNumberUtil.PhoneNumberFormat.NATIONAL);
        } catch (NumberParseException e) {
        Log.d(TAG, "getMdn exception: ");
        if(mdn.startsWith("+")) {
            mdn = mdn.substring(1);
        }
        return mdn;
    }
        result = result.trim();
        result = PhoneNumberUtils.stripSeparators(result);
        Log.d(TAG, "getMdn exit: " + result);
        return result;
    }

    private static String getImei() {
        //TelephonyManager telephonyManager = TelephonyManager.from(sContext);
        //return telephonyManager.getImei();
        return IMEI_STRING;
    }

    private static String getUuid() {
        return "123456";
    }

    private static boolean isAppTokenRequired() {
        return SystemProperties.getInt(PROP_APP_TOKEN_REQUIRED, 1) == 1 ? true : false;
    }
}
