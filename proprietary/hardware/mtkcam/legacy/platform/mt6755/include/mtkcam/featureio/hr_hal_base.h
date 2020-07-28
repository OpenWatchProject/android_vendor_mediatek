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

#ifndef _HR_HAL_BASE_H_
#define _HR_HAL_BASE_H_

#include <mtkcam/common/faces.h>
#include <mtkcam/algorithm/libhrd/MTKHrd.h>
/*******************************************************************************
*
********************************************************************************/
typedef unsigned int MUINT32;
typedef int MINT32;
typedef unsigned char MUINT8;
typedef signed int    MBOOL;
//typedef uint64_t MUINT64;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef NULL
#define NULL 0
#endif


/*******************************************************************************
*
********************************************************************************/
enum HalHRObject_e {
    HAL_HR_OBJ_NONE = 0,
    HAL_HR_OBJ_SW,
    HAL_HR_OBJ_HW,
    HAL_HR_OBJ_UNKNOWN = 0xFF
} ;

typedef struct {
    MINT32			value;
    MINT32			quality;
    MINT32			isvalid;
    MINT32			hasPP;
    MINT32          percentage;
    MINT32          stoptype;
    MINT32          x1;
    MINT32          y1;
    MINT32          x2;
    MINT32          y2;
	MINT32          *aiWaveform;
	MINT32          prev_w;
	MINT32          prev_h;
	MINT32          facing;
} HR_RESULT;

/*******************************************************************************
*
********************************************************************************/
class halHRBase {
public:
    //
    static halHRBase* createInstance(HalHRObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halHRBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalHRInit () -
    //! \brief init heart rate detection
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halHRInit(MUINT32 fdW, MUINT32 fdH, MUINT8* WorkingBuffer, MUINT32 debug) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalHRDo () -
    //! \brief process heart rate detection 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halHRDo(MUINT8* ImageBuffer2, MINT32 rRotation_Info, MUINT64 timestamp, MINT32 face_num, MtkCameraFace face, MUINT32 mode, MUINT32 DoReset) {return 0;}
    
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalHRUninit () -
    //! \brief HRD uninit 
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halHRUninit() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // halHRGetResult () -
    //! \brief get hrd result
    //
    /////////////////////////////////////////////////////////////////////////    
    virtual MINT32 halHRGetResult(HR_RESULT *result) {return 0;}
};

class halHRTmp : public halHRBase {
public:
    //
    static halHRBase* getInstance();
    virtual void destroyInstance();
    //
    halHRTmp() {}; 
    virtual ~halHRTmp() {};
};

#endif

