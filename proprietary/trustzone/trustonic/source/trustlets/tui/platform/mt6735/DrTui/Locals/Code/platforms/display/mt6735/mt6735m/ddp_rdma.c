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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#define LOG_TAG "RDMA"
#include "ddp_log.h"

#include "ddp_reg.h"
#include "ddp_rdma.h"
#include "display_tui.h"
//#include "ddp_dump.h"
//#include "lcm_drv.h"

enum RDMA_INPUT_FORMAT {
    RDMA_INPUT_FORMAT_BGR565    = 0,
    RDMA_INPUT_FORMAT_RGB888    = 1,
    RDMA_INPUT_FORMAT_RGBA8888  = 2,
    RDMA_INPUT_FORMAT_ARGB8888  = 3,
    RDMA_INPUT_FORMAT_VYUY      = 4,
    RDMA_INPUT_FORMAT_YVYU      = 5,

    RDMA_INPUT_FORMAT_RGB565    = 6,
    RDMA_INPUT_FORMAT_BGR888    = 7,
    RDMA_INPUT_FORMAT_BGRA8888  = 8,
    RDMA_INPUT_FORMAT_ABGR8888  = 9,
    RDMA_INPUT_FORMAT_UYVY      = 10,
    RDMA_INPUT_FORMAT_YUYV      = 11,

    RDMA_INPUT_FORMAT_UNKNOW    = 32,
};

static unsigned int rdma_fps[RDMA_INSTANCES]={60,60};

static enum RDMA_INPUT_FORMAT rdma_input_format_convert(DpColorFormat fmt)
{
   enum RDMA_INPUT_FORMAT rdma_fmt = RDMA_INPUT_FORMAT_RGB565;
   switch(fmt)
   {
       case eBGR565           :
           rdma_fmt = RDMA_INPUT_FORMAT_BGR565    ;  break;
       case eRGB888           :
           rdma_fmt = RDMA_INPUT_FORMAT_RGB888    ;  break;
       case eRGBA8888         :
           rdma_fmt = RDMA_INPUT_FORMAT_RGBA8888  ;  break;
       case eARGB8888         :
           rdma_fmt = RDMA_INPUT_FORMAT_ARGB8888  ;  break;
       case eVYUY             :
           rdma_fmt = RDMA_INPUT_FORMAT_VYUY      ;  break;
       case eYVYU             :
           rdma_fmt = RDMA_INPUT_FORMAT_YVYU      ;  break;
       case eRGB565           :
           rdma_fmt = RDMA_INPUT_FORMAT_RGB565    ;  break;
       case eBGR888           :
           rdma_fmt = RDMA_INPUT_FORMAT_BGR888    ;  break;
       case eBGRA8888         :
           rdma_fmt = RDMA_INPUT_FORMAT_BGRA8888  ;  break;
       case eABGR8888         :
           rdma_fmt = RDMA_INPUT_FORMAT_ABGR8888  ;  break;
       case eUYVY             :
           rdma_fmt = RDMA_INPUT_FORMAT_UYVY      ;  break;
       case eYUY2             :
           rdma_fmt = RDMA_INPUT_FORMAT_YUYV      ;  break;
       default:
           DDPERR("rdma_fmt_convert fmt=%d, rdma_fmt=%d \n", fmt, rdma_fmt);
   }
   return rdma_fmt;
}

static unsigned int rdma_input_format_byte_swap(enum RDMA_INPUT_FORMAT inputFormat)
{
    int input_swap = 0;
    switch(inputFormat)
    {
        case RDMA_INPUT_FORMAT_BGR565:
        case RDMA_INPUT_FORMAT_RGB888:
        case RDMA_INPUT_FORMAT_RGBA8888:
        case RDMA_INPUT_FORMAT_ARGB8888:
        case RDMA_INPUT_FORMAT_VYUY:
        case RDMA_INPUT_FORMAT_YVYU:
        	input_swap = 1;
        	break;
        case RDMA_INPUT_FORMAT_RGB565:
        case RDMA_INPUT_FORMAT_BGR888:
        case RDMA_INPUT_FORMAT_BGRA8888:
        case RDMA_INPUT_FORMAT_ABGR8888:
        case RDMA_INPUT_FORMAT_UYVY:
        case RDMA_INPUT_FORMAT_YUYV:
        	input_swap = 0;
        	break;
        default:
            DDPERR("unknow RDMA input format is %d\n", inputFormat);
            ASSERT(0);
    }
    return input_swap;
}

