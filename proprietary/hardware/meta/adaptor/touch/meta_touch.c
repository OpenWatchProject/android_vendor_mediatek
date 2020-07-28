#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <log/log.h>
#include <unistd.h>
#include "meta_touch.h"


#define TOUCH_LOG ALOGD
const char *FILENAME = "/sys/module/tpd_setting/parameters/tpd_load_status";

void Meta_Touch_OP(Touch_REQ *req, char *peer_buff, unsigned short peer_len)
{
	Touch_CNF TouchMetaReturn;
	int status = 0;
	FILE *fp;

	TOUCH_LOG("peer_buff %p, peer_len %u \n", (void*)peer_buff, peer_len);

	memset(&TouchMetaReturn, 0, sizeof(TouchMetaReturn));

	TouchMetaReturn.header.id=req->header.id+1;
	TouchMetaReturn.header.token = req->header.token;
	TouchMetaReturn.status=META_FAILED;

	if ((fp = fopen(FILENAME, "r")) == NULL)
		TOUCH_LOG("can not open tpd_setting sysfs.\n");
	if (fp != NULL) {
		status = fgetc(fp);
		fclose(fp);
	}

	if((char)status == '1')
		TouchMetaReturn.status=META_SUCCESS;

	TOUCH_LOG("Cap touch status:%d\n", status);

	if (false == WriteDataToPC(&TouchMetaReturn,sizeof(Touch_CNF),NULL,0)) {
		TOUCH_LOG("%s : WriteDataToPC() fail 2\n", __FUNCTION__);
    }
	TOUCH_LOG("%s : Finish !\n", __FUNCTION__);
}

BOOL Meta_Touch_Init(void)
{
	return true;
}

BOOL Meta_Touch_Deinit(void)
{
	return true;
}
