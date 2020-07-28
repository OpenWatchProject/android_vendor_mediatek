/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef __RP_DATA_CONNECTION_INFO_H__
#define __RP_DATA_CONNECTION_INFO_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <utils/String16.h>

#include "RfxStatusDefs.h"
#include "RfxController.h"

using ::android::String16;

/*****************************************************************************
 * Class RpDataConnectionInfo
 *****************************************************************************/

class RpDataConnectionInfo {
public:
    RpDataConnectionInfo();
    virtual ~RpDataConnectionInfo();
    void dump();
public:
    int             interfaceId; /* Context ID, uniquely identifies this call */
    String16        apn;         /* apn name */
    String16        type;        /* One of the PDP_type values in TS 27.007 section 10.1.1.
                                    For example, "IP", "IPV6", "IPV4V6", or "PPP". If status is
                                    PDP_FAIL_ONLY_SINGLE_BEARER_ALLOWED this is the type supported
                                    such as "IP" or "IPV6" */
    String16        ifname;      /* The network interface name */
    String16        addresses;   /* A space-delimited list of addresses with optional "/" prefix length,
                                    e.g., "192.0.1.3" or "192.0.1.11/16 2001:db8::1/64".
                                    May not be empty, typically 1 IPv4 or 1 IPv6 or
                                    one of each. If the prefix length is absent the addresses
                                    are assumed to be point to point with IPv4 having a prefix
                                    length of 32 and IPv6 128. */
};

#endif /* __RP_DATA_CONNECTION_INFO_H__ */

