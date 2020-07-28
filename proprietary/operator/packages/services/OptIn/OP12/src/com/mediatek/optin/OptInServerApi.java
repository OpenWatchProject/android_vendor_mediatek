package com.mediatek.optin;

import android.accounts.NetworkErrorException;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.optin.ErrorCodes;
import com.mediatek.optin.parser.EmergencyAddressParser;
import com.mediatek.optin.parser.RequestResponse;
import com.mediatek.optin.parser.XmlConstructor;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.SocketTimeoutException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Random;

import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

/** Class interacting with network.
 */
public class OptInServerApi {

    static final String TAG = "OP12OptInServerApi";
    // TODO: get actual URL
    private static final String SERVER_URL = "https://spg.vzw.com";
    private static final int CONNECTION_TIMER = 15000;

    /** Interface for network response callback.
     */
    public interface OptInResponseCallback {
        /** Query response callback method.
         * @param rsp response
         * @return
         */
        public void onQueryAddressResponse(RequestResponse.QueryAddressResponse rsp);
        /** Add response callback method.
         * @param rsp response
         * @return
         */
        public void onAddUpdateAddressResponse(RequestResponse.AddUpdateAddressResponse rsp);
        // For future development
        //public void onDeleteAddressResponse();

        /** Network error callback method.
         * @param cause error cause
         * @return
         */
        public void onNetworkError(int cause);
    }

    private static Random sRandomGenerator = new Random();
    private Context mContext;
    private static OptInServerApi sOptInServerApi;

    private HandlerThread mThread;
    private Handler mThreadHandler;
    private volatile ConnectivityManager mConnectivityManager;
    private TelephonyManager mTelephonyManager;
    private NetworkRequest mNetworkRequest = null;
    private Network mNetwork = null;
    private int mSubId = 0;
    // The callback to register when we request PDN network
    private ConnectivityManager.NetworkCallback mNetworkCallback;
    // Timeout used to call ConnectivityManager.requestNetwork
    // Given that the telephony layer will retry on failures, this timeout should be high enough.
    private static final int NETWORK_REQUEST_TIMEOUT_MILLIS = 30 * 60 * 1000;
    // Wait timeout for this class, a little bit longer than the above timeout
    // for dual ct ,the pdn timer set to 1min
    private static final int NETWORK_REQUEST_TIMEOUT_MILLIS_FOR_DUAL_CT = 1 * 60 * 1000;
    private static final int NETWORK_ACQUIRE_TIMEOUT_MILLIS =
            NETWORK_REQUEST_TIMEOUT_MILLIS + (5 * 1000);
    // Waiting time used before releasing a network prematurely. This allows the MMS download
    // acknowledgement messages to be sent using the same network that was used to download the data
    private static final int NETWORK_RELEASE_TIMEOUT_MILLIS = 5 * 1000;

    private int mCurrentTransactionId = -1;

    /** Method to get OptInServerApi instance.
     * @param context context
     * @return OptInServerApi
     */
    public static OptInServerApi getInstance(Context context) {
        if (sOptInServerApi == null) {
            sOptInServerApi = new OptInServerApi(context);
        }
        return sOptInServerApi;
    }

    private OptInServerApi(Context context) {
        mContext = context;

        mThread = new HandlerThread("EmergencyAddressNwIntreface-worker");
        mThread.start();
        mThreadHandler = new Handler(mThread.getLooper());
    }

    /** Method to query E911 Address from server.
     * @param rspCallback rspCallback
     * @return
     */
    public void queryE911Address(OptInResponseCallback rspCallback) {
        Log.d(TAG, "Query request");
        sendRequest(RequestResponse.QUERY_REQUEST, null, rspCallback);
    }

    /** Method to add E911 address at server.
     * @param userDetails userDetails
     * @param rspCallback rspCallback
     * @return
     */
    public void addAddress(RequestResponse.Address userDetails,
            OptInResponseCallback rspCallback) {
        Log.d(TAG, "Add, request: " + userDetails);
        sendRequest(RequestResponse.ADD_REQUEST, userDetails, rspCallback);
    }

    /** Method to perform partial validation of E911 address.
     * @param userDetails userDetails
     * @param rspCallback rspCallback
     * @return
     */
    public void performPartialValidationAddress(RequestResponse.Address userDetails,
        OptInResponseCallback rspCallback) {
        Log.d(TAG, "PartialValidation, request: " + userDetails);
        sendRequest(RequestResponse.PARTIAL_VALIDATION_REQUEST, userDetails, rspCallback);
    }

    /** Method to delete E911 address at server.
     * @param userDetails userDetails
     * @param rspCallback rspCallback
     * @return
     */
    public void deleteAddress(RequestResponse.Address userDetails,
            OptInResponseCallback rspCallback) {
        Log.d(TAG, "Delete, request: " + userDetails);
        sendRequest(RequestResponse.DELETE_REQUEST, userDetails, rspCallback);
    }

