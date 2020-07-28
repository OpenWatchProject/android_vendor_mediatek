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
 TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "AeFlowCCU"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG (1)
#endif

#include <stdlib.h>
#include "AeFlowCCU.h"
#include <mtkcam/utils/std/Log.h>
#include <cutils/properties.h>
#include <ae_param.h>
#include <aaa_error_code.h>
#include <isp_tuning_mgr.h>
#include <aaa_scheduler.h>
#include "n3d_sync2a_tuning_param.h"
#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

#define AE_FLOW_LOG(fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE(fmt, ##arg); \
        } else { \
            CAM_LOGD(fmt, ##arg); \
        } \
    }while(0)
#define AE_FLOW_LOG_IF(cond, fmt, arg...) \
    do { \
        if (0) { \
            CAM_LOGE_IF(cond, fmt, ##arg); \
        } else { \
            CAM_LOGD_IF(cond, fmt, ##arg); \
        } \
    }while(0)

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCcuIf;

MUINT32 Log2x1000[LumLog2x1000_TABLE_SIZE] = {
       0,    0, 1000, 1585, 2000, 2322, 2585, 2807,
    3000, 3170, 3322, 3459, 3585, 3700, 3807, 3907,
    4000, 4087, 4170, 4248, 4322, 4392, 4459, 4524,
    4585, 4644, 4700, 4755, 4807, 4858, 4907, 4954,
    5000, 5044, 5087, 5129, 5170, 5209, 5248, 5285,
    5322, 5358, 5392, 5426, 5459, 5492, 5524, 5555,
    5585, 5615, 5644, 5672, 5700, 5728, 5755, 5781,
    5807, 5833, 5858, 5883, 5907, 5931, 5954, 5977,
    6000, 6022, 6044, 6066, 6087, 6109, 6129, 6150,
    6170, 6190, 6209, 6229, 6248, 6267, 6285, 6304,
    6322, 6340, 6358, 6375, 6392, 6409, 6426, 6443,
    6459, 6476, 6492, 6508, 6524, 6539, 6555, 6570,
    6585, 6600, 6615, 6629, 6644, 6658, 6672, 6687,
    6700, 6714, 6728, 6741, 6755, 6768, 6781, 6794,
    6807, 6820, 6833, 6845, 6858, 6870, 6883, 6895,
    6907, 6919, 6931, 6943, 6954, 6966, 6977, 6989,
    7000, 7011, 7022, 7033, 7044, 7055, 7066, 7077,
    7087, 7098, 7109, 7119, 7129, 7140, 7150, 7160,
    7170, 7180, 7190, 7200, 7209, 7219, 7229, 7238,
    7248, 7257, 7267, 7276, 7285, 7295, 7304, 7313,
    7322, 7331, 7340, 7349, 7358, 7366, 7375, 7384,
    7392, 7401, 7409, 7418, 7426, 7435, 7443, 7451,
    7459, 7468, 7476, 7484, 7492, 7500, 7508, 7516,
    7524, 7531, 7539, 7547, 7555, 7562, 7570, 7577,
    7585, 7592, 7600, 7607, 7615, 7622, 7629, 7637,
    7644, 7651, 7658, 7665, 7672, 7679, 7687, 7693,
    7700, 7707, 7714, 7721, 7728, 7735, 7741, 7748,
    7755, 7762, 7768, 7775, 7781, 7788, 7794, 7801,
    7807, 7814, 7820, 7827, 7833, 7839, 7845, 7852,
    7858, 7864, 7870, 7877, 7883, 7889, 7895, 7901,
    7907, 7913, 7919, 7925, 7931, 7937, 7943, 7948,
    7954, 7960, 7966, 7972, 7977, 7983, 7989, 7994,
    8000, 8006, 8011, 8017, 8022, 8028, 8033, 8039,
    8044, 8050, 8055, 8061, 8066, 8071, 8077, 8082,
    8087, 8093, 8098, 8103, 8109, 8114, 8119, 8124,
    8129, 8134, 8140, 8145, 8150, 8155, 8160, 8165,
    8170, 8175, 8180, 8185, 8190, 8195, 8200, 8205,
    8209, 8214, 8219, 8224, 8229, 8234, 8238, 8243,
    8248, 8253, 8257, 8262, 8267, 8271, 8276, 8281,
    8285, 8290, 8295, 8299, 8304, 8308, 8313, 8317,
    8322, 8326, 8331, 8335, 8340, 8344, 8349, 8353,
    8358, 8362, 8366, 8371, 8375, 8379, 8384, 8388,
    8392, 8397, 8401, 8405, 8409, 8414, 8418, 8422,
    8426, 8430, 8435, 8439, 8443, 8447, 8451, 8455,
    8459, 8464, 8468, 8472, 8476, 8480, 8484, 8488,
    8492, 8496, 8500, 8504, 8508, 8512, 8516, 8520,
    8524, 8527, 8531, 8535, 8539, 8543, 8547, 8551,
    8555, 8558, 8562, 8566, 8570, 8574, 8577, 8581,
    8585, 8589, 8592, 8596, 8600, 8604, 8607, 8611,
    8615, 8618, 8622, 8626, 8629, 8633, 8637, 8640,
    8644, 8647, 8651, 8655, 8658, 8662, 8665, 8669,
    8672, 8676, 8679, 8683, 8687, 8690, 8693, 8697,
    8700, 8704, 8707, 8711, 8714, 8718, 8721, 8725,
    8728, 8731, 8735, 8738, 8741, 8745, 8748, 8752,
    8755, 8758, 8762, 8765, 8768, 8771, 8775, 8778,
    8781, 8785, 8788, 8791, 8794, 8798, 8801, 8804,
    8807, 8811, 8814, 8817, 8820, 8823, 8827, 8830,
    8833, 8836, 8839, 8842, 8845, 8849, 8852, 8855,
    8858, 8861, 8864, 8867, 8870, 8873, 8877, 8880,
    8883, 8886, 8889, 8892, 8895, 8898, 8901, 8904,
    8907, 8910, 8913, 8916, 8919, 8922, 8925, 8928,
    8931, 8934, 8937, 8940, 8943, 8945, 8948, 8951,
    8954, 8957, 8960, 8963, 8966, 8969, 8972, 8974,
    8977, 8980, 8983, 8986, 8989, 8992, 8994, 8997,
    9000, 9003, 9006, 9008, 9011, 9014, 9017, 9020,
    9022, 9025, 9028, 9031, 9033, 9036, 9039, 9042,
    9044, 9047, 9050, 9053, 9055, 9058, 9061, 9063,
    9066, 9069, 9071, 9074, 9077, 9079, 9082, 9085,
    9087, 9090, 9093, 9095, 9098, 9101, 9103, 9106,
    9109, 9111, 9114, 9116, 9119, 9122, 9124, 9127,
    9129, 9132, 9134, 9137, 9140, 9142, 9145, 9147,
    9150, 9152, 9155, 9157, 9160, 9162, 9165, 9167,
    9170, 9172, 9175, 9177, 9180, 9182, 9185, 9187,
    9190, 9192, 9195, 9197, 9200, 9202, 9205, 9207,
    9209, 9212, 9214, 9217, 9219, 9222, 9224, 9226,
    9229, 9231, 9234, 9236, 9238, 9241, 9243, 9246,
    9248, 9250, 9253, 9255, 9257, 9260, 9262, 9264,
    9267, 9269, 9271, 9274, 9276, 9278, 9281, 9283,
    9285, 9288, 9290, 9292, 9295, 9297, 9299, 9301,
    9304, 9306, 9308, 9311, 9313, 9315, 9317, 9320,
    9322, 9324, 9326, 9329, 9331, 9333, 9335, 9338,
    9340, 9342, 9344, 9347, 9349, 9351, 9353, 9355,
    9358, 9360, 9362, 9364, 9366, 9369, 9371, 9373,
    9375, 9377, 9379, 9382, 9384, 9386, 9388, 9390,
    9392, 9394, 9397, 9399, 9401, 9403, 9405, 9407,
    9409, 9412, 9414, 9416, 9418, 9420, 9422, 9424,
    9426, 9428, 9430, 9433, 9435, 9437, 9439, 9441,
    9443, 9445, 9447, 9449, 9451, 9453, 9455, 9457,
    9459, 9461, 9464, 9466, 9468, 9470, 9472, 9474,
    9476, 9478, 9480, 9482, 9484, 9486, 9488, 9490,
    9492, 9494, 9496, 9498, 9500, 9502, 9504, 9506,
    9508, 9510, 9512, 9514, 9516, 9518, 9520, 9522,
    9524, 9526, 9527, 9529, 9531, 9533, 9535, 9537,
    9539, 9541, 9543, 9545, 9547, 9549, 9551, 9553,
    9555, 9557, 9558, 9560, 9562, 9564, 9566, 9568,
    9570, 9572, 9574, 9576, 9577, 9579, 9581, 9583,
    9585, 9587, 9589, 9591, 9592, 9594, 9596, 9598,
    9600, 9602, 9604, 9605, 9607, 9609, 9611, 9613,
    9615, 9617, 9618, 9620, 9622, 9624, 9626, 9628,
    9629, 9631, 9633, 9635, 9637, 9638, 9640, 9642,
    9644, 9646, 9647, 9649, 9651, 9653, 9655, 9656,
    9658, 9660, 9662, 9664, 9665, 9667, 9669, 9671,
    9672, 9674, 9676, 9678, 9679, 9681, 9683, 9685,
    9687, 9688, 9690, 9692, 9693, 9695, 9697, 9699,
    9700, 9702, 9704, 9706, 9707, 9709, 9711, 9713,
    9714, 9716, 9718, 9719, 9721, 9723, 9725, 9726,
    9728, 9730, 9731, 9733, 9735, 9736, 9738, 9740,
    9741, 9743, 9745, 9747, 9748, 9750, 9752, 9753,
    9755, 9757, 9758, 9760, 9762, 9763, 9765, 9767,
    9768, 9770, 9771, 9773, 9775, 9776, 9778, 9780,
    9781, 9783, 9785, 9786, 9788, 9790, 9791, 9793,
    9794, 9796, 9798, 9799, 9801, 9803, 9804, 9806,
    9807, 9809, 9811, 9812, 9814, 9815, 9817, 9819,
    9820, 9822, 9823, 9825, 9827, 9828, 9830, 9831,
    9833, 9834, 9836, 9838, 9839, 9841, 9842, 9844,
    9845, 9847, 9849, 9850, 9852, 9853, 9855, 9856,
    9858, 9860, 9861, 9863, 9864, 9866, 9867, 9869,
    9870, 9872, 9873, 9875, 9877, 9878, 9880, 9881,
    9883, 9884, 9886, 9887, 9889, 9890, 9892, 9893,
    9895, 9896, 9898, 9899, 9901, 9902, 9904, 9905,
    9907, 9908, 9910, 9911, 9913, 9914, 9916, 9917,
    9919, 9920, 9922, 9923, 9925, 9926, 9928, 9929,
    9931, 9932, 9934, 9935, 9937, 9938, 9940, 9941,
    9943, 9944, 9945, 9947, 9948, 9950, 9951, 9953,
    9954, 9956, 9957, 9959, 9960, 9961, 9963, 9964,
    9966, 9967, 9969, 9970, 9972, 9973, 9974, 9976,
    9977, 9979, 9980, 9982, 9983, 9984, 9986, 9987,
    9989, 9990, 9992, 9993, 9994, 9996, 9997, 9999
};

static std::array<MY_INST_AEFLOW, SENSOR_IDX_MAX> gMultiton;

IAeFlow*
AeFlowCCU::
getInstance(ESensorDev_T sensor)
{
    AE_FLOW_LOG("eSensorDev(0x%02x)", (MUINT32)sensor);

    int i4SensorIdx = NS3Av3::mapSensorDevToIdx(sensor);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
    }

    MY_INST_AEFLOW& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<AeFlowCCU>(sensor);
    } );

    return rSingleton.instance.get();
}

