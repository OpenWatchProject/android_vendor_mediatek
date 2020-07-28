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
#ifndef _HWSYNC_DRV_IMP_H_
#define _HWSYNC_DRV_IMP_H_
//-----------------------------------------------------------------------------
//------------Thread-------------
//#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
//-------------------------------
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <utils/threads.h>
#include <list>
#include <vector>
//
#include <mtkcam/drv/IHwSyncDrv.h>
//-----------------------------------------------------------------------------
using namespace std;
using namespace android;
using namespace NSCam;
//-----------------------------------------------------------------------------


//#define SMI_CLOCK    (286000000)	///<smi clock, the reference clock of n3d related count,268.6666667
//#define REF_CLOCK    (SMI_CLOCK/100000)
//#define DIFF_THRECNT (SMI_CLOCK/1000)   //1000us -> 1000*smi/1000000
#define CVGTHRE_VDDIFF  (500)   //us
#define CVGTHRE_FPSDIFF (500)   //us


/******************************************************************************
 *
 * @enum HW_SYNC_STATE_ENUM
 * @brief cmd enum for sendCommand.
 * @details
 *
 ******************************************************************************/
typedef enum{
    HW_SYNC_STATE_NONE 			= 0X00,
    HW_SYNC_STATE_READY2RUN	    = 0X01,
    HW_SYNC_STATE_SYNCING 		= 0x02,
    HW_SYNC_STATE_READY2LEAVE 	= 0x03,
    HW_SYNC_STATE_MODESWITCH 	= 0x04,
    HW_SYNC_STATE_IDLE 	                     = 0x05, //the duration between AE exection stage
}HW_SYNC_STATE;

/******************************************************************************
 *
 * @enum ExecCmd
 * @brief enum for execution stage.
 * @details
 *
 ******************************************************************************/
enum ExecCmd
{
    ExecCmd_UNKNOWN = 0,
    ExecCmd_CHECKDIFF,
    ExecCmd_DOSYNC,
    ExecCmd_LEAVE
};

/******************************************************************************
 *
 * @struct ADJSENSOR_INFO
 * @brief parameter for the sensor we want to adjust.
 * @details
 *
 ******************************************************************************/
typedef struct ADJSENSOR_INFO
{
	MINT32 	pclk;					///<pixel clock of the sensor we want to adjust
	MUINT32 period[2];			    ///<period information without dummy at the current scenario{line pixel, frame length}
	MUINT32 dummyInfo[2];           ///<dummy information(dummy pixel, dummy line)
	MUINT32 dummyLNeeded;		    ///<the dummy line we need to set at the current frame
	MUINT32 dummyLFinal;		    ///<the dummy line we finally set to sensor at the current frame
	MUINT32 dummyLUB;
	MUINT32 dummyLLB;
	MUINT32 sw_curRegCntTus;        ///<current frame period between VDs of adjusted sensor
	//
	ADJSENSOR_INFO()
    : pclk(0)
    , period{0x0,0x0}
    , dummyInfo{0x0,0x0}
    , dummyLNeeded(0)
    , dummyLFinal(0)
    , dummyLUB(0)
    , dummyLLB(0)
    , sw_curRegCntTus(0)
    {}
};

/******************************************************************************
 *
 * @struct N3D_INFO
 * @brief parameter for n3d registers.
 * @details
 *
 ******************************************************************************/
struct N3D_INFO
{
    MUINT32 order;                  ///<the leading situation at the current frame(1 for vs1 leads vs2, and 0 for vs2 leads vs1)
    MUINT32 vs_regCnt[2];           ///<the time period count of sensor1 (main sensor) and sensor2 (main2 sensor)
                                    ///<vs_reg Cnt(n) actually is vs count(n-1) due to double buffer is adopted.
    MUINT32 vs_regTus[2];
    //trick point
    //the different value calculated from n3d DiffCNT_reg is the value of pre frame,
    //but the order[bit 31] we get is the situation of current frame
    //we could use dbgPort and cnt2 to estimate the different value of current frame
    MUINT32 vdDiffCnt;
    MUINT32 vdDiffTus;
    MUINT32 vs2_vs1Cnt;             ///<debug port is the count value of sensor2 sampled by sensor1 (unit:count)
    MUINT32 vs2_vs1Tus;             ///<debug port is the count value of sensor2 sampled by sensor1 (unit:us)
    //
    N3D_INFO()
    : order(0x0)
    , vdDiffCnt(0x0)
    , vdDiffTus(0x0)
    , vs_regCnt{0x0,0x0}
    , vs_regTus{0x0,0x0}
    , vs2_vs1Cnt(0x0)
    , vs2_vs1Tus(0x0)
    {}
};



/******************************************************************************
 *
 * @class HWSyncDrv Derived Class
 * @brief Driver to sync VDs.
 * @details
 *
 ******************************************************************************/
class HWSyncDrvImp : public HWSyncDrv
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    HWSyncDrvImp();
    virtual ~HWSyncDrvImp();
//
public:
    /**
     * @brief get the singleton instance
     *
     * @details
     *
     * @note
     *
     * @return
     * A singleton instance to this class.
     *
     */
    static HWSyncDrv*  	getInstance(void);
    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    virtual void    	destroyInstance(void);
    /**
     * @brief init the hwsync driver
     *
     * @details
     * @param[in] sensorIdx: sensor index (0,1,2..).
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32   	init(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx,MINT32 aeCyclePeriod = -1);
    /**
     * @brief uninit the hwsync driver
     *
     * @details
     * @param[in] sensorIdx: sensor index(0,1,2..).
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
    virtual MINT32   	uninit(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  General Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
     * @brief sendCommand to change setting
     *
     * @details
     * @param[in] cmd: command.
     * @param[in] senDev: sensor dev.
     * @param[in] senScen: sensor scenario.
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
     virtual MINT32 sendCommand(HW_SYNC_CMD_ENUM cmd,MUINT32 senDev,MUINTPTR senScen, MUINTPTR  currfps=0x0, MUINTPTR  clockstage=0x0);



};
//-----------------------------------------------------------------------------
#endif


