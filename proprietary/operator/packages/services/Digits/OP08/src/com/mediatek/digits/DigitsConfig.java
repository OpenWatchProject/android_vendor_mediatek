package com.mediatek.digits;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import java.io.IOException;
import java.io.StringReader;
import java.lang.Integer;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

public class DigitsConfig {
    static protected final String TAG = "DigitsConfig";

    private static DigitsConfig sDigitsConfig;
    private Context mContext;

    private int mConfigRefreshTime;
    private boolean mConfigRefreshOnPowerUp;
    private boolean mConfigRefreshSIMSwap;
    private boolean mJansky_service;
    private String mEntitlement_server_FQDN;
    private String mEntitlement_server_redirect_URI;
    private String mEntitlement_server_token_URI;
    private String mIAMRedirectURL;
    private String mIAMRedirectSignUpURL;
    private String mIAMOAuthClientIDNSDS;
    private String mIAMOAuthClientIDNSDSScope;
    private String mIAMOAuthClientIDService;
    private int mConnManager_timer;
    private int mVowifi_timer;
    private String mIAMOAuthClientIDServiceScope;
    private String mIAMConsumerProfileURI;
    private String mE911ServerURI;
    private String mWSG_URI;
    private boolean mIAMOAuthRequired;
    private String mCNSMWInitiateURL;
    private String mCNSMWAuthorizeURL;
    private String mCNSMWAuthorizeUpdateURL;
    private String mCNSMWQueryURL;
    private String mCNSMWCancelURL;
    private String mBfEncText;
    private String mGCM_Sender_ID;
    private int mGCM_Update_Token_TTL;

    // Private constructor
    private DigitsConfig(Context context, String deviceConfigStr) {
        mContext = context;
        parseXml(deviceConfigStr);
    }

    public static DigitsConfig getInstance() {
        if (sDigitsConfig != null) {
            return sDigitsConfig;
        }
        return null;
    }

    /**
    * Create instance
    *
    * @param xml string
    */
    public static synchronized void createInstance(Context context, String deviceConfigStr) {
        DigitsConfig util = new DigitsConfig(context, deviceConfigStr);
        sDigitsConfig = util;
    }

    public boolean isJanskyServerAllowed() {

        return mJansky_service;
    }

    public String getEntitlementServerFQDN() {

        return mEntitlement_server_FQDN;
    }

    public String getEntitlementServerRedirectURI() {

        return mEntitlement_server_redirect_URI;
    }

    public String getEntitlementServerTokenURI() {

        return mEntitlement_server_token_URI;
    }

    public String getIAMRedirectURL() {

        return mIAMRedirectURL;
    }

    public String getIAMRedirectSignUpURL() {

        return mIAMRedirectSignUpURL;
    }

    public String getIAMOAuthClientIDNSDS() {

        return mIAMOAuthClientIDNSDS;
    }

    public String getIAMOAuthClientIDNSDSScope() {

        return mIAMOAuthClientIDNSDSScope;
    }

    public String getIAMOAuthClientIDService() {

        return mIAMOAuthClientIDService;
    }

    public int getConnManagerTimer() {

        return mConnManager_timer;
    }

    public int getVowifiTimer() {

        return mVowifi_timer;
    }

    public String getIAMOAuthClientIDServiceScope() {

        return mIAMOAuthClientIDServiceScope;
    }

    public String getIAMConsumerProfileURI() {

        return mIAMConsumerProfileURI;
    }

    public String getE911ServerURI() {

        return mE911ServerURI;
    }

    public String getWSGURI() {

        return mWSG_URI;
    }

    public boolean isIAMOAuthRequired() {

        return mIAMOAuthRequired;
    }

    public String getCNSMWInitiateURL() {

        return mCNSMWInitiateURL;
    }

    public String getCNSMWAuthorizeURL() {

        return mCNSMWAuthorizeURL;
    }

    public String getCNSMWAuthorizeUpdateURL() {

        return mCNSMWAuthorizeUpdateURL;
    }

    public String getCNSMWQueryURL() {

        return mCNSMWQueryURL;
    }

