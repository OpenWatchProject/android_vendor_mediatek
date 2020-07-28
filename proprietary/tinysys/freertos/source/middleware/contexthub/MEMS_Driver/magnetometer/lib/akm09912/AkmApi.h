/******************************************************************************
 *
 *  $Id: $
 *
 * -- Copyright Notice --
 *
 * Copyright (c) 2004 Asahi Kasei Microdevices Corporation, Japan
 * All Rights Reserved.
 *
 * This software program is the proprietary program of Asahi Kasei Microdevices
 * Corporation("AKM") licensed to authorized Licensee under the respective
 * agreement between the Licensee and AKM only for use with AKM's electronic
 * compass IC.
 *
 * THIS SOFTWARE IS PROVIDED TO YOU "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABLITY, FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT OF
 * THIRD PARTY RIGHTS, AND WE SHALL NOT BE LIABLE FOR ANY LOSSES AND DAMAGES
 * WHICH MAY OCCUR THROUGH USE OF THIS SOFTWARE.
 *
 * -- End Asahi Kasei Microdevices Copyright Notice --
 *
 ******************************************************************************/
#ifndef __AKMAPI_H__
#define __AKMAPI_H__

#include <stdint.h>
#include "AKCompass.h"


#define AKMAPI_ERROR    1
#define AKMAPI_SUCCESS  0


#define AKD_SUCCESS         1   ///< The return value. It represents success.
#define AKD_ERROR           0   ///< The return value. It represents error.
#define AKD_FAIL            -1  ///< The return value. It represents the function fails.

#define AKD_DATA_READY          1
#define AKD_DATA_NOT_READY      0

//void Logger_log(const char * label, int level, const char * msg, ...);

/*****************************************************************************/
/*! Prepare magnetic measurement.
*/
void AKM_Open( void);
void AKM_SetDevice(uint8_t *wia);

#ifdef USE_I2C_OPERATION
void AKD_Sleep( uint16_t millisecond);
#endif

int16* GetBDATA(void);

int16_t AKD_TxData(
    const uint8_t address,
    const uint8_t* data,
    const uint16_t numberOfBytesToWrite
);

int16_t AKD_RxData(
    const uint8_t address,
    uint8_t* data,
    const uint16_t numberOfBytesToRead
);

int AKM_ForceCalibration(void);

void AKM_ReadFUSEROM(void);
int16_t AKD_GetFormation( void);

/*****************************************************************************/
/*! Start magnetic measurement.
*/
int16_t AKM_StartMeasurement( void);

/*****************************************************************************/
/*! Check whether the data is ready or not.
  \retval #AKD_DATA_NOT_READY Data is not ready.
  \retval #AKD_DATA_READY Data is ready.
*/
int16_t AKM_IsDataReady( void);

/*****************************************************************************/
/*! Get magnetic measurement data from AK09911.
   \retval #AKM_API_ERROR Failed to get data.
   \retval #AKM_API_SUCCESS Succeeded to get data.
 */
int16_t AKM_SetMagData(int regx, int regy, int regz, uint64_t currTime);

int16_t AKM_SetGyroData(const double grx, const double gry, const double grz, uint64_t currTime);

AKSCPRMS* GetPRMS(void);

/*****************************************************************************
 * mag_cal[0] mag_cal[1] mag_cal[2]   Calibrated Magnetometer, Unit: uT
 * mag_offset[0] mag_offset[1] mag_offset[2]   Magnetometer offset, Unit: uT
 *
 */
int16_t AKM_Calibrate(
        double  mag_cal[],
        double  mag_offset[],
        int16_t  *accuracy
);


#define CONVERT_AKSC_TO_MICROTESLA  (0.06f)

/*****************************************************************************/
/*! Stop the measurement and free the resource.
*/
void AKM_Close( void);


/*****************************************************************************/
/*! Structure for Library Information
*/
typedef struct _AklibInfo{
    int16_t partno;
    int16_t major;
    int16_t minor;
    int16_t revision;
    int16_t datecode;
    int16_t variation;
} AKLIBINFO;


/*****************************************************************************/
/*! Get the AKM library information.

  \param[out] info A pointer for AKLIBINFO structure.
*/
void AKM_GetLibraryInfo( AKLIBINFO* info);


/*****************************************************************************/
/*! Test result structure
*/
typedef struct _AkTestFailed{
    const char *ptestno;        // Pointer for test number string
    const char *ptestname;      // Pointer for test name string
    int16_t testdata;           // Tested data
    int16_t lolimit;            // lower limit
    int16_t hilimit;            // higher limit
} AKTESTFAILED;


/*****************************************************************************/
/*! Process a device test sequence.
  \param[out] A pointer to AKTESTRESULT structure.
  \retval	#AKMAPI_TEST_PASS (pass),  #AKMAPI_TEST_FAIL (fail), #AKMAPI_TEST_ERROR (error)
*/
int16_t AKM_TestDevice( AKTESTFAILED *result);

#define AKMAPI_TEST_PASS 1
#define AKMAPI_TEST_FAIL -1
#define AKMAPI_TEST_ERROR 0

#define AKM_MAX_TEST_NUM    31

/*****************************************************************************/
/*! Debug data structure.
*/
typedef struct _AkDebugInfo{
    uint16_t errFlag;   // Return value of GetMagneticVector()
                        // 0x00 // The process has been successfully done.
                        // 0x01 // The formation is changed.
                        // 0x02 // Data read error occurred.
                        // 0x04 // Data overflow occurred.
                        // 0x08 // Offset values overflow.
                        // 0x10 // hbase was changed.
                        // 0x20 // A fluctuation of magnetic field occurred.
                        // 0x40 // AKSC_VNorm error.

    uint8_t ST1;        // Status 1
    int16_t HXHL;       // X-axis packed measurement data 16bit
    int16_t HYHL;       // Y-axis packed measurement data 16bit
    int16_t HZHL;       // Z-axis packed measurement data 16bit
    uint8_t ST2;        // Status 2

    uint8_t ASAX;       // Magnetic sensor X-axis sensitivity adjustment value
    uint8_t ASAY;       // Magnetic sensor Y-axis sensitivity adjustment value
    uint8_t ASAZ;       // Magnetic sensor Z-axis sensitivity adjustment value

    int16_t hdata_x;    // Magnetic sensor X-axis data that format is suited for SmartCompass library
    int16_t hdata_y;    // Magnetic sensor Y-axis data that format is suited for SmartCompass library
    int16_t hdata_z;    // Magnetic sensor Z-axis data that format is suited for SmartCompass library

    int32_t hbase_x;    // Magnetic sensor X-axis data base
    int32_t hbase_y;    // Magnetic sensor Y-axis data base
    int32_t hbase_z;    // Magnetic sensor Z-axis data base
} AKDEBUGINFO;

/*****************************************************************************/
/*! Get the debug data.
*/
AKDEBUGINFO* AKM_GetDebugInfo( void);
int16_t AKM_SaveContext(float offset[3]);
int16_t AKM_ReloadContext(float offset[3]);

#endif /* __AKMAPI_H__ */
