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

import java.util.List;
import java.util.ArrayList;
import java.util.UUID;
import java.util.Random;
import java.math.BigInteger;

import com.gd.mobicore.pa.jni.CommonPAWrapper;

import com.gd.mobicore.pa.ifc.RootPAServiceIfc;
import com.gd.mobicore.pa.ifc.CmpMsg;
import com.gd.mobicore.pa.ifc.CmpCommand;
import com.gd.mobicore.pa.ifc.CmpResponse;
import com.gd.mobicore.pa.ifc.CommandResult;
import com.gd.mobicore.pa.ifc.BooleanResult;
import com.gd.mobicore.pa.ifc.SPID;
import com.gd.mobicore.pa.ifc.Version;
import com.gd.mobicore.pa.ifc.SUID;
import com.gd.mobicore.pa.ifc.SPContainerStructure;
import com.gd.mobicore.pa.ifc.SPContainerStateParcel;
import com.gd.mobicore.pa.ifc.SPContainerState;
import com.gd.mobicore.pa.ifc.TrustletContainer;
import com.gd.mobicore.pa.ifc.TrustletContainerState;
import com.gd.mobicore.pa.ifc.IfcVersion;

public class ProvisioningService extends BaseService {
//    protected static final String TAG = "RootPA-J";

    private static final int PROVISIONING_UID_FOR_LOCK=0x11110000;
    private static final int LONG_SIZE=8;
    private final RootPAServiceIfc.Stub mBinder = new ServiceIfc();

    // using this instead of anonymous inner class in order to allow call to some of the private methods we define here
    private class ServiceIfc extends RootPAServiceIfc.Stub {

        public ServiceIfc(){
            super();
        }

        private CommonPAWrapper commonPAWrapper(){
            return ProvisioningService.this.commonPAWrapper();
        }

