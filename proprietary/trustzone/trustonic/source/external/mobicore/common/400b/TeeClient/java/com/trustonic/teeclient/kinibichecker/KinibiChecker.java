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

import java.util.EnumSet;
import android.content.Context;
import android.os.Build;


import android.util.Log;

import com.trustonic.teeclient.*;
import com.trustonic.teeclient.kinibichecker.TEEError;
import com.trustonic.teeclient.kinibichecker.KinibiCheckerJni;
import com.trustonic.teeclient.kinibichecker.FilesValidator;
import com.trustonic.teeclient.kinibichecker.SELinuxValidator;
import com.trustonic.teeclient.kinibichecker.FindKinibiVersion;

public class KinibiChecker {
    private final static int MC_ERR_DAEMON_UNREACHABLE = 15;
    private final static int MC_ERR_UNKNOWN = 6;
    private final static int MC_ERR_INVALID_OPERATION = 9;

    private final static int LOLLIPOP_MR1 = 22; //Android 5.1
    private final static int LOLLIPOP = 21; //Android 5.0
    private final static int KIKAT_WATCH = 20; //Android 4.4W
    private final static int KIKAT = 19; //Android 4.4
    private final static int JELLY_BEAN_MR2 = 18; //Android 4.3

    public static final String TAG="TEE-KCv2";

    private Context mContext;
    private String mProductID;
    private String mSuid;

    private TEEStatus mKCv2;

    public KinibiChecker(Context appContext) {
        mContext = appContext;

        mProductID = new String("UTF-8");
        mSuid = new String("UTF-8");
    }

    public String getProductID(){
        return mProductID;
    }

    public String getSuid() {
        return mSuid;
    }

    private void setProductID(String str){
        if ( mProductID != null )
            mProductID = str;
    }

    private void setSuid(String str){
        if ( mSuid != null )
            mSuid = str;
    }

    private void parseErrorcode(int jniRetCode )
    {
        if ( jniRetCode == 0 )
            return ;  // Don't need to check

        if( !isTDPAppInstalled(mContext) )
        {
            mKCv2.setTEEStatus(TEEError.TEE_PROXY_INSTALLATION_REQUIRED, "TDP is not installed");
            return;
        }

        switch(jniRetCode)
        {
                //case MC_ERR_DAEMON_DEVICE_NOT_OPEN:
                case MC_ERR_DAEMON_UNREACHABLE:
                case MC_ERR_INVALID_OPERATION:
                case MC_ERR_UNKNOWN:
                    if( isPolicyUpdateRequired())
                        mKCv2.setTEEStatus(TEEError.TEE_PROXY_UNREACHABLE_ERROR, TEEError.TEE_PROXY_UNREACHABLE_ERROR.toString());
                    else
                        mKCv2.setTEEStatus(TEEError.TEE_PROXY_LICENSE_REQUIRED, TEEError.TEE_PROXY_LICENSE_REQUIRED.toString());
                    break;

                default:
                        mKCv2.setTEEStatus(TEEError.TEE_NOT_SUPPORTED, "TeeProxyservice error(" + jniRetCode + ")");
                    break;
         }

       printTEECheckResult();
       return ;
    }

    private void printTEECheckResult()
    {
        if(mKCv2.cause != null)
            Log.d("TEEError","Reason: "+ mKCv2.cause);
    }

    private EnumSet<TEEError> getStateCheck_TeeHistoricalIssue(String teeVersion) {

        EnumSet<TEEError> deviceErrata = EnumSet.noneOf(TEEError.class);

        int exist = new FindKinibiVersion(teeVersion).getResult();
        if (exist > 0) deviceErrata.add(TEEError.TEE_NOT_SUPPORTED);

        return deviceErrata;
    }

    private boolean isAvailable_NwdComponent() {
        return isMcDeviceAvailable()&&isRegistryAccessible() ;
    }

    private boolean isAvailable_OTAComponent() {
        return isRootPaAvailable();
    }

    private boolean isRegistryAccessible(){
        return new FilesValidator().hasValidMcRegistryPermissionsExt();
    }

    private boolean isMcDeviceAvailable(){
        return new FilesValidator().hasValidMcNodePermissions();
    }

