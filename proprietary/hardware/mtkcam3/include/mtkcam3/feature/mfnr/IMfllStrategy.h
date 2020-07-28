#ifndef __IMFLLSTRATEGY_H__
#define __IMFLLSTRATEGY_H__

#include "MfllDefs.h"
#include "MfllTypes.h"

#include "IMfllNvram.h"

#include <utils/RefBase.h> // android::RefBase

using android::sp;

/**
 *  MfllStrategy has responsibility to decide if doing MFB (Multi Frame Blending)
 *  and suggest the suggested ISO and exposure if using AIS (Anti-Shake)
 */
namespace mfll {
class IMfllStrategy : public android::RefBase {
public:
    static IMfllStrategy* createInstance(void);
    virtual void destroyInstance(void);

/* interfaces */
public:
    /**
     *  Mfll strategy needs a NVRAM information provider to give a strategy
     */
    virtual enum MfllErr init(sp<IMfllNvram> &nvramProvider) = 0;

    /**
     *  Give a information and query output
     */
    virtual enum MfllErr queryStrategy(
            const MfllStrategyConfig_t &cfg, MfllStrategyConfig_t *out) = 0;
protected:
    virtual ~IMfllStrategy() { };
};
}; /* namespace mfll */

#endif//__IMFLLSTRATEGY_H__
