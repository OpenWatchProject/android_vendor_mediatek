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
    Class that contains content management protocol command. The class inherits is parcelable.
    @see CmpMsg
    @see CmpResponse
*/
public class CmpCommand extends CmpMsg{
    public static final int COMMAND_LENGTH_IDX=4;
    private static final int COMMAND_RESERVED_IDX=8;

    /**
    This is not directly tied to the content of the command but to execution of
    multiple commands. If ignoreError is set, the execution continues even if
    this command returns an error. By default ignoreError is false.
    @return boolean value telling whether the error should be ignored
    */
    public boolean ignoreError(){
        return ignoreError_;
    }

    /**
    This is not directly tied to the content of the command but to execution of
    multiple commands. If ignoreError is set, the execution continues even if
    this command returns an error. By default ignoreError is false.
    @param ignore boolean value telling whether the error should be ignored
    */
    public void setIgnoreError(boolean ignore){

        ignoreError_=ignore;
    }

    /**
        @return id of the command
    */
    public int commandId(){
        return msgId();
    }

    /**
        @param id command id, see possible values from CmpMsg
    */
    public void setCommandId(int id){
        setMsgId(id);
    }

    public void setLength(int length)
    {
        // MC_CMP_CMD_GET_VERSION, MC_CMP_CMD_GENERATE_AUTH_TOKEN and MC_CMP_CMD_GET_SUID do not have length field for legacy reasons
        if(msgId()!=MC_CMP_CMD_GET_VERSION
        && msgId()!=MC_CMP_CMD_GENERATE_AUTH_TOKEN
        && msgId()!=MC_CMP_CMD_GET_SUID
	    && length > 4) // not setting length for messages that only have id
        {
            setInt(COMMAND_LENGTH_IDX, length);
        }
    }

    /**
        Constructor
        @param commandId see possible values from CmpMsg
    */
    public CmpCommand(int commandId){
        super();
        setCommandId(commandId);
    }

    public CmpCommand(byte[] content){
        super(content);
    }

    /**
        Costructor required by Parcelable
    */
    public CmpCommand(Parcel in){
        super(in);
    }


    public static final Parcelable.Creator<CmpCommand> CREATOR = new Parcelable.Creator<CmpCommand>(){
        public CmpCommand createFromParcel(Parcel in){
            return new CmpCommand(in);
        }

        public CmpCommand[] newArray(int size){
            return null;
        }
    };

    @Override
    public void writeToParcel(Parcel out, int flags){
        out.writeByte((byte)((ignoreError_==true)?1:0));
        super.writeToParcel(out, flags);
    }

    public void readFromParcel(Parcel in){
        ignoreError_=(in.readByte()==1);
        super.readFromParcel(in);
    }

}
