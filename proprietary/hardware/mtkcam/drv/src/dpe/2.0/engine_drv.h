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
#ifndef _ENGINE_DRV_H_
#define _ENGINE_DRV_H_

#include <dpecommon_v20.h>
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


/******************************************************************************
 *
 * @enum ECmd
 * @brief enum for enqueue/dequeue signal control(from RscStream.h).
 * @details
 *
 ******************************************************************************/
enum ECmd
{
    ECmd_UNKNOWN = 0,
    ECmd_ENQUE,
    ECmd_ENQUE_FEEDBACK,
    ECmd_DEQUENEXTCQ,
    ECmd_INIT,
    ECmd_UNINIT
};

/*
 * Engine Control Table
 */
typedef struct
{
    /*TODO: use SUB_ENGINE_ID in rsccmmon? */
    MUINT32 id;
    ECmd cmd;
    MUINT32 int_st;
}ENGINE_CTRL;

/*
 * Engine Error Status
 */
enum
{
    ERR_NULL_TABLE = -1,
    ERR_INVALID_ENGINE = -2
};
/**************************************************************************/
/*                   C L A S S    D E C L A R A T I O N                   */
/**************************************************************************/
template <class T>
class EngineDrv
{
    protected:
        virtual         ~EngineDrv(){}
    public:
        static EngineDrv*  createInstance();
        virtual void    destroyInstance(void) = 0;
        virtual MBOOL   init(const char* userName="") = 0;
        virtual MBOOL   uninit(const char* userName="") = 0;
        virtual MBOOL   waitFrameDone(unsigned int Status, MINT32 timeoutMs) = 0;
        /*TODO : argument template for RSCConfig?*/
        virtual MBOOL   enque(vector<T>& ConfigVec) = 0;
        virtual MBOOL   deque(vector<T>& ConfigVec) = 0;

        MINT32 queryCmd(MUINT32 engine, ECmd& cmd)
        {
            if(ctl_tbl == NULL) {
                return ERR_NULL_TABLE;
            }

            if(engine >= eENGINE_MAX) {
                cmd = ECmd_UNKNOWN;
                return ERR_INVALID_ENGINE;
            }

            cmd = ctl_tbl[engine].cmd;

            return 0;
        }

        MUINT32 queryInterrupt(MUINT32 engine)
        {

            if(ctl_tbl == NULL) {
                return ERR_NULL_TABLE;
            }

            if(engine >= eENGINE_MAX) {
                return ERR_INVALID_ENGINE;
            }
            return ctl_tbl[engine].int_st;
        }

    protected:
        const ENGINE_CTRL* ctl_tbl;
};

#endif  // _ENGINE_DRV_H_
