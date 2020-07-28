/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package verizon.net.sip;


import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.net.sip.SipException;
import android.net.Uri;
import android.util.Log;
import android.os.HwBinder;
import android.os.SystemClock;
import android.os.RemoteException;

/*import android.os.IBinder;
import android.os.ServiceManager;
import android.telephony.TelephonyManager;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.LinkProperties;
import android.net.RouteInfo;
import com.mediatek.ims.internal.IMtkImsService;
import com.mediatek.ims.internal.MtkImsManager;
*/
import java.util.HashMap;
import java.util.ArrayList;
import java.net.InetAddress;
import java.util.concurrent.atomic.AtomicLong;
import vendor.mediatek.hardware.clientapi.V1_0.IClientapi;
import vendor.mediatek.hardware.clientapi.V1_0.IClientapiIndication;

public class SipDialog {

    private static final String TAG = "SipDialogImpl";
    //private static IMtkImsService mMtkImsService = null;
    //private static INetworkManagementService mNetd = null;
    //private int mPhoneNum;
    public Context mContext;
    private Listener mListener = null;
    private volatile IClientapi mClientapiHal;
    private final IClientapiDeathRecipient mIClientapiDeathRecipient = new IClientapiDeathRecipient();
    private final AtomicLong mClientapiHalCookie = new AtomicLong(0);
    private PendingIntent mIncomingMessagePendingIntent;
    private ClientapiIndication mClientapiIndication = new ClientapiIndication();

    public SipDialog(Context InContext, PendingIntent incomingMessagePendingIntent) throws SipException{
        mContext = InContext;
        mIncomingMessagePendingIntent = incomingMessagePendingIntent;
        mListener = new Listener();
        mListener.setOwner(this);
        if(!StartConnection()){
            Log.e (TAG, "SipDialog Hidl connection fail");
        }
        //for setupdMediaPath
        //test put extra function
        //Log.v (TAG, "putExtra testing:");
        //mListener.onSipReceived("");
        /*try {
            mIncomingMessagePendingIntent.send();
        } 
        catch (PendingIntent.CanceledException e) {
            Log.e(TAG,"PendingIntent cancel: " + e);
        }*/
    }

    public class Listener  {
        private SipDialog mOwner;
        public void setOwner(SipDialog owner) {
            mOwner = owner;
        }
        public void onSipReceived(String sipMessage){
            Log.v(TAG,"onSipReceived ::(" + sipMessage);
            CallFilterSipSend(mContext,"content://com.cequint.ecid/lookup",sipMessage);
            try {
                Intent result = new Intent();
                result.putExtra("sip_content", sipMessage);
                mIncomingMessagePendingIntent.send(mContext,0,result);
            }
            catch (PendingIntent.CanceledException e) {
                Log.e(TAG,"PendingIntent cancel: " + e);
            }
        }
    }

    public void setListener(HashMap<String,String> filters,SipDialog.Listener listener) { //HashMap<String,String[]>
        Log.v(TAG, "Client api TC: SipDialog setListener" );
        mListener = listener;
    }

    private boolean StartConnection(){
        Log.v(TAG,"connectClientAPIProxy");

        for (int count = 0 ; count < 10 ; count++) {
            try {
                Log.v(TAG,"getting clientapi_hal_service...(" + count + ")");
                mClientapiHal = IClientapi.getService("clientapi_hal_service");
                if (mClientapiHal != null) {
                    /* linkToDeath */
                    mClientapiHal.linkToDeath(mIClientapiDeathRecipient,
                                             mClientapiHalCookie.incrementAndGet());

                    /* setResponseFunctions */
                    Log.v(TAG,"setResponseFunctions");
                    mClientapiHal.setResponseFunctions(mClientapiIndication);

                    /* start domain event dispatcher to recieve broadcast */
                    //Log.v(TAG,"initSetup");
                    //initSetup();
                    break;
                } else {
                    Log.v(TAG,"getService fail");
                    return false;
                }
            } catch (RemoteException | RuntimeException e) {
                mClientapiHal = null;
                Log.v(TAG,"enable Clientapi hal error: " + e);
            }
            SystemClock.sleep(1000 * (count + 1));
        }
        //mEventQueue.startEventQueuePolling();
        return true;
    }

