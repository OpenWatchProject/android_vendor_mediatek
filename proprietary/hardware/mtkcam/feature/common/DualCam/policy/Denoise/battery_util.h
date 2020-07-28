/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef BATTERY_UTIL_H_
#define BATTERY_UTIL_H_

#include <mtkcam/def/common.h>
#include <stdio.h>
#include <string.h>
#include <error.h>
#include <vector>

#define BATTERY_UITL_LOG_TAG "BatteryUtil"

class BatteryUtil
{
public:
    /**
     * \brief Same definition as BATTERY_STATUS_* in BatteryManager(Java)
     */
    enum ENUM_BATTERY_STATUS
    {
        BATTERY_STATUS_UNKNOWN      = 1,
        BATTERY_STATUS_CHARGING     = 2,
        BATTERY_STATUS_DISCHARGING  = 3,
        BATTERY_STATUS_NOT_CHARGING = 4,
        BATTERY_STATUS_FULL         = 5,
    };

    /**
     * \brief Same definition as BATTERY_HEALTH_* in BatteryManager(Java)
     */
    enum ENUM_BATTERY_HEALTH
    {
        BATTERY_HEALTH_UNKNOWN              = 1,
        BATTERY_HEALTH_GOOD                 = 2,
        BATTERY_HEALTH_OVERHEAT             = 3,
        BATTERY_HEALTH_DEAD                 = 4,
        BATTERY_HEALTH_OVER_VOLTAGE         = 5,
        BATTERY_HEALTH_UNSPECIFIED_FAILURE  = 6,
        BATTERY_HEALTH_COLD                 = 7,
    };

    struct BATTERY_PARAMS_T
    {
        ENUM_BATTERY_STATUS     status      = BATTERY_STATUS_UNKNOWN;   /**< Same definition as BATTERY_STATUS_* in BatteryManager(Java)  */
        ENUM_BATTERY_HEALTH     health      = BATTERY_HEALTH_UNKNOWN;   /**< Same definition as BATTERY_HEALTH_* in BatteryManager(Java)  */
        int                     capacity    = 0;                        /**< [0-100] */
        float                   voltage     = 0.0f;                     /**< Volt */
        float                   temperature = 0.0f;                     /**< Celsius */
    };

public:
    /**
     * \brief Get battery voltage in Volt
     * \return Battery voltage in Volt. 0 if error occurs.
     */
    static float getBatteryVoltage()
    {
        float voltage = 0.0f;

        FILE *fp = fopen("/sys/class/power_supply/battery/batt_vol", "r");
        if(fp) {
            int vol = 0;
            fscanf(fp, "%d", &vol);
            voltage = vol / 1000.0f;
            fclose(fp);
        } else {
            __printBatteryError("voltage");
        }

        return voltage;
    }

    /**
     * \brief Get battery capacity, range: [0-100]
     * \return Battery capacity, range: [0-100]
     */
    static int getBatteryCapacity()
    {
        int capacity = 0;

        FILE *fp = fopen("/sys/class/power_supply/battery/capacity", "r");
        if(fp) {
            fscanf(fp, "%d", &capacity);
            fclose(fp);
        } else {
            __printBatteryError("capacity");
        }

        return capacity;
    }

    /**
     * \brief Get battery temperature in Celsius degree
     * \return Battery temperature in Celsius degree
     */
    static float getBatteryTemperature()
    {
        float temperature = 0.0f;

        FILE *fp = fopen("/sys/class/power_supply/battery/batt_temp", "r");
        if(fp) {
            int t;
            fscanf(fp, "%d", &t);
            temperature = t/10.0f;
            fclose(fp);
        } else {
            __printBatteryError("temperature");
        }

        return temperature;
    }

