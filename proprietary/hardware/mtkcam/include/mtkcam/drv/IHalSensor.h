/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_DRV_IHALSENSOR_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_DRV_IHALSENSOR_H_

#include <mtkcam/def/common.h>
#include <mtkcam/utils/module/module.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <kd_imgsensor_define.h>
#include <vector>


class IBaseCamExif;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *  Sensor Scenario ID
 ******************************************************************************/
enum
{
    SENSOR_SCENARIO_ID_NORMAL_PREVIEW = MSDK_SCENARIO_ID_CAMERA_PREVIEW,
    SENSOR_SCENARIO_ID_NORMAL_CAPTURE = MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,
    SENSOR_SCENARIO_ID_NORMAL_VIDEO   = MSDK_SCENARIO_ID_VIDEO_PREVIEW,
    SENSOR_SCENARIO_ID_SLIM_VIDEO1    = MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO,
    SENSOR_SCENARIO_ID_SLIM_VIDEO2    = MSDK_SCENARIO_ID_SLIM_VIDEO,
    SENSOR_SCENARIO_ID_CUSTOM1        = MSDK_SCENARIO_ID_CUSTOM1,
    SENSOR_SCENARIO_ID_CUSTOM2        = MSDK_SCENARIO_ID_CUSTOM2,
    SENSOR_SCENARIO_ID_CUSTOM3        = MSDK_SCENARIO_ID_CUSTOM3,
    SENSOR_SCENARIO_ID_CUSTOM4        = MSDK_SCENARIO_ID_CUSTOM4,
    SENSOR_SCENARIO_ID_CUSTOM5        = MSDK_SCENARIO_ID_CUSTOM5,
    /**************************************************************************
     * All unnamed scenario id for a specific sensor must be started with
     * values >= SENSOR_SCENARIO_ID_UNNAMED_START.
     **************************************************************************/
    SENSOR_SCENARIO_ID_UNNAMED_START = 0x100,
};

enum
{
    SENSOR_DEV_NONE = 0x00,
    SENSOR_DEV_MAIN = 0x01,
    SENSOR_DEV_SUB  = 0x02,
    SENSOR_DEV_PIP  = 0x03,
    SENSOR_DEV_MAIN_2 = 0x04,
    SENSOR_DEV_MAIN_3D = 0x05,
    SENSOR_DEV_SUB_2 = 0x08,
    SENSOR_DEV_MAIN_3 = 0x10,
    SENSOR_DEV_SUB_3 = 0x20,
    SENSOR_DEV_MAIN_4 = 0x40
};

enum
{
    IDX_MAIN_CAM	= 0x00,
    IDX_SUB_CAM		= 0x01,
    IDX_MAIN2_CAM	= 0x02,
    IDX_SUB2_CAM	= 0x03,
    IDX_MAIN3_CAM       = 0x04,
    IDX_MAX_CAM_NUMBER,
};

enum
{
    CAM_TG_NONE = 0x0,
    CAM_TG_1    = 0x1,
    CAM_TG_2    = 0x2,
    CAM_TG_3    = 0x3,
    CAM_SV_1    = 0x10,
    CAM_SV_2    = 0x11,
    CAM_SV_3    = 0x12,
    CAM_SV_4    = 0x13,
    CAM_SV_5    = 0x14,
    CAM_SV_6    = 0x15,
    CAM_SV_7    = 0x16,
    CAM_SV_8    = 0x17,
    CAM_SV_MASK = 0x10, /* _CAM_SV_ */
    CAM_TG_ERR  = 0xFF
};

enum
{
    ONE_PIXEL_MODE  = 0x0,
    TWO_PIXEL_MODE  = 0x1,
    FOUR_PIXEL_MODE = 0x2,
    EIGHT_PIXEL_MODE = 0X3,
    UNKNOWN_PIXEL_MODE = 0xFF,
};

enum
{
    SENSOR_TYPE_UNKNOWN = 0x0,
    SENSOR_TYPE_RAW,
    SENSOR_TYPE_YUV,
    SENSOR_TYPE_RGB,
    SENSOR_TYPE_JPEG,
};

enum
{
    RAW_SENSOR_8BIT = 0x0,
    RAW_SENSOR_10BIT,
    RAW_SENSOR_12BIT,
    RAW_SENSOR_14BIT,
    RAW_SENSOR_ERROR = 0xFF,
};

enum
{
    SENSOR_FORMAT_ORDER_RAW_B = 0x0,
    SENSOR_FORMAT_ORDER_RAW_Gb,
    SENSOR_FORMAT_ORDER_RAW_Gr,
    SENSOR_FORMAT_ORDER_RAW_R,
    SENSOR_FORMAT_ORDER_UYVY,
    SENSOR_FORMAT_ORDER_VYUY,
    SENSOR_FORMAT_ORDER_YUYV,
    SENSOR_FORMAT_ORDER_YVYU,
    SENSOR_FORMAT_ORDER_MONO,
    SENSOR_FORMAT_ORDER_NONE = 0xFF,
};

enum
{
    SENSOR_RAW_Bayer = 0x0,
    SENSOR_RAW_MONO,
    SENSOR_RAW_RWB,
    SENSOR_RAW_PD, /* For 3A use, PD sensor needs to enable PDC. (pd sensor but not dual pd) */
    SENSOR_RAW_4CELL,
    SENSOR_RAW_4CELL_BAYER,
    SENSOR_RAW_4CELL_HW_BAYER, /*Sensor HW Remosaic*/
    SENSOR_RAW_FMT_NONE = 0xFF,
};

