#ifndef __AAL_SERVICE_CLIENT_H__
#define __AAL_SERVICE_CLIENT_H__

#include <stdint.h>
#include <sys/types.h>

#include <binder/Binder.h>
#include <utils/Singleton.h>
#include <utils/StrongPointer.h>
#include "IAALService.h"

namespace android {

class IAALService;
class AALClient : public Singleton<AALClient>
{
    friend class Singleton<AALClient>;

public:
    status_t setMode(int32_t mode);
    status_t setFunction(uint32_t funcFlags);
    status_t setFunctionProperty(uint32_t funcFlags);
    status_t setLightSensorValue(int32_t value);
    status_t setScreenState(int32_t state, int32_t brightness);
    status_t setSmartBacklightStrength(int32_t level);
    status_t setSmartBacklightRange(int32_t level);
    status_t setReadabilityLevel(int32_t level);
    status_t setLowBLReadabilityLevel(int32_t level);
    status_t getESSLEDMinStep(uint32_t *value);
    status_t setESSLEDMinStep(uint32_t value);
    status_t getESSOLEDMinStep(uint32_t *value);
    status_t setESSOLEDMinStep(uint32_t value);
    status_t getParameters(AALParameters *outParam);

    status_t custInvoke(int32_t cmd, int64_t arg);

    status_t readField(uint32_t field, uint32_t *value);
    status_t writeField(uint32_t field, uint32_t value);

    // Following APIs is available only if AAL_RUNTIME_TUNING_SUPPORT is defined
    status_t setAdaptField(IAALService::AdaptFieldId field, void *data, int32_t size, uint32_t *serial);
    status_t getAdaptSerial(IAALService::AdaptFieldId field, uint32_t *value);
    status_t getAdaptField(IAALService::AdaptFieldId field, void *data, int32_t size, uint32_t *serial);

private:
    AALClient();

    // DeathRecipient interface
    void serviceDied();

    status_t assertStateLocked() const;

    mutable Mutex mLock;
    mutable sp<IAALService> mAALService;
    mutable sp<IBinder::DeathRecipient> mDeathObserver;
};

};


#define AALClient_readField(field, value_ptr) \
    android::AALClient::getInstance().readField((field), (value_ptr))

#define AALClient_writeField(field, value) \
    android::AALClient::getInstance().writeField((field), (value))

#endif
