
#define LOG_TAG "EEPROM_Test"


#include <utils/Errors.h>
#include <cutils/properties.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <mtkcam/drv/IHalSensor.h>


#define PRINT_MSG(fmt, arg...)    printf("[%s]" fmt, __FUNCTION__, ##arg)
#define PRINT_WRN(fmt, arg...)    printf("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define PRINT_ERR(fmt, arg...)    printf("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)


using namespace NSCam;
using namespace android;

/*******************************************************************************
*
********************************************************************************/
void show_Sensors()
{
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    if(pHalSensorList) {
        pHalSensorList->searchSensors();
        unsigned int const sensorNum = pHalSensorList->queryNumberOfSensors();

        PRINT_MSG("sensorNum %d \n",sensorNum);
        for( unsigned int i = 0; i < sensorNum; i++) {
            int sensorIndex = i;
            PRINT_MSG("name:%s type:%d \n", pHalSensorList->queryDriverName(sensorIndex), pHalSensorList->queryType(sensorIndex));
            PRINT_MSG("index:%d, SensorDevIdx:%d \n", sensorIndex, pHalSensorList->querySensorDevIdx(sensorIndex));
        }

    }

}



/*******************************************************************************
* test_EEPROM
********************************************************************************/
void test_EEPROM(int senidx)
{
    char const *const szCallerName = "R1";
    IHalSensor *pHalSensor = NULL;
    IHalSensor::ConfigParam configParam;
    SensorStaticInfo sensorStaticInfo;
    SensorDynamicInfo sensorDynamicInfo;
    CAM_CAL_DATA_STRUCT GetCamCalData;
    PCAM_CAL_DATA_STRUCT pCamcalData = &GetCamCalData;

    IHalSensorList*const pHalSensorList = IHalSensorList::get();
    pHalSensorList->searchSensors();
    int const sensorNum = pHalSensorList->queryNumberOfSensors();

    if(senidx < sensorNum) {
        int sensorIndexDual = pHalSensorList->querySensorDevIdx(senidx);
        int sensorIndex = senidx;

        PRINT_MSG("name:%s type:%d \n", pHalSensorList->queryDriverName(sensorIndex), pHalSensorList->queryType(sensorIndex));
        PRINT_MSG("index:%d, SensorDevIdx:%d \n", sensorIndex, pHalSensorList->querySensorDevIdx(sensorIndex));

        pHalSensorList->querySensorStaticInfo(sensorIndexDual, &sensorStaticInfo);


        PRINT_MSG("Sensor index:%d, sensor id:0x%x\n", sensorIndex, sensorStaticInfo.sensorDevID);

        pHalSensor = pHalSensorList->createSensor(szCallerName, sensorIndex);

        if(!pHalSensor)
        {
            PRINT_ERR("createSensor fail");
            return;
        }

        PRINT_MSG("pHalSensor->powerOn start\n");
        pHalSensor->powerOn(szCallerName, 1, (MUINT *)&sensorIndex);

        PRINT_MSG("get eeprom start\n");
        CamCalDrvBase *p = CamCalDrvBase::createInstance();
        for (unsigned int cmd = (unsigned int) CAMERA_CAM_CAL_DATA_MODULE_VERSION;
                cmd < (unsigned int) CAMERA_CAM_CAL_DATA_LIST; cmd++) {
            p->GetCamCalCalData(sensorIndexDual, (CAMERA_CAM_CAL_TYPE_ENUM) cmd, pCamcalData);
        }
        PRINT_MSG("S2aVer = 0x%x\n", (pCamcalData->Single2A).S2aVer);

        PRINT_MSG("--- CamCalCalData v2 ---\n");

        int rr = 0;
        {
            CAM_CAL_MODULE_VERSION_STRUCT version_data;
            rr = p->GetCamCalCalDataV2(sensorIndexDual, CAMERA_CAM_CAL_DATA_MODULE_VERSION,
                    &version_data, sizeof(version_data));
            PRINT_MSG("[%d] mtk module version = %d\n", rr, (version_data.DataVer));
        }

        {
            CAM_CAL_PART_NUM_STRUCT part_num_data;
            rr = p->GetCamCalCalDataV2(sensorIndexDual, CAMERA_CAM_CAL_DATA_PART_NUMBER,
                    &part_num_data, sizeof(part_num_data));
            PRINT_MSG("[%d] mtk part num = %x %x .. %x\n", rr,
                    part_num_data.PartNumber[0], part_num_data.PartNumber[1], part_num_data.PartNumber[23]);
        }

        {
            CAM_CAL_LSC_DATA_STRUCT lsc_data;
            rr = p->GetCamCalCalDataV2(sensorIndexDual, CAMERA_CAM_CAL_DATA_SHADING_TABLE,
                    &lsc_data, sizeof(lsc_data));
            PRINT_MSG("[%d] mtk lsc type = %d\n", rr, (lsc_data.SingleLsc.LscTable.MtkLcsData.MtkLscType));
        }

        {
            CAM_CAL_2A_DATA_STRUCT t2a_data;
            rr = p->GetCamCalCalDataV2(sensorIndexDual, CAMERA_CAM_CAL_DATA_3A_GAIN,
                    &t2a_data, sizeof(t2a_data));
            PRINT_MSG("[%d] mtk 2a S2aVer = %d\n", rr, (t2a_data.Single2A.S2aVer));
            PRINT_MSG("[%d] mtk 2a Warm = %d\n", rr,
                    (t2a_data.Single2A.S2aAF_t.Warm_AF_macro_calibration_temperature));
            PRINT_MSG("[%d] mtk 2a Pos = %d\n", rr,
                    (t2a_data.Single2A.S2aAF_t.Posture_AF_infinite_calibration));
        }

        {
            CAM_CAL_PDAF_DATA_STRUCT pdaf_data;
            rr = p->GetCamCalCalDataV2(sensorIndexDual, CAMERA_CAM_CAL_DATA_PDAF,
                    &pdaf_data, sizeof(pdaf_data));
            PRINT_MSG("[%d] mtk pdaf size = %u\n", rr, (pdaf_data.PDAF.Size_of_PDAF));
        }

        {
            CAM_CAL_STEREO_DATA_STRUCT stereo_data;
            rr = p->GetCamCalCalDataV2(sensorIndexDual, CAMERA_CAM_CAL_DATA_STEREO_DATA,
                    &stereo_data, sizeof(stereo_data));
            PRINT_MSG("[%d] mtk stereo size = %u\n", rr, (stereo_data.Stereo_Data.Size_of_Stereo_Data));
        }

        {
            CAM_CAL_DATA_STRUCT dump_data;
            rr = p->GetCamCalCalDataV2(sensorIndexDual, CAMERA_CAM_CAL_DATA_DUMP,
                    &dump_data, sizeof(dump_data));
            PRINT_MSG("[%d] mtk stereo size = %u\n", rr, (dump_data.Stereo_Data.Size_of_Stereo_Data));
        }

        {
            CAM_CAL_LENS_ID_STRUCT lens_data;
            rr = p->GetCamCalCalDataV2(sensorIndexDual, CAMERA_CAM_CAL_DATA_LENS_ID,
                    &lens_data, sizeof(lens_data));
            PRINT_MSG("[%d] mtk lens id = %x .. %x\n", rr, lens_data.LensDrvId[0], lens_data.LensDrvId[9]);
        }

        PRINT_MSG("pHalSensor->powerOff\n");
        pHalSensor->powerOff(szCallerName, 1, (MUINT *)&sensorIndex);
    } else {
        PRINT_ERR("sensor idx = %d, but only %d sensors\n", senidx, sensorNum);
    }
}

/*******************************************************************************
*                 Main Function
********************************************************************************/
int main(int argc, char** argv)
{
    int ret = 0;
    int sensorIdx = 0;
    PRINT_MSG("sizeof long : %d\n", (int)sizeof(long));
    if (argc < 2) {
        show_Sensors();
        PRINT_MSG("Param: 1 <sensorIdx>\n");
        PRINT_MSG("<sensorIdx> : main(0), Sub(1), Main2(2), sub2(3)\n");
        return -1;
    }
    if(argc > 1)
        sensorIdx = atoi(argv[1]);

    PRINT_MSG("argc num:%d\n", argc);
    test_EEPROM(sensorIdx);
    PRINT_MSG("done\n");

    return ret;
}