static unsigned int rdma_input_format_bpp(enum RDMA_INPUT_FORMAT inputFormat)
{
    int bpp = 0;
    switch(inputFormat)
    {
        case RDMA_INPUT_FORMAT_BGR565:
        case RDMA_INPUT_FORMAT_RGB565:
        case RDMA_INPUT_FORMAT_VYUY:
        case RDMA_INPUT_FORMAT_UYVY:
        case RDMA_INPUT_FORMAT_YVYU:
        case RDMA_INPUT_FORMAT_YUYV:
            bpp = 2;
            break;
        case RDMA_INPUT_FORMAT_RGB888:
        case RDMA_INPUT_FORMAT_BGR888:
            bpp = 3;
            break;
        case RDMA_INPUT_FORMAT_ARGB8888:
        case RDMA_INPUT_FORMAT_ABGR8888:
        case RDMA_INPUT_FORMAT_RGBA8888:
        case RDMA_INPUT_FORMAT_BGRA8888:
            bpp = 4;
            break;
        default:
            DDPERR("unknown RDMA input format = %d\n", inputFormat);
            ASSERT(0);
    }
    return  bpp;
}

static unsigned int rdma_input_format_color_space(enum RDMA_INPUT_FORMAT inputFormat)
{
    int space = 0;
    switch(inputFormat)
    {
        case RDMA_INPUT_FORMAT_BGR565:
        case RDMA_INPUT_FORMAT_RGB565:
        case RDMA_INPUT_FORMAT_RGB888:
        case RDMA_INPUT_FORMAT_BGR888:
        case RDMA_INPUT_FORMAT_RGBA8888:
        case RDMA_INPUT_FORMAT_BGRA8888:
        case RDMA_INPUT_FORMAT_ARGB8888:
        case RDMA_INPUT_FORMAT_ABGR8888:
            space = 0;
            break;
        case RDMA_INPUT_FORMAT_VYUY:
        case RDMA_INPUT_FORMAT_UYVY:
        case RDMA_INPUT_FORMAT_YVYU:
        case RDMA_INPUT_FORMAT_YUYV:
            space = 1;
            break;
        default:
            DDPERR("unknown RDMA input format = %d\n", inputFormat);
            ASSERT(0);
    }
    return space;
}

static unsigned int rdma_input_format_reg_value(enum RDMA_INPUT_FORMAT inputFormat)
{
    int reg_value = 0;
    switch(inputFormat)
    {
        case RDMA_INPUT_FORMAT_BGR565:
        case RDMA_INPUT_FORMAT_RGB565:
            reg_value = 0x0;
        	break;
        case RDMA_INPUT_FORMAT_RGB888:
        case RDMA_INPUT_FORMAT_BGR888:
            reg_value = 0x1;
        	break;
        case RDMA_INPUT_FORMAT_RGBA8888:
        case RDMA_INPUT_FORMAT_BGRA8888:
            reg_value = 0x2;
        	break;
        case RDMA_INPUT_FORMAT_ARGB8888:
        case RDMA_INPUT_FORMAT_ABGR8888:
            reg_value = 0x3;
        	break;
        case RDMA_INPUT_FORMAT_VYUY:
        case RDMA_INPUT_FORMAT_UYVY:
          	reg_value = 0x4;
        	break;
        case RDMA_INPUT_FORMAT_YVYU:
        case RDMA_INPUT_FORMAT_YUYV:
        	reg_value = 0x5;
        	break;
        default:
            DDPERR("unknow RDMA input format is %d\n", inputFormat);
            ASSERT(0);
    }
    return reg_value;
}