enum
{
    SENSOR_PREVIEW_DELAY = 0,			/*!<Request delay frame for sensor set to preview mode be valid */
    SENSOR_VIDEO_DELAY,					/*!<Request delay frame for sensor set to video mode be valid */
    SENSOR_CAPTURE_DELAY,				/*!<Request delay frame for sensor set to capture mode be valid */
    SENSOR_YUV_AWB_SETTING_DELAY,		/*!<Request delay frame for YUV sensor set WB setting be valid*/
    SENSOR_YUV_EFFECT_SETTING_DELAY,	/*!<Request delay frame for YUV sensor set color effect be valid */
    SENSOR_AE_SHUTTER_DELAY,			/*!<Request delay frame for sensor set AE shutter be valid */
    SENSOR_AE_GAIN_DELAY,				/*!<Request delay frame for sensor set AE gain be valid */
    SENSOR_AE_ISP_DELAY,
};

enum
{
    SENSOR_VHDR_MODE_NONE  = 0x0,
    SENSOR_VHDR_MODE_IVHDR = 0x1,
    SENSOR_VHDR_MODE_MVHDR = 0x2,
    SENSOR_VHDR_MODE_ZVHDR = 0x9,
    SENSOR_VHDR_MODE_4CELL_MVHDR = 0xA
};

enum
{
    ZHDR_MODE_NONE     = 0x0,
    ZHDR_MODE_G0_R0_B0 = 0x01,
    ZHDR_MODE_G0_R0_B1 = 0x02,
    ZHDR_MODE_G0_R1_B0 = 0x03,
    ZHDR_MODE_G0_R1_B1 = 0x04,
    ZHDR_MODE_G1_R0_B0 = 0x05,
    ZHDR_MODE_G1_R0_B1 = 0x06,
    ZHDR_MODE_G1_R1_B0 = 0x07,
    ZHDR_MODE_G1_R1_B1 = 0x08
};

enum {
    FOUR_CELL_CAL_TYPE_GAIN_TBL    =  0x0,
    FOUR_CELL_CAL_TYPE_XTALK_CAL   =  0x0,
    FOUR_CELL_CAL_TYPE_DPC         =  0x1,
    FOUR_CELL_CAL_TYPE_FD_DPC      =  0x2
};

