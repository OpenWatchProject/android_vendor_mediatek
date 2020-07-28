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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_CAMIO_INORMALPIPE_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_CAMIO_INORMALPIPE_H_

#include <utils/Mutex.h>    // For android::Mutex.
#include <mtkcam/utils/module/module.h>
#include "IHalCamIO.h"


/******************************************************************************
 *
 ******************************************************************************/
using namespace std;
namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

typedef enum {
    eCamHwPathCfg_One_TG = 0,
    eCamHwPathCfg_Two_TG,
    eCamHwPathCfg_Num,
} E_CamHwPathCfg;

struct NormalPipe_QueryIn {
    MUINT32             width;      //unit:pix
    E_CamPixelMode      pixMode;

    NormalPipe_QueryIn(
        MUINT32 _width = 0,
        E_CamPixelMode _pixMode = ePixMode_NONE
        )
        :width(_width)
        ,pixMode(_pixMode)
    {}
};

struct NormalPipe_InputInfo {
    EImageFormat        format;
    MUINT32             width;
    MUINT32             height;
    E_CamPixelMode      pixelMode;
    MUINT32             sensorIdx;
    MUINT32             scenarioId;
    MUINT32             rrz_out_w;
    E_CamPattern        pattern;
    MBOOL               bin_off;
    MBOOL               SecOn;

    NormalPipe_InputInfo(
        EImageFormat    _format = eImgFmt_IMPLEMENTATION_DEFINED,
        MUINT32         _width = 0,
        E_CamPixelMode  _pixelMode = ePixMode_NONE
        )
        : format(_format)
        , width(_width)
        , height(0)
        , pixelMode(_pixelMode)
        , sensorIdx(0)
        , scenarioId(0)
        , rrz_out_w(0)
        , pattern(eCAM_NORMAL)
        , bin_off(MFALSE)
        , SecOn(MFALSE){}
    NormalPipe_InputInfo(
        MUINT32         _width,
        MUINT32         _height
        ){
        NormalPipe_InputInfo();
        width = _width;
        height = _height;
    }
};


typedef union
{
        struct
        {
                MUINT32 D_TWIN          : 1;    // 1: dynamic twin is ON, 0: dynamic twin is OFF.
                MUINT32 SensorNum       : 2;    // max sensor number for direct-link
                MUINT32 D_BayerENC      : 1;    // 1: dynamic bayer encorder(perframe ctrl) , 0: static bayer encoder.
                MUINT32 isFLK           : 1;    // 1:FLK is supported at current sensor
                MUINT32 isLMV           : 1;    // 1:LMV is supported at current sensor
                MUINT32 isRSS           : 1;    // 1:RSS is supported at current sensor
                MUINT32 isPDE           : 1;    // 1:PDE is supported at current sensor
                MUINT32 isFull_dir_YUV  : 1;    // 1:Fullsize direct YUV is supported at current sensor
                MUINT32 isScaled_Y      : 1;    // 1:Scaled direct Y is supported at current sensor
                MUINT32 isScaled_YUV    : 1;    // 1:Scaled direct YUV is supported at current sensor
                MUINT32 isG_Scaled_YUV  : 1;    // 1:IND. Gamma Scaled direct YUV is supported at current sensor
                MUINT32 isRAWI          : 3;    // plz reference to enum:E_CamRawiPath
                MUINT32 needCfgSensorByMW : 1;    // 1:Need P1Node config sensor. 0:Legacy flow config sensor by Normalpipe
                MUINT32 rsv             : 16;
        } Bits = {0};
        MUINT32 Raw;
}Que_Func;