static char*  rdma_intput_format_name(enum RDMA_INPUT_FORMAT fmt, int swap)
{
    switch(fmt)
    {
        case RDMA_INPUT_FORMAT_BGR565   :
	    {
			return swap ? "eBGR565" : "eRGB565";
    	}
        case RDMA_INPUT_FORMAT_RGB565   :
	    {
			return "eRGB565";
    	}
        case RDMA_INPUT_FORMAT_RGB888   :
		{
            return swap ? "eRGB888" : "eBGR888";
		}
        case RDMA_INPUT_FORMAT_BGR888   :
		{
            return "eBGR888";
		}
        case RDMA_INPUT_FORMAT_RGBA8888 :
		{
			return swap ? "eRGBA888" : "eBGRA888";
		}
        case RDMA_INPUT_FORMAT_BGRA8888 :
		{
			return "eBGRA888";
		}
        case RDMA_INPUT_FORMAT_ARGB8888 :
		{
			return swap ? "eARGB8888" : "eABGR8888";
		}
        case RDMA_INPUT_FORMAT_ABGR8888 :
		{
			return "eABGR8888";
		}
        case RDMA_INPUT_FORMAT_VYUY :
		{
			return swap ? "eVYUY" : "eUYVY";
		}
        case RDMA_INPUT_FORMAT_UYVY :
		{
			return "eUYVY";
		}
        case RDMA_INPUT_FORMAT_YVYU :
		{
			return swap ? "eYVYU" : "eYUY2";
		}
        case RDMA_INPUT_FORMAT_YUYV :
		{
			return "eYUY2";
		}
        default:
		{
			DDPERR("rdma_intput_format_name unknow fmt=%d, swap=%d \n", fmt, swap);
			break;
    	}
    }
    return "unknow";
}

static unsigned int rdma_index(DISP_MODULE_ENUM module)
{
    int idx = 0;
    switch(module) {
        case DISP_MODULE_RDMA0:
            idx = 0;
            break;
        case DISP_MODULE_RDMA1:
            idx = 1;
            break;
        case DISP_MODULE_RDMA2:
            idx = 2;
            break;
        default:
            DDPERR("invalid rdma module=%d \n", module);// invalid module
            ASSERT(0);
    }
    return idx;
}

unsigned int rdma_disable_irq_backup(DISP_MODULE_ENUM module, void *handle)
{
	unsigned int idx = rdma_index(module);
    ASSERT(idx <= 2);
	unsigned int old_val;

	old_val = DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_ENABLE);
	DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_ENABLE, 0x0);

    return old_val;
}

int rdma_irq_restore(DISP_MODULE_ENUM module, void *handle, unsigned int irq_val)
{
	unsigned int idx = rdma_index(module);
    ASSERT(idx <= 2);
	DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_ENABLE, irq_val);

    return 0;
}



static int rdma_start(DISP_MODULE_ENUM module, void * handle) {
    unsigned int idx = rdma_index(module);
    ASSERT(idx <= 2);

    DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_ENABLE, 0x1F);
    DISP_REG_SET_FIELD(handle,GLOBAL_CON_FLD_ENGINE_EN, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON, 1);

    return 0;
}

static int rdma_stop(DISP_MODULE_ENUM module,void * handle) {
    unsigned int idx = rdma_index(module);
    ASSERT(idx <= 2);

    DISP_REG_SET_FIELD(handle,GLOBAL_CON_FLD_ENGINE_EN, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON, 0);
    DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_ENABLE, 0);
    DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS, 0);
    return 0;
}

static void udelay(int us)
{
	ASSERT(0);
}

static int rdma_reset(DISP_MODULE_ENUM module,void * handle) {
    unsigned int delay_cnt=0;
    int ret =0;
    unsigned int idx = rdma_index(module);

    ASSERT(idx <= 2);

    DISP_REG_SET_FIELD(handle,GLOBAL_CON_FLD_SOFT_RESET, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON, 1);
    while((DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON)&0x700)==0x100)
    {
        delay_cnt++;
        udelay(10);
        if(delay_cnt>10000)
        {
            ret = -1;
            DDPERR("rdma%d_reset timeout, stage 1! DISP_REG_RDMA_GLOBAL_CON=0x%x \n", idx, DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON));
            break;
        }
    }
    DISP_REG_SET_FIELD(handle,GLOBAL_CON_FLD_SOFT_RESET, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON, 0);
    delay_cnt =0;
    while((DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON)&0x700)!=0x100)
    {
        delay_cnt++;
        udelay(10);
        if(delay_cnt>10000)
        {
            ret = -1;
            DDPERR("rdma%d_reset timeout, stage 2! DISP_REG_RDMA_GLOBAL_CON=0x%x \n", idx, DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON));
            break;
        }
    }
    return ret;
}


int rdma_reg_backup(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned long offset = idx*DISP_RDMA_INDEX_OFFSET;

	disp_tui_reg_backup(offset+DISP_REG_RDMA_GLOBAL_CON);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_MEM_CON);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_MEM_START_ADDR);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_MEM_SRC_PITCH);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_INT_ENABLE);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_SIZE_CON_0);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_SIZE_CON_1);
	disp_tui_reg_backup(offset+DISP_REG_RDMA_FIFO_CON);

	return 0;
}


