#ifndef HWC_HRT_INTERFACE_H
#define HWC_HRT_INTERFACE_H

#include <sstream>
#include <vector>

#include <utils/StrongPointer.h>
#include <utils/String8.h>
#include <utils/RefBase.h>

using namespace android;

class HWCDisplay;

class HrtCommon : public RefBase
{
public:
    virtual ~HrtCommon() {}

    virtual bool isEnabled() const;

    virtual bool isRPOEnabled() const;

    virtual void dump(String8* str);

    virtual void printQueryValidLayerResult();

    virtual void modifyMdpDstRoiIfRejectedByRpo(const std::vector<sp<HWCDisplay> >& displays);

    virtual void run(std::vector<sp<HWCDisplay> >& displays, const bool& is_skip_validate);

    virtual void fillLayerConfigList(const std::vector<sp<HWCDisplay> >& displays) = 0;

    virtual void fillDispLayer(const std::vector<sp<HWCDisplay> >& displays) = 0;

    virtual void fillLayerInfoOfDispatcherJob(const std::vector<sp<HWCDisplay> >& displays) = 0;

    virtual void setCompType(const std::vector<sp<HWCDisplay> >& displays);

    virtual bool queryValidLayer() = 0;
protected:
    std::stringstream m_hrt_result;
};

HrtCommon* createHrt();

#endif
