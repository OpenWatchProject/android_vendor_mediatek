#include <string.h>
#include "camera_custom_cam_cal.h"//for 658x new compilier option#include "camera_custom_eeprom.h"

CAM_CAL_TYPE_ENUM CAM_CALInit(void)
{
	return CAM_CAL_USED;
}

/*LukeHu+-150706=For Multi-sensor calibration*/
CAM_CAL_DEV_NAME_STATUS CAM_CALDeviceName(char* DevName, unsigned int bufSize, MUINT32 sensorID)
{
    char* str;
    unsigned int remainSize;

    // Test argument
    if (bufSize <= strlen(DevName)) {
        // Invalid argument for bufSize
        return CAM_CAL_BAD_ARGUMENT;
    }

    remainSize = bufSize - strlen(DevName) - 1;

    switch (sensorID) {
        case 0x2108:
            str = "CAM_CAL_DRV1";
            break;
        case 0x8858:
            str = "CAM_CAL_DRV2";
            break;
        default:
            str = "CAM_CAL_DRV";
            break;
    }

    if (remainSize > strlen(str)) {
        strncat(DevName, str, remainSize);
        //Succeed
        return CAM_CAL_SUCCESS;
    } else {
        // Exceed buffer
        return CAM_CAL_EXCEED_BUFFER;
    }
}


