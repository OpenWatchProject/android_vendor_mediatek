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

//
// Wrapper class for common C part of the Root Provisioning Agent, the C
// files under Android/jni
//

package com.gd.mobicore.pa.jni;

import com.gd.mobicore.pa.service.Log;
import android.os.Build;
import android.os.Build.VERSION;
import android.telephony.TelephonyManager;
import android.content.Context;

import java.util.List;
import com.gd.mobicore.pa.service.BaseService;
import com.gd.mobicore.pa.ifc.CmpCommand;
import com.gd.mobicore.pa.ifc.CmpResponse;

public class CommonPAWrapper {
    private static final String TAG = "RootPA-J";
    private BaseService service_;

    public CommonPAWrapper(BaseService service){
        service_=service;
        Log.d(TAG,"CommonPAWrapper.java: constructor");
    }

    public native int openSession();
    public native void closeSession();
    public native int executeCmpCommands(int uid, List<CmpCommand> commands, List<CmpResponse> responses);
    public native int getVersion(byte[] productId, List<String> keys, List<Integer> values);
    public native int getSuid(byte[] suid);
    public native int isRootContainerRegistered(boolean[] result);
    public native int isSpContainerRegistered(int spid, boolean[] result);
    public native int getSPContainerState(int spid, int[] state);
    public native int getSPContainerStructure(int spid, int[] ints, byte[][] uuidArray, int[] trustletStates);
    public native int doProvisioning(int uid, int spid, byte[] seAddress);
	public native int installTrustlet(int spid,
                                      byte[] uuid,
                                      int dataType,
                                      byte[] tltOrKeyData,
                                      int minTltVersion,
                                      byte[] tltPukHash,
                                      int memoryType,
                                      int numberOfInstances,
                                      int flags,
                                      byte[] seAddress);
    public native int unregisterRootContainer(byte[] seAddress);
    public native int storeTA(int spid, byte[] uuid, byte[] taBinary);
    public native void setEnvironmentVariable(byte[] variable, byte[] value);

    static{
        Log.d(TAG,"CommonPAWrapper.java: static");
    	try {
            System.loadLibrary("commonpawrapper");
    	} catch (Throwable e) {
            Log.d(TAG,"loading common wrapper failed, trying to load test");
             System.loadLibrary("commonpawrapper_test");
        }
    }

    // callbacks from C code

     public String getFilesDirPath(){
        Log.d(TAG,"CommonPAWrapper.getFilesDirPath");
        return service_.getFilesDirPath();
     }

     public void provisioningStateCallback(int state, int ret){
        Log.d(TAG,"CommonPAWrapper.provisioningStateCallback");
        service_.provisioningStateCallback(state, ret);
     }

    private static final int IMEI_ESN_INDEX=0;
    private static final int MNO_INDEX=1;
    private static final int BRAND_INDEX=2;
    private static final int MANUFACTURER_INDEX=3;
    private static final int HARDWARE_INDEX=4;
    private static final int MODEL_INDEX=5;
    private static final int VERSION_INDEX=6;
    private static final int RESPONSE_ARRAY_SIZE=7;

    public String[] getSystemInfo(){
        Log.d(TAG,">>CommonPAWrapper.getSystemInfo");
        String[] response= new String[RESPONSE_ARRAY_SIZE];
        response[IMEI_ESN_INDEX]="";
        response[MNO_INDEX]="";
        response[BRAND_INDEX]=Build.BRAND;
        response[MANUFACTURER_INDEX]=Build.MANUFACTURER;
        response[HARDWARE_INDEX]=Build.HARDWARE;
        response[MODEL_INDEX]=Build.MODEL;
        response[VERSION_INDEX]=Build.VERSION.CODENAME+" "+Build.VERSION.INCREMENTAL+" "+Build.VERSION.RELEASE+" "+Integer.toString(Build.VERSION.SDK_INT);

        Log.d(TAG,"<<CommonPAWrapper.getSystemInfo "+response[IMEI_ESN_INDEX]+" "+response[MANUFACTURER_INDEX]+" "+response[VERSION_INDEX]);
        return response;
     }


    public void trustletInstallCallback(byte[] trustlet){
        Log.d(TAG,">>CommonPAWrapper.trustletInstallCallback "+trustlet.length);
        service_.trustletInstallCallback(trustlet);
        Log.d(TAG,"<<CommonPAWrapper.trustletInstallCallback");
    }
}
