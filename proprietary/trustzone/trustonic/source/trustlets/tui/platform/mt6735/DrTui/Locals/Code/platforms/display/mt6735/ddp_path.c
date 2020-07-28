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

#define LOG_TAG "ddp_path"
#include "ddp_log.h"
#include "ddp_reg.h"
#include "ddp_path.h"
//#include "primary_display.h"

#pragma GCC optimize("O0")

int ddp_path_init()
{
    // mout
    mout_map[0].reg = DISP_REG_CONFIG_DISP_OVL0_MOUT_EN;
#if defined(MTK_FB_OVL1_SUPPORT)
    mout_map[1].reg = DISP_REG_CONFIG_DISP_OVL1_MOUT_EN;
#endif
    mout_map[2].reg = DISP_REG_CONFIG_DISP_DITHER_MOUT_EN;
    mout_map[3].reg = DISP_REG_CONFIG_DISP_UFOE_MOUT_EN;
    // sel_out
    sel_out_map[0].reg = DISP_REG_CONFIG_DISP_RDMA0_SOUT_SEL_IN;
    sel_out_map[1].reg = DISP_REG_CONFIG_DISP_RDMA1_SOUT_SEL_IN;
    // sel_in
    sel_in_map[0].reg = DISP_REG_CONFIG_DISP_COLOR0_SEL_IN; // COLOR_SEL
    sel_in_map[1].reg = DISP_REG_CONFIG_DISP_UFOE_SEL_IN; //UFOE_SEL
    sel_in_map[2].reg = DISP_REG_CONFIG_DSI0_SEL_IN; //DSI0_SEL
    sel_in_map[3].reg = DISP_REG_CONFIG_DPI0_SEL_IN; //DPI0_SEL
    sel_in_map[4].reg = DISP_REG_CONFIG_DISP_WDMA0_SEL_IN; // WDMA_SEL
}

char* ddp_get_scenario_name(DDP_SCENARIO_ENUM scenario)
{
    switch(scenario)
    {
        case DDP_SCENARIO_PRIMARY_DISP:              return "primary_disp";
        case DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP: return "primary_rdma0_color0_disp";
        case DDP_SCENARIO_PRIMARY_RDMA0_DISP:        return "primary_rdma0_disp";
        case DDP_SCENARIO_PRIMARY_BYPASS_RDMA:       return "primary_bypass_rdma";
        case DDP_SCENARIO_PRIMARY_OVL_MEMOUT:        return "primary_ovl_memout";
        case DDP_SCENARIO_PRIMARY_DITHER_MEMOUT:     return "primary_dither_memout";
        case DDP_SCENARIO_PRIMARY_UFOE_MEMOUT:       return "primary_ufoe_memout";
        case DDP_SCENARIO_SUB_DISP:                  return "sub_disp";
        case DDP_SCENARIO_SUB_RDMA1_DISP:            return "sub_rdma1_disp";
        case DDP_SCENARIO_SUB_OVL_MEMOUT:            return "sub_ovl_memout";
        case DDP_SCENARIO_PRIMARY_ALL:               return "primary_all";
        case DDP_SCENARIO_SUB_ALL:                   return "sub_all";
        case DDP_SCENARIO_MULTIPLE_OVL:              return "multi_ovl";
        default:
             DDPMSG("invalid scenario id=%d\n", scenario);
             return "unknown";
    }
}

int ddp_is_scenario_on_primary(DDP_SCENARIO_ENUM scenario)
{
   int on_primary = 0;
   switch(scenario)
   {
        case DDP_SCENARIO_PRIMARY_DISP:
        case DDP_SCENARIO_PRIMARY_RDMA0_COLOR0_DISP:
        case DDP_SCENARIO_PRIMARY_RDMA0_DISP:
        case DDP_SCENARIO_PRIMARY_BYPASS_RDMA:
        case DDP_SCENARIO_PRIMARY_OVL_MEMOUT:
        case DDP_SCENARIO_PRIMARY_DITHER_MEMOUT:
        case DDP_SCENARIO_PRIMARY_UFOE_MEMOUT:
        case DDP_SCENARIO_PRIMARY_ALL:
        case DDP_SCENARIO_MULTIPLE_OVL:
            on_primary = 1;
            break;
        case DDP_SCENARIO_SUB_DISP:
        case DDP_SCENARIO_SUB_RDMA1_DISP:
        case DDP_SCENARIO_SUB_OVL_MEMOUT:
        case DDP_SCENARIO_SUB_ALL:
            on_primary = 0;
            break;
        default:
            DDPMSG("invalid scenario id=%d\n", scenario);
   }

   return on_primary;

}