    private boolean isRootPaAvailable(){
        return new FilesValidator().hasValidRootPA();
    }

    private boolean isTDPAppInstalled(Context c){
        return new TeeClient(c).isTeeProxyServiceInstalled();
    }

    private boolean isPolicyUpdateRequired() {

        boolean result = false;
        //Android 4.3 ~ 5.0 has a problem of SeLinux Policy in Samunsg device.
        String SelinuxVer = new SELinuxValidator().getSELinuxVersion();

        if ( (SelinuxVer == null) || (Build.VERSION.SDK_INT > LOLLIPOP_MR1) )
                return result;

        switch (Build.VERSION.SDK_INT)
        {
            case JELLY_BEAN_MR2: //API 18, 4.3
            case KIKAT:          //API 19, 4.4
            case KIKAT_WATCH:    //API 20, 4.4W
                result = true;
                break;

            case LOLLIPOP:      //API 21, 5.0
            case LOLLIPOP_MR1:  //API 22, 5.1
                String str1 = SelinuxVer.substring(SelinuxVer.length()-4, SelinuxVer.length());
                String refstr = "0036";

                if ( str1.compareTo(refstr) < 0 ) //trim 1 < trim2
                {
                    result = true;
                }
                break;

            default:
                break;
        }

        return result;

    }

    /*****************/
    /* Public Method */
    /*****************/

    public boolean isTuiAvailable(){
        return new FilesValidator().hasTUI();
    }

    public EnumSet<TEEError> getDeviceErrata()
    {
        mKCv2 = new TEEStatus();

        //1. Integration Check
        if(!isAvailable_NwdComponent() )
        {
            mKCv2.setTEEStatus(TEEError.TEE_NOT_SUPPORTED,"Cannot access to NWd Component");
        } else if ( !isAvailable_OTAComponent() ) {
            mKCv2.setTEEStatus(TEEError.TEE_NOT_SUPPORTED,"OTA component isn't available");
        } else {
            try {
                KinibiCheckerJni infoAdapter = new KinibiCheckerJni(mContext);
                int jniRetCode = infoAdapter.getReturnCode();

                if (jniRetCode != 0)
                {
                    parseErrorcode(jniRetCode);
                    return mKCv2.teeError;
                }

                String teeVersion = infoAdapter.getProductId() == null ? "" : infoAdapter.getProductId();
                String suid = infoAdapter.getSuid() == null ? "" : infoAdapter.getSuid();

                if (teeVersion != null) setProductID(teeVersion);
                if (suid != null ) setSuid(suid);

                if (teeVersion.isEmpty() || suid.isEmpty())
                    mKCv2.setTEEStatus(TEEError.TEE_NOT_SUPPORTED,"Couldn't getVersion from SWd");
                else
                {
                    String invalid_suid ="00000000000000000000000000000000";
                    String str =suid.replaceAll("\\p{Z}", ""); //Remove space from suid buffer.

                    if(str.equals(invalid_suid))
                        mKCv2.setTEEStatus(TEEError.TEE_NOT_SUPPORTED, "Invalid SUID:"+suid);
                    else if ( !getStateCheck_TeeHistoricalIssue(teeVersion).isEmpty())
                        mKCv2.setTEEStatus(TEEError.TEE_NOT_SUPPORTED, "historical issue:" + teeVersion);



                }
            } catch (UnsatisfiedLinkError e) {
                mKCv2.setTEEStatus(TEEError.TEE_NOT_SUPPORTED,"Cannot load client library");

            } catch (ExceptionInInitializerError e) {
                mKCv2.setTEEStatus(TEEError.TEE_NOT_SUPPORTED,"Cannot load client library");
            }
        }

        printTEECheckResult();
        return mKCv2.teeError;
    }

}

class TEEStatus {

    EnumSet<TEEError> teeError;
    String cause;

    public TEEStatus()
    {
        teeError = EnumSet.noneOf(TEEError.class);
        cause = null;
    }

    public void setTEEStatus( TEEError err , String str)
    {
        if(err ==null || str == null) return;

        teeError.add(err);
        cause = str;
    }
}
