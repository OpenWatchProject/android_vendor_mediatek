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
#ifndef _CCUIF_H_
#define _CCUIF_H_
//
#include <vector>
#include <map>
#include <list>

//
using namespace std;
//
#include "ccu_udrv.h"
#include "iccu.h"
#include "ccuif_compat.h"


//
/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {
namespace NSCcuDrv   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/

#define SIZE_CCU_INOUT_DATA_BUFFER (1024)
#define SIZE_CCU_AE_CUST_DATA_BUFFER (1024 * 50)
#define SIZE_CCU_AE_ONCHANGE_DATA_BUFFER (1024 * 50)

/*******************************************************************************
*
********************************************************************************/




class CcuIF : public ICcu
{
//
public:
    static CcuIF*   Create(MINT8 const szUsrName[32], E_CCUIF_INPUT InPut);
            void    Destroy(void);

                    CcuIF();
                    virtual ~CcuIF();
private:
    //
    class ResMgr
    {
        public:
        MBOOL   occupied;
        char    m_User[32];
        MUINT32 source;
        ResMgr(){
            occupied = MFALSE;
            m_User[0] = '\0';
            source = CCU_MAX;
        }
    };

    typedef enum _E_FSM{
        op_unknown  = 0,
        op_init     = 1,
        op_cfg      = 2,
        op_start    = 3,
        op_stop     = 4,
        op_uninit   = 5,
        op_cmd      = 6,
        op_endeq    = 7
    }E_FSM;

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

    virtual MBOOL   start();
    virtual MBOOL   stop(MBOOL bForce = MFALSE, MBOOL detachUni = MFALSE);

    virtual MBOOL   suspend();
    virtual MBOOL   resume();

    //initialization
    virtual MBOOL ccu_check_sensor_support(WHICH_SENSOR_T sensorDev);
    virtual int ccu_boot(WHICH_SENSOR_T sensorDev);
    virtual int ccu_sensor_initialize(SENSOR_INFO_IN_T *infoIn, SENSOR_INFO_OUT_T *infoOut);
    virtual int ccu_ae_initialize(CCU_AE_INITI_PARAM_T *aeInitParam);
    virtual int ccu_ae_start();

    //per-frame
    virtual int ccu_ae_set_ap_ready();
    virtual int ccu_ae_set_frame_sync_data(ccu_ae_frame_sync_data *frameSyncdata);
    virtual int ccu_ae_get_output(AE_CORE_MAIN_OUT *output);
    virtual int ccu_ae_set_ap_output(AE_CORE_CTRL_CPU_EXP_INFO *apAeOuput);
    virtual int ccu_ae_set_max_framerate( MUINT16 framerate, kal_bool min_framelength_en);
    virtual int ccu_ae_set_onchange_data(ccu_ae_onchange_data *onchangeData);

    //on-change
    virtual int ccu_ae_set_ev_comp(LIB3A_AE_EVCOMP_T *apAeEv);
    virtual int ccu_ae_set_skip_algo(MBOOL doSkip);
    virtual int ccu_ae_set_roi(ccu_ae_roi *apAeRoi);

    //un-initialization
    virtual int ccu_shutdown();

    //memory/buffer
    MBOOL ccu_get_buffer(int drv_h, int len, int *buf_share_fd, char **buf_va);
    MBOOL ccu_free_buffer(int drv_h, int len,int buf_share_fd, char *buf_va);
    MBOOL mmapMVA( int buf_share_fd, ion_user_handle_t *p_ion_handle,unsigned int *mva );
    MBOOL munmapMVA( ion_user_handle_t ion_handle );

    //debug
    MUINT32 readInfoReg(MUINT32 regNo);
//
private:
    virtual int copy_ae_init_param_compat(CCU_AE_INITI_PARAM_T *aeInitParam, COMPAT_CCU_AE_INITI_PARAM_T *compatAeInitParam);
    virtual int copy_onchange_data_compat(ccu_ae_onchange_data *onchangeData, COMPAT_ccu_ae_onchange_data *compatOnchangeData);
    virtual int send_ccu_command(ccu_msg_id msgId);

    MBOOL                   occupied;
    MINT8                   m_szUsrName[32];
    CCU_HW_MODULE           m_hwModule;

    int                     gIonDevFD;
    int                     buf_share_fd_in_data;
    char                   *buf_va_in_data;
    unsigned int            buf_mva_in_data;
    ion_user_handle_t       ion_handle_in_data;
    unsigned int            sz_in_data;

    int                     buf_share_fd_out_data;
    char                   *buf_va_out_data;
    unsigned int            buf_mva_out_data;
    ion_user_handle_t       ion_handle_out_data;
    unsigned int            sz_out_data;

    int                     buf_share_fd_ae_cust_data;
    char                   *buf_va_ae_cust_data;
    unsigned int            buf_mva_ae_cust_data;
    ion_user_handle_t       ion_handle_ae_cust_data;
    unsigned int            sz_ae_cust_data;

    int                     buf_share_fd_ae_onchange_data;
    char                   *buf_va_ae_onchange_data;
    unsigned int            buf_mva_ae_onchange_data;
    ion_user_handle_t       ion_handle_ae_onchange_data;
    unsigned int            sz_ae_onchange_data;

    CcuDrvImp              *pDrvCcu;

    //MTKM4UDrv m4u_client;
};

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCcuDrv
};  //namespace NSCcuIf
#endif  //  _CCUIF_H_