AeFlowCCU::
AeFlowCCU(ESensorDev_T sensor)
: m_eSensorDev(sensor)
, m_i4SensorIdx(0)
, m_eSensorMode(ESensorMode_Preview)
, m_pICcuAe(NULL)
, m_bIsCCUStart(MFALSE)
, m_bIsCCUWorking(MFALSE)
, m_bIsCCUAEWorking(MFALSE)
, m_bIsCCUPaused(MFALSE)
, m_bIsCCUResultGet(MFALSE)
//, m_bCCUAEFlag(MTRUE)
, m_bCCUIsSensorSupported(MFALSE)
, m_bEnableFlickerFPSstate(MFALSE)
, m_bControlCCUInfochange(MFALSE)
, m_bControlOBCInfochange(MFALSE)
, m_i4OBCTableidx(0)
, m_bSetCCUFrameRate(MFALSE)
, m_3ALogEnable(MFALSE)
, m_bIsLongExpControlbyCPU(MFALSE)
, m_bIsFirstcontrolCCU(MFALSE)
, m_bIsNeedCCUSensorInit(MTRUE)
{
    memset(&rOnchangeData, 0, sizeof(ccu_ae_onchange_data));
    AE_FLOW_LOG("[AeFlowCCU:Construct] SensorDev:%d rOnchangeData reset\n", (MUINT32)m_eSensorDev);
}

MVOID
AeFlowCCU::
init()
{
    AE_FLOW_LOG("[%s] SensorDev:%d\n", __FUNCTION__, (MUINT32)m_eSensorDev);
    //m_pICcuAe = ICcu::createInstance((const MINT8 *)"AeFlow", ICcu::AEAWB, ICcu::AEAWBCTRL);
    m_pICcuAe = ICcuCtrlAe::createInstance();
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.aaa.pvlog.enable", value, "0");
    m_3ALogEnable = atoi(value);
}

MVOID
AeFlowCCU::
uninit()
{
    AE_FLOW_LOG("[%s] SensorDev:%d\n", __FUNCTION__, (MUINT32)m_eSensorDev);

    if (m_pICcuAe != NULL)
    {
        m_pICcuAe->destroyInstance();
        m_pICcuAe = NULL;
    }
}

MVOID getAdbProperty(AE_NVRAM_T * pNvram)
{
    char default_value[PROPERTY_VALUE_MAX];
    char value[PROPERTY_VALUE_MAX];
    ///////////////////////////in calFD40()///////////////////////////////////////////////////////////////////
    sprintf(default_value, "%d", pNvram->rAeParam.AEStableThd.u4FaceInStableThd);
    property_get("vendor.debug.face_ae.in_thd", value, default_value);
    pNvram->rAeParam.AEStableThd.u4FaceInStableThd = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.AEStableThd.u4FaceOutB2TStableThd);
    property_get("vendor.debug.face_ae.out_thd_b2t", value, default_value);
    pNvram->rAeParam.AEStableThd.u4FaceOutB2TStableThd = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.AEStableThd.u4FaceOutD2TStableThd);
    property_get("vendor.debug.face_ae.out_thd_d2t", value, default_value);
    pNvram->rAeParam.AEStableThd.u4FaceOutD2TStableThd = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_InToOutThdMaxCnt);
    property_get("vendor.debug.face_ae.in_to_out_thd_cnt", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_InToOutThdMaxCnt = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBnd);
    property_get("vendor.debug.face_ae.thd_low_bnd", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBnd = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBndNum);
    property_get("vendor.debug.face_ae.thd_low_bnd_num", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_LimitStableThdLowBndNum = atoi(value);
    sprintf(default_value, "%d", pNvram->rCCTConfig.rAEFaceMovingRatio.u4B2TEnd);
    property_get("vendor.debug.face_ae.b2t_end", value, default_value);
    pNvram->rCCTConfig.rAEFaceMovingRatio.u4B2TEnd = atoi(value);
    sprintf(default_value, "%d", pNvram->rCCTConfig.rAEFaceMovingRatio.u4B2TStart);
    property_get("vendor.debug.face_ae.b2t_start", value, default_value);
    pNvram->rCCTConfig.rAEFaceMovingRatio.u4B2TStart = atoi(value);
    sprintf(default_value, "%d", pNvram->rCCTConfig.rAEFaceMovingRatio.u4D2TEnd);
    property_get("vendor.debug.face_ae.d2t_end", value, default_value);
    pNvram->rCCTConfig.rAEFaceMovingRatio.u4D2TEnd = atoi(value);
    sprintf(default_value, "%d", pNvram->rCCTConfig.rAEFaceMovingRatio.u4D2TStart);
    property_get("vendor.debug.face_ae.d2t_start", value, default_value);
    pNvram->rCCTConfig.rAEFaceMovingRatio.u4D2TStart = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_FaceMotionLockRat);
    property_get("vendor.debug.face_ae.motion_lock_rat", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_FaceMotionLockRat = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4FD_ContinueTrustCnt);
    property_get("vendor.debug.face_ae.cont_trust_cnt", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4FD_ContinueTrustCnt = atoi(value);
    sprintf(default_value, "%d", pNvram->rAeParam.FaceSmooth.u4MaxFDYHighBound);
    property_get("vendor.debug.face_ae.y_high_bound", value, default_value);
    pNvram->rAeParam.FaceSmooth.u4MaxFDYHighBound = atoi(value);
}

