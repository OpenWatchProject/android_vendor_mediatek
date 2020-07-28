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
/*
** $Log: hr_hal_base.h $
 *
*/

#ifndef _HR_HAL_H_
#define _HR_HAL_H_

#define DUMP_IMAGE (0)
#define HRD_ENABLE (0)

#include <mtkcam3/feature/HeartRate/hr_hal_base.h>

//class MTKHeartrateDetector;
/*******************************************************************************
*
********************************************************************************/

struct MtkHeartrateMetadata {
    MINT32          value;
    MINT32          quality;
    MINT32          isvalid;
    MINT32          hasPP;
};

class halHR: public halHRBase
{
public:
    //
    static halHRBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halHRBase () -
    //! \brief HR Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////
    halHR();

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual ~halHR();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalHRInit () -
    //! \brief init heartrate detection
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halHRInit(MUINT32 fdW, MUINT32 fdH, MUINT8* WorkingBuffer, MUINT32 debug);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalHRDo () -
    //! \brief process heartrate detection
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halHRDo(MUINT8* ImageBuffer2, MINT32 rRotation_Info, uint64_t timestamp, MINT32 face_num, MtkCameraFace face, MUINT32 mode, MUINT32 DoReset);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalHRUninit () -
    //! \brief HR uninit
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halHRUninit();

    /////////////////////////////////////////////////////////////////////////
    //
    // halHRGetResult () -
    //! \brief get heartrate detection result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halHRGetResult(HR_RESULT *result);

protected:

    #if HRD_ENABLE
    MTKHrd* m_pMTKHRObj;
    HRD_INIT_STRUCT m_HRInitData;
    HRD_GET_PROC_INFO_STRUCT m_HRProcInfo;
    HRD_SET_WORKING_BUFFER_STRUCT m_HRWorkbufInfo;
    HRD_SET_PROC_INFO_STRUCT m_HRDataInfo;
    HRD_RESULT_INFO_STRUCT m_HRresult;
    #endif

    MUINT32 m_HRW = 0;
    MUINT32 m_HRH = 0;
    MUINT32 m_DispRoate = 0;
    MUINT32 m_RegisterBuff = 0;
    MUINT32 m_BuffCount = 0;
    MUINT8 m_DetectPara = 0;
    MUINT32 m_mode = 0;
    MUINT8 *m_pWorkingBuf = NULL;
};

#endif