    public String getCNSMWCancelURL() {

        return mCNSMWCancelURL;
    }

    public String getBfEncText() {

        return mBfEncText;
    }

    public String getGCMSenderID() {

        return mGCM_Sender_ID;
    }

    public int getGCMUpdateTokenTTL() {

        return mGCM_Update_Token_TTL;
    }

    public int getConfigRefreshTime() {

        return mConfigRefreshTime;
    }

    public String getPhone2ActivationURL() {

        return null;
    }

    public String getPhone2DeActivationURL() {

        return null;
    }

    public String getMStoreFQDN() {

        return null;
    }

    public boolean isUsePhone20whileRoamingDomestic() {

        return false;
    }

    public boolean isUsePhone20whileRoamingInternational() {

        return false;
    }

    public int getCdPnTimeout() {

        return 0;
    }

    public int getPushSyncDelay() {

        return 0;
    }

    public String getHelpURL() {

        return null;
    }

    public int getTRNTimeout(String type) {

        return 0;
    }

    /*
    public JSONObject getAuthenticateInitiateObject() {

    	return null;
    }

    public JSONObject getAuthenticateAuthorizeObject() {

    	return null;
    }

    public JSONObject GetAuthorizeUpdateObject() {

    	return null;
    }

    public JSONObject getAuthenticateQueryObject() {

    	return null;
    }

    public JSONObject getAuthenticateCancelObject() {

    	return null;
    }
    */