char* ddp_get_mutex_sof_name(MUTEX_SOF mode)
{
    switch(mode)
    {
        case SOF_SINGLE  :    return "single";
        case SOF_DSI0    :    return "dsi0";
        case SOF_DSI1    :    return "dsi1";
        case SOF_DPI0    :    return "dpi0";
        default:
             DDPMSG("invalid sof =%d\n", mode);
             return "unknown";
    }
}

char* ddp_get_mode_name(DDP_MODE ddp_mode)
{
    switch(ddp_mode)
    {
        case DDP_VIDEO_MODE  :    return "vido_mode";
        case DDP_CMD_MODE    :    return "cmd_mode";
        default:
             DDPMSG("invalid ddp mode =%d\n", ddp_mode);
             return "unknown";
    }
}

static int ddp_get_module_num_l(int* module_list)
{
    unsigned int num = 0;
    while(*(module_list+num)!=-1)
    {
        num++;

        if(num==DDP_ENING_NUM)
          break;
    }
    return num;
}

static int _ddp_check_path_l(int *module_list, int strict)
{
    unsigned int i, j, k;
    int step = 0;
    int valid =0, ret = 0;
    unsigned int mout;
    unsigned int path_error = 0;
    unsigned int module_num = ddp_get_module_num_l(module_list);
    DDPDUMP("check_path: %s to %s\n",ddp_get_module_name(module_list[0])
            ,ddp_get_module_name(module_list[module_num-1]));
    // check mout
    for(i = 0 ; i < module_num - 1 ; i++)
    {
        for(j = 0 ; j < DDP_MOUT_NUM ; j++)
        {
            if(module_list[i] == mout_map[j].id)
            {
                mout = 0;
                //find next module which can be connected
                step = i+1;
                while(module_can_connect[module_list[step]].bit==0 && step<module_num)
                {
                   step++;
                }
                ASSERT(step<module_num);
                for(k = 0 ; k < 5 ; k++)
                {
                    if(mout_map[j].out_id_bit_map[k].m == -1)
                        break;
                    if(mout_map[j].out_id_bit_map[k].m == module_list[step])
                    {
                        mout |= mout_map[j].out_id_bit_map[k].v;
                        valid =1;
                        break;
                    }
                }
                if(valid)
                {
					int print_warning = 0;
                    valid =0;
                    if((DISP_REG_GET(mout_map[j].reg) & mout)==0)
                    {
                        path_error += 1;
						print_warning = 1;
                    }
                    else if(DISP_REG_GET(mout_map[j].reg) != mout)
                    {
						if(strict)
							path_error += 1;
						print_warning = 1;
                    }
					if(print_warning)
                        DDPDUMP("warning: %s mout expect=0x%x, real=0x%x \n",
                            ddp_get_module_name(module_list[i]),
                            mout,
                            DISP_REG_GET(mout_map[j].reg));
                }
				else
				{
                    path_error += 1;
                    DDPDUMP("error:%s mout, can't find next module, real=0x%x \n",
                        ddp_get_module_name(module_list[i]),
                        DISP_REG_GET(mout_map[j].reg));
				}
                break;
             }
        }
    }
    // check out select
    for(i = 0 ; i < module_num - 1 ; i++)
    {
        for(j = 0 ; j < DDP_SEL_OUT_NUM ; j++)
        {
            if(module_list[i] == sel_out_map[j].id)
            {
                 //find next module which can be connected
                step = i+1;
                while(module_can_connect[module_list[step]].bit==0 && step<module_num)
                {
                   step++;
                }
                ASSERT(step<module_num);
                for(k = 0 ; k < 4 ; k++)
                {
                    if(sel_out_map[j].id_bit_map[k] == -1)
                        break;
                    if(sel_out_map[j].id_bit_map[k] == module_list[step])
                    {
                        if(DISP_REG_GET(sel_out_map[j].reg) != k)
                        {
                            path_error += 1;
                            DDPDUMP("error:out_s %s not connect to %s, expect=0x%x, real=0x%x \n",
                               ddp_get_module_name(module_list[i]),ddp_get_module_name(module_list[step]),
                               k,
                               DISP_REG_GET(sel_out_map[j].reg));
                        }
                        break;
                    }
                 }
            }
        }
    }
    // check input select
    for(i = 1 ; i < module_num ; i++)
    {
        for(j = 0 ; j < DDP_SEL_IN_NUM ; j++)
        {
            if(module_list[i] == sel_in_map[j].id)
            {
                //find next module which can be connected
                step = i-1;
                while(module_can_connect[module_list[step]].bit==0 && step>0)
                {
                   step--;
                }
                 ASSERT(step>=0);
                for(k = 0 ; k < 4 ; k++)
                {
                    if(sel_in_map[j].id_bit_map[k] == -1)
                        break;
                    if(sel_in_map[j].id_bit_map[k] == module_list[step])
                    {
                        if(DISP_REG_GET(sel_in_map[j].reg) != k)
                        {
                            path_error += 1;
                            DDPDUMP("error:in_s %s not connect to %s, expect=0x%x, real=0x%x \n",
                               ddp_get_module_name(module_list[step]),ddp_get_module_name(module_list[i]),
                               k,
                               DISP_REG_GET(sel_in_map[j].reg));
                        }
                        break;
                    }
                 }
            }
        }
    }
    if(path_error == 0)
    {
        DDPDUMP("path: %s to %s is connected\n",ddp_get_module_name(module_list[0]),
               ddp_get_module_name(module_list[module_num - 1]));
    }
    else
    {
        DDPDUMP("path: %s to %s not connected!!!\n",ddp_get_module_name(module_list[0]),
        ddp_get_module_name(module_list[module_num - 1]));
    }
	return path_error;
}