struct NormalPipe_QueryInfo {
    MUINT32 x_pix;                  //horizontal resolution, unit:pix
    MUINT32 stride_pix;             //stride, uint:pix. this is a approximative value under pak mode
    MUINT32 stride_byte;            //stride, uint:byte
    MUINT32 stride_B[3];            //3-plane stride. unit:byte
    MUINT32 xsize_byte;
    MUINT32 crop_x;                        //crop start point-x , unit:pix
    vector<NSCam::EImageFormat> query_fmt; //query support fmt
    MUINT32 bs_ratio;                      //bayer scaler scaling ratio, unit:%
    MUINT32 pipelinebitdepth;              //how many bits pipeline deal with
    MBOOL   D_TWIN;                        // 1: dynamic twin is ON, 0: dynamic twin is OFF.
    MBOOL   IQlv;
    MUINT32 sen_num;
    Que_Func function;
    MUINT32 uni_num;                //the number of uni
    MBOOL   D_Pak;                  // 1: support dynamic pak  0: no support dynamic pak.
    MUINT32 burstNum;               //support burst number
    MUINT32 pattern ;               //support pattern
    MUINT32 IQ_SW_Ver;              //IQ switch version. 0:not support,1:isp50,2:isp60
    NormalPipe_QueryInfo (
                MUINT32 _x_pix = 0,
                MUINT32 _stride_pix = 0,
                MUINT32 _stride_byte = 0,
                MUINT32 _xsize_byte = 0,
                MUINT32 _crop_x = 0,
                MUINT32 _bs_ratio = 100,
                MUINT32 _pipelinebitdepth = CAM_Pipeline_12BITS
            )
        : x_pix(_x_pix)
        , stride_pix(_stride_pix)
        , stride_byte(_stride_byte)
        , xsize_byte(_xsize_byte)
        , crop_x(_crop_x)
        , bs_ratio(_bs_ratio)
        , pipelinebitdepth(_pipelinebitdepth)
        , D_TWIN(0)
        , IQlv(0)
        , sen_num(2)
        , uni_num(1)
        , D_Pak(MFALSE)
        , burstNum(0)
        , pattern(0)
    {
        stride_B[0] = stride_B[1] = stride_B[2] = 0;
        query_fmt.clear();
        D_TWIN = 0;
        sen_num = 2;
        function.Raw = 0;
        function.Bits.D_TWIN = D_TWIN;
        function.Bits.SensorNum = sen_num;
        function.Bits.D_BayerENC = 0;
        IQ_SW_Ver = 0;
        function.Bits.needCfgSensorByMW = 0;
    }
};

typedef struct QueryInData_t
{
    MUINT32        sensorIdx;
    MUINT32        scenarioId;
    MUINT32        rrz_out_w;
    MUINT32        minCamProcessedFps;
    E_CamPattern   pattern;
    E_CamIQLevel   IQlv;

    QueryInData_t()
    :sensorIdx(0)
    ,scenarioId(0)
    ,rrz_out_w(0)
    ,minCamProcessedFps(0)
    ,pattern(eCAM_NORMAL)
    ,IQlv(eCamIQ_MAX){}
};

typedef struct QueryOutData_t
{
    MUINT32        sensorIdx;
    MBOOL          isTwin;
    E_CamIQLevel   IQlv;
    MUINT32        clklv;
    MBOOL          result;
    Que_Func       function;
    E_INPUT        targetTg;
    CAM_RESCONFIG  camResConfig;

    QueryOutData_t()
    :sensorIdx(0)
    ,isTwin(MFALSE)
    ,IQlv(eCamIQ_MAX)
    ,clklv(0)
    ,result(MFALSE)
    ,targetTg(TG_CAM_MAX)
    {
    }
};

enum EPipeSelect{
    EPipeSelect_None                    = 0x00000000,
    EPipeSelect_Normal                  = 0x00000001,
    EPipeSelect_NormalSv                = 0x00000010,
};

typedef struct SEN_INFO{
    MUINT32        sensorIdx;
    MUINT32        scenarioId;
    MUINT32        rrz_out_w;
    E_CamPattern   pattern;
    MBOOL          bin_off;    // force bin off
    MBOOL          stt_off;    // no demand for output statistic data

    SEN_INFO()
    :sensorIdx(0)
    ,scenarioId(0)
    ,rrz_out_w(0)
    ,pattern(eCAM_NORMAL)
    ,bin_off(MFALSE)
    ,stt_off(MFALSE){}
};