        public CommandResult executeCmpCommands(int uid, List<CmpCommand> commands, List<CmpResponse> responses){

            Log.d(TAG,">>RootPAServiceIfc.Stub.executeCmpCommands "+commands+" "+responses);

            if(commands==null||responses==null){ // having null out variable leads to null pointer exception in the client, however we still want to do checking so that there is not unncessary execution of the following code
                Log.d(TAG,"RootPAServiceIfc.Stub.executeCmpCommands, illegal argument");
                return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
            }

            if(locked(uid)){
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }

            int ret=CommandResult.ROOTPA_OK;
            try{
                ret=commonPAWrapper().executeCmpCommands(uid, commands, responses);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().executeCmpCommands exception: ", e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            Log.d(TAG,"<<RootPAServiceIfc.Stub.executeCmpCommands");
            return new CommandResult(ret);
        }

        public CommandResult isRootContainerRegistered(BooleanResult result){
            Log.d(TAG,">>RootPAServiceIfc.Stub.isRootContainerRegistered");

            if(result==null){  // having null out variable leads to null pointer exception in the client, however we stll want to do checking so that there is not unncessary execution of the following code
                Log.d(TAG,"RootPAServiceIfc.Stub.isRootContainerRegistered result null");
                return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
            }

            int internalUidForLock=new Random().nextInt();

            if(!ProvisioningService.this.acquireLock(internalUidForLock, false).isOk())
            {
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }

            boolean[] isRegistered = new boolean[1];
            int ret=CommandResult.ROOTPA_OK;
            try{
                ret=commonPAWrapper().isRootContainerRegistered(isRegistered);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().isRootContainerRegistered exception: ", e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }
            result.setResult(isRegistered[0]);

            CommandResult res=ProvisioningService.this.releaseLock(internalUidForLock, false);
            if(!res.isOk()){
                Log.e(TAG,"releasing lock failed, res: "+res.result());
                // this return code is not returned to the client since
                // the command may have succeeded and there is just something wrong with the lock
                // we leave it the the next command if the problem remains
            }

            Log.d(TAG,"<<RootPAServiceIfc.Stub.isRootContainerRegistered");
            return new CommandResult(ret);
        }

        public CommandResult isSPContainerRegistered(SPID spid, BooleanResult result){
            Log.d(TAG,">>RootPAServiceIfc.Stub.isSPContainerRegistered");

            if(spid==null || result==null){  // having null out variable leads to null pointer exception in the client, however we still want to do checking so that there is not unncessary execution of the following code
                Log.d(TAG,"RootPAServiceIfc.Stub.isSPContainerRegistered spid "+spid+" result "+result);
                return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
            }

            boolean[] isRegistered = new boolean[1];
            int ret=CommandResult.ROOTPA_OK;

            int internalUidForLock=new Random().nextInt();
            if(!ProvisioningService.this.acquireLock(internalUidForLock, false).isOk())
            {
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }

            try{
                ret=commonPAWrapper().isSpContainerRegistered(spid.spid(), isRegistered);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().isSpContainerRegistered exception: ", e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }
            result.setResult(isRegistered[0]);

            CommandResult res=ProvisioningService.this.releaseLock(internalUidForLock, false);
            if(!res.isOk()){
                Log.e(TAG,"releasing lock failed, res: "+res.result());
                // this return code is not returned to the client since
                // the command may have succeeded and there is just something wrong with the lock
                // we leave it the the next command if the problem remains
            }


            Log.d(TAG,"<<RootPAServiceIfc.Stub.isSPContainerRegistered ");
            return new CommandResult(ret);
        }

        public CommandResult getVersion(Version version){
            Log.d(TAG,">>RootPAServiceIfc.Stub.getVersion");

            int internalUidForLock=new Random().nextInt();
            if(!ProvisioningService.this.acquireLock(internalUidForLock, false).isOk())
            {
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }

            int ret=CommandResult.ROOTPA_OK;
            byte[] productId=new byte[64]; // max length of product id from mcVersionInfo.h
            Bundle versionBundle= new Bundle();
            List<String> keys = new ArrayList<String>();
            List<Integer> values = new ArrayList<Integer>();

            try{
                ret=commonPAWrapper().getVersion(productId, keys, values);
                if(ret == CommandResult.ROOTPA_OK && (keys.size() != values.size())){
                    ret=CommandResult.ROOTPA_ERROR_INTERNAL;
                }
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().getVersion exception: ", e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            if(ret==CommandResult.ROOTPA_OK){
                version.setProductId(new String(productId).trim());

                for(int i=0; i<values.size(); i++){
                    versionBundle.putInt(keys.get(i), values.get(i));
                }

                version.setVersion(versionBundle);
            }

            CommandResult res=ProvisioningService.this.releaseLock(internalUidForLock, false);
            if(!res.isOk()){
                Log.e(TAG,"releasing lock failed, res: "+res.result());
                // this return code is not returned to the client since
                // the command may have succeeded and there is just something wrong with the lock
                // we leave it the the next command if the problem remains
            }

            Log.d(TAG,"<<RootPAServiceIfc.Stub.getVersion "+ret);
            return new CommandResult(ret);
        }


        public CommandResult getDeviceId(SUID suid){
            Log.d(TAG,">>RootPAServiceIfc.Stub.getDeviceId");

            int internalUidForLock=new Random().nextInt();
            if(!ProvisioningService.this.acquireLock(internalUidForLock, false).isOk())
            {
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }

            int ret=CommandResult.ROOTPA_OK;
            byte[] suidArray=new byte[16]; // suid length
            try{
                ret=commonPAWrapper().getSuid(suidArray);
                suid.setSuid(suidArray);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().getSuid exception: ", e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            CommandResult res=ProvisioningService.this.releaseLock(internalUidForLock, false);
            if(!res.isOk()){
                Log.e(TAG,"releasing lock failed, res: "+res.result());
                // this return code is not returned to the client since
                // the command may have succeeded and there is just something wrong with the lock
                // we leave it the the next command if the problem remains
            }


            Log.d(TAG,"<<RootPAServiceIfc.Stub.getDeviceId");
            return new CommandResult(ret);
        }

        public CommandResult acquireLock(int uid){
            return ProvisioningService.this.acquireLock(uid, true);
        }

        public CommandResult releaseLock(int uid){
            return ProvisioningService.this.releaseLock(uid, true);
        }

        public CommandResult doProvisioning(int uid, SPID spid){
            Log.d(TAG,">>RootPAServiceIfc.Stub.doProvisioning");
            int ret=CommandResult.ROOTPA_OK;

            if(spid==null){  // having null out variable leads to null pointer exception in the client, however we still want to do checking so that there is not unncessary execution of the following code
                Log.d(TAG,"RootPAServiceIfc.Stub.doProvisioning spid==null");
                return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
            }

            // we do not use uid here since we do not want to let the client to released the lock, it is done
            // internally at CommonPAWrapper.java when sending Intents.

            int tmpSuid=uid+PROVISIONING_UID_FOR_LOCK+new Random().nextInt();

            if(!ProvisioningService.this.acquireLock(tmpSuid, false).isOk()){
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }
            doProvisioningLockSuid_=tmpSuid;

            try{
                setupProxy();
                ret=commonPAWrapper().doProvisioning(uid, spid.spid(), se_);
            }catch(Exception e){
                Log.d(TAG,"CommonPAWrapper()).doProvisioning failed "+e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            Log.d(TAG,"CommonPAWrapper()).doProvisioning returned "+ret);
            if(ret!=CommandResult.ROOTPA_OK){
                if(!ProvisioningService.this.releaseLock(doProvisioningLockSuid_, false).isOk()){
                    Log.e(TAG,"releasing lock failed after doProvisioning returned an error");
                }
                doProvisioningLockSuid_=0;
            }

            Log.d(TAG,"<<RootPAServiceIfc.Stub.doProvisioning");
            return new CommandResult(ret);
        }

        public CommandResult getSPContainerStructure(SPID spid, SPContainerStructure cs){
            Log.d(TAG,">>RootPAServiceIfc.Stub.getSPContainerStructure");

            if(spid==null||cs==null){ // having null out variable leads to null pointer exception in the client, however we still want to do checking so that there is not unncessary execution of the following code
                return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
            }

            int internalUidForLock=new Random().nextInt();
            if(!ProvisioningService.this.acquireLock(internalUidForLock, false).isOk())
            {
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }

            int ret=CommandResult.ROOTPA_OK;

            final int CONTAINER_STATE_IDX=0;
            final int NUMBER_OF_TLTS_IDX=1;
            final int NUMBER_OF_ELEMENTS=2;
            final int MAX_NUMBER_OF_TRUSTLETS=16;
            final int UUID_LENGTH=16;

            int[] ints = new int[NUMBER_OF_ELEMENTS];
            int[] trustletStates = new int[MAX_NUMBER_OF_TRUSTLETS];
            byte[][] uuidArray = new byte[MAX_NUMBER_OF_TRUSTLETS][];

            try{
                ret=commonPAWrapper().getSPContainerStructure(spid.spid(), ints, uuidArray, trustletStates);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().getSPContainerStructure exception: ", e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            if(ret==CommandResult.ROOTPA_OK){

                SPContainerState s=mapSpContainerState(ints[CONTAINER_STATE_IDX]);
          	    cs.setState(s);
                if (s == SPContainerState.UNDEFINED){
                    ret=CommandResult.ROOTPA_ERROR_INTERNAL;
                }

                for(int i=0; i<ints[NUMBER_OF_TLTS_IDX]; i++){
                    byte[] msBytes=new byte[LONG_SIZE];
                    System.arraycopy(uuidArray[i], 0, msBytes, 0, LONG_SIZE);
                    BigInteger mostSignificant=new BigInteger(msBytes);

                    byte[] lsBytes=new byte[LONG_SIZE];
                    System.arraycopy(uuidArray[i], LONG_SIZE, lsBytes, 0, LONG_SIZE);
                    BigInteger leastSignificant=new BigInteger(lsBytes);

                    Log.d(TAG,"UUID: ls ms"+leastSignificant+" "+mostSignificant);

                    TrustletContainerState ts=mapTltContainerState(trustletStates[i]);
                    if (ts == TrustletContainerState.UNDEFINED){
                        ret=CommandResult.ROOTPA_ERROR_INTERNAL;
                    }
                    cs.add(new TrustletContainer(new UUID(mostSignificant.longValue(), leastSignificant.longValue()), ts));
                }

            }else if (ret==CommandResult.ROOTPA_ERROR_INTERNAL_NO_CONTAINER){
           	    cs.setState(SPContainerState.DOES_NOT_EXIST);
                ret=CommandResult.ROOTPA_OK;
            }

            CommandResult res=ProvisioningService.this.releaseLock(internalUidForLock, false);
            if(!res.isOk()){
                Log.e(TAG,"releasing lock failed, res: "+res.result());
                // this return code is not returned to the client since
                // the command may have succeeded and there is just something wrong with the lock
                // we leave it the the next command if the problem remains
            }


            Log.d(TAG,"<<RootPAServiceIfc.Stub.getSPContainerStructure");
            return new CommandResult(ret);
        }


        public CommandResult getSPContainerState(SPID spid, SPContainerStateParcel state){
            Log.d(TAG,">>RootPAServiceIfc.Stub.getSPContainerState");

            if(spid==null||state==null){ // having null out variable leads to null pointer exception in the client, however we still want to do checking so that there is not unncessary execution of the following code
                return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
            }

            int internalUidForLock=new Random().nextInt();
            if(!ProvisioningService.this.acquireLock(internalUidForLock, false).isOk())
            {
                return new CommandResult(CommandResult.ROOTPA_ERROR_LOCK);
            }

            int ret=CommandResult.ROOTPA_OK;
            int[] containerState = new int[1];

            try{
                ret=commonPAWrapper().getSPContainerState(spid.spid(), containerState);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().getSPContainerState exception: ", e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            Log.d(TAG,"RootPAServiceIfc.Stub.getSPContainerState received " + containerState[0] + " "+ ret);

            if(ret==CommandResult.ROOTPA_OK){

                SPContainerState s=mapSpContainerState(containerState[0]);
          	    state.setEnumeratedValue(s);
                if (s == SPContainerState.UNDEFINED){
                    ret=CommandResult.ROOTPA_ERROR_INTERNAL;
                }
            }else if (ret==CommandResult.ROOTPA_ERROR_INTERNAL_NO_CONTAINER){
           	    state.setEnumeratedValue(SPContainerState.DOES_NOT_EXIST);
                ret=CommandResult.ROOTPA_OK;
            }

            CommandResult res=ProvisioningService.this.releaseLock(internalUidForLock, false);
            if(!res.isOk()){
                Log.e(TAG,"releasing lock failed, res: "+res.result());
                // this return code is not returned to the client since
                // the command may have succeeded and there is just something wrong with the lock
                // we leave it the the next command if the problem remains
            }


            Log.d(TAG,"<<RootPAServiceIfc.Stub.getSPContainerState");
            return new CommandResult(ret);
        }

        public CommandResult storeTA(SPID spid, byte[] uuid, byte[] taBinary){
            Log.d(TAG,">>RootPAServiceIfc.Stub.storeTA");
            if(spid==null||uuid==null||taBinary==null|| taBinary.length == 0 || spid.spid()==0){ // having null out variable leads to null pointer exception in the client, however we still want to do checking so that there is not unncessary execution of the following code
                return new CommandResult(CommandResult.ROOTPA_ERROR_ILLEGAL_ARGUMENT);
            }

            int ret=CommandResult.ROOTPA_OK;
            try{
                ret=commonPAWrapper().storeTA(spid.spid(), uuid, taBinary);
            }catch(Exception e){
                Log.e(TAG,"CommonPAWrapper().storeTA exception: ", e);
                ret=CommandResult.ROOTPA_ERROR_INTERNAL;
            }

            Log.d(TAG,"<<RootPAServiceIfc.Stub.storeTA");
            return new CommandResult(ret);
        }

        private final static int  MC_CONT_STATE_UNREGISTERED=0;
        private final static int  MC_CONT_STATE_REGISTERED=1;
        private final static int  MC_CONT_STATE_ACTIVATED=2;
        private final static int  MC_CONT_STATE_ROOT_LOCKED=3;
        private final static int  MC_CONT_STATE_SP_LOCKED=4;
        private final static int  MC_CONT_STATE_ROOT_SP_LOCKED=5;

        private TrustletContainerState mapTltContainerState(int containerState){
            TrustletContainerState state=TrustletContainerState.UNDEFINED;
            switch (containerState){
                case MC_CONT_STATE_REGISTERED:
                    state=TrustletContainerState.REGISTERED;
                    break;
                case MC_CONT_STATE_ACTIVATED:
                    state=TrustletContainerState.ACTIVATED;
                    break;
                case MC_CONT_STATE_SP_LOCKED:
               	    state=TrustletContainerState.SP_LOCKED;
                    break;
                default:
                    Log.e(TAG,"mapTltContainerState returning undefined: "+ containerState);
               	    state=TrustletContainerState.UNDEFINED;
                    break;
            }
            return state;
        }

        private SPContainerState mapSpContainerState(int containerState){
            SPContainerState state=SPContainerState.UNDEFINED;
            switch (containerState){
                case MC_CONT_STATE_UNREGISTERED:
                    state=SPContainerState.DOES_NOT_EXIST;
                    break;
                case MC_CONT_STATE_REGISTERED:
                    state=SPContainerState.REGISTERED;
                    break;
                case MC_CONT_STATE_ACTIVATED:
                    state=SPContainerState.ACTIVATED;
                    break;
                case MC_CONT_STATE_ROOT_LOCKED:
               	    state=SPContainerState.ROOT_LOCKED;
                    break;
                case MC_CONT_STATE_SP_LOCKED:
               	    state=SPContainerState.SP_LOCKED;
                    break;
                case MC_CONT_STATE_ROOT_SP_LOCKED:
               	    state=SPContainerState.ROOT_SP_LOCKED;
                    break;
                default:
                    Log.e(TAG,"mapSpContainerState returning undefined: "+ containerState);
               	    state=SPContainerState.UNDEFINED;
                    break;
            }
            return state;
        }
    };

    @Override
    public void onCreate() {
        Log.d(TAG,"Hello, ProvisioningService onCreate");
        super.onCreate();
    }

    @Override
    public void onLowMemory() {
        Log.d(TAG,"ProvisioningService onLowMemory");
        super.onLowMemory();
    }

    public void onDestroy(){
        Log.d(TAG,"ProvisioningService being destroyed");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent){
        try{
            se_ = intent.getByteArrayExtra("SE");
        }catch(Exception e){
            Log.i(TAG,"ProvisioningService something wrong in the given ip "+e );
        }

        try{
            Log.setLoggingLevel(intent.getIntExtra("LOG",0));
        }catch(Exception e){
            Log.i(TAG,"ProvisioningService something wrong in the given logging level "+e );
        }
        Log.i(TAG,"ProvisioningService binding, IfcVersion: " +IfcVersion.ROOTPA_ANDROID_API_VERSION_MAJOR+"."+IfcVersion.ROOTPA_ANDROID_API_VERSION_MINOR);
        if(se_!=null) Log.d(TAG,new String(se_));
        return mBinder;
    }

    @Override
    public int  onStartCommand(Intent i, int flags, int startid){
        Log.d(TAG,"ProvisioningService starting");
        return START_STICKY;
    }
}
