/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#ifndef __GF_DR_IPCH_H__
#define __GF_DR_IPCH_H__

#include "DrApi/DrApiCommon.h"
#include "DrApi/DrApiError.h"

extern drApiResult_t gf_dr_invoke_cmd_entry_point(gf_dr_cmd_t *cmd, threadid_t ipcClient);

extern void *drUtilsMapTaskBuffer(const taskid_t taskId, const void *startVirtClient, const size_t len, const uint32_t attr);
#endif // __GF_DR_IPCH_H__
