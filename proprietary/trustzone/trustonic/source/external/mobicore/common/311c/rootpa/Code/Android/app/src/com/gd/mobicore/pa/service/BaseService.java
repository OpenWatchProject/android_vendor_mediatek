/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.gd.mobicore.pa.service;

import android.app.Service;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.net.NetworkInfo;
import android.net.ConnectivityManager;

import java.io.File;
import java.net.URI;
import java.net.Proxy;
import java.net.Proxy.Type;
import java.net.ProxySelector;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.atomic.AtomicInteger;

import com.gd.mobicore.pa.jni.CommonPAWrapper;
import com.gd.mobicore.pa.ifc.RootPAProvisioningIntents;
import com.gd.mobicore.pa.ifc.CommandResult;

public abstract class BaseService extends Service {
    protected static final String TAG = "RootPA-J";

    /*
        being statically linked library, the Common C implementation does not handle locks,
        they must be handled in the using implementation, in this case here.
    */
    private static final int LOCK_FREE=0;
    private static final AtomicInteger lock_= new AtomicInteger(LOCK_FREE);
    private static final int LOCK_TIMEOUT_MS=300000; //5 mins in ms
    private TimerTask timerTask_=null;
    private Timer timer_=null;

    protected int doProvisioningLockSuid_=0;
    protected byte[] se_ = null;

    private static final int C_CONNECTING_SERVICE_ENABLER=1;
    private static final int C_AUTHENTICATING_SOC=2;
    private static final int C_CREATING_ROOT_CONTAINER=3;
    private static final int C_AUTHENTICATING_ROOT=4;
    private static final int C_CREATING_SP_CONTAINER=5;
    private static final int C_FINISHED_PROVISIONING=6;
    private static final int C_ERROR=7;
    private static final int C_UNREGISTERING_ROOT_CONTAINER=8;
    private static final int C_PROVISIONING_STATE_THREAD_EXITING=0xDEAD;

    protected final CommonPAWrapper commonPaWrapper_=new CommonPAWrapper(this);
    private boolean sessionOpened_=false;

    protected CommonPAWrapper commonPAWrapper(){
        return commonPaWrapper_;
    }

    protected synchronized CommandResult acquireLock(int uid, boolean openSession){
        Log.d(TAG,">>BaseService.acquireLock "+uid+" "+lock_.get()+" "+timer_);
        if(uid==LOCK_FREE){
            return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
        }
        boolean result=lock_.compareAndSet(LOCK_FREE, uid);
        if(result==true || lock_.get() == uid){

            if(result==true && openSession==true && sessionOpened_==false){
                Log.d(TAG,"BaseService.acquireLock, openingSession");
                commonPAWrapper().openSession();
                sessionOpened_=true;
            }

            if(timer_!=null){
                timerTask_.cancel();
                timer_.cancel();
            }

            timer_=new Timer();
            timerTask_=new TimerTask(){
                public void run(){
                    Log.i(TAG,"Timer expired, releasing lock");
                    lock_.set(LOCK_FREE);
                    if(sessionOpened_==true){
                        Log.d(TAG,"BaseService.Timer.run, closingSession");
                        commonPAWrapper().closeSession();
                        sessionOpened_=false;
                    }
                }
            };
            timer_.schedule(timerTask_,LOCK_TIMEOUT_MS);
            Log.d(TAG,"<<BaseService.acquireLock, successfull return "+timer_);
            return new CommandResult(CommandResult.ROOTPA_OK);
        }
        return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
    }

    // this is public for the ProvisioningService to call it
    protected synchronized CommandResult releaseLock(int uid, boolean closeSession){
        Log.d(TAG,"BaseService.releaseLock "+uid+" "+lock_.get()+" "+timer_);

        if(uid==LOCK_FREE){
            return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
        }

        if((lock_.get()==LOCK_FREE) || (lock_.compareAndSet(uid, LOCK_FREE)==true)){

            if(closeSession==true && sessionOpened_==true){
                Log.d(TAG,"BaseService.releaseLock, closingSession");
                commonPAWrapper().closeSession();
                sessionOpened_=false;
            }

            if(timer_!=null){
                timerTask_.cancel();
                timerTask_=null;
                timer_.cancel();
                timer_=null;
            }
            return new CommandResult(CommandResult.ROOTPA_OK);
        }
        return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
    }

    /**
        Since libcurl is able to read and use proxy settings from http_proxy environment variable, we set the proxy here.
        This should be changed every time the connection changes so that there are always correct proxy settings available
    */

