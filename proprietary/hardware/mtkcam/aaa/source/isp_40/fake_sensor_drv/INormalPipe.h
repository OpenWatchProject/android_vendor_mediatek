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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_FAKEDRV_INORMALPIPE_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_AAA_FAKEDRV_INORMALPIPE_H_

//#define IOPIPE_SET_MODUL_REG(handle,RegName,Value) HWRWCTL_SET_MODUL_REG(handle,RegName,Value)
//#define IOPIPE_SET_MODUL_REGS(handle, StartRegName, size, ValueArray) HWRWCTL_SET_MODUL_REGS(handle, StartRegName, size, ValueArray)

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

#define MAX_SENSOR_CNT 4

enum ENPipeCmd {
    ENPipeCmd_GET_TG_INDEX          = 0x0800,
    ENPipeCmd_GET_BURSTQNUM         = 0x0801,
    ENPipeCmd_SET_STT_SOF_CB        = 0x0802,
    ENPipeCmd_CLR_STT_SOF_CB        = 0x0803,
    ENPipeCmd_GET_LAST_ENQ_SOF      = 0x0804,
    ENPipeCmd_SET_MEM_CTRL          = 0x0805,
    ENPipeCmd_SET_IDLE_HOLD_CB      = 0x0806,
    ENPipeCmd_GET_STT_CUR_BUF       = 0x0812,

    ENPipeCmd_GET_TG_OUT_SIZE       = 0x110F,
    ENPipeCmd_GET_RMX_OUT_SIZE      = 0x1110,//
    ENPipeCmd_GET_HBIN_INFO         = 0x1111,//
    ENPipeCmd_GET_EIS_INFO          = 0x1112,
    ENPipeCmd_GET_UNI_INFO          = 0x1113,
    ENPipeCmd_GET_BIN_INFO          = 0x1114,
    ENPipeCmd_SET_EIS_CBFP          = 0x1117,
    ENPipeCmd_SET_LCS_CBFP          = 0x1118,
    ENPipeCmd_SET_SGG2_CBFP         = 0X1119,
    ENPipeCmd_GET_PMX_INFO          = 0X111A,
    ENPipeCmd_GET_PBIN_INFO         = 0X111A,
    ENPipeCmd_GET_CUR_FRM_STATUS    = 0x111D,
    ENPipeCmd_GET_CUR_SOF_IDX       = 0x111E,
    ENPIPECmd_AE_SMOOTH             = 0x1105,
    ENPipeCmd_HIGHSPEED_AE          = 0x1106,
    ENPipeCmd_SET_TG_INT_LINE       = 0x110E,

    ENPipeCmd_SET_MODULE_EN         = 0x1401,
    ENPipeCmd_SET_MODULE_SEL        = 0x1402,
    ENPipeCmd_SET_MODULE_CFG        = 0x1403,
    ENPipeCmd_GET_MODULE_HANDLE     = 0x1404,
    ENPipeCmd_SET_MODULE_CFG_DONE   = 0x1405,
    ENPipeCmd_RELEASE_MODULE_HANDLE = 0x1406,
    ENPipeCmd_SET_MODULE_DBG_DUMP   = 0x1407,

    ENPipeCmd_MAX
};

enum ENPipeQueryCmd {
    ENPipeQueryCmd_X_PIX        = 0x0001,
    ENPipeQueryCmd_X_BYTE       = 0x0002,
    ENPipeQueryCmd_CROP_X_PIX   = 0x0004,
    ENPipeQueryCmd_CROP_X_BYTE  = 0x0008,
    ENPipeQueryCmd_CROP_START_X = 0x0010,
    ENPipeQueryCmd_STRIDE_PIX   = 0x0020,
    ENPipeQueryCmd_STRIDE_BYTE  = 0x0040,
    ENPipeQueryCmd_QUERY_FMT  	= 0x0080,
};

/*****************************************************************************
*
* @class INormalPipe
* @brief CamIO Pipe Interface for Normal pipe in frame based mode.
* @details
* The data path will be Sensor --> ISP-- Mem.
*
******************************************************************************/

/******************************************************************************
 *
 ******************************************************************************/
class INormalPipeModule
{
public:     ////                    Operations.

    /**
     * @brief Module ID
     */
    static MUINT32                  moduleId() { return 1; }

    /**
     * @brief Return the singleton of this module.
     */
    static auto                     get()
                                    {
                                            static INormalPipeModule singleton; return (INormalPipeModule*)&singleton;
                                    }

public:     ////                    Operations.
                                   ~INormalPipeModule() {}
                                    INormalPipeModule()
                                    {
                                    }

    int                             get_sub_module_api_version(uint32_t const** versions, size_t* count, int index) { static MUINT32 mVersion = 1; *versions = &mVersion; *count = 1; index = 1; return 0;}

public:     ////                    Operations.

    /**
     * @brief Create a sub-module instance.
     * @param[in] sensorIndex:  The sensor index.
     * @param[in] szCallerName: The caller name.
     * @param[in] apiVersion:   The sub-module API version.
     * @param[out] rpInstance:  The created sub-module instance. Callers have to
     *                          cast it to the real type based on the specified
     *                          sub-module api version.
     * @details
     * @note
     * @return an error code (0 == success; otherwise failure)
     */
    MERROR                  createSubModule(
                                        MUINT32 sensorIndex,
                                        char const* szCallerName,
                                        MUINT32 apiVersion,
                                        MVOID** ppInstance
                                    );

    /**
     * @brief Query pipe capability.
     * @param[in] portIdx:  Refer to PortMap.h, 'PortID::index' field
     * @param[in] eCmd:     width/stride pixel/byte crop constraint to query. Cmds are bitmap, plz refer to enum:ENPipeQueryCmd
     * @param[in] imgFmt:   EImageFormat in hardware/include/mtkcam/ImageFormat.h
     * @param[in] input:    input query information
     * @param[out] queryInfo: result
     * @details
     * @note
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    // virtual MBOOL                   query(
    //                                     MUINT32 portIdx,
    //                                     MUINT32 eCmd,
    //                                     MINT imgFmt,
    //                                     NormalPipe_QueryIn const& input,
    //                                     NormalPipe_QueryInfo &queryInfo
    //                                 )   const                               = 0;
};

class INormalPipe
{
public:     ////                    Instantiation.
    static  INormalPipe     *createInstance(MUINT32 sensorIndex,char const* szCallerName);
    virtual                 ~INormalPipe(){};

public:     ////                    Attributes.
    virtual MVOID   destroyInstance(char const* szCallerName) = 0;

    virtual MBOOL   init() = 0;

    virtual MBOOL   uninit() = 0;

    virtual MINT32  attach(const char* UserName) = 0;

    virtual MBOOL   wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                            MUINT32 TimeoutMs = 0xFFFFFFFF, SignalTimeInfo *pTime = NULL) = 0;

    virtual MBOOL   signal(EPipeSignal eSignal, const MINT32 mUserKey) = 0;

    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3) = 0;

    virtual MUINT32 getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical = MTRUE) = 0;
    virtual MUINT32 getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical = MTRUE) = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCamIO
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_INORMALSTREAM_H_

