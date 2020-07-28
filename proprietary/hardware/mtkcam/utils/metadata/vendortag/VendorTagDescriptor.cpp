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

#define LOG_TAG "mtkcam-vendortag"
//
#include <stdint.h>
#include <dlfcn.h>
//
#include <system/camera_vendor_tags.h>
//
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/metadata/IVendorTagDescriptor.h>
#include "VendorTagTable.h"
#include <mtkcam/utils/metadata/mtk_metadata_types.h>
//
using namespace NSCam;

/*******************************************************************************
 *
 ********************************************************************************/
namespace {
class SectionHelper
{
protected:
    const std::vector<VendorTagSection_t>&
            mvSection;
    size_t  mTagCount;

public:
            SectionHelper()
                : mvSection(getGlobalSections())
                , mTagCount(0)
            {
                mTagCount = 0;
                for (auto const& s : mvSection) {
                    mTagCount += s.tags.size();
                }
            }

    auto    getSections() const -> const std::vector<VendorTagSection_t>&
            {
                return mvSection;
            }

    auto    getTagCount() const -> size_t
            {
                return mTagCount;
            }

    auto    getSection(uint32_t tag) const -> VendorTagSection_t const*
            {
                for (auto const& s : mvSection) {
                    if  ( s.tagStart <= tag && s.tagEnd > tag ) {
                        META_LOGV("getSection: tag:0x%x in section:%s", tag, s.sectionName);
                        return &s;
                    }
                }
                META_LOGE("not a vender tag:0x%x", tag);
                return nullptr;
            }

    auto    getTag(uint32_t tag) const -> VendorTag_t const*
            {
                if  ( auto s = getSection(tag) ) {
                    auto t = s->tags.find(tag);
                    if  (t != s->tags.end()) {
                        META_LOGV("tag:0x%x %s", tag, t->second.tagName);
                        return &(t->second);
                    }
                    META_LOGE("tag:0x%x not in section:%s", tag, s->sectionName);
                }
                return nullptr;
            }

    auto    getSectionName(uint32_t tag) const -> const char*
            {
                if  ( auto s = getSection(tag) ) {
                    return s->sectionName;
                }
                return nullptr;
            }

    auto    getTagName(uint32_t tag) const -> const char*
            {
                if  ( auto t = getTag(tag) ) {
                    return t->tagName;
                }
                return nullptr;
            }

    auto    getTagType(uint32_t tag) const -> int
            {
                if  ( auto t = getTag(tag) ) {
                    return t->tagType;
                }
                return -1;
            }

};//class

static auto const& getSectionHelper()
{
    static SectionHelper inst;
    return inst;
}

}//namesapce


/*******************************************************************************
 *
 ********************************************************************************/
static auto static_get_tag_count(const vendor_tag_ops_t* /*ops*/) -> int
{
    return getSectionHelper().getTagCount();
}


static auto static_get_all_tags(const vendor_tag_ops_t* /*ops*/, uint32_t* tag_array) -> void
{
    if  ( ! tag_array ) {
        META_LOGE("bad tag_array");
        return;
    }
    META_LOGV("get_all_tags");
    for (auto const& s : getSectionHelper().getSections()) {
        for (auto const& t : s.tags) {
            *tag_array++ = t.second.tagId;
        }
    }
}


static auto static_get_section_name(const vendor_tag_ops_t* /*ops*/, uint32_t tag) -> const char*
{
    return getSectionHelper().getSectionName(tag);
}


static auto static_get_tag_name(const vendor_tag_ops_t* /*ops*/, uint32_t tag) -> const char*
{
    return getSectionHelper().getTagName(tag);
}


static auto static_get_tag_type(const vendor_tag_ops_t* /*ops*/, uint32_t tag) -> int
{
    return getSectionHelper().getTagType(tag);
}


static auto get_vendor_tag_ops()
{
    static vendor_tag_ops_t const sOps =
    {
        .get_tag_count = static_get_tag_count,
        .get_all_tags = static_get_all_tags,
        .get_section_name = static_get_section_name,
        .get_tag_name = static_get_tag_name,
        .get_tag_type = static_get_tag_type,
    };

    return &sOps;
}


/******************************************************************************
 *
 ******************************************************************************/
static int setAsGlobalVendorTagSections(vendor_tag_ops_t const* ops)
{
    META_LOGD("+");
    int status = 0;
    char const szModulePath[] = "libcamera_metadata.so";
    char const szEntrySymbol[] = "set_camera_metadata_vendor_ops";
    void* pfnEntry = NULL;
    void* pLib = ::dlopen(szModulePath, RTLD_NOW);
    if  ( ! pLib )
    {
        char const *err_str = ::dlerror();
        META_LOGE("dlopen: %s error=%s", szModulePath, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    pfnEntry = ::dlsym(pLib, szEntrySymbol);
    if  ( ! pfnEntry )
    {
        char const *err_str = ::dlerror();
        META_LOGE("dlsym: %s error=%s", szEntrySymbol, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    typedef int (*set_camera_metadata_vendor_ops)(const vendor_tag_ops_t *query_ops);
    status = reinterpret_cast<set_camera_metadata_vendor_ops>(pfnEntry)(ops);

lbExit:
    if  ( pLib )
    {
        ::dlclose(pLib);
        pLib = NULL;
    }

    META_LOGD("- %p:%s(%p) return res:%s(%d) in %s", pfnEntry, szEntrySymbol, ops, strerror(-status), status, szModulePath);
    META_LOGE_IF(0!=status, "static vendor metadata may fail");
    return status;
}


/******************************************************************************
 *
 ******************************************************************************/
class VendorTagDescriptorImpl : public IVendorTagDescriptor
{
public:
                    VendorTagDescriptorImpl()
                    {
                        setAsGlobalVendorTagSections(get_vendor_tag_ops());
                    }

    virtual auto    getSections() const -> const std::vector<VendorTagSection>&
                    {
                        return getSectionHelper().getSections();
                    }

    virtual auto    getTagCount() const -> size_t
                    {
                        return getSectionHelper().getTagCount();
                    }

    virtual auto    getSectionName(uint32_t tag) const -> const char*
                    {
                        return getSectionHelper().getSectionName(tag);
                    }

    virtual auto    getTagName(uint32_t tag) const -> const char*
                    {
                        return getSectionHelper().getTagName(tag);
                    }

    virtual auto    getTagType(uint32_t tag) const -> int
                    {
                        return getSectionHelper().getTagType(tag);
                    }

};  //class


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
IVendorTagDescriptor*
getVendorTagDescriptor()
{
    static VendorTagDescriptorImpl inst;
    return &inst;
}
}//NSCam