typedef struct PIPE_SEL{
    MUINT32        sensorIdx;
    MUINT32        pipeSel;

    PIPE_SEL()
    :sensorIdx(0)
    ,pipeSel(EPipeSelect_None){}
};

typedef struct QueryInResAlloc_t
{
    MUINT32        sensorIdx;
    MUINT32        scenarioId;
    MUINT32        maxCamProcessedOutW; // rrz_out_w as-is
    MUINT32        minCamProcessedFps;  // describes minimum fps processing capability of CAM pass1, 30:preview/dcif 480:smvr
    E_CamPattern   pattern;
    E_CamIQLevel   IQlv;

    QueryInResAlloc_t()
    :sensorIdx(0)
    ,scenarioId(0)
    ,maxCamProcessedOutW(0)
    ,minCamProcessedFps(0)
    ,pattern(eCAM_NORMAL)
    ,IQlv(eCamIQ_MAX){}
};

typedef struct QueryOutResAlloc_t
{
    MUINT32         sensorIdx;
    MUINT32         clklv;
    MBOOL           needFullFpsSVRawDump; // if true, direct couple path is needed
    EPipeSelect     pipeSel;
    QueryOutResAlloc_t()
    :sensorIdx(0)
    ,clklv(0)
    ,needFullFpsSVRawDump(MFALSE)
    ,pipeSel(EPipeSelect_None){}
};


typedef enum {
    TUNING_FREQ_ALIGN_RAW_FPS = 0x01, //isp4.x, isp5.x
    TUNING_FREQ_ALIGN_ENQ_REQ = 0x02  //isp6.0
} QUERY_TUNING_FREQ;

typedef struct {
   MBOOL  mSupported;

} NormalPipe_EIS_Info;

typedef struct {
   MBOOL  mSupported;

} NormalPipe_RSS_Info;

typedef struct {
    MBOOL  mEnabled;
    MSize  size;

} NormalPipe_HBIN_Info;

typedef enum {
    ENPipe_UNKNOWN  = 0,
    ENPipe_CAM_A    = 1,
    ENPipe_CAM_B    = 2,
    ENPipe_CAM_C    = 3,

    ENPipe_CAM_MAX  = 4,    //number of above enum

}ENPipe_CAM;

typedef struct{
    ENPipe_CAM          Pipe_path;
    std::vector<MRect>  v_rect;     //if path == (ENPipe_CAM_A|ENPipe_CAM_B) =>vector at 0 map to CAM_A's crop, vector at 1 map to CAM_B's crop
}NormalPipe_CROP_INFO;

typedef std::vector<NormalPipe_CROP_INFO>    V_NormalPipe_CROP_INFO;

typedef struct{
    ENPipe_CAM          Pipe_path;
    MUINT32             regAddr;
}NormalPipe_XXXReg;

typedef std::vector<NormalPipe_XXXReg> V_NormalPipe_TwinReg;
typedef std::vector<NormalPipe_XXXReg> V_NormalPipe_MagReg;

/**
    enum for current frame status.
    mapping to kernel status
*/
typedef enum {
    _normal_status          = 0,
    _drop_frame_status      = 1,
    _last_working_frame     = 2,
    _1st_enqloop_status     = 3,
} NormalPipe_FRM_STATUS;


