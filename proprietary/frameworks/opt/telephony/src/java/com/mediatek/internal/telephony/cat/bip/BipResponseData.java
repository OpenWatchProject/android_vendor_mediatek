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
 * have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony.cat;

import com.android.internal.telephony.EncodeException;
import com.android.internal.telephony.GsmAlphabet;
import java.util.Calendar;
import java.util.TimeZone;
import java.util.ArrayList;
import java.util.Iterator;
import android.os.SystemProperties;
import android.text.TextUtils;



import java.io.ByteArrayOutputStream;
import java.io.UnsupportedEncodingException;
import java.net.InetAddress;

import com.android.internal.telephony.cat.ComprehensionTlvTag;
import com.android.internal.telephony.cat.ResponseData;


abstract class BipResponseData extends ResponseData{

}

class OpenChannelResponseDataEx extends OpenChannelResponseData {
    int mProtocolType = -1;
    DnsServerAddress mDnsServerAddress = null;

    OpenChannelResponseDataEx(ChannelStatus channelStatus, BearerDesc bearerDesc, int bufferSize,
                              int protocolType) {
        super(channelStatus, bearerDesc, bufferSize);
        MtkCatLog.d("[BIP]",
                "OpenChannelResponseDataEx-constructor: protocolType "+ protocolType);
        mProtocolType = protocolType;
    }
    OpenChannelResponseDataEx(ChannelStatus channelStatus, BearerDesc bearerDesc, int bufferSize,
        DnsServerAddress dnsServerAddress) {
        super(channelStatus, bearerDesc, bufferSize);
        mDnsServerAddress = dnsServerAddress;
    }
    @Override
    public void format(ByteArrayOutputStream buf) {
        if (buf == null) {
            MtkCatLog.e("[BIP]", "OpenChannelResponseDataEx-format: buf is null");
            return;
        }
        if (BipUtils.TRANSPORT_PROTOCOL_TCP_REMOTE == mProtocolType ||
           BipUtils.TRANSPORT_PROTOCOL_UDP_REMOTE == mProtocolType) {
            if (null == mBearerDesc) {
                MtkCatLog.e("[BIP]", "OpenChannelResponseDataEx-format: bearer null");
                return;
            } else if ((mBearerDesc.bearerType != BipUtils.BEARER_TYPE_GPRS) &&
                       (mBearerDesc.bearerType != BipUtils.BEARER_TYPE_DEFAULT) &&
                       (mBearerDesc.bearerType != BipUtils.BEARER_TYPE_UTRAN) &&
                       (mBearerDesc.bearerType != BipUtils.BEARER_TYPE_EUTRAN)) {
                MtkCatLog.e("[BIP]", "OpenChannelResponseDataEx-format: bearer type is not gprs");
            }
        }
        int tag;
        int length = 0;
        if (mChannelStatus != null) {
            MtkCatLog.d("[BIP]", "OpenChannelResponseDataEx-format: Write channel status into TR");
            tag = ComprehensionTlvTag.CHANNEL_STATUS.value();
            buf.write(tag);
            length = 0x02;
            buf.write(length);
            buf.write(mChannelStatus.mChannelId | mChannelStatus.mChannelStatus); //For TCP status
            buf.write(mChannelStatus.mChannelStatusInfo);
            MtkCatLog.d("[BIP]", "OpenChannel Channel status Rsp:tag[" + tag + "],len[" + length +
                     "],cId[" + mChannelStatus.mChannelId + "],status[" +
                     mChannelStatus.mChannelStatus + "]");
        } else {
            MtkCatLog.d("[BIP]", "No Channel status in TR.");
        }
        if (mBearerDesc != null) {
            /*6.8, only required in response to OPEN CHANNEL proactive commands,
                        where Bearer description is mandatory in the command.*/
            MtkCatLog.d("[BIP]", "Write bearer description into TR. bearerType: " +
                    mBearerDesc.bearerType);
            tag = ComprehensionTlvTag.BEARER_DESCRIPTION.value();
            buf.write(tag);
            if (BipUtils.BEARER_TYPE_GPRS == mBearerDesc.bearerType) {
                if (mBearerDesc instanceof GPRSBearerDesc) {
                    GPRSBearerDesc gprsBD = (GPRSBearerDesc) mBearerDesc;
                    length = 0x07;
                    buf.write(length);
                    buf.write(gprsBD.bearerType);
                    buf.write(gprsBD.precedence);
                    buf.write(gprsBD.delay);
                    buf.write(gprsBD.reliability);
                    buf.write(gprsBD.peak);
                    buf.write(gprsBD.mean);
                    buf.write(gprsBD.pdpType);
                    MtkCatLog.d("[BIP]", "OpenChannelResponseDataEx-format: tag: " + tag
                            + ",length: " + length
                            + ",bearerType: " + gprsBD.bearerType
                            + ",precedence: " + gprsBD.precedence
                            + ",delay: " + gprsBD.delay
                            + ",reliability: " + gprsBD.reliability
                            + ",peak: " + gprsBD.peak
                            + ",mean: " + gprsBD.mean
                            + ",pdp type: " + gprsBD.pdpType);
                } else {
                    MtkCatLog.d("[BIP]", "Not expected GPRSBearerDesc instance");
                }
            } else if (BipUtils.BEARER_TYPE_EUTRAN == mBearerDesc.bearerType) {
                int[] bufferArr = new int[10];
                int index = 0;
                if (mBearerDesc instanceof EUTranBearerDesc) {
                    EUTranBearerDesc euTranBD = (EUTranBearerDesc) mBearerDesc;
                    if (euTranBD.QCI != 0) {
                        bufferArr[index] = euTranBD.QCI; index++;
                    }
                    if (euTranBD.maxBitRateU != 0) {
                        bufferArr[index] = euTranBD.maxBitRateU; index++;
                    }
                    if (euTranBD.maxBitRateD != 0) {
                        bufferArr[index] = euTranBD.maxBitRateD; index++;
                    }
                    if (euTranBD.guarBitRateU != 0) {
                        bufferArr[index] = euTranBD.guarBitRateU; index++;
                    }
                    if (euTranBD.guarBitRateD != 0) {
                        bufferArr[index] = euTranBD.guarBitRateD; index++;
                    }
                    if (euTranBD.maxBitRateUEx != 0) {
                        bufferArr[index] = euTranBD.maxBitRateUEx; index++;
                    }
                    if (euTranBD.maxBitRateDEx != 0) {
                        bufferArr[index] = euTranBD.maxBitRateDEx; index++;
                    }
                    if (euTranBD.guarBitRateUEx != 0) {
                        bufferArr[index] = euTranBD.guarBitRateUEx; index++;
                    }
                    if (euTranBD.guarBitRateDEx != 0) {
                        bufferArr[index] = euTranBD.guarBitRateDEx; index++;
                    }
                    if (euTranBD.pdnType != 0) {
                        bufferArr[index] = euTranBD.pdnType; index++;
                    }
                    MtkCatLog.d("[BIP]", "EUTranBearerDesc length: " + index);
                    if (0 < index) {
                        buf.write(index + 1);
                    } else {
                        buf.write(1);
                    }
                    buf.write(euTranBD.bearerType);
                    for (int i = 0; i < index; i++) {
                        buf.write(bufferArr[i]);
                        MtkCatLog.d("[BIP]", "EUTranBearerDesc buf: " + bufferArr[i]);
                    }
                } else {
                    MtkCatLog.d("[BIP]", "Not expected EUTranBearerDesc instance");
                }
            } else if (BipUtils.BEARER_TYPE_UTRAN == mBearerDesc.bearerType) {
                if (mBearerDesc instanceof UTranBearerDesc) {
                    UTranBearerDesc uTranBD = (UTranBearerDesc) mBearerDesc;
                    length = 0x12;
                    buf.write(length);
                    buf.write(uTranBD.bearerType);
                    buf.write(uTranBD.trafficClass);
                    buf.write(uTranBD.maxBitRateUL_High);
                    buf.write(uTranBD.maxBitRateUL_Low);
                    buf.write(uTranBD.maxBitRateDL_High);
                    buf.write(uTranBD.maxBitRateDL_Low);
                    buf.write(uTranBD.guarBitRateUL_High);
                    buf.write(uTranBD.guarBitRateUL_Low);
                    buf.write(uTranBD.guarBitRateDL_High);
                    buf.write(uTranBD.guarBitRateDL_Low);
                    buf.write(uTranBD.deliveryOrder);
                    buf.write(uTranBD.maxSduSize);
                    buf.write(uTranBD.sduErrorRatio);
                    buf.write(uTranBD.residualBitErrorRadio);
                    buf.write(uTranBD.deliveryOfErroneousSdus);
                    buf.write(uTranBD.transferDelay);
                    buf.write(uTranBD.trafficHandlingPriority);
                    buf.write(uTranBD.pdpType);
                    MtkCatLog.d("[BIP]", "OpenChannelResponseDataEx-format: tag: " + tag
                            + ",length: " + length
                            + ",bearerType: " + uTranBD.bearerType
                            + ",trafficClass: " + uTranBD.trafficClass
                            + ",maxBitRateUL_High: " + uTranBD.maxBitRateUL_High
                            + ",maxBitRateUL_Low: " + uTranBD.maxBitRateUL_Low
                            + ",maxBitRateDL_High: " + uTranBD.maxBitRateDL_High
                            + ",maxBitRateDL_Low: " + uTranBD.maxBitRateDL_Low
                            + ",guarBitRateUL_High: " + uTranBD.guarBitRateUL_High
                            + ",guarBitRateUL_Low: " + uTranBD.guarBitRateUL_Low
                            + ",guarBitRateDL_High: " + uTranBD.guarBitRateDL_High
                            + ",guarBitRateDL_Low: " + uTranBD.guarBitRateDL_Low
                            + ",deliveryOrder: " + uTranBD.deliveryOrder
                            + ",maxSduSize: " + uTranBD.maxSduSize
                            + ",sduErrorRatio: " + uTranBD.sduErrorRatio
                            + ",residualBitErrorRadio: " + uTranBD.residualBitErrorRadio
                            + ",deliveryOfErroneousSdus: " + uTranBD.deliveryOfErroneousSdus
                            + ",transferDelay: " + uTranBD.transferDelay
                            + ",trafficHandlingPriority: " + uTranBD.trafficHandlingPriority
                            + ",pdp type: " + uTranBD.pdpType);
                } else {
                    MtkCatLog.d("[BIP]", "Not expected UTranBearerDesc instance");
                }
            } else if (BipUtils.BEARER_TYPE_DEFAULT == mBearerDesc.bearerType) {
                buf.write(1);
                buf.write(((DefaultBearerDesc) mBearerDesc).bearerType);
            }
        } else {
            MtkCatLog.d("[BIP]", "No bearer description in TR.");
        }
        if (mBufferSize >= 0) {
            MtkCatLog.d("[BIP]", "Write buffer size into TR.[" + mBufferSize + "]");
            tag = ComprehensionTlvTag.BUFFER_SIZE.value();
            buf.write(tag);
            length = 0x02;
            buf.write(length);
            buf.write(mBufferSize >> 8);
            buf.write(mBufferSize & 0xff);
            MtkCatLog.d("[BIP]", "OpenChannelResponseDataEx-format: tag: " + tag
                    + ",length: " + length
                    + ",buffer size(hi-byte): " + (mBufferSize >> 8)
                    + ",buffer size(low-byte): " + (mBufferSize & 0xff));
        } else {
            MtkCatLog.d("[BIP]", "No buffer size in TR.[" + mBufferSize + "]");
        }
        if (null != mDnsServerAddress) {
            for (InetAddress addr : mDnsServerAddress.dnsAddresses) {
                byte[] rawAddress = addr.getAddress();
                if (null != rawAddress) {
                    tag = ComprehensionTlvTag.DNS_SERVER_ADDRESS.value();
                    buf.write(tag);
                    buf.write(rawAddress.length + 1);
                    if (rawAddress.length == 4) {
                        buf.write(BipUtils.ADDRESS_TYPE_IPV4);
                    } else if (rawAddress.length == 16) {
                        buf.write(BipUtils.ADDRESS_TYPE_IPV6);
                    } else {
                        MtkCatLog.e("[BIP]", "length error: " + rawAddress.length);
                        buf.write(BipUtils.ADDRESS_TYPE_IPV4);
                    }
                    buf.write(rawAddress, 0, rawAddress.length);
                }
            }
        }
    }

}
class OpenChannelResponseData extends ResponseData {
    ChannelStatus mChannelStatus = null;
    BearerDesc mBearerDesc = null;
    int mBufferSize = 0;