enum {
    SENSOR_CMD_SET_SENSOR_EXP_TIME	        = 0x1000,	/*!<Command to set sensor exposure time in unit us. arg1:[input]exp time, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_SENSOR_EXP_LINE,						/*!<Command to set sensor exposure line. arg1:[input]exp line, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_SENSOR_GAIN,							/*!<Command to set sensor gain. arg1:[input]gain, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_SENSOR_DUAL_GAIN,					/*!<Command to set sensor gain. arg1:[input]gain, arg2:[input]gain, arg3:N/A*/
    SENSOR_CMD_SET_FLICKER_FRAME_RATE,					/*!<Command to set sensor gain. arg1:[input]gain, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_VIDEO_FRAME_RATE,					/*!<Command to set video scenario sensor frame rate. arg1:[input]video frame rate, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_AE_EXPOSURE_GAIN_SYNC,				/*!<Command to set AE sync write shutter and gain. arg1:[input]raw gain R, Gr raw gain Gb, B exposure time, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_CCT_FEATURE_CONTROL,					/*!<Command to set CCT feature. arg1:[input]ACDK_SENSOR_FEATURE_ENUM, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA,				/*!<Command to set sensor calibration data. arg1:[input]SET_SENSOR_CALIBRATION_DATA_STRUCT, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO,			/*!<Command to set sensor max output frame rate by scenario. arg1:[input] ACDK_SCENARIO_ID_ENUM, arg2:[input] frame rate (10base), arg3:N/A*/
    SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,					/*!<Command to set sensor test pattern output enable control. arg1:[input] on/off, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_SENSOR_ESHUTTER_GAIN,                /*!<Command to set sensor sensor shutter and sensor gain. arg1:[input] pointer of shutter and gain, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_OB_LOCK,                             /*!<Command to set sensor OB lock enable/disable control. arg1:[input] on/off, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN,             /*!<Command to set sensor exposure time in unit us & Gain. arg1:[input]long exp time, arg2:short exp time, arg3:Gain*/
    SENSOR_CMD_SET_SENSOR_HDR_SHUTTER,                  /*!<Command to set sensor exposure time in unit us & Gain. arg1:[input]long exp time, arg2:short exp time*/
    SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN,                 /*!<Command to set sensor exposure time in unit us & Gain. arg1:[input]long exp time, arg2:short exp time, arg3:Gain*/
    SENSOR_CMD_SET_SENSOR_AWB_GAIN, 				    /*!<Command to set sensor exposure time in unit us & Gain. arg1:[input]long exp time, arg2:short exp time, arg3:Gain*/
    SENSOR_CMD_SET_SENSOR_ISO, 			                /*!<Command to set sensor ISO*/
    SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD,                  /*!<Command to set sensor AWB update/disable from OTP control. arg1:[input] on/off, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_SENSOR_OTP_LSC_CMD,                  /*!<Command to set sensor LSC update/disable from OTP control. arg1:[input] on/off, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_MIN_MAX_FPS,                         /*!<Command to set sensor frame rate. arg1:[input]Max FPS, arg2:MIN Fps,*/
    SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME,               /*!<Command to set sensor exposure time and frame time in unit us. arg1:[input]exp time,arg2:frame time, arg3:N/A*/
    SENSOR_CMD_SET_SENSOR_EXP_TIME_BUF_MODE,
    SENSOR_CMD_SET_SENSOR_EXP_LINE_BUF_MODE,
    SENSOR_CMD_SET_SENSOR_GAIN_BUF_MODE,
    SENSOR_CMD_SET_I2C_BUF_MODE_EN,
    SENSOR_CMD_SET_STREAMING_SUSPEND,                   /*!<Command to let sensor enter suspend mode, stop streaming. arg1:N/A, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_STREAMING_RESUME,                    /*!<Command to let sensor enter streaming mode, start streaming. arg1:[input]exp time, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_N3D_CONFIG              = 0x1100,    /*!<Command to set native 3D mode configuration. arg1: [input]halSensorN3dConfig_t, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_N3D_I2C_POS,                         /*!<Command to set count in SMI clock rate for I2C1&2 delay arg1:[input]position count, arg2:N/A, arg3:N/A */
    SENSOR_CMD_SET_N3D_I2C_TRIGGER,                     /*!<Command to set hardware trigger mode arg1:[input]trigger mode, arg2:N/A, arg3:N/A */
    SENSOR_CMD_SET_N3D_I2C_STREAM_REGDATA,              /*!<Command to set I2C command to N3D I2C buffer used to control stream arg1:N/A, arg2:N/A, arg3:N/A */
    SENSOR_CMD_SET_N3D_START_STREAMING,                 /*!<Command to set sensor driver to start straming out arg1:N/A, arg2:N/A, arg3:N/A  */
    SENSOR_CMD_SET_N3D_STOP_STREAMING,                  /*!<Command to set sensor driver to stop straming out arg1:N/A, arg2:N/A, arg3:N/A  */
    SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT = 0x2000,	/*!<Command to get sensor operation unstable frame number. arg1:[output] delay frame count, arg2:[input] operation mode, arg3:N/A*/
    SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,					/*!<Command to get sensor operating pixel clock. arg1:[output] pixel clock frequency, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,			/*!<Command to get sensor operating frame line length and line pxel value. arg1:[output] line pixels & frame line number, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_SENSOR_FEATURE_INFO,					/*!<Command to get sensor feature information. arg1:[output] feature info, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,		/*!<Command to get sensor default output frame rate by scenario. arg1:[input] ACDK_SCENARIO_ID_ENUM, arg2:[output] frame rate (10 base), arg3:N/A*/
    SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE,			/*!<Command to get sensor test pattern output check sum value. arg1:[output] test pattern check sum value, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_TEMPERATURE_VALUE,                   /*!<Command to get sensor temperature value (unit: degree). arg1:[output] temperature, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO,                /*!<Command to get sensor Crop window size */
    SENSOR_CMD_GET_SENSOR_PIXELMODE,                    /*!<Command to get sensor pixel mode by scenario */
    SENSOR_CMD_GET_SENSOR_PDAF_INFO,                    /*!<Command to get sensor PDAF postion informatoin */
    SENSOR_CMD_GET_SENSOR_POWER_ON_STETE,               /*!<Command to get sensor power on state arg1:[output]sensorDevId, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT = 0x2100,/*!<Command to get native 3D module difference count value arg1:[output] halSensorN3dDiffCnt_t, arg2:N/A, arg3:N/A  */
    SENSOR_CMD_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME,     /*!<Command to get time between stream out command to vsync occur. arg1:[output] time(us), arg2:N/A, arg3:N/A  */
    SENSOR_CMD_SET_YUV_FEATURE_CMD			= 0x3000,	/*!<Command to set YUV sensor feature. arg1:[input] feature id, arg2:[input] feature value, arg3:N/A*/
    SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE,				/*!<Command to set YUV sensor single focus. arg1:N/A, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_YUV_CANCEL_AF,						/*!<Command to cancel YUV sensor AF. arg1:N/A, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_YUV_CONSTANT_AF,						/*!<Command to set YUV sensor constant focus. arg1:N/A, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_YUV_INFINITY_AF,                     /*!<Command to set YUV sensor infinite focus. arg1:N/A, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_YUV_AF_WINDOW,    					/*!<Command to set YUV sensor AF window. arg1:[input]AF window, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_YUV_AE_WINDOW,   					/*!<Command to set YUV sensor AE window. arg1:[input]AE window, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_YUV_AUTOTEST,                        /*!<Command to set YUV sensor autotest. arg1:[input]YUV_AUTOTEST_T cmd, arg2:[input] param, arg3:N/A*/
    SENSOR_CMD_SET_YUV_3A_CMD,                          /*!<Command to set YUV sensor 3A cmd. arg1:[input]ACDK_SENSOR_3A_LOCK_ENUM cmd, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_SET_YUV_GAIN_AND_EXP_LINE,               /*!<Command to set YUV sensor gain & exposure line cmd. arg1:[input]gain, arg2:[input]exp line, arg3:N/A*/
    SENSOR_CMD_GET_SENSOR_VC_INFO,
    SENSOR_CMD_GET_YUV_AF_STATUS			= 0x4000,   /*!<Command to get YUV sensor AF status. arg1:[output] af status, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_AE_STATUS,                   /*!<Command to get YUV sensor current AE and AWB status. arg1:[output] AE status, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_AWB_STATUS,                   /*!<Command to get YUV sensor current AE and AWB status. arg1:[output] AE status, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN,            /*!<Command to get YUV sensor AE shutter/gain for EV5 & 13 and AWB gain value. arg1:[output] pSensorAEAWBRefStruct, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN,   /*!<Command to get YUV sensor current AE shutter/gain and AWB gain value. arg1:[output] pSensorAEAWBCurStruct, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS,          /*!<Command to get YUV sensor AF window number. arg1:[output] window number, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS,       /*!<Command to get YUV sensor AE window number. arg1:[output] window number, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_EXIF_INFO,                       /*!<Command to get YUV sensor EXIF info. arg1:[output] SENSOR_EXIF_INFO_STRUCT, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_DELAY_INFO,                      /*!<Command to get YUV sensor delay info. arg1:[output] SENSOR_DELAY_INFO_STRUCT, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_AE_AWB_LOCK,                     /*!<Command to get YUV sensor AE/AWB lock capability. arg1:[output] AE lock support, arg2: AWB lock support, arg3:N/A*/
    SENSOR_CMD_GET_YUV_STROBE_INFO,                     /*!<Command to get YUV sensor current strobe info. arg1:[output] SENSOR_FLASHLIGHT_AE_INFO_STRUCT, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_YUV_TRIGGER_FLASHLIGHT_INFO,         /*!<Command to get YUV sensor current flashlight auto fire necessity. arg1:[output] fire or not, arg2:N/A, arg3:N/A*/
    SENSOR_CMD_GET_PDAF_DATA,                           /*!<Command to get PDAF data from  sensor OTP. arg1:[input] offset, arg2:[input]pointer of buffer, arg3:[input]size*/
    SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY,                /*!<Command to get PDAF capacity by scenario ID. arg1:[input] scenario, arg2:support PDAF or not*/
    SENSOR_CMD_SET_PDFOCUS_AREA,						/*!<Command to set sensor PD Focus area*/
    SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER,			    /*!<Command to get PDAF data from  sensor OTP. arg1:[input] offset, arg2:[input]pointer of buffer, arg3:[input]size*/
    SENSOR_CMD_DEBUG_P1_DQ_SENINF_STATUS,			    /*!<Command to do debug task at p1 dequeue*/
    SENSOR_CMD_GET_SENSOR_HDR_CAPACITY, 				/*!<Command to get HDR capability by scenario*/
    SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING,
    SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING,
    SENSOR_CMD_GET_4CELL_SENSOR,                        /*!<Command to get 4-Cell Sensor data from  sensor OTP. arg1:[input] 4CELL_CAL_TYPE_ENUM, arg2:[output]pointer of buffer, arg3:[output]size*/
    SENSOR_CMD_SET_SENINF_CAM_TG_MUX,
    SENSOR_CMD_SET_TEST_MODEL,
    SENSOR_CMD_DEBUG_GET_SENINF_METER,
    SENSOR_CMD_GET_MIPI_PIXEL_RATE,
    SENSOR_CMD_SET_SENSOR_HDR_ATR,
    SENSOR_CMD_SET_SENSOR_HDR_TRI_GAIN,
    SENSOR_CMD_SET_SENSOR_HDR_TRI_SHUTTER,
    SENSOR_CMD_SET_SENSOR_LSC_TBL,
    SENSOR_CMD_GET_VERTICAL_BLANKING,
    SENSOR_CMD_GET_SENSOR_SYNC_MODE_CAPACITY,           /*!<Command to get the sensor's master/slave mode capacity.*/
    SENSOR_CMD_GET_SENSOR_SYNC_MODE,                    /*!<Command to get the current sensor to master/slave mode.*/
    SENSOR_CMD_SET_SENSOR_SYNC_MODE,                    /*!<Command to set the sensor to master/slave mode. arg1:[input] sync mode*/
    SENSOR_CMD_SET_DUAL_CAM_MODE,                       /*!<Command to set dual cam mode or not. arg1:[input] dual cam or not*/
    SENSOR_CMD_GET_PIXEL_RATE,
    SENSOR_CMD_GET_SENSOR_VC_INFO2,
    SENSOR_CMD_GET_AE_EFFECTIVE_FRAME_FOR_LE,
    SENSOR_CMD_GET_AE_FRAME_MODE_FOR_LE,
    SENSOR_CMD_GET_HORIZONTAL_BLANKING,
    SENSOR_CMD_GET_PIXEL_CLOCK_FREQ_BY_SCENARIO,
    SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM_BY_SCENARIO,
    SENSOR_CMD_GET_SENINF_PIXEL_RATE,
    SENSOR_CMD_GET_BINNING_TYPE,
    SENSOR_CMD_GET_START_OF_EXPOSURE,