enum ENPipeCmd {
    ENPipeCmd_GET_TG_INDEX          = 0x0800,
    ENPipeCmd_GET_BURSTQNUM         = 0x0801,
    ENPipeCmd_SET_STT_SOF_CB        = 0x0802,
    ENPipeCmd_CLR_STT_SOF_CB        = 0x0803,
    ENPipeCmd_GET_LAST_ENQ_SOF      = 0x0804,
    ENPipeCmd_SET_MEM_CTRL          = 0x0805,
    ENPipeCmd_SET_IDLE_HOLD_CB      = 0x0806,
    ENPipeCmd_SET_SUSPEND_STAT_CB   = 0x0807,
    ENPipeCmd_GET_STT_CUR_BUF       = 0x0812,
    ENPipeCmd_SET_STT_SUSPEND_CB    = 0x0813,
    ENPipeCmd_SET_HW_PATH_CFG       = 0x0814,
    ENPipeCmd_GET_HW_PATH_CFG       = 0x0815,
    ENPipeCmd_SET_ENQ_THRD_CB       = 0x0816,
    ENPipeCmd_SET_STT_DSWITCH_CB    = 0x0817,
    ENPipeCmd_CLR_STT_DSWITCH_CB    = 0x0818,

    ENPIPECmd_AE_SMOOTH             = 0x1105,
    ENPipeCmd_HIGHSPEED_AE          = 0x1106,
    ENPipeCmd_SET_TG_INT_LINE       = 0x110E,
    ENPipeCmd_GET_TG_OUT_SIZE       = 0x110F,
    ENPipeCmd_GET_RMX_OUT_SIZE      = 0x1110,//
    ENPipeCmd_GET_HBIN_INFO         = 0x1111,//
    ENPipeCmd_GET_EIS_INFO          = 0x1112,
    ENPipeCmd_GET_UNI_INFO          = 0x1113,
    ENPipeCmd_GET_BIN_INFO          = 0x1114,
    ENPipeCmd_GET_RSS_INFO          = 0x1115,
    ENPipeCmd_SET_EIS_CBFP          = 0x1117,
    ENPipeCmd_SET_LCS_CBFP          = 0x1118,
    ENPipeCmd_SET_SGG2_CBFP         = 0X1119,//ISP4/5 for SGG2;ISP6 for GSE
    ENPipeCmd_SET_RSS_CBFP          = 0X111A,
    ENPipeCmd_GET_PMX_INFO          = 0X111B,
    ENPipeCmd_GET_IMGO_INFO         = 0x111C,
    ENPipeCmd_GET_CUR_FRM_STATUS    = 0x111D,
    ENPipeCmd_GET_CUR_SOF_IDX       = 0x111E,
    ENPipeCmd_GET_RCP_SIZE          = 0x111F,// arg1's data type: V_NormalPipe_CROP_INFO
    ENPipeCmd_UNI_SWITCH            = 0x1120,
    ENPipeCmd_GET_UNI_SWITCH_STATE  = 0x1121,
    ENPipeCmd_GET_MAGIC_REG_ADDR    = 0x1122,// arg1: output the register address of magic number. data type: V_NormalPipe_MagReg.
    ENPipeCmd_GET_DTwin_INFO        = 0x1123,//arg1: output dynamic twin is turned ON/OFF , only BiXXco/ViXXon/CaXXon support.
    ENPipeCmd_GET_TWIN_REG_ADDR     = 0x1124,// arg1: output the register address of twin status. data type: V_NormalPipe_TwinReg.
    ENPipeCmd_SET_FAST_AF           = 0x1127,
    ENPipeCmd_SET_RRZ_CBFP          = 0x1128,
    ENPipeCmd_SET_TUNING_CBFP       = 0x1129,
    ENPipeCmd_SET_WT_TUNING_CBFP    = 0x112A,// W+T Tuning CB for change sensor.
    ENPipeCmd_GET_CUR_RAW           = 0x112B,//arg1: output current master cam path, type:ENPipe_CAM
    ENPipeCmd_SET_REGDUMP_CBFP      = 0x1130,
    ENPipeCmd_SET_PHYREGDUMP_BFCB   = 0x1131,//arg1: input buffer . type:P1_TUNING_NOTIFY*
    ENPipeCmd_GET_FLK_INFO          = 0x113A,
    ENPipeCmd_SET_QUALITY           = 0x113B,
    ENPipeCmd_GET_QUALITY           = 0x113C,/*configpipe result is also use this cmd to get first IQ value*/
    ENPipeCmd_GET_RMB_INFO          = 0x113D,
    ENPipeCmd_SET_AWB_CBFP          = 0x113E,
    ENPipeCmd_GET_PIPE_BIT_MODE     = 0x113F,
    ENPipeCmd_SET_CRZO_R1_CBFP      = 0x1140, // DYUV related cmd
    ENPipeCmd_SET_CRZO_R2_CBFP      = 0x1141,
    ENPipeCmd_SET_CQSTART_CBFP      = 0x1142,
    ENPipeCmd_SET_3A_HAL_CBFP       = 0x1143,
    ENPipeCmd_SET_YUVO_R1_CBFP      = 0x1144, // DYUV related cmd
    ENPipeCmd_SET_RSSO_R2_CBFP      = 0x1145,