    OpenChannelResponseData(ChannelStatus channelStatus, BearerDesc bearerDesc, int bufferSize) {
        super();
        if (channelStatus != null) {
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-constructor: channelStatus cid/status : "
                    + channelStatus.mChannelId + "/" + channelStatus.mChannelStatus);
        } else {
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-constructor: channelStatus is null");
        }
        if (bearerDesc != null) {
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-constructor: bearerDesc bearerType "
                    + bearerDesc.bearerType);
        } else {
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-constructor: bearerDesc is null");
        }

        MtkCatLog.d("[BIP]", "OpenChannelResponseData-constructor: buffer size is " + bufferSize);

        mChannelStatus = channelStatus;
        mBearerDesc = bearerDesc;
        mBufferSize = bufferSize;
    }

    @Override
    public void format(ByteArrayOutputStream buf) {
        if (buf == null) {
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: buf is null");
            return;
        }

        if (mBearerDesc == null) {
            MtkCatLog.e("[BIP]", "OpenChannelResponseData-format: mBearerDesc is null");
            return;
        }

        if (((GPRSBearerDesc) mBearerDesc).bearerType != BipUtils.BEARER_TYPE_GPRS) {
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: bearer type is not gprs");
            return;
        }

        int tag;

        if (/* mChannelStatus != null && */ mBufferSize > 0) {
            if (mChannelStatus != null) {
                MtkCatLog.d("[BIP]",
                        "OpenChannelResponseData-format: Write channel status into TR");
                tag = ComprehensionTlvTag.CHANNEL_STATUS.value();
                MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: tag: " + tag);
                buf.write(tag);
                MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: length: " + 0x02);
                buf.write(0x02);
                MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: channel id & isActivated: "
                        + (mChannelStatus.mChannelId | (mChannelStatus.isActivated ? 0x80 : 0x00)));
                buf.write(mChannelStatus.mChannelId | (mChannelStatus.isActivated ? 0x80 : 0x00));
                MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: channel status: "
                        + mChannelStatus.mChannelStatus);
                buf.write(mChannelStatus.mChannelStatus);
            }

            MtkCatLog.d("[BIP]", "Write bearer description into TR");
            tag = ComprehensionTlvTag.BEARER_DESCRIPTION.value();
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: tag: " + tag);
            buf.write(tag);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: length: " + 0x07);
            buf.write(0x07);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: bearer type: "
                    + ((GPRSBearerDesc) mBearerDesc).bearerType);
            buf.write(((GPRSBearerDesc) mBearerDesc).bearerType);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: precedence: "
                    + ((GPRSBearerDesc) mBearerDesc).precedence);
            buf.write(((GPRSBearerDesc) mBearerDesc).precedence);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: delay: " +
                     ((GPRSBearerDesc) mBearerDesc).delay);
            buf.write(((GPRSBearerDesc) mBearerDesc).delay);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: reliability: "
                    + ((GPRSBearerDesc) mBearerDesc).reliability);
            buf.write(((GPRSBearerDesc) mBearerDesc).reliability);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: peak: " +
                     ((GPRSBearerDesc) mBearerDesc).peak);
            buf.write(((GPRSBearerDesc) mBearerDesc).peak);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: mean: " +
                     ((GPRSBearerDesc) mBearerDesc).mean);
            buf.write(((GPRSBearerDesc) mBearerDesc).mean);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: pdp type: " +
                     ((GPRSBearerDesc) mBearerDesc).pdpType);
            buf.write(((GPRSBearerDesc) mBearerDesc).pdpType);

            MtkCatLog.d("[BIP]", "Write buffer size into TR");
            tag = ComprehensionTlvTag.BUFFER_SIZE.value();
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: tag: " + tag);
            buf.write(tag);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: length: " + 0x02);
            buf.write(0x02);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: length(hi-byte): "
                    + (mBufferSize >> 8));
            buf.write(mBufferSize >> 8);
            MtkCatLog.d("[BIP]", "OpenChannelResponseData-format: length(low-byte): "
                    + (mBufferSize & 0xff));
            buf.write(mBufferSize & 0xff);
        } else {
            MtkCatLog.d("[BIP]", "Miss ChannelStatus, BearerDesc or BufferSize");
        }
    }
}