    /** Method to cancel sent request.
     * @return
     */
    public synchronized void cancelRequest() {
        // Cannot close the ongoing connection as the thread on which its processing is blocked.
        // So, set Transaction id as negative, as genuine id will be >0.
        Log.d(TAG, "Cancel request");
        setCurrentTransactionId(-1);
        releaseRequestLocked(mNetworkCallback);
    }


    /**
     * Acquire the PDN network for WifiCalling Opt-in
     *
     * @param requestId request ID for logging
     * @throws java.lang.Exception if we fail to acquire it
     */
    public void acquireNetwork() throws Exception {
        Log.d(TAG, "PDN Opt-in: acquireNetwork start");
        synchronized (this) {
            // Since we are acquiring the network, remove the network release task if exists.
            if (mNetwork != null) {
                Log.d(TAG, "PDN Opt-in: acquireNetwork already exists");
                return;
            }
            mNetworkRequest = new NetworkRequest.Builder()
            .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
            .addCapability(NetworkCapabilities.NET_CAPABILITY_FOTA)
            .setNetworkSpecifier(Integer.toString(mSubId)).build();
            if(checkWifiOnAndConnected()) {
                mNetworkRequest = new NetworkRequest.Builder()
                .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                .addCapability(NetworkCapabilities.NET_CAPABILITY_FOTA)
                .setNetworkSpecifier(Integer.toString(mSubId)).build();
            }
            // Not available, so start a new request if not done yet
            if (mNetworkCallback == null) {
                Log.d(TAG, "PDN Opt-in: start new network request");
                startNewNetworkRequestLocked();
            }
            final long shouldEnd = SystemClock.elapsedRealtime() + NETWORK_ACQUIRE_TIMEOUT_MILLIS;
            long waitTime = NETWORK_ACQUIRE_TIMEOUT_MILLIS;
            while (waitTime > 0) {
                try {
                    this.wait(waitTime);
                } catch (InterruptedException e) {
                    Log.w(TAG, "acquireNetwork: acquire network wait interrupted");
                }
                if (mNetwork != null) {
                    Log.d(TAG, "PDN Opt-in: acquire network success");
                    return;
                }
                // Calculate remaining waiting time to make sure we wait the full timeout period
                waitTime = shouldEnd - SystemClock.elapsedRealtime();
            }
            // Timed out, so release the request and fail
            Log.e(TAG, "PDN Opt-in: timed out");
            throw new Exception("Acquiring network timed out");
        }
    }

    /**
     * Network callback for our network request
     */
    private class NetworkRequestCallback extends ConnectivityManager.NetworkCallback {
        @Override
        public void onAvailable(Network network) {
            super.onAvailable(network);
            synchronized (OptInServerApi.this) {
                mNetwork = network;
                Log.i(TAG, "NetworkCallbackListener.onAvailable: network=" + network);
                OptInServerApi.this.notifyAll();
            }
        }

        @Override
        public void onLost(Network network) {
            super.onLost(network);
            synchronized (OptInServerApi.this) {
                mNetwork = null;
                Log.i(TAG, "NetworkCallbackListener.onLost: network=" + network);
                OptInServerApi.this.notifyAll();
            }
        }

        @Override
        public void onUnavailable() {
            super.onUnavailable();
            synchronized (OptInServerApi.this) {
                mNetwork = null;
                Log.i(TAG, "NetworkCallbackListener.onUnavailable: network=");
                OptInServerApi.this.notifyAll();
            }
        }
    }
    /**
     * Start a new {@link android.net.NetworkRequest} for Opt-in
     */
    private void startNewNetworkRequestLocked() {
        final ConnectivityManager connectivityManager = getConnectivityManager();
        mNetworkCallback = new NetworkRequestCallback();
        //long subId = SubscriptionManager.getDefaultSubId();
        //Log.d(TAG,"newRequest subid = " + subId);
        int networkType = getTelephonyManager().getDataNetworkType();
        Log.d(TAG,"newRequest networkType = " + networkType);
        if (TelephonyManager.NETWORK_TYPE_UNKNOWN == networkType) {
            connectivityManager.requestNetwork(
                    mNetworkRequest, mNetworkCallback, NETWORK_REQUEST_TIMEOUT_MILLIS_FOR_DUAL_CT);
        } else {
            connectivityManager.requestNetwork(
                    mNetworkRequest, mNetworkCallback, NETWORK_REQUEST_TIMEOUT_MILLIS);
        }

    }

