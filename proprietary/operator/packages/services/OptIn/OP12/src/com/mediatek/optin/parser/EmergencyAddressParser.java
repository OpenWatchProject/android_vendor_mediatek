package com.mediatek.optin.parser;

import android.util.Log;
import android.util.Xml;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;

/** Parser class.
 */
public class EmergencyAddressParser {

    private static final String TAG = "OP12EmergencyAddressParser";

    private static final String NONE_TAG = "none";
    private static final String SVC_HDR_TAG = "SvcHdr";
    private static final String SVC_INFO_TAG = "SvcInfo";
    private static final String SVC_SVCNM_TAG = "svcNm";
    private static final String SVC_SUB_SVC_NM_TAG = "subSvcNm";
    private static final String ERR_INFO_TAG = "ErrInfo";
    private static final String ERR_CODE_TAG = "errCd";
    private static final String ERR_LVL_TAG = "errLvl";
    private static final String ERR_MSG_TAG = "errMsg";
    private static final String SVC_BDY_TAG = "SvcBdy";
    private static final String SVC_RESP_TAG = "SvcResp";
    private static final String USER_DETAIL_LIST_TAG = "UserDetailList";
    private static final String USER_DETAIL_TAG = "UserDetail";
    private static final String ALT_ADDR_LIST_TAG = "AlternateAddressList";
    private static final String ALT_ADDR_DETAIL_TAG = "AltAddressDetail";

    private static final String REQ_TYPE_TAG = "reqType";
    private static final String MDN_TAG = "mdn";
    private static final String IMEI_TAG = "imei";
    private static final String UUID_TAG = "uuid";
    private static final String ADDR_TAG = "Address";

    private static final String HOUSE_NUMB_TAG = "houseNumber";
    private static final String ROAD_TAG = "road";
    private static final String LOCATION_TAG = "location";
    private static final String CITY_TAG = "city";
    private static final String STATE_TAG = "state";
    private static final String ZIP_TAG = "zip";
    private static final String COUNTRY_TAG = "country";

    /** Parsed data.
     */
    public class ParserData {
        public String mTagName;
        public String mListType;
        public String mSvcNm;
        public String mSubSvcNm;
        public RequestResponse.ErrorCode mErrCode;
        public RequestResponse.UserDetail mUserDetail;
        public RequestResponse.Address addr;
        private ArrayList<RequestResponse.Address> mAltAddress
                = new ArrayList<RequestResponse.Address>();
    }

    private static EmergencyAddressParser mParser = null;
    private XmlPullParser mXmlParser = null;
    private int mLastEventType = -1;

    /** Method to get parser instance.
     * @return EmergencyAddressParser
     */
    public static EmergencyAddressParser getInstance() {
        if (mParser == null) {
            mParser = new EmergencyAddressParser();
        }
        return mParser;
    }

    private EmergencyAddressParser() {
        mXmlParser = Xml.newPullParser();
    }

