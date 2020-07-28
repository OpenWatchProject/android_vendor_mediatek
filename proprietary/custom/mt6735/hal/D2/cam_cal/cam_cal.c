#include "camera_custom_cam_cal.h"//for 658x new compilier option#include "camera_custom_eeprom.h"
#include <string.h>

CAM_CAL_TYPE_ENUM CAM_CALInit(void)
{
	return CAM_CAL_USED;
}

CAM_CAL_DEV_NAME_STATUS CAM_CALDeviceName(char* DevName, unsigned int bufSize)
{
    char* str= "CAM_CAL_DRV";
    unsigned int remainSize;

    // Test argument
    if (bufSize <= strlen(DevName)) {
        // Invalid argument for bufSize
        return CAM_CAL_BAD_ARGUMENT;
    }

    remainSize = bufSize - strlen(DevName) - 1;

    if (remainSize > strlen(str)) {
        strncat(DevName, str, remainSize);
        //Succeed
        return CAM_CAL_SUCCESS;
    } else {
        // Exceed buffer
        return CAM_CAL_EXCEED_BUFFER;
    }
}