    SENSOR_CMD_MAX                 = 0xFFFF
} ;

struct SensorStaticInfo{
    SensorStaticInfo(): sensorDevID(0xffff), sensorType(0), sensorFormatOrder(0), rawSensorBit(0), rawFmtType(0), iHDRSupport(false), PDAF_Support(0),
    HDR_Support(0), previewWidth(0), previewHeight(0), captureWidth(0), captureHeight(0), videoWidth(0), videoHeight(0), video1Width(0), video1Height(0),
    video2Width(0), video2Height(0), SensorCustom1Width(0), SensorCustom1Height(0), SensorCustom2Width(0), SensorCustom2Height(0), SensorCustom3Width(0),
    SensorCustom3Height(0), SensorCustom4Width(0), SensorCustom4Height(0), SensorCustom5Width(0), SensorCustom5Height(0), SensorGrabStartX_PRV(0),
    SensorGrabStartY_PRV(0), SensorGrabStartX_CAP(0), SensorGrabStartY_CAP(0), SensorGrabStartX_VD(0), SensorGrabStartY_VD(0), SensorGrabStartX_VD1(0),
    SensorGrabStartY_VD1(0), SensorGrabStartX_VD2(0), SensorGrabStartY_VD2(0), SensorGrabStartX_CST1(0), SensorGrabStartY_CST1(0), SensorGrabStartX_CST2(0),
    SensorGrabStartY_CST2(0), SensorGrabStartX_CST3(0), SensorGrabStartY_CST3(0), SensorGrabStartX_CST4(0), SensorGrabStartY_CST4(0), SensorGrabStartX_CST5(0),
    SensorGrabStartY_CST5(0), previewDelayFrame(0), captureDelayFrame(0), videoDelayFrame(0), video1DelayFrame(0), video2DelayFrame(0),
    aeShutDelayFrame(0), aeSensorGainDelayFrame(0), aeISPGainDelayFrame(0), FrameTimeDelayFrame(0), Custom1DelayFrame(0), Custom2DelayFrame(0),
    Custom3DelayFrame(0), Custom4DelayFrame(0), Custom5DelayFrame(0), orientationAngle(0), facingDirection(0), previewFrameRate(0), captureFrameRate(0),
    videoFrameRate(0), video1FrameRate(0), video2FrameRate(0), custom1FrameRate(0), custom2FrameRate(0), custom3FrameRate(0), custom4FrameRate(0),
    custom5FrameRate(0), fakeOrientation(false), horizontalViewAngle(0), verticalViewAngle(0), previewActiveWidth(0), previewActiveHeight(0),
    captureActiveWidth(0), captureActiveHeight(0), videoActiveWidth(0), videowActiveHeight(0), previewHoizontalOutputOffset(0),
    previewVerticalOutputOffset(0), captureHoizontalOutputOffset(0), captureVerticalOutputOffset(0), videoHoizontalOutputOffset(0),
    videoVerticalOutputOffset(0), iHDR_First_IS_LE(0), SensorModeNum(0), virtualChannelSupport(false), PerFrameCTL_Support(0), ZHDR_MODE(0),
    SensorOrientation(0), sensorModuleID(0) {}

