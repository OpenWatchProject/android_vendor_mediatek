#ifndef __AAL_SERVICE_CLIENT_H__
#define __AAL_SERVICE_CLIENT_H__

#include <stdint.h>
#include <sys/types.h>

#include <binder/Binder.h>
#include <utils/Singleton.h>
#include <utils/StrongPointer.h>

// This platform does not support AAL

namespace android {

struct AALParameters
{
    int readabilityLevel;
    int lowBLReadabilityLevel;
    int smartBacklightStrength;
    int smartBacklightRange;
};

class AALClient : public Singleton<AALClient>
{
    friend class Singleton<AALClient>;

public:
    status_t setMode(int32_t mode) { return -1; }
    status_t setBacklightColor(int32_t color) { return -1; }
    status_t setBacklightBrightness(int32_t level) { return -1; }
    status_t setLightSensorValue(int32_t value) { return -1; }
    status_t setScreenState(int32_t state, int32_t brightness) { return -1; }
    status_t setSmartBacklightStrength(int32_t level) { return -1; }

    status_t getParameters(AALParameters *outParam) {
        outParam->readabilityLevel = 128;
        outParam->lowBLReadabilityLevel = 128;
        outParam->smartBacklightStrength = 128;
        outParam->smartBacklightRange = 128;
        return -1;
    }

private:
    AALClient() { }
};

};

#endif
