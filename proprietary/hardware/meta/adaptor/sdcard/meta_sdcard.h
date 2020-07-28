#ifndef __META_SDCARD_H__
#define __META_SDCARD_H__

#include "meta_sdcard_para.h"
#include "MetaPub.h"

#define MAX_NUM_SDCARDS 	(4)

#include <log/log.h>
#undef LOG_TAG
#define LOG_TAG "META_SDCARD"

#define META_SDCARD_LOG(...) \
	do { \
		ALOGD(__VA_ARGS__); \
	} while (0)

typedef void (*SDCARD_CNF_CB)(SDCARD_CNF *cnf);

extern void Meta_SDcard_Register(SDCARD_CNF_CB callback);

#endif