    //cmd for p1hwcfg, only isp3.0 support
    ENPipeCmd_SET_MODULE_EN         = 0x1401,
    ENPipeCmd_SET_MODULE_SEL        = 0x1402,
    ENPipeCmd_SET_MODULE_CFG        = 0x1403,
    ENPipeCmd_GET_MODULE_HANDLE     = 0x1404,
    ENPipeCmd_SET_MODULE_CFG_DONE   = 0x1405,
    ENPipeCmd_RELEASE_MODULE_HANDLE = 0x1406,
    ENPipeCmd_SET_MODULE_DBG_DUMP   = 0x1407,
    //cmd for AF special HBIN1 request
    ENPipeCmd_GET_HBIN1_INFO        = 0x1408,

    ENPipeCmd_MAX
};

enum ENPipeQueryCmd {
    ENPipeQueryCmd_UNKNOWN               = 0x00000000,
    ENPipeQueryCmd_X_PIX                 = 0x00000001,
    ENPipeQueryCmd_X_BYTE                = 0x00000002,
    ENPipeQueryCmd_CROP_X_PIX            = 0x00000004,   //query when using cropping, (cropping size != input size)
    ENPipeQueryCmd_CROP_X_BYTE           = 0x00000008,   //query when using cropping, (cropping size != input size)
    //
    ENPipeQueryCmd_CROP_START_X          = 0x00000010,   //query when using cropping, the unit of crop start x is pixel.
    ENPipeQueryCmd_STRIDE_PIX            = 0x00000020,
    ENPipeQueryCmd_STRIDE_BYTE           = 0x00000040,
    //
    ENPipeQueryCmd_MAX_SEN_NUM           = 0x00000100,
    //
    ENPipeQueryCmd_PIPELINE_BITDEPTH     = 0x00000200,
    ENPipeQueryCmd_IQ_LEVEL              = 0x00000400,
    ENPipeQueryCmd_ISP_RES               = 0x00000800,
    //
    ENPipeQueryCmd_BURST_NUM             = 0x00001000,
    ENPipeQueryCmd_SUPPORT_PATTERN       = 0x00002000,
    ENPipeQueryCmd_MAX_PREVIEW_SIZE      = 0x00004000,
    //
    ENPipeQueryCmd_QUERY_FMT             = 0x10000000,
    ENPipeQueryCmd_BS_RATIO              = 0x20000000,   //bayer scaler max scaling ratio,unit:%
    ENPipeQueryCmd_D_Twin                = 0x40000000,   //query dynamic twin is supported or not
    ENPipeQueryCmd_D_BayerEnc            = 0x80000000,   //query dynamic Bayer encoder is supported or not
    //
    ENPipeQueryCmd_UNI_NUM               = 0x01000000,   //query numbers of UNI(ex.EIS/FLK..)
    ENPipeQueryCmd_DYNAMIC_PAK           = 0x02000000,   //query dynamic pak is supported or not
    ENPipeQueryCmd_RSV                   = 0x04000000,
    ENPipeQueryCmd_IQ_SW_VER             = 0x80000000,   //query IQ change version.(0:not support,1:isp50,2:isp60)
    //
    ENPipeQueryCmd_HW_RES_MGR            = 0x00100000,   //hardware resource management, query which kind of pipeline to use, cam?, camsv?, dcif?, or stagger?
    ENPipeQueryCmd_PDO_AVAILABLE         = 0x00200000,   //query whether platform supports pdo or not
    ENPipeQueryCmd_HW_RES_ALLOC          = 0x00400000,
    ENPipeQueryCmd_TUNING_FREQ           = 0x00800000,
    //
    ENPipeQueryCmd_SEC_CAP               = 0x00010000,   //query secure camera constraint
    ENPipeQueryCmd_NEED_CFG_SENSOR_BY_MW = 0x00020000,   //query if need config sensor by P1Node instead by Normalpipe