    BroadcastReceiver networkChangeReceiver_=null;
    protected void setupProxy()
    {
        byte[] proxyAddress=null;
        ProxySelector defaultProxySelector = ProxySelector.getDefault();

        if(defaultProxySelector != null){
            URI uri=null;
            List<Proxy> proxyList=null;
            try{
                if(se_==null){
                    uri=new URI("https://se.cgbe.trustonic.com"); // the URI here does not matter a lot, as long as one exists. We try to use as real one as is easily possible
                }else{
                    uri=new URI(new String(se_));
                }
                proxyList = defaultProxySelector.select(uri);
                if (proxyList.size() > 0)
                {
                    Proxy proxy = proxyList.get(0);
                    Log.d(TAG,"BaseService.setupProxy proxy "+proxy); // there should be only one element in the list in the current Android versions, it is for the current connection
                    if(proxy != Proxy.NO_PROXY){
                        Log.d(TAG,"BaseService.setupProxy proxy.type "+proxy.type());
                        if(proxy.type()==Proxy.Type.HTTP){
                            // TODO-future there is currently no way for the user to store proxy user name and password in Android,
                            // so they need to be asked at connection time. There is not any kind of user/password support for proxies in RootPA.
                            // If we were able to get username/password we would add them to http(s)_proxy here.
                            // if(username && password) proxyAddress=username+":"+password; (and add the next line just remove +1 from indexOf)
                            proxyAddress=proxy.toString().substring(proxy.toString().indexOf("@")+1).getBytes();
                        }
                    }
                }

            }catch(Exception e){
                Log.e(TAG,"BaseService.setupProxy FAILURE in getting the proxy: "+e.toString());
            }
        }

        commonPAWrapper().setEnvironmentVariable("http_proxy".getBytes(), proxyAddress);
        commonPAWrapper().setEnvironmentVariable("https_proxy".getBytes(), proxyAddress);
        Log.d(TAG,"BaseService.setupProxy just set the proxy to: "+(proxyAddress==null?proxyAddress:new String(proxyAddress)));

        // start listening to intents on network changes if not doing it already
        // this is important since the proxy settings are network specific
        if(networkChangeReceiver_==null){
            networkChangeReceiver_=new BroadcastReceiver(){
                public void onReceive(Context ctx, Intent intent){
                    Log.d(TAG, "BaseService: Network connection changed");
                    try{
                        NetworkInfo ni=((ConnectivityManager)ctx.getSystemService(Context.CONNECTIVITY_SERVICE)).getActiveNetworkInfo();
                        if(ni!=null && ni.isConnectedOrConnecting()) {
                            Log.d(TAG,"BaseService: Network "+ni.getTypeName()+" connected");
                            setupProxy();
                        }else{
                            if(ni!=null){
                                Log.d(TAG, "BaseService: network state "+ni.getState());
                            }else{
                                Log.d(TAG, "BaseService: no network info");
                            }
                        }
                    }catch(Exception e){
                        Log.e(TAG, "BaseService: Network connection change handling FAILURE "+e);
                    }
                }
            };
            IntentFilter filter=new IntentFilter("android.net.conn.CONNECTIVITY_CHANGE");
            registerReceiver(networkChangeReceiver_, filter);
        }
    }

    protected synchronized boolean locked(int uid){
        return(lock_.get() != uid && uid != LOCK_FREE);
    }

    /**
    This method is called from the C code to send the trustlet binary to the client
    (trustlet connector/"sp.pa" for develope trustlet) that then can store it where desired.
    */
    public void trustletInstallCallback(byte[] trustlet){
        Log.d(TAG,">>BaseService.trustletInstallCallback");
        Intent intent=new Intent(RootPAProvisioningIntents.INSTALL_TRUSTLET);
        intent.putExtra(RootPAProvisioningIntents.TRUSTLET, trustlet);
        sendBroadcast(intent);
        Log.d(TAG,"<<BaseService.trustletInstallCallback");
    }

    /**
    This method is called from the C code to get the path for files directory
    */
    public String getFilesDirPath(){
        File file = this.getFilesDir();
        if (null != file){
            return file.getAbsolutePath();
        } else {
            return null;
        }
    }