MVOID
AeFlowCCU::
start(MVOID * pAEInitInput)
{
    static MBOOL boot_failed = MFALSE;
    if (boot_failed)
    {
        AE_FLOW_LOG("[%s()] ICCU boot failed before, skip starting ccu\n", __FUNCTION__);
        return;
    }

    if (!m_bIsCCUStart)
    {
        AE_FLOW_INPUT_INFO_T* pinitInfo = ((AE_FLOW_INPUT_INFO_T*) pAEInitInput);
        m_i4SensorIdx = pinitInfo->i4SensorIdx;
        m_eSensorMode = pinitInfo->eSensorMode;
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] SensorDev:%d SensorIdx:%d SensorMode:%d\n", __FUNCTION__, (MUINT32)m_eSensorDev, m_i4SensorIdx, m_eSensorMode);

        SENSOR_INFO_IN_T rSensorinfoInput;
        SENSOR_INFO_OUT_T rSensorinfoOutput;
        CCU_AE_INITI_PARAM_T rAEInitparam;
        CCU_AE_CONTROL_CFG_T rAEControlcfg;
        CcuCtrlAeInitParam rCCUAEInitParam;
        AE_CORE_INIT rAEcoreInit;
        memset(&rSensorinfoInput, 0, sizeof(SENSOR_INFO_IN_T));
        memset(&rSensorinfoOutput, 0, sizeof(SENSOR_INFO_OUT_T));
        memset(&rAEInitparam, 0, sizeof(CCU_AE_INITI_PARAM_T));
        memset(&rAEControlcfg, 0, sizeof(CCU_AE_CONTROL_CFG_T));
        memset(&rAEcoreInit, 0, sizeof(AE_CORE_INIT));

        if (m_bIsNeedCCUSensorInit)
        {
            // init CCU
            AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] CCU initial start, sensor id:%x\n", __FUNCTION__, m_i4SensorIdx);
            if (m_pICcuAe->init(m_i4SensorIdx, m_eSensorDev) != CCU_CTRL_SUCCEED)
            {
                AE_FLOW_LOG("[%s()] m_pICcuAe->init() fail\n", __FUNCTION__);
                m_bCCUIsSensorSupported = MFALSE;
                return;
            }

            AE_FLOW_LOG("[%s()] CCU initial & boot done\n", __FUNCTION__);

            // start Sensor and I2C
            //rSensorinfoInput.u32SensorId = (MUINT32)m_eSensorDev;
            //rSensorinfoInput.u32SensorId = getSensorId(m_i4SensorIdx, m_eSensorDev);

            rSensorinfoInput.eScenario = static_cast < MUINT32 > (m_eSensorMode);
            AE_FLOW_LOG("[%s()] rSensorinfoInput.eScenario:%d\n", __FUNCTION__, rSensorinfoInput.eScenario);
            //AE_FLOW_LOG("rSensorinfoInput.eWhichSensor : %d", rSensorinfoInput.eWhichSensor);
            //AE_FLOW_LOG("rSensorinfoInput.pu8BufferVAddr : %d", rSensorinfoInput.pu8BufferVAddr);
            //AE_FLOW_LOG("rSensorinfoInput.sensorI2cSlaveAddr : %d", rSensorinfoInput.sensorI2cSlaveAddr);
            //AE_FLOW_LOG("rSensorinfoInput.u16BufferLen : %d", rSensorinfoInput.u16BufferLen);
            //AE_FLOW_LOG("rSensorinfoInput.u16FPS : %d", rSensorinfoInput.u16FPS);

            if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_SENSOR_INIT, &rSensorinfoInput, &rSensorinfoOutput))
            {
                AE_FLOW_LOG("[%s()] ccu_sensor_initialize fail\n", __FUNCTION__);
                m_bCCUIsSensorSupported = MFALSE;
                return;
            }
            else
            {
                m_bIsLongExpControlbyCPU = (rSensorinfoOutput.u8SupportedByCCU & 0x10) >> 4;
                AE_FLOW_LOG("[%s()] ccu_sensor_initialize success, m_bIsLongExpControlbyCPU:%x\n", __FUNCTION__, m_bIsLongExpControlbyCPU);
                if (!rSensorinfoOutput.u8SupportedByCCU)
                {
                    AE_FLOW_LOG("[%s()] this sensor is not supported by CCU\n", __FUNCTION__);
                    m_bCCUIsSensorSupported = MFALSE;
                    return;
                }
            }
        }
        else
        {
            AE_FLOW_LOG("[%s()] ccu sensor already init\n", __FUNCTION__);
            m_bIsNeedCCUSensorInit = MTRUE;
        }
        // init ccu ae
        rAEControlcfg.hdr_stat_en = (MBOOL)(pinitInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.bEnableHDRLSB > 0);
        rAEControlcfg.ae_overcnt_en = pinitInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.bEnableAEOVERCNTconfig;
        rAEControlcfg.tsf_stat_en = pinitInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.bEnableTSFSTATconfig;
        rAEControlcfg.bits_num = (pinitInfo->u4AAOmode == 1)? 14:12;
        rAEControlcfg.ae_footprint_hum = pinitInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumX;
        rAEControlcfg.ae_footprint_vum = pinitInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumY;
        rAEControlcfg.exp_on_start.m_u4Index = pinitInfo->u4Index;
        rAEControlcfg.exp_on_start.m_u4IndexF = pinitInfo->u4IndexF;
        rAEControlcfg.exp_on_start.u4Eposuretime = pinitInfo->u4Exp;
        rAEControlcfg.exp_on_start.u4AfeGain = pinitInfo->u4Afe;
        rAEControlcfg.exp_on_start.u4IspGain = pinitInfo->u4Isp;
        rAEControlcfg.exp_on_start.u4ISO = pinitInfo->u4RealISO;
        rAEControlcfg.aao_line_width = 2760;

        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] hdr_stat_en/ae_overcnt_en/tsf_stat_en/bits_num:%x/%x/%x/%x\n", __FUNCTION__,
            rAEControlcfg.hdr_stat_en, rAEControlcfg.ae_overcnt_en, rAEControlcfg.tsf_stat_en, rAEControlcfg.bits_num);
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] ae_footprint_hum/ae_footprint_vum:%x/%x\n", __FUNCTION__,
            rAEControlcfg.ae_footprint_hum, rAEControlcfg.ae_footprint_vum);
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] m_u4Index/m_u4IndexF:%x/%x u4Eposuretime/u4AfeGain/u4IspGain/u4ISO:%x/%x/%x/%x\n", __FUNCTION__,
            rAEControlcfg.exp_on_start.m_u4Index, rAEControlcfg.exp_on_start.m_u4IndexF, rAEControlcfg.exp_on_start.u4Eposuretime,
            rAEControlcfg.exp_on_start.u4AfeGain, rAEControlcfg.exp_on_start.u4IspGain, rAEControlcfg.exp_on_start.u4ISO);

        //Fill up AE_CORE_INIT
        rAEcoreInit.u4LastIdx = 100; //For integration test fix value
        rAEcoreInit.EndBankIdx = 0;
        //rAEcoreInit.m_AeTuningParam will be fill inside MSG_TO_CCU_AE_INIT cmd handler
        //rAEcoreInit.m_AeCCTConfig will be fill inside MSG_TO_CCU_AE_INIT cmd handler
        rAEcoreInit.m_i4AEMaxBlockWidth = pinitInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumX;
        rAEcoreInit.m_i4AEMaxBlockHeight = pinitInfo->pAEInitInput->rAENVRAM->rAeParam.strAEParasetting.u4AEWinodwNumY;
        rAEcoreInit.Call_AE_Core_init = pinitInfo->bIsCCUAEInit;
        // {
        // int i;
        // for (i = 0; i < 5; ++ i)
        // {
        // memcpy(&(rAEcoreInit.m_pWtTbl_W[i][0]), &(pinitInfo->pAEInitInput->rAENVRAM->rAeParam.Weighting[0].W[i][0]), 5 * sizeof(MUINT32));
        // }
        // }
        // //memcpy(rAEcoreInit.m_pWtTbl_W, pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pWeighting[0]->W, 5 *5 *sizeof(MUINT32));
        // rAEcoreInit.m_AECmdSet_bRotateWeighting = 0; //temp
        //!!!memcpy(&rAEcoreInit.OBC_ISO_IDX_Range, ???, sizeof(MINT32) *5);
        rAEcoreInit.m_u4AETarget = pinitInfo->pAEInitInput->rAENVRAM->rCCTConfig.u4AETarget;
        rAEcoreInit.u4InitIndex = pinitInfo->pAEInitInput->rAENVRAM->rCCTConfig.u4InitIndex;
        //rAEcoreInit.m_rIspIsoEnv = *(pinitInfo->pISPNvramISOInterval);
        memcpy(&rAEcoreInit.OBC_Table[0], &(pinitInfo->pISPNvramOBC_Table[0]), 4 * sizeof(CCU_ISP_NVRAM_OBC_T));

        //!!!rAEcoreInit.u2Length = ???;
        rAEcoreInit.m_LumLog2x1000 = (MINT32 *)Log2x1000;
        rAEcoreInit.pEVValueArray = &(pinitInfo->pAEInitInput->rAENVRAM->rAeParam.EVValueArray.EVValue[0]);

        rAEcoreInit.i4BVOffset = pinitInfo->pAEInitInput->rAENVRAM->rCCTConfig.i4BVOffset;
        rAEcoreInit.i4MaxBV = pinitInfo->pPreviewTableCurrent->i4MaxBV;
        rAEcoreInit.i4MinBV = pinitInfo->pPreviewTableCurrent->i4MinBV;

        rAEcoreInit.pCurrentTableF = (CCU_strFinerEvPline*)pinitInfo->pPreviewTableF;
        rAEcoreInit.pCurrentTable = (CCU_strEvPline*)pinitInfo->pPreviewTableCurrent->pCurrentTable;

        rAEcoreInit.m_u4IndexFMax = pinitInfo->u4IndexFMax;
        rAEcoreInit.m_u4IndexFMin = pinitInfo->u4IndexFMin;

        rAEcoreInit.m_u4IndexMax = pinitInfo->u4IndexFMax / pinitInfo->u4FinerEVIdxBase;
        rAEcoreInit.m_u4IndexMin = pinitInfo->u4IndexFMin / pinitInfo->u4FinerEVIdxBase;
        rAEcoreInit.m_u4FinerEVIdxBase = pinitInfo->u4FinerEVIdxBase; // Real index base
        rAEcoreInit.m_u4AAO_AWBValueWinSize = pinitInfo->pAEInitInput->u4AAO_AWBValueWinSize; // bytes of AWB Value window
        rAEcoreInit.m_u4AAO_AWBSumWinSize = pinitInfo->pAEInitInput->u4AAO_AWBSumWinSize; // bytes of AWB Sum window
        rAEcoreInit.m_u4AAO_AEYWinSize = pinitInfo->pAEInitInput->u4AAO_AEYWinSize; // bytes of AEY window
        rAEcoreInit.m_u4AAO_AEOverWinSize = pinitInfo->pAEInitInput->u4AAO_AEOverWinSize; // bytes of AEOverCnt window
        rAEcoreInit.m_u4AAO_HistSize = pinitInfo->pAEInitInput->u4AAO_HistSize; // bytes of each Hist
        rAEcoreInit.m_u4PSO_SE_AWBWinSize = pinitInfo->pAEInitInput->u4PSO_SE_AWBWinSize; // bytes of AWB window
        rAEcoreInit.m_u4PSO_LE_AWBWinSize = pinitInfo->pAEInitInput->u4PSO_LE_AWBWinSize; // bytes of AWB window
        rAEcoreInit.m_u4PSO_SE_AEYWinSize = pinitInfo->pAEInitInput->u4PSO_SE_AEYWinSize; // bytes of AEY window
        rAEcoreInit.m_u4PSO_LE_AEYWinSize = pinitInfo->pAEInitInput->u4PSO_LE_AEYWinSize; // bytes of AEY window
        rAEcoreInit.m_u4PSO_SE_HistSize = pinitInfo->pAEInitInput->u4PSO_SE_HistSize; // bytes of each Hist
        rAEcoreInit.m_u4PSO_LE_HistSize = pinitInfo->pAEInitInput->u4PSO_LE_HistSize; // bytes of each Hist

        rAEcoreInit.u2LengthV2 = pinitInfo->u2Length;
        memcpy(&(rAEcoreInit.IDX_PartitionV2), pinitInfo->u2IDX_Partition, NVRAM_ISP_REGS_ISO_SUPPORT_NUM * sizeof(MINT16));
        memcpy(&(rAEcoreInit.OBC_TableV2), pinitInfo->pISP5NvramOBC_Table, NVRAM_ISP_REGS_ISO_SUPPORT_NUM * sizeof(CCU_ISP_NVRAM_OBC_T));

        //pinitInfo->u2IDX_Partition
        // MINT16 IDX_PartitionV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
        // CCU_ISP_NVRAM_OBC_T OBC_TableV2[NVRAM_ISP_REGS_ISO_SUPPORT_NUM];
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4FinerEVIdxBase %x\n", rAEcoreInit.m_u4FinerEVIdxBase);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4AAO_AWBValueWinSize %x\n", rAEcoreInit.m_u4AAO_AWBValueWinSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4AAO_AWBSumWinSize %x\n", rAEcoreInit.m_u4AAO_AWBSumWinSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4AAO_AEYWinSize %x\n", rAEcoreInit.m_u4AAO_AEYWinSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4AAO_AEOverWinSize %x\n", rAEcoreInit.m_u4AAO_AEOverWinSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4AAO_HistSize %x\n", rAEcoreInit.m_u4AAO_HistSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4PSO_SE_AWBWinSize %x\n", rAEcoreInit.m_u4PSO_SE_AWBWinSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4PSO_LE_AWBWinSize %x\n", rAEcoreInit.m_u4PSO_LE_AWBWinSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4PSO_SE_AEYWinSize %x\n", rAEcoreInit.m_u4PSO_SE_AEYWinSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4PSO_LE_AEYWinSize %x\n", rAEcoreInit.m_u4PSO_LE_AEYWinSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4PSO_SE_HistSize %x\n", rAEcoreInit.m_u4PSO_SE_HistSize);
        AE_FLOW_LOG_IF(m_3ALogEnable, "m_u4PSO_LE_HistSize %x\n", rAEcoreInit.m_u4PSO_LE_HistSize);
        rAEcoreInit.eAEMeteringMode = (CCU_LIB3A_AE_METERING_MODE_T)pinitInfo->pAEInitInput->eAEMeteringMode;
        rAEcoreInit.eAEScene = (CCU_LIB3A_AE_SCENE_T)pinitInfo->pAEInitInput->eAEScene;
        rAEcoreInit.eAECamMode = (CCU_LIB3A_AECAM_MODE_T)pinitInfo->pAEInitInput->eAECamMode;
        rAEcoreInit.eAEFlickerMode = (CCU_LIB3A_AE_FLICKER_MODE_T)pinitInfo->pAEInitInput->eAEFlickerMode;
        rAEcoreInit.eAEAutoFlickerMode = (CCU_LIB3A_AE_FLICKER_AUTO_MODE_T)pinitInfo->pAEInitInput->eAEAutoFlickerMode;
        rAEcoreInit.eAEEVcomp = (CCU_LIB3A_AE_EVCOMP_T)pinitInfo->pAEInitInput->eAEEVcomp;
        rAEcoreInit.u4AEISOSpeed = pinitInfo->pAEInitInput->u4AEISOSpeed;
        rAEcoreInit.i4AEMaxFps = pinitInfo->pAEInitInput->i4AEMaxFps;
        rAEcoreInit.i4AEMinFps = pinitInfo->pAEInitInput->i4AEMinFps;
        rAEcoreInit.i4SensorMode = pinitInfo->pAEInitInput->i4SensorMode;
        rAEcoreInit.eSensorDev = (CCU_AE_SENSOR_DEV_T)pinitInfo->pAEInitInput->eSensorDev;
        rAEcoreInit.eAEScenario = (CCU_CAM_SCENARIO_T)pinitInfo->pAEInitInput->eAEScenario;
        rAEcoreInit.pAETouchMovingRatio = (CCU_strAEMovingRatio*)&((pinitInfo->pAEInitInput->rAENVRAM->rAeParam.AETouchMovingRatio));
        getAdbProperty(pinitInfo->pAEInitInput->rAENVRAM);
        rAEcoreInit.pAeNVRAM = (CCU_AE_NVRAM_T *)(pinitInfo->pAEInitInput->rAENVRAM);
        //memcpy(&(rAEcoreInit.rDevicesInfo), &(pinitInfo->pAEInitInput->rAENVRAM->rDevicesInfo), sizeof(CCU_AE_DEVICES_INFO_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgnvram Size %zu", sizeof(CCU_AE_NVRAM_T));

        AE_FLOW_LOG_IF(m_3ALogEnable, "rAEcoreInit.i4AEMaxFps %x", rAEcoreInit.i4AEMaxFps);
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgnvram Top protect %x", rAEcoreInit.pAeNVRAM->u4TopProtect);
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgprob mgr u4FlatDarkPcent %x\n", pinitInfo->pAEInitInput->rAENVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4FlatDarkPcent);
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgprob mgr u4FlatBrightPcent %x \n", pinitInfo->pAEInitInput->rAENVRAM->rCCTConfig.rMeteringSpec.rHS_Spec.u4FlatBrightPcent);
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgnvram mgr i4SWHdrLEThr %x\n", pinitInfo->pAEInitInput->rAENVRAM->rHDRAEConfig.i4SWHdrLEThr);
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgnvram mgr i4SWHdrSERatio %x\n", pinitInfo->pAEInitInput->rAENVRAM->rHDRAEConfig.i4SWHdrSERatio);
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgnvram mgr i4SWHdrSETarget %x\n", pinitInfo->pAEInitInput->rAENVRAM->rHDRAEConfig.i4SWHdrSETarget);
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgnvram mgr i4SWHdrBaseGain %x\n", pinitInfo->pAEInitInput->rAENVRAM->rHDRAEConfig.i4SWHdrBaseGain);
        // AE_FLOW_LOG("dbgw mgr touch %x", pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pAETouchMovingRatio->u4SpeedUpRatio);
        // AE_FLOW_LOG("dbgw mgr touch %x", pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pAETouchMovingRatio->u4GlobalRatio);
        // AE_FLOW_LOG("dbgw mgr touch %x", pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pAETouchMovingRatio->u4Bright2TargetEnd);
        // AE_FLOW_LOG("dbgw mgr touch %x", pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pAETouchMovingRatio->u4Dark2TargetStart);
        // AE_FLOW_LOG("dbgw mgr touch %x", pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pAETouchMovingRatio->u4B2TEnd);
        // AE_FLOW_LOG("dbgw mgr touch %x", pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pAETouchMovingRatio->u4B2TStart);
        // AE_FLOW_LOG("dbgw mgr touch %x", pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pAETouchMovingRatio->u4D2TEnd);
        // AE_FLOW_LOG("dbgw mgr touch %x", pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pAETouchMovingRatio->u4D2TStart);

        //rAEcoreInit.m_pWtTbl_W[0][0] = 6;
        //rAEcoreInit.m_pWtTbl_W[0][1] = 19;
        //rAEcoreInit.m_pWtTbl_W[0][2] = 28;
        //rAEcoreInit.m_pWtTbl_W[0][3] = 19;
        //rAEcoreInit.m_pWtTbl_W[0][4] = 6;
        //rAEcoreInit.m_pWtTbl_W[1][0] = 19;
        //rAEcoreInit.m_pWtTbl_W[1][1] = 60;
        //rAEcoreInit.m_pWtTbl_W[1][2] = 88;
        //rAEcoreInit.m_pWtTbl_W[1][3] = 60;
        //rAEcoreInit.m_pWtTbl_W[1][4] = 19;
        //rAEcoreInit.m_pWtTbl_W[2][0] = 28;
        //rAEcoreInit.m_pWtTbl_W[2][1] = 88;
        //rAEcoreInit.m_pWtTbl_W[2][2] = 160;
        //rAEcoreInit.m_pWtTbl_W[2][3] = 88;
        //rAEcoreInit.m_pWtTbl_W[2][4] = 28;
        //rAEcoreInit.m_pWtTbl_W[3][0] = 19;
        //rAEcoreInit.m_pWtTbl_W[3][1] = 60;
        //rAEcoreInit.m_pWtTbl_W[3][2] = 88;
        //rAEcoreInit.m_pWtTbl_W[3][3] = 60;
        //rAEcoreInit.m_pWtTbl_W[3][4] = 19;
        //rAEcoreInit.m_pWtTbl_W[4][0] = 6;
        //rAEcoreInit.m_pWtTbl_W[4][1] = 19;
        //rAEcoreInit.m_pWtTbl_W[4][2] = 28;
        //rAEcoreInit.m_pWtTbl_W[4][3] = 19;
        //rAEcoreInit.m_pWtTbl_W[4][4] = 6;
        // {
        // int i, j;
        // for (i = 0; i < 5; i++)
        // {
        // for (j = 0; j < 5; j++)
        // {
        // //rAEcoreInit.m_pWtTbl_W[i][j] = pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pWeighting[0]->W[i][j];
        // AE_FLOW_LOG("WeightingTable %x", rAEcoreInit.m_pWtTbl_W[i][j]);
        // }
        // }
        // }

        //AE_FLOW_LOG("pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pWeighting[0][0] : %x", pinitInfo->pAEInitInput->rAENVRAM->rAeParam.pWeighting[1][1]);
        /*AE_FLOW_LOG_IF(m_3ALogEnable,"[%s()] AEIDBG CPU u4LastIdx: %x obcTable: %x, %x, %x curTable m_u4IndexMax: %x 0/1/2/100/200/300/400/MAX-1/MAX: %x, %x, %x, %x, %x, %x, %x, %x, %x\n", __FUNCTION__,
            rAEcoreInit.u4LastIdx,rAEcoreInit.OBC_Table[0].set[0], rAEcoreInit.OBC_Table[1].set[0], rAEcoreInit.OBC_Table[2].set[0],
            rAEcoreInit.m_u4IndexFMax,rAEcoreInit.pCurrentTableF->sPlineTable[0].u4Eposuretime,
            rAEcoreInit.pCurrentTableF->sPlineTable[1].u4Eposuretime,
            rAEcoreInit.pCurrentTableF->sPlineTable[2].u4Eposuretime,
            rAEcoreInit.pCurrentTableF->sPlineTable[100].u4Eposuretime,
            rAEcoreInit.pCurrentTableF->sPlineTable[200].u4Eposuretime,
            rAEcoreInit.pCurrentTableF->sPlineTable[300].u4Eposuretime,
            rAEcoreInit.pCurrentTableF->sPlineTable[400].u4Eposuretime,
            rAEcoreInit.pCurrentTableF->sPlineTable[rAEcoreInit.m_u4IndexFMax-1].u4Eposuretime,
            rAEcoreInit.pCurrentTableF->sPlineTable[rAEcoreInit.m_u4IndexFMax].u4Eposuretime);*/

        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] AEIDBG sizeof(AE_CORE_INIT): %zu\n", __FUNCTION__, sizeof(AE_CORE_INIT));

        rAEcoreInit.OBC_TableV2[0].set[0] = 0xabab1111;
        rAEcoreInit.OBC_TableV2[19].set[7] = 0xfcfc9999;

        rAEInitparam.control_cfg = rAEControlcfg;
        rAEInitparam.algo_init_param = rAEcoreInit;

        rCCUAEInitParam.ccuAeInitParam = rAEInitparam;
        rCCUAEInitParam.aeParam = &(pinitInfo->pAEInitInput->rAENVRAM->rAeParam);
        rCCUAEInitParam.nvramData = (pinitInfo->pAEInitInput->rAENVRAM);

        //get AE sync init info
        const strSyncAEInitInfo * syncAeInfo = getSyncAEInitInfo();

        AE_FLOW_LOG("[%s()] syncInfo.EVOffset_main[0]:%x RGB2YCoef_main[0]:%x pDeltaBVtoRatioArray[last][last]:%x\n", __FUNCTION__,
            syncAeInfo->EVOffset_main[0], syncAeInfo->RGB2YCoef_main[0], syncAeInfo->pDeltaBVtoRatioArray[SYNC_DUAL_CAM_DENOISE_MAX -1][MAX_MAPPING_DELTABV_ISPRATIO -1]);

        rCCUAEInitParam.ccuAeInitParam.sync_algo_init_param = (CCU_strSyncAEInitInfo *)syncAeInfo;

        //AE_LOG("rAEcoreInit.pCurrentTable addr: %p\n", &rAEInitparam.algo_init_param.pCurrentTable);
        if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_INIT, &rCCUAEInitParam, NULL))
        {
            AE_FLOW_LOG("[%s()] ccu_ae_initialize fail\n", __FUNCTION__);
            m_bCCUIsSensorSupported = MFALSE;
            return;
        }
        AE_FLOW_LOG("[%s()] ccu_ae_initialize success\n", __FUNCTION__);

        /*int debugInfo;
        while(1)
        {
            debugInfo = m_pICcu->readInfoReg(20);
            AE_FLOW_LOG( "[%s()] bankdone debug: %x\n", __FUNCTION__, debugInfo);
            usleep(1000);
        }*/
        ccu_max_framerate_data max_framerate_data;
        max_framerate_data.framerate = (MUINT16)pinitInfo->i4AEMaxFps;
        max_framerate_data.min_framelength_en = MTRUE;
        AE_FLOW_LOG("[%s()] m_bIsFirstcontrolCCU = MTRUE m_i4AEMaxFps:0x%x\n", __FUNCTION__, max_framerate_data.framerate);
        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_MAX_FRAMERATE, &max_framerate_data, NULL);
        m_bSetCCUFrameRate = MTRUE;
        m_bControlCCUInfochange = MTRUE;
        m_bControlOBCInfochange = MTRUE;
        m_bIsCCUStart = MTRUE;
        m_bIsCCUPaused = MFALSE;
        m_bIsCCUResultGet = MFALSE;
        m_bCCUIsSensorSupported = MTRUE;
        m_bIsFirstcontrolCCU = MTRUE;

        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEParamCFG cpu:%zu ccu:%zu\n", sizeof(strAEParamCFG), sizeof(CCU_strAEParamCFG));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strWeightTable cpu:%zu ccu:%zu\n", sizeof(strWeightTable), sizeof(CCU_strWeightTable));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAFPlineInfo cpu:%zu ccu:%zu\n", sizeof(strAFPlineInfo), sizeof(CCU_strAFPlineInfo));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAFPlineInfo cpu:%zu ccu:%zu\n", sizeof(strAFPlineInfo), sizeof(CCU_strAFPlineInfo));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAFPlineInfo cpu:%zu ccu:%zu\n", sizeof(strAFPlineInfo), sizeof(CCU_strAFPlineInfo));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAFPlineInfo cpu:%zu ccu:%zu\n", sizeof(strAFPlineInfo), sizeof(CCU_strAFPlineInfo));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strEVC cpu:%zu ccu:%zu\n", sizeof(strEVC), sizeof(CCU_strEVC));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMoveTable cpu:%zu ccu:%zu\n", sizeof(strAEMoveTable), sizeof(CCU_strAEMoveTable));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMoveTable cpu:%zu ccu:%zu\n", sizeof(strAEMoveTable), sizeof(CCU_strAEMoveTable));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMoveTable cpu:%zu ccu:%zu\n", sizeof(strAEMoveTable), sizeof(CCU_strAEMoveTable));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMoveTable cpu:%zu ccu:%zu\n", sizeof(strAEMoveTable), sizeof(CCU_strAEMoveTable));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAELimiterTable cpu:%zu ccu:%zu\n", sizeof(strAELimiterTable), sizeof(CCU_strAELimiterTable));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww VdoDynamicFrameRate_T cpu:%zu ccu:%zu\n", sizeof(VdoDynamicFrameRate_T), sizeof(CCU_VdoDynamicFrameRate_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww AE_HIST_WIN_CFG_T cpu:%zu ccu:%zu\n", sizeof(AE_HIST_WIN_CFG_T), sizeof(CCU_AE_HIST_WIN_CFG_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww AE_HIST_WIN_CFG_T cpu:%zu ccu:%zu\n", sizeof(AE_HIST_WIN_CFG_T), sizeof(CCU_AE_HIST_WIN_CFG_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww PS_HIST_WIN_CFG_T cpu:%zu ccu:%zu\n", sizeof(PS_HIST_WIN_CFG_T), sizeof(CCU_PS_HIST_WIN_CFG_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strFaceLocSize cpu:%zu ccu:%zu\n", sizeof(strFaceLocSize), sizeof(CCU_strFaceLocSize));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strPerframeCFG cpu:%zu ccu:%zu\n", sizeof(strPerframeCFG), sizeof(CCU_strPerframeCFG));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAETgTuningPara cpu:%zu ccu:%zu\n", sizeof(strAETgTuningPara), sizeof(CCU_strAETgTuningPara));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strEVC cpu:%zu ccu:%zu\n", sizeof(strEVC), sizeof(CCU_strEVC));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strEVC cpu:%zu ccu:%zu\n", sizeof(strEVC), sizeof(CCU_strEVC));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEStableThd cpu:%zu ccu:%zu\n", sizeof(strAEStableThd), sizeof(CCU_strAEStableThd));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strBVAccRatio cpu:%zu ccu:%zu\n", sizeof(strBVAccRatio), sizeof(CCU_strBVAccRatio));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strPSOConverge cpu:%zu ccu:%zu\n", sizeof(strPSOConverge), sizeof(CCU_strPSOConverge));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strNonCWRAcc cpu:%zu ccu:%zu\n", sizeof(strNonCWRAcc), sizeof(CCU_strNonCWRAcc));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strFaceSmooth cpu:%zu ccu:%zu\n", sizeof(strFaceSmooth), sizeof(CCU_strFaceSmooth));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strFaceWeight cpu:%zu ccu:%zu\n", sizeof(strFaceWeight), sizeof(CCU_strFaceWeight));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAECWRTempSmooth cpu:%zu ccu:%zu\n", sizeof(strAECWRTempSmooth), sizeof(CCU_strAECWRTempSmooth));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strFaceLandMarkCtrl cpu:%zu ccu:%zu\n", sizeof(strFaceLandMarkCtrl), sizeof(CCU_strFaceLandMarkCtrl));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAOEBVRef cpu:%zu ccu:%zu\n", sizeof(strAOEBVRef), sizeof(CCU_strAOEBVRef)); //16->8?

        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strNS_CDF cpu:%zu ccu:%zu\n", sizeof(strNS_CDF), sizeof(CCU_strNS_CDF));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMovingRatio cpu:%zu ccu:%zu\n", sizeof(strAEMovingRatio), sizeof(CCU_strAEMovingRatio));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strHSFlatSkyCFG cpu:%zu ccu:%zu\n", sizeof(strHSFlatSkyCFG), sizeof(CCU_strHSFlatSkyCFG));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strABL_absDiff cpu:%zu ccu:%zu\n", sizeof(strABL_absDiff), sizeof(CCU_strABL_absDiff));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strHSMultiStep cpu:%zu ccu:%zu\n", sizeof(strHSMultiStep), sizeof(CCU_strHSMultiStep));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strHistStableCFG cpu:%zu ccu:%zu\n", sizeof(strHistStableCFG), sizeof(CCU_strHistStableCFG));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strOverExpoAOECFG cpu:%zu ccu:%zu\n", sizeof(strOverExpoAOECFG), sizeof(CCU_strOverExpoAOECFG));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strMainTargetCFG cpu:%zu ccu:%zu\n", sizeof(strMainTargetCFG), sizeof(CCU_strMainTargetCFG));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strHSv4p0CFG cpu:%zu ccu:%zu\n", sizeof(strHSv4p0CFG), sizeof(CCU_strHSv4p0CFG));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strNSBVCFG cpu:%zu ccu:%zu\n", sizeof(strNSBVCFG), sizeof(CCU_strNSBVCFG));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAEMovingRatio cpu:%zu ccu:%zu\n", sizeof(strAEMovingRatio), sizeof(CCU_strAEMovingRatio));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww HDR_AE_PARAM_T cpu:%zu ccu:%zu\n", sizeof(HDR_AE_PARAM_T), sizeof(CCU_HDR_AE_PARAM_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAESceneMapping cpu:%zu ccu:%zu\n", sizeof(strAESceneMapping), sizeof(CCU_strAESceneMapping));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strAESceneMapping cpu:%zu ccu:%zu\n", sizeof(strAESceneMapping), sizeof(CCU_strAESceneMapping));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww strDynamicFlare cpu:%zu ccu:%zu\n", sizeof(strDynamicFlare), sizeof(CCU_strDynamicFlare));

        AE_FLOW_LOG_IF(m_3ALogEnable, "dbgww NVRAM cpu:%zu ccu:%zu\n", sizeof(AE_NVRAM_T), sizeof(CCU_AE_NVRAM_T));

        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_eHDRPARAM_ID %zu\n", sizeof(CCU_eHDRPARAM_ID));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_DETECTOR_T %zu\n", sizeof(CCU_HDR_DETECTOR_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_AUTO_ENHANCE_T %zu\n", sizeof(CCU_HDR_AUTO_ENHANCE_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_AE_TARGET_T %zu\n", sizeof(CCU_HDR_AE_TARGET_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_MISC_PROP_T %zu\n", sizeof(CCU_HDR_MISC_PROP_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_CHDR_T %zu\n", sizeof(CCU_HDR_CHDR_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AE_TARGET_PROB_T %zu\n ", sizeof(CCU_AE_TARGET_PROB_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AUTO_BRIGHT_ENHANCE_T %zu\n ", sizeof(CCU_AUTO_BRIGHT_ENHANCE_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AUTO_FLICKER_RATIO_T %zu\n ", sizeof(CCU_AUTO_FLICKER_RATIO_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AUTO_SMOOTH_T %zu\n ", sizeof(CCU_AUTO_SMOOTH_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_AUTO_TUNING_T %zu\n ", sizeof(CCU_AUTO_TUNING_T));
        AE_FLOW_LOG_IF(m_3ALogEnable, "dbghdr CCU_HDR_FACE_T %zu\n ", sizeof(CCU_HDR_FACE_T));
    }
}

MVOID
AeFlowCCU::
stop()
{
    //if CCU is under pause state, shutdown & uninit flow should also take place
    if (m_bIsCCUStart /*|| m_bIsCCUPaused */ )
    {
        AE_FLOW_LOG("[%s] SensorDev:%d\n", __FUNCTION__, m_eSensorDev);

        if (CCU_CTRL_SUCCEED != m_pICcuAe->ccuControl(MSG_TO_CCU_AE_STOP, NULL, NULL))
        {
            AE_FLOW_LOG("[%s] ccu_ae_stop fail\n", __FUNCTION__);
            return;
        }
        m_bIsCCUStart = MFALSE;
        m_bIsCCUAEWorking = MFALSE;
        m_bIsCCUResultGet = MFALSE;
        //m_bIsCCUPaused = MFALSE;
    }
}

MVOID
AeFlowCCU::
pause(MBOOL IsNeedCCUSensorInit)
{
    if (m_bIsCCUStart)
    {
        AE_FLOW_LOG("[%s()] m_bIsNeedCCUSensorInit:%d->%d\n", __FUNCTION__, m_bIsNeedCCUSensorInit, IsNeedCCUSensorInit);
        this->stop();
        m_bIsCCUStart = MFALSE;
        m_bIsCCUAEWorking = MFALSE;
        m_bIsNeedCCUSensorInit = IsNeedCCUSensorInit;
        //m_bIsCCUPaused = MTRUE;
    }

}

MBOOL
AeFlowCCU::
queryStatus(MUINT32 index)
{
    E_AE_FLOW_CCU_STATUS_T eStatus = (E_AE_FLOW_CCU_STATUS_T) index;
    switch (eStatus)
    {
        case E_AE_FLOW_CCU_WORKING:
            return IsCCUWorking();
        case E_AE_FLOW_CCU_AE_WORKING:
            return IsCCUAEWorking();
        case E_AE_FLOW_CCU_SENSOR_SUPPORTED:
            return m_bCCUIsSensorSupported;
        case E_AE_FLOW_CCU_AE_RESULT_GET:
            return m_bIsCCUResultGet;
        case E_AE_FLOW_CCU_LONG_EXP_CONTROL:
            return m_bIsLongExpControlbyCPU;
        default:
            AE_FLOW_LOG("[%s():Warning] SensorDev:%d index:%d\n", __FUNCTION__, m_eSensorDev, index);
            return MFALSE;
    }
}

MVOID
AeFlowCCU::
controltrigger(MUINT32 index, MBOOL btrigger)
{
    E_AE_FLOW_CCU_TRIGER_T etrigger = (E_AE_FLOW_CCU_TRIGER_T) index;
    switch (etrigger)
    {
        case E_AE_FLOW_CCU_TRIGGER_APINFO:
            m_bControlCCUInfochange = MTRUE;
            break;
        case E_AE_FLOW_CCU_TRIGGER_MAXFPS:
            m_bSetCCUFrameRate = MTRUE;
            break;
        case E_AE_FLOW_CCU_TRIGGER_FLICKER_FPS:
            m_bEnableFlickerFPSstate = btrigger;
            break;
        case E_AE_FLOW_CCU_TRIGGER_SENSOR_SUPPORT:
            m_bCCUIsSensorSupported = btrigger;
            break;
        case E_AE_FLOW_CCU_TRIGGER_SENSOR_INIT:
            m_bIsNeedCCUSensorInit = btrigger;
            break;
        default:
            AE_FLOW_LOG("[%s():Warning] SensorDev:%d index:%d\n", __FUNCTION__, m_eSensorDev, index);
    }
}

MVOID
AeFlowCCU::
startCCUAE()
{
    if (!m_bIsCCUAEWorking && IsCCUWorking())
    {
        AE_FLOW_LOG("[%s] SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
        m_pICcuAe->ccuControl(MSG_TO_CCU_AE_START, NULL, NULL);
        m_bIsCCUAEWorking = MTRUE;
    }
}

MVOID
AeFlowCCU::
controlCCU(AE_FLOW_CONTROL_INFO_T * pcontrolInfo, MBOOL bIsDoPvAEControlCCU)
{
    //ccu_skip_algo_data rSkipAlgoData;
    if (IsCCUWorking() && IsCCUAEWorking())
    {
        // Fill Control Info data - add control member to avoid always memory copy
        if (m_bIsFirstcontrolCCU || ((!bIsDoPvAEControlCCU) && (m_bControlCCUInfochange || m_bControlOBCInfochange)))
        {
            //ccu_ae_onchange_data rOnchangeData;
            //memset(&rOnchangeData, 0, sizeof(ccu_ae_onchange_data));
            // rOnchangeData.u2LengthV2 = pcontrolInfo->u2Length;
            // memcpy(&(rOnchangeData.IDX_PartitionV2), pcontrolInfo->u2IDX_Partition, NVRAM_ISP_REGS_ISO_SUPPORT_NUM *sizeof(MINT16));
            memcpy(&(rOnchangeData.OBC_TableV2), pcontrolInfo->pISP5NvramOBC_Table, NVRAM_ISP_REGS_ISO_SUPPORT_NUM *sizeof(CCU_ISP_NVRAM_OBC_T));
            // In order not to modify interface, we assign variables to unused variables and put required variables in high bits.

            MUINT32 u4IndexFMax, u4IndexFMin;
            u4IndexFMax = pcontrolInfo->u4IndexFMax;
            u4IndexFMin = pcontrolInfo->u4IndexFMin;
            rOnchangeData.m_u4IndexFMax = pcontrolInfo->u4IndexFMax | (pcontrolInfo->u4FDProb << 16);
            rOnchangeData.m_u4IndexFMin = pcontrolInfo->u4IndexFMin | (pcontrolInfo->u4FaceFailCnt << 16);
            rOnchangeData.auto_flicker_en = m_bEnableFlickerFPSstate;
            rOnchangeData.pCurrentTableF = (CCU_strFinerEvPline*)pcontrolInfo->pPreviewTableF;
            rOnchangeData.pCurrentTable = (CCU_strEvPline*)pcontrolInfo->pPreviewTableCurrent->pCurrentTable;
            rOnchangeData.i4MaxBV = pcontrolInfo->pPreviewTableCurrent->i4MaxBV;
            rOnchangeData.i4MinBV = pcontrolInfo->pPreviewTableCurrent->i4MinBV;
            rOnchangeData.m_u4Prvflare = pcontrolInfo->u4Prvflare; //HARD -CODE TMP, WAITING FOR BEING FULLED UP
            rOnchangeData.m_u4Index = pcontrolInfo->u4CurrentIndex;
            rOnchangeData.m_u4IndexF = pcontrolInfo->u4CurrentIndexF;
            rOnchangeData.u4UpdateLockIndex = pcontrolInfo->bUpdateAELockIdx;

            rOnchangeData.pAeNVRAM = (CCU_AE_NVRAM_T*)pcontrolInfo->pAEOnchNVRAM;
            AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] AEIDBG CPU ONCH curTable m_u4IndexMax:%x u4Index:%x u4IndexF:%x CCUInfochange:%d OBCInfochange:%d m_u4Prvflare:%d m_bIsFirstcontrolCCU:%d\n", __FUNCTION__,
                u4IndexFMax, pcontrolInfo->u4CurrentIndex, pcontrolInfo->u4CurrentIndexF, m_bControlCCUInfochange, m_bControlOBCInfochange, rOnchangeData.m_u4Prvflare, m_bIsFirstcontrolCCU);
            /*AE_FLOW_LOG_IF(m_3ALogEnable,"AEIDBG CPU ONCH curTable 0/1/2/100/200/300/400/MAX-1/MAX: %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
            rOnchangeData.pCurrentTableF->sPlineTable[0].u4Eposuretime,
            rOnchangeData.pCurrentTableF->sPlineTable[1].u4Eposuretime,
            rOnchangeData.pCurrentTableF->sPlineTable[2].u4Eposuretime,
            rOnchangeData.pCurrentTableF->sPlineTable[100].u4Eposuretime,
            rOnchangeData.pCurrentTableF->sPlineTable[200].u4Eposuretime,
            rOnchangeData.pCurrentTableF->sPlineTable[300].u4Eposuretime,
            rOnchangeData.pCurrentTableF->sPlineTable[400].u4Eposuretime,
            rOnchangeData.pCurrentTableF->sPlineTable[u4IndexFMax-1].u4Eposuretime,
            rOnchangeData.pCurrentTableF->sPlineTable[u4IndexFMax].u4Eposuretime);*/

            rOnchangeData.m_u4FinerEVIdxBase = pcontrolInfo->u4FinerEVBase | (pcontrolInfo->u4FaceFoundCnt << 16);
            /*for (int i = 0; i < 438; ++i)
            {
                AE_FLOW_LOG("#pval_cpu[%x]=%x", i, rOnchangeData.pCurrentTable->sPlineTable[i].u4Eposuretime);
            }
            */
            AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] pAEOnchNVRAM bAEv4p0MeterEnable:%d\n", __FUNCTION__, pcontrolInfo->pAEOnchNVRAM->rAeParam.bAEv4p0MeterEnable);
            if (&(pcontrolInfo->pISPNvramOBC_Table[0]) != NULL && m_bControlOBCInfochange)
            {
                memcpy(&rOnchangeData.OBC_Table[0], &(pcontrolInfo->pISPNvramOBC_Table[0]), 4 *sizeof(CCU_ISP_NVRAM_OBC_T));
                m_i4OBCTableidx = pcontrolInfo->OBCTableidx;
                AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] m_i4SensorDev:%d i4OBCtableidx:%d [0][1][2][3](offset/gain):[%d/%d][%d/%d][%d/%d][%d/%d]\n",
                    __FUNCTION__, m_eSensorDev, m_i4OBCTableidx,
                    rOnchangeData.OBC_Table[0].offst0.val, rOnchangeData.OBC_Table[0].gain0.val,
                    rOnchangeData.OBC_Table[1].offst0.val, rOnchangeData.OBC_Table[1].gain0.val,
                    rOnchangeData.OBC_Table[2].offst0.val, rOnchangeData.OBC_Table[2].gain0.val,
                    rOnchangeData.OBC_Table[3].offst0.val, rOnchangeData.OBC_Table[3].gain0.val);
                m_bControlOBCInfochange = MFALSE;
            }

            m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AP_AE_ONCHANGE_DATA, &rOnchangeData, NULL);
            m_bControlCCUInfochange = MFALSE;
            m_bIsFirstcontrolCCU = MFALSE;
            AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] MaxIdx/MinIdx/FlickerState/pPlineTableAddress/pPlinetable->eID %d/%d/%d/%d\n",
                __FUNCTION__,
                rOnchangeData.m_u4IndexFMax,
                rOnchangeData.m_u4IndexFMin,
                rOnchangeData.auto_flicker_en,
                //rOnchangeData.pCurrentTable,
                pcontrolInfo->pPreviewTableCurrent->eID);
        }
        // Fill frame sync data
        ccu_ae_frame_sync_data rCcuFrameSyncData;
        memset(&rCcuFrameSyncData, 0, sizeof(ccu_ae_frame_sync_data));
        rCcuFrameSyncData.is_zooming = pcontrolInfo->bZoomChange;
        rCcuFrameSyncData.magic_num = pcontrolInfo->u4HwMagicNum;
        rCcuFrameSyncData.do_skip = pcontrolInfo->bskipCCUAlgo;
        rCcuFrameSyncData.do_manual = pcontrolInfo->bManualAE;
        rCcuFrameSyncData.manual_exp.u4Eposuretime = pcontrolInfo->u4ManualExp;
        rCcuFrameSyncData.manual_exp.u4AfeGain = pcontrolInfo->u4ManualAfeGain;
        rCcuFrameSyncData.manual_exp.u4IspGain = pcontrolInfo->u4ManualISPGain;
        rCcuFrameSyncData.manual_exp.u4ISO = pcontrolInfo->u4ManualISO;
        rCcuFrameSyncData.manual_exp.m_u4Index = pcontrolInfo->u4CurrentIndex& 0x0000ffff;
        rCcuFrameSyncData.manual_exp.m_u4IndexF = pcontrolInfo->u4CurrentIndexF;
        rCcuFrameSyncData.algo_frame_data = pcontrolInfo->mFrameData;
        rCcuFrameSyncData.force_reset_ae_status = pcontrolInfo->bForceResetCCUStable;

        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] is_zooming:0x%x skip:%d\n", __FUNCTION__, rCcuFrameSyncData.is_zooming, rCcuFrameSyncData.do_skip);
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] magic_num:0x%x\n", __FUNCTION__, rCcuFrameSyncData.magic_num);

        // Fill APAE data
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] CCU_ZoomRatio:%d Nvram index:%d GyroRot[2]:%d\n", __FUNCTION__,
            pcontrolInfo->i4ZoomRatio, pcontrolInfo->u4AENVRAMIdx, rCcuFrameSyncData.algo_frame_data.m_eAEFDArea.GyroRot[2]);
        ccu_ae_roi rAPAERoi;
        memset(&rAPAERoi, 0, sizeof(ccu_ae_roi));
        rAPAERoi.AEWinBlock_u4XHi = pcontrolInfo->rAEStatCfg.m_AEWinBlock.u4XHi;
        rAPAERoi.AEWinBlock_u4XLow = pcontrolInfo->rAEStatCfg.m_AEWinBlock.u4XLow;
        rAPAERoi.AEWinBlock_u4YHi = pcontrolInfo->rAEStatCfg.m_AEWinBlock.u4YHi;
        rAPAERoi.AEWinBlock_u4YLow = pcontrolInfo->rAEStatCfg.m_AEWinBlock.u4YLow;
        rCcuFrameSyncData.algo_frame_data.m_AEWinBlock.u4XHi = pcontrolInfo->rAEStatCfg.m_AEWinBlock.u4XHi;
        rCcuFrameSyncData.algo_frame_data.m_AEWinBlock.u4XLow = pcontrolInfo->rAEStatCfg.m_AEWinBlock.u4XLow;
        rCcuFrameSyncData.algo_frame_data.m_AEWinBlock.u4YHi = pcontrolInfo->rAEStatCfg.m_AEWinBlock.u4YHi;
        rCcuFrameSyncData.algo_frame_data.m_AEWinBlock.u4YLow = pcontrolInfo->rAEStatCfg.m_AEWinBlock.u4YLow;
        rCcuFrameSyncData.algo_frame_data.eAEMeteringMode = (CCU_LIB3A_AE_METERING_MODE_T)pcontrolInfo->u4AEMeteringMode;
        //rCcuFrameSyncData.algo_frame_data.u4NvramIdx = pcontrolInfo->i4ZoomRatio, pcontrolInfo->u4AENVRAMIdx; /* fix build warning */
        rCcuFrameSyncData.algo_frame_data.m_u4MeteringCnt = pcontrolInfo->mFrameData.m_u4MeteringCnt;
        rCcuFrameSyncData.algo_frame_data.m_AETOUCHWinBlock[0].u4XHi = pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4XHi;
        rCcuFrameSyncData.algo_frame_data.m_AETOUCHWinBlock[0].u4XLow = pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4XLow;
        rCcuFrameSyncData.algo_frame_data.m_AETOUCHWinBlock[0].u4YHi = pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4YHi;
        rCcuFrameSyncData.algo_frame_data.m_AETOUCHWinBlock[0].u4YLow = pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4YLow;
        rCcuFrameSyncData.algo_frame_data.m_AETOUCHWinBlock[0].u4Weight = pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4Weight;
        //m_3ALogEnable = 1;
        AE_FLOW_LOG_IF(m_3ALogEnable, "mgrawb %x\n", rCcuFrameSyncData.algo_frame_data.pAAOProcInfo.awb_gain.i4R);
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] TouchArea u4XHi:%x u4XLow:%x u4YHi:%x u4YLow:%x u4Weight:%x\n", __FUNCTION__,
            pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4XHi, pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4XLow,
            pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4YHi, pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4YLow,
            pcontrolInfo->mFrameData.m_AETOUCHWinBlock[0].u4Weight);
        //m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AE_EV, &(pcontrolInfo->eAEEVcomp), NULL);
        /*if (pcontrolInfo->bskipCCUAlgo){
            rSkipAlgoData.do_skip = MTRUE;
        }else{
            rSkipAlgoData.do_skip = MFALSE;
        }*/
        //AE_LOG_IF(m_3ALogEnable, "[%s()]rSkipAlgoData.do_skip:0x%x\n", __FUNCTION__, rSkipAlgoData.do_skip);
        //m_pICcuAe->ccuControl(MSG_TO_CCU_SET_SKIP_ALGO, &rSkipAlgoData, NULL);
        m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AP_AE_FRAME_SYNC_DATA, &rCcuFrameSyncData, NULL);
        if (m_bSetCCUFrameRate && (!bIsDoPvAEControlCCU))
        {
            ccu_max_framerate_data max_framerate_data;
            max_framerate_data.framerate = (MUINT16)pcontrolInfo->i4AEMaxFps;
            max_framerate_data.min_framelength_en = MTRUE;
            AE_FLOW_LOG("[%s()] m_i4AEMaxFps:0x%x\n", __FUNCTION__, max_framerate_data.framerate);
            m_pICcuAe->ccuControl(MSG_TO_CCU_SET_MAX_FRAMERATE, &max_framerate_data, NULL);
            m_bSetCCUFrameRate = MFALSE;
        }
        //AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()]rAPAERoi.AEWinBlock_u4XHi:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4XHi);
        //AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()]rAPAERoi.AEWinBlock_u4XLow:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4XLow);
        //AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()]rAPAERoi.AEWinBlock_u4YHi:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4YHi);
        //AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()]rAPAERoi.AEWinBlock_u4YLow:0x%x\n", __FUNCTION__, rAPAERoi.AEWinBlock_u4YLow);
        //m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AE_ROI, &rAPAERoi, NULL);
    }
}

