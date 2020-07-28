/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifndef _CCT_HANDLE_H_
#define _CCT_HANDLE_H_

#include "nvram_drv.h"

//#include "AcdkTypes.h"
#include "AcdkCctBase.h"

#include <mtkcam/drv/IHalSensor.h>


//#include "CctIF.h"               // path:vendor/mediatek/proprietary/hardware/mtkcam/include/mtkcam/main/acdk
//#include "AcdkCommon.h"        // path:vendor/mediatek/proprietary/hardware/mtkcam/include/mtkcam/main/acdk

#include "cct_op_data.h"

#include <functional>

#include <utils/String8.h>

typedef enum {
    CCTIF_NO_ERROR         = 0,            ///< The function work successfully
    CCTIF_UNKNOWN_ERROR    = 0x80000000,   ///< Unknown error
    CCTIF_INVALID_DRIVER   = 0x80000001,
    CCTIF_UNSUPPORT_SENSOR_TYPE = 0x80000002,
    CCTIF_BAD_CTRL_CODE,
    CCTIF_BAD_PARAM,
    CCTIF_NOT_INIT,
} CCTIF_ERROR_ENUM;

#include "IHal3A.h"

using namespace NS3Av3;

/////////////////////////////////////////////////////////////////////////
typedef ACDK_CDVT_SENSOR_TEST_OUTPUT_T;
typedef ACDK_CDVT_RAW_ANALYSIS_RESULT_T;

class CctHandle
{
public:
    static CctHandle *createInstance(CAMERA_DUAL_CAMERA_SENSOR_ENUM eSensorEnum);
    void destroyInstance();

    MINT32 cct_OpDispatch(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn, MUINT32 u4ParaOutLen, MUINT8 *puParaOut, MUINT32 *pu4RealParaOutLen );
    MINT32 cct_InternalOp(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn, MUINT32 u4ParaOutLen, MUINT8 *puParaOut, MUINT32 *pu4RealParaOutLen );
    MINT32 cct_GetCctOutBufSize(CCT_OP_ID op, MINT32 dtype);
    MINT32 cct_GetCctOutBufSize(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn);
    void init(std::function<void(void*, MINT32)> capCB);

protected:
    //CctHandle();
    CctHandle( NVRAM_CAMERA_ISP_PARAM_STRUCT* pbuf_isp, NVRAM_CAMERA_SHADING_STRUCT* pbuf_shd, NVRAM_CAMERA_3A_STRUCT* pbuf_3a, NVRAM_LENS_PARA_STRUCT* pbuf_ln, NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT* pbuf_flash_cal, NvramDrvBase*	pnvram_drv, MUINT32 isensor_dev);
	~CctHandle();

private:
    MINT32  cct_getSensorStaticInfo();
    MINT32  cct_QuerySensor(MVOID *a_pCCTSensorInfoOut, MUINT32 *pRealParaOutLen);
    MINT32  cct_GetSensorRes(MVOID *pCCTSensorResOut, MUINT32 *pRealParaOutLen);
    MINT32  cct_ReadSensorReg(MVOID *puParaIn, MVOID *puParaOut, MUINT32 *pu4RealParaOutLen);
    MINT32  cct_WriteSensorReg(MVOID *puParaIn);
    MINT32  setIspOnOff(MUINT32 const u4Category, MBOOL const fgOn);
    MINT32  getIspOnOff(MUINT32 const u4Category, MBOOL& rfgOn) const;

