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
import android.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * IMS configuration retrieved after IMS registration.
 */
public class Configuration implements Parcelable {

    /**
     * Transport used for IMS registration
     */
    @IntDef({
            TCP,
            UDP,
            TLS,
            SCTP
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface Transport {}

    /**
     * Transport TCP
     */
    public static final int TCP = 1;
    /**
     * Transport UDP
     */
    public static final int UDP = 2;
    /**
     * Transport TLS, RFU.
     */
    public static final int TLS = 4;
    /**
     * Transport SCTP, RFU.
     */
    public static final int SCTP = 8;

    /**
     * IP version used for IMS registration
     */
    @IntDef({
            IPv4,
            IPv6
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface IpVersion {}

    /**
     * IP protocol version 4
     */
    public static final int IPv4 = 0;
    /**
     * IP protocol version 6
     */
    public static final int IPv6 = 1;

    public Configuration() {
        init();
    }

    public Configuration(Configuration other) {
        localAddress = other.localAddress;
        localPort = other.localPort;
        proxyAddress = other.proxyAddress;
        proxyPort = other.proxyPort;
        protocol = other.protocol;
        ipVersion = other.ipVersion;
        IMPU = other.IMPU;
        IMPI = other.IMPI;
        homeDomain = other.homeDomain;
        userAgent = other.userAgent;
        pAssociatedUri = other.pAssociatedUri;
        instanceId = other.instanceId;
        serviceRoute = other.serviceRoute;
        pAccessNetworkInfo = other.pAccessNetworkInfo;
        pLastAccessNetworkInfo = other.pLastAccessNetworkInfo;
        publicGruu = other.publicGruu;
        tempGruu = other.tempGruu;
        digitVlineNumber = other.digitVlineNumber;
        pIdentifier = other.pIdentifier;
        pPreferredAssociation = other.pPreferredAssociation;
        regRcsFeatureTags = other.regRcsFeatureTags;
    }

    protected Configuration(Parcel in) {
        localAddress = in.readString();
        localPort = in.readInt();
        proxyAddress = in.readString();
        proxyPort = in.readInt();
        protocol = in.readInt();
        ipVersion = in.readInt();
        IMPU = in.readString();
        IMPI = in.readString();
        homeDomain = in.readString();
        userAgent = in.readString();
        pAssociatedUri = in.readString();
        instanceId = in.readString();
        serviceRoute = in.readString();
        pAccessNetworkInfo = in.readString();
        pLastAccessNetworkInfo = in.readString();
        publicGruu = in.readString();
        tempGruu = in.readString();
        digitVlineNumber = in.readInt();
        pIdentifier = in.readString();
        pPreferredAssociation = in.readString();
        regRcsFeatureTags = in.readInt();
    }

    public static final Creator<Configuration> CREATOR = new Creator<Configuration>() {
        @Override
        public Configuration createFromParcel(Parcel in) {
            return new Configuration(in);
        }

        @Override
        public Configuration[] newArray(int size) {
            return new Configuration[size];
        }
    };

    public void reset() {
        init();
    }

    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("Configuration->");
        builder.append("localAddress[");
        builder.append(localAddress);
        builder.append("],");
        builder.append("localPort[");
        builder.append(localPort);
        builder.append("],");
        builder.append("proxyAddress[");
        builder.append(proxyAddress);
        builder.append("],");
        builder.append("proxyPort[");
        builder.append(proxyPort);
        builder.append("],");
        builder.append("protocol[");
        builder.append(protocol);
        builder.append("],");
        builder.append("ipVersion[");
        builder.append(ipVersion);
        builder.append("],");
        builder.append("IMPU[");
        builder.append(IMPU);
        builder.append("],");
        builder.append("IMPI[");
        builder.append(IMPI);
        builder.append("],");
        builder.append("homeDomain[");
        builder.append(homeDomain);
        builder.append("],");
        builder.append("userAgent[");
        builder.append(userAgent);
        builder.append("],");
        builder.append("pAssociatedUri[");
        builder.append(pAssociatedUri);
        builder.append("],");
        builder.append("instanceId[");
        builder.append(instanceId);
        builder.append("],");
        builder.append("serviceRoute[");
        builder.append(serviceRoute);
        builder.append("],");
        builder.append("pAccessNetworkInfo[");
        builder.append(pAccessNetworkInfo);
        builder.append("],");
        builder.append("pLastAccessNetworkInfo[");
        builder.append(pLastAccessNetworkInfo);
        builder.append("],");
        builder.append("publicGruu[");
        builder.append(publicGruu);
        builder.append("],");
        builder.append("tempGruu[");
        builder.append(tempGruu);
        builder.append("],");
        builder.append("regRcsFeatureTags[");
        builder.append(regRcsFeatureTags);
        builder.append("]");

        return builder.toString();
    }

    public String getLocalAddress() {
        return localAddress;
    }

    public int getLocalPort() {
        return localPort;
    }

    public String getProxyAddress() {
        return proxyAddress;
    }

    public int getProxyPort() {
        return proxyPort;
    }

    public @Transport int getProtocol() {
        return protocol;
    }

    public @IpVersion int getIpVersion() {
        return ipVersion;
    }

    public String getIMPU() {
        return IMPU;
    }

    public String getIMPI() {
        return IMPI;
    }

    public String getHomeDomain() {
        return homeDomain;
    }

    public String getUserAgent() {
        return userAgent;
    }

    public String[] getPAssociatedUri() {
        return pAssociatedUri.split(",");
    }

    public String getInstanceId() {
        return instanceId;
    }

    public String[] getServiceRoute() {
        return serviceRoute.split(",");
    }

    public String getPAccessNetworkInfo() {
        return pAccessNetworkInfo;
    }

    public String getPLastAccessNetworkInfo() {
        return pLastAccessNetworkInfo;
    }

    public String getPublicGruu() {
        return publicGruu;
    }

    public String getTempGruu() {
        return tempGruu;
    }

    public int getDigitVlineNumber() {
        return digitVlineNumber;
    }

    public String getPIdentifier() {
        return pIdentifier;
    }

    public String getPPreferredAssociation() {
        return pPreferredAssociation;
    }

    public int getRegRcsFeatureTags() {
        return regRcsFeatureTags;
    }

    public void setLocalAddress(String localAddress) {
        this.localAddress = localAddress;
    }

    public void setLocalPort(int localPort) {
        this.localPort = localPort;
    }

    public void setProxyAddress(String proxyAddress) {
        this.proxyAddress = proxyAddress;
    }

    public void setProxyPort(int proxyPort) {
        this.proxyPort = proxyPort;
    }

    public void setProtocol(int protocol) {
        this.protocol = protocol;
    }

    public void setIpVersion(int ipVersion) {
        this.ipVersion = ipVersion;
    }

    public void setIMPU(String IMPU) {
        this.IMPU = IMPU;
    }

    public void setIMPI(String IMPI) {
        this.IMPI = IMPI;
    }

    public void setHomeDomain(String homeDomain) {
        this.homeDomain = homeDomain;
    }

    public void setUserAgent(String userAgent) {
        this.userAgent = userAgent;
    }

    public void setPAssociatedUri(String pAssociatedUri) {
        this.pAssociatedUri = pAssociatedUri;
    }

    public void setInstanceId(String instanceId) {
        this.instanceId = instanceId;
    }

    public void setServiceRoute(String serviceRoute) {
        this.serviceRoute = serviceRoute;
    }

    public void setPAccessNetworkInfo(String pAccessNetworkInfo) {
        this.pAccessNetworkInfo = pAccessNetworkInfo;
    }

    public void setPLastAccessNetworkInfo(String pLastAccessNetworkInfo) {
        this.pLastAccessNetworkInfo = pLastAccessNetworkInfo;
    }

    public void setPublicGruu(String publicGruu) {
        this.publicGruu = publicGruu;
    }

    public void setTempGruu(String tempGruu) {
        this.tempGruu = tempGruu;
    }

    public void setDigitVlineNumber(int digitVlineNumber) {
        this.digitVlineNumber = digitVlineNumber;
    }

    public void setPIdentifier(String pIdentifier) {
        this.pIdentifier = pIdentifier;
    }

    public void setPPreferredAssociation(String pPreferredAssociation) {
        this.pPreferredAssociation = pPreferredAssociation;
    }

    public void setRegRcsFeatureTags(int regRcsFeatureTags) {
        this.regRcsFeatureTags = regRcsFeatureTags;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(localAddress);
        dest.writeInt(localPort);
        dest.writeString(proxyAddress);
        dest.writeInt(proxyPort);
        dest.writeInt(protocol);
        dest.writeInt(ipVersion);
        dest.writeString(IMPU);
        dest.writeString(IMPI);
        dest.writeString(homeDomain);
        dest.writeString(userAgent);
        dest.writeString(pAssociatedUri);
        dest.writeString(instanceId);
        dest.writeString(serviceRoute);
        dest.writeString(pAccessNetworkInfo);
        dest.writeString(pLastAccessNetworkInfo);
        dest.writeString(publicGruu);
        dest.writeString(tempGruu);
        dest.writeInt(digitVlineNumber);
        dest.writeString(pIdentifier);
        dest.writeString(pPreferredAssociation);
        dest.writeInt(regRcsFeatureTags);
    }

    private void init() {
        localAddress = "";
        localPort = 0;
        proxyAddress = "";
        proxyPort = 0;
        protocol = UDP;
        ipVersion = IPv4;
        IMPU = "";
        IMPI = "";
        homeDomain = "";
        userAgent = "";
        pAssociatedUri = "";
        instanceId = "";
        serviceRoute = "";
        pAccessNetworkInfo = "";
        pLastAccessNetworkInfo = "";
        publicGruu = "";
        tempGruu = "";
        digitVlineNumber = 0;
        pIdentifier = "";
        pPreferredAssociation = "";
        regRcsFeatureTags = 0;
    }

    private String localAddress;
    private int localPort;
    private String proxyAddress;
    private int proxyPort;
    private @Transport int protocol;
    private @IpVersion int ipVersion;
    private String IMPU;
    private String IMPI;
    private String homeDomain;
    private String userAgent;
    private String pAssociatedUri;
    private String instanceId;
    private String serviceRoute;
    private String pAccessNetworkInfo;
    private String pLastAccessNetworkInfo;
    private String publicGruu;
    private String tempGruu;
    private int digitVlineNumber;
    private String pIdentifier;
    private String pPreferredAssociation;
    private int regRcsFeatureTags;
}
