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
#include <map>
//
#include <mtkcam/drv/IHwSyncDrv.h>
#include <mtkcam/drv/IHalSensor.h>

//-----------------------------------------------------------------------------
using namespace std;
using namespace android;
using namespace NSCam;
//-----------------------------------------------------------------------------

//support sensor type: delay time must <= this define
#define MAX_SUPPORT_FRAMELENGTH_DELAY_NUM 2

/******************************************************************************
 *
 * @enum HW_SYNC_STATE_ENUM
 * @brief cmd enum for sendCommand.
 * @details
 *
 ******************************************************************************/
typedef enum{
    HW_SYNC_STATE_NONE          = 0X00,
    HW_SYNC_STATE_READY2RUN     = 0X01,
    HW_SYNC_STATE_SYNCING       = 0x02,
    HW_SYNC_STATE_READY2LEAVE   = 0x03,
    HW_SYNC_STATE_MODESWITCH    = 0x04,
    HW_SYNC_STATE_IDLE                       = 0x05, //the duration between AE exection stage
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
    ExecCmd_DOSYNC,
    ExecCmd_SET_SYNC_FRMTIME_MAIN1,
    ExecCmd_SET_SYNC_FRMTIME_MAIN2,
    ExecCmd_LEAVE
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
    //trick point
    //the different value calculated from n3d DiffCNT_reg is the value of pre frame,
    //but the order[bit 31] we get is the situation of current frame
    //we could use dbgPort and cnt2 to estimate the different value of current frame
    MUINT32 vdDiffCnt;
    MUINT32 vdDiffTus;
    MUINT32 vs_regCnt[2];           ///<the time period count of sensor1 (main sensor) and sensor2 (main2 sensor)
                                    ///<vs_reg Cnt(n) actually is vs count(n-1) due to double buffer is adopted.
    MUINT32 vs_regTus[2];
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


/**
    N3D input source enum
*/
typedef enum{
    E_TG_A  = 0,
    E_TG_B  = 1,
    E_TG_MAX= 2,
}E_N3D_SOURCE;

/**
    sensor device.
*/
typedef enum{
    SEN1    = 0,    // mapping to info of sensor which is cfged 1st
    SEN2    = 1,    //mapping to info of sensor which is cfged 2nd
    MAX_SEN = 2,
}E_SENDEV;

typedef struct _T_DATACfg
{
    MUINT32 FrameTime[MAX_SEN]; // 2nd adjust frame time , in order to recover sensor's fps
    MUINT32 expTime[MAX_SEN];   // shutter speed , input by AE at 1st adjust frame time
}T_DATACfg;


class HWSyncInit;

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
    static HWSyncDrv*   getInstance(void);
    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    virtual void        destroyInstance(void);
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
    virtual MINT32      init(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx, MINT32 aeCyclePeriod = -1);
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
    virtual MINT32      uninit(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx);
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
    virtual MINT32      sendCommand(HW_SYNC_CMD_ENUM cmd,MUINT32 senDev,MUINTPTR senScen, MUINTPTR currfps, MUINTPTR arg4);

};


//-----------------------------------------------------------------------------
#endif