    MUINT sensorDevID; //0x5642,....
    MUINT sensorType;  //SENSOR_TYPE_RAW, ....
    MUINT sensorFormatOrder; //SENSOR_FORMAT_ORDER_RAW_B,...
    MUINT rawSensorBit; //RAW_SENSOR_8BIT,....
    MUINT rawFmtType;   //Bayer or Mono,....
    MBOOL iHDRSupport;
    MUINT PDAF_Support;/*0: NO PDAF, 1: PDAF Raw Data mode, 2:PDAF VC mode(Full), 3:PDAF VC mode(Binning), 4: PDAF DualPD Raw Data mode, 5: PDAF DualPD VC mode*/
    MUINT HDR_Support;/*0: NO HDR, 1: iHDR, 2:mvHDR, 3:zHDR*/
    MUINT previewWidth;
    MUINT previewHeight;
    MUINT captureWidth;
    MUINT captureHeight;
    MUINT videoWidth;
    MUINT videoHeight;
    MUINT video1Width;
    MUINT video1Height;
    MUINT video2Width;
    MUINT video2Height;
    MUINT SensorCustom1Width;   // new for custom
    MUINT SensorCustom1Height;
    MUINT SensorCustom2Width;
    MUINT SensorCustom2Height;
    MUINT SensorCustom3Width;
    MUINT SensorCustom3Height;
    MUINT SensorCustom4Width;
    MUINT SensorCustom4Height;
    MUINT SensorCustom5Width;
    MUINT SensorCustom5Height;
    MUINT SensorGrabStartX_PRV;       /* MSDK_SCENARIO_ID_CAMERA_PREVIEW */
    MUINT SensorGrabStartY_PRV;
    MUINT SensorGrabStartX_CAP;       /* MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG & ZSD */
    MUINT SensorGrabStartY_CAP;
    MUINT SensorGrabStartX_VD;        /* MSDK_SCENARIO_ID_VIDEO_PREVIEW */
    MUINT SensorGrabStartY_VD;
    MUINT SensorGrabStartX_VD1;       /* MSDK_SCENARIO_ID_VIDEO_PREVIEW */
    MUINT SensorGrabStartY_VD1;
    MUINT SensorGrabStartX_VD2;       /* MSDK_SCENARIO_ID_VIDEO_PREVIEW */
    MUINT SensorGrabStartY_VD2;
    MUINT SensorGrabStartX_CST1;      /* SENSOR_SCENARIO_ID_CUSTOM1 */
    MUINT SensorGrabStartY_CST1;
    MUINT SensorGrabStartX_CST2;      /* SENSOR_SCENARIO_ID_CUSTOM2 */
    MUINT SensorGrabStartY_CST2;
    MUINT SensorGrabStartX_CST3;       /* SENSOR_SCENARIO_ID_CUSTOM3 */
    MUINT SensorGrabStartY_CST3;
    MUINT SensorGrabStartX_CST4;       /* SENSOR_SCENARIO_ID_CUSTOM4 */
    MUINT SensorGrabStartY_CST4;
    MUINT SensorGrabStartX_CST5;       /* SENSOR_SCENARIO_ID_CUSTOM5 */
    MUINT SensorGrabStartY_CST5;
    MUINT previewDelayFrame;
    MUINT captureDelayFrame;
    MUINT videoDelayFrame;
    MUINT video1DelayFrame;
    MUINT video2DelayFrame;
    MUINT aeShutDelayFrame;
    MUINT aeSensorGainDelayFrame;
    MUINT aeISPGainDelayFrame;
    MUINT FrameTimeDelayFrame;
    MUINT Custom1DelayFrame;
    MUINT Custom2DelayFrame;
    MUINT Custom3DelayFrame;
    MUINT Custom4DelayFrame;
    MUINT Custom5DelayFrame;
    MUINT orientationAngle;
    MUINT facingDirection;
    MUINT previewFrameRate;//10 base
    MUINT captureFrameRate;//10 base
    MUINT videoFrameRate;//10 base
    MUINT video1FrameRate;//10 base
    MUINT video2FrameRate;//10 base
    MUINT custom1FrameRate;//10 base
    MUINT custom2FrameRate;//10 base
    MUINT custom3FrameRate;//10 base
    MUINT custom4FrameRate;//10 base
    MUINT custom5FrameRate;//10 base
    MBOOL fakeOrientation;
    MUINT horizontalViewAngle;
    MUINT verticalViewAngle;
    MUINT previewActiveWidth;//3d use onlyl?
    MUINT previewActiveHeight;//3d use onlyl?
    MUINT captureActiveWidth;//3d use onlyl?
    MUINT captureActiveHeight;//3d use onlyl?
    MUINT videoActiveWidth;//3d use onlyl?
    MUINT videowActiveHeight;//3d use onlyl?
    MUINT previewHoizontalOutputOffset;//3d use onlyl?
    MUINT previewVerticalOutputOffset; //3d use onlyl?
    MUINT captureHoizontalOutputOffset;//3d use onlyl?
    MUINT captureVerticalOutputOffset; //3d use onlyl?
    MUINT videoHoizontalOutputOffset;//3d use onlyl?
    MUINT videoVerticalOutputOffset; //3d use onlyl?
    MUINT8  iHDR_First_IS_LE;
    MUINT8  SensorModeNum;
    MBOOL   virtualChannelSupport;
    MUINT8  PerFrameCTL_Support;
    MUINT8  ZHDR_MODE; /*0: zhdr no support , 1~ 8 suppport pattern*/
    MUINT  SensorOrientation;
    MUINT sensorModuleID;
};

