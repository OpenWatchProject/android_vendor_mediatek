/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.ims.rcsua;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * IMS registration information.
 */
public class RegistrationInfo implements Parcelable{

    public RegistrationInfo(int regState, int radioTech, int regMode) {
        this.regState = regState;
        this.radioTech = radioTech;
        this.regMode = regMode;
        this.imsConfig = new Configuration();
    }

    public RegistrationInfo() {
        this(RcsUaService.REG_STATE_NOT_REGISTERED,
                RcsUaService.REG_RADIO_NONE,
                RcsUaService.REG_MODE_IMS);
    }

    protected RegistrationInfo(Parcel in) {
        regState = in.readInt();
        radioTech = in.readInt();
        regMode = in.readInt();
        imsConfig = in.readParcelable(Configuration.class.getClassLoader());
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(regState);
        dest.writeInt(radioTech);
        dest.writeInt(regMode);
        dest.writeParcelable(imsConfig, flags);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<RegistrationInfo> CREATOR = new Creator<RegistrationInfo>() {
        @Override
        public RegistrationInfo createFromParcel(Parcel in) {
            return new RegistrationInfo(in);
        }

        @Override
        public RegistrationInfo[] newArray(int size) {
            return new RegistrationInfo[size];
        }
    };

    @Override
    public String toString() {
        String result = "RegistrationInfo: ";

        result += "regState[" + regState + "]";
        result += ",radioTech[" + radioTech + "]";
        result += ",regMode[" + regMode + "]";

        return result;
    }

    public @RcsUaService.RegistrationState int getRegState() {
        return regState;
    }

    public @RcsUaService.ImsRadioTech int getRadioTech() {
        return radioTech;
    }

    public @RcsUaService.RegistrationMode int getRegMode() {
        return regMode;
    }

    public Configuration getImsConfig() {
        return imsConfig;
    }

    /**
     * Read current IMS configuration.
     *
     * @return null in case IMS not registered.
     */
    public Configuration readImsConfiguration() {
        return isRegistered() ? new Configuration(imsConfig) : null;
    }

    /**
     * Check whether IMS registration is established.
     *
     * @return true for registered, false for otherwise.
     */
    public boolean isRegistered() {
        return regState != RcsUaService.REG_STATE_NOT_REGISTERED;
    }

    public boolean isRegistered(@RcsUaService.RegistrationMode int mode) {
        return regMode == mode
                && regState != RcsUaService.REG_STATE_NOT_REGISTERED;
    }

    public void setRegState(int regState) {
        this.regState = regState;
    }

    public void setRadioTech(int radioTech) {
        this.radioTech = radioTech;
    }

    public void setRegMode(int regMode) {
        this.regMode = regMode;
    }

    public void setImsConfiguration(final Configuration imsConfig) {
        this.imsConfig = imsConfig;
    }

    private int regState;
    private int radioTech;
    private int regMode;
    private Configuration imsConfig;
}