    final class IClientapiDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            Log.e(TAG,"Clientapi hal serviceDied");
            // reconnect to hidl server directly
            // let the for loop take care UA reborn timing
            StartConnection();
        }
    }

    public class ClientapiIndication extends IClientapiIndication.Stub {
        private byte[] arrayListTobyte(ArrayList<Byte> data, int length) {
            byte[] byteList = new byte[length];
            for(int i = 0; i < length; i++) {
                byteList[i] = data.get(i);
            }
            Log.v(TAG,"arrayListTobyte, byteList = " + byteList);
            return byteList;
        }

        public void readEvent(ArrayList<Byte> data, int request_id, int length) {
            Log.v(TAG,"<< readEvent(" + length + "):" + " request_id = " + request_id);
            byte buf [];
            String msg="empty";
            try{
                buf = arrayListTobyte(data, length);
                msg = new String(buf, "UTF-8");
            } catch (Throwable ex){
                Log.e (TAG, "SipMediaCall initial fail");
            }
            Log.v(TAG,"readEvent->"+msg);

            mListener.onSipReceived(msg);
            /*logger.debug("<< readEvent(" + length + "):" + " request_id = " + request_id);

            byte buf [];
            RcsUaEvent event;

            buf = arrayListTobyte(data, length);

            event = new RcsUaEvent(request_id);
            event.putBytes(buf);

            if (event != null) {
                mEventQueue.addEvent(RCSProxyEventQueue.INCOMING_EVENT_MSG, event);
            }*/
        }
    }


    public String sendSIP(InetAddress imsServer,int port, String sipMessage) {
        Log.v(TAG,"sendSIP"+sipMessage);
        if (sipMessage == null) {
            Log.v(TAG,"sendMsgToClientapiUAProxy fail, event is null");
            return "";
        }
        if (mClientapiHal == null) {
            Log.v(TAG,"sendMsgToClientapiUAProxy fail, mClientapiHal is null");
            return "";
        }
        // send the event to UA
        int requestId = 0;//event.getRequestID();
        int length = sipMessage.length();//event.getDataLen();
        byte [] data = sipMessage.getBytes();//event.getData();
        try {
            Log.v(TAG,"sendMsgToClientapiUAProxy with Id"+requestId+"len"+length+"data"+data);
            mClientapiHal.writeEvent(requestId, length, byteToArrayList(length, data));
        } catch (RemoteException e) {
            Log.e(TAG,"hal writeEvent e: " + e);
        }
        //Log.v (TAG, "putExtra testing:");
        //mListener.onSipReceived(sipMessage);
        return sipMessage;
    }

    /*public String sendSIP(String requestUri, InetAddress imsServer, SipRequest sipMessage,String to,String from) {
        return "";
    }*/

    //need an API to setupMediaPath, for 3rd app to pass datathrough IMS PDN
    //Allows an application to instruct the IMS engine
    //to setup a static route over IMS APN to the other party/parties in a dialog.
    //This is required to allow applications to send data directly
    //to each party or an application server over IMS.
    public void setupMediaPath(String remoteIP) { //String remoteIP[]
        /*int netId; //IMS network Id
        LinkProperties lp; //IMS network's LinkProperties
        InetAddress addr; // remotIP()

        IBinder bIms = ServiceManager.getService(MtkImsManager.MTK_IMS_SERVICE);
        mMtkImsService = IMtkImsService.Stub.asInterface(bIms);
        if (mMtkImsService == null) {
             return;
        }
        mPhoneNum = TelephonyManager.getDefault().getPhoneCount();
        int[] getImsState = new int[mPhoneNum];
        try {
            getImsState = mMtkImsService.getImsNetworkState(NetworkCapabilities.NET_CAPABILITY_IMS);
            if (!(getImsState[0] == NetworkInfo.State.CONNECTED.ordinal())) {
                return;
            }
        } catch(RemoteException e) {
            return;
        }

        IBinder bNet = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNetd = INetworkManagementService.Stub.asInterface(bNet);
        if (mNetd == null) {
             return;
        }

        addr.getByAddress(remoteIP);
        IBinder b = ServiceManager.getService(Context.NETWORKMANAGEMENT_SERVICE);
        mNMService = INetworkManagementService.Stub.asInterface(b);
        INetworkManagementService mNetd;
        RouteInfo bestRoute = RouteInfo.selectBestRoute(lp.getAllRoutes(), addr);
        if (bestRoute == null) {
            bestRoute = RouteInfo.makeHostRoute(addr, lp.getInterfaceName());
        } else {
            String iface = bestRoute.getInterface();
            if (bestRoute.getGateway().equals(addr)) {
                // if there is no better route, add the implied hostroute for our gateway
                bestRoute = RouteInfo.makeHostRoute(addr, iface);
            } else {
                // if we will connect to this through another route, add a direct route
                // to it's gateway
                bestRoute = RouteInfo.makeHostRoute(addr, bestRoute.getGateway(), iface);
            }
        }
        if (DBG) log("Adding legacy route " + bestRoute +
                " for UID/PID " + Binder.getCallingUid() + "/" + Binder.getCallingPid());
        try {
            mNetd.addLegacyRouteForNetId(netId, bestRoute, Binder.getCallingUid());
        } catch (Exception e) {
            // never crash - catch them all
            if (DBG) loge("Exception trying to add a route: " + e);
            return;
        }*/
    }

    public void CallFilterSipSend(Context context,String uri, String sipMessage){
        Log.v(TAG,"CallFilterSipSend (Uri :" + uri + ", msg :" + sipMessage);
        ContentResolver theResolver = context.getContentResolver();
        //byte[] SIPMessageBytes = readSipMessage(); // get sip message as byte[]
        ContentValues values = new ContentValues();
        values.put("sipmessage", sipMessage); //new String(sipMessage, “ISO-8859-1”
        try{
            Uri insertUri = Uri.parse(uri);
            Uri returnUri = theResolver.insert(insertUri, values);
            if (returnUri == null){
                //Assume no caller data available
                Log.v(TAG,"Insert Fail : not an INVITE message, is not a well formed INVITE message, or does not have any ECID associated data");
            }
            else{
                //Go ahead and do ECIDContentProvider.query() to get caller data.
                Log.v(TAG,"Insert Success ");
            }
        }
        catch (Exception ex){
            Log.d(TAG, "Exception in CallFilterSipSend calling ContentResolver.insert() "+ ex);
        }
    }

    private ArrayList<Byte> byteToArrayList(int length, byte [] value) {
        ArrayList<Byte> al = new ArrayList<Byte>();
        Log.v(TAG,"byteToArrayList, value.length = " + value.length + ", length = " + length);
        for(int i = 0; i < length; i++) {
            al.add(value[i]);
        }
        return al;
    }
}
