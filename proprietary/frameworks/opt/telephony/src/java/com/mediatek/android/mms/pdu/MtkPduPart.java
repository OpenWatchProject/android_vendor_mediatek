/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
 package com.mediatek.android.mms.pdu;

 import com.google.android.mms.pdu.PduPart;
 public class MtkPduPart extends PduPart{
   /*
      * M: Whether the part should be update or not.
      * The default value should be true because 3rd app
      * would not change this.
      */
     private boolean mNeedUpdate = true;
    /// New variable and methods
    /// M:Code analyze 001,add for extendsional field,they are not support in google default code @{
    public static final int P_X_WAP_CONTENT_URI = 0xB0;
    public static final int P_DATE = 0x92;

    /// @}

    /// ALPS03901614.
    /// Transfer-encoding in mms part header
    /// If has Transfer-encoding in mms part headers, if can't recognize this Transfer-encoding,
    /// CONTENT_ID and CONTENT_LOCATION will be skip and not stored in db, this may cause
    /// mms part can't be shown in Mms UI
    public static final int P_TRANSFER_ENCODING = 0xA7;

    /// M:Code analyze 002,add some new string @{
    /**
     * Content=Id string.
     * @internal
     */
    public static final String CONTENT_TYPE =
            "Content-Type";
    /**
     * Content=Id string.
     */
    public static final String CONTENT_ID =
            "Content-ID";

    /**
     * Content=Location string.
     */
    public static final String CONTENT_LOCATION =
            "Content-Location";

    /**
     * Content=Id string.
     */
    public static final String CONTENT_DISPOSITION =
            "Content-Disposition";

 /**
     * Content=Name string.
     */
    public static final String PARA_NAME =
            "Name";
    /// @}


   public MtkPduPart() {
         super();
   }

       /// @}
    /// M:Code analyze 003,add a method for getting XWap uri @{
    /* Q0 migration*/
    /*
    public byte[] getXWapContentUri() {
        return (byte[]) mPartHeader.get(P_X_WAP_CONTENT_URI);
    }
    */
    /// @}

    /// M:Code analyze 004,add a method for setting XWap uri @P
    /* Q0 migration*/
    /*
    public void setXWapContentUri(byte[] wapcontenturi) {
        if (null == wapcontenturi) {
            throw new NullPointerException("null content-id");
        }

        mPartHeader.put(P_X_WAP_CONTENT_URI, wapcontenturi);
    }
    */
    /// @}


    /// M:Code analyze 005,add a method for getting Date @{
    /* Q0 migration*/
    /*
    public long getDate() {
        return ((Long) mPartHeader.get(P_DATE)).longValue();
    }
    */
    /// @}

    /// M:Code analyze 006,add a method for setting Date @{
    /* Q0 migration*/
    /*
    public void setDate(long date) {
        mPartHeader.put(P_DATE, date);
    }
    */
    /// @}

    /// M: Get and set value that the part need update or not. @{
    /* Q0 migration*/
    /*
    public boolean needUpdate() {
        return mNeedUpdate;
    }
    */

    /*
     * M: Part can be set as need update. According to this flag, to update part.
     * @internal
     */
    /* Q0 migration*/
    /*
    public void setNeedUpdate(boolean needUpdate) {
        this.mNeedUpdate = needUpdate;
    }
    */
    /// @}
}