int rdma_config(DISP_MODULE_ENUM module,
                    enum RDMA_MODE mode,
                    unsigned long address,
                    DpColorFormat inFormat,
                    unsigned pitch,
                    unsigned width,
                    unsigned height,
                    unsigned ufoe_enable,
		           	DISP_BUFFER_TYPE sec,
                    void * handle)
{

    unsigned int output_is_yuv = 0;
    enum RDMA_INPUT_FORMAT inputFormat = rdma_input_format_convert(inFormat);
    unsigned int  bpp                  = rdma_input_format_bpp(inputFormat);
    unsigned int input_is_yuv          = rdma_input_format_color_space(inputFormat);
    unsigned int input_swap            = rdma_input_format_byte_swap(inputFormat);
    unsigned int input_format_reg      = rdma_input_format_reg_value(inputFormat);
    unsigned int  color_matrix         = 0x6; //0100 MTX_JPEG_TO_RGB (YUV FUll TO RGB)
    unsigned int idx = rdma_index(module);

    DDPMSG("RDMAConfig idx %d, mode %d, address 0x%x, inputformat %s, pitch %u, width %u, height %u,sec%d\n",
                         idx, mode, address, rdma_intput_format_name(inputFormat,input_swap), pitch,width, height, sec);
    ASSERT(idx <= 2);
    if((width > RDMA_MAX_WIDTH) || (height > RDMA_MAX_HEIGHT))
    {
    	  DDPERR("RDMA input overflow, w=%d, h=%d, max_w=%d, max_h=%d\n", width, height, RDMA_MAX_WIDTH, RDMA_MAX_HEIGHT);
    }
    if(input_is_yuv==1 && output_is_yuv==0)
    {
	    DISP_REG_SET_FIELD(handle,SIZE_CON_0_FLD_MATRIX_ENABLE, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 1);
	    DISP_REG_SET_FIELD(handle,SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, color_matrix);
    }
    else if(input_is_yuv==0 && output_is_yuv==1)
    {
        color_matrix = 0x2; //0x0010, RGB_TO_BT601
	    DISP_REG_SET_FIELD(handle,SIZE_CON_0_FLD_MATRIX_ENABLE, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 1);
	    DISP_REG_SET_FIELD(handle,SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, color_matrix);
    }
    else
    {
        DISP_REG_SET_FIELD(handle,SIZE_CON_0_FLD_MATRIX_ENABLE, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 0);
        DISP_REG_SET_FIELD(handle,SIZE_CON_0_FLD_MATRIX_INT_MTX_SEL, idx * DISP_RDMA_INDEX_OFFSET + DISP_REG_RDMA_SIZE_CON_0, 0);
    }

    DISP_REG_SET_FIELD(handle,GLOBAL_CON_FLD_MODE_SEL, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON, mode);
    // FORMAT & SWAP only works when RDMA memory mode, set both to 0 when RDMA direct link mode.
    DISP_REG_SET_FIELD(handle,MEM_CON_FLD_MEM_MODE_INPUT_FORMAT, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_MEM_CON, ((mode == RDMA_MODE_DIRECT_LINK) ? 0 : input_format_reg&0xf));
    DISP_REG_SET_FIELD(handle,MEM_CON_FLD_MEM_MODE_INPUT_SWAP, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_MEM_CON, ((mode == RDMA_MODE_DIRECT_LINK) ? 0 : input_swap));

	if(sec != DISP_SECURE_BUFFER) {
    DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_MEM_START_ADDR, address);
	} else {
		ASSERT(0);
	}

    DISP_REG_SET(handle,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_MEM_SRC_PITCH, pitch);
    DISP_REG_SET_FIELD(handle,SIZE_CON_0_FLD_OUTPUT_FRAME_WIDTH, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_SIZE_CON_0, width);
    DISP_REG_SET_FIELD(handle,SIZE_CON_1_FLD_OUTPUT_FRAME_HEIGHT, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_SIZE_CON_1, height);

    if(ufoe_enable==0)  //UFOE bypassed, enable RDMA underflow intr, else disable RDMA underflow intr
    {
	    DISP_REG_SET_FIELD(handle,FIFO_CON_FLD_FIFO_UNDERFLOW_EN, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_FIFO_CON, 1);
        // DISP_REG_SET_FIELD(handle,FIFO_CON_FLD_OUTPUT_VALID_FIFO_THRESHOLD, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_FIFO_CON, 0x100);
    }
    else
    {
	    DISP_REG_SET_FIELD(handle,FIFO_CON_FLD_FIFO_UNDERFLOW_EN, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_FIFO_CON, 1);
		// force set rdma output threshold to maximum value, to avoid ufoe caused rdma underflow.
        DISP_REG_SET_FIELD(handle,FIFO_CON_FLD_OUTPUT_VALID_FIFO_THRESHOLD, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_FIFO_CON, 496); //FHD:304, HD:203, QHD:151
        //DISP_REG_SET_FIELD(handle,FIFO_CON_FLD_OUTPUT_VALID_FIFO_THRESHOLD, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_FIFO_CON, width*3*3/16/2); //FHD:304, HD:203, QHD:151
    }

    return 0;
}

