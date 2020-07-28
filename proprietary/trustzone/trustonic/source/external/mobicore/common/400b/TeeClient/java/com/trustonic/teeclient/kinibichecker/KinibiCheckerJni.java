/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
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
package com.trustonic.teeclient.kinibichecker;

import android.content.Context;

public class KinibiCheckerJni {
    private Context mContext;

    private String[] mInfoList = { "Suid", "ProductId", "Control Interface",
        "Secure Object", "Load Format", "Container", "MobiConfig",
        "Trustlet API", "Driver API", "CMP" };

    private String mSuid;
    private String mProductId;
    private String mMci;
    private String mSo;
    private String mMclf;
    private String mContainer;
    private String mMcConfig;
    private String mTlApi;
    private String mDrApi;
    private String mCmp;

    private int returnCode;


    public KinibiCheckerJni(Context context) {
        mContext = context;
        returnCode = getInfo(mContext.getApplicationContext(), 3, 0);
    }

    public int getCount() {
        return mInfoList.length;
    }

    public String getProductId() {
        return mProductId;
    }

    public String getSuid() {
        return mSuid;
    }

    public int getReturnCode(){
        return returnCode;
    }

    public native int getInfo(Context appContext, int cmpVersionMajor, int cmpVersionMinor);
    static {
        System.loadLibrary("TeeClient");
//        System.loadLibrary("KinibiInfo");
    }
}

