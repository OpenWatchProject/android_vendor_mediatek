#include <limits.h>
#include <gtest/gtest.h>

// #include <mtkcam/drv/IHalSensor.h>
// #include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>

// using namespace NSCam;
// using namespace android;

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

// #define USER_NAME "HAL_UT"

// #define MAX_SENSOR_COUNT 4

// class UT
// {
// public:
//     UT() : sensorIndex{-1, -1, -1, -1}
//     {
//         NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
//         pHalSensorList->searchSensors();
//         SENSOR_COUNT = pHalSensorList->queryNumberOfSensors();

//         m_isReadyToUT = powerOnSensor();
//     }

//     ~UT() {
//         for(int i = 0; i < MAX_SENSOR_COUNT; i++) {
//             if(sensorIndex[i] >= 0) {
//                 powerOffSensor(sensorIndex[i]);
//             }
//         }

//         if(pHalSensor) {
//             pHalSensor->destroyInstance(USER_NAME);
//             pHalSensor = NULL;
//         }
//     }

//     bool isReadyToUT()
//     {
//         return m_isReadyToUT;
//     }

// protected:
//     bool powerOnSensor()
//     {
//         //======== Power on sensor ==========
//         if(SENSOR_COUNT < 2) {
//             printf("Error: Invalid sensor count: %d\n", SENSOR_COUNT);
//             return false;
//         }

//         NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();

//         StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_REAR_REAR);
//         StereoSettingProvider::getStereoSensorIndex(sensorIndex[0], sensorIndex[1]);
//         if(SENSOR_COUNT > 3) {
//             StereoSettingProvider::setStereoProfile(STEREO_SENSOR_PROFILE_FRONT_FRONT);
//             StereoSettingProvider::getStereoSensorIndex(sensorIndex[2], sensorIndex[3]);
//         }
//         MUINT pIndex[] = { (MUINT)sensorIndex[0], (MUINT)sensorIndex[1],
//                            (MUINT)sensorIndex[2], (MUINT)sensorIndex[3]};
//         if(!pHalSensorList)
//         {
//             MY_LOGE("pHalSensorList == NULL");
//             return false;
//         }
//         //
//         pHalSensor = pHalSensorList->createSensor( USER_NAME,
//                                                    (SENSOR_COUNT < 4) ? 2 : 4,
//                                                    pIndex);
//         if(pHalSensor == NULL)
//         {
//            MY_LOGE("pHalSensor is NULL");
//            return false;
//         }
//         // In stereo mode, Main1 needs power on first.
//         // Power on main1 and main2 successively one after another.
//         if( !pHalSensor->powerOn(USER_NAME, 1, &pIndex[0]) )
//         {
//             MY_LOGE("sensor power on failed: %d", pIndex[0]);
//             return false;
//         }
//         if( !pHalSensor->powerOn(USER_NAME, 1, &pIndex[1]) )
//         {
//             MY_LOGE("sensor power on failed: %d", pIndex[1]);
//             powerOffSensor(pIndex[0]);
//             return false;
//         }

//         if(SENSOR_COUNT > 3) {
//             if( !pHalSensor->powerOn(USER_NAME, 1, &pIndex[2]) )
//             {
//                 MY_LOGE("sensor power on failed: %d", pIndex[2]);
//                 powerOffSensor(pIndex[0]);
//                 powerOffSensor(pIndex[1]);
//                 return false;
//             }
//             if( !pHalSensor->powerOn(USER_NAME, 1, &pIndex[3]) )
//             {
//                 MY_LOGE("sensor power on failed: %d", pIndex[3]);
//                 powerOffSensor(pIndex[0]);
//                 powerOffSensor(pIndex[1]);
//                 powerOffSensor(pIndex[2]);
//                 return false;
//             }
//         }

//         return true;
//     }

//     bool powerOffSensor(MUINT index)
//     {
//         if( !pHalSensor->powerOff(USER_NAME, 1, &index) )
//         {
//             MY_LOGE("sensor power off failed: %d", index);
//             return false;
//         }

//         return true;
//     }

// private:
//     IHalSensor *pHalSensor;
//     int SENSOR_COUNT;
//     int sensorIndex[MAX_SENSOR_COUNT];
//     bool m_isReadyToUT;
// };

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    // UT ut;
    // if(ut.isReadyToUT()) {
        return RUN_ALL_TESTS();
    // }

    return 0;
}