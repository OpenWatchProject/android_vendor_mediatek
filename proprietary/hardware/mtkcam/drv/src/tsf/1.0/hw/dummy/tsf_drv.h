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
#ifndef _TSF_DRV_H_
#define _TSF_DRV_H_

//for ldvt feature/3a/mw build pass only, should be removed after ldvt phase


#include "utils/Mutex.h"    // android mutex
#include "tsfcommon.h"
#include <vector>


using namespace android;
using namespace std;
using namespace NSCam;
using namespace NSIoPipe;

#define TSF_INT_ST           (1<<0)
struct TSF_REG_STRUCT {
	unsigned int module;
	unsigned int Addr;	/* register's addr */
	unsigned int Val;	/* register's value */
};

struct TSF_REG_IO_STRUCT {
	struct TSF_REG_STRUCT *pData;	/* pointer to TSF_REG_STRUCT */
	unsigned int Count;	/* count */
};

enum TSF_IRQ_CLEAR_ENUM {
	TSF_IRQ_CLEAR_NONE,	/*non-clear wait, clear after wait */
	TSF_IRQ_CLEAR_WAIT,	/*clear wait, clear before and after wait */
	TSF_IRQ_WAIT_CLEAR,	/*wait the signal and clear it, avoid the hw executime is too s hort. */
	TSF_IRQ_CLEAR_STATUS,	/*clear specific status only */
	TSF_IRQ_CLEAR_ALL	/*clear all status */
};

/**
* module's interrupt , each module should have its own isr.
* note:
* mapping to isr table,ISR_TABLE when using no device tree
*/
enum TSF_IRQ_TYPE_ENUM {
	TSF_IRQ_TYPE_INT_TSF_ST,	/* TSF */
	TSF_IRQ_TYPE_AMOUNT
};

struct TSF_WAIT_IRQ_STRUCT {
	enum TSF_IRQ_CLEAR_ENUM Clear;
	enum TSF_IRQ_TYPE_ENUM Type;
	unsigned int Status;	/*IRQ Status */
	unsigned int Timeout;
	int UserKey;		/* user key for doing interrupt operation */
	int ProcessID;		/* user ProcessID (will filled in kernel) */
	unsigned int bDumpReg;	/* check dump register or not */
};

struct TSF_CLEAR_IRQ_STRUCT {
	enum TSF_IRQ_TYPE_ENUM Type;
	int UserKey;		/* user key for doing interrupt operation */
	unsigned int Status;	/*Input */
};

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
#define TSF_DRV_DEV_NAME            "/dev/camera-tsf"

/**
    Register R/W IO ctrl
*/
typedef enum{
    TSF_DRV_RW_IOCTL    = 0,    //RW register via IOCTRL
    TSF_DRV_RW_MMAP     = 1,    //RW register via MMAP
    TSF_DRV_R_ONLY      = 2,    //R only
}TSF_DRV_RW_MODE;

/**
    R/W register structure
*/
typedef struct
{
    MUINT32     module;
    MUINT32     Addr;
    MUINT32     Data;
}TSF_DRV_REG_IO_STRUCT;


#define TSF_RESET_TIMEOUT                (1)
#define TSF_START_OFFSET                 (0xC00)
#define TSF_INT_EN_OFFSET                (0xC04)
#define TSF_INT_OFFSET                   (0xC08)
#define TSF_CTRL_OFFSET                  (0xC0C)

#define TSF_COEFF_1_OFFSET               (0xC10)
#define TSF_COEFF_2_OFFSET               (0xC14)
#define TSF_COEFF_3_OFFSET               (0xC18)
#define TSF_COEFF_4_OFFSET               (0xC1C)
#define TSF_CRYPTION_OFFSET              (0xC20)

#define TSFO_BASE_ADDR_OFFSET            (0x460)
#define TSFO_OFST_ADDR_OFFSET            (0x468)
#define TSFO_XSIZE_OFFSET                (0x470)
#define TSFO_YSIZE_OFFSET                (0x474)
#define TSFO_STRIDE_OFFSET               (0x478)
#define TSFO_CROP_OFFSET                 (0x488)