class SendDataResponseData extends ResponseData {
    int mTxBufferSize = 0;

    SendDataResponseData(int size) {
        super();
        mTxBufferSize = size;
    }

    @Override
    public void format(ByteArrayOutputStream buf) {
        if (buf == null) {
            return;
        }

        int tag;

        tag = 0x80 | ComprehensionTlvTag.CHANNEL_DATA_LENGTH.value();
        buf.write(tag);
        buf.write(1);
        if (mTxBufferSize >= 0xFF) {
            buf.write(0xFF);
        } else {
            buf.write(mTxBufferSize);
        }
    }
}

class ReceiveDataResponseData extends ResponseData {
    byte[] mData = null;
    int mRemainingCount = 0;

    ReceiveDataResponseData(byte[] data, int remaining) {
        super();
        mData = data;
        mRemainingCount = remaining;
    }

    @Override
    public void format(ByteArrayOutputStream buf) {
        if (buf == null) {
            return;
        }

        int tag;

        tag = 0x80 | ComprehensionTlvTag.CHANNEL_DATA.value();
        buf.write(tag);

        if (mData != null) {
            if (mData.length >= 0x80) {
                buf.write(0x81);
            }

            buf.write(mData.length);
            buf.write(mData, 0, mData.length);
        } else {
            buf.write(0);
        }

        tag = 0x80 | ComprehensionTlvTag.CHANNEL_DATA_LENGTH.value();
        buf.write(tag);
        buf.write(0x01);

        MtkCatLog.d("[BIP]", "ReceiveDataResponseData: length: " + mRemainingCount);

        if (mRemainingCount >= 0xFF) {
            buf.write(0xFF);
        } else {
            buf.write(mRemainingCount);
        }
    }
}

