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

package com.gd.mobicore.pa.ifc;

import android.os.Parcel;
import android.os.Parcelable;

/**
    Result of commands requested from interfaces in com.gd.mobicore.pa.ifc package
*/
public class CommandResult implements Parcelable{

    /*
    NOTE to the maintainer. These values and documentation needs to be in line with the ones in rootpaErrors.h
    */

    /**
    No errors detected, successful execution
    */
    public static final int ROOTPA_OK=0x00000000;

    /**
    Client has requested unsupported CMP command or command that it can not execute via the used interface.
    Possible steps to recover: send only supported CMP commands or update to RootPA that supports handling the particular command in the used interface.
    */
    public static final int ROOTPA_COMMAND_NOT_SUPPORTED=0x00000001;

    /**
    Either rootpa is locked by another client, or the client requests lock or unlock when it is not allowed to do that.
    Possible steps to recover: wait until the lock is released
    */
    public static final int ROOTPA_ERROR_LOCK=0x00000002;

    /**
    Error in one of the CMP commands, see command specific response for more details.
    */
    public static final int ROOTPA_ERROR_COMMAND_EXECUTION=0x00000003;

    /**
    Registry returned an error when trying to write a container.  mcDaemon could be dead or something seriously wrong in the file system.
    Possible steps to recover: rebooting the device may help
    */
    public static final int ROOTPA_ERROR_REGISTRY=0x00000004;

    /**
    Error in communicating with t-base secure side. This is returned when any of the mcDeamon API calls related to communication with secure side fails.
    Possible steps to recover: rebooting the device may help
    */
    public static final int ROOTPA_ERROR_MOBICORE_CONNECTION=0x00000005;

    /**
    Either Nwd or Swd software is out of memory.
    Possible steps to recover: release memory
    */
    public static final int ROOTPA_ERROR_OUT_OF_MEMORY=0x00000006;

    /**
    Rootpa internal error. This error is returned in various situations when something unexpected went wrong e.g. message from CMTL can‘t be interpreted, SE returned an error indicating invalid data, bad request or similar or base64 decoding failed
    Possible steps to recover: rebooting or updating the device may help
    */
    public static final int ROOTPA_ERROR_INTERNAL=0x00000007;

    /**
    Given argument is not allowed (in many cases it is NULL) or e.g. the format of xml is unsupported.
    Possible steps to recover: give correct argument
    */
    public static final int ROOTPA_ERROR_ILLEGAL_ARGUMENT=0x00000008;

    /**
    Error in network connection or use of networking library.
    Possible steps to recover: create working network connection (avoid firewalls and proxies that require password)
    */
    public static final int ROOTPA_ERROR_NETWORK=0x00000009;

    /**
    Error returned by XML library. Problems in parsing received XML command or creating new XML response.
    */
    public static final int ROOTPA_ERROR_XML=0x0000000A;

    /**
    Registry returned an error when trying to read a container. Most likely the container does not exist.
    */
    public static final int ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE=0x0000000B;

    /**
    CMP version of the device is not supported by SE.
    Possible steps to recover: use CMP version supported by SE (>=3.0)
    */
    public static final int ROOTPA_ERROR_SE_CMP_VERSION=0x0000000C;

    /**
    Precoditions for SP container installation are not met in SE.
    Possible steps to recover: register used SPID to SE
    */
    public static final int ROOTPA_ERROR_SE_PRECONDITION_NOT_MET=0x0000000D;

    /**
    Requested SP container does not exist. This is not always considered an error but is used as an informative return code. As this is internal return code, user of RootPA services should never see this.
    Possible steps to recover: add SP container or request container with different SPID
    */
    public static final int ROOTPA_ERROR_INTERNAL_NO_CONTAINER=0x00000030;


    private int result_;

    public static final Parcelable.Creator<CommandResult> CREATOR = new Parcelable.Creator<CommandResult>(){
        public CommandResult createFromParcel(Parcel in){
            return new CommandResult(in);
        }

        public CommandResult[] newArray(int size){
            return null;
        }
    };

    /**
        Constructor
        @param result of the command
    */
    public CommandResult(int result){
        result_=result;
    }

    public CommandResult(){
        result_=ROOTPA_OK;
    }

    /**
        Constructor required by parcelable
    */
    public CommandResult(Parcel in){
        readFromParcel(in);
    }

    public int result(){
        return result_;
    }

    public void setValue(int value){
        result_=value;
    }

    public boolean isOk(){
        return (result_==ROOTPA_OK);
    }

    @Override
    public String toString() {
        String ret=new Integer(result_).toString();
        switch(result_){
            case ROOTPA_OK:
                ret+=": ROOTPA_OK";
                break;
            case ROOTPA_COMMAND_NOT_SUPPORTED:
                ret+=": ROOTPA_COMMAND_NOT_SUPPORTED";
                break;
            case ROOTPA_ERROR_INTERNAL:
                ret+=": ROOTPA_ERROR_INTERNAL";
                break;
            case ROOTPA_ERROR_LOCK:
                ret+=": ROOTPA_ERROR_LOCK";
                break;
            case ROOTPA_ERROR_ILLEGAL_ARGUMENT:
                ret+=": ROOTPA_ERROR_ILLEGAL_ARGUMENT";
                break;
            case ROOTPA_ERROR_INTERNAL_NO_CONTAINER:
                ret+=": ROOTPA_ERROR_INTERNAL_NO_CONTAINER";
                break;
            case ROOTPA_ERROR_COMMAND_EXECUTION:
                ret+=": ROOTPA_ERROR_COMMAND_EXECUTION";
                break;
            case ROOTPA_ERROR_REGISTRY:
                ret+=": ROOTPA_ERROR_REGISTRY";
                break;
            case ROOTPA_ERROR_MOBICORE_CONNECTION:
                ret+=": ROOTPA_ERROR_MOBICORE_CONNECTION";
                break;
            case ROOTPA_ERROR_OUT_OF_MEMORY:
                ret+=": ROOTPA_ERROR_OUT_OF_MEMORY";
                break;
            case ROOTPA_ERROR_NETWORK:
                ret+=": ROOTPA_ERROR_NETWORK";
                break;
            case ROOTPA_ERROR_XML:
                ret+=": ROOTPA_ERROR_XML";
                break;
            case ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE:
                ret+=": ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE";
                break;
            case ROOTPA_ERROR_SE_CMP_VERSION:
                ret+=": ROOTPA_ERROR_SE_CMP_VERSION";
                break;
            case ROOTPA_ERROR_SE_PRECONDITION_NOT_MET:
                ret+=": ROOTPA_ERROR_SE_PRECONDITION_NOT_MET";
                break;
            default:
                break;
        }

        return ret;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags){
        out.writeInt(result_);
    }

    private void readFromParcel(Parcel in){
        result_=in.readInt();
    }

}
