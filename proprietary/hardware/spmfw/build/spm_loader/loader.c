
#include "loader.h"
//#include <syslog.h>
#include <private/android_filesystem_config.h>
#include <utils/Log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "spm_loader"
#define SPM_FIRMWARE_CHAR_DEVICE			"/dev/spm"

static int gLoaderFd = -1;

int main(void)
{
	int iRet = -1;
	int count = 0;
	char buf[4];

	do{
		//usleep(1000000);
		gLoaderFd = open(SPM_FIRMWARE_CHAR_DEVICE, O_RDONLY | O_NOCTTY);
		if((gLoaderFd < 0) && (count < 20))
		{
			count ++;
			ALOGI("Can't open device node(%s) count(%d)\n", SPM_FIRMWARE_CHAR_DEVICE,count);
			usleep(500000);
		}
		else
			break;
	}while(1);

	if (gLoaderFd > 0) {
		iRet = read(gLoaderFd, buf, 1);
		close(gLoaderFd);
	}

	return iRet;
}



