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
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>

using namespace std;

namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_X_PIX;
    typedef struct{
        MUINT32          portId;
        EImageFormat     format;
        MUINT32          width;
        E_CamPixelMode   pixelMode;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MUINT32              QueryOutput;
}sCAM_QUERY_X_PIX;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_CROP_X_PIX;
    typedef struct{
        MUINT32          portId;
        EImageFormat     format;
        MUINT32          width;
        E_CamPixelMode   pixelMode;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MUINT32              QueryOutput;
}sCAM_QUERY_CROP_X_PIX;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_X_BYTE;

    typedef struct{
        MUINT32          portId;
        EImageFormat     format;
        MUINT32          width;
        E_CamPixelMode   pixelMode;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MUINT32              QueryOutput;
}sCAM_QUERY_X_BYTE;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_CROP_X_BYTE;

    typedef struct{
        MUINT32          portId;
        EImageFormat     format;
        MUINT32          width;
        E_CamPixelMode   pixelMode;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MUINT32              QueryOutput;
}sCAM_QUERY_CROP_X_BYTE;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_CROP_START_X;

    typedef struct{
        MUINT32          portId;
        EImageFormat     format;
        MUINT32          width;
        E_CamPixelMode   pixelMode;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MUINT32              QueryOutput;
}sCAM_QUERY_CROP_START_X;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_STRIDE_PIX;

    typedef struct{
        MUINT32          portId;
        EImageFormat     format;
        MUINT32          width;
        E_CamPixelMode   pixelMode;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MUINT32              QueryOutput;
}sCAM_QUERY_STRIDE_PIX;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_STRIDE_BYTE;

    typedef struct{
        MUINT32          portId;
        EImageFormat     format;
        MUINT32          width;
        E_CamPixelMode   pixelMode;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MUINT32              QueryOutput;
}sCAM_QUERY_STRIDE_BYTE;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_MAX_SEN_NUM;

    MUINT32              QueryOutput;
}sCAM_QUERY_MAX_SEN_NUM;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_PIPELINE_BITDEPTH;

    MUINT32              QueryOutput;
}sCAM_QUERY_PIPELINE_BITDEPTH;


typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_BURST_NUM;

    MUINT32              QueryOutput;
}sCAM_QUERY_BURST_NUM;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_SUPPORT_PATTERN;

    MUINT32              QueryOutput;
}sCAM_QUERY_SUPPORT_PATTERN;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_QUERY_FMT;
    typedef struct{
        MUINT32          portId;
        MBOOL            SecOn;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    vector<EImageFormat> QueryOutput;
}sCAM_QUERY_QUERY_FMT;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_BS_RATIO;
    typedef struct{
        MUINT32              portId;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MUINT32              QueryOutput;
}sCAM_QUERY_BS_RATIO;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_D_Twin;

    MBOOL                QueryOutput;
}sCAM_QUERY_D_Twin;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_D_BayerEnc;

    MUINT32              QueryOutput;
}sCAM_QUERY_D_BAYERENC;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_NEED_CFG_SENSOR_BY_MW;

    MUINT32              QueryOutput;
}sCAM_QUERY_NEED_CONFIG_SENSOR_BY_MW;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_IQ_LEVEL;
    typedef struct{
        vector<QueryInData_t>          vInData;
        vector<vector<QueryOutData_t>> vOutData;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MBOOL                QueryOutput;
}sCAM_QUERY_IQ_LEVEL;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_ISP_RES;
    typedef struct {
        MUINT32      sensorIdx;
        MUINT32      scenarioId;
        MUINT32      rrz_out_w;
        E_CamPattern pattern;
        MBOOL        bin_off;
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MBOOL                QueryOutput;
}sCAM_QUERY_ISP_RES;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_HW_RES_MGR;

    vector<SEN_INFO>    QueryInput;
    vector<PIPE_SEL>    QueryOutput;
}sCAM_QUERY_HW_RES_MGR;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_PDO_AVAILABLE;

    MBOOL               QueryOutput;
}sCAM_QUERY_PDO_AVAILABLE;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_DYNAMIC_PAK;

    MBOOL              QueryOutput;
}sCAM_QUERY_DYNAMIC_PAK;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_MAX_PREVIEW_SIZE;

    MUINT32              QueryOutput;
}sCAM_QUERY_MAX_PREVIEW_SIZE;


typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_FUNC;
    typedef struct{
        vector<QueryInData_t>          vInData;
        vector<QueryOutData_t> vOutData;         //query output resoult
    }QUERY_INPUT;

    QUERY_INPUT          QueryInput;
    MBOOL                QueryOutput;                   //this query-cmd is supported or not. MTURE : supported.
}sCAM_QUERY_FUNC;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_HW_RES_ALLOC;
    typedef struct{
        vector<QueryInResAlloc_t> vInData;
        vector<QueryOutResAlloc_t> vOutData;
    }QUERY_IO_PARAM;

    QUERY_IO_PARAM       QueryParam;
    MBOOL                result;
}sCAM_QUERY_HW_RES_ALLOC;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_TUNING_FREQ;
    QUERY_TUNING_FREQ   tuningFreq;
}sCAM_QUERY_TUNING_FREQ;

typedef struct
{
    ENPipeQueryCmd const Cmd = ENPipeQueryCmd_SEC_CAP;

    MBOOL                QueryOutput;
}sCAM_QUERY_SEC_CAP;

}
}
}

