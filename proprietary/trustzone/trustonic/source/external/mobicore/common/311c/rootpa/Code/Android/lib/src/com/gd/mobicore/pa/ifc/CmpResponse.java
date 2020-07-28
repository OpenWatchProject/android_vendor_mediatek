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
    Class that contains content management protocol response. The class inherits parcelable.
    @see CmpMsg
    @see CmpCommand
*/
public class CmpResponse extends CmpMsg{
    private static final int RETURN_CODE_IDX=4;
    private static final int RESPONSE_LENGTH_IDX=8;
    private static final int RESPONSE_RESERVED_IDX=12;

    /**
        @return response id
    */
    public int responseId(){
        return msgId();
    }

    /**
        @param id respose id of the message
    */
    public void setResponseId(int id){
        setMsgId(id);
    }

    /**
        @return returnCode
    */
    public int returnCode(){
        return getInt(RETURN_CODE_IDX);
    }

    /**
        @param ret return code for the message
    */
    public void setReturnCode(int ret){
        setInt(RETURN_CODE_IDX, ret);
    }

    public void setLength(int length)
    {
        // MC_CMP_CMD_GET_VERSION and MC_CMP_CMD_GET_SUID do not have length field for legacy reasons
        if(responseId()!=responseIdToCommandId(MC_CMP_CMD_GET_VERSION)
        && responseId()!=responseIdToCommandId(MC_CMP_CMD_GENERATE_AUTH_TOKEN)
        && responseId()!=responseIdToCommandId(MC_CMP_CMD_GET_SUID)
        & length > 0)  // not setting length for messages that only have id
        {
            setInt(RESPONSE_LENGTH_IDX, length);
        }
    }


    /**
        Default constructor
    */
    public CmpResponse(){
        super();
    }

    /**
        Constructor
        @param content the whole response as byte array
    */
    public CmpResponse(byte[] content){
        super(content);
    }

    /**
        Constructor required by parcelable
    */
    public CmpResponse(Parcel in){
        super(in);
    }

    public static final Parcelable.Creator<CmpResponse> CREATOR = new Parcelable.Creator<CmpResponse>(){
        public CmpResponse createFromParcel(Parcel in){
            return new CmpResponse(in);
        }

        public CmpResponse[] newArray(int size){
            return null;
        }
    };


}
