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
** $Log: fd_hal_base.h $
 *
*/

#ifndef _FD_HAL_BASE_H_
#define _FD_HAL_BASE_H_

//#include <system/camera.h>
//#include "MediaHal.h"
//#include <mhal/inc/camera/faces.h>
#include <faces.h>
/*******************************************************************************
*
********************************************************************************/
typedef unsigned int MUINT32;
typedef int MINT32;
typedef unsigned char MUINT8;
typedef signed int    MBOOL;
typedef intptr_t      MINTPTR;
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
enum HalFDMode_e {
    HAL_FD_MODE_FD = 0,
    HAL_FD_MODE_SD,
    HAL_FD_MODE_VFB,
    HAL_FD_MODE_CFB,
    HAL_FD_MODE_MANUAL
} ;

enum HalFDObject_e {
    HAL_FD_OBJ_NONE = 0,
    HAL_FD_OBJ_SW,
    HAL_FD_OBJ_HW,
    HAL_FD_OBJ_FDFT_SW,
    HAL_FD_OBJ_UNKNOWN = 0xFF
} ;

enum HalFDVersion_e {
    HAL_FD_VER_NONE = 0,
    HAL_FD_VER_HW36,
    HAL_FD_VER_HW37,
    HAL_FD_VER_SW36,
    HAL_FD_VER_HW40,
    HAL_FD_VER_HW41,
    HAL_FD_VER_HW42,
    HAL_FD_VER_HW43,
    HAL_FD_VER_HW50,
    HAL_FD_VER_HW51,
};


struct FD_RESULT {

    MINT32 rect[4];
    MINT32 score;
    MINT32 rop_dir;
    MINT32 rip_dir;
};

struct FD_Frame_Parameters {
    MUINT8 *pScaleImages;
    MUINT8 *pRGB565Image;
    MUINT8 *pPureYImage;
    MUINT8 *pImageBufferPhyP0; // Plane 0 of preview image physical address
    MUINT8 *pImageBufferPhyP1; // Plane 1 of preview image physical address
    MUINT8 *pImageBufferPhyP2; // Plane 2 of preview image physical address
    MUINT8 *pImageBufferVirtual;
    MINT32 Rotation_Info;
    MUINT8 SDEnable;
    MUINT8 AEStable;
    MUINT8 padding_w;
    MUINT8 padding_h;
    MINT32 gammaType;
    MINT32 LvValue;
};

/*******************************************************************************
*
********************************************************************************/
class halFDBase {
public:
    //
    static halFDBase* createInstance(HalFDObject_e eobject);
    virtual void      destroyInstance() = 0;
    virtual ~halFDBase() {};
    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDInit () -
    //! \brief init face detection
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDInit(MUINT32 /*fdW*/, MUINT32 /*fdH*/, MUINT8* /*WorkingBuffer*/, MUINT32 /*WorkingBufferSize*/, MBOOL, MUINT8 /*Current_mode*/, MINT32 FldNum = 1) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // halFDGetVersion () -
    //! \brief get FD version
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDGetVersion() {return HAL_FD_VER_NONE;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDDo () -
    //! \brief process face detection
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDDo(struct FD_Frame_Parameters &/*Param*/) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDUninit () -
    //! \brief fd uninit
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDUninit() {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDGetFaceInfo () -
    //! \brief get face detection result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual  MINT32 halFDGetFaceInfo(MtkCameraFaceMetadata* /*fd_info_result*/) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDGetFaceResult () -
    //! \brief get face detection result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDGetFaceResult(MtkCameraFaceMetadata* /*fd_result*/, MINT32 ResultMode = 1) {(void)ResultMode;return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalSDGetSmileResult () -
    //! \brief get smile detection result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halSDGetSmileResult( ) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // halFDYUYV2ExtractY () -
    //! \brief create Y Channel
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDYUYV2ExtractY(MUINT8* /*dstAddr*/, MUINT8* /*srcAddr*/, MUINT32 /*src_width*/, MUINT32 /*src_height*/) {return 0;}

    /////////////////////////////////////////////////////////////////////////
    //
    // halFTBufferCreate () -
    //! \brief create face tracking buffer
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFTBufferCreate(MUINT8* /*dstAddr*/, MUINT8* /*srcAddr*/, MUINT8 /*ucPlane*/, MUINT32 /*src_width*/, MUINT32 /*src_height*/) {return 0;}

    virtual MINT32 halGetFTBufferSize(MUINT32* /*width*/, MUINT32* /*height*/) {return 0;}

    virtual MINT32 halFTBufferCreateAsync(MUINT8* /*dstAddr*/, MUINT8* /*srcAddr*/, MUINT8 /*ucPlane*/, MUINT32 /*src_width*/, MUINT32 /*src_height*/) {return 0;}

    virtual MINT32 halGetGammaSetting(int*& /*gammaCtrl*/) {return 0;}

};

class halFDTmp : public halFDBase {
public:
    //
    static halFDBase* getInstance();
    virtual void destroyInstance();
    //
    halFDTmp() {};
    virtual ~halFDTmp() {};
};

#endif