    /** Method to start parsing of data given.
     * @param xmlData xmlData
     * @return ParserData ParserData
     */
    public ParserData parse(String xmlData) {

        if (mParser == null) {
            Log.e(TAG, "EmergencyAddressParser is null.");
            return null;
        }
        if(xmlData.contains("ns2:")) {
            Log.d(TAG, "parse xml old = " + xmlData);
            xmlData = xmlData.replaceAll("ns2:", "");
            Log.d(TAG, "parse xml new = " + xmlData);
        }
        ParserData data = new ParserData();
        data.mListType = NONE_TAG;
        try {
            mXmlParser.setInput(new StringReader(xmlData));
            int eventType = -1;
            eventType = mXmlParser.next();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                Log.d(TAG, "eventType = " + eventType);
                switch (eventType) {
                    case XmlPullParser.START_DOCUMENT:
                        break;

                    case XmlPullParser.START_TAG:
                        data.mTagName = mXmlParser.getName();
                        getStartTagData(data);
                        break;

                    case XmlPullParser.END_TAG:
                        data.mTagName = mXmlParser.getName();
                        getEndTagData(data);
                        break;

                    case XmlPullParser.TEXT:
                        if(mLastEventType != XmlPullParser.END_TAG)
                        getTextData(data);
                        break;

                    default:
                        Log.e(TAG, "in Default, eventType is : " + eventType);
                        break;
                }
                mLastEventType = eventType;
                eventType = mXmlParser.next();
                Log.d(TAG, "mXmlParser next event" + eventType + " mLastEventType "+ eventType);
            }
            Log.d(TAG, "eventType outwhile = " + eventType);
        } catch (XmlPullParserException e) {
            Log.e(TAG, "get eventType Exception, e is : " + e);
            data.mAltAddress.clear();
        } catch (IOException e) {
            Log.e(TAG, "get eventType Exception, e is : " + e);
            data.mAltAddress.clear();
        } catch (Exception e) {
            Log.e(TAG, "parse Exception, e is : " + e);
            data.mAltAddress.clear();
        }
        if (EmergencyAddressParser.getErrDetails(data) == null) {
           Log.v(TAG, "err: null");
        } else {
        Log.v(TAG, "error: " + EmergencyAddressParser.getErrDetails(data).toString());
        }
        if (EmergencyAddressParser.getUserDetails(data) == null) {
           Log.v(TAG, "userDetails: null");
        } else {
        Log.v(TAG, "userDetails: " + EmergencyAddressParser.getUserDetails(data).toString());
        }
        return data;
    }

    /** Method to get parsed Svc number.
    * @param data data
     * @return String
     */
    public static String getSvcNm(ParserData data) {
        return (data == null ? null : data.mSvcNm);
    }

    /** Method to get parsed sub Svc number.
     * @param data data
     * @return String
     */
    public static String getSubSvcNm(ParserData data) {
        return (data == null ? null : data.mSubSvcNm);
    }

    /** Method to get parsed error details.
     * @param data data
     * @return RequestResponse.ErrorCode
     */
    public static RequestResponse.ErrorCode getErrDetails(ParserData data) {
        return (data == null ? null : data.mErrCode);
    }

    /** Method to get parsed user details.
     * @param data data
     * @return RequestResponse.UserDetail
     */
    public static RequestResponse.UserDetail getUserDetails(ParserData data) {
        return (data == null ? null : data.mUserDetail);
    }

    /** Method to get parsed list of alternate address.
     * @param data data
     * @return ArrayList<RequestResponse.Address>
     */
    public static ArrayList<RequestResponse.Address> getAltAddressList(ParserData data) {
        return (data == null ? null : data.mAltAddress);
    }

    private void getStartTagData(ParserData parserData) {
            Log.d(TAG, "getStartTagData mTagName = " + parserData.mTagName);
            Log.d(TAG, "getStartTagData mListType = " + parserData.mListType);
            if (parserData.mTagName.equalsIgnoreCase(ERR_INFO_TAG)) {
                // Get the value of the type attribute
                parserData.mErrCode = new RequestResponse.ErrorCode();
            } else if (parserData.mTagName.equalsIgnoreCase(USER_DETAIL_TAG)) {
                // Get the value of the type attribute
                // Get the value of name and value attributes
                parserData.mUserDetail = new RequestResponse.UserDetail();
            } else if (parserData.mTagName.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                // Get the value of the type attribute
                // Get the value of name and value attributes
                parserData.mListType = ALT_ADDR_DETAIL_TAG;
            } else if (parserData.mTagName.equalsIgnoreCase(ADDR_TAG)
                    && parserData.mListType.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                // Get the value of the type attribute
                // Get the value of name and value attributes
                parserData.addr = new RequestResponse.Address();
            }
        }

        private void getEndTagData(ParserData parserData) {
            Log.d(TAG, "getEndTagData mTagName = " + parserData.mTagName);
            Log.d(TAG, "getEndTagData mListType = " + parserData.mListType);
            if (parserData.mTagName.equalsIgnoreCase(ADDR_TAG)) {
                if (parserData.mListType.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                    if (parserData.addr != null) {
                        parserData.mAltAddress.add(parserData.addr);
                        parserData.addr = null;
                    }
                }
            } else if (parserData.mTagName.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                parserData.mListType = NONE_TAG;
            }
        }

        private void getTextData(ParserData parserData) {
            Log.d(TAG, "getTextData mTagName = " + parserData.mTagName);
            Log.d(TAG, "getTextData mListType = " + parserData.mListType);
            Log.d(TAG, "getTextData parsedText = " + mXmlParser.getText());
            if(mXmlParser.getText() !=null) {
                if(mXmlParser.getText().equalsIgnoreCase("") || mXmlParser.getText().isEmpty()) {
                    Log.d(TAG, "getTextData return empty parser ");
                    return;
                }
            } else if(mXmlParser.getText() == null) {
                Log.d(TAG, "getTextData return null ");
                return;
            }
            if (parserData.mTagName.equalsIgnoreCase(SVC_SVCNM_TAG)) {
                parserData.mSvcNm = mXmlParser.getText();
            } else if (parserData.mTagName.equalsIgnoreCase(SVC_SVCNM_TAG)) {
                parserData.mSubSvcNm = mXmlParser.getText();
            } else if (parserData.mTagName.equalsIgnoreCase(ERR_CODE_TAG)) {
                if (parserData.mErrCode != null) {
                    parserData.mErrCode.errCode = mXmlParser.getText();
                    Log.d(TAG, "getTextData ERR_CODE_TAG = " + parserData.mErrCode.errCode);
                } else {
                    Log.d(TAG, "parserData.mErrCode null");
                }
            } else if (parserData.mTagName.equalsIgnoreCase(ERR_LVL_TAG)) {
                if (parserData.mErrCode != null) {
                    parserData.mErrCode.errLevel = mXmlParser.getText();
                }
            } else if (parserData.mTagName.equalsIgnoreCase(ERR_MSG_TAG)) {
                if (parserData.mErrCode != null) {
                    parserData.mErrCode.errMessage = mXmlParser.getText();
                }
            } else if (parserData.mTagName.equalsIgnoreCase(REQ_TYPE_TAG)) {
                if (parserData.mUserDetail != null) {
                    parserData.mUserDetail.reqType = mXmlParser.getText();
                }
            } else if (parserData.mTagName.equalsIgnoreCase(MDN_TAG)) {
                if (parserData.mUserDetail != null) {
                    parserData.mUserDetail.mdn = mXmlParser.getText();
                }
            } else if (parserData.mTagName.equalsIgnoreCase(IMEI_TAG)) {
                if (parserData.mUserDetail != null) {
                    parserData.mUserDetail.imei = mXmlParser.getText();
                }
            } else if (parserData.mTagName.equalsIgnoreCase(UUID_TAG)) {
                if (parserData.mUserDetail != null) {
                    parserData.mUserDetail.uuid = mXmlParser.getText();
                }
            } else if (parserData.mTagName.equalsIgnoreCase(HOUSE_NUMB_TAG)) {
                if (parserData.mListType.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                    if (parserData.addr != null) {
                        Log.d(TAG, "getEndTagData house numb = " + mXmlParser.getText());
                        parserData.addr.houseNumber = mXmlParser.getText();
                    }
                } else {
                    if (parserData.mUserDetail != null
                            && parserData.mUserDetail.userAddress != null) {
                        parserData.mUserDetail.userAddress.houseNumber = mXmlParser.getText();
                    }
                }
            } else if (parserData.mTagName.equalsIgnoreCase(ROAD_TAG)) {
                if (parserData.mListType.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                    if (parserData.addr != null) {
                        parserData.addr.road = mXmlParser.getText();
                    }
                } else {
                    if (parserData.mUserDetail != null
                            && parserData.mUserDetail.userAddress != null) {
                        parserData.mUserDetail.userAddress.road = mXmlParser.getText();
                    }
                }
        } else if (parserData.mTagName.equalsIgnoreCase(LOCATION_TAG)) {
            if (parserData.mListType.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                if (parserData.addr != null) {
                    parserData.addr.location = mXmlParser.getText();
                }
            } else {
                if (parserData.mUserDetail != null && parserData.mUserDetail.userAddress != null) {
                    parserData.mUserDetail.userAddress.location = mXmlParser.getText();
                }
            }
            } else if (parserData.mTagName.equalsIgnoreCase(CITY_TAG)) {
                if (parserData.mListType.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                    if (parserData.addr != null) {
                        parserData.addr.city = mXmlParser.getText();
                    }
                } else {
                    if (parserData.mUserDetail != null
                            && parserData.mUserDetail.userAddress != null) {
                        parserData.mUserDetail.userAddress.city = mXmlParser.getText();
                    }
                }
            } else if (parserData.mTagName.equalsIgnoreCase(STATE_TAG)) {
                if (parserData.mListType.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                    if (parserData.addr != null) {
                        parserData.addr.state = mXmlParser.getText();
                    }
                } else {
                    if (parserData.mUserDetail != null
                            && parserData.mUserDetail.userAddress != null) {
                        parserData.mUserDetail.userAddress.state = mXmlParser.getText();
                    }
                }
            } else if (parserData.mTagName.equalsIgnoreCase(ZIP_TAG)) {
                if (parserData.mListType.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                    if (parserData.addr != null) {
                        parserData.addr.zip = mXmlParser.getText();
                    }
                } else {
                    if (parserData.mUserDetail != null
                            && parserData.mUserDetail.userAddress != null) {
                        parserData.mUserDetail.userAddress.zip = mXmlParser.getText();
                    }
                }
            } else if (parserData.mTagName.equalsIgnoreCase(COUNTRY_TAG)) {
                if (parserData.mListType.equalsIgnoreCase(ALT_ADDR_DETAIL_TAG)) {
                    if (parserData.addr != null) {
                        parserData.addr.country = mXmlParser.getText();
                    }
                } else {
                    if (parserData.mUserDetail != null
                            && parserData.mUserDetail.userAddress != null) {
                        parserData.mUserDetail.userAddress.country = mXmlParser.getText();
                    }
                }
            }
        }
}