static int ddp_check_path_l(int *module_list)
{
	return _ddp_check_path_l(module_list, 0);
}
static int ddp_check_path_l_strict(int *module_list)
{
	return _ddp_check_path_l(module_list, 1);
}

static MUTEX_SOF ddp_get_mutex_sof(DISP_MODULE_ENUM dest_module, DDP_MODE ddp_mode)
{
    MUTEX_SOF mode = SOF_SINGLE;
    switch(dest_module)
    {
        case DISP_MODULE_DSI0:
        {
             mode = (ddp_mode==DDP_VIDEO_MODE ? SOF_DSI0 : SOF_SINGLE);
             break;
        }
        case DISP_MODULE_DSI1:
        {
             mode = (ddp_mode==DDP_VIDEO_MODE ? SOF_DSI1 : SOF_SINGLE);
             break;
        }
        case DISP_MODULE_DSIDUAL:
        {
             mode = (ddp_mode==DDP_VIDEO_MODE ? SOF_DSI0 : SOF_SINGLE);
             break;
        }
        case DISP_MODULE_DPI:
        {
            mode = SOF_DSI1;//SOF_DPI0;   has one DSI, so the DPI should use 1 for mutex_sof
            break;
        }
        case DISP_MODULE_WDMA0:
        case DISP_MODULE_WDMA1:
            mode = SOF_SINGLE;
            break;
        default:
            DDPERR("get mutex sof, invalid param dst module = %s(%d), dis mode %s\n",
                ddp_get_module_name(dest_module), dest_module,ddp_get_mode_name(ddp_mode));
    }
    DDPDBG("mutex sof: %s dst module %s:%s\n",
                          ddp_get_mutex_sof_name(mode), ddp_get_module_name(dest_module),ddp_get_mode_name(ddp_mode));
    return mode;
}

