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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define LOG_TAG "PostAlgo/ppl_plugin"

#include <bitset>
#include <typeinfo>
#include <algorithm>
#include <sstream>

#include <cutils/properties.h>
#include <utils/std/Log.h>
#include <utils/std/Format.h>

#include <plugin/PipelinePlugin.h>
#include <plugin/PipelinePluginType.h>

using namespace std;
using namespace NSCam::Utils::Format;


#if 0
using namespace NSCam::Utils;
#endif


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSPipelinePlugin {

namespace {

/******************************************************************************
 * Debug Utilities
 ******************************************************************************/

#define PROPERTY_TYPE(TYPE, PROPNAME, DEF_VALUE)         \
class TYPE final                                         \
{                                                        \
public:                                                  \
    TYPE() = delete;                                     \
public:                                                  \
    static constexpr const char* NAME     = PROPNAME;   \
    static constexpr MINT32  const DEFAULT = DEF_VALUE;  \
};

template<typename TProp>
class PropertyGetter
{
public:
    inline static MINT32 getValue()
    {
        static const MINT32 ret = []() {
            MINT32 val = property_get_int32(TProp::NAME, TProp::DEFAULT);
            MY_LOGI("prop:%s, value:%d(%d), ", TProp::NAME, val, TProp::DEFAULT);
            return val;
        }();
        return ret;
    }
};

// We can set the macro variable DEF_VALUE for desired default value
PROPERTY_TYPE(MULTI_INSTANCE, "vendor.debug.camera.plugin.enable.multiinst", 1);
PROPERTY_TYPE(TOLEN_SELECTION, "vendor.debug.camera.plugin.enable.tokenSel", 1);
PROPERTY_TYPE(DEBUD_LEVEL, "vendor.debug.camera.plugin.debug.Level", 1);

inline auto getIsEnableMultiInstance() -> MBOOL
{
    return (PropertyGetter<MULTI_INSTANCE>::getValue() > 0);
}

inline auto getIsEnableTokenSel() -> MINT32
{
    return (PropertyGetter<TOLEN_SELECTION>::getValue() > 0);
}

inline auto getDebugLevel() -> MINT32
{
    return PropertyGetter<DEBUD_LEVEL>::getValue();
}

/******************************************************************************
 * Common Utilities
 ******************************************************************************/

inline auto getHash(MINT32 uniqueKey, MINT32 openId, MINT32 openId2) -> MUINT64
{
    union Hash
    {
        const MUINT64 mValue = 0;
        struct
        {
            MINT32 mOpenId      : 16;
            MINT32 mOpenId2     : 16;
            MINT32 mUniqueKey   : 32;
        };

        Hash(MINT32 openId, MINT32 openId2, MINT32 uniqueKey)
        {
            mOpenId = openId;
            mOpenId2 = openId2;
            mUniqueKey = uniqueKey;
        }
    };
    return Hash(openId, openId2, uniqueKey).mValue;
}

template<typename T>
const std::string to_string(const T& obj)
{
    return (obj == nullptr) ? "Nan" : std::move(obj->to_string());
}

template<typename T>
auto getName() -> const std::string&
{
    // need to enable complier RIIT flag for using operator typeid
    // static const std::string ret = typeid(T).name();
    // return ret;
    static const std::string ret = "unknown";
    return ret;
    #if 0
    return "unknown";
    #endif
}

#define SPECIFIED_TEMPLATE_FUN_GETNAME(PLUGINNAME)      \
template<>                                              \
auto getName<PLUGINNAME>() -> const std::string&        \
{                                                       \
    static const std::string ret = #PLUGINNAME;         \
    return ret;                                         \
}

SPECIFIED_TEMPLATE_FUN_GETNAME(Raw);
SPECIFIED_TEMPLATE_FUN_GETNAME(MultiFrame);
SPECIFIED_TEMPLATE_FUN_GETNAME(Fusion);
SPECIFIED_TEMPLATE_FUN_GETNAME(Yuv);
SPECIFIED_TEMPLATE_FUN_GETNAME(Depth);
SPECIFIED_TEMPLATE_FUN_GETNAME(Bokeh);
SPECIFIED_TEMPLATE_FUN_GETNAME(Join);

};  //namespace anonymous

/******************************************************************************
 * PluginRegistry Implementation
 ******************************************************************************/

template<typename T>
typename PluginRegistry<T>::ProviderRegistry
PluginRegistry<T>::sProviderRegistry;

template<typename T>
typename PluginRegistry<T>::InterfaceRegistry
PluginRegistry<T>::sInterfaceRegistry;

template<typename T>
void PluginRegistry<T>::addProvider(ConstructProvider fnConstructor)
{
    ProviderRegistry& reg = ofProvider();
    reg.push_back(fnConstructor);
}

template<typename T>
void PluginRegistry<T>::addProvider(ConstructProvider fnConstructor, PLUGIN_ID_TYPE id)
{
    ProviderRegistry& reg = ofProvider();
    reg.push_back(ConstructProviderEntry(fnConstructor, id));
}

template<typename T>
void PluginRegistry<T>::addInterface(ConstructInterface fnConstructor)
{
    InterfaceRegistry& reg = ofInterface();
    reg.push_back(fnConstructor);
}

template class PluginRegistry<Raw>;
template class PluginRegistry<MultiFrame>;
template class PluginRegistry<Fusion>;
template class PluginRegistry<Yuv>;
template class PluginRegistry<Depth>;
template class PluginRegistry<Bokeh>;
template class PluginRegistry<Join>;

/******************************************************************************
* PipelinePlugin Implementation
******************************************************************************/

template<typename T>
struct PipelinePlugin<T>::Selection::Token final
{
    const MINT32 mUniqueKey;
    const MINT32 mReqNum;
    const MINT32 mIndexNum;

    Token(MINT32 iUniqueKey, MINT32 iReqNum, MINT32 iIndexNum)
    : mUniqueKey(iUniqueKey)
    , mReqNum(iReqNum)
    , mIndexNum(iIndexNum)
    {

    }

    MBOOL operator==(const Token& other) const
    {
        return (mUniqueKey == other.mUniqueKey)
            && (mReqNum == other.mReqNum)
            && (mIndexNum == other.mIndexNum);
    }

    std::string to_string() const
    {
        std::ostringstream ss;
        ss << "["
           << mUniqueKey << ", "
           << mReqNum << ", "
           << mIndexNum << "]";
        //check
        return ss.str();
        #if 0
        return std::move(ss.str());
        #endif
    }
};

template<typename T>
typename PipelinePlugin<T>::Selection::TokenPtr
PipelinePlugin<T>::Selection::createToken(MINT32 iUniqueKey, MINT32 iReqNum, MINT32 iIndexNum)
{
    return make_shared<Token>(iUniqueKey, iReqNum, iIndexNum);
}

template<typename T>
std::map<MUINT64, typename PipelinePlugin<T>::WeakPtr>
PipelinePlugin<T>::mInstances;

template<typename T>
const std::vector<typename PipelinePlugin<T>::IProvider::Ptr>&
PipelinePlugin<T>::getProviders()
{
    typedef PluginRegistry<T> Registry;

    if (mpProviders.size() == 0) {
        for (auto r : Registry::ofProvider()) {
            typename IProvider::Ptr provider = r.mConstruct();
            provider->set(mOpenId, mOpenId2);
            mpProviders.push_back(provider);
        }
    }

    return mpProviders;
};

template<typename T>
const std::vector<typename PipelinePlugin<T>::IProvider::Ptr>&
PipelinePlugin<T>::getProviders(PLUGIN_ID_TYPE mask)
{
    typedef PluginRegistry<T> Registry;

    if (mpProviders.size() == 0) {
        for (auto r : Registry::ofProvider()) {
            if( !r.mID || (r.mID & mask) )
            {
                typename IProvider::Ptr provider = r.mConstruct();
                provider->set(mOpenId, mOpenId2);
                mpProviders.push_back(provider);
            }
        }
    }

    return mpProviders;
};

template<typename T>
typename PipelinePlugin<T>::Request::Ptr
PipelinePlugin<T>::createRequest()
{
    return std::make_shared<PipelinePlugin<T>::Request>();
}

template<typename T>
typename PipelinePlugin<T>::IInterface::Ptr
PipelinePlugin<T>::getInterface()
{
    typedef PluginRegistry<T> Registry;

    if (mpInterface == nullptr) {
        for (auto c : Registry::ofInterface()) {
            mpInterface = c();
            break;
        }
    }

    return mpInterface;
}

template<typename T>
const typename PipelinePlugin<T>::Selection&
PipelinePlugin<T>::getSelection(typename IProvider::Ptr provider)
{
    static Selection sel;
    auto intf = getInterface();
    if (intf != nullptr) {
        Selection sel;
        intf->offer(sel);
        provider->negotiate(sel);
    }

    return sel;
}

template<typename T>
typename PipelinePlugin<T>::Selection::Ptr
PipelinePlugin<T>::createSelection()
{
    return std::make_shared<PipelinePlugin<T>::Selection>();
}

template<typename T>
MVOID
PipelinePlugin<T>::pushSelection(typename IProvider::Ptr provider, typename Selection::Ptr sel)
{
    std::lock_guard<std::mutex> l(mMutex);
    mpSelections[provider].push_back(sel);
    MY_LOGD_IF(getDebugLevel() > 0, "%s: push selection, provAddr:%p, selAddr:%p, token:%s",
        getName<T>().c_str(), provider.get(),
        sel.get(), ((sel == nullptr) ? "Nan" : to_string(sel->mTokenPtr).c_str()));
}

template<typename T>
size_t
PipelinePlugin<T>::numOfSelection(typename IProvider::Ptr provider)
{
    std::lock_guard<std::mutex> l(mMutex);
    return mpSelections[provider].size();
}

template<typename T>
typename PipelinePlugin<T>::Selection::Ptr
PipelinePlugin<T>::popSelection(typename IProvider::Ptr provider, typename Selection::TokenPtr token)
{
    std::lock_guard<std::mutex> l(mMutex);
    auto& selections = mpSelections[provider];
    const MBOOL isUseToken = getIsEnableTokenSel() && (token != nullptr);
    if (selections.empty()) {
        MY_LOGD_IF(getDebugLevel() > 0, "%s: empty selections, provAddr:%p, token(%d):%s",
            getName<T>().c_str(), provider.get(), isUseToken, to_string(token).c_str());
        return nullptr;
    }

    typename Selection::Ptr sel = nullptr;
    if (isUseToken) {
        auto foundItem = std::find_if(selections.begin(), selections.end(),
            [&token] (typename Selection::Ptr& item) {
                return (item.get() != nullptr) && (item->mTokenPtr != nullptr) && (*(item->mTokenPtr) == *token);
            });

        if (foundItem != selections.end()) {
            sel = *foundItem;
            selections.erase(foundItem);
        } else {
            MY_LOGW("%s: cannot pop selection by specified token, token(%d):%s",
                getName<T>().c_str(), isUseToken, to_string(token).c_str());
        }
    } else {
        sel = selections.front();
        selections.pop_front();
    }

    MY_LOGD_IF(getDebugLevel() > 0, "%s: pop selection, provAddr:%p, selAddr:%p, token(%d):%s",
        getName<T>().c_str(), provider.get(),
        sel.get(), isUseToken, ((sel == nullptr) ? "Nan" : to_string(sel->mTokenPtr).c_str()));

    return sel;
}

template<typename T>
typename PipelinePlugin<T>::Selection::Ptr
PipelinePlugin<T>::frontSelection(typename IProvider::Ptr provider, typename Selection::TokenPtr token)
{
    std::lock_guard<std::mutex> l(mMutex);
    auto& selections = mpSelections[provider];
    const MBOOL isUseToken = getIsEnableTokenSel() && (token != nullptr);
    if (selections.empty()) {
        MY_LOGD_IF(getDebugLevel() > 0, "%s: empty selections, provAddr:%p, token(%d):%s",
            getName<T>().c_str(), provider.get(), isUseToken, to_string(token).c_str());
        return nullptr;
    }

    typename Selection::Ptr sel = nullptr;
    if (isUseToken) {
        auto foundItem = std::find_if(selections.begin(), selections.end(),
            [&token] (typename Selection::Ptr& item) {
                return (item.get() != nullptr) && (item->mTokenPtr != nullptr) && (*(item->mTokenPtr) == *token);
            });

        if (foundItem != selections.end()) {
            sel = *foundItem;
        } else {
            MY_LOGW("%s: cannot pop selection by specified token, token(%d):%s",
                getName<T>().c_str(), isUseToken, to_string(token).c_str());
        }
    } else {
        sel = selections.front();
    }

    MY_LOGD_IF(getDebugLevel() > 0, "%s: front selection, provAddr:%p, selAddr:%p, token(%d):%s",
        getName<T>().c_str(), provider.get(),
        sel.get(), isUseToken, ((sel == nullptr) ? "Nan" : to_string(sel->mTokenPtr).c_str()));

    return sel;
}

template<typename T>
MVOID
PipelinePlugin<T>::dump(std::ostream& os)
{
    FieldPrinter printer = FieldPrinter(os);

    Selection sel;
    auto intf = getInterface();

    if (intf == nullptr) {
        os << "  [No Interface Registered]" << std::endl;
        return;
    }

    intf->offer(sel);
    os << "  [Interface Capability]" << std::endl;
    Reflector::ForEach(sel, printer);

    int n = 0;
    for (auto provider : getProviders()) {
        os << "  [Provider " << ++n << " Property]" << std::endl;
        Reflector::ForEach(provider->property(), printer);
    }
}

template<typename T>
typename PipelinePlugin<T>::Ptr
PipelinePlugin<T>::getInstance(MINT32 iOpenId, MINT32 iOpenId2)
{
    return getInstance(-1, {iOpenId, iOpenId2});
}

template<typename T>
typename PipelinePlugin<T>::Ptr
PipelinePlugin<T>::getInstance(MINT32 iUniqueKey, const std::vector<MINT32>& vOpenIds)
{
    if(vOpenIds.size() < 1 || vOpenIds.size() > 2)
    {
        MY_LOGE("%s: the openIds count is not supported, size:%zu", getName<T>().c_str(), vOpenIds.size());
        return nullptr;
    }

    const MINT32 iOpenId = vOpenIds[0];
    const MINT32 iOpenId2 = (vOpenIds.size() > 1) ? vOpenIds[1] : -1;

    MUINT64 hash = 0;
    const MBOOL isEnableMultiInstance = getIsEnableMultiInstance();
    if (isEnableMultiInstance) {
        hash = getHash(iUniqueKey, iOpenId, iOpenId2);
    }
    else {
        hash = iOpenId;
        if (iOpenId2 > 0)
            hash += (iOpenId2 + 1) * 100;
    }

    MY_LOGD_IF(getDebugLevel() > 0, "%s: get hash, hash(%d):%#" PRIx64 ", uniqueKey:%d, openId:%d, openId2:%d",
        getName<T>().c_str(), isEnableMultiInstance, hash, iUniqueKey, iOpenId, iOpenId2);

    Ptr sp =  mInstances[hash].lock();
    if (sp == nullptr) {
        sp = std::make_shared<PipelinePlugin<T>>(iOpenId, iOpenId2);
        mInstances[hash] = sp;
        MY_LOGI("%s: create plugin(%p), hash(%d):%#" PRIx64 ", uniqueKey:%d, openId:%d, openId2:%d",
            getName<T>().c_str(), sp.get(), isEnableMultiInstance, hash, iUniqueKey, iOpenId, iOpenId2);
    }

    return sp;
}

template<typename T>
PipelinePlugin<T>::PipelinePlugin(MINT32 iOpenId, MINT32 iOpenId2)
: mOpenId(iOpenId)
, mOpenId2(iOpenId2)
, mpInterface(nullptr)
{
    MY_LOGI("%s: ctor(%p), openId:%d, openId2:%d", getName<T>().c_str(), this, mOpenId, mOpenId2);
}

template<typename T>
PipelinePlugin<T>::~PipelinePlugin()
{
    mpInterface = nullptr;
    mpProviders.clear();
    MY_LOGI("%s: dtor(%p), openId:%d, openId2:%d", getName<T>().c_str(), this, mOpenId, mOpenId2);

    for (const auto& item : mpSelections) {
        const auto& provider = item.first;
        const auto& selections = item.second;
        if (!selections.empty()) {
            MINT32 index = 0;
            MY_LOGW("%s: selctions is not empty, provAddr:%p, size:%zu", getName<T>().c_str(), provider.get(), mpSelections.size());
            for (auto& sel : selections) {
                MY_LOGW("%s: remained selction, #:%d, sleAddr:%p, token:%s",
                    getName<T>().c_str(), index,
                    sel.get(), ((sel == nullptr) ? "Nan" : to_string(sel->mTokenPtr).c_str()));
                index++;
            }
        }
    }
}

template class PipelinePlugin<Raw>;
template class PipelinePlugin<MultiFrame>;
template class PipelinePlugin<Fusion>;
template class PipelinePlugin<Yuv>;
template class PipelinePlugin<Depth>;
template class PipelinePlugin<Bokeh>;
template class PipelinePlugin<Join>;


/******************************************************************************
 *
 ******************************************************************************/
class MetadataSelection::Implementor
{
public:
    Implementor()
        : mRequired(MFALSE)
    {}

    MBOOL               mRequired;
    MetadataPtr         mControl;
    MetadataPtr         mAddtional;
    MetadataPtr         mDummy;
};

MetadataSelection::MetadataSelection()
    : mImpl(new Implementor())
{}

MetadataSelection::MetadataSelection(const MetadataSelection& ms)
    : mImpl(new Implementor())
{
    if (ms.mImpl != nullptr)
        *mImpl = *ms.mImpl;
}

MetadataSelection::~MetadataSelection()
{
    delete mImpl;
}

MetadataSelection& MetadataSelection::setRequired(MBOOL required)
{
    mImpl->mRequired = required;
    return *this;
}

MBOOL MetadataSelection::getRequired() const
{
    return mImpl->mRequired;
}

MetadataSelection& MetadataSelection::setControl(MetadataPtr control)
{
    mImpl->mControl = control;
    return *this;
}

MetadataSelection& MetadataSelection::setAddtional(MetadataPtr addtional)
{
    mImpl->mAddtional = addtional;
    return *this;
}

MetadataSelection& MetadataSelection::setDummy(MetadataPtr dummy)
{
    mImpl->mDummy = dummy;
    return *this;
}

MetadataPtr MetadataSelection::getControl() const
{
    return mImpl->mControl;
}

MetadataPtr MetadataSelection::getAddtional() const
{
    return mImpl->mAddtional;
}

MetadataPtr MetadataSelection::getDummy() const
{
    return mImpl->mDummy;
}

MVOID MetadataSelection::dump(std::ostream& os) const
{
    os << "{ }";
}

/******************************************************************************
 *
 ******************************************************************************/
class BufferSelection::Implementor
{
public:
    Implementor()
        : mRequired(MFALSE)
        , mOptional(MFALSE)
        , mSpecifiedSize(0, 0)
        , mAlignment(0, 0)
        , mDirtyFormats(MTRUE)
        , mDirtySizes(MTRUE)
    {}

    MBOOL               mRequired;
    MBOOL               mOptional;
    std::vector<MINT>   mAcceptedFormats;
    std::vector<MINT>   mAcceptedSizes;
    MSize               mSpecifiedSize;
    MSize               mAlignment;
    std::vector<MINT>   mvSupportFormats;
    std::vector<MINT>   mSupportSizes;
    std::vector<MINT>   mFormats;
    std::vector<MINT>   mSizes;
    MBOOL               mDirtyFormats;
    MBOOL               mDirtySizes;
};

BufferSelection::BufferSelection()
    : mImpl(new Implementor())
{}

BufferSelection::BufferSelection(const BufferSelection& bs)
    : mImpl(new Implementor())
{
    if (bs.mImpl != nullptr)
        *mImpl = *bs.mImpl;
}

BufferSelection::~BufferSelection()
{
    delete mImpl;
}

BufferSelection& BufferSelection::setRequired(MBOOL required)
{
    mImpl->mRequired = required;
    return *this;
}

BufferSelection& BufferSelection::setOptional(MBOOL optional)
{
    mImpl->mOptional = optional;
    return *this;
}

MBOOL BufferSelection::getRequired() const
{
    return mImpl->mRequired;
}

MBOOL BufferSelection::getOptional() const
{
    return mImpl->mOptional;
}

BufferSelection& BufferSelection::addAcceptedFormat(MINT fmt)
{
    mImpl->mAcceptedFormats.push_back(fmt);
    mImpl->mDirtyFormats = MTRUE;
    return *this;
}

BufferSelection& BufferSelection::addAcceptedSize(MINT sz)
{
    mImpl->mAcceptedSizes.push_back(sz);
    mImpl->mDirtySizes = MTRUE;
    return *this;
}

BufferSelection& BufferSelection::setSpecifiedSize(const MSize& sz)
{
    mImpl->mSpecifiedSize = sz;
    return *this;
}

BufferSelection& BufferSelection::setAlignment(MUINT32 width, MUINT32 height)
{
    mImpl->mAlignment.w = width;
    mImpl->mAlignment.h = height;
    return *this;
}

const MSize& BufferSelection::getSpecifiedSize() const
{
    return mImpl->mSpecifiedSize;
}

MVOID BufferSelection::getAlignment(MUINT32& width, MUINT32& height) const
{
    width = mImpl->mAlignment.w;
    height = mImpl->mAlignment.h;
}

MBOOL BufferSelection::isValid() const
{
    return getSizes().size() > 0 && getFormats().size() > 0;
}


BufferSelection& BufferSelection::addSupportFormat(MINT fmt)
{
    mImpl->mvSupportFormats.push_back(fmt);
    mImpl->mDirtyFormats = MTRUE;
    return *this;
}

BufferSelection& BufferSelection::addSupportSize(MINT sz)
{
    mImpl->mSupportSizes.push_back(sz);
    mImpl->mDirtySizes = MTRUE;
    return *this;
}

const vector<MINT>& BufferSelection::getFormats() const
{
    auto& formats = mImpl->mFormats;
    if (mImpl->mDirtyFormats) {
        formats.clear();

        auto& rSupport = mImpl->mvSupportFormats;
        for (MINT fmt : mImpl->mAcceptedFormats) {
            if (find(rSupport.begin(), rSupport.end(), fmt) != rSupport.end()) {
                formats.push_back(fmt);
            }
        }
    }

    return formats;
}

const vector<MINT>& BufferSelection::getSizes() const
{
    auto& sizes = mImpl->mSizes;
    if (mImpl->mDirtySizes) {
        sizes.clear();

        auto& rSupport = mImpl->mSupportSizes;
        for (MINT fmt : mImpl->mAcceptedSizes) {
            if (find(rSupport.begin(), rSupport.end(), fmt) != rSupport.end()) {
                sizes.push_back(fmt);
            }
        }
    }

    return sizes;
}


MVOID BufferSelection::dump(std::ostream& os) const
{
    os << "{";

    // print support format
    bool firstElement = true;
    for (auto fmt : mImpl->mvSupportFormats) {
        if (firstElement) {
            os << " Format:[";
            firstElement = false;
        } else
            os << " ";

        os << queryImageFormatName(fmt);
    }
    if (!firstElement)
        os << "]";

    auto stringizeImageSize = [](MINT s) -> const char *
    {
        switch(s) {
            case eImgSize_Full:
                return "Full";
            case eImgSize_Resized:
                return "Resized";
            case eImgSize_Specified:
                return "Specified";
        }
        return "Unknown";
    };

    // print support size
    firstElement = true;
    for(auto v : mImpl->mSupportSizes) {
        if (firstElement) {
            os << " Size:[";
            firstElement = false;
        } else
            os << " ";

        os << stringizeImageSize(v);
    }
    if (!firstElement)
        os << "]";

    // print accept format
    firstElement = true;
    for(auto fmt : mImpl->mAcceptedFormats) {
        if (firstElement) {
            os << " Accepted format:[";
            firstElement = false;
        } else
            os << " ";

        os << queryImageFormatName(fmt);
    }
    if (!firstElement)
        os << "]";

    // print accept size
    firstElement = true;
    for(auto v : mImpl->mAcceptedSizes) {
        if (firstElement) {
            os << " Accepted sizes:[";
            firstElement = false;
        } else
            os << " ";

        os << stringizeImageSize(v);
    }
    if (!firstElement)
        os << "]";


    // print specific size
    if (!!mImpl->mSpecifiedSize)
        os << " Specified:(" << mImpl->mSpecifiedSize.w << "x" <<  mImpl->mSpecifiedSize.h << ")";

    if (!!mImpl->mAlignment)
        os << " Align:(" << mImpl->mAlignment.w << "/" <<  mImpl->mAlignment.h << ")";
    os << " }";
}


/******************************************************************************
 * Object Printer
 ******************************************************************************/
std::ostream& operator<<(std::ostream& os, const std::shared_ptr<BufferHandle> hnd) {
    if (hnd == nullptr)
        return os << "{ null }";

    hnd->dump(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<MetadataHandle> hnd) {
    if (hnd == nullptr)
        return os << "{ null }";

    hnd->dump(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, const BufferSelection& sel) {
    sel.dump(os);
    return os;
}

std::ostream& operator<<(std::ostream& os, const MetadataSelection& sel) {
    sel.dump(os);
    return os;
}

/******************************************************************************
*
******************************************************************************/
};  //namespace NSPipelinePlugin
};  //namespace NSCam

