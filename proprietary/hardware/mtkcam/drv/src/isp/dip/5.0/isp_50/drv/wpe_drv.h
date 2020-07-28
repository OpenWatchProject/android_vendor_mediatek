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
#ifndef _WPE_DRV_H_
#define _WPE_DRV_H_

//for ldvt feature/3a/mw build pass only, should be removed after ldvt phase


#include "utils/Mutex.h"    // android mutex
//#include "isp_reg.h"
#include "camera_wpe.h"
#include <mtkcam/drv/def/wpecommon.h>
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
#define WPE_DRV_DEV_NAME            "/dev/camera-wpe"

/**
    Register R/W IO ctrl
*/
typedef enum{
    WPE_DRV_RW_IOCTL    = 0,    //RW register via IOCTRL
    WPE_DRV_RW_MMAP     = 1,    //RW register via MMAP
    WPE_DRV_R_ONLY      = 2,    //R only
}WPE_DRV_RW_MODE;

/**
    R/W register structure
*/
typedef struct
{
    MUINT32     module;
    MUINT32     Addr;
    MUINT32     Data;
}WPE_DRV_REG_IO_STRUCT;

////////////////////////////////////////////////////////////////////////////////////////////////
/**
    Set/Get device information
*/
typedef enum{
    _SET_WPE_BUF_INFO          = (1L<<0),      //data size:
}E_WPE_DEVICE_INFO;


/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/

class WpeDrv
{
    protected:
        virtual         ~WpeDrv(){}
    public:
        static WpeDrv*  createInstance();
        virtual void    destroyInstance(void) = 0;
        virtual MBOOL   init(const char* userName="") = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;

        virtual MBOOL   waitIrq(WPE_WAIT_IRQ_STRUCT* pWaitIrq) = 0;
        virtual MBOOL   clearIrq(WPE_CLEAR_IRQ_STRUCT* pClearIrq) = 0;

        virtual MBOOL   readRegs(WPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0) = 0;
        virtual MBOOL   writeRegs(WPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0) = 0;

        virtual MBOOL   waitWPEFrameDone(unsigned int Status, MINT32 timeoutMs) = 0;
        virtual MBOOL   enqueWPE(vector<WPE_Config>& WpeConfigVec) = 0;

        virtual MBOOL   dequeWPE(vector<WPE_Config>& WpeConfigVec) = 0;
		virtual MBOOL	waitDeque() = 0;
		virtual MBOOL   setDeviceInfo(E_WPE_DEVICE_INFO eCmd,MUINT8* pData) = 0;

    public:
        static MINT32       m_Fd;
        static WPE_DRV_RW_MODE     m_regRWMode;

};

/**
    default accessible module: WPE only
*/
class WpeDrvImp : public WpeDrv
{
    public:
                        ~WpeDrvImp(void){}
                        WpeDrvImp(void);

    public:

        static WpeDrv*  getInstance();
        virtual void    destroyInstance(void){};

        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   waitIrq(WPE_WAIT_IRQ_STRUCT* pWaitIrq);
        virtual MBOOL   clearIrq(WPE_CLEAR_IRQ_STRUCT* pClearIrq);

        virtual MBOOL   readRegs(WPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(WPE_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);


        virtual MBOOL   waitWPEFrameDone(unsigned int Status, MINT32 timeoutMs);
        virtual MBOOL   enqueWPE(vector<WPE_Config>& WpeConfigVec);

        virtual MBOOL   dequeWPE(vector<WPE_Config>& WpeConfigVec);
		virtual MBOOL   waitDeque();
		virtual MBOOL   setDeviceInfo(E_WPE_DEVICE_INFO eCmd,MUINT8* pData);


    private:
        MUINT32 getRWMode(void);
        MBOOL setRWMode(WPE_DRV_RW_MODE rwMode);

    private:

        static volatile MINT32     m_UserCnt;
        static MUINT32*     m_pWpeHwRegAddr;

        static android::Mutex   m_WpeInitMutex;
        mutable android::Mutex  WpeRegMutex;

        //for dbg convenience,dump current user when uninit()
        static char m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support max user number: MAX_USER_NUMBER.


};

#endif  // _WPE_DRV_H_
