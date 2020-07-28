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

#include "pd_buf_common.h"
#include "kd_imgsensor.h"
#include <aaa_log.h>

#define LOG_TAG "pd_mgr_list"

typedef struct
{
    UINT32 SensorId;
    EPDBuf_Type_t  type;

} PDBuf_List_t;


PDBuf_List_t PDList_main[MAX_NUM_OF_SUPPORT_SENSOR] =
{

#if defined(S5K3P3SX_MIPI_RAW)
	{S5K3P3SX_SENSOR_ID, EPDBuf_Raw},
#endif
#if defined(OV23850_MIPI_RAW)
	{OV23850_SENSOR_ID, EPDBuf_VC},
#endif
#if defined(IMX230_MIPI_RAW)
	{IMX230_SENSOR_ID,  EPDBuf_VC_Open},
#endif
#if defined(S5K2P8_MIPI_RAW)
	{S5K2P8_SENSOR_ID,	EPDBuf_Raw},
#endif
#if defined(IMX258_MIPI_RAW)
	{IMX258_SENSOR_ID,	EPDBuf_VC},
#endif
#if defined(S5K3M2_MIPI_RAW)
	{S5K3M2_SENSOR_ID,	EPDBuf_Raw},
#endif

};

PDBuf_List_t PDList_sub[MAX_NUM_OF_SUPPORT_SENSOR] =
{
#if defined(S5K3P3SX_MIPI_RAW)
	{S5K3P3SX_SENSOR_ID, EPDBuf_Raw},
#endif
#if defined(OV23850_MIPI_RAW)
		{OV23850_SENSOR_ID, EPDBuf_VC},
#endif
	
#if defined(IMX230_MIPI_RAW)
		{IMX230_SENSOR_ID,	EPDBuf_VC_Open},
#endif
#if defined(S5K2P8_MIPI_RAW)
		{S5K2P8_SENSOR_ID,	EPDBuf_Raw},
#endif
#if defined(S5K3M2_MIPI_RAW)
	{S5K3M2_SENSOR_ID,	EPDBuf_Raw},
#endif

};

PDBuf_List_t PDList_main2[MAX_NUM_OF_SUPPORT_SENSOR] =
{
#if defined(S5K3P3SX_MIPI_RAW)
	{S5K3P3SX_SENSOR_ID, EPDBuf_Raw},
#endif
#if defined(OV23850_MIPI_RAW)
		{OV23850_SENSOR_ID, EPDBuf_VC},
#endif
	
#if defined(IMX230_MIPI_RAW)
		{IMX230_SENSOR_ID,	EPDBuf_VC_Open},
#endif
#if defined(S5K2P8_MIPI_RAW)
		{S5K2P8_SENSOR_ID,	EPDBuf_Raw},
#endif
#if defined(S5K3M2_MIPI_RAW)
	{S5K3M2_SENSOR_ID,	EPDBuf_Raw},
#endif

};


EPDBuf_Type_t GetPDBuf_Type( unsigned int a_u4CurrSensorDev, unsigned int a_u4CurrSensorId)
{
	PDBuf_List_t *ptrlist=NULL;
	EPDBuf_Type_t retval = EPDBuf_NotDef;
	
	if( a_u4CurrSensorDev==2) //sub
		ptrlist = PDList_sub;
	else if(a_u4CurrSensorDev==4) //main 2
		ptrlist = PDList_main2;
	else  // main or others
		ptrlist = PDList_main;

	for( int i=0; i<MAX_NUM_OF_SUPPORT_SENSOR; i++)
	{
		MY_LOG("ID 0x%x, Type %d", ptrlist[i].SensorId, ptrlist[i].type);
		if( ptrlist[i].SensorId == a_u4CurrSensorId)
		{
			retval = ptrlist[i].type;
			break;
		}
	}


	return retval;
}