MVOID
AeFlowCCU::
PresetControlCCU(AE_FLOW_CONTROL_INFO_T * pcontrolInfo)
{
    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
    //m_bIsDoPvAEControlCCU = MFALSE;
    controlCCU(pcontrolInfo, MFALSE);
}

MVOID
AeFlowCCU::
getCCUResult(AE_CORE_MAIN_OUT * poutput, MBOOL isCCUResultGet)
{
    if (IsCCUWorking() && IsCCUAEWorking())
    {
        struct ccu_ae_output output;

        m_pICcuAe->ccuControl(MSG_TO_CCU_GET_CCU_OUTPUT, NULL, &output);

        memcpy(poutput, &output, sizeof(AE_CORE_MAIN_OUT));

        AAA_TRACE_D("getCCUResult (%d)", output.exp_set_stat);
        if (isCCUResultGet)
            m_bIsCCUResultGet = output.exp_set_stat;
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] ExpSetStat:%d IndexF:%d isDoPvAE:%d\n", __FUNCTION__, m_bIsCCUResultGet, poutput->a_Output.u4IndexF, isCCUResultGet);
        AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] Index/RealBV/BVx1000/BV/AoeBV/EV: %d/%d/%d/%d/%d/%d\n", __FUNCTION__,
            poutput->a_Output.u4Index, poutput->a_Output.i4RealBV, poutput->a_Output.i4RealBVx1000,
            poutput->a_Output.Bv, poutput->a_Output.AoeCompBv, poutput->a_Output.i4EV);//, poutput->enumCpuAction, poutput->enumCcuAction);
        AAA_TRACE_END_D;
    }
}