    //query hw-function is supported or not at current sensor.(ref to struct:Que_Func)
    ENPipeQueryCmd_FUNC                 =(ENPipeQueryCmd_RSV|ENPipeQueryCmd_MAX_SEN_NUM|ENPipeQueryCmd_D_Twin|ENPipeQueryCmd_D_BayerEnc|ENPipeQueryCmd_NEED_CFG_SENSOR_BY_MW),
};


/******************************************************************************
 *
 ******************************************************************************/
class INormalPipeModule : public mtkcam_module
{
public:     ////                    Operations.

    /**
     * @brief Module ID
     */
    static MUINT32                  moduleId() { return MTKCAM_MODULE_ID_DRV_IOPIPE_CAMIO_NORMALPIPE; }

    /**
     * @brief Return the singleton of this module.
     */
    static auto                     get()
                                    {
                                        return (INormalPipeModule*)GET_MTKCAM_MODULE_EXTENSION(moduleId());
                                    }

public:     ////                    Operations.
    virtual                         ~INormalPipeModule() {}
                                    INormalPipeModule()
                                    {
                                        get_module_api_version      = NULL;
                                        get_module_id               = moduleId;
                                        get_module_extension        = NULL;
                                        get_sub_module_api_version  = NULL;
                                    }

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
    virtual MERROR                  createSubModule(
                                        MUINT32 sensorIndex,
                                        char const* szCallerName,
                                        MUINT32 apiVersion,
                                        MVOID** ppInstance,
                                        MUINT32 pipeSelect = EPipeSelect_Normal
                                    )                                       = 0;

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
    virtual MBOOL                   query(
                                        MUINT32 portIdx,
                                        MUINT32 eCmd,
                                        MINT imgFmt,
                                        NormalPipe_QueryIn const& input,
                                        NormalPipe_QueryInfo &queryInfo
                                    )   const                               = 0;
	virtual MBOOL                   query(
                                        MUINT32 eCmd,
                                        MUINTPTR IO_struct
                                    )   const {eCmd;IO_struct;return MFALSE;}

    /* query LMV Capability by TG.
     * @param[in] inputTG: the LMV capability of RAW which user wants to query.
     * @param[out] bSupportHWLMV: Real LMV HW exists or not.
     * @param[out] bSupportVirtualLMV: support virtual LMV or not.
     * @return
     * - MTRUE: query success; MFALSE: query fail.
     */
    virtual MBOOL queryLMVSupported(
                    E_INPUT inputTG,
                    MBOOL &bSupportHWLMV,
                    MBOOL &bSupportVirtualLMV
                  ) const {inputTG; bSupportHWLMV = 1; bSupportVirtualLMV = 0; return MFALSE; };

};



/*****************************************************************************
*
* @class INormalPipe
* @brief CamIO Pipe Interface for Normal pipe in frame based mode.
* @details
* The data path will be Sensor --> ISP-- Mem.
*
******************************************************************************/
class INormalPipe : public IHalCamIO
{
public:     ////                    Attributes.

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    static  char const*             pipeName() { return "CamIO::NormalPipe"; }

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    virtual char const*             getPipeName() const { return pipeName(); }
    virtual MBOOL get_isExecutable(NormalPipe_InputInfo const ResDataIn, MBOOL &bIsExe) { return MFALSE; }
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSCamIOPipe
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_INORMALSTREAM_H_

