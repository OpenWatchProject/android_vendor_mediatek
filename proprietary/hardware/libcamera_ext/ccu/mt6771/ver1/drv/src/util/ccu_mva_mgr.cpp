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
#define LOG_TAG "ccumva"

#include "ccu_mva_mgr.h"

/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {
namespace NSCcuDrv   {

MBOOL CcuMvaMgrIon::mmapMva_ion(int buf_share_fd, ion_user_handle_t *p_ion_handle,unsigned int *mva )
{
    struct ion_sys_data sys_data;
    struct ion_mm_data  mm_data;
    //ion_user_handle_t   ion_handle;
    int err;

    LOG_DBG("+:%s\n",__FUNCTION__);

    //a. get handle from IonBufFd and increase handle ref count
    if(ion_import(gIonDevFD, buf_share_fd, p_ion_handle))
    {
        LOG_ERR("ion_import fail, ion_handle(0x%x)", *p_ion_handle);
        return false;
    }
    LOG_VRB("ion_import: share_fd %d, ion_handle %d", buf_share_fd, *p_ion_handle);
    //b. config buffer
    mm_data.mm_cmd = ION_MM_CONFIG_BUFFER;
    mm_data.config_buffer_param.handle      = *p_ion_handle;
    //mm_data.config_buffer_param.eModuleID   = M4U_PORT_CAM_CCUG ;
    mm_data.config_buffer_param.eModuleID   = 0;
    mm_data.config_buffer_param.security    = 0;
    mm_data.config_buffer_param.coherent    = 0;
    err = ion_custom_ioctl(gIonDevFD, ION_CMD_MULTIMEDIA, &mm_data);
    if(err == (-ION_ERROR_CONFIG_LOCKED))
    {
        LOG_WRN("ion_custom_ioctl Double config after map phy address");
    }
    else if(err != 0)
    {
        LOG_WRN("ion_custom_ioctl ION_CMD_MULTIMEDIA Config Buffer failed!");
    }
    //c. map physical address
    sys_data.sys_cmd = ION_SYS_GET_PHYS;
    sys_data.get_phys_param.handle = *p_ion_handle;
    if(ion_custom_ioctl(gIonDevFD, ION_CMD_SYSTEM, &sys_data))
    {
        LOG_ERR("ion_custom_ioctl get_phys_param failed!");
        return false;
    }
	//
	*mva = (unsigned int)sys_data.get_phys_param.phy_addr;

    //req_buf->plane[plane_idx].ion_handle = ion_handle;
    //req_buf->plane[plane_idx].mva        = sys_data.get_phys_param.phy_addr;
    //LOG_VRB("get_phys: buffer[%d], plane[%d], mva 0x%x", buf_idx, plane_idx, buf_n->planes[plane_idx].ptr);

    LOG_DBG("\n");
    LOG_DBG("-:%s\n",__FUNCTION__);

	return true;
}

MBOOL CcuMvaMgrIon::munmapMva_ion(ion_user_handle_t ion_handle )
{
       LOG_DBG("+:%s\n",__FUNCTION__);

        // decrease handle ref count
        if(ion_free(gIonDevFD, ion_handle))
        {
            LOG_ERR("ion_free fail");
            return false;
        }
        LOG_VRB("ion_free: ion_handle %d", ion_handle);

        LOG_DBG("-:%s\n",__FUNCTION__);
        return true;
}

};  //namespace NSCcuDrv
};  //namespace NSCcuIf