MVOID
AeFlowCCU::
setupParamPvAE(MVOID * input, MVOID * output)
{
    startCCUAE();
    //m_bIsDoPvAEControlCCU = MTRUE;
    controlCCU((AE_FLOW_CONTROL_INFO_T*) input, MTRUE);
    getCCUResult((AE_CORE_MAIN_OUT*) output, MTRUE);

    AE_FLOW_LOG_IF(m_3ALogEnable, "[%s()] SensorDev:%d\n", __FUNCTION__, m_eSensorDev);
}

MVOID
AeFlowCCU::
update(AE_CORE_CTRL_CPU_EXP_INFO * /* input */ ) /* fix build warning */
{
    if (IsCCUWorking() && IsCCUAEWorking())
    {
        //m_pICcuAe->ccuControl(MSG_TO_CCU_SET_AP_AE_OUTPUT, input, NULL);
    }
}

MVOID
AeFlowCCU::getdebuginfo(AE_DEBUG_INFO_T &exif_info)
{
    //m_pICcuAe->ccuControl(MSG_TO_CCU_GET_AE_DEBUG_INFO, NULL, &exif_info);
    m_pICcuAe->getDebugInfo(&exif_info);
}

MVOID
AeFlowCCU::getAEInfoForISP(AE_INFO_T &ae_info)
{
    //if (IsCCUWorking() && IsCCUAEWorking()){
    m_pICcuAe->getAeInfoForIsp(&ae_info);
    //}
}

MUINT
AeFlowCCU::
getSensorId(MUINT m_i4SensorIdx, MUINT m_eSensorDev)
{
    return m_i4SensorIdx;
}

MBOOL
AeFlowCCU::
IsCCUWorking()
{
    MBOOL res = MFALSE;
    if (m_bIsCCUStart)
    {
        res = MTRUE;
    }
    m_bIsCCUWorking = res;
    return res;
}

MBOOL
AeFlowCCU::
IsCCUAEWorking()
{
    return m_bIsCCUAEWorking;
}

AeFlowCCU::
~AeFlowCCU()
{
    AE_FLOW_LOG("[AeFlowCCU:Destruct] SensorDev:%d\n", (MUINT32)m_eSensorDev);
}

