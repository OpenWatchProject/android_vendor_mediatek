/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef __RTST_UTILS_H__
#define __RTST_UTILS_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <Parcel.h>
#include "RfxStatusDefs.h"
#include "RfxVariant.h"
#include "Rfx.h"
#include "SortedVector.h"
#include "RtstSocket.h"

/*****************************************************************************
 * Name Space
 *****************************************************************************/
using ::android::Parcel;
using ::android::SortedVector;

/*****************************************************************************
 * Class RtstUtils
 *****************************************************************************/
/*
 * Utils class
 */
class RtstUtils {
// External Method
public:
    // Write C-style string to Parcel object
    //
    // RETURNS: void
    static void writeStringToParcel(
        Parcel &p,     // [IN/OUT] Parcel Object
        const char *s  // [IN] The C-style string
    );

    // Get the string from parcel object
    // The caller need to free the string after using this function
    //
    // RETURNS: the string
    static char* getStringFromParcel(
        Parcel &p     // [IN] Parcel object
    );

    // Check a fd if readable with a timeout
    //
    // RETURNS: a position value when readable or -1/0
    static int pollWait(
        int fd,       // [IN] the socket fd
        int timeout   // [IN] the timeout value
    );


    // Dump the buffer in hex format to log
    //
    // RETURNS: void
    static void dumpToHexFormatLog(
        const char* prefix,        // The prefix of the dump
        const char* data,          // The buffer need to be dump
        int len                    // the length of the buffer
    );

    // Convert the HEX string to binary
    //
    // RETURNS: void
    static void hex2Bin(
        const char *src, // [IN] the hex string
        char *target,    // [OUT] the binary array
        int len          // [IN] the length of the binary array
    );

    static void cleanSocketBuffer(const RtstFd &s, int timeout);

// Implementation
private:
    static char char2Int(char c);
};

#endif /* __RTST_UTILS_H__ */