    MVOID   setIspOnOff_SL2F(MBOOL const fgOn);
    MVOID   setIspOnOff_DBS(MBOOL const fgOn);
    MVOID   setIspOnOff_ADBS(MBOOL const fgOn);
    MVOID   setIspOnOff_OBC(MBOOL const fgOn);
    MVOID   setIspOnOff_BPC(MBOOL const fgOn);
    MVOID   setIspOnOff_NR1(MBOOL const fgOn);
    MVOID   setIspOnOff_PDC(MBOOL const fgOn);
    MVOID   setIspOnOff_RMM(MBOOL const fgOn);
    MVOID   setIspOnOff_RNR(MBOOL const fgOn);
    MVOID   setIspOnOff_SL2(MBOOL const fgOn);
    MVOID   setIspOnOff_UDM(MBOOL const fgOn);
    MVOID   setIspOnOff_CCM(MBOOL const fgOn);
    //MVOID   setIspOnOff_CCM_CFC(MBOOL const fgOn);
    MVOID   setIspOnOff_GGM(MBOOL const fgOn);
    MVOID   setIspOnOff_IHDR_GGM(MBOOL const fgOn);
    MVOID   setIspOnOff_PCA(MBOOL const fgOn);
    MVOID   setIspOnOff_ANR(MBOOL const fgOn);
    MVOID   setIspOnOff_ANR2(MBOOL const fgOn);
    MVOID   setIspOnOff_BOK(MBOOL const fgOn);
    MVOID   setIspOnOff_CCR(MBOOL const fgOn);
    MVOID   setIspOnOff_HFG(MBOOL const fgOn);
    MVOID   setIspOnOff_EE(MBOOL const fgOn);
    MVOID   setIspOnOff_NR3D(MBOOL const fgOn);
    MVOID   setIspOnOff_MFB(MBOOL const fgOn);
    MVOID   setIspOnOff_MIXER3(MBOOL const fgOn);
    MVOID   setIspOnOff_COLOR(MBOOL const fgOn);
    MVOID   setIspOnOff_HLR(MBOOL const fgOn);
    MVOID   setIspOnOff_ABF(MBOOL const fgOn);

    MBOOL   getIspOnOff_SL2F() const;
    MBOOL   getIspOnOff_DBS() const;
    MBOOL   getIspOnOff_ADBS() const;
    MBOOL   getIspOnOff_OBC() const;
    MBOOL   getIspOnOff_BPC() const;
    MBOOL   getIspOnOff_NR1() const;
    MBOOL   getIspOnOff_PDC() const;
    MBOOL   getIspOnOff_RMM() const;
    MBOOL   getIspOnOff_RNR() const;
    MBOOL   getIspOnOff_SL2() const;
    MBOOL   getIspOnOff_UDM() const;
    MBOOL   getIspOnOff_CCM() const;
    //MBOOL   getIspOnOff_CCM_CFC() const;
    MBOOL   getIspOnOff_GGM() const;
    MBOOL   getIspOnOff_IHDR_GGM() const;
    MBOOL   getIspOnOff_PCA() const;
    MBOOL   getIspOnOff_ANR() const;
    MBOOL   getIspOnOff_ANR2() const;
    MBOOL   getIspOnOff_CCR() const;
    MBOOL   getIspOnOff_BOK() const;
    MBOOL   getIspOnOff_HFG() const;
    MBOOL   getIspOnOff_EE() const;
    MBOOL   getIspOnOff_NR3D() const;
    MBOOL   getIspOnOff_MFB() const;
    MBOOL   getIspOnOff_MIXER3() const;
    MBOOL   getIspOnOff_COLOR() const;
    MBOOL   getIspOnOff_HLR() const;
    MBOOL   getIspOnOff_ABF() const;

    MINT32 cct_HandleSensorOp(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn, MUINT32 u4ParaOutLen, MUINT8 *puParaOut, MUINT32 *pu4RealParaOutLen );
    MINT32 cct_Handle3AOp(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn, MUINT32 u4ParaOutLen, MUINT8 *puParaOut, MUINT32 *pu4RealParaOutLen );
    MINT32 cct_HandleIspOp(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn, MUINT32 u4ParaOutLen, MUINT8 *puParaOut, MUINT32 *pu4RealParaOutLen );
    MINT32 cct_HandleNvramOp(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn, MUINT32 u4ParaOutLen, MUINT8 *puParaOut, MUINT32 *pu4RealParaOutLen );
    MINT32 cct_HandleCalibrationOp(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn, MUINT32 u4ParaOutLen, MUINT8 *puParaOut, MUINT32 *pu4RealParaOutLen );
    MINT32 cct_HandleEmcamOp(CCT_OP_ID op, MUINT32 u4ParaInLen, MUINT8 *puParaIn, MUINT32 u4ParaOutLen, MUINT8 *puParaOut, MUINT32 *pu4RealParaOutLen );

