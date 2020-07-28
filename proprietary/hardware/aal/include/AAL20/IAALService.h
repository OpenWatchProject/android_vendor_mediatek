#ifndef __IAALSERVICE_H__
#define __IAALSERVICE_H__

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/BinderService.h>


// Not all version supports Android runtime tuning.
// Runtime tuning is available only if this macro is defined.
#define MTK_AAL_RUNTIME_TUNING_SUPPORT


namespace android
{

struct AALParameters
{
    int readabilityLevel;
    int lowBLReadabilityLevel;
    int smartBacklightStrength;
    int smartBacklightRange;
};

//
//  Holder service for pass objects between processes.
//
class IAALService : public IInterface
{
protected:
    enum {
        AAL_SET_FUNCTION = IBinder::FIRST_CALL_TRANSACTION,
        AAL_SET_FUNCTION_PROPERTY,
        AAL_SET_LIGHT_SENSOR_VALUE,
        AAL_SET_SCREEN_STATE,
        AAL_SET_SMART_BACKLIGHT_STRENGTH,
        AAL_SET_SMART_BACKLIGHT_RANGE,
        AAL_SET_READABILITY_LEVEL,
        AAL_SET_LOW_BL_READABILITY_LEVEL,
        AAL_GET_ESSLEDMINSTEP,
        AAL_SET_ESSLEDMINSTEP,
        AAL_GET_ESSOLEDMINSTEP,
        AAL_SET_ESSOLEDMINSTEP,
        AAL_GET_PARAMETERS,
        AAL_CUST_INVOKE,
        AAL_WRITE_FIELD,
        AAL_READ_FIELD,
        AAL_SET_ADAPT_FIELD,
        AAL_GET_ADAPT_SERIAL,
        AAL_GET_ADAPT_FIELD
    };

public:

    // screen brightenss mode copy from Settings.System
    enum BrightnessMode {
        /** SCREEN_BRIGHTNESS_MODE value for manual mode. */
        SCREEN_BRIGHTNESS_MODE_MANUAL = 0,
        /** SCREEN_BRIGHTNESS_MODE value for automatic mode. */
        SCREEN_BRIGHTNESS_MODE_AUTOMATIC,
        /** SCREEN_ECO_BRIGHTNESS_MODE value for automatic eco backlight mode. */
        SCREEN_BRIGHTNESS_ECO_MODE_AUTOMATIC,
    };

    enum IScreenState {
        SCREEN_STATE_OFF = 0,
        SCREEN_STATE_DOZE = 1,
        SCREEN_STATE_DIM = 2,
        SCREEN_STATE_BRIGHT
    };

    enum {
        FUNC_NONE = 0x0,
        FUNC_CABC = 0x2,
        FUNC_DRE  = 0x4
    };

    DECLARE_META_INTERFACE(AALService);

    virtual status_t setFunction(uint32_t funcFlags) = 0;
    virtual status_t setFunctionProperty(uint32_t funcFlags) = 0;
    virtual status_t setLightSensorValue(int32_t value) = 0;
    virtual status_t setScreenState(int32_t state, int32_t brightness) = 0;
    virtual status_t setSmartBacklightStrength(int32_t level) = 0;
    virtual status_t setSmartBacklightRange(int32_t level) = 0;
    virtual status_t setReadabilityLevel(int32_t level) = 0;
    virtual status_t setLowBLReadabilityLevel(int32_t level) = 0;
    virtual status_t getESSLEDMinStep(uint32_t *value) = 0;
    virtual status_t setESSLEDMinStep(uint32_t value) = 0;
    virtual status_t getESSOLEDMinStep(uint32_t *value) = 0;
    virtual status_t setESSOLEDMinStep(uint32_t value) = 0;
    virtual status_t getParameters(AALParameters *outParam) = 0;

    virtual status_t custInvoke(int32_t cmd, int64_t arg) = 0;

    virtual status_t readField(uint32_t field, uint32_t *value) = 0;
    virtual status_t writeField(uint32_t field, uint32_t value) = 0;

    // Runtime tuning for Android framework
    enum AdaptFieldId {
        ALI2BLI_CURVE_LENGTH = 0,
        ALI2BLI_CURVE = 1,
        BLI_RAMP_RATE_BRIGHTEN = 2,
        BLI_RAMP_RATE_DARKEN = 3
    };

    // Following APIs is available only if AAL_RUNTIME_TUNING_SUPPORT is defined
    virtual status_t setAdaptField(AdaptFieldId field, void *data, int32_t size, uint32_t *serial) = 0;
    virtual status_t getAdaptSerial(AdaptFieldId field, uint32_t *value) = 0;
    virtual status_t getAdaptField(AdaptFieldId field, void *data, int32_t size, uint32_t *serial) = 0;
};

class BnAALService : public BnInterface<IAALService>
{
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};

};

#endif