enum
{
    EmbInfo     = 0x0, /*Embedded data*/
    YInfo       = 0x1,
    AEInfo      = 0x2,
    FlickerInfo = 0x3,
    HDR_DATA_MAX_NUM
};

struct SeninfConfigInfo

{
    MUINT32 grab_start_offset;
    MUINT32 grab_size_x;
    MUINT32 tg_info;
    MUINT32 mux_pixel_mode;
    MUINT32 feature_type;
    MUINT32 seninf_mux_info;
    MUINT32 finalized;

    SeninfConfigInfo()
    : grab_start_offset(0)
    , grab_size_x(0)
    , tg_info(0)
    , mux_pixel_mode(0)
    , feature_type(0)
    , seninf_mux_info(0)
    , finalized(0){}

};

struct SensorDynamicInfo {
    MUINT32 TgInfo;       /*TG_NONE,TG_1,...*/
    MUINT32 pixelMode;    /*ONE_PIXEL_MODE, TWO_PIXEL_MODE, FOUR_PIXEL_,MODE*/
    MUINT32 TgVR1Info;    /* CAM_TG_1, CAM_TG_2, CAM_TG_3, CAM_TG_4, CAM_TG_5, CAM_TG_6*/
    MUINT32 TgVR2Info;
    MUINT32 TgCLKInfo;    /*Unit : Khz*/
    MUINT32 HDRInfo;
    MUINT32 PDAFInfo;
    MUINT32 HDRPixelMode; /*ONE_PIXEL_MODE, TWO_PIXEL_MODE, FOUR_PIXEL_,MODE*/
    MUINT32 PDAFPixelMode;
    MUINT32 CamInfo[HDR_DATA_MAX_NUM];
    MUINT32 PixelMode[HDR_DATA_MAX_NUM];
    std::vector<SeninfConfigInfo> config_infos;
};

//N3D config
/**
 *@struct<SENSOR_N3D_CONFIG_STRUCT>
 *@brief:structure used in sensor hal provided sendCommand() api with ID SENSOR_CMD_SET_N3D_CONFIG to set native 3D control configuration
 */
struct SensorN3dConfig{
    MUINT32 u4N3dEn;            /*!<N3D module enable control*/
    MUINT32 u4I2C1En;           /*!<N3D 1st I2C control enable control*/
    MUINT32 u4I2C2En;           /*!<N3D 2nd I2C control enable control*/
    MUINT32 u4N3dMode;          /*!<N3D I2C control mode*/
    MUINT32 u4DiffCntEn;        /*!<N3D sensor vsync difference count enable control */
    MUINT32 u4DiffCntThr;       /*!<N3D sensor vsync difference threshold */
} ;

//N3D different count
/**
 *@struct<SENSOR_N3D_DIFFERENCE_COUNT_STRUCT>
 *@brief:structure used in sensor hal provided sendCommand() api with ID SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT to get native 3D vsync difference information
 */
