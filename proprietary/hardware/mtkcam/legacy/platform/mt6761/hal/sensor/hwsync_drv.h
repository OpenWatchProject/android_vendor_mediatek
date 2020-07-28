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
#ifndef _HWSYNC_DRV_LEGACY_H_
#define _HWSYNC_DRV_LEGACY_H_
#include <mtkcam/drv/IHwSyncDrv.h>
#if 0
/******************************************************************************
 *
 * @enum HW_SYNC_CMD_ENUM
 * @brief cmd enum for sendCommand.
 * @details 
 *
 ******************************************************************************/
typedef enum{
	HW_SYNC_CMD_SET_MODE 		= 0x01,         //sensor mode(by resolution and fps)
	HW_SYNC_CMD_DISABLE_SYNC   = 0x02,         //stop hwsync syncing precedure (before 3a set exposeure)
	HW_SYNC_CMD_ENABLE_SYNC     = 0x03,         //start hwsync syncing precedure (after 3a set exposeure)
	HW_SYNC_CMD_UNDATE_CLOCK     = 0x04,         //update clock base (control by stereo adapter)
}HW_SYNC_CMD_ENUM;

/******************************************************************************
 *
 * @enum HW_SYNC_USER_ENUM
 * @brief the user enum for using hwsync driver.
 * @details 
 *
 ******************************************************************************/
typedef enum{
        HW_SYNC_USER_HALSENSOR                  = 0x00,
        HW_SYNC_USER_AE 		                  = 0x01,
        HW_SYNC_USER_STEREO_ADAPTER 		       = 0x02,
}HW_SYNC_USER_ENUM;

/******************************************************************************
 *
 * @enum HW_SYNC_CLOCK_STAGE
 * @brief the clock stage user could adjust
 * @details 
 *
 ******************************************************************************/
typedef enum{
        HW_SYNC_CLOCK_STAGE_0                  = 0x00,
        HW_SYNC_CLOCK_STAGE_1 		   = 0x01,
}HW_SYNC_CLOCK_STAGE_ENUM;


/******************************************************************************
 *
 * @class HWSyncDrv BaseClass
 * @brief Driver to sync VDs.
 * @details
 *
 ******************************************************************************/
class HWSyncDrv
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    HWSyncDrv(){};
    virtual ~HWSyncDrv() {};
//
public:
    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static HWSyncDrv*  	createInstance();
    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    virtual void    	destroyInstance(void) = 0;
    virtual MBOOL   	init(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx) = 0;
    virtual MBOOL   	uninit(HW_SYNC_USER_ENUM user, MUINT32 sensorIdx) = 0;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  General Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++    
	virtual MINT32 		sendCommand(HW_SYNC_CMD_ENUM cmd,MUINT32 senDev,MUINT32 senScen, MUINT32 currfps, HW_SYNC_CLOCK_STAGE_ENUM clockstage=HW_SYNC_CLOCK_STAGE_0)=0;
};
#endif
//----------------------------------------------------------------------------
#endif  // _HWSYNC_DRV_H_