static void ddp_check_mutex_l(int mutex_id, int* module_list, DDP_MODE ddp_mode)
{
    int i=0;
    uint32_t real_value = 0;
    uint32_t expect_value = 0;
    uint32_t real_sof  = 0;
    MUTEX_SOF  expect_sof  = SOF_SINGLE;
    int module_num = ddp_get_module_num_l(module_list);
    if(mutex_id < DISP_MUTEX_DDP_FIRST || mutex_id > DISP_MUTEX_DDP_LAST)
    {
        DDPDUMP("error:check mutex fail:exceed mutex max (0 ~ %d)\n",DISP_MUTEX_DDP_LAST);
        return;
    }
    real_value = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_MOD(mutex_id));
    for(i = 0 ; i < module_num ; i++)
    {
        if(module_mutex_map[module_list[i]].bit != -1)
            expect_value |= (1 << module_mutex_map[module_list[i]].bit);
    }
    if( expect_value != real_value )
    {
       DDPDUMP("error:mutex %d error: expect 0x%x, real 0x%x\n",mutex_id,expect_value,real_value);
    }
    real_sof  = DISP_REG_GET(DISP_REG_CONFIG_MUTEX_SOF(mutex_id));
    expect_sof = ddp_get_mutex_sof(module_list[module_num-1],ddp_mode);
    if((uint32_t)expect_sof != real_sof)
    {
       DDPDUMP("error:mutex %d sof error: expect %s, real %s\n", mutex_id,
             ddp_get_mutex_sof_name(expect_sof),
             ddp_get_mutex_sof_name((MUTEX_SOF)real_sof));
    }
}

int ddp_mutex_enable_l(int mutex_idx,void * handle)
{
    //DDPDBG("mutex %d enable\n", mutex_idx);
    DISP_REG_SET(handle,DISP_REG_CONFIG_MUTEX_EN(mutex_idx),1);
    return 0;
}

int ddp_get_module_num(DDP_SCENARIO_ENUM scenario)
{
    return ddp_get_module_num_l(module_list_scenario[scenario]);
}

static void ddp_print_scenario(DDP_SCENARIO_ENUM scenario)
{
    int i =0;
    char path[512]={'\0'};
    int num = ddp_get_module_num(scenario);
    for(i=0; i< num; i++)
    {
        strcat(path,ddp_get_module_name(module_list_scenario[scenario][i]));
    }
    DDPMSG("scenario %s have modules: %s\n",ddp_get_scenario_name(scenario),path);
}

static int  ddp_find_module_index(DDP_SCENARIO_ENUM ddp_scenario, DISP_MODULE_ENUM module)
{
    int i=0;
    for(i=0; i< DDP_ENING_NUM; i++)
    {
        if(module_list_scenario[ddp_scenario][i] == module)
        {
            return i;
        }
    }
    DDPDBG("find module: can not find module %s on scenario %s\n",ddp_get_module_name(module),
            ddp_get_scenario_name(ddp_scenario));
    return -1;
}

// set display interface when kernel init
int ddp_set_dst_module(DDP_SCENARIO_ENUM scenario, DISP_MODULE_ENUM dst_module)
{
    int i = 0;
    DDPMSG("ddp_set_dst_module, scenario=%s, dst_module=%s \n",
           ddp_get_scenario_name(scenario), ddp_get_module_name(dst_module));
    if (ddp_find_module_index(scenario,dst_module) > 0) {
        DDPDBG("%s is already on path\n",ddp_get_module_name(dst_module));
        return 0;
    }
    i = ddp_get_module_num_l(module_list_scenario[scenario])-1;
    ASSERT(i>=0);
    if (dst_module == DISP_MODULE_DSIDUAL) {
        if (i < (DDP_ENING_NUM -1)) {
            module_list_scenario[scenario][i++] = DISP_MODULE_SPLIT1;
        }else{
            DDPERR("set dst module over up bound\n");
            return -1;
        }
    }
    else
    {
        if(ddp_get_dst_module(scenario)==DISP_MODULE_DSIDUAL)
        {
            if (i >= 1) {
                module_list_scenario[scenario][i--] = -1;
            }else{
                DDPERR("set dst module over low bound\n");
                return -1;
            }
        }
    }
    module_list_scenario[scenario][i] = dst_module;
    if (scenario == DDP_SCENARIO_PRIMARY_ALL) {
        ddp_set_dst_module(DDP_SCENARIO_PRIMARY_DISP,dst_module);
    }else if(scenario==DDP_SCENARIO_SUB_ALL) {
        ddp_set_dst_module(DDP_SCENARIO_SUB_DISP,dst_module);
    }
    ddp_print_scenario(scenario);
    return 0;
}

