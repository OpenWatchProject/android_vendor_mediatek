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
#ifndef _DPE_DRV_H_
#define _DPE_DRV_H_

//for ldvt feature/3a/mw build pass only, should be removed after ldvt phase


#include "utils/Mutex.h"    // android mutex
#include "dpe_reg.h"
#include "camera_dpe.h"
#include <mtkcam/drv/def/dpecommon_v20.h>
#include <vector>


using namespace android;
using namespace std;
using namespace NSCam;
using namespace NSIoPipe;

/**************************************************************************/
/*                      D E F I N E S / M A C R O S                       */
/**************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////
/**
    max user number supported by this module.
*/
#define MAX_USER_NUMBER     32
#define MAX_USER_NAME_SIZE  32

/**
    kernel device name
*/
#define DPE_DRV_DEV_NAME            "/dev/camera-dpe"

/**
    Register R/W IO ctrl
*/
typedef enum{
    DPE_DRV_RW_IOCTL    = 0,    //RW register via IOCTRL
    DPE_DRV_RW_MMAP     = 1,    //RW register via MMAP
    DPE_DRV_R_ONLY      = 2,    //R only
}DPE_DRV_RW_MODE;

/**
    R/W register structure
*/
typedef struct
{
    MUINT32     module;
    MUINT32     Addr;
    MUINT32     Data;
}DPE_DRV_REG_IO_STRUCT;


/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/

class DpeDrv
{
    protected:
        virtual         ~DpeDrv(){}
    public:
        static DpeDrv*  createInstance();
        virtual void    destroyInstance(void) = 0;
        virtual MBOOL   init(const char* userName="") = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;

        virtual MBOOL   waitIrq(DPE_WAIT_IRQ_STRUCT* pWaitIrq) = 0;
        virtual MBOOL   clearIrq(DPE_CLEAR_IRQ_STRUCT* pClearIrq) = 0;

        virtual MBOOL   readRegs(DPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0) = 0;
        virtual MBOOL   writeRegs(DPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0) = 0;

        virtual MBOOL   waitDPEFrameDone(unsigned int Status, MINT32 timeoutMs) = 0;
        virtual MBOOL   enqueDPE(vector<NSCam::NSIoPipe::DPEConfig>& DpeConfigVec) = 0;

        virtual MBOOL   dequeDPE(vector<NSCam::NSIoPipe::DPEConfig>& DpeConfigVec) = 0;

    public:
        static MINT32       m_Fd;
        static DPE_DRV_RW_MODE     m_regRWMode;

};

/**
    default accessible module: DPE only
*/
class DpeDrvImp : public DpeDrv
{
    public:
                        ~DpeDrvImp(void){}
                        DpeDrvImp(void);

    public:

        static DpeDrv*  getInstance();
        virtual void    destroyInstance(void){};

        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   waitIrq(DPE_WAIT_IRQ_STRUCT* pWaitIrq);
        virtual MBOOL   clearIrq(DPE_CLEAR_IRQ_STRUCT* pClearIrq);

        virtual MBOOL   readRegs(DPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(DPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);


        virtual MBOOL   waitDPEFrameDone(unsigned int Status, MINT32 timeoutMs);
        virtual MBOOL   enqueDPE(vector<NSCam::NSIoPipe::DPEConfig>& DpeConfigVec);

        virtual MBOOL   dequeDPE(vector<NSCam::NSIoPipe::DPEConfig>& DpeConfigVec);


    private:
        MUINT32 getRWMode(void);
        MBOOL setRWMode(DPE_DRV_RW_MODE rwMode);

    private:

        static volatile MINT32     m_UserCnt;
        static MUINT32*     m_pDpeHwRegAddr;

        static android::Mutex   m_DpeInitMutex;
        mutable android::Mutex  DpeRegMutex;

        //for dbg convenience,dump current user when uninit()
        static char m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support max user number: MAX_USER_NUMBER.


};

#endif  // _DPE_DRV_H_
