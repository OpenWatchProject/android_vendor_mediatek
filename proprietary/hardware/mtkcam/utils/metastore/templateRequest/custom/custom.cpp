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

#define LOG_TAG "MtkCam/TemplateRequest.Custom"
//
#include <system/camera_metadata.h>
#include <hardware/camera3.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <custom_metadata/custom_metadata_tag.h>

#include "Info.h"
#include "TagTypes.h"
//
using namespace android;
using namespace NSCam;
using namespace NSTemplateRequest;

/******************************************************************************
 *
 ******************************************************************************/
#define CONFIG_METADATA_BEGIN(_tag_) \
    { \
        IMetadata::IEntry myENTRY(_tag_); \
        IMetadata& myCAPABILITY = rMetadata;\


#define CONFIG_METADATA_END() \
        { \
            MERROR err = myCAPABILITY.update(myENTRY.tag(), myENTRY); \
            if  ( OK != err ) { \
                META_LOGE("IMetadata::update(), tag:%d err:%d", myENTRY.tag(), err); \
                return  MFALSE; \
            } \
        } \
        }

#define CONFIG_METADATA2_BEGIN(_tag_) \
            { \
                IMetadata::IEntry myENTRY(_tag_);\


#define CONFIG_METADATA2_END() \
                myCAPABILITY2.update(myENTRY.tag(), myENTRY); \
            }

#define CONFIG_ENTRY_METADATA(_macros_...) \
    { \
            IMetadata myCAPABILITY2; \
            _macros_ \
            myENTRY.push_back(myCAPABILITY2, Type2Type<IMetadata>()); \
    }


#define CONFIG_ENTRY_VALUE(_value_, _type_) \
        myENTRY.push_back(_value_, Type2Type<_type_>());


// append metadata for tag
#define APPEND_METADATA_BEGIN(_tag_) \
    { \
        IMetadata::IEntry myENTRY = rMetadata.entryFor(_tag_); \
        IMetadata& myCAPABILITY = rMetadata; \
        if ( myENTRY.isEmpty() ) { \
            META_FATAL("No entry for tag: 0x%x before, use CONFIG_METADATA_BEGIN", _tag_); \
        }

#define APPEND_METADATA_END() \
        { \
            MERROR err = myCAPABILITY.update(myENTRY.tag(), myENTRY); \
            if  ( OK != err ) { \
                META_LOGE("IMetadata::update(), tag:%d err:%d", myENTRY.tag(), err); \
                return  MFALSE; \
            } \
        } \
        }

#define APPEND_ENTRY_VALUE(_value_, _type_) \
        myENTRY.push_back(_value_, Type2Type<_type_>());


#define REMOVE_METADATA(_tag_) \
        META_LOGW_IF(0, "remove tag: 0x%x (count: %u)", _tag_, rMetadata.entryFor(_tag_).count()); \
        rMetadata.remove(_tag_);

/******************************************************************************
 *
 ******************************************************************************/
#include "custgen.config_request.h"  //[TBD] does request package binds with sensor?

/******************************************************************************
 *
 ******************************************************************************/
namespace android {
namespace NSRequestTemplate {
void
showCustInfo()
{
#if defined(MY_CUST_VERSION)
    META_LOGD("MY_CUST_VERSION=\"%s\"", MY_CUST_VERSION);
#endif
#if defined(MY_CUST_FTABLE_FILE_LIST)
    META_LOGD("MY_CUST_FTABLE_FILE_LIST=\"%s\"", MY_CUST_FTABLE_FILE_LIST);
#endif
#if defined(MY_CUST_FTABLE_FINAL_FILE)
    META_LOGD("MY_CUST_FTABLE_FINAL_FILE=\"%s\"", MY_CUST_FTABLE_FINAL_FILE);
#endif
}
};  //  namespace NSTemplateRequest
};  //  namespace android


