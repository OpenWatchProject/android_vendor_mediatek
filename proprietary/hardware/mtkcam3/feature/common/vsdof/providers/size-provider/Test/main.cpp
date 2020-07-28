#include <iostream>
#include <sstream>  //For ostringstream
#include <limits.h>
#include <gtest/gtest.h>

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/LogicalCam/IHalLogicalDeviceList.h>
#include <mtkcam/aaa/IHal3A.h>

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

#define USER_NAME "HAL_UT"

using namespace std;
using namespace NSCam;
using namespace android;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;

class UT
{
public:
    UT()
    {
        pLogicalDeviceList = MAKE_HalLogicalDeviceList();
        pLogicalDeviceList->searchDevices();
        SENSOR_COUNT = pLogicalDeviceList->queryNumberOfSensors();
        DEVICE_COUNT = pLogicalDeviceList->queryNumberOfDevices();

        std::vector<int32_t> availableLogicalDevices;
        for(int i = SENSOR_COUNT; i < DEVICE_COUNT; ++i) {
            auto sensorList = pLogicalDeviceList->getSensorId(i);
            if(sensorList.size() > 1) {
                availableLogicalDevices.push_back(i);
            }
        }

        auto getSensorNameList = [&](int32_t id) -> string
        {
            auto sensorList = pLogicalDeviceList->getSensorId(id);
            std::vector<string> nameList;
            for(auto &id : sensorList) {
                nameList.push_back(pLogicalDeviceList->queryDriverName(id));
            }
            std::ostringstream oss;
            std::copy(nameList.begin(), nameList.end()-1, std::ostream_iterator<string>(oss, "+"));
            oss << nameList.back();

            return oss.str();
        };

        if(availableLogicalDevices.size() == 1) {
            logicalDeviceID = availableLogicalDevices[0];
            printf("Set logical device: %d(%s)\n", logicalDeviceID, getSensorNameList(logicalDeviceID).c_str());
        } else {
            printf("Select logical device:\n");
            int index = 0;
            int input;
            for(auto &id : availableLogicalDevices) {
                printf("[%d] %d(%s)\n", index++, id, getSensorNameList(id).c_str());
            }

            cin >> input;
            if(input >= availableLogicalDevices.size()) {
                input = 0;
            }
            logicalDeviceID = availableLogicalDevices[input];
        }

        if(logicalDeviceID > 0) {
            auto sensorList = pLogicalDeviceList->getSensorId(logicalDeviceID);
            sensorIDs.resize(sensorList.size());
            std::copy(sensorList.begin(), sensorList.end(), sensorIDs.begin());
            StereoSettingProvider::setLogicalDeviceID(logicalDeviceID);

            m_isReadyToUT = powerOnSensor();
            initMetadata();
        }
    }

    ~UT() {
        clearMetadata();

        if( !pHalSensor->powerOff(USER_NAME, sensorIDs.size(), &sensorIDs[0]) )
        {
            MY_LOGE("sensor power off failed");
        }

        if(pHalSensor) {
            pHalSensor->destroyInstance(USER_NAME);
            pHalSensor = NULL;
        }
    }

    bool isReadyToUT()
    {
        return m_isReadyToUT;
    }

protected:
    bool powerOnSensor()
    {
        //======== Power on sensor ==========
        if(sensorIDs.size() < 2) {
            return false;
        }

        NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
        if(!pHalSensorList)
        {
            MY_LOGE("pHalSensorList == NULL");
            return false;
        }
        //
        pHalSensor = pHalSensorList->createSensor( USER_NAME,
                                                   sensorIDs.size(),
                                                   &sensorIDs[0]);
        if(pHalSensor == NULL)
        {
           MY_LOGE("pHalSensor is NULL");
           return false;
        }

        if( !pHalSensor->powerOn(USER_NAME, sensorIDs.size(), &sensorIDs[0]) )
        {
            MY_LOGE("sensor power on failed");
            return false;
        }

        return true;
    }

    void initMetadata()
    {
        sp<IMetadataProvider> pMetadataProvider;
        for(int i = 0; i < DEVICE_COUNT; ++i) {
            pMetadataProvider = IMetadataProvider::create(i);
            NSMetadataProviderManager::add(i, pMetadataProvider.get());
        }
    }

    void clearMetadata()
    {
        NSMetadataProviderManager::clear();
    }

private:
    IHalSensor *pHalSensor = nullptr;
    IHalLogicalDeviceList *pLogicalDeviceList = nullptr;
    int SENSOR_COUNT = 0;
    int DEVICE_COUNT = 0;
    std::vector<MUINT> sensorIDs;
    int logicalDeviceID = -1;
    bool m_isReadyToUT;
};

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);

    UT ut;
    if(ut.isReadyToUT()) {
        return RUN_ALL_TESTS();
    }

    return 0;
}