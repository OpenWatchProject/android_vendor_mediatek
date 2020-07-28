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

package com.mediatek.op112.ims;

import android.content.Context;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.Rlog;
import com.mediatek.ims.MtkImsReasonInfo;
import com.mediatek.ims.SipMessage;
import com.mediatek.ims.ext.OpImsCallSessionProxyBase;

/**
 * Plugin implementation for ImsCallSessionProxy.
 */
public class OpImsCallSessionProxyImpl extends OpImsCallSessionProxyBase {
    private static final String TAG = "Op112ImsCallSessionProxyImpl";
    Context mContext;

    /** Constructor.
     * @param context context
     */
     public OpImsCallSessionProxyImpl(Context context) {
        mContext = context;
        Rlog.d(TAG, "OpImsCallSessionProxyImpl 112");
    }

    @Override
    public ImsReasonInfo getImsReasonInfo(SipMessage sipMsg) {
        ImsReasonInfo info = null;
        Rlog.d(TAG, "getImsReasonInfo sipCode " + sipMsg.getCode() + " reasonHeader "
            + sipMsg.getReasonHeader());
        int sipErrCode = sipMsg.getCode();
        String reasonHeader = sipMsg.getReasonHeader();
        switch (sipErrCode) {
            case 301:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_301_MOVED_PERMANENTLY, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_301_MOVED_PERMANENTLY, 0);
                }
                /// @}
                break;
            case 400:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_400_BAD_REQUEST, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_400_BAD_REQUEST, 0);
                }
                /// @}
                break;
            case 401:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_401_UNAUTHORIZED, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_401_UNAUTHORIZED, 0);
                }
                /// @}
                break;
            case 402:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_402_PAYMENT_REQUIRED, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_402_PAYMENT_REQUIRED, 0);
                }
                /// @}
                break;
            case 403:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_403_FORBIDDEN, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_403_FORBIDDEN, 0);
                }
                /// @}
                break;
            case 404:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_404_NOT_FOUND, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_404_NOT_FOUND, 0);
                }
                /// @}
                break;
            case 405:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_405_METHOD_NOT_ALLOWED, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_405_METHOD_NOT_ALLOWED, 0);
                }
                /// @}
                break;
            case 406:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_406_NOT_ACCEPTABLE, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_406_NOT_ACCEPTABLE, 0);
                }
                /// @}
                break;
            case 407:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_407_PROXY_AUTHENTICATION_REQUIRED,
                            0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_407_PROXY_AUTHENTICATION_REQUIRED,
                            0);
                }
                /// @}
                break;
            case 408:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_408_REQUEST_TIMEOUT, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_408_REQUEST_TIMEOUT, 0);
                }
                /// @}
                break;
            case 409:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_409_CONFLICT, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_409_CONFLICT, 0);
                }
                /// @}
                break;
            case 410:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_410_GONE, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_410_GONE, 0);
                }
                /// @}
                break;
            case 411:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_411_LENGTH_REQUIRED, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_411_LENGTH_REQUIRED, 0);
                }
                /// @}
                break;
            case 413:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_413_REQUEST_ENTRY_TOO_LONG,
                            0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_413_REQUEST_ENTRY_TOO_LONG,
                            0);
                }
                /// @}
                break;
            case 414:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_414_REQUEST_URI_TOO_LONG, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_414_REQUEST_URI_TOO_LONG,
                            0);
                }
                /// @}
                break;
            case 415:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_415_UNSUPPORTED_MEDIA_TYPE, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_415_UNSUPPORTED_MEDIA_TYPE,
                            0);
                }
                /// @}
                break;
            case 416:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_416_UNSUPPORTED_URI_SCHEME, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_416_UNSUPPORTED_URI_SCHEME,
                            0);
                }
                /// @}
                break;
            case 420:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_420_BAD_EXTENSION, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_420_BAD_EXTENSION, 0);
                }
                /// @}
                break;
            case 421:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_421_BAD_EXTENSION_REQUIRED, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_421_BAD_EXTENSION_REQUIRED,
                            0);
                }
                /// @}
                break;
            case 423:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_423_INTERVAL_TOO_BRIEF, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_423_INTERVAL_TOO_BRIEF, 0);
                }
                /// @}
                break;
            case 480:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_480_TEMPORARILY_UNAVAILABLE,
                            0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_480_TEMPORARILY_UNAVAILABLE,
                            0);
                }
                /// @}
                break;
            case 481:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_481_CALL_TRANSACTION_NOT_EXIST,
                            0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_481_CALL_TRANSACTION_NOT_EXIST, 0);
                }
                /// @}
                break;
            case 482:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_482_LOOP_DETECTED, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_482_LOOP_DETECTED, 0);
                }
                /// @}
                break;
            case 483:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_483_TOO_MANY_HOPS, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_483_TOO_MANY_HOPS, 0);
                }
                /// @}
                break;
            case 484:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_484_TOO_ADDRESS_INCOMPLETE,
                            0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_484_TOO_ADDRESS_INCOMPLETE,
                            0);
                }
                /// @}
                break;
            case 485:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_485_AMBIGUOUS, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_485_AMBIGUOUS, 0);
                }
                /// @}
                break;
            case 486:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_486_BUSY_HERE, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_486_BUSY_HERE, 0);
                }
                /// @}
                break;
            case 487:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_487_REQUEST_TERMINATED, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_487_REQUEST_TERMINATED, 0);
                }
                /// @}
                break;
            case 488:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_488_NOT_ACCEPTABLE_HERE, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_488_NOT_ACCEPTABLE_HERE, 0);
                }
                /// @}
                break;
            case 500:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_500_SERVER_INTERNAL_ERROR, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_500_SERVER_INTERNAL_ERROR,
                            0);
                }
                /// @}
                break;
            case 501:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_501_NOT_IMPLEMENTED, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_501_NOT_IMPLEMENTED, 0);
                }
                /// @}
                break;
            case 502:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_502_BAD_GATEWAY, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_502_BAD_GATEWAY, 0);
                }
                /// @}
                break;
            case 503:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_503_SERVICE_UNAVAILABLE, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_503_SERVICE_UNAVAILABLE, 0);
                }
                /// @}
                break;
            case 504:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_504_GATEWAY_TIMEOUT, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_504_GATEWAY_TIMEOUT, 0);
                }
                /// @}
                break;
            case 505:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_505_VERSION_NOT_SUPPORTED, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_505_VERSION_NOT_SUPPORTED,
                            0);
                }
                /// @}
                break;
            case 513:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_513_MESSAGE_TOO_LONG, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_513_MESSAGE_TOO_LONG, 0);
                }
                /// @}
                break;
            case 600:
                /// M: For Telcel requirement. @{
                if (reasonHeader != null && reasonHeader.length() != 0) {
                    info = new ImsReasonInfo(
                            MtkImsReasonInfo.CODE_SIP_600_BUSY_EVERYWHERE, 0, reasonHeader);
                } else {
                    info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_600_BUSY_EVERYWHERE, 0);
                }
                    /// @}
                    break;
                case 603:
                    /// M: For Telcel requirement. @{
                    if (reasonHeader != null && reasonHeader.length() != 0) {
                        info = new ImsReasonInfo(
                                MtkImsReasonInfo.CODE_SIP_603_DECLINE, 0, reasonHeader);
                    } else {
                        info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_603_DECLINE, 0);
                    }
                    /// @}
                    break;
                case 604:
                    /// M: For Telcel requirement. @{
                    if (reasonHeader != null && reasonHeader.length() != 0) {
                        info = new ImsReasonInfo(
                                MtkImsReasonInfo.CODE_SIP_604_DOES_NOT_EXIST_ANYWHERE,
                                0, reasonHeader);
                    } else {
                        info = new ImsReasonInfo(
                                MtkImsReasonInfo.CODE_SIP_604_DOES_NOT_EXIST_ANYWHERE, 0);
                    }
                    /// @}
                    break;
                case 606:
                    /// M: For Telcel requirement. @{
                    if (reasonHeader != null && reasonHeader.length() != 0) {
                        info = new ImsReasonInfo(
                                MtkImsReasonInfo.CODE_SIP_606_NOT_ACCEPTABLE, 0, reasonHeader);
                    } else {
                        info = new ImsReasonInfo(MtkImsReasonInfo.CODE_SIP_606_NOT_ACCEPTABLE, 0);
                    }
                    /// @}
                    break;
                default:
                    break;
        }
        Rlog.d(TAG, "getImsReasonInfo(): " + info);
        return info;
    }
}