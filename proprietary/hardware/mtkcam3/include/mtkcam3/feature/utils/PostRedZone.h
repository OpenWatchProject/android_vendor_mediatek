/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_CAMERA_FEATURE_UTIL_POST_RED_ZONE_H_
#define _MTK_CAMERA_FEATURE_UTIL_POST_RED_ZONE_H_

#include <cstddef>
#include <cstdint>
#include <sys/mman.h>

namespace NSCam {
namespace NSCamFeature {

class PostRedZone
{
private:
    // can query via "getconf PAGE_SIZE" or sysconf()
    static constexpr intptr_t __PAGE_SIZE = 4096;
public:
    static void* mynew(std::size_t count)
    {
        intptr_t __DATA_PAGES = (sizeof(std::size_t) + sizeof(char*) + count + __PAGE_SIZE - 1) / __PAGE_SIZE;
        char *spaceAddr = new char[__PAGE_SIZE * (__DATA_PAGES + 2)];
        intptr_t redZone = reinterpret_cast<intptr_t>(spaceAddr) + (__PAGE_SIZE * (__DATA_PAGES + 1));
        redZone = (redZone & ~(__PAGE_SIZE - 1));
        mprotect(reinterpret_cast<void*>(redZone), __PAGE_SIZE, PROT_READ);
        intptr_t objAddr = redZone - count;
        *(reinterpret_cast<char**>(objAddr - sizeof(char*))) = spaceAddr;
        *(reinterpret_cast<std::size_t*>(objAddr - sizeof(char*) - sizeof(std::size_t))) = count;
        //MY_LOGD("spaceAddr(%p) objAddr(%" PRIiPTR ") count(%zu)", spaceAddr, objAddr, count);
        return reinterpret_cast<void*>(objAddr);
    }

    static void mydelete(void *objAddrP) noexcept
    {
        intptr_t objAddr = reinterpret_cast<intptr_t>(objAddrP);
        std::size_t count = *(reinterpret_cast<std::size_t*>(objAddr - sizeof(char*) - sizeof(std::size_t)));
        intptr_t redZone = objAddr + count;
        mprotect(reinterpret_cast<void*>(redZone), __PAGE_SIZE, PROT_READ | PROT_WRITE);
        char *spaceAddr = *(reinterpret_cast<char**>(objAddr - sizeof(char*)));
        //MY_LOGD("spaceAddr(%p) objAddr(%" PRIiPTR ") count(%zu)", spaceAddr, objAddr, count);
        delete [] spaceAddr;
    }
};


} //NSCamFeature
} //NSCam
#endif