    private boolean checkWifiOnAndConnected() {
        WifiManager wifiMgr = (WifiManager) mContext.getSystemService(Context.WIFI_SERVICE);
        if (wifiMgr.isWifiEnabled()) {
            WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
            if( wifiInfo.getNetworkId() == -1 ){
                Log.d(TAG, "Not connected to an access point");
                return false;
            }
            Log.d(TAG, "Connected to wifi access point");
            return true;
        }
        else {
            Log.d(TAG, "Wi-Fi adapter is OFF");
            return false;
        }
    }

    /**
     * Release the current {@link android.net.NetworkRequest} for Opt-in
     *
     * @param callback the {@link android.net.ConnectivityManager.NetworkCallback} to unregister
     */
    private void releaseRequestLocked(ConnectivityManager.NetworkCallback callback) {
        if (callback != null) {
            Log.d(TAG, "Opt-in: releaseRequestLocked");
            final ConnectivityManager connectivityManager = getConnectivityManager();
            try {
                connectivityManager.unregisterNetworkCallback(callback);
            } catch (IllegalArgumentException e) {
                Log.w(TAG,"Unregister network callback exception", e);
            }
        }
        mNetworkCallback = null;
        mNetwork = null;
        mNetworkRequest = null;
    }

    private ConnectivityManager getConnectivityManager() {
        if (mConnectivityManager == null) {
            mConnectivityManager = (ConnectivityManager) mContext.getSystemService(
                    Context.CONNECTIVITY_SERVICE);
        }
        return mConnectivityManager;
    }

    private TelephonyManager getTelephonyManager() {
        if (mTelephonyManager == null) {
            mTelephonyManager = (TelephonyManager) mContext.getSystemService(
                    Context.TELEPHONY_SERVICE);
        }
        return mTelephonyManager;
    }

    private void sendRequest(int reqType, RequestResponse.Address userDetails,
            OptInResponseCallback rspCallback) {
        /* Discard next request if one is underprocessing. Entertain one request at a time.
         * To queue new request, first cancel ongoing request.*/
        if (getCurrentTransactionId() > 0) {
            Log.d(TAG, "Transaction:" + getCurrentTransactionId() + " ongoing.Drop new request");
            return;
        }
        int transactionId = getNewTransactionId();
        setCurrentTransactionId(transactionId);
        mSubId = SubscriptionManager.getDefaultVoiceSubscriptionId();
        try {
            acquireNetwork();
        } catch (Exception e1) {
            // TODO Auto-generated catch block
            e1.printStackTrace();
        }
        HttpURLConnection connection = null;
        try {
            URL url = new URL(getServerUrl());
            if (mNetwork == null) {
                Log.d(TAG, "mNetwork is null, so do not send request");
                return;
              }
            connection = (HttpURLConnection) mNetwork.openConnection(url);
            connection.setConnectTimeout(CONNECTION_TIMER);
            connection.setReadTimeout(CONNECTION_TIMER);
            // TODO: add reuqest header
            connection.setRequestMethod("POST");
            connection.setRequestProperty("Content-Type","application/xml");
            //con.setRequestProperty("User-Agent", USER_AGENT);
            connection.setRequestProperty("Accept-Language", "en-US,en;q=0.5");

            String xmlFile = XmlConstructor.makeXml(mContext, reqType, userDetails);
            Log.d(TAG, "xmlFile: " + xmlFile);
            // Send post request
            connection.setDoOutput(true);
            //con.setDoInput(true);
            DataOutputStream wr = new DataOutputStream(connection.getOutputStream());
            wr.writeBytes(xmlFile);
            wr.flush();
            wr.close();
            int responseCode = connection.getResponseCode();
            Log.d(TAG, "TransactionId:" + transactionId + ",Response Code:" + responseCode);
            if (isResponseCodeValid(responseCode)) {
                BufferedReader in = new BufferedReader(
                        new InputStreamReader(connection.getInputStream()));
                String inputLine;
                StringBuffer response = new StringBuffer();

                while ((inputLine = in.readLine()) != null) {
                    response.append(inputLine);
                }
                in.close();
                Log.d(TAG, "Response: " + response.toString());
                sendResponse(transactionId, reqType, response.toString(), rspCallback);
            } else {
                sendErrorResponse(transactionId, responseCode, rspCallback);
            }
        } catch (MalformedURLException e) {
            Log.e(TAG, "MalformedURLException: " + e);
            sendErrorResponse(transactionId, ErrorCodes.E911_ERR_MALFORMED_URL, rspCallback);
        } catch (ProtocolException e) {
            Log.e(TAG, "ProtocolException: " + e);
            sendErrorResponse(transactionId, ErrorCodes.E911_ERR_PROTOCOL_EXP, rspCallback);
        } catch (SocketTimeoutException e) {
            Log.e(TAG, "SocketTimeoutException: " + e);
            sendErrorResponse(transactionId, ErrorCodes.E911_ERR_CODE_TIMEOUT, rspCallback);
        } catch (IOException e) {
            Log.e(TAG, "IOException: " + e);
            sendErrorResponse(transactionId, ErrorCodes.E911_ERR_IO_EXP, rspCallback);
        } finally {
            if (connection != null) {
                connection.disconnect();
            }
            setCurrentTransactionId(-1);
        }
        releaseRequestLocked(mNetworkCallback);
    }

