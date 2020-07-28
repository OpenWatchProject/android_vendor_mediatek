/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef __MFLLUTILITES_H__
#define __MFLLUTILITES_H__

#include <cstdint>
#include <string>
#include <sys/resource.h>

namespace mfll {

#ifdef  MFLL_MAKE_REVISION
#   error   "MFLL_MAKE_REVISION should be defined before this line"
#else
//          Makes revision 32 bits value.
#   define  MFLL_MAKE_REVISION(M, N, F) ((M << 20) | (N << 10) | (F))
#   define  MFLL_MAJOR_VER(VER)   (VER >> 20)
#   define  MFLL_MINOR_VER(VER)   ((VER & 0xFFC00) >> 10)
#   define  MFLL_FEATURE_VER(VER) ((VER & 0x3FF))
#endif


// Make revision to a string
template<class T = int> inline
std::string makeRevisionString(T major, T minor, T featured)
{
    return std::string()
        .append(std::to_string(major))
        .append(".")
        .append(std::to_string(minor))
        .append(".")
        .append(std::to_string(featured));
}

inline int setThreadPriority(int prior, int& originprior)
{
    int _ori = ::getpriority(PRIO_PROCESS, 0);
    int _result = ::setpriority(PRIO_PROCESS, 0, prior);
    if (__builtin_expect( _result == 0, true )) {
        originprior = _ori;
    }
    return _result;
}

// Priciple to make the MF_TAG revision information
//  @param major        Main version information.
//  @param minor        Sub version information.
//  @return             A information in a 32 bits data
inline uint32_t makeDebugExifTagVersion(uint32_t major, uint32_t minor)
{
    // Bit[00:15]: Major version
    // Bit[16:31]: Minor version
    return (major & 0xFFFF) | ((minor & 0xFFFF) << 16);
}

}; // namespace mfll
#endif//__MFLLUTILITES_H__
