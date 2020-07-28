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
#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1


#include "pd_buf_common.h"
#include "kd_imgsensor.h"
#include <aaa_log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_mgr_list"

typedef struct
{
    MUINT32 SensorId;
    MUINT32  type;

} PDBuf_List_t;


PDBuf_List_t PDList_main[MAX_SIZE_OF_PD_SENSOR_LIST] =
{
#if defined(OV13855_MIPI_RAW)
    {OV13855_SENSOR_ID, (EPDBUF_PDO)},
#endif
#if defined(S5K3L8_MIPI_RAW)
    {S5K3L8_SENSOR_ID, (EPDBUF_PDO)},
#endif
#if defined(OV12A10_MIPI_RAW)
   {OV12A10_SENSOR_ID, (EPDBUF_PDO)},
#endif
#if defined(IMX486_SENSOR_ID)
   {IMX486_SENSOR_ID, (EPDBUF_PDO)},
#endif
#if defined(S5K3P8SX_MIPI_RAW)
    {S5K3P8SX_SENSOR_ID, EPDBUF_VC},
#endif
#if defined(IMX362_MIPI_RAW)
    {IMX362_SENSOR_ID, EPDBUF_DUALPD_RAW},
#endif
#if defined(OV23850_MIPI_RAW)
    {OV23850_SENSOR_ID, EPDBUF_VC},
#endif
#if defined(OV13870_MIPI_RAW)
    {OV13870_SENSOR_ID, EPDBUF_VC | EPDBUF_RAW_LEGACY},
#endif
#if defined(OV16880_MIPI_RAW)
    {OV16880_SENSOR_ID, (EPDBUF_VC | EPDBUF_PDO)},
#endif
#if defined(IMX230_MIPI_RAW)
    {IMX230_SENSOR_ID,  EPDBUF_VC_OPEN},
#endif
#if defined(S5K2P7_MIPI_RAW)
    {S5K2P7_SENSOR_ID,    EPDBUF_VC},
#endif
#if defined(S5K2P8_MIPI_RAW)
    {S5K2P8_SENSOR_ID,    EPDBUF_PDO},
#endif
#if defined(S5K3M3_MIPI_RAW)
    {S5K3M3_SENSOR_ID, EPDBUF_VC},
#endif
#if defined(IMX258_MIPI_RAW)
    {IMX258_SENSOR_ID,    (EPDBUF_VC | EPDBUF_PDO | EPDBUF_RAW_LEGACY)},
#endif
#if defined(IMX499_MIPI_RAW)
    {IMX499_SENSOR_ID,    (EPDBUF_VC | EPDBUF_RAW_LEGACY)},
#endif
#if defined(S5K3M2_MIPI_RAW)
    {S5K3M2_SENSOR_ID,    EPDBUF_RAW_LEGACY},
#endif
#if defined(S5K2X8_MIPI_RAW)
    {S5K2X8_SENSOR_ID,    EPDBUF_PDO},
#endif
#if defined(S5K2L7_MIPI_RAW)
    {S5K2L7_SENSOR_ID,    EPDBUF_DUALPD_RAW | EPDBUF_DUALPD_VC},
#endif
#if defined(IMX338_MIPI_RAW)
    {IMX338_SENSOR_ID,    EPDBUF_VC_OPEN},
#endif
#if defined(IMX386_MIPI_RAW)
    {IMX386_SENSOR_ID,    EPDBUF_VC_OPEN},
#endif
#if defined(IMX519_MIPI_RAW)
    {IMX519_SENSOR_ID,    EPDBUF_VC_OPEN},
#endif
#if defined(IMX398_MIPI_RAW)
    {IMX398_SENSOR_ID,    EPDBUF_PDO},
#endif
#if defined(IMX586_MIPI_RAW)
    {IMX586_SENSOR_ID,    EPDBUF_VC},
#endif

    /* Please add sensor ID and PD type before this line.*/
    {0x0000, 0x0000},
};

