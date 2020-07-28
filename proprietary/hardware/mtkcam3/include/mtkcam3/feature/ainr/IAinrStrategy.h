#ifndef __IAINRSTRATEGY_H__
#define __IAINRSTRATEGY_H__

#include "AinrDefs.h"
#include "AinrTypes.h"

#include "IAinrNvram.h"


/**
 *  AinrStrategy has responsibility to decide if doing MFB (Multi Frame Blending)
 *  and suggest the suggested ISO and exposure if using AIS (Anti-Shake)
 */
namespace ainr {
class IAinrStrategy {
public:
    static std::shared_ptr<IAinrStrategy> createInstance(void);
/* interfaces */
public:
    /**
     *  Ainr strategy needs a NVRAM information provider to give a strategy
     */
    virtual enum AinrErr init(const std::shared_ptr<IAinrNvram> &nvramProvider) = 0;

    /**
     *  Give a information and query output
     */
    virtual enum AinrErr queryStrategy(
            const AinrStrategyConfig_t &cfg, AinrStrategyConfig_t *out) = 0;
protected:
    virtual ~IAinrStrategy() { };
};
}; /* namespace ainr */

#endif//__IAINRSTRATEGY_H__