struct SensorN3dDiffCnt{
    MUINT32 u4DiffCnt;          /*!<vsync difference count between vysnc1 & vsync2*/
    MUINT32 u4Vsync1Cnt;        /*!<*vsync1 valid count*/
    MUINT32 u4Vsync2Cnt;        /*!<vsync2 valid count*/
    MUINT32 u4DebugPort;        /*!<vsync2 count number sample at vysnc1*/
    MUINT32 u4Data[10];         /*!<reserve*/
} ;

struct SensorCropWinInfo{
    MUINT16 full_w;
    MUINT16 full_h;
    MUINT16 x0_offset;
    MUINT16 y0_offset;
    MUINT16 w0_size;
    MUINT16 h0_size;
    MUINT16 scale_w;
    MUINT16 scale_h;
    MUINT16 x1_offset;
    MUINT16 y1_offset;
    MUINT16 w1_size;
    MUINT16 h1_size;
    MUINT16 x2_tg_offset;
    MUINT16 y2_tg_offset;
    MUINT16 w2_tg_size;
    MUINT16 h2_tg_size;
} ;
struct SensorVCInfo2{
	MUINT16 VC_Num;
	MUINT16 VC_PixelNum;
	MUINT16 ModeSelect;   /* 0: auto mode, 1:direct mode  */
	MUINT16 EXPO_Ratio;   /* 1/1, 1/2, 1/4, 1/8 */
	MUINT16 ODValue;      /* OD Vaule */
	MUINT16 RG_STATSMODE; /* STATS divistion mdoe 0: 16x16, 1:8x8, 2:4x4, 3:1x1*/
	std::vector<SINGLE_VC_INFO2> vcInfo2s;
};

struct SensorVCInfo{
    MUINT16 VC_Num;
    MUINT16 VC_PixelNum;
    MUINT16 ModeSelect;   /* 0: auto mode, 1:direct mode  */
    MUINT16 EXPO_Ratio;   /* 1/1, 1/2, 1/4, 1/8 */
    MUINT16 ODValue;      /* OD Vaule */
    MUINT16 RG_STATSMODE; /* STATS divistion mdoe 0: 16x16, 1:8x8, 2:4x4, 3:1x1*/
    MUINT16 VC0_ID;
    MUINT16 VC0_DataType;
    MUINT16 VC0_SIZEH;
    MUINT16 VC0_SIZEV;
    MUINT16 VC1_ID;
    MUINT16 VC1_DataType;
    MUINT16 VC1_SIZEH;
    MUINT16 VC1_SIZEV;
    MUINT16 VC2_ID;
    MUINT16 VC2_DataType;
    MUINT16 VC2_SIZEH;
    MUINT16 VC2_SIZEV;
    MUINT16 VC3_ID;
    MUINT16 VC3_DataType;
    MUINT16 VC3_SIZEH;
    MUINT16 VC3_SIZEV;
    MUINT16 VC4_ID;
    MUINT16 VC4_DataType;
    MUINT16 VC4_SIZEH;
    MUINT16 VC4_SIZEV;
    MUINT16 VC5_ID;
    MUINT16 VC5_DataType;
    MUINT16 VC5_SIZEH;
    MUINT16 VC5_SIZEV;
} ;

struct SENSOR_AWB_GAIN
{
    MUINT32 ABS_GAIN_GR;
    MUINT32 ABS_GAIN_R;
    MUINT32 ABS_GAIN_B;
    MUINT32 ABS_GAIN_GB;
};

struct SensorPDAFInfo{
    MUINT32 i4OffsetX;      /* start offset of first PD block */
    MUINT32 i4OffsetY;
    MUINT32 i4PitchX;       /* PD block pitch */
    MUINT32 i4PitchY;
    MUINT32 i4PairNum;      /* PD pair num in one block */
    MUINT32 i4SubBlkW;      /* sub block width (one PD pair in one sub block) */
    MUINT32 i4SubBlkH;      /* sub block height */
    MUINT32 i4PosL[16][2];  /* left pd pixel position in one block*/
    MUINT32 i4PosR[16][2];  /* right pd pixel position in one block*/
    MUINT32 iMirrorFlip;	/* 0:IMAGE_NORMAL,1:IMAGE_H_MIRROR,2:IMAGE_V_MIRROR,3:IMAGE_HV_MIRROR*/
    MUINT32 i4BlockNumX;
    MUINT32 i4BlockNumY;
    MUINT32 i4LeFirst;	    /* 1: 1st line is long exposure, 0: 1st line is short exposure*/
    MUINT32 i4Crop[10][2];  /* [scenario][crop] -> (xcrop, ycrop) */
};


/* for 4-cell Sensor Info */
struct FourCellSensorInfo{
    unsigned short  iNum;
    unsigned char 	*iBuffer;
};


/******************************************************************************
 *  Hal Sensor Interface.
 ******************************************************************************/
