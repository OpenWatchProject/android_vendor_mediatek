/* Copyright Statement:
*
* This oftware/firmware and related documentation ("MediaTek Software") are
* proteed under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MedTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESCT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALLALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THEEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE T ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/
#include <unwind.h>
#include <dynamic_object.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const uint32_t __do_ws_start;
extern const uint32_t __do_ws_end;
extern const uint32_t __exidx_start;  // start address of .ARM section in tinysys-scp
extern const uint32_t __exidx_end;  // end address of .ARM section in tinysys-scp

static _Unwind_Ptr dyn_obj_Unwind_Find_exidx(_Unwind_Ptr pc, int *pcount, doSectionsInfo_t *h) {
    uint32_t addr = (uint32_t) pc;
    if ((addr > (uint32_t)(&__do_ws_start)) && (addr < (uint32_t)(&__do_ws_end))) {
        // We're in DO
        *pcount = (int)((h->do_exidx_end - h->do_exidx_start) / 8);
        return (_Unwind_Ptr)h->do_exidx_start;
    } else {
        // We're in tinysys-scp
        uint32_t s = (uint32_t)(&__exidx_start);
        uint32_t e = (uint32_t)(&__exidx_end);
        *pcount = (int)((e - s) / 8);
        return (_Unwind_Ptr)s;
    }
}

_Unwind_Ptr __gnu_Unwind_Find_exidx(_Unwind_Ptr pc, int *pcount) {
    return dyn_obj_Unwind_Find_exidx(pc, pcount, (doSectionsInfo_t *)(&__do_ws_start));
}

#ifdef __cplusplus
}
#endif
