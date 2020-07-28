#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "libnvram.h"
#include "libnvram_log.h"
#include "nvram_platform.h"
#include "errno.h"

#if 0
typedef int(*pfCallbackForPlatform)(NVRAM_PLATFORM_T*);
pfCallbackForPlatform callback_for_nvram_platform = NULL;
extern pfCallbackForPlatform callback_for_nvram_platform;
#endif


int nvram_platform_callback(NVRAM_PLATFORM_T* pPlatform ) {

	pPlatform->log_block = 2;
	pPlatform->resv_block = 2;
	pPlatform->DM_block = 1;
#if 0 //FIXME
	pPlatform->layout_version = 0;
	pPlatform->header_offset = 0;
#endif
	ALOGD("nvram_layout_callback will run!!!");
	return 0;
}

#if 0
int init_nvram_platform_callback(void) {

	ALOGD("init_nvram_platform_callback!!!");
	callback_for_nvram_platform = nvram_platform_callback;
	return 0;
}
#endif