DISP_MODULE_ENUM ddp_get_dst_module(DDP_SCENARIO_ENUM ddp_scenario)
{
    DISP_MODULE_ENUM module_name = DISP_MODULE_UNKNOWN;
    int module_num = ddp_get_module_num_l(module_list_scenario[ddp_scenario])-1;
    if(module_num >=0 )
    	module_name = module_list_scenario[ddp_scenario][module_num];

    DDPMSG("ddp_get_dst_module, scneario=%s, dst_module=%s\n",
        ddp_get_scenario_name(ddp_scenario),
        ddp_get_module_name(module_name));

    return module_name;
}

int * ddp_get_scenario_list(DDP_SCENARIO_ENUM ddp_scenario)
{
	return module_list_scenario[ddp_scenario];
}

int  ddp_is_module_in_scenario(DDP_SCENARIO_ENUM ddp_scenario, DISP_MODULE_ENUM module)
{
    int i=0;
    for(i=0; i< DDP_ENING_NUM; i++)
    {
        if(module_list_scenario[ddp_scenario][i] == module)
        {
            return 1;
        }
    }

    return 0;
}

void ddp_check_path(DDP_SCENARIO_ENUM scenario)
{
    DDPDBG("path check path on scenario %s\n", ddp_get_scenario_name(scenario));

    if(scenario == DDP_SCENARIO_PRIMARY_ALL)
    {
        ddp_check_path_l(module_list_scenario[DDP_SCENARIO_PRIMARY_DISP]);
        ddp_check_path_l(module_list_scenario[DDP_SCENARIO_PRIMARY_OVL_MEMOUT]);
    }
    else if(scenario == DDP_SCENARIO_SUB_ALL)
    {
        ddp_check_path_l(module_list_scenario[DDP_SCENARIO_SUB_DISP]);
        ddp_check_path_l(module_list_scenario[DDP_SCENARIO_SUB_OVL_MEMOUT]);
    }
    else
    {
        ddp_check_path_l(module_list_scenario[scenario]);
    }
    return ;
}

int ddp_check_path_strict(DDP_SCENARIO_ENUM scenario)
{
    DDPDBG("path check path on scenario %s\n", ddp_get_scenario_name(scenario));

    if(scenario == DDP_SCENARIO_PRIMARY_ALL)
    {
		DDPAEE("%s, not support primary_all\n", __func__);
		return -1;
    }
    else if(scenario == DDP_SCENARIO_SUB_ALL)
    {
		DDPAEE("%s, not support sub_all\n", __func__);
		return -1;
    }
    else
    {
        return ddp_check_path_l_strict(module_list_scenario[scenario]);
    }
}


void ddp_check_mutex(int mutex_id, DDP_SCENARIO_ENUM scenario, DDP_MODE mode)
{
     DDPDBG("check mutex %d on scenario %s\n",mutex_id,
             ddp_get_scenario_name(scenario));
     ddp_check_mutex_l(mutex_id,module_list_scenario[scenario],mode);
     return ;
}

int ddp_mutex_reg_backup(int mutex_id)
{
	disp_tui_reg_backup(DISP_REG_CONFIG_MUTEX_MOD(mutex_id));
	disp_tui_reg_backup(DISP_REG_CONFIG_MUTEX_SOF(mutex_id));
	return 0;
}

int ddp_check_engine_status(int mutexID)
{
    // check engines' clock bit &  enable bit & status bit before unlock mutex
    // should not needed, in comdq do?
    int result = 0;
    return result;
}

int disp_get_dst_module(DDP_SCENARIO_ENUM scenario)
{
    return ddp_get_dst_module(scenario);
}
