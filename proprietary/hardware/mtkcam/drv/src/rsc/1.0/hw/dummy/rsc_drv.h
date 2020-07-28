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
#ifndef _RSC_DRV_H_
#define _RSC_DRV_H_

//for ldvt feature/3a/mw build pass only, should be removed after ldvt phase


#include "utils/Mutex.h"    // android mutex
#include "rsc_reg.h"
#include <mtkcam/drv/def/rsccommon.h>
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
#define RSC_DRV_DEV_NAME            "/dev/camera-rsc"

/**
    Register R/W IO ctrl
*/
typedef enum{
    RSC_DRV_RW_IOCTL    = 0,    //RW register via IOCTRL
    RSC_DRV_RW_MMAP     = 1,    //RW register via MMAP
    RSC_DRV_R_ONLY      = 2,    //R only
}RSC_DRV_RW_MODE;

/**
    R/W register structure
*/
typedef struct
{
    MUINT32     module;
    MUINT32     Addr;
    MUINT32     Data;
}RSC_DRV_REG_IO_STRUCT;

typedef enum {
	RSC_IRQ_CLEAR_NONE,	/* non-clear wait, clear after wait */
	RSC_IRQ_CLEAR_WAIT,	/* clear wait, clear before and after wait */
	RSC_IRQ_WAIT_CLEAR,	/* wait the signal and clear it, avoid the hw executime is too s hort. */
	RSC_IRQ_CLEAR_STATUS,	/* clear specific status only */
	RSC_IRQ_CLEAR_ALL	/* clear all status */
} RSC_IRQ_CLEAR_ENUM;

typedef enum {
	RSC_IRQ_TYPE_INT_RSC_ST,	/* RSC */
	RSC_IRQ_TYPE_AMOUNT
} RSC_IRQ_TYPE_ENUM;

typedef struct {
	RSC_IRQ_CLEAR_ENUM Clear;
	RSC_IRQ_TYPE_ENUM Type;
	unsigned int Status;	/*IRQ Status */
	unsigned int Timeout;
	int UserKey;		/* user key for doing interrupt operation */
	int ProcessID;		/* user ProcessID (will filled in kernel) */
	unsigned int bDumpReg;	/* check dump register or not */
} RSC_WAIT_IRQ_STRUCT;

typedef struct {
	RSC_IRQ_TYPE_ENUM Type;
	int UserKey;		/* user key for doing interrupt operation */
	unsigned int Status;	/* Input */
} RSC_CLEAR_IRQ_STRUCT;
#define RSC_REG_RANGE           (0x1000)
#define RSC_INT_ST              ((unsigned int)1<<0)

/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/

class RscDrv
{
    protected:
        virtual         ~RscDrv(){}
    public:
        static RscDrv*  createInstance();
        virtual void    destroyInstance(void) = 0;
        virtual MBOOL   init(const char* userName="") = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;

        virtual MBOOL   waitIrq(RSC_WAIT_IRQ_STRUCT* pWaitIrq) = 0;
        virtual MBOOL   clearIrq(RSC_CLEAR_IRQ_STRUCT* pClearIrq) = 0;

        virtual MBOOL   readRegs(RSC_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0) = 0;
        virtual MBOOL   writeRegs(RSC_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0) = 0;

        virtual MBOOL   waitRSCFrameDone(unsigned int Status, MINT32 timeoutMs) = 0;
        virtual MBOOL   enqueRSC(vector<NSCam::NSIoPipe::RSCConfig>& RscConfigVec) = 0;

        virtual MBOOL   dequeRSC(vector<NSCam::NSIoPipe::RSCConfig>& RscConfigVec) = 0;

    public:
        static MINT32       m_Fd;
        static RSC_DRV_RW_MODE     m_regRWMode;

};

/**
    default accessible module: RSC only
*/
class RscDrvImp : public RscDrv
{
    public:
                        ~RscDrvImp(void){}
                        RscDrvImp(void);

    public:

        static RscDrv*  getInstance();
        virtual void    destroyInstance(void){};

        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   waitIrq(RSC_WAIT_IRQ_STRUCT* pWaitIrq);
        virtual MBOOL   clearIrq(RSC_CLEAR_IRQ_STRUCT* pClearIrq);

        virtual MBOOL   readRegs(RSC_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(RSC_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);


        virtual MBOOL   waitRSCFrameDone(unsigned int Status, MINT32 timeoutMs);
        virtual MBOOL   enqueRSC(vector<NSCam::NSIoPipe::RSCConfig>& RscConfigVec);

        virtual MBOOL   dequeRSC(vector<NSCam::NSIoPipe::RSCConfig>& RscConfigVec);


    private:
        MUINT32 getRWMode(void);
        MBOOL setRWMode(RSC_DRV_RW_MODE rwMode);

    private:

        static volatile MINT32     m_UserCnt;
        static MUINT32*     m_pRscHwRegAddr;

        static android::Mutex   m_RscInitMutex;
        mutable android::Mutex  RscRegMutex;

        //for dbg convenience,dump current user when uninit()
        static char m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support max user number: MAX_USER_NUMBER.


};

#endif  // _RSC_DRV_H_
