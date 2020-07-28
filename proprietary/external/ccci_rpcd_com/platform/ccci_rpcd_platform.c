/*
* Copyright (C) 2011-2017 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
//#include "hardware/ccci_intf.h"
#include "power.h"
//#include <assert.h>

#include <cutils/log.h>
#include "ccci_rpcd_platform.h"

#define RPC_WAKE_LOCK_NAME "ccci_rpc"

void rpc_com_wakelock(int lock)
{
    if (lock)
    	acquire_wake_lock(PARTIAL_WAKE_LOCK, RPC_WAKE_LOCK_NAME);
    else
    	release_wake_lock(RPC_WAKE_LOCK_NAME);
}

int mtk_property_get(char *property_name, char *property_val, char *default_val)
{
    return property_get(property_name, property_val, default_val);
}

