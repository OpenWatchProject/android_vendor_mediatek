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
import com.gd.mobicore.pa.ifc.RootPAOemIfc;
import com.gd.mobicore.pa.ifc.CommandResult;
import com.gd.mobicore.pa.ifc.BooleanResult;
import com.gd.mobicore.pa.ifc.TrustletContainer;
import com.gd.mobicore.pa.ifc.IfcVersion;

public class OemService extends BaseService {
    private static final String TAG = "RootPA-J";

    private final RootPAOemIfc.Stub mBinder = new ServiceIfc();
    private static final int OEM_UID_FOR_LOCK=0x33330000;

    private class ServiceIfc extends RootPAOemIfc.Stub {
        public ServiceIfc(){
            super();
        }

        private CommonPAWrapper commonPAWrapper(){
            return OemService.this.commonPAWrapper();
        }

        public CommandResult unregisterRootContainer(){
            Log.d(TAG,">>RootPAServiceIfc.Stub.unregisterRootContainer");

            int tmpSuid=OEM_UID_FOR_LOCK+new Random().nextInt(); // this may override the uid used in lock, which means it will not be

            if(!OemService.this.acquireLock(tmpSuid, false).isOk()){
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }

            doProvisioningLockSuid_=tmpSuid;
            Log.d(TAG,"RootPAServiceIfc.Stub.unregisterRootContainer calling JNI");

            int ret=CommandResult.ROOTPA_OK;

            try{
                setupProxy();
            	ret=commonPAWrapper().unregisterRootContainer(se_);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().unregisterRootContainer exception: ", e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            CommandResult res=OemService.this.releaseLock(doProvisioningLockSuid_, false);
            if(!res.isOk()){
                Log.e(TAG,"releasing lock failed, res: "+res.result());
                // this return code is not returned to the client since
                // the command may have succeeded and there is just something wrong with the lock
                // we leave it the the next command if the problem remains
            }

            Log.d(TAG,"<<RootPAServiceIfc.Stub.unregisterRootContainer");
            return new CommandResult(ret);
        }

    }

    @Override
    public void onCreate() {
        Log.d(TAG,"Hello, OemService onCreate");
        super.onCreate();
    }

    @Override
    public void onLowMemory() {
        Log.d(TAG,"OemService onLowMemory");
        super.onLowMemory();
    }

    public void onDestroy(){
        Log.d(TAG,"OemService being destroyed");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent){
        try{
            se_ = intent.getByteArrayExtra("SE");
        }catch(Exception e){
            Log.i(TAG,"OemService something wrong in the given ip "+e );
        }

        try{
            Log.setLoggingLevel(intent.getIntExtra("LOG",0));
        }catch(Exception e){
            Log.i(TAG,"OemService something wrong in the given logging level "+e );
        }
        Log.i(TAG,"OemService binding, IfcVersion: " +IfcVersion.ROOTPA_ANDROID_API_VERSION_MAJOR+"."+IfcVersion.ROOTPA_ANDROID_API_VERSION_MINOR);
        if(se_!=null) Log.d(TAG,new String(se_));
        return mBinder;
    }

    @Override
    public int  onStartCommand(Intent i, int flags, int startid){
        Log.d(TAG,"OemService starting");
        return START_STICKY;
    }
}