#define TSFI_BASE_ADDR_OFFSET            (0x4c0)
#define TSFI_OFST_ADDR_OFFSET            (0x4c8)
#define TSFI_XSIZE_OFFSET                (0x4d0)
#define TSFI_YSIZE_OFFSET                (0x4d4)
#define TSFI_STRIDE_OFFSET               (0x4d8)


#define TSF_DMA_BUS_SIZE_08BIT            0x0)
#define TSF_DMA_BUS_SIZE_16BIT            (0x1 << 16 )
#define TSF_DMA_BUS_SIZE_24BIT            (0x2 << 16 )
#define TSF_DMA_BUS_SIZE_32BIT            (0x3 << 16 )
#define TSF_DMA_BUS_SIZE_48BIT            (0x4 << 16 )

#define TSF_DMA_BUS_SIZE_EN               (0x1 << 24 )

#define TSF_MEMALIGNED 16
#define TSF_MEMALIGNED_MINUS1 (TSF_MEMALIGNED - 1)


/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/

class TsfDrv
{
    protected:
        virtual         ~TsfDrv(){}
    public:
        static TsfDrv*  createInstance();
        virtual void    destroyInstance(void) = 0;
        virtual MBOOL   init(const char* userName="") = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;

        virtual MBOOL   waitIrq(TSF_WAIT_IRQ_STRUCT* pWaitIrq) = 0;
        virtual MBOOL   clearIrq(TSF_CLEAR_IRQ_STRUCT* pClearIrq) = 0;

        virtual MBOOL   readRegs(TSF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0) = 0;
        virtual MBOOL   writeRegs(TSF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0) = 0;
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0) = 0;

        virtual MBOOL   waitTSFFrameDone(unsigned int Status, MINT32 timeoutMs) = 0;

        virtual MBOOL   enqueTSF(TSFConfig const& rTSFConfig) = 0;

    public:
        static MINT32       m_Fd;
        static TSF_DRV_RW_MODE     m_regRWMode;

};

/**
    default accessible module: TSF only
*/
class TsfDrvImp : public TsfDrv
{
    public:
                        ~TsfDrvImp(void){}
                        TsfDrvImp(void);

    public:

        static TsfDrv*  getInstance();
        virtual void    destroyInstance(void){};

        virtual MBOOL   init(const char* userName="");
        virtual MBOOL   uninit(const char* userName="");

        virtual MBOOL   waitIrq(TSF_WAIT_IRQ_STRUCT* pWaitIrq);
        virtual MBOOL   clearIrq(TSF_CLEAR_IRQ_STRUCT* pClearIrq);

        virtual MBOOL   readRegs(TSF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MUINT32 readReg(MUINT32 Addr,MINT32 caller=0);
        virtual MBOOL   writeRegs(TSF_DRV_REG_IO_STRUCT*  pRegIo,MUINT32 Count,MINT32 caller=0);
        virtual MBOOL   writeReg(MUINT32 Addr,unsigned long Data,MINT32 caller=0);
        virtual MBOOL   waitTSFFrameDone(unsigned int Status, MINT32 timeoutMs);

        virtual MBOOL   enqueTSF(TSFConfig const& rTSFConfig);

    private:
        MUINT32 getRWMode(void);
        MBOOL setRWMode(TSF_DRV_RW_MODE rwMode);

    private:

        static volatile MINT32     m_UserCnt;
        static MUINT32*     m_pTsfHwRegAddr;
        //static MUINT32*     m_pCCUHwRegAddr;

        static android::Mutex   m_TsfInitMutex;
        mutable android::Mutex  TsfRegMutex;

        //for dbg convenience,dump current user when uninit()
        static char m_UserName[MAX_USER_NUMBER][MAX_USER_NAME_SIZE]; //support max user number: MAX_USER_NUMBER.


};

#endif  // _TSF_DRV_H_
