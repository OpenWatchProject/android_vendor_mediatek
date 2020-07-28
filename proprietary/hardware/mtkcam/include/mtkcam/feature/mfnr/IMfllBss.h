#ifndef __IMFLLBSS_H__
#define __IMFLLBSS_H__

#include "MfllDefs.h"
#include "MfllTypes.h"
#include "IMfllNvram.h"
#include "IMfllCore.h"

#include <utils/RefBase.h> // android::RefBase
#include <vector> // std::vector

using android::sp;
using std::vector;

namespace mfll {
class IMfllBss : public android::RefBase {
public:
    static IMfllBss* createInstance(void);
    void destroyInstance(void);

/* interfaces */
public:
    /**
     *  To init MfllBSS, and MfllBSS needs a NVRAM provider
     *  @param nvramProvider    Instance of IMfllNvram
     *  @return                 Returns MfllErr_Ok if init ok
     */
    virtual enum MfllErr init(sp<IMfllNvram>& nvramProvider) = 0;

    /**
     *  Calcuate bss
     *  @param imgs             IMfllImageBuffer contains image
     *  @param [in/out]mvs      Motion vectors, the motion vector will be updated
     *                          as sorted mvs.
     *  @return                 New index
     */
    virtual  vector<int> bss(
            const vector< sp<IMfllImageBuffer> > &imgs,
            vector<MfllMotionVector_t> &mvs,
            vector<int64_t> &timestamps) = 0;

    /**
     *  Caller (MfllCore) will invoke this function to set itemself to MfllBss
     *  @param c                The pointer of MfllCore caller
     *  @return                 Returns MfllErr_Ok if ok
     */
    virtual enum MfllErr setMfllCore(IMfllCore *c) = 0;

    /**
     *  To setup the percentage of ROI size from the original image
     *  @param p                Percentage in integer
     */
    virtual void setRoiPercentage(const int &p) = 0;

    /**
     *  To setup the pipeline uniquekey from first deque frame of mfll
     *  @param k                Key in integer
     */
    virtual void setUniqueKey(const int &k) = 0;

    /**
     *  BSS may tell how many frames are supposed to be skipped due to it's not
     *  good enough for blending
     *  @return                 The number of suggested skipping frame count (backward)
     *  @note                   Counting from the end to the beginning.
     *                          e.g: getSkipFrameCount returns 2, and we have 6 frame,
     *                               the suggested skipping frames are 5th, and 6th.
     */
    virtual size_t getSkipFrameCount(void) = 0;

protected:
    virtual ~IMfllBss(){};
};
}; /* namespace mfll */

#endif//__IMFLLBSS_H__
