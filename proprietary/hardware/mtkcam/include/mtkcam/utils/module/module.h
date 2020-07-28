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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_MODULE_MODULE_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_MODULE_MODULE_H_
//
/******************************************************************************
 *
 ******************************************************************************/
#include <stdint.h>
#include <sys/cdefs.h>
#include <mtkcam/main/core/module.h>

__BEGIN_DECLS


/*****************************************************************************
 * @brief Get the mtkcam module factory.
 *
 * @details Given a mtkcam module ID, return its corresponding module factory.
 * The real type of module factory depends on the given module ID, and users
 * must cast the output to the real type of module factory.
 *
 * @note
 *
 * @param[in] module_id: mtkcam module ID.
 *
 * @return a module factory corresponding to the given module ID.
 *
 ******************************************************************************/
void*   getMtkcamModuleFactory(uint32_t module_id);


#define MAKE_MTKCAM_MODULE(_module_id_, _module_factory_type_, ...) \
    ({void* factory = getMtkcamModuleFactory(_module_id_); (factory ? (((_module_factory_type_)factory)(__VA_ARGS__)) : NULL);})



/*****************************************************************************
 * @brief Get the mtkcam module.
 *
 * @details Given a mtkcam module ID, return its corresponding mtkcam module.
 *
 * @note
 *
 * @param[in] module_id: mtkcam module ID.
 *
 * @param[out] module: a pointer to mtkcam module.
 *
 * @return: 0 == success, <0 == error and *module == NULL
 *
 ******************************************************************************/
int getMtkcamModule(uint32_t module_id, mtkcam_module** module);


#define GET_MTKCAM_MODULE_EXTENSION(_module_id_) \
    ({mtkcam_module* m = NULL; ((0 == getMtkcamModule(_module_id_, &m) && m) ? m->get_module_extension() : NULL);})



/******************************************************************************
 *
 ******************************************************************************/
__END_DECLS
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_MODULE_MODULE_H_

