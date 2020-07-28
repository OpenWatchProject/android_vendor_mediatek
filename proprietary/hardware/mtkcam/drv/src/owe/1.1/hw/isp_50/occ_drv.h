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
#ifndef _OCC_DRV_H_
#define _OCC_DRV_H_

//for ldvt feature/3a/mw build pass only, should be removed after ldvt phase


#include "utils/Mutex.h"    // android mutex
#include "owe_reg.h"
#include "camera_owe.h"
#include <mtkcam/drv/def/owecommon.h>
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
#define OWE_DRV_DEV_NAME            "/dev/camera-owe"

/**
    Register R/W IO ctrl
*/
typedef enum{
    OWE_DRV_RW_IOCTL    = 0,    //RW register via IOCTRL
    OWE_DRV_RW_MMAP     = 1,    //RW register via MMAP
    OWE_DRV_R_ONLY      = 2,    //R only
}OWE_DRV_RW_MODE;

/**
    R/W register structure
*/
typedef struct
{
    MUINT32     module;
    MUINT32     Addr;
    MUINT32     Data;
}OWE_DRV_REG_IO_STRUCT;

/*
 * Engine Customization Control
 */
const ENGINE_CTRL occ_ctl[eENGINE_MAX] =
{
    {
      .id = eOCC,
      .cmd    = ECmd_ENQUE,
      .int_st = OWE_OCC_INT_ST,
    },

    {
      .id = eWMFE,
      .cmd    = ECmd_ENQUE,
      .int_st = OWE_WMFE_INT_ST,
    }

};

#define WMFE_TBLI_SIZE 256
/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
/**
    default accessible module: OWE only
*/
class OccDrvImp : public EngineDrv<OCCConfig>
{
    public:
                        ~OccDrvImp(void){}
                        OccDrvImp(void);

    public:

        static EngineDrv<OCCConfig>*  getInstance();
        virtual void    destroyInstance(void){};

        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   waitIrq(OWE_WAIT_IRQ_STRUCT* pWaitIrq);
        virtual MBOOL   clearIrq(OWE_CLEAR_IRQ_STRUCT* pClearIrq);

        virtual MBOOL   readRegs(OWE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(OWE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);


        virtual MBOOL   waitFrameDone(unsigned int Status, MINT32 timeoutMs);
        virtual MBOOL   enque(vector<NSCam::NSIoPipe::OCCConfig>& OccConfigVec);

        virtual MBOOL   deque(vector<NSCam::NSIoPipe::OCCConfig>& OccConfigVec);

    public:
        static MINT32       m_Fd;
        static OWE_DRV_RW_MODE     m_regRWMode;

    private:
        MUINT32 getRWMode(void);
        MBOOL setRWMode(OWE_DRV_RW_MODE rwMode);

    private:

        static volatile MINT32     m_UserCnt;
        static MUINT32*     m_pHwRegAddr;

        static android::Mutex   mInitMutex;
        mutable android::Mutex  RegMutex;

        //for dbg convenience,dump current user when uninit()
        static char m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support max user number: MAX_USER_NUMBER.

        virtual MBOOL   checkOCCSetting(NSCam::NSIoPipe::OCCConfig* pOccCfg);

};

#endif  // _OCC_DRV_H_