    private void sendResponse(int transactionId, int reqType, String response,
            OptInResponseCallback rspCallback) {
        int currentTransId = getCurrentTransactionId();
        if (transactionId != currentTransId) {
            Log.d(TAG, "sendResponse, transactionId:" + transactionId + ", ongoingTrans:"
                    + currentTransId + ". Drop response");
            return;
        }
        EmergencyAddressParser.ParserData parsedData = parseResponse(response);
        switch (reqType) {
            case RequestResponse.QUERY_REQUEST:
                sendQueryResponse(parsedData, rspCallback);
                break;
            case RequestResponse.PARTIAL_VALIDATION_REQUEST:
            case RequestResponse.ADD_REQUEST:
                sendAddResponse(parsedData, rspCallback);
                break;
            default:
                 Log.d(TAG, "invalid reqType: " + reqType);
                 break;
        }
    }

    private synchronized void sendQueryResponse(EmergencyAddressParser.ParserData parsedData,
           OptInResponseCallback rspCallback) {
        RequestResponse.ErrorCode err = EmergencyAddressParser.getErrDetails(parsedData);
        RequestResponse.UserDetail userDetails = EmergencyAddressParser.getUserDetails(parsedData);
        RequestResponse.QueryAddressResponse rsp
                = new RequestResponse.QueryAddressResponse(err, userDetails);
        Log.d(TAG, "errStatus: " + rsp.errStatus.errCode);
        Log.d(TAG, "userDetails: " + rsp.userDetail);
        if (rspCallback != null) {
            rspCallback.onQueryAddressResponse(rsp);
        }
    }

    private synchronized void sendAddResponse(EmergencyAddressParser.ParserData parsedData,
            OptInResponseCallback rspCallback) {
        RequestResponse.ErrorCode err = EmergencyAddressParser.getErrDetails(parsedData);
        RequestResponse.UserDetail userDetails = EmergencyAddressParser.getUserDetails(parsedData);
        ArrayList<RequestResponse.Address> altAddress = EmergencyAddressParser
                .getAltAddressList(parsedData);
        RequestResponse.AddUpdateAddressResponse rsp
                = new RequestResponse.AddUpdateAddressResponse(err, userDetails, altAddress);
        Log.d(TAG, "errStatus: " + rsp.errStatus);
        Log.d(TAG, "userDetails: " + rsp.userDetail);
        Log.d(TAG, "altAddress: " + rsp.altAddress);
        if (rspCallback != null) {
            rspCallback.onAddUpdateAddressResponse(rsp);
        }
    }

    private synchronized void sendErrorResponse(int transactionId, int error,
            OptInResponseCallback rspCallback) {
        int currentTransId = getCurrentTransactionId();
        if (transactionId != currentTransId) {
            Log.d(TAG, "sendErrorResponse, transactionId:" + transactionId + "ongoingTrans:"
                    + currentTransId);
            return;
        }
        Log.d(TAG, "sendErrorResponse, error: " + error + ", callback:" + rspCallback);
        if (rspCallback != null) {
            rspCallback.onNetworkError(error);
        }
    }

    private EmergencyAddressParser.ParserData parseResponse(String xml) {
        EmergencyAddressParser parser = EmergencyAddressParser.getInstance();
        EmergencyAddressParser.ParserData parsedData = parser.parse(xml);
        return parsedData;
    }

    private boolean isResponseCodeValid(int responseCode) {
        boolean d = (responseCode == HttpURLConnection.HTTP_OK) ? true : false;
        Log.d(TAG, "isResponseCodeValid: " + d);
        return (responseCode == HttpURLConnection.HTTP_OK) ? true : false;
    }

    private String getServerUrl() {
        String sesUrl = SystemProperties.get("persist.vendor.opt-in.url", SERVER_URL);
        Log.d(TAG, "sesUrl = " + sesUrl);
        return sesUrl;
    }

    private synchronized void setCurrentTransactionId(int id) {
        Log.d(TAG, "setCurrentTransactionId: " + id);
        mCurrentTransactionId = id;
    }

    private synchronized int getCurrentTransactionId() {
        return mCurrentTransactionId;
    }

    private synchronized static int getNewTransactionId() {
        return sRandomGenerator.nextInt(10000);
    }
}