class GetMultipleChannelStatusResponseData extends ResponseData {
    ArrayList mArrList = null;

    GetMultipleChannelStatusResponseData(ArrayList arrList) {
        mArrList = arrList;
    }

    @Override
    public void format(ByteArrayOutputStream buf) {
        if (buf == null) {
            return;
        }

        int tag = 0x80 | ComprehensionTlvTag.CHANNEL_STATUS.value();
        MtkCatLog.d("[BIP]", "ChannelStatusResp: size: " + mArrList.size());

        if (0 < mArrList.size()) {
            Iterator iterator = mArrList.iterator();
            ChannelStatus chStatus = null;
            while (iterator.hasNext()) {
                buf.write(tag);
                buf.write(0x02);
                chStatus = (ChannelStatus) iterator.next();
                buf.write((chStatus.mChannelId & 0x07) | (chStatus.mChannelStatus));
                buf.write(chStatus.mChannelStatusInfo);
                MtkCatLog.d("[BIP]", "ChannelStatusResp: cid:" + chStatus.mChannelId + ",status:" +
                         chStatus.mChannelStatus + ",info:" + chStatus.mChannelStatusInfo);
            }
        } else { //No channel available, link not established or PDP context not activated.
            MtkCatLog.d("[BIP]", "ChannelStatusResp: no channel status.");
            buf.write(tag);
            buf.write(0x02);
            buf.write(0x00);
            buf.write(0x00);
        }
    }
}

