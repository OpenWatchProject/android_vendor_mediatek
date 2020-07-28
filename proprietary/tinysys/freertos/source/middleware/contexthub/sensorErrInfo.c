/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sensorErrInfo.h>


void __attribute__((weak)) spi_send_err_to_ap(void) {
    osLog(LOG_ERROR, "not support errinfo to ap\n");
    return;
}

void __attribute__((weak)) i2c_send_err_to_ap(void) {
    osLog(LOG_ERROR, "not support errinfo to ap\n");
    return;
}

int sendSensorErrToAp(err_sensor_id_t err_sensor, err_case_id_t err_case, const char *name) {
    error_info err_infos;
    int err = 0;

    err_infos.case_id = err_case;
    err_infos.sensor_id = err_sensor;
    snprintf(err_infos.context, sizeof(err_infos.context),
        "%s error", name);
    err = scp_send_err_to_ap(&err_infos);
    return err;
}

void sendBusErrToAp(const void *err_sensor) {
    if (err_sensor == NULL) {
        osLog(LOG_ERROR, "no sensor id\n");
        return;
    }
    switch ((err_sensor_id_t)err_sensor) {
        case ERR_SENSOR_SPI:
            spi_send_err_to_ap();
            break;
        case ERR_SENSOR_I2C:
            i2c_send_err_to_ap();
            break;
        default :
            break;
    }
}

