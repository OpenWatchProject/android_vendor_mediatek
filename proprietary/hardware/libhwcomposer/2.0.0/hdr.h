#ifndef HWC_HDR_H_
#define HWC_HDR_H_

// In this file, including any file under libhwcompsoer folder is prohibited!!!
#include <vector>
#include <system/graphics.h>

#include <utils/Singleton.h>

#include "utils/tools.h"
using namespace android;

enum
{
    SMPTE2086 = 0x000003FF,
    CTA861_3  = 0x00000C00,
};

struct HwcHdrMetadata
{

    android_smpte2086_metadata smpte2086{};
    android_cta861_3_metadata cta8613{};
    uint32_t type;
};

class HwcHdrUtils : public Singleton<HwcHdrUtils>
{
public:
    HwcHdrUtils()
        : m_enable(false)
    { }

    void setHDRFeature(const bool& enable);

    bool shouldChangeMetadata(uint32_t numElements, const int32_t* keys, const float* metadata,
                         const HwcHdrMetadata& hwcmetadata);

    void setMetadata(uint32_t numElements, const int32_t* keys, const float* metadata,
                    HwcHdrMetadata* hwcmetadata);

    void fillMetadatatoGrallocExtra(HwcHdrMetadata hwcmetadata, PrivateHandle handle, int32_t dataspace);

    // platform supported hdr cap
    std::vector<int32_t> m_hdr_capabilities;

    // platform supported hdr metadata
    std::vector<int32_t> m_hdr_metadata_keys;

private:
    bool m_enable;
};

#endif // HWC_HDR_H_