class IHalSensor
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    /**
     * @brief Used in configure.
     */
    struct ConfigParam
    {
        MUINT                       index;              //  sensor index
        MSize                       crop;               //  TG crop size in pixels.
        MUINT                       scenarioId;         //  sensor scenario ID.
        MBOOL                       isBypassScenario;
        MBOOL                       isContinuous;
        MUINT                       HDRMode;            //iHDR mode : 0: no HDR, 1: IHDR, 2:MHDR
        MUINT                       framerate;          // Unit : FPS, 10based, 0: will run sensor mode default setting
        MUINT                       twopixelOn;         //Force ture on two pixel mode by user
        MUINT                       debugMode;          // 1: Enable test model (TM) , 2:Enable sensor test pattern(Patern gen)
        MUINT                       exposureTime;
        MUINT                       gain;
        MUINT                       exposureTime_se;
        MUINT                       gain_se;
        MUINT                       exposureTime_me;
        MUINT                       gain_me;
        MUINT                       exposureTime_vse;
        MUINT                       gain_vse;
        MUINT                       isp_pixel_mode;
        MUINT                       isp_tg_num;
        MUINT                       isp_grab_alignment;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Destructor.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IHalSensor() {}

public:     ////                    Instantiation.

    /**
     * Destroy this instance created from IHalSensorList::createSensor.
     */
    virtual MVOID                   destroyInstance(
                                        char const* szCallerName = ""
                                    )                                           = 0;

public:     ////                    Operations.

    /**
     * Turn on/off the power of sensor(s).
     */
    virtual MBOOL                   powerOn(
                                        char  const *szCallerName,
                                        MUINT const  uCountOfIndex,
                                        MUINT const *pArrayOfIndex
                                    )                                           = 0;
    virtual MBOOL                   powerOff(
                                        char  const *szCallerName,
                                        MUINT const  uCountOfIndex,
                                        MUINT const *pArrayOfIndex
                                    )                                           = 0;


    /**
     * Configure the sensor(s).
     */
    virtual MBOOL                   configure(
                                        MUINT const         uCountOfParam,
                                        ConfigParam const*  pConfigParam
                                    )                                           = 0;

    /**
     * Common interface to access sensor for specified command index and sensorDevIdx .It must be used after powerOn
     */
    virtual MINT                    sendCommand(
                                        MUINT    indexDual,
                                        MUINTPTR cmd,
                                        MUINTPTR arg1,
                                        MUINTPTR arg2,
                                        MUINTPTR arg3
                                    )                                           = 0;

	/**
     * Query sensor dynamic information by sensorDevIdx..
      */
    virtual MBOOL                   querySensorDynamicInfo(
                                        MUINT32            indexDual,
                                        SensorDynamicInfo *pSensorDynamicInfo
                                    )                                            = 0;
	/**
     * Sensor set exif debug information.
      */

	virtual MINT32                  setDebugInfo(IBaseCamExif *pIBaseCamExif) = 0;

	virtual MINT32                  reset(){return 0;} /*LukeHu++150410=For ESD-Need To Be Fix*/
};


/******************************************************************************
 *  Hal Sensor List Interface.
 ******************************************************************************/
class IHalSensorList
{
public:     ////                    Instantiation.
    static  IHalSensorList*          get();

protected:  ////                    Destructor.

    /**
     * Disallowed to directly delete a raw pointer.
     */
    virtual                         ~IHalSensorList() {}

public:     ////                    Attributes.
    virtual MUINT                       enableSecure(char const * /*szCallerName*/) {return 0;}
    virtual MUINT                      disableSecure(char const * /*szCallerName*/) {return 0;}

    /**
     * Query the number of image sensors.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryNumberOfSensors() const                = 0;

    /**
     * Query static information for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual IMetadata const&        queryStaticInfo(MUINT const index) const    = 0;

    /**
     * Query the driver name for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual char const*             queryDriverName(MUINT const index) const    = 0;

    /**
     * Query the sensor type of NSSensorType::Type for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryType(MUINT const index) const          = 0;

    /**
     * Query the sensor facing direction for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual MUINT                   queryFacingDirection(
                                        MUINT const index
                                    ) const                                     = 0;

    /**
     * Query SensorDev Index by sensor list index.
     * This call is legal only after searchSensors().
     * Return SENSOR_DEV_MAIN, SENSOR_DEV_SUB,...
     */
	virtual MUINT                   querySensorDevIdx(MUINT const index) const  = 0;

    /**
     * Query static SensorStaticInfo for a specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual SensorStaticInfo const* querySensorStaticInfo(
                                        MUINT const indexDual
                                    ) const                                     = 0;

    /**
     * Query Sensor Information.
     * This call is legal only after searchSensors().
     */
    virtual MVOID                   querySensorStaticInfo(
                                        MUINT indexDual,
                                        SensorStaticInfo *pSensorStaticInfo
                                    ) const                                     = 0;

public:     ////                    Operations.

    /**
     * Search sensors and return the number of image sensors.
     */
    virtual MUINT                   searchSensors()                             = 0;

    /**
     * Create an instance of IHalSensor for a single specific sensor index.
     * This call is legal only after searchSensors().
     */
    virtual IHalSensor*             createSensor(
                                        char const* szCallerName,
                                        MUINT const index
                                    )                                           = 0;

    /**
     * Create an instance of IHalSensor for multiple specific sensor indexes.
     * This call is legal only after searchSensors().
     */
    virtual IHalSensor*             createSensor(
                                        char const* szCallerName,
                                        MUINT const uCountOfIndex,
                                        MUINT const*pArrayOfIndex
                                    )                                           = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCam


/**
 * @brief The definition of the maker of IHalSensorList instance.
 */
typedef NSCam::IHalSensorList* (*HalSensorList_FACTORY_T)();
#define MAKE_HalSensorList(...) \
    MAKE_MTKCAM_MODULE(MTKCAM_MODULE_ID_DRV_HAL_SENSORLIST, HalSensorList_FACTORY_T, __VA_ARGS__)


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_DRV_IHALSENSOR_H_