PDBuf_List_t PDList_sub[MAX_SIZE_OF_PD_SENSOR_LIST] =
{
#if defined(IMX362_MIPI_RAW)
    {IMX362_SENSOR_ID, EPDBUF_DUALPD_RAW},
#endif
#if defined(OV23850_MIPI_RAW)
    {OV23850_SENSOR_ID, EPDBUF_VC},
#endif
#if defined(OV16880_MIPI_RAW)
    {OV16880_SENSOR_ID, EPDBUF_RAW_LEGACY},
#endif
#if defined(IMX230_MIPI_RAW)
    {IMX230_SENSOR_ID,    EPDBUF_VC_OPEN},
#endif
#if defined(S5K2P8_MIPI_RAW)
    {S5K2P8_SENSOR_ID,    EPDBUF_PDO},
#endif
#if defined(S5K3M2_MIPI_RAW)
    {S5K3M2_SENSOR_ID,    EPDBUF_RAW_LEGACY},
#endif
#if defined(S5K2L7_MIPI_RAW)
    {S5K2L7_SENSOR_ID,    EPDBUF_DUALPD_RAW},
#endif

    /* Please add sensor ID and PD type before this line.*/
    {0x0000, 0x0000},
};

PDBuf_List_t PDList_main2[MAX_SIZE_OF_PD_SENSOR_LIST] =
{
#if defined(IMX362_MIPI_RAW)
    {IMX362_SENSOR_ID, EPDBUF_DUALPD_RAW},
#endif
#if defined(OV23850_MIPI_RAW)
    {OV23850_SENSOR_ID, EPDBUF_VC},
#endif
#if defined(OV16880_MIPI_RAW)
    {OV16880_SENSOR_ID, EPDBUF_RAW_LEGACY},
#endif
#if defined(IMX230_MIPI_RAW)
    {IMX230_SENSOR_ID,    EPDBUF_VC_OPEN},
#endif
#if defined(S5K2P8_MIPI_RAW)
    {S5K2P8_SENSOR_ID,    EPDBUF_PDO},
#endif
#if defined(S5K3M2_MIPI_RAW)
    {S5K3M2_SENSOR_ID,    EPDBUF_RAW_LEGACY},
#endif
#if defined(S5K2L7_MIPI_RAW)
    {S5K2L7_SENSOR_ID,    EPDBUF_DUALPD_RAW},
#endif
#if defined(S5K3M3_MIPI_RAW)
    {S5K3M3_SENSOR_ID, EPDBUF_VC},
#endif

    /* Please add sensor ID and PD type before this line.*/
    {0x0000, 0x0000},
};

PDBuf_List_t PDList_sub2[MAX_SIZE_OF_PD_SENSOR_LIST] =
{
    /* Please add sensor ID and PD type before this line.*/
    {0x0000, 0x0000},
};

PDBuf_List_t PDList_main3[MAX_SIZE_OF_PD_SENSOR_LIST] =
{
    /* Please add sensor ID and PD type before this line.*/
    {0x0000, 0x0000},
};

MUINT32 GetPDBuf_Type( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId)
{
    MUINT32  retval  = EPDBUF_NOTDEF;

    PDBuf_List_t *ptrlist = NULL;
    if( a_u4CurrSensorDev==0x2)
    {
        /* sub */
        ptrlist = PDList_sub;
    }
    else if( a_u4CurrSensorDev==0x4)
    {
        /* main 2 */
        ptrlist = PDList_main2;
    }
    else if( a_u4CurrSensorDev==0x8)
    {
        /* sub 2 */
        ptrlist = PDList_sub2;
    }
    else if( a_u4CurrSensorDev==0x10)
    {
        /* main 3 */
        ptrlist = PDList_main3;
    }
    else
    {
        /* main or others */
        ptrlist = PDList_main;
    }

    AAA_LOGD("dev %d", a_u4CurrSensorDev);
    for( int i=0; i<MAX_SIZE_OF_PD_SENSOR_LIST; i++)
    {
        AAA_LOGD("ID 0x%04x, Type 0x%x", ptrlist[i].SensorId, ptrlist[i].type);
        if( ptrlist[i].SensorId == a_u4CurrSensorId)
        {
            /* searching done */
            retval = ptrlist[i].type;
            break;
        }

        if( ptrlist[i].SensorId == 0x0000)
        {
            /* end of list */
            break;
        }
    }

    return retval;
}


