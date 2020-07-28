/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <nanohubPacket.h>
#include <sensors.h>
#include <seos.h>

#include "contexthub_fw.h"
#include "contexthub_core.h"

extern uint32_t apGetStepCounter;
extern uint32_t apGetFloorCounter;
uint8_t step_debug_trace = 0;
uint8_t activity_debug_trace = 0;

static void stepCounterGetData(void *sample)
{
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = apGetStepCounter;
    //osLog(LOG_DEBUG, "stepCounterGetData stepcounter: %d\n", singleSample->idata);
}
static void floorCounterGetData(void *sample)
{
    struct SingleAxisDataPoint *singleSample = (struct SingleAxisDataPoint *)sample;
    singleSample->idata = apGetFloorCounter;
    osLog(LOG_DEBUG, "floorCounterGetData stepcounter: %ld\n", singleSample->idata);
}
static void activityGetData(void *sample)
{
}

static void stepRecognitionSetDebugTrace(int32_t trace)
{
    step_debug_trace = trace;
}

static void activitySetDebugTrace(int32_t trace)
{
    activity_debug_trace = trace;
}

void virtualSensorCoreInit(void)
{
    struct sensorCoreInfo mInfo;

    mInfo.sensType = SENS_TYPE_STEP_COUNT;
    mInfo.getData = stepCounterGetData;
    mInfo.setDebugTrace = stepRecognitionSetDebugTrace;
    sensorCoreRegister(&mInfo);

    mInfo.sensType = SENS_TYPE_ACTIVITY;
    mInfo.getData = activityGetData;
    mInfo.setDebugTrace = activitySetDebugTrace;
    sensorCoreRegister(&mInfo);

    mInfo.sensType = SENS_TYPE_FLOOR_COUNT;
    mInfo.getData = floorCounterGetData;
    sensorCoreRegister(&mInfo);
}
