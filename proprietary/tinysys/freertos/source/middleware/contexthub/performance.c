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
#include <eventnums.h>
#include <heap.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <sensors.h>
#include <seos.h>
#include <timer.h>
#include <util.h>
#include <math.h>
#include <plat/inc/rtc.h>
#include "scp_sem.h"
#include "dma.h"
#include <mt_gpt.h>
#include <performance.h>
#include "scp_ipi.h"

#ifdef DEBUG_PERFORMANCE

struct check_point records[1][MAX_CHECK_POINT];

bool isPhycSensor(uint32_t sensor_type)
{
    if (sensor_type == SENS_TYPE_ACCEL)
        return true;
    return false;
}

int getIndexOfRow(uint32_t output_sensor)
{
    switch (output_sensor) {
        case SENS_TYPE_ACCEL:
            return 0;
        default:
            return -1;
    }
}

int getIndexOfColume(uint32_t input_sensor, enum SENSOR_STATUS status)
{
    switch (input_sensor) {
        case SENS_TYPE_ACCEL:
            return status;
        default:
            return -1;
    }
}

bool isValid(uint32_t sensor_type)
{
    //if (sensor_type >= 0 && sensor_type <= 40)
    if (sensor_type == SENS_TYPE_ACCEL)
        return true;
    return false;

}

void mark_timestamp(uint32_t input_sensor, uint32_t output_sensor, enum SENSOR_STATUS status, uint64_t time)
{
    /* check the param validation */
    if (isValid(output_sensor) == false)
        return;

    if (isPhycSensor(output_sensor)) {
        int r_index = getIndexOfRow(output_sensor);
        int c_index = getIndexOfColume(input_sensor, status);

        records[r_index][c_index].check_time = time;

        if (status == MAX_CHECK_POINT - 1) {
            int i;

            for (i = 0; i < status; i++) {
                records[r_index][i].delta_time += records[r_index][i + 1].check_time - records[r_index][i].check_time;
            }
            if (++records[r_index][status].count == LIMIT) {
                //output the resoult
                // to be done
                for (i = 0; i < MAX_CHECK_POINT; i++) {
                    osLog(LOG_ERROR, "scpT[%d] timestamp is:%llu\n", i, records[r_index][i].check_time);
                    osLog(LOG_ERROR, "scpT%d-scpT%d average delta:%llu\n", i + 1, i, records[r_index][i].delta_time / LIMIT);
                    records[r_index][i].delta_time = 0;
                }
            }

        }
    } else {
        // Virtual Sensor, TBD
    }
}
#endif