    private void parseXml(String deviceConfigStr) {
        try {
            XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
            factory.setNamespaceAware(true);
            XmlPullParser xpp = factory.newPullParser();
            String tagName = "";
            xpp.setInput( new StringReader(deviceConfigStr) ); // pass input whatever xml you have
            int eventType = xpp.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                if(eventType == XmlPullParser.START_DOCUMENT) {
                    //Log.d(TAG,"Start device-config document");
                } else if(eventType == XmlPullParser.START_TAG) {
                    tagName = xpp.getName();
                    //Log.d(TAG,"Start tag " + tagName);
                    if (tagName.equalsIgnoreCase("configRefreshTime")) {
                        mConfigRefreshTime = Integer.valueOf(xpp.nextText());
                    } else if (tagName.equalsIgnoreCase("configRefreshOnPowerUp")) {
                        mConfigRefreshOnPowerUp = xpp.nextText().equalsIgnoreCase("true");;
                    } else if (tagName.equalsIgnoreCase("configRefreshSIMSwap")) {
                        mConfigRefreshSIMSwap = xpp.nextText().equalsIgnoreCase("true");
                    } else if (tagName.equalsIgnoreCase("jansky_service")) {
                        mJansky_service = xpp.nextText().equalsIgnoreCase("true");
                    } else if (tagName.equalsIgnoreCase("entitlement_server_FQDN")) {
                        mEntitlement_server_FQDN = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("entitlement_server_redirect_URI")) {
                        mEntitlement_server_redirect_URI = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("entitlement_server_token_URI")) {
                        mEntitlement_server_token_URI = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("IAMRedirectURL")) {
                        mIAMRedirectURL = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("IAMRedirectSignUpURL")) {
                        mIAMRedirectSignUpURL = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("IAMOAuthClientIDNSDS")) {
                        mIAMOAuthClientIDNSDS = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("IAMOAuthClientIDNSDSScope")) {
                        mIAMOAuthClientIDNSDSScope = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("IAMOAuthClientIDService")) {
                        mIAMOAuthClientIDService = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("connManager_timer")) {
                        mConnManager_timer = Integer.valueOf(xpp.nextText());
                    } else if (tagName.equalsIgnoreCase("vowifi_timer")) {
                        mVowifi_timer = Integer.valueOf(xpp.nextText());
                    } else if (tagName.equalsIgnoreCase("IAMOAuthClientIDServiceScope")) {
                        mIAMOAuthClientIDServiceScope = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("IAMConsumerProfileURI")) {
                        mIAMConsumerProfileURI = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("E911ServerURI")) {
                        mE911ServerURI = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("WSG_URI")) {
                        mWSG_URI = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("IAMOAuthRequired")) {
                        mIAMOAuthRequired = xpp.nextText().equalsIgnoreCase("true");
                    } else if (tagName.equalsIgnoreCase("url")) {
                        if (xpp.getAttributeValue(null, "name").equalsIgnoreCase("initiate")) {
                            mCNSMWInitiateURL = xpp.getAttributeValue(null, "uri");
                        } else if (xpp.getAttributeValue(null, "name").equalsIgnoreCase("authorize")) {
                            mCNSMWAuthorizeURL = xpp.getAttributeValue(null, "uri");
                        } else if (xpp.getAttributeValue(null, "name").equalsIgnoreCase("authorizeUpdate")) {
                            mCNSMWAuthorizeUpdateURL = xpp.getAttributeValue(null, "uri");
                        } else if (xpp.getAttributeValue(null, "name").equalsIgnoreCase("query")) {
                            mCNSMWQueryURL = xpp.getAttributeValue(null, "uri");
                        } else if (xpp.getAttributeValue(null, "name").equalsIgnoreCase("cancel")) {
                            mCNSMWCancelURL = xpp.getAttributeValue(null, "uri");
                        }
                    } else if (tagName.equalsIgnoreCase("bfEncText")) {
                        mBfEncText = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("GCM_Sender_ID")) {
                        mGCM_Sender_ID = xpp.nextText();
                    } else if (tagName.equalsIgnoreCase("GCM_Update_Token_TTL")) {
                        mGCM_Update_Token_TTL = Integer.valueOf(xpp.nextText());
                    }
                }
                eventType = xpp.next();
            }
            Log.d(TAG,"End device-config document");
        } catch (XmlPullParserException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void dump() {
        Log.d(TAG, "dump " +
              ", mConfigRefreshTime=" + mConfigRefreshTime +
              ", mConfigRefreshOnPowerUp=" + mConfigRefreshOnPowerUp +
              ", mConfigRefreshSIMSwap=" + mConfigRefreshSIMSwap +
              ", mJansky_service=" + mJansky_service +
              ", mEntitlement_server_FQDN=" + mEntitlement_server_FQDN +
              ", mEntitlement_server_redirect_URI=" + mEntitlement_server_redirect_URI +
              ", mEntitlement_server_token_URI=" + mEntitlement_server_token_URI +
              ", mIAMRedirectURL=" + mIAMRedirectURL +
              ", mIAMRedirectSignUpURL=" + mIAMRedirectSignUpURL +
              ", mIAMOAuthClientIDNSDS=" + mIAMOAuthClientIDNSDS +
              ", mIAMOAuthClientIDNSDSScope=" + mIAMOAuthClientIDNSDSScope +
              ", mIAMOAuthClientIDService=" + mIAMOAuthClientIDService +
              ", mConnManager_timer=" + mConnManager_timer +
              ", mVowifi_timer=" + mVowifi_timer +
              ", mIAMOAuthClientIDServiceScope=" + mIAMOAuthClientIDServiceScope +
              ", mIAMConsumerProfileURI=" + mIAMConsumerProfileURI +
              ", mE911ServerURI=" + mE911ServerURI +
              ", mWSG_URI=" + mWSG_URI +
              ", mIAMOAuthRequired=" + mIAMOAuthRequired +
              ", mCNSMWInitiateURL=" + mCNSMWInitiateURL +
              ", mCNSMWAuthorizeURL=" + mCNSMWAuthorizeURL +
              ", mCNSMWAuthorizeUpdateURL=" + mCNSMWAuthorizeUpdateURL +
              ", mCNSMWQueryURL=" + mCNSMWQueryURL +
              ", mCNSMWCancelURL=" + mCNSMWCancelURL +
              ", mBfEncText=" + mBfEncText +
              ", mGCM_Sender_ID=" + mGCM_Sender_ID +
              ", mGCM_Update_Token_TTL=" + mGCM_Update_Token_TTL);
    }

}
