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

/**
    this module support only CAM + DIP only.
    other module like FDVT CAMSV UNI... etc. need to re-pack its owner module via inheriting this module
*/
#ifndef _FDVT_DRV_H_
#define _FDVT_DRV_H_

//for ldvt feature/3a/mw build pass only, should be removed after ldvt phase


#include "utils/Mutex.h"    // android mutex
//#include "fdvt_reg.h"
#include "camera_fdvt.h"
#include <mtkcam/drv/def/fdvtcommon.h>
#include <vector>
#include "engine_drv.h"

using namespace android;
using namespace std;
using namespace NSCam;
using namespace NSIoPipe;

/**************************************************************************/
/*                      D E F I N E S / M A C R O S                       */
/**************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////

/**
    kernel device name
*/
#define FDVT_DRV_DEV_NAME            "/dev/camera-fdvt"

typedef unsigned int        MUINT32;
typedef unsigned short      MUINT16;
typedef unsigned char       MUINT8;

typedef intptr_t            MINTPTR;
typedef uintptr_t           MUINTPTR;


/**
    Register R/W IO ctrl
*/
typedef enum{
    FDVT_DRV_RW_IOCTL    = 0,    //RW register via IOCTRL
    FDVT_DRV_RW_MMAP     = 1,    //RW register via MMAP
    FDVT_DRV_R_ONLY      = 2,    //R only
}FDVT_DRV_RW_MODE;

/**
    R/W register structure
*/
typedef struct
{
    MUINT32     module;
    MUINT32     Addr;
    MUINT32     Data;
}FDVT_DRV_REG_IO_STRUCT;

/*
 * Engine Customization Control
 */
const ENGINE_CTRL fdvt_ctl[eENGINE_MAX] =
{
    {
      .id = eFDVT,
      .cmd    = ECmd_ENQUE_FEEDBACK,
      .int_st = FDVT_INT_ST,
    }
};

typedef struct
{
	FDVTMODE FD_MODE;
	MUINT16 MAX_SRC_Input_Width = 0;
	MUINT16 MAX_SRC_Input_Height = 0;
	MUINT16 SRC_Input_Width = 0;
	MUINT16 SRC_Input_Height = 0;
	FDVTFORMAT SRC_IMG_FMT;
	FDVTINPUTDEGREE INPUT_ROTATE_DEGREE;
	signed short RPN_ANCHOR_THRD = 5;
	MUINT16 pyramid_width;
	//int RPN_ANCHOR_THRD = 5;
	MUINT64 *source_img_address = NULL;
	MUINT64 *source_img_address_UV = NULL;

	MUINT64 *FDMODE_FD_Config_VA = NULL;
	MUINT64 *FDMODE_RS_Config_VA = NULL;
	MUINT64 *FDMODE_YUV2RGB_Config_VA = NULL;

	MUINT64 *ATTRMODE_FD_Config_VA[10] = {NULL};
	MUINT64 *ATTRMODE_YUV2RGB_Config_VA[10] = {NULL};

	MUINT64 *POSEMODE_FD_Config_VA[10] = {NULL};
	MUINT64 *POSEMODE_YUV2RGB_Config_VA[10] = {NULL};

	MUINT64 *RS_Pyramid0_R_Result_VA = NULL;
	MUINT64 *RS_Pyramid0_G_Result_VA = NULL;
	MUINT64 *RS_Pyramid0_B_Result_VA = NULL;
	MUINT64 *RS_Pyramid1_R_Result_VA = NULL;
	MUINT64 *RS_Pyramid1_G_Result_VA = NULL;
	MUINT64 *RS_Pyramid1_B_Result_VA = NULL;
	MUINT64 *RS_Pyramid2_R_Result_VA = NULL;
	MUINT64 *RS_Pyramid2_G_Result_VA = NULL;
	MUINT64 *RS_Pyramid2_B_Result_VA = NULL;

	MUINT32 *FDMODE_FD_Config_PA = NULL;
	MUINT32 *FDMODE_RS_Config_PA = NULL;
	MUINT32 *FDMODE_YUV2RGB_Config_PA = NULL;

	MUINT32 *ATTRMODE_FD_Config_PA[10] = {NULL};
	MUINT32 *ATTRMODE_YUV2RGB_Config_PA[10] = {NULL};

	MUINT32 *POSEMODE_FD_Config_PA[10] = {NULL};
	MUINT32 *POSEMODE_YUV2RGB_Config_PA[10] = {NULL};

	MUINT32 *RS_Pyramid0_R_Result_PA = NULL;
	MUINT32 *RS_Pyramid0_G_Result_PA = NULL;
	MUINT32 *RS_Pyramid0_B_Result_PA = NULL;
	MUINT32 *RS_Pyramid1_R_Result_PA = NULL;
	MUINT32 *RS_Pyramid1_G_Result_PA = NULL;
	MUINT32 *RS_Pyramid1_B_Result_PA = NULL;
	MUINT32 *RS_Pyramid2_R_Result_PA = NULL;
	MUINT32 *RS_Pyramid2_G_Result_PA = NULL;
	MUINT32 *RS_Pyramid2_B_Result_PA = NULL;
} FdDrv_Para;

