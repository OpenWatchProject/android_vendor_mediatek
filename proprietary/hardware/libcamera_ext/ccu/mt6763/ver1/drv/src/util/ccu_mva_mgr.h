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
#ifndef _CCU_MVA_H_
#define _CCU_MVA_H_

#include <map>

#include <sys/mman.h>
#include <ion/ion.h>                    // Android standard ION api
#include <linux/ion_drv.h>              // define for ion_mm_data_t
#include <libion_mtk/include/ion.h>     // interface for mtk ion
#include <m4u_lib.h>

using namespace std;

/*******************************************************************************
*
********************************************************************************/
namespace NSCcuIf {
namespace NSCcuDrv   {

class ICcuMvaMgr
{
public:
    virtual ICcuMvaMgr *createInstance() = 0;
    virtual MBOOL mmapMva(void *vaIn, unsigned int *mvaOut, size_t size) = 0;
    virtual MBOOL munmapMva(unsigned int mva) = 0;
    virtual MBOOL munmapAllMva() = 0;
};

class CcuMvaMgrIon : public ICcuMvaMgr
{
private:
    map<mva, ion_user_handle_t> mapMvaRecord;

public:
    virtual ICcuMvaMgr *createInstance();
    virtual MBOOL mmapMva(void *vaIn, unsigned int *mvaOut, size_t size);
    virtual MBOOL munmapMva(unsigned int mva);
    virtual MBOOL munmapAllMva();

private:
    virtual MBOOL mmapMva_ion(int buf_share_fd, ion_user_handle_t *p_ion_handle,unsigned int *mva );
    virtual MBOOL munmapMva_ion(ion_user_handle_t ion_handle);

};


};  //namespace NSCcuDrv
};  //namespace NSCcuIf
#endif  //  _CCUIF_H_

