package com.mediatek.ims.internal;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.location.Address;
import android.location.Geocoder;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;
import com.mediatek.ims.ImsAdapter;
import com.mediatek.ims.ImsAdapter.VaEvent;
import com.mediatek.ims.ImsAdapter.VaSocketIO;
import com.mediatek.ims.ImsEventDispatcher;
import com.mediatek.ims.VaConstants;


import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * WfcDispatcher handles all requests from each UA (reg, call, etc) and IMCB for VoWiFi purpose.
 * <p>
 * The flow path would be: UA(reg, call) <-> IMCB <-> IMSM <-> IMSA <-> WfcDispatcher.
 * <p>
 * Since IMCB/IMSM are relay modules, so the event's data structure defined as:
 * <pre>{@code
 *     UA_IMSA_Request_Data {
 *         UA_Msg_Hdr_t    ua_msg_hdr;    // used for relay modules: IMCB/IMSM.
 *
 *         UINT32 type_id; // used for end-to-end modules: all UAs/WfcDispatcher.
 *                         // See WfcDispatcher: UA <-> WfcDispatcher Message ID definition.
 *         Byte[] payload; // length is defined by different msgs.
 *     }
 *
 *     UA_Msg_Hdr_t {
 *         imcf_uint8 transaction_id; // 0 is valid for one-way to UA
 *         imcf_uint8 pad[3];
 *         imcf_uint8 data[0];
 *     }
 * }</pre>
 * <p>
 * All kinds of these data structure will be sent by IMSM as a VaEvent,
 * and all VaEvent for WfcDispatcher have the same request ID: MSG_ID_REQUEST_VOWIFI_RELATED_INFO,
 * and WfcDispatcher will response all these VaEvents with ID: MSG_ID_RESPONSE_VOWIFI_RELATED_INFO.
 */
public class WfcDispatcher implements ImsEventDispatcher.VaEventDispatcher {
    private static final String TAG = "Wfc-IMSA";
    private static final boolean DEBUG = false;

    // android.provider.Settings.Global.WFC_AID_VALUE;
    private static final String AID_SETTING_URI_STR = "wfc_aid_value";
    private static final Uri AID_SETTING_URI = Settings.Global.getUriFor(AID_SETTING_URI_STR);

    /* UA <-> WfcDispatcher Message ID definition for each VoWiFi related information request. */

    // Geo Location: the payload defined as GeoLocationTask and handled in handleGeoLocationInfo().
    private static final int MSG_REG_IMSA_REQUEST_GEO_LOCATION_INFO = 96009;
    private static final int MSG_REG_IMSA_RESPONSE_GETO_LOCATION_INFO = 91030;

    private static final String ACTION_LOCATED_PLMN_CHANGED
            = "com.mediatek.intent.action.LOCATED_PLMN_CHANGED";
    private static final String EXTRA_ISO = "iso";

    /* UA <-> WfcDispatcher Message ID definition end */

    private static class GeoLocationTask {
        int phoneId;
        int transactionId;

        // for MSG_REG_IMSA_REQUEST_GEO_LOCATION_INFO and
        // MSG_REG_IMSA_RESPONSE_GETO_LOCATION_INFO.
        //int typeId; //int_32 same as msg id.

        // Payload definition.
        double latitude; // int_64
        double longitude; // int_64
        int accuracy;
        String method = ""; // char[16]
        String city = ""; // char[32]
        String state = ""; // char[32]
        String zip = ""; // char [8]
        String countryCode = ""; // char[8]

        @Override
        public String toString() {
            return "[GeoLocationTask objId: " + hashCode() +
                    ", phoneId: " + phoneId +
                    ", transactionId: " + transactionId +
                    //", latitude: " + latitude +
                    //", longitude: " + longitude +
                    ", accuracy: " + accuracy +
                    ", method: " + method +
                    ", city: " + city +
                    ", state: " + state +
                    ", zip: " + zip +
                    ", countryCode: " + countryCode;
        }
    }