    /**
     This method is called from the C code to send the intents while executing doProvisioning
    */
    public void provisioningStateCallback(int state, int ret){
        Log.d(TAG,">>provisioningStateCallback "+state+" "+ret);

        // since sommunication with SE may take consirderable amount of time, we refresh the Lock timer
        // by calling acquireLock every time a state notification callback is called. This way the lock
        // will not timeout before the communication with SE is complete.
        try{
            CommandResult res=acquireLock(doProvisioningLockSuid_, false);
            if(!res.isOk()){
                Log.e(TAG,"provisioningStateCallback re-acquiring lock failed, res: "+res.result());
            }
        }catch(Exception e){
            Log.e(TAG,"provisioningStateCallback re-acquiring lock failed: "+e);
        }

        Intent intent=new Intent(RootPAProvisioningIntents.PROVISIONING_PROGRESS_UPDATE);
        switch(state){
            case C_CONNECTING_SERVICE_ENABLER:
                intent.putExtra(RootPAProvisioningIntents.STATE, RootPAProvisioningIntents.CONNECTING_SERVICE_ENABLER);
                break;
            case C_AUTHENTICATING_SOC:
                intent.putExtra(RootPAProvisioningIntents.STATE, RootPAProvisioningIntents.AUTHENTICATING_SOC);
                break;
            case C_CREATING_ROOT_CONTAINER:
                intent.putExtra(RootPAProvisioningIntents.STATE, RootPAProvisioningIntents.CREATING_ROOT_CONTAINER);
                break;
            case C_AUTHENTICATING_ROOT:
                intent.putExtra(RootPAProvisioningIntents.STATE, RootPAProvisioningIntents.AUTHENTICATING_ROOT);
                break;
            case C_CREATING_SP_CONTAINER:
                intent.putExtra(RootPAProvisioningIntents.STATE, RootPAProvisioningIntents.CREATING_SP_CONTAINER);
                break;
            case C_FINISHED_PROVISIONING:
                intent.putExtra(RootPAProvisioningIntents.STATE, RootPAProvisioningIntents.FINISHED_PROVISIONING);
                break;
            case C_UNREGISTERING_ROOT_CONTAINER:
                intent.putExtra(RootPAProvisioningIntents.STATE, RootPAProvisioningIntents.UNREGISTERING_ROOT_CONTAINER);
                break;
            case C_ERROR:
                intent=new Intent(RootPAProvisioningIntents.PROVISIONING_ERROR);

                intent.putExtra(RootPAProvisioningIntents.ERROR, ret);
                break;

            case C_PROVISIONING_STATE_THREAD_EXITING:
                try{
                    CommandResult res=releaseLock(doProvisioningLockSuid_, false);
                    if(!res.isOk()){
                        Log.e(TAG,"provisioningStateCallback releasing lock failed, res: "+res.result());
                    }
                    doProvisioningLockSuid_=0;
                    intent=null; // no intent sent in this case
                }catch(Exception e){
                    Log.e(TAG,"provisioningStateCallback releasing lock failed: "+e);
                }
                if(networkChangeReceiver_!=null){
                    unregisterReceiver(networkChangeReceiver_);
                    networkChangeReceiver_=null;
                }
                sendBroadcast(new Intent(RootPAProvisioningIntents.FINISHED_ROOT_PROVISIONING));
                break;
            default:
                Log.e(TAG,"unknown state: "+state);
                intent=null;
                break;
        }
        if(intent!=null){
            sendBroadcast(intent);
        }

        Log.d(TAG,"<<provisioningStateCallback ");
    }

    public void onConfigurationChanged(android.content.res.Configuration newConfig){
        super.onConfigurationChanged(newConfig);
        Log.d(TAG,"BaseService.onConfigurationChanged");
    }

    public void onCreate(){
        super.onCreate();
        Log.d(TAG,"BaseService.onCreate");
    }

    public void onDestroy(){
        if(networkChangeReceiver_!=null){
            unregisterReceiver(networkChangeReceiver_);
            networkChangeReceiver_=null;
        }
        Log.d(TAG,"BaseService.onDestroy");
    }

    public void onLowMemory(){
        super.onLowMemory();
        Log.d(TAG,"BaseService.onLowMemory");
    }

    public void onRebind(Intent intent){
        super.onRebind(intent);
        Log.d(TAG,"BaseService.onRebind");
    }

    public int onStartCommand(Intent intent, int flags, int startId){
        int res=super.onStartCommand(intent, flags, startId);
        Log.d(TAG,"BaseService.onStartCommand");
        return res;
    }

    public void onTaskRemoved(Intent intent){
        super.onTaskRemoved(intent);
        Log.d(TAG,"BaseService.onTaskRemoved");
    }


    public void onTrimMemory(int level){
        super.onTrimMemory(level);
        Log.d(TAG,"BaseService.onTrimMemory");
    }

    public boolean onUnbind(Intent intent){
        boolean res=super.onUnbind(intent);
        Log.d(TAG,"BaseService.onUnbind");
        return res;
    }

}