int rdma_is_mem_mode(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
    enum RDMA_MODE mode;

    ASSERT(idx <= 2);
    mode = DISP_REG_GET_FIELD(GLOBAL_CON_FLD_MODE_SEL, idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_GLOBAL_CON);
	return mode == RDMA_MODE_MEMORY;

}

#define RDMA_INT_FRAME_START_BIT 1
#define RDMA_INT_FRAME_DONE_BIT 2
#define RDMA_INT_FRAME_ABNORMAL_BIT 3
#define RDMA_INT_FRAME_UNDERFLOW_BIT 4

int rdma_wait_frame_done_by_polling(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	/* clear interrupt first*/
    DISP_REG_SET(NULL,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS, 0x0);

	while(1) {

		regval = DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS);
	    DISP_REG_SET(NULL,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS, ~regval);

		if(regval & (1<<RDMA_INT_FRAME_ABNORMAL_BIT)) {
			DDPMSG("error: rdma frame abnormal int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_UNDERFLOW_BIT)) {
			DDPMSG("error: rdma frame underflow int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_DONE_BIT)) {
			return 0;
		}
	}
}

static int rdma_irq_num[2] = {227, 0};
static int rdma_irq_attached[2] = {0, 0};

int rdma_irq_attach(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	int irq_num = rdma_irq_num[idx];
	drApiResult_t ret;

	if(!rdma_irq_attached[idx]) {
		ret = drApiIntrAttach(irq_num, INTR_MODE_LOW_LEVEL);
		if(ret != DRAPI_OK){
			DDPMSG("error to attach irq %d flag=%d, ret=%d\n",
				irq_num, INTR_MODE_LOW_LEVEL, ret);
			return -1;
		}
		rdma_irq_attached[idx] = 1;
	}
	return 0;
}

int rdma_irq_dettach(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	int irq_num = rdma_irq_num[idx];
	drApiResult_t ret;

	if(rdma_irq_attached[idx]) {
		ret = drApiIntrDetach(irq_num);
		if(ret != DRAPI_OK) {
			DDPMSG("error to detach irq %d ret=%d\n",
				irq_num, ret);
			return -1;
		}
		rdma_irq_attached[idx] = 0;
	}
	return 0;
}

int rdma_wait_frame_done_by_interrupt(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	unsigned int regval;
	int irq_num = rdma_irq_num[idx];
	drApiResult_t ret;

	if(!rdma_irq_attached[idx]) {
		ret = rdma_irq_attach(module);
		if(ret)
			goto err1;
	}

    DISP_REG_SET(NULL,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS, 0x0);
	rdma_irq_restore(module, NULL, 0x1C);

	while(1) {
		ret = drApiWaitForIntr(irq_num, TIME_INFINITE, NULL);
		if(ret != DRAPI_OK){
			DDPMSG("error to wait irq %d time=%d, ret=%d\n",
				irq_num, TIME_INFINITE, ret);
			goto err1;
		}

		regval = DISP_REG_GET(idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS);
	    DISP_REG_SET(NULL,idx*DISP_RDMA_INDEX_OFFSET+ DISP_REG_RDMA_INT_STATUS, ~regval);

		if(regval & (1<<RDMA_INT_FRAME_ABNORMAL_BIT)) {
			DDPMSG("error: rdma frame abnormal int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_UNDERFLOW_BIT)) {
			DDPMSG("error: rdma frame underflow int happens 0x%x\n", regval);
		}

		if(regval & (1<<RDMA_INT_FRAME_DONE_BIT)) {
			rdma_irq_restore(module, NULL, 0);
			return 0;
		}
	}

	return 0;

err1:
	rdma_irq_restore(module, NULL, 0);
	return rdma_wait_frame_done_by_polling(module);

}

CMDQ_EVENT_ENUM rdma_get_EOF_cmdq_event(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
	CMDQ_EVENT_ENUM event;

	if(idx == 0)
		event = CMDQ_EVENT_DISP_RDMA0_EOF;
	else if(idx == 1)
		event = CMDQ_EVENT_DISP_RDMA1_EOF;
	else if(idx == 2)
		event = CMDQ_EVENT_DISP_RDMA2_EOF;
	else
		ASSERT(0);

	return event;
}

int rdma_wait_frame_done_by_cmdq(DISP_MODULE_ENUM module, cmdqRecHandle cmdq_handle,
			int clear_before_wait, int clear_after_wait)
{
	CMDQ_EVENT_ENUM event;
	int ret = 0;
	ASSERT(cmdq_handle != NULL);
	event = rdma_get_EOF_cmdq_event(module);

	if(clear_before_wait)
		ret |= cmdqRecClearEventToken(cmdq_handle, event);

	if(clear_after_wait)
		ret |= cmdqRecWait(cmdq_handle, event);
	else
		ret |= cmdqRecWaitNoClear(cmdq_handle, event);

	if(ret)
		DDPERR("%s error ret=%d\n", __func__);

	return ret;
}

void rdma_dump_reg(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
    unsigned int off_st = DISP_RDMA_INDEX_OFFSET*idx;
    DDPDUMP("== DISP RDMA%d REGS ==\n", idx);
    DDPDUMP("RDMA:0x000=0x%08x,0x004=0x%08x,0x010=0x%08x,0x014=0x%08x\n",
                  DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_INT_STATUS + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0 + off_st));

    DDPDUMP("RDMA:0x018=0x%08x,0x01c=0x%08x,0x024=0x%08x,0xf00=0x%08x\n",
                  DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1 + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_TARGET_LINE + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_MEM_CON + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + off_st));

     DDPDUMP("RDMA:0x02c=0x%08x,0x030=0x%08x,0x034=0x%08x,0x038=0x%08x\n",
                  DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0 + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_MEM_SLOW_CON + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_1 + off_st));

     DDPDUMP("RDMA:0x040=0x%08x,0x044=0x%08x,0x078=0x%08x,0x07c=0x%08x\n",
                  DISP_REG_GET(DISP_REG_RDMA_FIFO_CON + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_FIFO_LOG + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_0 + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_1 + off_st));

      DDPDUMP("RDMA:0x080=0x%08x,0x084=0x%08x,0x088=0x%08x,0x08c=0x%08x\n",
                  DISP_REG_GET(DISP_REG_RDMA_PRE_ADD_2 + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_POST_ADD_0 + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_POST_ADD_1 + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_POST_ADD_2 + off_st));

      DDPDUMP("RDMA:0x090=0x%08x,0x094=0x%08x,0x094=0x%08x\n",
                  DISP_REG_GET(DISP_REG_RDMA_DUMMY + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_DEBUG_OUT_SEL + off_st),
                  DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR + off_st));
    return;
}

