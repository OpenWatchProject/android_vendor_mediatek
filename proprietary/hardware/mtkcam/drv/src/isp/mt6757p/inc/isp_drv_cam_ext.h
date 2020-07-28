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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#ifndef _ISP_DRV_CAM_EXT_H_
#define _ISP_DRV_CAM_EXT_H_

#include <imem_drv.h>

/**************************************************************************/
/*    Used to two dma ring buffer control by one cq                       */
/**************************************************************************/
class ISP_DRV_CAM_EXT : public ISP_DRV_CAM
{
    /****************************************************
        Define and Structure
    ****************************************************/
    #define RingBuf_MODULE_EXT_MAX   (7+1)  // +1 for cq inner baseaddr when high-speed mode,+3 for EXT dma cq_cmd

    private:
    /****************************************************
        Used to two dma ring buffer control by one cq,
        extend structure "CQ_INFO_RTBC_ST_CAM" by add 2nd dma setting
    ****************************************************/
        typedef struct _cq_info_rtbc_st_CAM_EXT_
        {
            CQ_CMD_ST_CAM   dma;
            CQ_CMD_ST_CAM   dma_fh;
            CQ_CMD_ST_CAM   fh_spare;
            //2nd dma cmd
            CQ_CMD_ST_CAM   dma_2nd;
            CQ_CMD_ST_CAM   dma_fh_2nd;
            CQ_CMD_ST_CAM   fh_spare_2nd;
            //
            CQ_CMD_ST_CAM   next_Thread_addr;
            CQ_CMD_ST_CAM   next_Thread_addr_inner;
            CQ_CMD_ST_CAM   end;
            //
            MUINT32         dma_base_pa;
            MUINT32         dma_fh_base_pa;
            MUINT32         dma_fh_spare[CAM_IMAGE_HEADER];
            //2nd dma
            MUINT32         dma_base_pa_2nd;
            MUINT32         dma_fh_base_pa_2nd;
            MUINT32         dma_fh_spare_2nd[CAM_IMAGE_HEADER];
        }CQ_INFO_RTBC_ST_CAM_EXT;

        typedef struct _cq_ring_cmd_st_CAM_EXT_
        {
            CQ_INFO_RTBC_ST_CAM_EXT cq_rtbc;
            unsigned long next_pa;
            struct _cq_ring_cmd_st_CAM_EXT_ *pNext;
        }CQ_RING_CMD_ST_CAM_EXT;

        typedef struct _cq_rtbc_ring_st_CAM_EXT_
        {
            CQ_RING_CMD_ST_CAM_EXT rtbc_ring[MAX_RING_SIZE];   //ring depth: sw maximum: 16 [hw maximus is depended on hw bits]
            unsigned int   dma_ring_size;
        }CQ_RTBC_RING_ST_CAM_EXT;

    /******************************
        Functions and variables
    ******************************/
    public:
        ISP_DRV_CAM_EXT(ISP_HW_MODULE module,E_ISP_CAM_CQ cq,MUINT32 pageIdx):ISP_DRV_CAM(module,cq,pageIdx){};
        ~ISP_DRV_CAM_EXT(){};

        //for CQ those descriptor is static, not programable
        MUINT32     cqRingBuf(CQ_RingBuf_ST *pBuf_ctrl);

    private:
        MBOOL       updateEnqCqRingBuf(CQ_RingBuf_ST *pBuf_ctrl);
        MUINT32*    update_FH_Spare(CQ_RingBuf_ST *pBuf_ctrl);
        MBOOL       CQ_Allocate_method2(MUINT32 step);

};

#endif //_ISP_DRV_CAM_EXT_H_
