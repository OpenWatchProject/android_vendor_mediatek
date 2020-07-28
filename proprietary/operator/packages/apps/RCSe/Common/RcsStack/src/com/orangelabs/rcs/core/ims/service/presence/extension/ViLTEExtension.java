package com.orangelabs.rcs.core.ims.service.presence.extension;

import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.settings.RcsSettings;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.wifi.WifiManager;
//import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.net.ConnectivityManager;


public class ViLTEExtension extends PresenceExtension {

    public final String TAG = "ViLTEExtension";
    boolean isActive;
    boolean isDuplex;
    private int currentNetworkType; 
    
    private volatile BroadcastReceiver ViLTEServiceListener = null; 

    //network type 
    private final int NONE_NETWORK = 0; //2g
    private final int GERAN_NETWORK = 1; //2g
    private final int UTRAN_NETWORK = 2; //3g
    private final int EUTRAN_NETWORK = 3; //LTE
    private final int EPDG_WIFI_NETWORK = 4;//EPDG OVER WIFI
    
    public ViLTEExtension(){
     super(PresenceExtension.EXTENSION_VILTE);
         extensionName = TAG;
         isActive = false;
         isDuplex = false;
         currentNetworkType = NONE_NETWORK;
    }

    
    @Override
    public void attachExtension(PresenceExtensionListener listener) {
        // TODO Auto-generated method stub
        logger.debug("attachExtension : " +extensionName);
        
       //intialize the value 
        super.attachExtension(listener);
        
        currentNetworkType = getNetworkType();
        
        currentNetworkType =  EPDG_WIFI_NETWORK;
        logger.debug("currentNetworkType : " +currentNetworkType);

        //initialize values 
        if(RcsSettings.getInstance().isIR94VideoCallSupported())
        {
            if((currentNetworkType == EPDG_WIFI_NETWORK) || (currentNetworkType == EUTRAN_NETWORK)){
            isActive = true;
            isDuplex = true;
        }
        }

       //register the vilte listener
       //registerViLTEListener();
    }

    @Override
    public void detachExtension() {
        
        logger.debug("detachExtension : " +extensionName);
        // TODO Auto-generated method stub
        super.detachExtension();
        
        //deregister the vilte listener 
        //deRegisterViLTEListener();
    }

    
    public void registerViLTEListener(){
        if(ViLTEServiceListener == null)
        {
            ViLTEServiceListener = new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    String action = intent.getAction();
                    // TODO Auto-generated method stub
                    /*if (action.equals(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED)){
                        
                    }else*/ 
                    if (action.equals(ConnectivityManager.CONNECTIVITY_ACTION)){
                        
                        int newNetwork = getNetworkType();
                        //if it goes from LTE/EPDG  ---> 2G/3G SET ACTIVE TO FALSE
                        if(
                       ((currentNetworkType == EUTRAN_NETWORK) || (currentNetworkType == EPDG_WIFI_NETWORK)) 
                        &&(newNetwork == NONE_NETWORK || newNetwork == GERAN_NETWORK || newNetwork == UTRAN_NETWORK)
                        )
                        {
                            isActive = false;
                            isDuplex = false;
                            
                            currentNetworkType = newNetwork;
                            notifyListener();
                        }
                        
                        //if it goes from 2G/3G SET --> LTE/EPDG  set ACTIVE TO true
                        else if(
                            (currentNetworkType == NONE_NETWORK || currentNetworkType == GERAN_NETWORK || currentNetworkType == UTRAN_NETWORK)
                            && ((newNetwork == EUTRAN_NETWORK) || (newNetwork == EPDG_WIFI_NETWORK)) 
                                )
                                {
                                    isActive = true;
                                    isDuplex = true;
                                    
                                    currentNetworkType = newNetwork;
                                    notifyListener();
                        }
                    }
                    
                }
            };
            
            /*IntentFilter filter = new IntentFilter();
            //filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);
            //filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
            filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
            AndroidFactory.getApplicationContext().registerReceiver(ViLTEServiceListener,filter);*/ 
        }
    }
    
    public void deRegisterViLTEListener()
    {
        if(ViLTEServiceListener!=null)
        {
            AndroidFactory.getApplicationContext().unregisterReceiver(ViLTEServiceListener);
        }
        
        isActive = false;
        isDuplex = false;
        ViLTEServiceListener = null;
        currentNetworkType = NONE_NETWORK;
    }
    
    public boolean isActive()
    {
        return isActive;
    }
    
    public boolean isDuplex()
    {
        return isDuplex;
    }


    @Override
    public void notifyListener() {
        // TODO Auto-generated method stub
        super.notifyListener();
    }
    
    private int getNetworkType() {
        ConnectivityManager connectivityMgr = (ConnectivityManager)AndroidFactory.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connectivityMgr.getActiveNetworkInfo();
        
        if((networkInfo !=null) && (networkInfo.isConnected())){
            return determineNetworkType(networkInfo.getType(), networkInfo.getSubtype());
        }else{
            return NONE_NETWORK;
        }
        
    }
    /**
     * Returns current network type.
     */
    private int determineNetworkType(int type, int subType) {

    logger.debug("determineNetworkType  : type -> " +type + " ; subType ->  "+subType);
    //CHECK FOR WFC 
        
    //EPDG OVER WIFI
   /* if(type == ConnectivityManager.TYPE_EPDG){
         return EPDG_WIFI_NETWORK;
    }
    //LTE OVER IMS
    else*/ if(type == ConnectivityManager.TYPE_MOBILE_IMS){
         return EUTRAN_NETWORK;
    }
    //MOBILE NETWORK
    else if(type == ConnectivityManager.TYPE_MOBILE)
    {
        switch (subType) {
        
        //2G NETWORKS
        case TelephonyManager.NETWORK_TYPE_GPRS:
        case TelephonyManager.NETWORK_TYPE_EDGE:
        case TelephonyManager.NETWORK_TYPE_CDMA:
        case TelephonyManager.NETWORK_TYPE_1xRTT:
        case TelephonyManager.NETWORK_TYPE_IDEN:
            return GERAN_NETWORK;
            
         //3G NETWORK   
        case TelephonyManager.NETWORK_TYPE_UMTS:
        case TelephonyManager.NETWORK_TYPE_EVDO_0:
        case TelephonyManager.NETWORK_TYPE_EVDO_A:
        case TelephonyManager.NETWORK_TYPE_HSDPA:
        case TelephonyManager.NETWORK_TYPE_HSUPA:
        case TelephonyManager.NETWORK_TYPE_HSPA:
        case TelephonyManager.NETWORK_TYPE_EVDO_B:
        case TelephonyManager.NETWORK_TYPE_EHRPD:
        case TelephonyManager.NETWORK_TYPE_HSPAP:
            return UTRAN_NETWORK;
            
         //LTE NETWORK   
        case TelephonyManager.NETWORK_TYPE_LTE:
            return EUTRAN_NETWORK;
        default:
              return NONE_NETWORK;
       }
     }
    
    return NONE_NETWORK;
    }
    
    private boolean isEPDGNetwork(int networkType){
        boolean status = false;
        
        ConnectivityManager connectivityMgr = (ConnectivityManager)AndroidFactory.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connectivityMgr.getNetworkInfo(networkType);
        
        if(networkInfo!= null && (networkInfo.getState() == NetworkInfo.State.CONNECTED))
        {
            String typeName = networkInfo.getTypeName();
            String subTypeName = networkInfo.getSubtypeName();
            
            if ("Wi-Fi".equals(typeName) && "IMS".equals(subTypeName)) {
                status = true;
            }
        }

        return status;
    }
    
}
