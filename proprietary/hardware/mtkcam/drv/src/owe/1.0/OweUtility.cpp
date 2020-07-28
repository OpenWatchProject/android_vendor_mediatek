/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <mtkcam/drv/iopipe/PostProc/OweUtility.h>
#include "OweStream.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {

/******************************************************************************
*
******************************************************************************/
MUINT32 OWEQueryInDMAStride(
    OWEDMAPort dmaport, MUINT32 fmt, MUINT32 subSamWidth)
{
    MUINT32 DMAStride = 0;
    switch (dmaport)
    {
        case DMA_OWMFE_IMGI:
            {
                if ((OWEIMGIFORMAT)fmt == OWE_IMGI_Y_FMT)
                {
                    DMAStride = subSamWidth;
                }
                else
                {
                    DMAStride = 2*subSamWidth;
                }
            }
            break;
        case DMA_OWMFE_DPI:  //Depth map, one pixel is one byte,
             {
                 if ((OWMFEDPIFORMAT)fmt == OWMFE_DPI_D_FMT)
                 {
                     DMAStride = subSamWidth;
                 }
                 else
                 {
                     DMAStride = 2*subSamWidth;
                 }
             }
             break;
        case DMA_OWMFE_TBLI:
             {
                DMAStride = WMFE_TBLI_SIZE;
             }
             break;
        default:
            {
                //LOG_ERR("dmaport(%d),WMFEDPIFORMAT(%d),subSamWidth(%d)",dmaport, fmt, subSamWidth);
            }
            break;
    }


    return DMAStride;
}


/******************************************************************************
*
******************************************************************************/
#if 0
MUINT32 OWEQueryOccOutDMAStride(
    OWEDMAPort dmaport, OCC_HORZ_DSMODE mode, MUINT32 OCC_ORG_WIDTH)
{
    MUINT32 DMAStride = 0;
    switch (dmaport)
    {
        default:
            {
                //LOG_ERR("dmaport(%d),OCC_HORZ_DSMODE(%d),OCC_ORG_WIDTH(%d)",dmaport, mode, OCC_ORG_WIDTH);
            }
            break;
    }

    return DMAStride;
}
#endif
/******************************************************************************
*
******************************************************************************/
MUINT32 OWEQueryWmfeOutDMAStride(
    OWEDMAPort dmaport, MUINT32 Wmfe_Width)
{
    MUINT32 DMAStride = 0;
    switch (dmaport)
    {
        case DMA_OWMFE_DPO:
             {
                DMAStride = ((Wmfe_Width+15)>>4)<<4;
             }
        default:
            {
                //LOG_ERR("dmaport(%d),Wmfe_Width(%d)",dmaport, Wmfe_Width);
            }
            break;
    }

    return DMAStride;
}


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