    private static final BlockingQueue<Runnable> sPoolWorkQueue = new LinkedBlockingDeque<>();
    private static final ThreadFactory sThreadFactory = new ThreadFactory() {
        private final AtomicInteger mCount = new AtomicInteger(1);
        public Thread newThread(Runnable r) {
            return new Thread(r, "WFC #" + mCount.getAndIncrement());
        }
    };
    private static final ThreadPoolExecutor sPoolExecutor = new ThreadPoolExecutor(
            0, 3, 30, TimeUnit.SECONDS, sPoolWorkQueue, sThreadFactory);

    private List<GeoLocationTask> mNetworkLocationTasks = new ArrayList<>();

    private Context mContext;
    private VaSocketIO mSocket;

    private String mAid;

    private boolean mImsEnabled = false;

    private String mPlmnCountryCode = "";

    private LocationListener mLocationListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {
            log("onLocationChanged: " + location);
            cancelNetworkGeoLocationRequest();
            mHandler.obtainMessage(EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE,
                location).sendToTarget();
        }

        @Override
        public void onProviderDisabled(String provider) {
            log("onProviderDisabled: " + provider);
        }

        @Override
        public void onProviderEnabled(String provider) {
            log("onProviderEnabled: " + provider);
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            log("onStatusChanged: " + provider + ", status=" + status);
        }
    };

    private static final int NETWORK_LOCATION_UPDATE_TIME = 1000;
    private LocationManager mLocationManager;

    private class SettingsObserver extends ContentObserver {
        public SettingsObserver(Handler handler) {
            super(handler);
        }

        private void register() {
            mContext.getContentResolver()
                    .registerContentObserver(AID_SETTING_URI, false, this);
        }

        private void unregister() {
            mContext.getContentResolver().unregisterContentObserver(this);
        }

        @Override
        public void onChange(boolean selfChange) {
            onChange(selfChange, null);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            if (AID_SETTING_URI.equals(uri)) {
                mAid = Settings.Global.getString(
                        mContext.getContentResolver(), AID_SETTING_URI_STR);
                log("Receive AID changed from Setting, AID=" + mAid);
                mHandler.obtainMessage(EVENT_MSG_UPDATE_AID_INFORMATION).sendToTarget();
            }
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent == null || intent.getAction() == null) {
                return;
            }
            log("onReceive action:" + intent.getAction());
            if (intent.getAction().equals(ACTION_LOCATED_PLMN_CHANGED)) {
                String lowerCaseCountryCode = (String) intent.getExtra(EXTRA_ISO);
                if (lowerCaseCountryCode != null) {

                    mPlmnCountryCode = lowerCaseCountryCode.toUpperCase();
                    log("ACTION_LOCATED_PLMN_CHANGED, iso: " + mPlmnCountryCode);

                } else {
                    log("iso country code is null");
                }
            }
        }
    };

    /* Event Message definition for handler */
    private static final int EVENT_MSG_REQUEST_GEO_LOCATION = 0;
    private static final int EVENT_MSG_REQUEST_NETWORK_LOCATION = 1;
    private static final int EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE = 2;
    private static final int EVENT_MSG_RESPONSE_GEO_LOCATION = 4;
    private static final int EVENT_MSG_UPDATE_AID_INFORMATION = 5;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;
            Object obj = msg.obj;

            log("handleMessage: msg=" + handlerEventMsgToString(what));

            switch (what) {
                case EVENT_MSG_REQUEST_GEO_LOCATION: {
                    GeoLocationTask locationReq = (GeoLocationTask) obj;
                    log("push GeoLocation task transaction-" +
                        locationReq.transactionId + " to queue");
                    sPoolExecutor.execute(new Runnable() {
                        @Override
                        public void run() {
                            log(" start for transaction-" + locationReq.transactionId);
                            updateGeoLocationFromLatLong(locationReq);
                            obtainMessage(EVENT_MSG_RESPONSE_GEO_LOCATION,
                                locationReq).sendToTarget();
                        }
                    });
                    break;
                }

                case EVENT_MSG_REQUEST_NETWORK_LOCATION: {
                    GeoLocationTask locationReq = (GeoLocationTask) obj;
                    mNetworkLocationTasks.add(locationReq);

                    if (!getLastKnownLocation(locationReq)) {
                        mNetworkLocationTasks.remove(locationReq);
                        log("getLastKnownLocation failed");
                        this.obtainMessage(EVENT_MSG_RESPONSE_GEO_LOCATION, locationReq)
                                .sendToTarget();
                    }

                    /*
                    if (!requestGeoLocationFromNetworkLocation()) {
                        mNetworkLocationTasks.remove(locationReq);
                        log("requestGeoLocationFromNetworkLocation failed");

                        obtainMessage(EVENT_MSG_RESPONSE_GEO_LOCATION, locationReq).sendToTarget();
                    } */

                    break;
                }

                case EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE: {
                    Location location = (Location) obj;
                    if (location == null) {
                        log("network location get null, unexpected result");
                        return;
                    }

                    double latitude = location.getLatitude();
                    double longitude = location.getLongitude();
                    log("update all GeoLocationTask");

                    for (GeoLocationTask locationTask : mNetworkLocationTasks) {
                        locationTask.latitude = latitude;
                        locationTask.longitude = longitude;
                        log("Get network location, send EVENT_MSG_REQUEST_GEO_LOCATION for"
                                + " transactionId-" + locationTask.transactionId);
                        obtainMessage(EVENT_MSG_REQUEST_GEO_LOCATION, locationTask).sendToTarget();
                    }
                    mNetworkLocationTasks.clear();
                    break;
                }

                case EVENT_MSG_RESPONSE_GEO_LOCATION: {
                    GeoLocationTask locationRsp = (GeoLocationTask) obj;
                    log("finish for transaction-" + locationRsp.transactionId);
                    handleGeoLocationResponse(locationRsp);
                    break;
                }

                case EVENT_MSG_UPDATE_AID_INFORMATION: {
                    handleAidInfoUpdate();
                    break;
                }

                default:
                    Log.w(TAG, "Unhandled message: " + handlerEventMsgToString(what));
                    break;
            }
        }
    };

    public WfcDispatcher(Context context, VaSocketIO IO) {
        log("WfcDispatcher()");
        mContext = context;
        mSocket = IO;
        mLocationManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);

        new SettingsObserver(null).register();

        registerForBroadcast();

        log("WfcDispatcher() end");
    }

    private void registerForBroadcast() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_LOCATED_PLMN_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
    }

    public void enableRequest(int phoneId) {
        log("enableRequest()");

        mImsEnabled = true;

        mAid = Settings.Global.getString(mContext.getContentResolver(), AID_SETTING_URI_STR);
        log("Trigger AID information update to IMCB, AID=" + mAid);
        mHandler.sendMessageDelayed(mHandler.obtainMessage(EVENT_MSG_UPDATE_AID_INFORMATION), 1000);
    }

    public void disableRequest(int phoneId) {
        log("disableRequest()");

        mImsEnabled = false;

        mNetworkLocationTasks.clear();
        // TODO: shutdown thread pool
    }

    public void vaEventCallback(VaEvent event) {
        try {
            int requestId = event.getRequestID();
            log("vaEventCallback: ID=" + imsaMsgToString(requestId) + "(" + requestId + ")");

            switch (requestId) {
                case VaConstants.MSG_ID_REQUEST_VOWIFI_RELATED_INFO: {
                    parseRequestDataPayload(event);
                    break;
                }

                default:
                    log("Unknown request, return directly ");
                    break;
            }
        } catch (Exception e) {
            Log.e(TAG, "Event exception", e);
        }
    }

    protected void log(String s) {
        if (DEBUG){
            Log.d(TAG, s);
        }
    }

    private void parseRequestDataPayload(VaEvent event) {
        int transactionId = event.getByte();
        event.getBytes(3);  //pad[3]
        int uaMsgId = event.getInt();

        log("parseRequestDataPayload: transaction-" + transactionId + ", uaMsgId="
                + uaMsgIdToString(uaMsgId) + "(" + uaMsgId + ")");
        switch (uaMsgId) {
            case MSG_REG_IMSA_REQUEST_GEO_LOCATION_INFO:
                handleGeoLocationRequest(transactionId, event);
                break;
            default:
                log("parseRequestDataPayload: unknown msgId");
                break;
        }
    }

    /**
     * Geo Location
     **/
    private void handleGeoLocationRequest(int transactionId, VaEvent event) {
        GeoLocationTask locationReq = new GeoLocationTask();
        locationReq.phoneId = event.getPhoneId();
        locationReq.transactionId = transactionId;
        locationReq.latitude = event.getDouble();
        locationReq.longitude = event.getDouble();
        locationReq.accuracy = event.getInt();

        log("handleGeoLocationRequest: get UA's request: " + locationReq);
        if (locationReq.latitude != 0 && locationReq.longitude != 0) {
            log("send EVENT_MSG_REQUEST_GEO_LOCATION for transactionId-" +
                locationReq.transactionId);

            locationReq.method = "GPS";
            mHandler.obtainMessage(EVENT_MSG_REQUEST_GEO_LOCATION, locationReq).sendToTarget();
        } else {
            log("send EVENT_MSG_REQUEST_NETWORK_LOCATION for transactionId-" +
                locationReq.transactionId);

            locationReq.method = "Network";
            mHandler.obtainMessage(EVENT_MSG_REQUEST_NETWORK_LOCATION, locationReq).sendToTarget();
        }
    }

    private void handleGeoLocationResponse(GeoLocationTask locationResult) {
        log("send " + imsaMsgToString(VaConstants.MSG_ID_RESPONSE_VOWIFI_RELATED_INFO) + "("
                + VaConstants.MSG_ID_RESPONSE_VOWIFI_RELATED_INFO
                + ") to IMSM: result=" + locationResult);

        ImsAdapter.VaEvent event = new ImsAdapter.VaEvent(
                locationResult.phoneId, VaConstants.MSG_ID_RESPONSE_VOWIFI_RELATED_INFO);
        event.putByte(locationResult.transactionId);
        event.putBytes(new byte[3]);

        event.putInt(MSG_REG_IMSA_RESPONSE_GETO_LOCATION_INFO);
        event.putDouble(locationResult.latitude);
        event.putDouble(locationResult.longitude);
        event.putInt(locationResult.accuracy);

        String method = locationResult.method == null? "":locationResult.method;
        event.putString(method, 16);

        String city = locationResult.city == null? "":locationResult.city;
        event.putString(city, 32);

        String state = locationResult.state == null? "Unknown":locationResult.state;
        event.putString(state, 32);

        String zip = locationResult.zip == null? "":locationResult.zip;
        event.putString(zip, 8);

        String countryCode = locationResult.countryCode == null? "":locationResult.countryCode;
        event.putString(countryCode, 8);

        writeEventToSocket(event);
    }

    /**
     * This function will be executed in worker thread.
     */
    private void updateGeoLocationFromLatLong(GeoLocationTask location) {
        if (!Geocoder.isPresent()) {
            log("getGeoLocationFromLatLong: this system has no GeoCoder implementation!!");
            return;
        }

        List<Address> lstAddress = null;
        try {
            Geocoder geocoder = new Geocoder(mContext, Locale.US);
            lstAddress = geocoder.getFromLocation(location.latitude, location.longitude, 1);
        } catch (IOException e) {
            Log.e(TAG, "geocoder.getFromLocation throw exception:", e);
        }

        if (lstAddress == null || lstAddress.isEmpty()) {
            log("getGeoLocationFromLatLong: get empty address, fill plmn:" + mPlmnCountryCode);
            location.countryCode = mPlmnCountryCode;
            return;
        }

        Address address = lstAddress.get(0);
        location.city = address.getLocality();
        if (location.city == null || location.city.equals("")) {
            location.city = address.getSubAdminArea();
        }
        location.state = address.getAdminArea();  // 'CA';
        if (location.state == null || location.state.equals("")) {
            location.state = lstAddress.get(0).getCountryName();
        }
        location.zip = address.getPostalCode();
        location.countryCode = address.getCountryCode();

        // update plmn country code
        if (location.countryCode != null && !location.countryCode.equals("")) {
            mPlmnCountryCode = location.countryCode;
        }

        log("getGeoLocationFromLatLong: location=" + location);
    }

    private boolean getLastKnownLocation(GeoLocationTask locationReq) {
        log("getLastKnownLocation");

        if (mLocationManager == null) {
            log("getLastKnownLocation: empty locationManager, return");
            return false;
        }

        // Get cached GPS location
        Location gpsLocation =
            mLocationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);

        if (gpsLocation != null) {
            locationReq.method = "GPS";

            log("GPS location: " + gpsLocation);

            mHandler.obtainMessage(EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE,
                 gpsLocation).sendToTarget();
            return true;
        }

        // Get cached network location
        Location networkLocation =
            mLocationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);

        if (networkLocation != null) {
            locationReq.method = "Network";

            log("Network location: " + networkLocation);

             mHandler.obtainMessage(EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE,
                 networkLocation).sendToTarget();
             return true;
        }

        log("getLastKnownLocation: no last known location");
        return false;
    }

    private boolean requestGeoLocationFromNetworkLocation() {
        if (mLocationManager == null) {
            log("getGeoLocationFromNetworkLocation: empty locationManager, return");
            return false;
        }

        String optr = SystemProperties.get("persist.vendor.operator.optr");
        do {
            if (null != optr && optr.equals("OP08")) {
                if (mLocationManager.getProvider(LocationManager.NETWORK_PROVIDER) != null) {
                    break;
                } else {
                    log("requestGeoLocationFromNetworkLocation:getProvider() is null!");
                    return false;
                }
            }

            if (!mLocationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER)) {
                log("requestGeoLocationFromNetworkLocation:"
                        + "this system has no networkProvider implementation!");
                return false;
            }
        } while (false);

        addPackageInLocationSettingsWhitelist();
        LocationRequest request = LocationRequest.createFromDeprecatedProvider(
                LocationManager.NETWORK_PROVIDER, NETWORK_LOCATION_UPDATE_TIME /*minTime*/,
                0 /*minDistance*/, false/*oneShot*/);
        request.setHideFromAppOps(true);
        request.setLocationSettingsIgnored(true);
        mLocationManager.requestLocationUpdates(request, mLocationListener, null);
        Log.d(TAG, "requestGeoLocationFromNetworkLocation");

        return true;
    }

    private void cancelNetworkGeoLocationRequest() {
        if (mLocationManager == null) {
            log("cancelNetworkGeoLocationRequest: empty locationManager, return");
            return;
        }
        mLocationManager.removeUpdates(mLocationListener);
        removePackageInLocationSettingsWhitelist();
        Log.d(TAG, "cancelNetworkGeoLocationRequest");
    }

    private void addPackageInLocationSettingsWhitelist() {
        final String LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST =
                "location_ignore_settings_package_whitelist";
        String packageName = mContext.getPackageName();

        String whitelist = Settings.Global.getString(
                mContext.getContentResolver(),
                LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST);
        if (whitelist == null || whitelist.indexOf(packageName) == -1) {
            String outStr = (whitelist == null) ? "" : whitelist + ",";
            outStr += packageName;
            log("Add WFC in location setting whitelist:" + outStr);
            Settings.Global.putString(
                    mContext.getContentResolver(),
                    LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST,
                    outStr);
        }
    }

    private void removePackageInLocationSettingsWhitelist() {
        final String LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST =
                "location_ignore_settings_package_whitelist";
        String packageName = mContext.getPackageName();

        String whitelist = Settings.Global.getString(
                mContext.getContentResolver(),
                LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST);
        int index = -1;
        String outStr = "";
        if (whitelist != null) {
            index = whitelist.indexOf("," + packageName);
            if (index != -1) { /// found ','+package
                outStr = whitelist.replace("," + packageName, "");
            } else { /// not found, try to find package name only
                index = whitelist.indexOf(packageName);
                if(index != -1) {
                    outStr = whitelist.replace(packageName, "");
                }
            }
        }

        if (index != -1) { /// outStr is replaced as new whitelist
            log("Remove WFC in location setting whitelist:" + outStr);
            Settings.Global.putString(
                    mContext.getContentResolver(),
                    LOCATION_IGNORE_SETTINGS_PACKAGE_WHITELIST,
                    outStr);
        }
    }

    private void handleAidInfoUpdate() {
        if (mSocket == null) {
            log("handleAidInfoUpdate: socket is null, can't send AID info.");
            return;
        }

        log("send " + imsaMsgToString(VaConstants.MSG_ID_UPDATE_IMCB_AID_INFO) + "("
                + VaConstants.MSG_ID_UPDATE_IMCB_AID_INFO
                + ") to IMSM: AID=" + mAid);

        ImsAdapter.VaEvent event = new ImsAdapter.VaEvent(
                ImsAdapter.Util.getDefaultVoltePhoneId(), VaConstants.MSG_ID_UPDATE_IMCB_AID_INFO);

        String aid = mAid!=null?mAid:"";
        event.putString(aid, 32);
        writeEventToSocket(event);
    }

    private void writeEventToSocket(ImsAdapter.VaEvent event) {
        if (mImsEnabled && mSocket != null && event != null) {
            mSocket.writeEvent(event);
        } else {
            Log.e(TAG, "Event discarded:" + event);
        }
    }

    private String imsaMsgToString(int msgId) {
        switch (msgId) {
            case VaConstants.MSG_ID_REQUEST_VOWIFI_RELATED_INFO:
                return "MSG_ID_REQUEST_VOWIFI_RELATED_INFO";
            case VaConstants.MSG_ID_RESPONSE_VOWIFI_RELATED_INFO:
                return "MSG_ID_RESPONSE_VOWIFI_RELATED_INFO";
            case VaConstants.MSG_ID_UPDATE_IMCB_AID_INFO:
                return "MSG_ID_UPDATE_IMCB_AID_INFO";
            default:
                return "Unknown Msg";
        }
    }

    private String uaMsgIdToString(int uaMsgId) {
        switch (uaMsgId) {
            case MSG_REG_IMSA_REQUEST_GEO_LOCATION_INFO:
                return "MSG_REG_IMSA_REQUEST_GEO_LOCATION_INFO";
            case MSG_REG_IMSA_RESPONSE_GETO_LOCATION_INFO:
                return "MSG_REG_IMSA_RESPONSE_GETO_LOCATION_INFO";
            default:
                return "Unknown Msg";
        }
    }

    private String handlerEventMsgToString(int eventMsg) {
        switch (eventMsg) {
            case EVENT_MSG_REQUEST_GEO_LOCATION:
                return "EVENT_MSG_REQUEST_GEO_LOCATION";
            case EVENT_MSG_REQUEST_NETWORK_LOCATION:
                return "EVENT_MSG_REQUEST_NETWORK_LOCATION";
            case EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE:
                return "EVENT_MSG_HANDLE_NETWORK_LOCATION_RESPONSE";
            case EVENT_MSG_RESPONSE_GEO_LOCATION:
                return "EVENT_MSG_RESPONSE_GEO_LOCATION";
            case EVENT_MSG_UPDATE_AID_INFORMATION:
                return "EVENT_MSG_UPDATE_AID_INFORMATION";
            default:
                return "EVENT_MSG_ID-" + eventMsg;
        }
    }
}
