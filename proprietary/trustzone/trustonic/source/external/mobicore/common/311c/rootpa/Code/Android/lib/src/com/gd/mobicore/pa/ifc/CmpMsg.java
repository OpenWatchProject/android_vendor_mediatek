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
    Base class for content management protocol commands and responses
    @see CmpCommand
    @see CmpResponse
*/
public abstract class CmpMsg implements Parcelable{

// CMP message ID's
    public static final int MC_CMP_CMD_AUTHENTICATE=0;
    public static final int MC_CMP_CMD_BEGIN_ROOT_AUTHENTICATION=1;
    public static final int MC_CMP_CMD_BEGIN_SOC_AUTHENTICATION=2;
    public static final int MC_CMP_CMD_BEGIN_SP_AUTHENTICATION=3;
    public static final int MC_CMP_CMD_GENERATE_AUTH_TOKEN=4;
    public static final int MC_CMP_CMD_GET_VERSION=5;

    public static final int MC_CMP_CMD_ROOT_CONT_LOCK_BY_ROOT=7;

    public static final int MC_CMP_CMD_ROOT_CONT_REGISTER_ACTIVATE=9;
    public static final int MC_CMP_CMD_ROOT_CONT_UNLOCK_BY_ROOT=10;
    public static final int MC_CMP_CMD_ROOT_CONT_UNREGISTER=11;
    public static final int MC_CMP_CMD_SP_CONT_ACTIVATE=12;
    public static final int MC_CMP_CMD_SP_CONT_LOCK_BY_ROOT=13;
    public static final int MC_CMP_CMD_SP_CONT_LOCK_BY_SP=14;
    public static final int MC_CMP_CMD_SP_CONT_REGISTER=15;
    public static final int MC_CMP_CMD_SP_CONT_REGISTER_ACTIVATE=16;
    public static final int MC_CMP_CMD_SP_CONT_UNLOCK_BY_ROOT=17;
    public static final int MC_CMP_CMD_SP_CONT_UNLOCK_BY_SP=18;
    public static final int MC_CMP_CMD_SP_CONT_UNREGISTER=19;
    public static final int MC_CMP_CMD_TLT_CONT_ACTIVATE=20;
    public static final int MC_CMP_CMD_TLT_CONT_LOCK_BY_SP=21;
    public static final int MC_CMP_CMD_TLT_CONT_PERSONALIZE=22;
    public static final int MC_CMP_CMD_TLT_CONT_REGISTER=23;
    public static final int MC_CMP_CMD_TLT_CONT_REGISTER_ACTIVATE=24;
    public static final int MC_CMP_CMD_TLT_CONT_UNLOCK_BY_SP=25;
    public static final int MC_CMP_CMD_TLT_CONT_UNREGISTER=26;
    public static final int MC_CMP_CMD_GET_SUID=27;
    public static final int MC_CMP_CMD_AUTHENTICATE_TERMINATE=28;

// indices and masks
    public static final int MSG_ID_IDX=0;
    public static final int INT_LENGTH=4;
    public static final int RSP_ID_MASK=(1 << 31);

    /**
        helper method to convert given command id to response id
        @param commandId id to be converted
        @return response id corresponding to given command id
    */
    public static final int commandIdToResponseId(int commandId){
        return (RSP_ID_MASK|commandId);
    }

    /**
        helper method to convert given command id to response id
        @param responseId id to be converted
        @return command id corresponding to given response id
    */
    public static final int responseIdToCommandId(int responseId){
        return (RSP_ID_MASK^responseId);
    }


    /**
        Constructor that takes in the whole message as by array
        @param content the whole message
    */
    public CmpMsg(byte[] content){
        super();
        setContent(content);
    }

    /**
        default constructor
    */
    public CmpMsg(){
        super();
    }

    /**
        @return size of the message
    */
    public int size(){
        if(content_==null) return 0;
        return content_.length;
    }

    /**
        Sets the whole message, if anything has been set before this method is called, it will be overwritten
        @param content the whole message
    */
    public void setContent(byte[] content)
    {
        content_=content;
        setLength(content.length);
    }

    /**
        @return the whole message as byte array
    */
    public byte[] toByteArray()
    {
        return content_;
    }


    protected int msgId()
    {
        return getInt(MSG_ID_IDX);
    }

    protected void setMsgId(int id)
    {
        setInt(MSG_ID_IDX, id);
    }


    public abstract void setLength(int length);

    private void createEmptyContentIfNeeded(int index, int size){
        if(content_==null){
            content_=new byte[size];
            setLength(content_.length); // we keep the length field automatically up to date
        }else if(content_.length<(index+size)){
            byte[] newarray=new byte[index+size];
            System.arraycopy(content_, 0, newarray, 0, content_.length);
            content_=newarray;
            setLength(content_.length); // we keep the length field automatically up to date
        }
    }
// generic setters ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
        Set an integer in given index in byte array. If the array is not long enough, lenghtens the array as needed
        @param index index of the start of the integer in byte array
        @param value to be set
    */
    public void setInt(int index, int value)
    {
        createEmptyContentIfNeeded(index, INT_LENGTH);
        content_[index+0]=(byte) (value & 0xFF);
        content_[index+1]=(byte)((value >> 8) & 0xFF);
        content_[index+2]=(byte) ((value >> 16) & 0xFF);
        content_[index+3]=(byte) ((value >> 24) & 0xFF);
    }


    /**
        Set an integer in given index in byte array. If the array is not long enough, lenghtens the array as needed
        @param index index of the start of the integer in byte array
        @param addBytes bytes to be added
    */
    public void setByteArray(int index, byte[] addBytes)
    {
        createEmptyContentIfNeeded(index,addBytes.length);
        System.arraycopy(addBytes, 0, content_, index, addBytes.length);
    }


// generic getters ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    /**
        Gets integer value from the message
        @param index index of the start of the integer in byte array
        @return integer stored in the given index
        @throws ArrayIndexOutOfBoundsException if given index is out of bounds of the stored message
    */
    public int getInt(int index) throws ArrayIndexOutOfBoundsException
    {
        return (content_[index] & 0xFF) +
               ((content_[index+1] & 0xFF) << 8) +
               ((content_[index+2] & 0xFF) << 16) +
               ((content_[index+3] & 0xFF) << 24);
    }

    /**
        Gets byte array from the message
        @param index index of the start of the requested byte array
        @param length length of the byte array to be returned
        @return byte array stored in the given index
        @throws ArrayIndexOutOfBoundsException if given index is out of bounds of the stored message
    */
    public byte[] getByteArray(int index, int length) throws ArrayIndexOutOfBoundsException
    {
        byte[] newarray=new byte[length];
        System.arraycopy(content_, index, newarray, 0, length);
        return newarray;
    }


    /**
        Gets string value from the message
        @param index index of the start of the string in byte array
        @param length length of the byte array to be returned as string
        @return string stored in the given index
        @throws ArrayIndexOutOfBoundsException if given index is out of bounds of the stored message
    */
    public String getString(int index, int length) throws ArrayIndexOutOfBoundsException
    {
        return new String(content_,index,length);
    }

// data ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    private byte[] content_;
    boolean ignoreError_=false;  // this is moved from CmpCommand since
                                 // for some reason it is always false
                                 // if CmpCommand.ignoreError() called
                                 // with JNI CallBooleanMethod but if
                                 // this is here it works

// stuff related to parcelable ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    /**
        constructor required by Parcelable
    */
    public CmpMsg(Parcel in){
        readFromParcel(in);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags){
        out.writeByteArray(content_);
    }

    public void readFromParcel(Parcel in){
        content_=in.createByteArray();
    }
}
