#ifndef __MFLLBSS_H__
#define __MFLLBSS_H__

#include <mtkcam3/feature/mfnr/IMfllBss.h>
#include <utils/Vector.h>
#include <mtkcam/aaa/IHal3A.h>
#include <custom/debug_exif/dbg_exif_param.h>
#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#endif

#include <BestShotSelection.h>
#include <camera_custom_mfll.h>


#define MFLL_BSS_ROI_PERCENTAGE 95

using std::vector;
using android::Vector;
using NSCam::IMetadata;

namespace mfll {
class MfllBss : public IMfllBss {
public:
    MfllBss();

public:
    enum MfllErr init(sp<IMfllNvram> &nvramProvider);

    vector<int> bss(
            const vector< sp<IMfllImageBuffer> > &imgs,
            vector<MfllMotionVector_t> &mvs,
            vector<int64_t> &timestamps);

    enum MfllErr setMfllCore(IMfllCore *c) { m_pCore = c; return MfllErr_Ok; }

    void setRoiPercentage(const int &p) { m_roiPercetange = p; }

    void setUniqueKey(const int &k) { /* not support */ }

    size_t getSkipFrameCount() { return 0; /* not support */ }

protected:
    int m_roiPercetange;
    sp<IMfllNvram> m_nvramProvider;
    IMfllCore *m_pCore;

protected:
    ~MfllBss() {};
}; // class MfllBss
}; // namespace mfll

#endif//__MFLLBSS_H__