    /**
     * \brief Get battery health
     * \return Battery health
     * \see ENUM_BATTERY_HEALTH
     */
    static ENUM_BATTERY_HEALTH getBatteryHealth()
    {
        // Text defined in $KERNEL/drivers/power/power_supply_sysfs.c: health_text
        static std::vector<const char *> HEALTH_TEXTS = {
            "Unknown", "Good", "Overheat", "Dead", "Over voltage", "Unspecified failure", "Cold"
        };
        static const size_t HEALTH_TEXTS_SIZE = HEALTH_TEXTS.size();

        ENUM_BATTERY_HEALTH health = BATTERY_HEALTH_UNKNOWN;

        FILE *fp = fopen("/sys/class/power_supply/battery/health", "r");
        if(fp) {
            const size_t BUFFER_SIZE = 32;
            char inBuf[BUFFER_SIZE];
            size_t readSize = fread(inBuf, 1, BUFFER_SIZE, fp);
            if(readSize > 0) {
                for(size_t n = 0; n < HEALTH_TEXTS_SIZE; ++n) {
                    if( !strncmp(HEALTH_TEXTS[n], inBuf, strlen(HEALTH_TEXTS[n])) ) {
                        health = static_cast<ENUM_BATTERY_HEALTH>(n+1);
                        break;
                    }
                }
            }

            fclose(fp);
        } else {
            __printBatteryError("health");
        }

        return health;
    }

    /**
     * \brief Get battery status
     * \return Battery status
     * \see ENUM_BATTERY_STATUS
     */
    static ENUM_BATTERY_STATUS getBatteryStatus()
    {
        // Text defined in $KERNEL/drivers/power/power_supply_sysfs.c: status_text
        static std::vector<const char *> STATUS_TEXTS = {
            "Unknown", "Charging", "Discharging", "Not charging", "Full"
        };
        static const size_t STATUS_TEXTS_SIZE = STATUS_TEXTS.size();

        ENUM_BATTERY_STATUS status = BATTERY_STATUS_UNKNOWN;

        FILE *fp = fopen("/sys/class/power_supply/battery/status", "r");
        if(fp) {
            const size_t BUFFER_SIZE = 32;
            char inBuf[BUFFER_SIZE];
            size_t readSize = fread(inBuf, 1, BUFFER_SIZE, fp);
            if(readSize > 0) {
                for(size_t n = 0; n < STATUS_TEXTS_SIZE; ++n) {
                    if( !strncmp(STATUS_TEXTS[n], inBuf, strlen(STATUS_TEXTS[n])) ) {
                        status = static_cast<ENUM_BATTERY_STATUS>(n+1);
                        break;
                    }
                }
            }

            fclose(fp);
        } else {
            __printBatteryError("status");
        }

        return status;
    }

    /**
     * \brief Get battery parameters
     * \details Kernel will update battery info in 10 seconds if status changed
     *          This API read and parse informations under /sys/class/power_supply/battery/
     *          Sample:
     *              #include <battery_util.h>
     *              BatteryUtil::BATTERY_PARAMS_T s;
     *              BatteryUtil::getBatteryParams(s);
     *
     * \param p BATTERY_PARAMS_T structure
     * \see getBatteryVoltage
     * \see getBatteryCapacity
     * \see getBatteryTemperature
     * \see getBatteryHealth
     * \see getBatteryStatus
     */
    static void getBatteryParams(BATTERY_PARAMS_T &p)
    {
        p.voltage     = getBatteryVoltage();
        p.capacity    = getBatteryCapacity();
        p.temperature = getBatteryTemperature();
        p.health      = getBatteryHealth();
        p.status      = getBatteryStatus();
    }

private:
    static void __printBatteryError(const char *failItem)
    {
        const int ERR_BUF_SIZE = 256;
        char errorBuf[ERR_BUF_SIZE];
        __android_log_print(ANDROID_LOG_ERROR, BATTERY_UITL_LOG_TAG,
                            "Fail to read battery %s: %s",
                            failItem,
                            strerror_r(errno, errorBuf, ERR_BUF_SIZE));
    }

};

#endif