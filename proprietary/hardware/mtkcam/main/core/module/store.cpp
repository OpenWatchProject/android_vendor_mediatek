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

#define LOG_TAG "mtkcam-module"
//
#include <string.h>
#include "local.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_MODULE_REGISTRY);

//
/******************************************************************************
 *
 ******************************************************************************/
namespace
{
    /**************************************************************************
     *
     **************************************************************************/
    enum
    {
        MODULE_GROUP_ID         = MTKCAM_MODULE_GROUP_ID,
        MODULE_GROUP_ID_START   = MTKCAM_MODULE_GROUP_ID_START,
        MODULE_GROUP_ID_END     = MTKCAM_MODULE_GROUP_ID_END,
        MODULE_GROUP_COUNT      = MTKCAM_MODULE_GROUP_ID_END - MTKCAM_MODULE_GROUP_ID_START,
    };


    /**************************************************************************
     *
     **************************************************************************/
    struct Store
    {
        /**********************************************************************
         *
         **********************************************************************/
        static
        mtkcam_module_info*
        get_module_info(unsigned int module_id)
        {
            struct ModuleStore
            {
                mtkcam_module_info  table[MODULE_GROUP_COUNT];
                ModuleStore()
                {
                    //::memset(&table, 0, sizeof(table));
                    for (auto i = 0; i < MODULE_GROUP_COUNT; i++) {
                        table[i].module_id = 0;
                        table[i].module_factory = nullptr;
                        table[i].register_name = nullptr;
                    }
                    MY_LOGI("ctor");
                }
            };
            static ModuleStore store;

            if  ( ! check_module_id(module_id) ) {
                return NULL;
            }

            return &store.table[MTKCAM_GET_MODULE_INDEX(module_id)];
        }


        /**********************************************************************
         *
         **********************************************************************/
        static
        void*
        get_module_factory(unsigned int module_id)
        {
            mtkcam_module_info const* info = get_module_info(module_id);
            if  ( ! info ) {
                return NULL;
            }

            if  ( ! info->module_factory ) {
                MY_LOGW("[module_id:0x%#x] Bad module_factory==NULL", module_id);
                dump_module(info);
                return NULL;
            }

            return info->module_factory;
        }


        /**********************************************************************
         *
         **********************************************************************/
        static
        void
        register_module(mtkcam_module_info const* info)
        {
            if  ( ! info ) {
                return;
            }
            //check module id
            if  ( ! check_module_id(info->module_id) ) {
                dump_module(info);
                return;
            }

            if  ( ! info->module_factory ) {
                MY_LOGW("Bad module_factory==NULL");
                dump_module(info);
                return;
            }
           if(!get_module_info(info->module_id))
           {
                return;
           }
            if  ( get_module_info(info->module_id)->module_factory ) {
                MY_LOGE("Has registered before");
                dump_module(get_module_info(info->module_id), "old");
                dump_module(info, "new");
                return;
            }

            dump_module(info, "registered");
            *get_module_info(info->module_id) = *info;
        }


        /**********************************************************************
         *
         **********************************************************************/
        static
        bool
        check_module_id(unsigned int module_id)
        {
            unsigned int const group_id = MTKCAM_GET_MODULE_GROUP_ID(module_id);
            unsigned int const module_index = MTKCAM_GET_MODULE_INDEX(module_id);

            //check module group id
            if  ( MODULE_GROUP_ID != group_id ) {
                MY_LOGE("Bad module_id(0x%#x) whose group id(%u) != %u ", module_id, group_id, MODULE_GROUP_ID);
                return false;
            }

            //check module index
            if  ( MODULE_GROUP_COUNT <= module_index ) {
                MY_LOGE("Bad module_id(0x%#x) whose module index(%u) >= module group count(%u) ", module_id, module_index, MODULE_GROUP_COUNT);
                return false;
            }

            return true;
        }


        /**********************************************************************
         *
         **********************************************************************/
        static
        void
        dump_module(
            mtkcam_module_info const* info,
            char const* prefix_msg = ""
        )
        {
            MY_LOGI(
                "[%s] module_id:0x%#x module_factory:%p register_name:%s",
                prefix_msg,
                info->module_id,
                info->module_factory,
                (info->register_name ? info->register_name : "unknown")
            );
        }

    };//struct


    /**************************************************************************
     *
     **************************************************************************/
    struct ShowLoading
    {
        ShowLoading()
        {
            ALOGI(
                "loading (MODULE_GROUP_ID:%u MODULE_GROUP_COUNT:%u ...",
                MODULE_GROUP_ID, MODULE_GROUP_COUNT
            );
        };
    };
    static const ShowLoading gShowLoading;


};//namespace


/******************************************************************************
 *
 ******************************************************************************/
void register_mtkcam_module(mtkcam_module_info const* info, NSCam::Int2Type<MTKCAM_MODULE_GROUP_ID>)
{
    Store::register_module(info);
}


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
void* MTKCAM_GET_MODULE_FACTORY(unsigned int moduleId)
{
    return Store::get_module_factory(moduleId);
}