typedef struct
{
	MUINT32 WriteIdx = 0;
	MUINT32 ReadIdx = 0;
	FDVTMODE FD_MODE[10];
	MUINT16 SRC_Input_Width[10] = {0};
	MUINT16 SRC_Input_Height[10] = {0};
	FDVTFORMAT SRC_IMG_FMT[10];
	FDVTINPUTDEGREE INPUT_ROTATE_DEGREE[10];
	MUINT64 *source_img_address[10] = {NULL};
	MUINT64 *source_img_address_UV[10] = {NULL};
} FdDrv_Attr_Para;

typedef struct
{
	MUINT32 WriteIdx = 0;
	MUINT32 ReadIdx = 0;
	FDVTMODE FD_MODE[10];
	MUINT16 SRC_Input_Width[10] = {0};
	MUINT16 SRC_Input_Height[10] = {0};
	FDVTFORMAT SRC_IMG_FMT[10];
	FDVTINPUTDEGREE INPUT_ROTATE_DEGREE[10];
	MUINT64 *source_img_address[10] = {NULL};
	MUINT64 *source_img_address_UV[10] = {NULL};
} FdDrv_Pose_Para;

typedef struct
{
	MUINT64 *fd_out_hw_VA[66][4] = {{NULL}};
 	MUINT64 *fd_kernel_VA[66][2] = {{NULL}};
 	MUINT64 *attr_out_hw_VA[18][4] = {{NULL}};
 	MUINT64 *attr_kernel_VA[18][2] = {{NULL}};
	MUINT64 *race_out_hw_VA[10] = {NULL};
	MUINT64 *gender_out_hw_VA[10] = {NULL};
 	MUINT64 *pose_out_hw_VA[18][4] = {{NULL}};
 	MUINT64 *pose_kernel_VA[18][2] = {{NULL}};
	MUINT64 *rip_out_hw_VA[10] = {NULL};
	MUINT64 *rop_out_hw_VA[10] = {NULL};

	MUINT32 *fd_out_hw_PA[66][4] = {{NULL}};
 	MUINT32 *fd_kernel_PA[66][2] = {{NULL}};
 	MUINT32 *attr_out_hw_PA[18][4] = {{NULL}};
 	MUINT32 *attr_kernel_PA[18][2] = {{NULL}};
	MUINT32 *race_out_hw_PA[10] = {NULL};
	MUINT32 *gender_out_hw_PA[10] = {NULL};
	MUINT32 *pose_out_hw_PA[18][4] = {{NULL}};
	MUINT32 *pose_kernel_PA[18][2] = {{NULL}};
	MUINT32 *rip_out_hw_PA[10] = {NULL};
	MUINT32 *rop_out_hw_PA[10] = {NULL};
} FdDrv_FD_DMA_Para;

/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
/**
    default accessible module: FDVT only
*/
class FdvtDrvImp : public EngineDrv<FDVTConfig>
{
    public:
                        ~FdvtDrvImp(void){};
                        FdvtDrvImp(void);

    public:

        static EngineDrv<FDVTConfig>*  getInstance();
        virtual void    destroyInstance(void){};

        virtual MBOOL   init(EGNInitInfo InitInfo, const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   waitIrq(FDVT_WAIT_IRQ_STRUCT* pWaitIrq);
        virtual MBOOL   clearIrq(FDVT_CLEAR_IRQ_STRUCT* pClearIrq);

        virtual MBOOL   readRegs(FDVT_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(FDVT_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);


        virtual MBOOL   waitFrameDone(unsigned int Status, MINT32 timeoutMs);
        virtual MBOOL   enque(vector<FDVTConfig>& ConfigVec);

        virtual MBOOL   deque(vector<FDVTConfig>& ConfigVec);

    public:
        static MINT32       m_Fd;
        static FDVT_DRV_RW_MODE     m_regRWMode;

    private:
        MUINT32 getRWMode(void);
        MBOOL setRWMode(FDVT_DRV_RW_MODE rwMode);

    private:

        static volatile MINT32     m_UserCnt;
        static MUINT32*     m_pFdvtHwRegAddr;

        static android::Mutex   m_FdvtInitMutex;
        mutable android::Mutex  FdvtRegMutex;

        //for dbg convenience,dump current user when uninit()
        static char m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support max user number: MAX_USER_NUMBER.

};

#endif  // _FDVT_DRV_H_