void rdma_dump_analysis(DISP_MODULE_ENUM module)
{
    unsigned int idx = rdma_index(module);
    DDPDUMP("==DISP RDMA%d ANALYSIS==\n", idx);
    DDPDUMP("rdma%d: en=%d, w=%d, h=%d, pitch=%d, addr=0x%x, fmt=%s, fifo_min=%d\n",
        idx,
        DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON+DISP_RDMA_INDEX_OFFSET*idx)&0x1,
        DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0+DISP_RDMA_INDEX_OFFSET*idx)&0xfff,
        DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1+DISP_RDMA_INDEX_OFFSET*idx)&0xfffff,
        DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH+DISP_RDMA_INDEX_OFFSET*idx),
        DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR+DISP_RDMA_INDEX_OFFSET*idx),
        rdma_intput_format_name(
            (DISP_REG_GET(DISP_REG_RDMA_MEM_CON+DISP_RDMA_INDEX_OFFSET*idx)>>4)&0xf,
            (DISP_REG_GET(DISP_REG_RDMA_MEM_CON+DISP_RDMA_INDEX_OFFSET*idx)>>8)&0x1
            ),
        DISP_REG_GET(DISP_REG_RDMA_FIFO_LOG+DISP_RDMA_INDEX_OFFSET*idx)
    );
    return ;
}