    MINT32 cctNvram_GetNvramData(CCT_NVRAM_DATA_T dataType, MINT32 inSize, MUINT8 *pInBuf, MINT32 outSize, void *pOutBuf, MINT32 *pRealOutSize);
    MINT32 cctNvram_SetNvramData(CCT_NVRAM_DATA_T dataType, MINT32 inSize, MUINT8 *pInBuf);
    MINT32 cctNvram_SetPartialNvramData(CCT_NVRAM_DATA_T dataType, MINT32 inSize, MUINT32 bufOffset, MUINT8 *pInBuf);
    MINT32 cctNvram_SaveNvramData(CCT_NVRAM_DATA_T dataType);
    MINT32 cctNvram_SetIspNvramData(ISP_NVRAM_REGISTER_STRUCT *pIspRegs);
    MINT32 cctNvram_GetNvramStruct(MINT32 inSize, MINT32 *pInBuf, MINT32 outSize, MINT32 *pOutBuf, MINT32 *pRealOutSize);

    /////////////////////////////////////////////////////////////////////////
    MUINT32 SetAFMode(MBOOL isAutoMode);
    MRESULT UnpackRAWImage();
    /////////////////////////////////////////////////////////////////////////
    MRESULT AnalyzeRAWImage(ACDK_CDVT_RAW_ANALYSIS_RESULT_T &RawAnlysRslt);
    MRESULT CalculateSlope(DOUBLE a_dX0,
                           DOUBLE a_dY0,
                           DOUBLE a_dX1,
                           DOUBLE a_dY1,
                           DOUBLE a_dX2,
                           DOUBLE a_dY2,
                           DOUBLE a_dX3,
                           DOUBLE a_dY3,
                           DOUBLE &a_dSlope);


