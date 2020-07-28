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
import android.os.IBinder;
import android.os.Bundle;

import java.util.Random;

import com.gd.mobicore.pa.jni.CommonPAWrapper;
import com.gd.mobicore.pa.ifc.RootPAProvisioningIntents;
import com.gd.mobicore.pa.ifc.RootPADeveloperIfc;
import com.gd.mobicore.pa.ifc.CommandResult;
import com.gd.mobicore.pa.ifc.BooleanResult;
import com.gd.mobicore.pa.ifc.TrustletContainer;
import com.gd.mobicore.pa.ifc.IfcVersion;

public class DeveloperService extends BaseService {

    private final RootPADeveloperIfc.Stub mBinder = new ServiceIfc();
    private static final int DEVELOPER_UID_FOR_LOCK=0x22220000;
    private static final int UUID_LENGTH=16;
    private static final int EXTERNAL_MEMORY=2;
    private static final int DEFAULT_MEMORY_TYPE=EXTERNAL_MEMORY;
    private static final int DEFAULT_NUMBER_OF_INSTANCES=1;
    private static final int DEFAULT_FLAGS=0;
    private static final byte[] DEFAULT_PUKHASH={0,0,0,0,0,0,0,0,0,0,
                                                 0,0,0,0,0,0,0,0,0,0,
                                                 0,0,0,0,0,0,0,0,0,0,0,0};
    private class ServiceIfc extends RootPADeveloperIfc.Stub {
        public ServiceIfc(){
            super();
        }

        // note that these values have to be in line with TltInstallationRequestDataType in rootpa.h
        public static final int REQUEST_DATA_TLT=1;
        public static final int REQUEST_DATA_KEY=2;


        private CommonPAWrapper commonPAWrapper(){
            return DeveloperService.this.commonPAWrapper();
        }

        private boolean uuidOk(byte[] uuid){
            if(uuid==null || uuid.length != UUID_LENGTH){
                Log.e(TAG,"DeveloperService.Stub.uuidOk NOK");
                return false;
            }
            Log.d(TAG,"DeveloperService.Stub.uuidOk OK");
            return true;
        }

        public CommandResult installTrustletOrKey(int spid, byte[] uuid, byte[] trustletBinary, byte[] key, int minTltVersion, byte[] tltPukHash){
            Log.d(TAG,">>DeveloperService.Stub.installTrustletOrKey");
            if(tltPukHash==null){
                tltPukHash=DEFAULT_PUKHASH;
            }

            if((trustletBinary == null && key == null) || (trustletBinary != null && key != null) || 0==spid || !uuidOk(uuid) ){
                return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
            }

            int tmpSuid=DEVELOPER_UID_FOR_LOCK+new Random().nextInt();

            if(!DeveloperService.this.acquireLock(tmpSuid, false).isOk()){
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }
            doProvisioningLockSuid_=tmpSuid;
            int err=0;
            byte[] data=null;
            int dataType;
            try{
                if(trustletBinary != null){
                    data=trustletBinary;
                    dataType=REQUEST_DATA_TLT;
                }else{
                    data=key;
                    dataType=REQUEST_DATA_KEY;
                }
                setupProxy();
                err=commonPAWrapper().installTrustlet(spid,
                                                      uuid,
                                                      dataType,
                                                      data,
                                                      minTltVersion,
                                                      tltPukHash,
                                                      DEFAULT_MEMORY_TYPE,
                                                      DEFAULT_NUMBER_OF_INSTANCES,
                                                      DEFAULT_FLAGS,
                                                      se_);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().installTrustletOrKey exception: ", e);
                err=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            Log.d(TAG,"<<DeveloperService.Stub.installTrustletOrKey");
            return new CommandResult(err);
        }

        public CommandResult installTrustlet(int spid,
                                             byte[] uuid,
                                             byte[] trustletBinary,
                                             int minTltVersion,
                                             byte[] tltPukHash,
                                             int memoryType,
                                             int numberOfInstances,
                                             int flags){
            Log.d(TAG,">>DeveloperService.Stub.installTrustlet");
            if(tltPukHash==null){
                tltPukHash=DEFAULT_PUKHASH;
            }


            if(trustletBinary == null || 0==spid || !uuidOk(uuid) || memoryType > EXTERNAL_MEMORY){
                return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
            }

            int tmpSuid=DEVELOPER_UID_FOR_LOCK+new Random().nextInt();

            if(!DeveloperService.this.acquireLock(tmpSuid, false).isOk()){
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }
            doProvisioningLockSuid_=tmpSuid;
            int err=0;
            try{
                setupProxy();
                err=commonPAWrapper().installTrustlet(spid,
                                                      uuid,
                                                      REQUEST_DATA_TLT,
                                                      trustletBinary,
                                                      minTltVersion,
                                                      tltPukHash,
                                                      memoryType,
                                                      numberOfInstances,
                                                      flags,
                                                      se_);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().installTrustlet exception: ", e);
                err=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            Log.d(TAG,"<<DeveloperService.Stub.installTrustlet");
            return new CommandResult(err);
        }
    }

    @Override
    public void onCreate() {
        Log.d(TAG,"Hello, DeveloperService onCreate");
        super.onCreate();
    }

    @Override
    public void onLowMemory() {
        Log.d(TAG,"DeveloperService onLowMemory");
        super.onLowMemory();
    }

    public void onDestroy(){
        Log.d(TAG,"DeveloperService being destroyed");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent){
        try{
            se_ = intent.getByteArrayExtra("SE");
        }catch(Exception e){
            Log.i(TAG,"DeveloperService something wrong in the given ip "+e );
        }

        try{
            Log.setLoggingLevel(intent.getIntExtra("LOG",0));
        }catch(Exception e){
            Log.i(TAG,"DeveloperService something wrong in the given logging level "+e );
        }
        Log.i(TAG,"DeveloperService binding, IfcVersion: " +IfcVersion.ROOTPA_ANDROID_API_VERSION_MAJOR+"."+IfcVersion.ROOTPA_ANDROID_API_VERSION_MINOR);
        if(se_!=null) Log.d(TAG,new String(se_));
        return mBinder;

    }

    @Override
    public int  onStartCommand(Intent i, int flags, int startid){
        Log.d(TAG,"DeveloperService starting");
        return START_STICKY;
    }
}


