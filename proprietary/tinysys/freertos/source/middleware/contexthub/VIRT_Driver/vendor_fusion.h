#ifndef __VENDOR_FUSION_H__
#define __VENDOR_FUSION_H__

struct InterfaceDataOut {
    uint64_t timeStamp;
    float vec[6];
    uint8_t status;
};

struct InterfaceDataIn {
    uint64_t timeStamp;
    float vec[3];
    uint8_t status;
};

enum FusionSensorType
{
    ORIENT,
    GRAVITY,
    GEOMAG,
    LINEAR,
    GAME,
    ROTAT,
    GYRO,
    NUM_OF_FUSION_SENSOR
};

struct InterfaceEnParam {
    enum FusionSensorType sensorType;
    int en;
    uint64_t latency;
};

struct VendorFusionInterfact_t {
    char *name;
    int (*initLib)(int hwGyroSupport);
    int (*enableLib)(struct InterfaceEnParam *pEnParam);
    int (*setGyroData)(struct InterfaceDataIn *inData);
    int (*setAccData)(struct InterfaceDataIn *inData);
    int (*setMagData)(struct InterfaceDataIn *inData);
    int (*getGravity)(struct InterfaceDataOut *outData);
    int (*getRotationVector)(struct InterfaceDataOut *outData);
    int (*getOrientation)(struct InterfaceDataOut *outData);
    int (*getLinearaccel)(struct InterfaceDataOut *outData);
    int (*getGameRotationVector)(struct InterfaceDataOut *outData);
    int (*getGeoMagnetic)(struct InterfaceDataOut *outData);
    int (*getVirtualGyro)(struct InterfaceDataOut *outData);
};

void registerVendorInterface(struct VendorFusionInterfact_t *interface);
#endif