    /////////////////////////////////////////////////////////////////////////
    // shading
    /////////////////////////////////////////////////////////////////////////
    MBOOL getShadingOnOff();
    MRESULT setShadingOnOff(MBOOL fgOnOff);
    MBOOL GetShadingRaw(eACDK_CAMERA_MODE mode, UINT8 /*ColorTemp*/);
    MRESULT SetShadingAE(MINT32 i4SensorDev);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrExpLinearity () -
    //!  brief exposure time linearity test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrExpLinearity(INT32 a_i4Gain,
                           INT32 a_i4ExpMode,
                           INT32 a_i4ExpStart,
                           INT32 a_i4ExpEnd,
                           INT32 a_i4ExpInterval,
                           INT32 a_i4PreCap,
                           ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrGainLinearity () -
    //!  @brief sensor gain linearity test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrGainLinearity(INT32 a_i4ExpTime,
                            INT32 a_i4GainStart,
                            INT32 a_i4GainEnd,
                            INT32 a_i4GainInterval,
                            INT32 a_i4PreCap,
                            ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrGainTableLinearity () -
    //!  @brief sensor gain linearity test (gain table)
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrGainTableLinearity(INT32 a_i4ExpTime,
                                 INT32 a_i4GainTableSize,
                                 INT32 *a_pi4GainTable,
                                 INT32 a_i4PreCap,
                                 ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrOBStability () -
    //!  brief OB stability test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrOBStability(INT32 a_i4ExpTime,
                                                   INT32 a_i4GainStart,
                                                   INT32 a_i4GainEnd,
                                                   INT32 a_i4GainInterval,
                                                   INT32 a_i4PreCap,
                                                   ACDK_CDVT_SENSOR_TEST_OUTPUT_T *prSensorTestOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrGainTableOBStability () -
    //!  brief OB stability test (gain table)
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrGainTableOBStability(INT32 a_i4ExpTime,
                                   INT32 a_i4GainTableSize,
                                   INT32 *a_pi4GainTable,
                                   INT32 a_i4PreCap,
                                   ACDK_CDVT_SENSOR_TEST_OUTPUT_T *a_prSensorTestOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalOB () -
    //!  brief OB calibration
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalOB(INT32 a_i4ExpTime,
                    INT32 a_i4Gain,
                    INT32 a_i4RepeatTimes,
                    INT32 a_i4PreCap,
                    ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalMinISO () -
    //!  brief minimum ISO calibration
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalMinISO(INT32 a_i4LV,
                        INT32 a_i4FNo,
                        INT32 a_i4OBLevel,
                        INT32 a_i450Hz60Hz,
                        INT32 a_i4PreCap,
                        ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalMinimumSaturationGain () -
    //!  brief minimum saturation gain calibration
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalMinimumSaturationGain(INT32 a_i4TargetDeclineRate,
                                       INT32 a_i4GainBuffer,
                                       INT32 a_i4OBLevel,
                                       INT32 a_i450Hz60Hz,
                                       INT32 a_i4PreCap,
                                       ACDK_CDVT_SENSOR_CALIBRATION_OUTPUT_T *prSensorCalibrationOutput);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrCalLenShading () -
    //!  @brief calibrate lens shading  test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrCalLenShading(INT32 a_i4XINIBorder,
                            INT32 a_i4XENDBorder,
                            INT32 a_i4YINIBorder,
                            INT32 a_i4YENDBorder,
                            UINT16 a_u2AttnRatio,
                            UINT32 a_u4Index,
                            INT32 a_i4PreCap,
                            UINT8 a_u1FixShadingIndex,
                            INT32 a_i4AvgWinSize);

    /////////////////////////////////////////////////////////////////////////
    //
    //   mrAEPlineTableLinearity () -
    //!  @brief AE Pline table clibration test
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT mrAEPlineTableLinearity(ACDK_CDVT_AE_PLINE_TEST_INPUT_T* in,
                                    int inSize,
                                    ACDK_CDVT_AE_PLINE_TEST_OUTPUT_T* out,
                                    int outSize,
                                    MUINT32* realOutSize);

    /////////////////////////////////////////////////////////////////////////
    //
    //   takePicture () -
    //!  Take Picture & Back to Preview
    //!
    /////////////////////////////////////////////////////////////////////////
    MRESULT TakePicture(MUINT32 a_i4SensorMode, MUINT32 a_i4OutFormat, MINT32 expTime);


    MRESULT TESTFUNC();


    int rawWidth;
    int rawHeight;
    int rawBitDepth;
    int rawColorOrder;
    // unpack buffer
    MUINT8 *rawUnpackBuf;
    int unpackBufSize;
    // packed file
    char rawFilePath[1024];
    int packFileSize;

    std::function<void(void*, MINT32)> capturCallBack;

private:
    CAMERA_DUAL_CAMERA_SENSOR_ENUM const m_eSensorEnum;

     MINT32  mSensorDev;
    NSCam::SensorStaticInfo m_SensorStaticInfo;
    MBOOL   m_bGetSensorStaticInfo;
    IHal3A *m_pIHal3A;

    NvramDrvBase*    m_pNvramDrv;

    NVRAM_CAMERA_ISP_PARAM_STRUCT&  m_rBuf_ISP;

    ISP_NVRAM_COMMON_STRUCT&        m_rISPComm;
    ISP_NVRAM_REGISTER_STRUCT&      m_rISPRegs;
    ISP_NVRAM_REG_INDEX_T&          m_rISPRegsIdx;
//    ISP_NVRAM_PCA_STRUCT&           m_rISPPca;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Shading
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    NVRAM buffer.

    NVRAM_CAMERA_SHADING_STRUCT&    m_rBuf_SD;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    3A (AE,AWB)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    NVRAM buffer.

    NVRAM_CAMERA_3A_STRUCT&    m_rBuf_3A;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    Lens (AF)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    NVRAM buffer.

    NVRAM_LENS_PARA_STRUCT&    m_rBuf_LN;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Flash Calibration
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:    ////    NVRAM buffer.

    NVRAM_CAMERA_FLASH_CALIBRATION_STRUCT& m_rBuf_FC;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//    EM Mode
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:
    MINT32 mEmFullScanInterval = 1;
    MINT32 mEmFullScanDacStep = 1;
    MINT32 mEmCaptureIso = 0;
    MINT32 mEmCaptureSensorGain = 0;
    MINT32 mEmCaptureGainMode = 0;      // 0:use Sensor Gain  1:use ISO
    MINT32 mEmCaptureExpTimeUs = 0; // unit: us
    android::String8 mEmResultPath = android::String8("/sdcard/DCIM/result.txt");
};


#endif

