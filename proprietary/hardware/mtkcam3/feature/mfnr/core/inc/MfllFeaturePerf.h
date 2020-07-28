#ifndef __MFLLFEATUREPERF_H__
#define __MFLLFEATUREPERF_H__

#include <mtkcam3/feature/mfnr/IMfllCore.h>
#include <mtkcam3/feature/mfnr/IMfllEventListener.h>

namespace mfll {

class MfllFeaturePerf : public IMfllEventListener {
public:
    // Factory
    static MfllFeaturePerf* createInstance();

protected:
    MfllFeaturePerf(void);

public:
    virtual ~MfllFeaturePerf(void);

/* implementation */
public:
    virtual void onEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL);
    virtual void doneEvent(enum EventType t, MfllEventStatus_t &status, void *mfllCore, void *param1 = NULL, void *param2 = NULL);
    virtual vector<enum EventType> getListenedEventTypes(void);
}; // class MfllFeaturePerf
}; //namespace mfll
#endif//__MFLLFEATUREPERF_H__
