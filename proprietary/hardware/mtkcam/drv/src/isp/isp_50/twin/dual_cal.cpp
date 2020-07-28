#include "MyUtils.h"


/* define to replace macro */
#define print_error_message(...) dual_cal_print_error_message(__VA_ARGS__)
#define get_current_file_name(...) dual_cal_get_current_file_name(__VA_ARGS__)
/* func prototype */
static const char *dual_cal_print_error_message(DUAL_MESSAGE_ENUM n);
static const char *dual_cal_get_current_file_name(const char *filename);
static DUAL_MESSAGE_ENUM dual_cal_dmx_rrz_rmx(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_dmx_rrz_rmx_af_p2(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_dbn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_bin(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_scm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_sgm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_sl2f(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_dbs(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_adbs(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_obc(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_rmg(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_rmm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_bnr(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_cpn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_lsc(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_dcpn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_rpg(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_rmb(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
/* bits field will always calculate by unsigned int */

int dual_cal_main(const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	if (ptr_in_param && ptr_out_param)
	{
		int dual_mode = DUAL_MODE_CAL(ptr_in_param->SW.DUAL_SEL);
		if (dual_mode > 1)
		{
			if ((TWIN_SCENARIO_NORMAL == ptr_in_param->SW.TWIN_SCENARIO) ||
				(TWIN_SCENARIO_AF_FAST_P1 == ptr_in_param->SW.TWIN_SCENARIO))
			{
				/* keep binning fake DMX half size */
				result = dual_cal_dmx_rrz_rmx(dual_mode, ptr_in_param, ptr_out_param);
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_sgm(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_sl2f(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_dbs(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_adbs(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_obc(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_rmg(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_bnr(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_rmm(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_lsc(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_dcpn(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_cpn(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_rpg(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					/* restore binning DMX real size finally */
					result = dual_cal_dbn(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					/* restore binning DMX real size finally */
					result = dual_cal_bin(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					/* restore binning DMX real size finally */
					result = dual_cal_scm(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_rmb(dual_mode, ptr_in_param, ptr_out_param);
				}
			}
			else if (TWIN_SCENARIO_AF_FAST_P2 == ptr_in_param->SW.TWIN_SCENARIO)
			{
				result = dual_cal_dmx_rrz_rmx_af_p2(dual_mode, ptr_in_param, ptr_out_param);
			}
			else
			{
				result = DUAL_MESSAGE_AF_FAST_MODE_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
	}
	else
	{
		result = DUAL_MESSAGE_NULL_PTR_ERROR;
	}
	if (DUAL_MESSAGE_OK == result)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

static DUAL_MESSAGE_ENUM dual_cal_dmx_rrz_rmx(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param,
									  DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	int dmx_mode_bf = 1;/* default 2 pixel mode, before bin, align dmx, final size */
	int dmx_cut_b = 1;/* default 2 pixel mode, before bin, align dmx, cut */
	int bin_mode_af = 1;/* default 2 pixel mode, after bin, align dmx, final size */
	int bin_cut_a = 1;/* default 2 pixel mode, after bin, align dmx, cut */
	int rcp_cut_a = 1;/* default 2 pixel mode, after bin, align imgo w/ rcp3, cut */
	int amx_cut_a = 1;/* default 2 pixel mode, after bin, algin bmx, cut */
	int bmx_cut_a = 1;/* default 2 pixel mode, after bin, algin bmx, cut */
	int raw_wd[3] = {DUAL_RAW_WD, DUAL_RAW_WD_D, DUAL_RAW_WD_T};
	int dmx_valid[3];
	int dual_bits = 0x0;
	int bin_sel_h_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
		((int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_E - (int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_S):
		ptr_in_param->SW.TWIN_RAWI_XSIZE;
	/* select max of AF & RRZ loss */
	int left_loss_dmx = 0;
	int right_loss_dmx = 0;
	int left_loss_dmx_d = 0;
	int right_loss_dmx_d = 0;
	/* following to update */
	int left_loss_af = 0;
	int right_loss_af = 0;
	int left_margin_af = 0;
	int right_margin_af = 0;
	int right_loss_rrz = 0;
	int right_loss_rrz_d = 0;
	/* following to minimize */
	int left_dmx_max = 0;
	int left_dmx_max_d = 0;
	int dual_id = 0;
	/* check DMX */
	if ((0 == ptr_in_param->CAM_A_CTL_EN.Bits.DMX_EN) || (1 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL) ||
		(1 < ptr_in_param->CAM_B_CTL_SEL.Bits.DMX_SEL) ||
		((dual_mode > 2) && (ptr_in_param->CAM_B_CTL_SEL.Bits.DMX_SEL != ptr_in_param->CAM_C_CTL_SEL.Bits.DMX_SEL)) ||
		(ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_POS_SEL) ||
		((0x3 == (ptr_in_param->SW.DUAL_SEL & 0x3)) && (0 == ptr_in_param->CAM_UNI_TOP_CTL.Bits.RAW_A_XMX_IN_SEL)) ||
		((0x6 == (ptr_in_param->SW.DUAL_SEL & 0x6)) && (0 == ptr_in_param->CAM_UNI_TOP_CTL.Bits.RAW_B_XMX_IN_SEL)) ||
		((0x0 == (ptr_in_param->SW.DUAL_SEL & 0x1)) && (0 == ptr_in_param->CAM_UNI_TOP_CTL.Bits.RAW_C_XMX_IN_SEL)) ||
		((0x0 == (ptr_in_param->SW.DUAL_SEL & 0x2)) && ptr_in_param->CAM_UNI_TOP_CTL.Bits.RAW_C_XMX_IN_SEL))
	{
		result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}
	/* check height */
	if ((unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT &
		((1<<((int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN + (int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)) - 1))
	{
		result = DUAL_MESSAGE_DMX_IN_ALIGN_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	/* upddate FMT_SEL */
	ptr_out_param->CAM_B_CTL_FMT_SEL.Raw = ptr_in_param->CAM_A_CTL_FMT_SEL.Raw;
	ptr_out_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_DMXI = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXI;
	ptr_out_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_DMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO;
	ptr_out_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_AMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_AMXO;
	ptr_out_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_BMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_BMXO;
	ptr_out_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_RMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_RMXO;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_FMT_SEL.Raw = ptr_in_param->CAM_A_CTL_FMT_SEL.Raw;
		ptr_out_param->CAM_C_CTL_FMT_SEL.Bits.PIX_BUS_DMXI = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXI;
		ptr_out_param->CAM_C_CTL_FMT_SEL.Bits.PIX_BUS_DMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO;
		ptr_out_param->CAM_C_CTL_FMT_SEL.Bits.PIX_BUS_AMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_AMXO;
		ptr_out_param->CAM_C_CTL_FMT_SEL.Bits.PIX_BUS_BMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_BMXO;
		ptr_out_param->CAM_C_CTL_FMT_SEL.Bits.PIX_BUS_RMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_RMXO;
	}
	/* update bmx pixel mode */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.BMX_EN)
	{
		if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_BMXO >= 1)
		{
			result = DUAL_MESSAGE_INVALID_PIX_BUS_BMXO_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_BMXO >= 2)
		{
			/* 4 pixel mode */
			bin_mode_af = 2;
		}
		if (ptr_in_param->CAM_A_CTL_EN2.Bits.RMB_EN)
		{
			if (ptr_in_param->CAM_A_RMG_HDR_CFG.Bits.RMG_ZHDR_EN && ((1 == (int)ptr_in_param->CAM_A_CTL_SEL.Bits.RMBN_SEL) ||
				(2 == (int)ptr_in_param->CAM_A_CTL_SEL.Bits.RMBN_SEL)))
			{
				bmx_cut_a++;/* 4 pixel mode */
				bin_mode_af++;
				dmx_mode_bf++;
				dmx_cut_b++;/* w/o crop */
				bin_cut_a++;/* w/o crop */
				if (2 != ptr_in_param->CAM_A_RMB_MODE.Bits.SENSOR_TYPE)
				{
					result = DUAL_MESSAGE_RMB_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
			else
			{
				if (2 == ptr_in_param->CAM_A_RMB_MODE.Bits.SENSOR_TYPE)
				{
					result = DUAL_MESSAGE_RMB_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
			bmx_cut_a += (int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC;/* 4 pixel mode */
			bin_mode_af += (int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC;
			dmx_mode_bf += (int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC;
			dmx_cut_b += (int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC;/* w/o crop */
			bin_cut_a += (int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC;/* w/o crop */
		}
	}
	if (ptr_in_param->CAM_A_CTL_EN2.Bits.AMX_EN)
	{
		int bin_mode_af_tmp = 1;
		if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_AMXO >= 2)
		{
			/* 4 pixel mode */
			bin_mode_af_tmp = 2;
			amx_cut_a = 2;
		}
		if (bin_mode_af < bin_mode_af_tmp)
		{
			bin_mode_af = bin_mode_af_tmp;
		}
	}
	/* update dmx pixel mode */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
	{
		dmx_mode_bf += 1;
		dmx_cut_b += 1;/* w/o crop */
	}
	if (ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN)
	{
		/* DBN in x8 align */
		dmx_mode_bf += 1;
		dmx_cut_b += 1;/* w/o crop */
	}
	if (ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN)
	{
		dmx_mode_bf += 1;
		dmx_cut_b += 1;
		if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO < 1)
		{
			result = DUAL_MESSAGE_INVALID_PIX_BUS_DMXO_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	/* check PIX_BUS_DMXO */
	if ((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN +
		(int)ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN > (int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO)
	{
		result = DUAL_MESSAGE_INVALID_PIX_BUS_DMXO_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	/* final sync with PIX_BUS_DMXI */
	if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXI >= 2)
	{
		/* 4 pixel mode */
		if (dmx_mode_bf < 2)
		{
			bin_mode_af += 2 - dmx_mode_bf;
			dmx_mode_bf = 2;
		}
		if (dmx_cut_b < 2)
		{
			bin_cut_a += 2 - dmx_cut_b;/* w/o crop */
			dmx_cut_b = 2;
		}
	}
	if (2 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)
	{
		if ((ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXI != ptr_in_param->CAM_UNI_TOP_FMT_SEL.Bits.PIX_BUS_RAWI) ||
			(0 == ptr_in_param->CAM_UNI_TOP_MOD_EN.Bits.UNP2_A_EN))
		{
			result = DUAL_MESSAGE_INVALID_PIX_BUS_DMXI_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	/* check dmx full size, align with PIX_BUS_DMXI */
	if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXI > 2)
	{
		if (bin_sel_h_size & 0x3)
		{
			result = DUAL_MESSAGE_DMX_IN_ALIGN_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	else
	{
		if (bin_sel_h_size & 0x1)
		{
			result = DUAL_MESSAGE_DMX_IN_ALIGN_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	/* check dmx full size, align with bin in */
	if (bin_sel_h_size & (DUAL_ALIGN_PIXEL_MODE(dmx_mode_bf) - 1))
	{
		result = DUAL_MESSAGE_DMX_OUT_ALIGN_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	/* shift size by binning modules enable */
	bin_sel_h_size = (unsigned int)bin_sel_h_size>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
		(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN);
	/* check after bin size, align with bin mode */
	if (bin_sel_h_size & (DUAL_ALIGN_PIXEL_MODE(bin_mode_af) - 1))
	{
		result = DUAL_MESSAGE_BMX_OUT_ALIGN_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	/* Error check */
	if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN && (((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL) &&
		(((0 == ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN) && (0 == ptr_in_param->CAM_A_CTL_EN.Bits.UFE_EN)) ||
		 (ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN && (1 == ptr_in_param->CAM_A_CTL_SEL.Bits.UFE_SEL)) ||
		 (ptr_in_param->CAM_A_CTL_EN.Bits.UFE_EN && (1 != ptr_in_param->CAM_A_CTL_SEL.Bits.UFE_SEL)) ||
		 (0 == ptr_in_param->CAM_A_CTL_EN.Bits.RCP3_EN)))))
	{
		result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	/* align RCP3 for IMGO, only sync for IMGO_A_XSIZE */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.RCP3_EN && (ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN || ptr_in_param->CAM_A_CTL_EN.Bits.UFE_EN) &&
		ptr_in_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN && (2 == ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL))
	{
	    if (1 == ptr_in_param->CAM_A_CTL_SEL.Bits.UFE_SEL)
		{
		    rcp_cut_a = 6;
	    }
	    else
	    {
		    if (DUAL_OUT_FMT_BAYER_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
		    {
			    /* 12 bits raw 4n */
			    if (rcp_cut_a < 2)
			    {
				    rcp_cut_a = 2;
			    }
		    }
		    else if ((DUAL_OUT_FMT_BAYER_10 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT) ||
			         (DUAL_OUT_FMT_BAYER_14 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT))
		    {
			    /* 10 & 14 bits raw 8n */
			    if (rcp_cut_a < 3)
			    {
				    rcp_cut_a = 3;
			    }
		    }
		    else if (DUAL_OUT_FMT_MIPI_10 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
		    {
			    /* MIPI 10 bits raw 32n */
			    if (rcp_cut_a < 5)
			    {
				    rcp_cut_a = 5;
			    }
		    }
		}
	}
	/* RTL verif or platform */
	if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
	{
		if (ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN)
		{
			if ((ptr_in_param->CAM_A_AF_CON2.Bits.AF_DS_EN || ptr_in_param->CAM_A_AF_CON.Bits.AF_H_GONLY) && (dual_mode > 2))
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			if (ptr_in_param->CAM_A_AF_CON2.Bits.AF_DS_EN && ptr_in_param->CAM_A_AF_CON.Bits.AF_H_GONLY)
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			left_margin_af = 0;
			/* AF FAST P1 */
			if (TWIN_SCENARIO_AF_FAST_P1 == ptr_in_param->SW.TWIN_SCENARIO)
			{
				if ((int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				else
				{
					if (dual_mode <= 2)
					{
						left_loss_af = DUAL_AF_TAPS*2;
						right_loss_af = DUAL_AF_TAPS*2 + 2;
					}
					else
					{
						left_loss_af = DUAL_AF_TAPS;
						right_loss_af = DUAL_AF_TAPS;
					}
					right_margin_af = DUAL_AF_MAX_BLOCK_WIDTH - 2;
				}
			}
			else
			{
				left_loss_af = (ptr_in_param->CAM_A_AF_CON.Bits.AF_H_GONLY || ptr_in_param->CAM_A_AF_CON2.Bits.AF_DS_EN)?(DUAL_AF_TAPS*2):DUAL_AF_TAPS;
				right_loss_af = ptr_in_param->CAM_A_AF_CON.Bits.AF_H_GONLY?(DUAL_AF_TAPS*2):
					(ptr_in_param->CAM_A_AF_CON2.Bits.AF_DS_EN?(DUAL_AF_TAPS*2 + 2):DUAL_AF_TAPS);
				if ((int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				right_margin_af = (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 2;
			}
		}
		else
		{
			if ((ptr_in_param->CAM_A_AF_CON2.Bits.AF_DS_EN || ptr_in_param->CAM_A_AF_CON.Bits.AF_H_GONLY) && (dual_mode > 2))
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			if (ptr_in_param->CAM_A_AF_CON2.Bits.AF_DS_EN && ptr_in_param->CAM_A_AF_CON.Bits.AF_H_GONLY)
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			if (dual_mode <= 2)
			{
				left_loss_af = DUAL_AF_TAPS*2;
				right_loss_af = DUAL_AF_TAPS*2 + 2;
			}
			else
			{
				left_loss_af = DUAL_AF_TAPS;
				right_loss_af = DUAL_AF_TAPS;
			}
			left_margin_af = 0;
			right_margin_af = DUAL_AF_MAX_BLOCK_WIDTH - 2;
		}
	}
	/* RRZ ofst check */
	if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.RRZO_EN)
	{
		if (bin_sel_h_size <= ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST + ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
			(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))
		{
			result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	for (int i=0;i<5;i++)
	{
		int left_loss_check = 0;
		int right_loss_check = 0;
		int left_loss_check_d = 0;
		int right_loss_check_d = 0;
		switch (i)
		{
			case 0:/* rcp */
				/* sync crop */
			    if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && (rcp_cut_a > 1))
				{
				    /* IMGO before BIN */
				    right_loss_check = DUAL_ALIGN_PIXEL_MODE(rcp_cut_a - (int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN) - 2;
				}
				else
				{
				    right_loss_check = DUAL_ALIGN_PIXEL_MODE(rcp_cut_a) - 2;
				}
				right_loss_check_d = right_loss_check;
				break;
			case 1:/* af */
				left_loss_check = left_loss_af + left_margin_af;
				right_loss_check = right_loss_af + right_margin_af;
				left_loss_check_d = left_loss_check;
				right_loss_check_d = right_loss_check;
				break;
			case 2:/* amx */
				right_loss_check = DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 2;
				right_loss_check_d = right_loss_check;
				break;
			case 3:/* bmx */
				right_loss_check = DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 2;
				right_loss_check_d = right_loss_check;
				break;
			case 4:/* rrz */
				if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.RRZO_EN)
				{
					if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
					{
						if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
						{
							/* default 12b */
							right_loss_check = (((unsigned int)((((long long)64*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
								ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
								ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							right_loss_check_d = (((unsigned int)((((long long)32*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
								ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
								ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							left_loss_check_d = (((unsigned int)((((long long)32*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
								ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
								ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
						}
						else
						{
							right_loss_check = (((unsigned int)((((long long)128*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
								ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
								ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							right_loss_check_d = (((unsigned int)((((long long)64*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
								ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
								ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							left_loss_check_d = (((unsigned int)((((long long)64*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
								ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
								ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
						}
					}
					else
					{
						if (DUAL_OUT_FMT_FG_8 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
						{
							/* min RLB align */
							right_loss_check = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?16:DUAL_RLB_ALIGN_PIX)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							right_loss_check_d = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?8:DUAL_RLB_ALIGN_PIX)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							left_loss_check_d = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?8:0)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
						}
						else if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
						{
							right_loss_check = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?32:DUAL_RLB_ALIGN_PIX)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							right_loss_check_d = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?16:DUAL_RLB_ALIGN_PIX)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							left_loss_check_d = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?16:0)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
						}
						else
						{
							/* new 32b */
							right_loss_check = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?28:DUAL_RLB_ALIGN_PIX)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							left_loss_check = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?4:0)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							right_loss_check_d = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?12:DUAL_RLB_ALIGN_PIX)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							left_loss_check_d = (((unsigned int)(((long long)(ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?20:0)*
								(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
								(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
								ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
						}
					}
				}
				else
				{
					if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_RMXO > 1)
					{
						if (ptr_in_param->CAM_A_CTL_EN.Bits.RRZ_EN)
						{
							right_loss_check = (((unsigned int)((((long long)DUAL_RLB_ALIGN_PIX*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
							    ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
								ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							right_loss_check_d = right_loss_check;
						}
						else
						{
							right_loss_check = 2;
							right_loss_check_d = 2;
						}
					}
				}
				right_loss_check += DUAL_TILE_LOSS_RRZ;
				left_loss_check += DUAL_TILE_LOSS_RRZ_L;
				right_loss_check_d += DUAL_TILE_LOSS_RRZ;
				left_loss_check_d += DUAL_TILE_LOSS_RRZ_L;
				right_loss_rrz = right_loss_check;
				right_loss_rrz_d = right_loss_check_d;
				break;
			default:
				break;
		}
		if (left_loss_dmx < left_loss_check)
		{
			left_loss_dmx = left_loss_check;
		}
		if (right_loss_dmx < right_loss_check)
		{
			right_loss_dmx = right_loss_check;
		}
		if (left_loss_dmx_d < left_loss_check_d)
		{
			left_loss_dmx_d = left_loss_check_d;
		}
		if (right_loss_dmx_d < right_loss_check_d)
		{
			right_loss_dmx_d = right_loss_check_d;
		}
	}
	/* padding left_loss & right_loss */
	left_loss_dmx += DUAL_TILE_LOSS_ALL_L;
	right_loss_dmx += DUAL_TILE_LOSS_ALL;
	left_loss_dmx_d += DUAL_TILE_LOSS_ALL_L;
	right_loss_dmx_d += DUAL_TILE_LOSS_ALL;
	/* update bin_cut_a for DMX to ensure HDR bit-true */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
	{
		/* remarked for af fast mode */
		if (ptr_in_param->CAM_A_RMG_HDR_CFG.Bits.RMG_ZHDR_EN)
		{
			/* DMX_A_STR, DMX_B_END, x4 after bin, x8 before bin */
			if (bin_cut_a < 2)
			{
				bin_cut_a = 2;/* w/o crop */
			}
		}
	}
	if (ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN)
	{
		/* DMX_A_STR, DMX_B_END, x4 after scm, x8 before scm */
		if (bin_cut_a < 2)
		{
			bin_cut_a = 2;/* w/o crop */
		}
	}
	/* dispatch max buffer size */
	for (int i=0;i<5;i++)
	{
		if ((1<<i) & ptr_in_param->SW.DUAL_SEL)
		{
			switch (i)
			{
				case 0:
				case 4:
					raw_wd[dual_id] = DUAL_RAW_WD;
					break;
				case 1:
					raw_wd[dual_id] = DUAL_RAW_WD_D;
					break;
				case 2:
					raw_wd[dual_id] = DUAL_RAW_WD_T;
					break;
			}
			dual_id++;
		}
	}
	/* minus loss */
	dmx_valid[0] = raw_wd[0] - right_loss_dmx - (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 2);
	if (dual_mode <= 2)
	{
		dmx_valid[1] = raw_wd[1] - left_loss_dmx - (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 2);
	}
	else
	{
		dmx_valid[1] = raw_wd[1] - left_loss_dmx - right_loss_dmx_d - 2*(DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 2);
		dmx_valid[2] = raw_wd[2] - left_loss_dmx_d - (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 2);
	}
	/* with af offset */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN)
	{
		/* normal */
		if (TWIN_SCENARIO_NORMAL == ptr_in_param->SW.TWIN_SCENARIO)
		{
			/* cal valid af config*/
			if ((ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE & 0x1) || (ptr_in_param->SW.TWIN_AF_OFFSET & 0x1))
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			/* RTL verif or platform */
			if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
			{
				/* with af offset */
				if (ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
				{
					if (bin_sel_h_size < ptr_in_param->SW.TWIN_AF_OFFSET + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
					{
						result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				else
				{
					result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
		}
	}
	/* dispatch dmx size */
	for (int i=0;i<dual_mode;i++)
	{
		int bin_sel_h_temp = bin_sel_h_size;
		int even_count = dual_mode - i;
		bool found_flag = false;
		int even_size[3];
		for (int j=0;j<dual_mode;j++)
		{
			if (0x0 == (dual_bits & (1<<j)))
			{
				even_size[j] = (1 == even_count)?(((bin_sel_h_temp + 1)>>1)<<1):
					((((bin_sel_h_temp + even_count - 1)/even_count + 1)>>1)<<1);
				bin_sel_h_temp -= even_size[j];
				even_count--;
				if (even_size[j] > dmx_valid[j])
				{
					/* keep max size and continue to dispatch */
					bin_sel_h_size -=  dmx_valid[j];
					dual_bits |= (1<<j);
					found_flag = true;
					if (ptr_in_param->SW.TWIN_SCENARIO)
					{
						dual_driver_printf("Error: non-averaged dmx size: dual mode: %d, j: %d, desired: %d > valid: %d\r\n", dual_mode, j, even_size[j], dmx_valid[j]);
						result = DUAL_MESSAGE_AF_FAST_MODE_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					break;
				}
			}
		}
		if (false == found_flag)
		{
			/* average then mask invalid */
			for (int j=0;j<dual_mode;j++)
			{
				if (0x0 == (dual_bits & (1<<j)))
				{
					dmx_valid[j] = even_size[j];
					bin_sel_h_size -= even_size[j];
					dual_bits |= (1<<j);
				}
			}
			break;
		}
	}
	/* check error */
	if (bin_sel_h_size || (dual_bits + 1 != (1 <<dual_mode)))
	{
	    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}
	/* DMX */
	ptr_out_param->CAM_A_DMX_CTL.Bits.DMX_EDGE = 0xF;
	ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X = 0;
	if ((dmx_valid[0] + right_loss_dmx) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1))
	{
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X = dmx_valid[0] + right_loss_dmx - 1 + DUAL_ALIGN_PIXEL_MODE(bin_cut_a) -
			((dmx_valid[0] + right_loss_dmx) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1));
	}
	else
	{
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X = dmx_valid[0] + right_loss_dmx - 1;
	}
	if (((int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + raw_wd[0] <= (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X) ||
		((int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X >= dmx_valid[0] + dmx_valid[1]))
	{
	    result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}
	/* check min size */
	left_dmx_max = dmx_valid[0] + DUAL_TILE_LOSS_ALL;
	if (dual_mode > 2)
	{
		left_dmx_max_d = dmx_valid[0] + dmx_valid[1] + DUAL_TILE_LOSS_ALL;
	}
	/* DMX_D */
	ptr_out_param->CAM_B_CTL_EN.Bits.DMX_EN = 1;
	ptr_out_param->CAM_B_DMX_CTL.Bits.DMX_EDGE = 0xF;
	if (dmx_valid[0] <= left_loss_dmx)
	{
		result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	else
	{
		if ((dmx_valid[0] - left_loss_dmx) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1))
		{
			ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X = dmx_valid[0] - left_loss_dmx -
				((dmx_valid[0] - left_loss_dmx) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1));
		}
		else
		{
			ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X = dmx_valid[0] - left_loss_dmx;
		}
	}
	if (dual_mode <= 2)
	{
		/* A+B, DMX_B_END_X */
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X = dmx_valid[0] + dmx_valid[1] - 1;
		if (((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + raw_wd[1] <= (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X) ||
			((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X >= dmx_valid[0] + dmx_valid[1]))
		{
			result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	else
	{
		/* change DMX_B_END_X */
		if ((dmx_valid[0] + dmx_valid[1] + right_loss_dmx_d) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1))
		{
			ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X = dmx_valid[0] + dmx_valid[1] + right_loss_dmx_d +
				(DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1) -
				((dmx_valid[0] + dmx_valid[1] + right_loss_dmx_d) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1));
		}
		else
		{
			ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X = dmx_valid[0] + dmx_valid[1] + right_loss_dmx_d - 1;
		}
		if (((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + raw_wd[1] <= (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X) ||
			((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X >= dmx_valid[0] + dmx_valid[1] + dmx_valid[2]))
		{
			result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* to revise for DMX_T */
		ptr_out_param->CAM_C_CTL_EN.Bits.DMX_EN = 1;
		ptr_out_param->CAM_C_DMX_CTL.Bits.DMX_EDGE = 0xF;
		if ((dmx_valid[0] + dmx_valid[1] - left_loss_dmx_d) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1))
		{
			ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X = dmx_valid[0] + dmx_valid[1] - left_loss_dmx_d -
				((dmx_valid[0] + dmx_valid[1] - left_loss_dmx_d) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1));
		}
		else
		{
			ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X = dmx_valid[0] + dmx_valid[1] - left_loss_dmx_d;
		}
		ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X = dmx_valid[0] + dmx_valid[1] + dmx_valid[2] - 1;
		if (((int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X + raw_wd[2] <= (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X) ||
			((int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X >= dmx_valid[0] + dmx_valid[1] + dmx_valid[2]))
		{
			result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	/* BMX */
	ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_STR_X = 0;
	if (dmx_valid[0] & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1))
	{
		ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X = dmx_valid[0] - 1 + DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) -
			(dmx_valid[0] & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1));
	}
	else
	{
		ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X = dmx_valid[0] - 1;
	}
	/* check min size */
	if (left_dmx_max < (int)ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X + DUAL_TILE_LOSS_ALL + 1)
	{
		left_dmx_max = (int)ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X + DUAL_TILE_LOSS_ALL + 1;
	}
	ptr_out_param->CAM_A_BMX_CTL.Bits.BMX_EDGE = 0xF;
	ptr_out_param->CAM_A_BMX_VSIZE.Bits.BMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
		((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN);
	/* add BMX_A_SIG_MODE1 & 2 */
	ptr_out_param->CAM_A_BMX_CTL.Bits.BMX_SINGLE_MODE_1 = 0;
	ptr_out_param->CAM_A_BMX_CTL.Bits.BMX_SINGLE_MODE_2 = 0;
	if ((ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_STR_X & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1)) ||
		(((int)ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1)))
	{
		result = DUAL_MESSAGE_BMX_SIZE_CAL_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	/* BMX_D */
	ptr_out_param->CAM_B_CTL_SEL.Bits.RMBN_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RMBN_SEL;
	ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X = (int)ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X + 1 -
		(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
	ptr_out_param->CAM_B_CTL_EN.Bits.BMX_EN = ptr_in_param->CAM_A_CTL_EN.Bits.BMX_EN;
	ptr_out_param->CAM_B_BMX_CTL.Bits.BMX_EDGE = 0xF;
	ptr_out_param->CAM_B_BMX_VSIZE.Bits.BMX_HT = ptr_out_param->CAM_A_BMX_VSIZE.Bits.BMX_HT;
	if (dual_mode <= 2)
	{
		/* add BMX_B_SIG_MODE1 & 2 */
		ptr_out_param->CAM_B_BMX_CTL.Bits.BMX_SINGLE_MODE_1 = 1;
		ptr_out_param->CAM_B_BMX_CTL.Bits.BMX_SINGLE_MODE_2 = 0;
		/* A+B, BMX_B_END_X */
		ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
	}
	else
	{
		/* update BMX_B_SIG_MODE1 & 2 */
		ptr_out_param->CAM_B_BMX_CTL.Bits.BMX_SINGLE_MODE_1 = 0;
		ptr_out_param->CAM_B_BMX_CTL.Bits.BMX_SINGLE_MODE_2 = 0;
		/* change BMX_B_END_X */
		if ((dmx_valid[0] + dmx_valid[1]) & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1))
		{
			ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X = dmx_valid[0] + dmx_valid[1] + (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1) -
				((dmx_valid[0] + dmx_valid[1]) & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1)) - (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		}
		else
		{
			ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X = dmx_valid[0] + dmx_valid[1] - 1 - (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		}
		/* to revise for BMX_T */
		ptr_out_param->CAM_C_CTL_SEL.Bits.RMBN_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RMBN_SEL;
		ptr_out_param->CAM_C_BMX_CROP.Bits.BMX_STR_X = (int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X + 1 +
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X - (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_C_BMX_CROP.Bits.BMX_END_X = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
		/* check min size */
		if (left_dmx_max_d < (int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + DUAL_TILE_LOSS_ALL + 1)
		{
			left_dmx_max_d = (int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + DUAL_TILE_LOSS_ALL + 1;
		}
		ptr_out_param->CAM_C_CTL_EN.Bits.BMX_EN = ptr_in_param->CAM_A_CTL_EN.Bits.BMX_EN;
		ptr_out_param->CAM_C_BMX_CTL.Bits.BMX_EDGE = 0xF;
		ptr_out_param->CAM_C_BMX_VSIZE.Bits.BMX_HT = ptr_out_param->CAM_A_BMX_VSIZE.Bits.BMX_HT;
		/* add BMX_C_SIG_MODE1 & 2 */
		ptr_out_param->CAM_C_BMX_CTL.Bits.BMX_SINGLE_MODE_1 = 1;
		ptr_out_param->CAM_C_BMX_CTL.Bits.BMX_SINGLE_MODE_2 = 0;
		if ((ptr_out_param->CAM_C_BMX_CROP.Bits.BMX_STR_X & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1)) ||
			(((int)ptr_out_param->CAM_C_BMX_CROP.Bits.BMX_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1)))
		{
			result = DUAL_MESSAGE_BMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	if ((ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1)) ||
		(((int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1)))
	{
		result = DUAL_MESSAGE_BMX_SIZE_CAL_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	/* AMX */
	ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_STR_X = 0;
	if (dmx_valid[0] & (DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1))
	{
		ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_END_X = dmx_valid[0] - 1 + DUAL_ALIGN_PIXEL_MODE(amx_cut_a) -
			(dmx_valid[0] & (DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1));
	}
	else
	{
		ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_END_X = dmx_valid[0] - 1;
	}
	/* check min size */
	if (left_dmx_max < (int)ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_END_X + DUAL_TILE_LOSS_ALL + 1)
	{
		left_dmx_max = (int)ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_END_X + DUAL_TILE_LOSS_ALL + 1;
	}
	ptr_out_param->CAM_A_AMX_CTL.Bits.AMX_EDGE = 0xF;
	ptr_out_param->CAM_A_AMX_VSIZE.Bits.AMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
		((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN);
	/* add AMX_A_SIG_MODE1 & 2 */
	ptr_out_param->CAM_A_AMX_CTL.Bits.AMX_SINGLE_MODE_1 = 0;
	ptr_out_param->CAM_A_AMX_CTL.Bits.AMX_SINGLE_MODE_2 = 0;
	if ((ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_STR_X & (DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1)) ||
		(((int)ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1)))
	{
		result = DUAL_MESSAGE_BMX_SIZE_CAL_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	/* AMX_D */
	ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_STR_X = (int)ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_END_X + 1 -
		(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
	ptr_out_param->CAM_B_CTL_EN2.Bits.AMX_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.AMX_EN;
	ptr_out_param->CAM_B_AMX_CTL.Bits.AMX_EDGE = 0xF;
	ptr_out_param->CAM_B_AMX_VSIZE.Bits.AMX_HT = ptr_out_param->CAM_A_AMX_VSIZE.Bits.AMX_HT;
	/* add AMX_B_SIG_MODE1 & 2 */
	if (dual_mode <= 2)
	{
		/* update AMX_B_SIG_MODE1 & 2 */
		ptr_out_param->CAM_B_AMX_CTL.Bits.AMX_SINGLE_MODE_1 = 1;
		ptr_out_param->CAM_B_AMX_CTL.Bits.AMX_SINGLE_MODE_2 = 0;
		/* A+B, AMX_B_END_X */
		ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
	}
	else
	{
		/* update AMX_B_SIG_MODE1 & 2 */
		ptr_out_param->CAM_B_AMX_CTL.Bits.AMX_SINGLE_MODE_1 = 0;
		ptr_out_param->CAM_B_AMX_CTL.Bits.AMX_SINGLE_MODE_2 = 0;
		/* change AMX_B_END_X */
		if ((dmx_valid[0] + dmx_valid[1]) & (DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1))
		{
			ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X = dmx_valid[0] + dmx_valid[1] + (DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1) -
				((dmx_valid[0] + dmx_valid[1]) & (DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1)) - (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		}
		else
		{
			ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X = dmx_valid[0] + dmx_valid[1] - 1 - (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		}
		/* AMX_T */
		ptr_out_param->CAM_C_AMX_CROP.Bits.AMX_STR_X = (int)ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X + 1 +
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X - (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_C_AMX_CROP.Bits.AMX_END_X = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
		/* check min size */
		if (left_dmx_max_d < (int)ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + DUAL_TILE_LOSS_ALL + 1)
		{
			left_dmx_max_d = (int)ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + DUAL_TILE_LOSS_ALL + 1;
		}
		ptr_out_param->CAM_C_CTL_EN2.Bits.AMX_EN = ptr_out_param->CAM_A_CTL_EN2.Bits.AMX_EN;
		ptr_out_param->CAM_C_AMX_CTL.Bits.AMX_EDGE = 0xF;
		ptr_out_param->CAM_C_AMX_VSIZE.Bits.AMX_HT = ptr_out_param->CAM_C_BMX_VSIZE.Bits.BMX_HT;
		/* add BMX_B_SIG_MODE1 & 2 */
		ptr_out_param->CAM_C_AMX_CTL.Bits.AMX_SINGLE_MODE_1 = 1;
		ptr_out_param->CAM_C_AMX_CTL.Bits.AMX_SINGLE_MODE_2 = 0;
		if ((ptr_out_param->CAM_C_AMX_CROP.Bits.AMX_STR_X & (DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1)) ||
			(((int)ptr_out_param->CAM_C_AMX_CROP.Bits.AMX_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1)))
		{
			result = DUAL_MESSAGE_BMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	if (((int)ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X + 1 - (int)ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_STR_X) &
		(DUAL_ALIGN_PIXEL_MODE(amx_cut_a) - 1))
	{
		result = DUAL_MESSAGE_BMX_SIZE_CAL_ERROR;
		dual_driver_printf("Error: %s\n", print_error_message(result));
		return result;
	}
	/* RCROP */
	ptr_out_param->CAM_B_CTL_EN.Bits.RCP_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RCP_EN;
	ptr_out_param->CAM_B_CTL_EN.Bits.SGG1_EN = ptr_in_param->CAM_A_CTL_EN.Bits.SGG1_EN;
	ptr_out_param->CAM_B_CTL_EN2.Bits.SGG5_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.SGG5_EN;
	ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN;
	if (dual_mode > 2)
	{
		/* A+B+C */
		ptr_out_param->CAM_C_CTL_EN.Bits.RCP_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RCP_EN;
		ptr_out_param->CAM_C_CTL_EN.Bits.SGG1_EN = ptr_in_param->CAM_A_CTL_EN.Bits.SGG1_EN;
		ptr_out_param->CAM_C_CTL_EN2.Bits.SGG5_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.SGG5_EN;
		ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN;
	}
	if (ptr_in_param->CAM_A_CTL_EN.Bits.RCP_EN)
	{
		/* RCROP */
		ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_STR_X = 0;
		ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = dmx_valid[0] + right_loss_af + right_margin_af - 1;
		ptr_out_param->CAM_A_RCP_CROP_CON2.Bits.RCP_STR_Y = 0;
		ptr_out_param->CAM_A_RCP_CROP_CON2.Bits.RCP_END_Y = ((unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
			((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN)) - 1;
		/* RCROP_D */
		ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X = dmx_valid[0] - left_loss_af  - left_margin_af -
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_B_RCP_CROP_CON2.Bits.RCP_STR_Y = 0;
		ptr_out_param->CAM_B_RCP_CROP_CON2.Bits.RCP_END_Y = ptr_out_param->CAM_A_RCP_CROP_CON2.Bits.RCP_END_Y;
		if (dual_mode <= 2)
		{
			/* to A+B, for RCP_D_END_X */
			ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = dmx_valid[0] + dmx_valid[1] - 1 -
				(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_B_CTL_SEL.Bits.RCP_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RCP_SEL;
			ptr_out_param->CAM_B_CTL_SEL.Bits.SGG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.SGG_SEL;
		}
		else
		{
			/* to A+B+C, for RCP_D_END_X */
			ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = dmx_valid[0] + dmx_valid[1] + right_loss_af + right_margin_af - 1 -
				(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
			/* to revise for RCP_T */
			ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_STR_X = dmx_valid[0]  + dmx_valid[1] - left_loss_af -
				(int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_END_X =  dmx_valid[0] + dmx_valid[1] + dmx_valid[2] - 1 -
				(int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_C_RCP_CROP_CON2.Bits.RCP_STR_Y = 0;
			ptr_out_param->CAM_C_RCP_CROP_CON2.Bits.RCP_END_Y = ptr_out_param->CAM_A_RCP_CROP_CON2.Bits.RCP_END_Y;
			ptr_out_param->CAM_C_CTL_SEL.Bits.RCP_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RCP_SEL;
			ptr_out_param->CAM_C_CTL_SEL.Bits.SGG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.SGG_SEL;
		}
		/* AF & AFO */
		if (ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN)
		{
			if (((1 != ptr_in_param->CAM_A_CTL_SEL.Bits.SGG_SEL) || (1 != ptr_in_param->CAM_B_CTL_SEL.Bits.SGG_SEL) ||
				(0 == ptr_in_param->CAM_A_CTL_EN.Bits.SGG1_EN) || (0 == ptr_in_param->CAM_A_CTL_EN2.Bits.SGG5_EN)) ||
				((dual_mode > 2) && ((1 != ptr_in_param->CAM_C_CTL_SEL.Bits.SGG_SEL))))
			{
				result = DUAL_MESSAGE_INVALID_AF_SGG_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			/* AF & AF_D */
			if (((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH) ||
				((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X - (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X +
				1 < DUAL_AF_MIN_IN_WIDTH))
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			if (dual_mode > 2)
			{
				if ((int)ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_END_X - (int)ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_STR_X +
					1 < DUAL_AF_MIN_IN_WIDTH)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
			/* RTL verif or platform */
			if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
			{
				/* normal */
				if (TWIN_SCENARIO_NORMAL == ptr_in_param->SW.TWIN_SCENARIO)
				{
					/* platform */
					if ((ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE != ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE) ||
						((int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH) ||
						((dmx_valid[0] + dmx_valid[1] + ((dual_mode > 2)?dmx_valid[2]:0)) < ptr_in_param->SW.TWIN_AF_OFFSET +
						(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM) ||
						(ptr_in_param->SW.TWIN_AF_BLOCK_XNUM <= 0))
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					/* check AF_A, minimize DMX_A_END_X */
					if (dmx_valid[0] <= ptr_in_param->SW.TWIN_AF_OFFSET)
					{
						/* B+C only */
						ptr_out_param->CAM_A_CTL_EN.Bits.RCP_EN = false;
						ptr_out_param->CAM_A_CTL_EN.Bits.SGG1_EN = false;
						ptr_out_param->CAM_A_CTL_EN2.Bits.SGG5_EN = false;
						ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN = false;
						ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN = false;
					}
					else
					{
						/* A+B , cover cross region by right_margin_af */
						ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET;
						/* check AF_A output all */
						if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
							ptr_in_param->SW.TWIN_AF_OFFSET + 2 <= dmx_valid[0] + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
						{
							/* A only */
							ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
						}
						else
						{
							/* B+C, minimize DMX_A_END_X */
							ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = (dmx_valid[0] + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
								2 - ptr_in_param->SW.TWIN_AF_OFFSET)/(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
						}
						/* reduce RCP size */
						ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_STR_X +
							(int)ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART + right_loss_af +
							(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
						/* protect AF min size */
						if ((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH)
						{
							ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = DUAL_AF_MIN_IN_WIDTH - 1;
						}
					}
				}
			}
			else
			{
				/* RTL different base addr */
				ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= 0;
				if (dmx_valid[0] < (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_in_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM)
				{
					ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = dmx_valid[0]/(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
				}
				/* reduce RCP size */
				ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_STR_X +
					(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
				/* protect AF min size */
				if ((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH)
				{
					ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = DUAL_AF_MIN_IN_WIDTH - 1;
				}
			}
			/* normal */
			if (TWIN_SCENARIO_NORMAL == ptr_in_param->SW.TWIN_SCENARIO)
			{
				if (ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN)
				{
					/* check max 128 */
					if (ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
					{
						if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
						{
							result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
						else
						{
							/* RTL */
							ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
						}
					}
				}
				/* RTL verif or platform */
				if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
				{
					if (ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN)
					{
						if (dual_mode <=2)
						{
							/* A+B */
							if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM > (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM)
							{
								/* A+B */
								ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
									 (dmx_valid[0] - left_loss_af - left_margin_af);
								ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM -
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM;
							}
							else
							{
								/* A only */
								ptr_out_param->CAM_B_CTL_EN.Bits.RCP_EN = false;
								ptr_out_param->CAM_B_CTL_EN.Bits.SGG1_EN = false;
								ptr_out_param->CAM_B_CTL_EN2.Bits.SGG5_EN = false;
								ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = false;
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = false;
							}
						}
						else
						{
							/* A+B+C */
							if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM > (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM)
							{
								/* A+B+C */
								if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
									ptr_in_param->SW.TWIN_AF_OFFSET + 2 <= dmx_valid[0] + dmx_valid[1] + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
								{
									/* A+B */
									ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM -
										(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM;
									ptr_out_param->CAM_C_CTL_EN.Bits.RCP_EN = false;
									ptr_out_param->CAM_C_CTL_EN.Bits.SGG1_EN = false;
									ptr_out_param->CAM_C_CTL_EN2.Bits.SGG5_EN = false;
									ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = false;
									ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN = false;
								}
								else
								{
									/* A+B+C, minimize DMX_B_END_X */
									ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = (dmx_valid[0] + dmx_valid[1] +
										(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 2 -
										ptr_in_param->SW.TWIN_AF_OFFSET)/(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
										(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM;
									ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM -
										(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM - (int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM;
									ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
										(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
										(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
										 (dmx_valid[0] + dmx_valid[1] - left_loss_af - left_margin_af);
								}
								ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
									 (dmx_valid[0] - left_loss_af - left_margin_af);
								/* reduce RCP size */
								ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X +
									(int)ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART + right_loss_af +
									(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
								/* protect AF min size */
								if ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH +
									(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X)
								{
									ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = DUAL_AF_MIN_IN_WIDTH - 1 +
										(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X;
								}
							}
							else
							{
								/* A only */
								ptr_out_param->CAM_B_CTL_EN.Bits.RCP_EN = false;
								ptr_out_param->CAM_B_CTL_EN.Bits.SGG1_EN = false;
								ptr_out_param->CAM_B_CTL_EN2.Bits.SGG5_EN = false;
								ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = false;
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = false;
								ptr_out_param->CAM_C_CTL_EN.Bits.RCP_EN = false;
								ptr_out_param->CAM_C_CTL_EN.Bits.SGG1_EN = false;
								ptr_out_param->CAM_C_CTL_EN2.Bits.SGG5_EN = false;
								ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = false;
								ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN = false;
							}
						}
					}
					else
					{
						/* B+C only */
						if (dual_mode <= 2)
						{
							/* B only */
							ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET - (dmx_valid[0] - left_loss_af - left_margin_af);
							ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
						}
						else
						{
							/* B+C only */
							if (dmx_valid[0] + dmx_valid[1] <= ptr_in_param->SW.TWIN_AF_OFFSET)
							{
								/* C only */
								ptr_out_param->CAM_B_CTL_EN.Bits.RCP_EN = false;
								ptr_out_param->CAM_B_CTL_EN.Bits.SGG1_EN = false;
								ptr_out_param->CAM_B_CTL_EN2.Bits.SGG5_EN = false;
								ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = false;
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = false;
								ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET - (dmx_valid[0] + dmx_valid[1] - left_loss_af - left_margin_af);
								ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
							}
							else
							{
								/* B+C only */
								ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET - (dmx_valid[0] - left_loss_af - left_margin_af);
								if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
									ptr_in_param->SW.TWIN_AF_OFFSET + 2 <= dmx_valid[0] + dmx_valid[1] + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
								{
									/* B only */
									ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
									ptr_out_param->CAM_C_CTL_EN.Bits.RCP_EN = false;
									ptr_out_param->CAM_C_CTL_EN.Bits.SGG1_EN = false;
									ptr_out_param->CAM_C_CTL_EN2.Bits.SGG5_EN = false;
									ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = false;
									ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN = false;
								}
								else
								{
									/* B+C, minimize DMX_B_END_X */
									ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = (dmx_valid[0] + dmx_valid[1] +
										(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 2 - ptr_in_param->SW.TWIN_AF_OFFSET)/
										(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
									ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
										(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
										 (dmx_valid[0] + dmx_valid[1] - left_loss_af - left_margin_af);
									ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM - ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM;
								}
								/* reduce RCP size */
								ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X +
									(int)ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART + right_loss_af +
									(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
								/* protect AF min size */
								if ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH +
									(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X)
								{
									ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = DUAL_AF_MIN_IN_WIDTH - 1 +
										(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X;
								}
							}
						}
					}
					ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_YSTART = ptr_in_param->CAM_A_AF_VLD.Bits.AF_VLD_YSTART;
					ptr_out_param->CAM_B_AF_CON.Raw = ptr_in_param->CAM_A_AF_CON.Raw;
					ptr_out_param->CAM_B_AF_CON2.Raw = ptr_in_param->CAM_A_AF_CON2.Raw;
					ptr_out_param->CAM_B_AF_BLK_0.Raw = ptr_in_param->CAM_A_AF_BLK_0.Raw;
					ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_YNUM = ptr_in_param->CAM_A_AF_BLK_1.Bits.AF_BLK_YNUM;
					ptr_out_param->CAM_B_AF_BLK_2.Raw = ptr_in_param->CAM_A_AF_BLK_2.Raw;
					ptr_out_param->CAM_B_AFO_DRS.Raw = ptr_in_param->CAM_A_AFO_DRS.Raw;
					ptr_out_param->CAM_B_AFO_CON.Raw = ptr_in_param->CAM_A_AFO_CON.Raw;
					ptr_out_param->CAM_B_AFO_CON2.Raw = ptr_in_param->CAM_A_AFO_CON2.Raw;
					ptr_out_param->CAM_B_AFO_CON3.Raw = ptr_in_param->CAM_A_AFO_CON3.Raw;
					ptr_out_param->CAM_B_AFO_CON4.Raw = ptr_in_param->CAM_A_AFO_CON4.Raw;
					if (dual_mode > 2)
					{
						ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_YSTART = ptr_in_param->CAM_A_AF_VLD.Bits.AF_VLD_YSTART;
						ptr_out_param->CAM_C_AF_CON.Raw = ptr_in_param->CAM_A_AF_CON.Raw;
						ptr_out_param->CAM_C_AF_CON2.Raw = ptr_in_param->CAM_A_AF_CON2.Raw;
						ptr_out_param->CAM_C_AF_BLK_0.Raw = ptr_in_param->CAM_A_AF_BLK_0.Raw;
						ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_YNUM = ptr_in_param->CAM_A_AF_BLK_1.Bits.AF_BLK_YNUM;
						ptr_out_param->CAM_C_AF_BLK_2.Raw = ptr_in_param->CAM_A_AF_BLK_2.Raw;
						ptr_out_param->CAM_C_AFO_DRS.Raw = ptr_in_param->CAM_A_AFO_DRS.Raw;
						ptr_out_param->CAM_C_AFO_CON.Raw = ptr_in_param->CAM_A_AFO_CON.Raw;
						ptr_out_param->CAM_C_AFO_CON2.Raw = ptr_in_param->CAM_A_AFO_CON2.Raw;
						ptr_out_param->CAM_C_AFO_CON3.Raw = ptr_in_param->CAM_A_AFO_CON3.Raw;
						ptr_out_param->CAM_C_AFO_CON4.Raw = ptr_in_param->CAM_A_AFO_CON4.Raw;
					}
				}
				else
				{
					/* RTL, diff base_addr */
					ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= 0;
					if ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X - (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X +
						1 < (int)ptr_in_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_in_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM)
					{
						ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X -
							(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X + 1)/(int)ptr_in_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE;
					}
					if (dual_mode > 2)
					{
						/* reduce RCP size */
						ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X +
							(int)ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART + right_loss_af +
							(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
						/* protect AF min size */
						if ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH +
							(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X)
						{
							ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = DUAL_AF_MIN_IN_WIDTH - 1 +
								(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X;
						}
						ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= 0;
						if ((int)ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_END_X - (int)ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_STR_X +
							1 < (int)ptr_in_param->CAM_C_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_in_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM)
						{
							ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = ((int)ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_END_X -
								(int)ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_STR_X + 1)/(int)ptr_in_param->CAM_C_AF_BLK_0.Bits.AF_BLK_XSIZE;
						}
					}
				}
				if (ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN)
				{
					/* check max 128 */
					if (ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
					{
						if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
						{
							result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
						else
						{
							/* RTL */
							ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
						}
					}
				}
				if (dual_mode > 2)
				{
					if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
					{
						/* platform */
						if (ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE != ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE)
						{
							result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
					}
					if (ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN)
					{
						/* check max 128 */
						if (ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
						{
							if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR)
							{
								result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
							else
							{
								/* RTL */
								ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
							}
						}
					}
				}
				if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
				{
					if (ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE < DUAL_AF_BLOCK_BYTE)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
					{
						ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
						ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE - 1;
						/* check stride */
						if ((unsigned int)ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR + (unsigned int)ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1 >
							(unsigned int)ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE)
						{
							if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
							{
								result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
							else
							{
								/* RTL */
								ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE -
									(unsigned int)ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR)/DUAL_AF_BLOCK_BYTE;
								ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE*(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM - 1;
							}
						}
					}
					if (ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN)
					{
						ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE - 1;
						if ((int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE < DUAL_AF_BLOCK_BYTE)
						{
							result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
						/* RTL verif or platform */
						if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
						{
							ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR = ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?
								((int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE):0;
							ptr_out_param->CAM_B_AFO_YSIZE.Raw = ptr_in_param->CAM_A_AFO_YSIZE.Raw;
						}
						if ((unsigned int)ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR + (unsigned int)ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE + 1 >
							(unsigned int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
						{
							if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
							{
								result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
							else
							{
								/* RTL */
								ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE -
									(unsigned int)ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR)/DUAL_AF_BLOCK_BYTE;
								ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE*(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM  - 1;
							}
						}
						if ((ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?((unsigned int)ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1):0) +
							(unsigned int)ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE + 1 > (unsigned int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
						{
							if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
							{
								result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
						}
					}
					if (dual_mode > 2)
					{
						if (ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN)
						{
							ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE - 1;
							if ((int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE < DUAL_AF_BLOCK_BYTE)
							{
								result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
							/* RTL verif or platform */
							if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR)
							{
								ptr_out_param->CAM_C_AFO_OFST_ADDR.Bits.OFFSET_ADDR = (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?
									((int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE):0) +
									(ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN?
									((int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE):0);
								ptr_out_param->CAM_C_AFO_YSIZE.Raw = ptr_in_param->CAM_A_AFO_YSIZE.Raw;
							}
							if ((unsigned int)ptr_out_param->CAM_C_AFO_OFST_ADDR.Bits.OFFSET_ADDR + (unsigned int)ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE + 1 >
								(unsigned int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE)
							{
								if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR)
								{
									result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
									dual_driver_printf("Error: %s\n", print_error_message(result));
									return result;
								}
								else
								{
									/* RTL */
									ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE -
										(unsigned int)ptr_out_param->CAM_C_AFO_OFST_ADDR.Bits.OFFSET_ADDR)/DUAL_AF_BLOCK_BYTE;
									ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE*(int)ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM  - 1;
								}
							}
							if ((ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?((unsigned int)ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1):0) +
								(ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN?((unsigned int)ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE + 1):0) +
								(unsigned int)ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE + 1> (unsigned int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE)
							{
								if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR)
								{
									result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
									dual_driver_printf("Error: %s\n", print_error_message(result));
									return result;
								}
							}
						}
					}
					/* AFO_A padding */
					if ((false == ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN) &&
						(ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR))
					{
						if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE)
						{
							ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= 0;
							ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
							ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR = DUAL_AF_BLOCK_BYTE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
							ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE - 1;
							ptr_out_param->CAM_A_CTL_EN.Bits.RCP_EN = true;
							ptr_out_param->CAM_A_CTL_EN.Bits.SGG1_EN = true;
							ptr_out_param->CAM_A_CTL_EN2.Bits.SGG5_EN = true;
							ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN = true;
							ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN = true;
							ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_STR_X = 0;
							ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
							/* protect AF min size */
							if ((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH)
							{
								ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = DUAL_AF_MIN_IN_WIDTH - 1;
							}
						}
					}
					if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
					{
						/* error check A */
						if ((3 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_GONLY))
						{
							if (ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
							{
								result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
						}
						else if ((3 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_AVG_LVL) ||
							((2 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_AVG_LVL) &&
							(1 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_GONLY)))
						{
							if (ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
							{
								result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
						}
						else
						{
							if (ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
							{
								result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
						}
					}
					/* AFO_B padding */
					if ((false == ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN) &&
						(ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR))
					{
						if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
						{
					 		ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= 0;
							ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
							ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR = DUAL_AF_BLOCK_BYTE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
							ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE - 1;
							ptr_out_param->CAM_B_CTL_EN.Bits.RCP_EN = true;
							ptr_out_param->CAM_B_CTL_EN.Bits.SGG1_EN = true;
							ptr_out_param->CAM_B_CTL_EN2.Bits.SGG5_EN = true;
							ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = true;
							ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = true;
							ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X = 0;
							ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
							/* protect AF min size */
							if ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH)
							{
								ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = DUAL_AF_MIN_IN_WIDTH - 1;
							}
						}
					}
					if (ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN)
					{
						/* error check B */
						if ((3 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_GONLY))
						{
							if (ptr_out_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
							{
								result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
						}
						else if ((3 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_AVG_LVL) ||
							((2 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_AVG_LVL) &&
							(1 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_GONLY)))
						{
							if (ptr_out_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
							{
								result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
						}
						else
						{
							if (ptr_out_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
							{
								result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
						}
					}
					if (dual_mode > 2)
					{
						/* AFO_Cpadding */
						if ((false == ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN) &&
							(ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR))
						{
							if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE)
							{
					 			ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= 0;
								ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
								ptr_out_param->CAM_C_AFO_OFST_ADDR.Bits.OFFSET_ADDR = DUAL_AF_BLOCK_BYTE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
								ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE - 1;
								ptr_out_param->CAM_C_CTL_EN.Bits.RCP_EN = true;
								ptr_out_param->CAM_C_CTL_EN.Bits.SGG1_EN = true;
								ptr_out_param->CAM_C_CTL_EN2.Bits.SGG5_EN = true;
								ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = true;
								ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN = true;
								ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_STR_X = 0;
								ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
								/* protect AF min size */
								if ((int)ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH)
								{
									ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_END_X = DUAL_AF_MIN_IN_WIDTH - 1;
								}
							}
						}
						if (ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN)
						{
							/* error check B */
							if ((3 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_GONLY))
							{
								if (ptr_out_param->CAM_C_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
								{
									result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
									dual_driver_printf("Error: %s\n", print_error_message(result));
									return result;
								}
							}
							else if ((3 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_AVG_LVL) ||
								((2 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_AVG_LVL) &&
								(1 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_GONLY)))
							{
								if (ptr_out_param->CAM_C_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
								{
									result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
									dual_driver_printf("Error: %s\n", print_error_message(result));
									return result;
								}
							}
							else
							{
								if (ptr_out_param->CAM_C_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
								{
									result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
									dual_driver_printf("Error: %s\n", print_error_message(result));
									return result;
								}
							}
						}
					}
				}
				/* sync RCP size & AF_IMAGE_WD */
				ptr_out_param->CAM_A_AF_SIZE.Bits.AF_IMAGE_WD = (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1;
				ptr_out_param->CAM_B_AF_SIZE.Bits.AF_IMAGE_WD = (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X -
					(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X + 1;
				if (dual_mode > 2)
				{
					ptr_out_param->CAM_C_AF_SIZE.Bits.AF_IMAGE_WD = (int)ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_END_X -
						(int)ptr_out_param->CAM_C_RCP_CROP_CON1.Bits.RCP_STR_X + 1;
				}
			}
			/* minimize dmx */
			if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
			{
				if (left_dmx_max < (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 + DUAL_TILE_LOSS_ALL)
				{
					left_dmx_max = (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 + DUAL_TILE_LOSS_ALL;
				}
			}
			if (dual_mode > 2)
			{
				if (ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN)
				{
					if (left_dmx_max_d < (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1 + DUAL_TILE_LOSS_ALL)
					{
						left_dmx_max_d = (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1 + DUAL_TILE_LOSS_ALL;
					}
				}
			}
		}
		else
		{
			/* AF_EN OFF */
			/* AF & AF_D */
			if (((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < DUAL_AF_MIN_IN_WIDTH) ||
				((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X - (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X +
				1 < DUAL_AF_MIN_IN_WIDTH))
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			ptr_out_param->CAM_B_CTL_SEL.Bits.RCP_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RCP_SEL;
			ptr_out_param->CAM_B_CTL_SEL.Bits.SGG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.SGG_SEL;
			if (left_dmx_max < (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 + DUAL_TILE_LOSS_ALL)
			{
				left_dmx_max = (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 + DUAL_TILE_LOSS_ALL;
			}
			if (dual_mode > 2)
			{
				ptr_out_param->CAM_C_CTL_SEL.Bits.RCP_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RCP_SEL;
				ptr_out_param->CAM_C_CTL_SEL.Bits.SGG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.SGG_SEL;
				if (left_dmx_max_d < (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1 + DUAL_TILE_LOSS_ALL)
				{
					left_dmx_max_d = (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1 + DUAL_TILE_LOSS_ALL;
				}
			}
		}
	}
	/* RCP3 & IMGO, only 1 pixel mode */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.RCP3_EN && (ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN || ptr_in_param->CAM_A_CTL_EN.Bits.UFE_EN) &&
		ptr_in_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN && (2 == ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL))
	{
	    int bit_per_pix = 8;
	    int bus_width_shift = 1;/* 2*8, 16 bits */
	    int dmx_shift = 0;
	    ptr_out_param->CAM_B_CTL_SEL.Bits.UFE_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.UFE_SEL;
	    ptr_out_param->CAM_B_CTL_SEL.Bits.IMG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL;
	    ptr_out_param->CAM_B_CTL_SEL.Bits.RCP3_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL;
	    if (dual_mode > 2)
	    {
		    ptr_out_param->CAM_C_CTL_SEL.Bits.UFE_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.UFE_SEL;
		    ptr_out_param->CAM_C_CTL_SEL.Bits.IMG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL;
		    ptr_out_param->CAM_C_CTL_SEL.Bits.RCP3_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL;
	    }
	    /* IMGO before BIN with 2 pixel mode */
	    if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
	    {
		    dmx_shift = 1;
		    bus_width_shift = 2;/* 4*8, 32 bits */
	    }
	    if (1 == ptr_in_param->CAM_A_CTL_SEL.Bits.UFE_SEL)
	    {
	        bus_width_shift = 3; /* 8*8, 64 bits */
	        switch (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
    	    {
    		    case DUAL_OUT_FMT_BAYER_10:/* bayer 10 */
    			    bit_per_pix = 10;
    			    break;
    		    case DUAL_OUT_FMT_BAYER_12:/* bayer 12 */
    			    bit_per_pix = 12;
    			    break;
                default:
                    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		            dual_driver_printf("Error: %s\n", print_error_message(result));
		            return result;
    	    }
  	    }
  	    else
  	    {
			switch (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
   	        {
				case DUAL_OUT_FMT_BAYER_8:
					bit_per_pix = 8;
					break;
    		    case DUAL_OUT_FMT_BAYER_10:/* bayer 10 */
    			    bit_per_pix = 10;
    			    break;
    		    case DUAL_OUT_FMT_MIPI_10:
    			    bus_width_shift = 3;/* 8*8, 64 bits */
    			    bit_per_pix = 10;
    			    break;
    		    case DUAL_OUT_FMT_BAYER_12:/* bayer 12 */
    			    bit_per_pix = 12;
    			    break;
    		    case DUAL_OUT_FMT_BAYER_14:/* bayer 14 */
    			    bit_per_pix = 14;
    			    break;
    		    case DUAL_OUT_FMT_BAYER_8_2:
    		    case DUAL_OUT_FMT_BAYER_10_2:
    		    case DUAL_OUT_FMT_BAYER_12_2:
    		    case DUAL_OUT_FMT_BAYER_14_2:
    			    bit_per_pix = 16;
    			    break;
    			default:
    				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		            dual_driver_printf("Error: %s\n", print_error_message(result));
		            return result;
   	        }
   	    }
	    /* RCROP3 */
	    ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X = 0;
	    if ((dmx_valid[0] << dmx_shift) & (DUAL_ALIGN_PIXEL_MODE(rcp_cut_a) - 1))
	    {
		    ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X = (dmx_valid[0] << dmx_shift) - 1 + DUAL_ALIGN_PIXEL_MODE(rcp_cut_a) -
			    ((dmx_valid[0] << dmx_shift) & (DUAL_ALIGN_PIXEL_MODE(rcp_cut_a) - 1));
	    }
	    else
	    {
		    ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X = (dmx_valid[0] << dmx_shift) - 1;
	    }
	    ptr_out_param->CAM_A_RCP3_CROP_CON2.Bits.RCP_STR_Y = 0;
	    ptr_out_param->CAM_A_RCP3_CROP_CON2.Bits.RCP_END_Y = ((int)ptr_out_param->CAM_A_BMX_VSIZE.Bits.BMX_HT << dmx_shift) - 1;
	    /* IMGO */
	    ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE = ((unsigned int)(((int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X -
		    (int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X + 1)*bit_per_pix + (1<<(bus_width_shift + 3)) - 1)>>(bus_width_shift + 3))*
		    (1<<bus_width_shift) - 1;
	    if (((int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X -
		    (int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) & (DUAL_ALIGN_PIXEL_MODE(rcp_cut_a) - 1))
	    {
		    result = DUAL_MESSAGE_RCP3_SIZE_CAL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }
	    /* RCROP3_D */
	    ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X = (int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X + 1 -
		    ((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X << dmx_shift);
	    ptr_out_param->CAM_B_RCP3_CROP_CON2.Bits.RCP_STR_Y = 0;
	    ptr_out_param->CAM_B_RCP3_CROP_CON2.Bits.RCP_END_Y = ptr_out_param->CAM_A_RCP3_CROP_CON2.Bits.RCP_END_Y;
	    if (dual_mode <= 2)
	    {
		    /* A+B */
		    ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X = ((((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1) << dmx_shift) - 1) -
			    ((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X << dmx_shift);
	    }
	    else
	    {
		    /* A+B+C */
		    if (((dmx_valid[0] + dmx_valid[1]) << dmx_shift) & (DUAL_ALIGN_PIXEL_MODE(rcp_cut_a) - 1))
		    {
			    ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X = ((dmx_valid[0] + dmx_valid[1]) << dmx_shift) +
				    (DUAL_ALIGN_PIXEL_MODE(rcp_cut_a) - 1) -
				    (((dmx_valid[0] + dmx_valid[1]) << dmx_shift) & (DUAL_ALIGN_PIXEL_MODE(rcp_cut_a) - 1)) -
				    ((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X << dmx_shift);
		    }
		    else
		    {
			    ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X = ((dmx_valid[0] + dmx_valid[1]) << dmx_shift) - 1 -
			  	    ((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X << dmx_shift);
		    }
		    /* RCROP3_T */
		    ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_STR_X = (int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X +
			    1 + ((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X << dmx_shift) -
			    ((int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X << dmx_shift);
		    ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_END_X = ((((int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X + 1) << dmx_shift) - 1) -
			    ((int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X << dmx_shift);
		    ptr_out_param->CAM_C_RCP3_CROP_CON2.Bits.RCP_STR_Y = 0;
		    ptr_out_param->CAM_C_RCP3_CROP_CON2.Bits.RCP_END_Y = ptr_out_param->CAM_A_RCP3_CROP_CON2.Bits.RCP_END_Y;
		    ptr_out_param->CAM_C_IMGO_XSIZE.Bits.XSIZE = ((unsigned int)(((int)ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_END_X -
			    (int)ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_STR_X + 1)*bit_per_pix)>>3) -1;
	    }
	    /* IMGO_D */
	    ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = ((unsigned int)(((int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X -
		    (int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X + 1)*bit_per_pix + (1<<(bus_width_shift + 3)) - 1)>>(bus_width_shift + 3))*
		    (1<<bus_width_shift) - 1;
	    /* clone reg */
	    ptr_out_param->CAM_B_CTL_EN.Bits.RCP3_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RCP3_EN;
	    ptr_out_param->CAM_B_CTL_EN.Bits.CPG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.CPG_EN;
	    ptr_out_param->CAM_B_CTL_EN.Bits.PAK_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN;
	    ptr_out_param->CAM_B_IMGO_CON.Raw = ptr_in_param->CAM_A_IMGO_CON.Raw;
	    ptr_out_param->CAM_B_IMGO_CON2.Raw = ptr_in_param->CAM_A_IMGO_CON2.Raw;
	    ptr_out_param->CAM_B_IMGO_CON3.Raw = ptr_in_param->CAM_A_IMGO_CON3.Raw;
	    ptr_out_param->CAM_B_IMGO_CON4.Raw = ptr_in_param->CAM_A_IMGO_CON4.Raw;
	    ptr_out_param->CAM_B_IMGO_STRIDE.Raw = ptr_in_param->CAM_A_IMGO_STRIDE.Raw;
	    ptr_out_param->CAM_B_IMGO_CROP.Bits.YOFFSET = ptr_in_param->CAM_A_IMGO_CROP.Bits.YOFFSET;
	    ptr_out_param->CAM_B_IMGO_YSIZE.Bits.YSIZE = ptr_in_param->CAM_A_IMGO_YSIZE.Bits.YSIZE;
	    /* force bus size disabled */
	    ptr_out_param->CAM_A_IMGO_STRIDE.Bits.BUS_SIZE_EN = 0;
	    ptr_out_param->CAM_B_IMGO_STRIDE.Bits.BUS_SIZE_EN = 0;
	    /* off_addr */
	    ptr_out_param->CAM_A_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
	    if (dual_mode > 2)
	    {
		    ptr_out_param->CAM_C_CTL_EN.Bits.RCP3_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RCP3_EN;
		    ptr_out_param->CAM_C_CTL_EN.Bits.CPG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.CPG_EN;
		    ptr_out_param->CAM_C_CTL_EN.Bits.PAK_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN;
		    ptr_out_param->CAM_C_IMGO_CON.Raw = ptr_in_param->CAM_A_IMGO_CON.Raw;
		    ptr_out_param->CAM_C_IMGO_CON2.Raw = ptr_in_param->CAM_A_IMGO_CON2.Raw;
		    ptr_out_param->CAM_C_IMGO_CON3.Raw = ptr_in_param->CAM_A_IMGO_CON3.Raw;
		    ptr_out_param->CAM_C_IMGO_CON4.Raw = ptr_in_param->CAM_A_IMGO_CON4.Raw;
		    ptr_out_param->CAM_C_IMGO_STRIDE.Raw = ptr_in_param->CAM_A_IMGO_STRIDE.Raw;
		    ptr_out_param->CAM_C_IMGO_CROP.Bits.YOFFSET = ptr_in_param->CAM_A_IMGO_CROP.Bits.YOFFSET;
		    ptr_out_param->CAM_C_IMGO_YSIZE.Bits.YSIZE = ptr_in_param->CAM_A_IMGO_YSIZE.Bits.YSIZE;
		    /* force bus size disabled */
		    ptr_out_param->CAM_C_IMGO_STRIDE.Bits.BUS_SIZE_EN = 0;
	    }
	    /* RTL */
	    if (ptr_in_param->CAM_A_IMGO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_IMGO_BASE_ADDR.Bits.BASE_ADDR)
	    {
		    if (dual_mode <= 2)
		    {
			    /* A+B */
			    if ((int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift) >= (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1)
			    {
				    /* B only */
				    ptr_out_param->CAM_A_CTL_EN.Bits.RCP3_EN = 0;
				    ptr_out_param->CAM_A_CTL_EN.Bits.CPG_EN = 0;
				    ptr_out_param->CAM_A_CTL_EN.Bits.PAK_EN = 0;
				    ptr_out_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN = 0;
				    /* B only */
				    ptr_out_param->CAM_B_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
				    ptr_out_param->CAM_B_IMGO_CROP.Bits.XOFFSET = (int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET -
					    (((unsigned int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1)>>bus_width_shift);
				    ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE;
			    }
			    else
			    {
				    /* A+B */
				    if ((int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift) + (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE <=
					    (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE)
				    {
					    /* A only */
					    ptr_out_param->CAM_B_CTL_EN.Bits.RCP3_EN = 0;
					    ptr_out_param->CAM_B_CTL_EN.Bits.CPG_EN = 0;
					    ptr_out_param->CAM_B_CTL_EN.Bits.PAK_EN = 0;
					    ptr_out_param->CAM_B_CTL_DMA_EN.Bits.IMGO_EN = 0;
					    /* A only */
					    ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE;
				    }
				    else
				    {
					    /* A+B */
					    ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE -= (int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift);
					    ptr_out_param->CAM_B_IMGO_CROP.Bits.XOFFSET = 0;
					    ptr_out_param->CAM_B_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1;
					    ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE -
						    ((int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1);
				    }
			    }
		    }
		    else
		    {
			    /* A+B+C */
			    if ((int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift) >= (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1)
			    {
				    /* B+C */
				    ptr_out_param->CAM_A_CTL_EN.Bits.RCP3_EN = 0;
				    ptr_out_param->CAM_A_CTL_EN.Bits.CPG_EN = 0;
				    ptr_out_param->CAM_A_CTL_EN.Bits.PAK_EN = 0;
				    ptr_out_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN = 0;
				    /* B+C */
				    if ((int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift) >=
					    (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1 + (int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE + 1)
				    {
					     /* C only */
					    ptr_out_param->CAM_B_CTL_EN.Bits.RCP3_EN = 0;
					    ptr_out_param->CAM_B_CTL_EN.Bits.CPG_EN = 0;
					    ptr_out_param->CAM_B_CTL_EN.Bits.PAK_EN = 0;
					    ptr_out_param->CAM_B_CTL_DMA_EN.Bits.IMGO_EN = 0;
					    /* C only */
					    ptr_out_param->CAM_C_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
					    ptr_out_param->CAM_C_IMGO_CROP.Bits.XOFFSET = (int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET -
						    (((unsigned int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1 +
						    (unsigned int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE + 1)>>bus_width_shift);
					    ptr_out_param->CAM_C_IMGO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE;
				    }
			  	    else
				    {
					    if ((int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift) + (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE >
						    (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1 + (int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE)
					    {
						    /* B+C */
						    ptr_out_param->CAM_B_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
						    ptr_out_param->CAM_B_IMGO_CROP.Bits.XOFFSET = (int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET -
							    (((unsigned int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1)>>bus_width_shift);
						    ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE -
 							    (int)ptr_out_param->CAM_B_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift);
						    ptr_out_param->CAM_C_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = (int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE + 1;
						    ptr_out_param->CAM_C_IMGO_CROP.Bits.XOFFSET = 0;
						    ptr_out_param->CAM_C_IMGO_XSIZE.Bits.XSIZE = (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE -
							    ((int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE + 1);
					    }
					    else
					    {
						    /* B */
						    ptr_out_param->CAM_B_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
						    ptr_out_param->CAM_B_IMGO_CROP.Bits.XOFFSET = (int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET -
							    (((unsigned int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1)>>bus_width_shift);
						    ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE;
						    ptr_out_param->CAM_C_CTL_EN.Bits.RCP3_EN = 0;
						    ptr_out_param->CAM_C_CTL_EN.Bits.CPG_EN = 0;
						    ptr_out_param->CAM_C_CTL_EN.Bits.PAK_EN = 0;
						    ptr_out_param->CAM_C_CTL_DMA_EN.Bits.IMGO_EN = 0;
					    }
				    }
			    }
			    else
			    {
				    /* A+B+C */
				    if ((int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift) + (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE <=
					    (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE)
				    {
					    /* A only */
					    ptr_out_param->CAM_B_CTL_EN.Bits.RCP3_EN = 0;
					    ptr_out_param->CAM_B_CTL_EN.Bits.CPG_EN = 0;
					    ptr_out_param->CAM_B_CTL_EN.Bits.PAK_EN = 0;
					    ptr_out_param->CAM_B_CTL_DMA_EN.Bits.IMGO_EN = 0;
					    ptr_out_param->CAM_C_CTL_EN.Bits.RCP3_EN = 0;
					    ptr_out_param->CAM_C_CTL_EN.Bits.CPG_EN = 0;
					    ptr_out_param->CAM_C_CTL_EN.Bits.PAK_EN = 0;
					    ptr_out_param->CAM_C_CTL_DMA_EN.Bits.IMGO_EN = 0;
					    /* A only */
					    ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE;
				    }
				    else
				    {
					    /* A+B+C */
					    if ((int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift) + (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE <=
						    (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1 + (int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE)
					    {
						    /* A+B */
						    ptr_out_param->CAM_C_CTL_EN.Bits.RCP3_EN = 0;
						    ptr_out_param->CAM_C_CTL_EN.Bits.CPG_EN = 0;
						    ptr_out_param->CAM_C_CTL_EN.Bits.PAK_EN = 0;
						    ptr_out_param->CAM_C_CTL_DMA_EN.Bits.IMGO_EN = 0;
						    /* A+B */
						    ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE -= (int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift);
						    ptr_out_param->CAM_B_IMGO_CROP.Bits.XOFFSET = 0;
						    ptr_out_param->CAM_B_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1;
						    ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE -
							    ((int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1);
					    }
					    else
					    {
						    /* A+B+C */
						    ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE -= (int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift);
						    ptr_out_param->CAM_B_IMGO_CROP.Bits.XOFFSET = 0;
						    ptr_out_param->CAM_B_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1;
						    ptr_out_param->CAM_C_IMGO_CROP.Bits.XOFFSET = 0;
						    ptr_out_param->CAM_C_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1 +
							    (int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE + 1;
						    ptr_out_param->CAM_C_IMGO_XSIZE.Bits.XSIZE = (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE -
							    ((int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1) - ((int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE + 1);
					    }
				    }
			    }
		    }
	    }
	    else
	    {
		    ptr_out_param->CAM_B_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
		    /* restore stride */
		    ptr_out_param->CAM_B_IMGO_STRIDE.Bits.STRIDE = ptr_in_param->CAM_B_IMGO_STRIDE.Bits.STRIDE;
		    /* check valid size only WDMA with XOFF and YOFF support */
		    if ((int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1 > (int)ptr_in_param->CAM_A_IMGO_STRIDE.Bits.STRIDE)
		    {
			    ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_in_param->CAM_A_IMGO_STRIDE.Bits.STRIDE>>bus_width_shift)<<bus_width_shift) - 1;
		    }
		    if ((int)ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE + 1 > (int)ptr_in_param->CAM_B_IMGO_STRIDE.Bits.STRIDE)
		    {
			    ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_in_param->CAM_B_IMGO_STRIDE.Bits.STRIDE>>bus_width_shift)<<bus_width_shift) - 1;
		    }
		    if (dual_mode > 2)
		    {
			    ptr_out_param->CAM_C_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
			    /* restore stride */
			    ptr_out_param->CAM_C_IMGO_STRIDE.Bits.STRIDE = ptr_in_param->CAM_C_IMGO_STRIDE.Bits.STRIDE;
			    if ((int)ptr_out_param->CAM_C_IMGO_XSIZE.Bits.XSIZE + 1 > (int)ptr_in_param->CAM_C_IMGO_STRIDE.Bits.STRIDE)
			    {
				    ptr_out_param->CAM_C_IMGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_in_param->CAM_C_IMGO_STRIDE.Bits.STRIDE>>bus_width_shift)<<bus_width_shift) - 1;
			    }
		    }
	    }
	    if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN)
	    {
		    /* check min size */
		    if (2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL)
		    {
			    if (((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + 1)*left_dmx_max < (int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X + 1)
			    {
				    left_dmx_max = ((unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X + 1)>>(int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN;
			    }
		    }
		    else
		    {
			    if (left_dmx_max < (int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X + 1 + DUAL_TILE_LOSS_ALL)
			    {
				    left_dmx_max = (int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X + 1 + DUAL_TILE_LOSS_ALL;
			    }
		    }
		    if (dual_mode > 2)
		    {
			    if (ptr_out_param->CAM_B_CTL_DMA_EN.Bits.IMGO_EN)
			    {
				    if (2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL)
				    {
					    if (((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + 1)*left_dmx_max_d < (int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X +
						     (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1)
					    {
						    left_dmx_max_d = ((unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X +
							    (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1)>>(int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN;
					    }
				    }
				    else
				    {
					    if (left_dmx_max_d < (int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X +
						    1 + DUAL_TILE_LOSS_ALL)
					    {
						    left_dmx_max_d = (int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X + (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X +
							    1 + DUAL_TILE_LOSS_ALL;
				  	    }
				    }
			    }
		    }
	    }
	    /* pixel mode */
	    if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
	    {
		    ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X = (unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X>>1;
		    ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X = (((unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X + 1)>>1) - 1;
		    ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X = (unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X>>1;
		    ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X = (((unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X + 1)>>1) - 1;
		    if (dual_mode > 2)
		    {
			    ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_STR_X = (unsigned int)ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_STR_X>>1;
			    ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_END_X = (((unsigned int)ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_END_X + 1)>>1) - 1;
		    }
	    }
        if (1 == ptr_in_param->CAM_A_CTL_SEL.Bits.UFE_SEL)
		{
			ptr_out_param->CAM_A_UFEO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
			ptr_out_param->CAM_B_UFEO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
			ptr_out_param->CAM_B_CTL_EN.Bits.UFE_EN = ptr_in_param->CAM_A_CTL_EN.Bits.UFE_EN;
			if (dual_mode > 2)
			{
			    ptr_out_param->CAM_C_UFEO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
			    ptr_out_param->CAM_C_CTL_EN.Bits.UFE_EN = ptr_in_param->CAM_A_CTL_EN.Bits.UFE_EN;
			}
			if (ptr_in_param->CAM_A_UFEO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_UFEO_BASE_ADDR.Bits.BASE_ADDR)
			{
				if (ptr_in_param->CAM_A_CTL_EN.Bits.UFE_EN && ptr_in_param->CAM_A_CTL_DMA_EN.Bits.UFEO_EN)
				{
					/* UFEO, test mode, size not x8 */
					ptr_out_param->CAM_B_UFEO_CON.Raw = ptr_in_param->CAM_A_UFEO_CON.Raw;
					ptr_out_param->CAM_B_UFEO_CON2.Raw = ptr_in_param->CAM_A_UFEO_CON2.Raw;
					ptr_out_param->CAM_B_UFEO_CON3.Raw = ptr_in_param->CAM_A_UFEO_CON3.Raw;
					ptr_out_param->CAM_B_UFEO_CON4.Raw = ptr_in_param->CAM_A_UFEO_CON4.Raw;
					ptr_out_param->CAM_B_UFEO_STRIDE.Raw = ptr_in_param->CAM_A_UFEO_STRIDE.Raw;
					ptr_out_param->CAM_B_UFEO_YSIZE.Bits.YSIZE = ptr_in_param->CAM_A_UFEO_YSIZE.Bits.YSIZE;
					ptr_out_param->CAM_B_UFE_CON.Raw = ptr_in_param->CAM_A_UFE_CON.Raw;
					if (dual_mode > 2)
					{
						ptr_out_param->CAM_C_UFEO_CON.Raw = ptr_in_param->CAM_A_UFEO_CON.Raw;
						ptr_out_param->CAM_C_UFEO_CON2.Raw = ptr_in_param->CAM_A_UFEO_CON2.Raw;
						ptr_out_param->CAM_C_UFEO_CON3.Raw = ptr_in_param->CAM_A_UFEO_CON3.Raw;
						ptr_out_param->CAM_C_UFEO_CON4.Raw = ptr_in_param->CAM_A_UFEO_CON4.Raw;
						ptr_out_param->CAM_C_UFEO_STRIDE.Raw = ptr_in_param->CAM_A_UFEO_STRIDE.Raw;
						ptr_out_param->CAM_C_UFEO_YSIZE.Bits.YSIZE = ptr_in_param->CAM_A_UFEO_YSIZE.Bits.YSIZE;
						ptr_out_param->CAM_C_UFE_CON.Raw = ptr_in_param->CAM_A_UFE_CON.Raw;
					}
					if (dual_mode <= 2)
					{
						if (0 == ptr_out_param->CAM_B_CTL_DMA_EN.Bits.IMGO_EN)
						{
							/* UFEO_A only */
							ptr_out_param->CAM_B_CTL_DMA_EN.Bits.UFEO_EN = 0;
							ptr_out_param->CAM_B_CTL_EN.Bits.UFE_EN = 0;
							ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X -
							    (unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
						}
						else if (0 == ptr_out_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN)
						{
							/* UFEO_B only */
							ptr_out_param->CAM_A_CTL_DMA_EN.Bits.UFEO_EN = 0;
							ptr_out_param->CAM_A_CTL_EN.Bits.UFE_EN = 0;
						    ptr_out_param->CAM_B_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X -
						        (unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
							/* restore CAM_A_UFEO_XSIZE for driver config sync */
							ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_UFEO_XSIZE.Bits.XSIZE;
						}
						else
						{
							/* UFEO, test mode, size not x8 */
							ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X -
							    (unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
							ptr_out_param->CAM_B_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X -
							    (unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
							ptr_out_param->CAM_B_UFEO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE + 1;
						}
					}
					else
					{
						/* check master disable */
						if (0 == ptr_out_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN)
						{
							/* master disable, UFEO_A off */
							ptr_out_param->CAM_A_CTL_DMA_EN.Bits.UFEO_EN = 0;
							ptr_out_param->CAM_A_CTL_EN.Bits.UFE_EN = 0;
							/* restore CAM_A_UFEO_XSIZE for driver config sync */
							ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_UFEO_XSIZE.Bits.XSIZE;
						}
						else
						{
							ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X -
							    (unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
						}
						if (0 == ptr_out_param->CAM_B_CTL_DMA_EN.Bits.IMGO_EN)
						{
							/* master disable, UFEO_B off */
							ptr_out_param->CAM_B_CTL_DMA_EN.Bits.UFEO_EN = 0;
							ptr_out_param->CAM_B_CTL_EN.Bits.UFE_EN = 0;
						}
						else
						{
							ptr_out_param->CAM_B_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X -
							    (unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
							ptr_out_param->CAM_B_UFEO_OFST_ADDR.Bits.OFFSET_ADDR = ptr_out_param->CAM_A_CTL_DMA_EN.Bits.UFEO_EN? ((unsigned int)ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE + 1): 0;
						}
						if (0 == ptr_out_param->CAM_C_CTL_DMA_EN.Bits.IMGO_EN)
						{
							/* master disable, ufeO_C off */
							ptr_out_param->CAM_C_CTL_DMA_EN.Bits.UFEO_EN = 0;
							ptr_out_param->CAM_C_CTL_EN.Bits.UFE_EN = 0;
						}
						else
						{
							ptr_out_param->CAM_C_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_END_X -
							    (unsigned int)ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
							ptr_out_param->CAM_C_UFEO_OFST_ADDR.Bits.OFFSET_ADDR = ptr_out_param->CAM_B_CTL_DMA_EN.Bits.UFEO_EN? ((unsigned int)ptr_out_param->CAM_B_UFEO_OFST_ADDR.Bits.OFFSET_ADDR + (unsigned int)ptr_out_param->CAM_B_UFEO_XSIZE.Bits.XSIZE + 1): 0;
						}
					}
				}
				else
				{
					if (ptr_in_param->CAM_A_CTL_EN.Bits.UFE_EN || ptr_in_param->CAM_A_CTL_DMA_EN.Bits.UFEO_EN)
					{
						result = DUAL_MESSAGE_UFEO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
			}
			else
			{
				/* UFEO */
				ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X -
				    (unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
				ptr_out_param->CAM_B_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X -
				    (unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
				if (dual_mode > 2)
				{
					ptr_out_param->CAM_C_UFEO_XSIZE.Bits.XSIZE = ((((unsigned int)ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_END_X -
					    (unsigned int)ptr_out_param->CAM_C_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) + 63)>>6) - 1;
				}
			}
		}
	}
	/* RRZ & RMX */
	ptr_out_param->CAM_B_CTL_EN.Bits.RRZ_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RRZ_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.RRZ_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RRZ_EN;
	}
	if (ptr_in_param->CAM_A_CTL_EN.Bits.RRZ_EN)
	{
		/*update size by bin enable */
		int rrz_in_w = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_E -
			(unsigned int)ptr_in_param-> CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN)):
			((unsigned int)ptr_in_param->SW.TWIN_RAWI_XSIZE>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN));
		int out_xe=0;
		int out_shift[3] = {0, 0, 0};/* 2<<out_shift[i], [0]: RRZ_A align, [1]: RRZ_B align, default align x2 */
		ptr_out_param->CAM_B_CTL_SEL.Bits.UFEG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL;
		ptr_out_param->CAM_B_CTL_SEL.Bits.HDS1_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.HDS1_SEL;
		if (dual_mode > 2)
		{
			ptr_out_param->CAM_C_CTL_SEL.Bits.UFEG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL;
			ptr_out_param->CAM_C_CTL_SEL.Bits.HDS1_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.HDS1_SEL;
		}
		if (0 == ptr_in_param->CAM_UNI_TOP_MOD_EN.Bits.RLB_A_EN)
		{
		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		/* check rrz_a alignment */
		if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
		{
			if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
			{
				out_shift[0] = 5;/* 2<<5, x64 */
			}
			else if (DUAL_OUT_FMT_FG_10 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
			{
				out_shift[0] = 6;/* 2<<6, x128 */
			}
			else
			{
				result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		else
		{
			if (DUAL_OUT_FMT_FG_8 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
			{
				out_shift[0] = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?3:0;/* 2<<3, x16 */
			}
			else if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
			{
				out_shift[0] = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?4:1;/* 2<<4, x32 */
			}
			else if (DUAL_OUT_FMT_FG_10 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
			{
				/* new 32b */
				out_shift[0] = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?4:2;/* 2<<4, x32 */
			}
			else
			{
				result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		/* check RMX size */
		if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_RMXO < 2)
		{
			if (ptr_in_param->SW.TWIN_RRZ_OUT_WD & 0x1)
			{
				result = DUAL_MESSAGE_RMX_OUT_ALIGN_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		else
		{
			if (ptr_in_param->SW.TWIN_RRZ_OUT_WD & 0x3)
			{
				result = DUAL_MESSAGE_RMX_OUT_ALIGN_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		/* check rrz_a/b alignment, minmum x8 for RLB monitor */
		if (out_shift[0] < DUAL_RLB_ALIGN_SHIFT)
		{
			out_shift[0] = DUAL_RLB_ALIGN_SHIFT;/* 2<<2, x8 */
		}
		if (dual_mode <= 2)
		{
			out_shift[1] = DUAL_RLB_ALIGN_SHIFT;/* x8 */
		}
		else
		{
			out_shift[1] = out_shift[0];/* x8 */
			out_shift[2] = DUAL_RLB_ALIGN_SHIFT;/* x8 */
		}
		/* copy from RRZ_IN_HT & RRZ_D_IN_HT from DMX & DMX_D */
		ptr_out_param->CAM_A_RRZ_IN_IMG.Bits.RRZ_IN_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
		((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN);
		if ((rrz_in_w < dual_mode*DUAL_RLB_ALIGN_PIX) || (ptr_in_param->SW.TWIN_RRZ_OUT_WD < dual_mode*DUAL_RLB_ALIGN_PIX) ||
			(ptr_in_param->SW.TWIN_RRZ_OUT_WD > DUAL_RRZ_WD_ALL))
		{
		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		/* coeff step */
		ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = (long long)(rrz_in_w - 1 -
			ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST)*
			DUAL_RRZ_PREC/(ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1);
		/* scaling down error check */
		if (ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP < DUAL_RRZ_PREC)
		{
			result = DUAL_MESSAGE_RRZ_SCALING_UP_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* check vertical step */
		if (0 == ptr_in_param->CAM_A_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP)
		{
			if (ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT <= 1)
		    {
			    result = DUAL_MESSAGE_INVALID_RRZ_IN_CROP_HT_ERROR;
			    dual_driver_printf("Error: %s\n", print_error_message(result));
			    return result;
		    }
		    else
		    {
			    /* re-cal RRZ_VERT_STEP */
			    ptr_out_param->CAM_A_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = (long long)(ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT - 1)*
				    DUAL_RRZ_PREC/((int)ptr_in_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT - 1);
			    if (ptr_out_param->CAM_A_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP < DUAL_RRZ_PREC)
			    {
				    result = DUAL_MESSAGE_RRZ_SCALING_UP_ERROR;
				    dual_driver_printf("Error: %s\n", print_error_message(result));
				    return result;
			    }
			    ptr_out_param->CAM_B_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = ptr_out_param->CAM_A_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP;
				if (dual_mode > 2)
				{
				    ptr_out_param->CAM_C_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = ptr_out_param->CAM_A_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP;
				}
		    }
	    }
	    /* copy vertical step */
	    ptr_out_param->CAM_B_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
	    ptr_out_param->CAM_B_RRZ_IN_IMG.Bits.RRZ_IN_HT = ptr_out_param->CAM_A_RRZ_IN_IMG.Bits.RRZ_IN_HT;
	    /* configure single_mode */
	    ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 0;
	    ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
		ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_EDGE = 0xF;
		ptr_out_param->CAM_A_RMX_VSIZE.Bits.RMX_HT = ptr_in_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
		ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_EDGE = 0xF;
		ptr_out_param->CAM_B_RMX_VSIZE.Bits.RMX_HT = ptr_in_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
		if (dual_mode <= 2)
		{
			ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;/* RRZ_B, diable from RRZ_C */
			ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
		}
		else
		{
			/* copy vertical step */
			ptr_out_param->CAM_C_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
			ptr_out_param->CAM_C_RRZ_IN_IMG.Bits.RRZ_IN_HT = ptr_out_param->CAM_A_RRZ_IN_IMG.Bits.RRZ_IN_HT;
			/* configure single_mode */
			ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 0;
			ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
			ptr_out_param->CAM_C_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;/* RRZ_C, diable from next */
			ptr_out_param->CAM_C_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
			ptr_out_param->CAM_C_RMX_CTL.Bits.RMX_EDGE = 0xF;
			ptr_out_param->CAM_C_RMX_VSIZE.Bits.RMX_HT = ptr_in_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
		}
		if (dual_mode <= 2)
		{
			/* loop dual mode from right */
			for (int i=0;i<2;i++)
			{
				int in_xs_dmx=0;
				int in_xs=0;
				int in_xe=0;
				int out_xs= 0;
				/* config forward backward pos */
				switch (i)
				{
					case 0:
						/* right */
						ptr_out_param->CAM_B_RRZ_IN_IMG.Bits.RRZ_IN_WD = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
							(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1;
						/* init pos */
						in_xs_dmx = ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
						/* right boundary */
						in_xs = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + DUAL_TILE_LOSS_ALL_L;
						in_xe = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X;
						out_xe= ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1;
						break;
					case 1:
						/* left */
						ptr_out_param->CAM_A_RRZ_IN_IMG.Bits.RRZ_IN_WD = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
							(int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1;
						/* init pos */
						in_xs_dmx = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X;
						out_xs = 0;
						in_xe = dmx_valid[0] + right_loss_rrz - 1;
						break;
					default:
						break;
				}
				/* backward & forward cal */
				if (out_xs < out_xe)
				{
					/* backward out_xe */
					long long end_temp = (long long)out_xe*ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP +
						(long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
						ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
					/* cal pos */
					if (end_temp + (long long)DUAL_TILE_LOSS_RRZ_TAP*DUAL_RRZ_PREC < (long long)in_xe*DUAL_RRZ_PREC)
					{
						int n;
						n = (int)((unsigned long long)(end_temp + DUAL_TILE_LOSS_RRZ_TAP*DUAL_RRZ_PREC)>>15);
						if (n & 0x1)
						{
							in_xe = n ;
						}
						else/* must be even */
						{
							in_xe = n + 1;
						}
					}
					else
					{
						if (in_xe + 1 < ptr_in_param->SW.TWIN_RRZ_OUT_WD)
						{
							result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
					}
					/* forward in_xs */
					if (in_xs < in_xe)
					{
						if (in_xs <= 0)
						{
							out_xs = 0;
						}
						else
						{
							int n;
							long long start_temp = (long long)(in_xs + DUAL_TILE_LOSS_RRZ_TAP_L)*DUAL_RRZ_PREC -
								(long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC -
								ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
							n = (int)(start_temp/ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP);
							if (((long long)n*ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP) < start_temp)
							{
								n = n + 1;
							}
							if (n < 0)
							{
								n = 0;
							}
							/* update align by pixel mode */
							/* RRZ_A, run i= 0 only */
							if (n & DUAL_RRZ_MASK(out_shift[i]))
							{
								out_xs = n + DUAL_RRZ_ALIGN(out_shift[i]) - (n & DUAL_RRZ_MASK(out_shift[i]));
							}
							else
							{
								out_xs = n;
							}
						}
					}
				}
				/* update RRZ width, offset, flag */
				if ((out_xs < out_xe) && (in_xs < in_xe))
				{
					long long temp_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
						ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST +
						(long long)out_xs*ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP -
						(long long)in_xs_dmx*DUAL_RRZ_PREC;
					switch (i)
					{
						case 0:/* to revise for RRZ_D & RMX_D */
							/* RRZ_D */
							ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = out_xe - out_xs + 1;
							ptr_out_param->CAM_B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)temp_offset>>15);
							ptr_out_param->CAM_B_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(temp_offset -
								(long long)ptr_out_param->CAM_B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X = out_xe - out_xs;
							/* check RRZ_D buffer size, must x8 align */
							if (ptr_in_param->SW.DUAL_SEL & 0x1)
							{
								/* A+B, A+C, align from start, C+A, B+A */
								ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
								ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD +
									DUAL_RLB_ALIGN_PIX - 1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1);
							}
							else
							{
								/* B+C, align from end */
								ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = DUAL_RRZ_WD_ALL -
									((((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
									1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1));
							}
							break;
						case 1:
							/* RRZ */
							ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = out_xe - out_xs + 1;
							ptr_out_param->CAM_A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)temp_offset>>15);
							ptr_out_param->CAM_A_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(temp_offset -
								(long long)ptr_out_param->CAM_A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X = out_xe - out_xs;
							/* check min size */
							if (left_dmx_max < in_xe + 1 + DUAL_TILE_LOSS_ALL)
							{
								left_dmx_max = in_xe + 1 + DUAL_TILE_LOSS_ALL;
							}
							/* check RRZ buffer size, must x8 align */
							if (ptr_in_param->SW.DUAL_SEL & 0x1)
							{
								/* A+B, A+C, align from start, C+A, B+A */
								if (((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
									1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1)) +
									(unsigned int)ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST > DUAL_RRZ_WD_ALL)
								{
									result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
									dual_driver_printf("Error: %s\n", print_error_message(result));
									return result;
								}
							}
							else
							{
								/* B+C, align from end */
								if (((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
									1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1)) >
									(unsigned int)ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST)
								{
									result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
									dual_driver_printf("Error: %s\n", print_error_message(result));
									return result;
								}
								else
								{
									/* align from end */
									ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST -
										((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
										1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1));
								}
							}
							break;
						default:
							break;
					}
					/* decrease out_xe to next start pos */
					out_xe = out_xs - 1;
				}
				else
				{
					/* RMX disabled */
					if ((out_xe > 0) && (out_xe + 1 < ptr_in_param->SW.TWIN_RRZ_OUT_WD) && (in_xs <= in_xe))
					{
						result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					switch (i)
					{
						case 0:/* to revise for RRZ_D & RMX_D */
							ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;/* RRZ_A, diable from RRZ_B */
							ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = DUAL_RRZ_ALIGN(out_shift[1]);
							ptr_out_param->CAM_B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
							ptr_out_param->CAM_B_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X = DUAL_RRZ_MASK(out_shift[1]);
							ptr_out_param->CAM_B_RMX_VSIZE.Bits.RMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN);
							/* check RRZ buffer size, must x8 align */
							if (ptr_in_param->CAM_A_RRZO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_RRZO_BASE_ADDR.Bits.BASE_ADDR)
							{
								if (ptr_in_param->SW.DUAL_SEL & 0x1)
								{
									/* A+B, A+C, align from start, B+A, C+A */
									ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
									ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
								}
								else
								{
									/* B+C, align from end */
									ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = DUAL_RRZ_WD_ALL;
								}
							}
							else
							{
								if (ptr_in_param->SW.DUAL_SEL & 0x1)
								{
									/* A+B, A+C, align from start, B+A, C+A */
									ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
									ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD +
										DUAL_RLB_ALIGN_PIX - 1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1);
								}
								else
								{
									/* B+C, align from end */
									ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = DUAL_RRZ_WD_ALL -
										((((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
										1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1));
								}
							}
							break;
						case 1:
							ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 1;/* RRZ_A, disable RRZ_A */
							ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = DUAL_RRZ_ALIGN(out_shift[1]);
							ptr_out_param->CAM_A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
							ptr_out_param->CAM_A_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X = DUAL_RRZ_MASK(out_shift[1]);
							ptr_out_param->CAM_A_RMX_VSIZE.Bits.RMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN);
							/* check RRZ buffer size, must x8 align */
							if (ptr_in_param->CAM_A_RRZO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_RRZO_BASE_ADDR.Bits.BASE_ADDR)
							{
								ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST;
							}
							else
							{
								if (ptr_in_param->SW.DUAL_SEL & 0x1)
								{
									/* A+B, A+C, align from start, C+A, B+A */
									if (((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
										1)>>3)<<3) +
										(unsigned int)ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST > DUAL_RRZ_WD_ALL)
									{
										result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
										dual_driver_printf("Error: %s\n", print_error_message(result));
										return result;
									}
								}
								else
								{
									/* B+C, align from end */
									if (((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
										1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1)) >
										(unsigned int)ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST)
									{
										result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
										dual_driver_printf("Error: %s\n", print_error_message(result));
										return result;
									}
									else
									{
										/* align from end */
										ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST -
											((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
											1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1));
									}
								}
							}
							break;
						default:
							break;
					}
				}
			}
		}
		else
		{
			/* loop dual mode from right */
			for (int i=0;i<3;i++)
			{
				int in_xs_dmx=0;
				int in_xs=0;
				int in_xe=0;
				int out_xs= 0;
				/* config forward backward pos */
				switch (i)
				{
					case 0:
						/* right */
						ptr_out_param->CAM_C_RRZ_IN_IMG.Bits.RRZ_IN_WD = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X -
							(int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X + 1;
						/* init pos */
						in_xs_dmx = ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
						/* right boundary */
						in_xs = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X + DUAL_TILE_LOSS_ALL_L;
						in_xe = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X;
						out_xe= ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1;
						break;
					case 1:
						/* center */
						ptr_out_param->CAM_B_RRZ_IN_IMG.Bits.RRZ_IN_WD = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
							(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1;
						/* init pos */
						in_xs_dmx = ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
						/* right boundary */
						in_xs = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + DUAL_TILE_LOSS_ALL_L;
						in_xe = dmx_valid[0] + dmx_valid[1] + right_loss_rrz_d - 1;
						break;
					case 2:
						/* left */
						ptr_out_param->CAM_A_RRZ_IN_IMG.Bits.RRZ_IN_WD = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
							(int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1;
						/* init pos */
						in_xs_dmx = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X;
						out_xs = 0;
						in_xe = dmx_valid[0] + right_loss_rrz - 1;
						break;
					default:
						break;
				}
				/* backward & forward cal */
				if (out_xs < out_xe)
				{
					/* backward out_xe */
					long long end_temp = (long long)out_xe*ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP +
						(long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
						ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
					/* cal pos */
					if (end_temp + (long long)DUAL_TILE_LOSS_RRZ_TAP*DUAL_RRZ_PREC < (long long)in_xe*DUAL_RRZ_PREC)
					{
						int n;
						n = (int)((unsigned long long)(end_temp + DUAL_TILE_LOSS_RRZ_TAP*DUAL_RRZ_PREC)>>15);
						if (n & 0x1)
						{
							in_xe = n ;
						}
						else/* must be even */
						{
							in_xe = n + 1;
						}
					}
					else
					{
						if (in_xe + 1 < ptr_in_param->SW.TWIN_RRZ_OUT_WD)
						{
							result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
					}
					/* forward in_xs */
					if (in_xs < in_xe)
					{
						if (in_xs <= 0)
						{
							out_xs = 0;
						}
						else
						{
							int n;
							long long start_temp = (long long)(in_xs + DUAL_TILE_LOSS_RRZ_TAP_L)*DUAL_RRZ_PREC -
								(long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC -
								ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
							n = (int)(start_temp/ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP);
							if (((long long)n*ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP) < start_temp)
							{
								n = n + 1;
							}
							if (n < 0)
							{
								n = 0;
							}
							/* update align by pixel mode */
							/* RRZ_A, run i= 0 only */
							if (n & DUAL_RRZ_MASK(out_shift[i]))
							{
								out_xs = n + DUAL_RRZ_ALIGN(out_shift[i]) - (n & DUAL_RRZ_MASK(out_shift[i]));
							}
							else
							{
								out_xs = n;
							}
						}
					}
				}
				/* update RRZ width, offset, flag */
				if ((out_xs < out_xe) && (in_xs < in_xe))
				{
					long long temp_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
						ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST +
						(long long)out_xs*ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP -
						(long long)in_xs_dmx*DUAL_RRZ_PREC;
					switch (i)
					{
						case 0:/* to revise for RRZ_T & RMX_T */
							/* RRZ_T */
							ptr_out_param->CAM_C_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = out_xe - out_xs + 1;
							ptr_out_param->CAM_C_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)temp_offset>>15);
							ptr_out_param->CAM_C_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(temp_offset -
								(long long)ptr_out_param->CAM_C_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
							ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_END_X = out_xe - out_xs;
							/* check RRZ_T buffer size, must x8 align */
							/* A+B+C, align from start, C+B+A */
							ptr_out_param->CAM_C_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
							ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_out_param->CAM_C_RRZ_OUT_IMG.Bits.RRZ_OUT_WD +
								DUAL_RLB_ALIGN_PIX - 1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1);
							break;
						case 1:/* to revise for RRZ_D & RMX_D */
							/* RRZ_D */
							ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = out_xe - out_xs + 1;
							ptr_out_param->CAM_B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)temp_offset>>15);
							ptr_out_param->CAM_B_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(temp_offset -
								(long long)ptr_out_param->CAM_B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X = out_xe - out_xs;
							/* check min size */
							if (left_dmx_max_d < in_xe + 1 + DUAL_TILE_LOSS_ALL)
							{
								left_dmx_max_d = in_xe + 1 + DUAL_TILE_LOSS_ALL;
							}
							/* check RRZ_D buffer size, must x8 align */
							/* A+B+C, align from start, C+B+A */
							ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (unsigned int)ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST +
								((((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
								1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1));
							break;
						case 2:
							/* RRZ */
							ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = out_xe - out_xs + 1;
							ptr_out_param->CAM_A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)temp_offset>>15);
							ptr_out_param->CAM_A_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(temp_offset -
								(long long)ptr_out_param->CAM_A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X = out_xe - out_xs;
							/* check min size */
							if (left_dmx_max < in_xe + 1 + DUAL_TILE_LOSS_ALL)
							{
								left_dmx_max = in_xe + 1 + DUAL_TILE_LOSS_ALL;
							}
							/* check RRZ buffer size, must x8 align */
							/* A+B+C, align from start, C+B+A */
							if (((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
								1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1)) +
								(unsigned int)ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST > DUAL_RRZ_WD_ALL)
							{
								result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
							break;
						default:
							break;
					}
					/* decrease out_xe to next start pos */
					out_xe = out_xs - 1;
				}
				else
				{
					/* RMX disabled */
					if ((out_xe > 0) && (out_xe + 1 < ptr_in_param->SW.TWIN_RRZ_OUT_WD) && (in_xs <= in_xe))
					{
						result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					switch (i)
					{
						case 0:/* to revise for RRZ_T & RMX_T */
							ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;/* RRZ_B, disable from RRZ_C */
							ptr_out_param->CAM_C_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = DUAL_RRZ_ALIGN(out_shift[2]);
							ptr_out_param->CAM_C_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
							ptr_out_param->CAM_C_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
							ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_END_X = DUAL_RRZ_MASK(out_shift[2]);
							ptr_out_param->CAM_C_RMX_VSIZE.Bits.RMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN);
							/* check RRZ buffer size, must x8 align */
							if (ptr_in_param->CAM_A_RRZO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_RRZO_BASE_ADDR.Bits.BASE_ADDR)
							{
								/* A+B+C, align from start, C+B+A */
								ptr_out_param->CAM_C_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
								ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
							}
							else
							{
								/* A+B+C, align from start, C+B+A */
								ptr_out_param->CAM_C_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
								ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_out_param->CAM_C_RRZ_OUT_IMG.Bits.RRZ_OUT_WD +
									DUAL_RLB_ALIGN_PIX - 1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1);
							}
							break;
						case 1:/* to revise for RRZ_D & RMX_D */
							if (ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
							{
								/* disable RRZ_C and disable RRZ_B, disable RRZ_A slave */
								ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;/* RRZ_A, disable RRZ_B & RRZ_C */
							}
							else
							{
								/* disable RRZ_B, if RRZ_B slave enable & RRZ_C enable */
								ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 1;/* RRZ_C enable, disable RRZ_B */
							}
							ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = DUAL_RRZ_ALIGN(out_shift[2]);
							ptr_out_param->CAM_B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
							ptr_out_param->CAM_B_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X = DUAL_RRZ_MASK(out_shift[2]);
							ptr_out_param->CAM_B_RMX_VSIZE.Bits.RMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN);
							/* check RRZ buffer size, must x8 align */
							if (ptr_in_param->CAM_A_RRZO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_RRZO_BASE_ADDR.Bits.BASE_ADDR)
							{
								/* A+B+C, align from start, C+B+A */
								ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST;
							}
							else
							{
								/* A+B+C, align from start, C+B+A */
								ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (unsigned int)ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST +
									((((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
									1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1));
							}
							break;
						case 2:
							ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 1;/*  RRZ_A, disable RRZ_A */
							ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = DUAL_RRZ_ALIGN(out_shift[2]);
							ptr_out_param->CAM_A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
							ptr_out_param->CAM_A_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X = DUAL_RRZ_MASK(out_shift[2]);
							ptr_out_param->CAM_A_RMX_VSIZE.Bits.RMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN);
							/* check RRZ buffer size, must x8 align */
							if (ptr_in_param->CAM_A_RRZO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_RRZO_BASE_ADDR.Bits.BASE_ADDR)
							{
								/* A+B+C, align from start, C+B+A */
								ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST;
							}
							else
							{
								/* A+B+C, align from start, C+B+A */
								if (((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX -
									1)>>(DUAL_RLB_ALIGN_SHIFT + 1))<<(DUAL_RLB_ALIGN_SHIFT + 1)) +
									(unsigned int)ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST > DUAL_RRZ_WD_ALL)
								{
									result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
									dual_driver_printf("Error: %s\n", print_error_message(result));
									return result;
								}
							}
							break;
						default:
							break;
					}
				}
			}
		}
		if (out_xe >= 0)
		{
			result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* dual RRZO */
		if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.RRZO_EN)
		{
			int bit_per_pix = 18;
			int bus_width_shift = 3;
			if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
			{
				if (0 == ptr_in_param->CAM_A_CTL_EN.Bits.UFEG_EN)
				{
					result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				ptr_out_param->CAM_B_CTL_EN.Bits.UFEG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.UFEG_EN;
				if (dual_mode > 2)
				{
					ptr_out_param->CAM_C_CTL_EN.Bits.UFEG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.UFEG_EN;
				}
				bus_width_shift = 4;/* (1<<4)*8, 128 bits */
				switch (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
				{
					case DUAL_OUT_FMT_FG_10:/* FG 10 */
						bit_per_pix = 15;
						break;
					case DUAL_OUT_FMT_FG_12:/* FG 12 */
						bit_per_pix = 18;
						break;
					default:/* FG 14 */
						bit_per_pix = 21;
						break;
				}
			}
			else
			{
				if (0 == ptr_in_param->CAM_A_CTL_EN.Bits.PAKG_EN)
				{
					result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				ptr_out_param->CAM_B_CTL_EN.Bits.PAKG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PAKG_EN;
				if (dual_mode > 2)
				{
					ptr_out_param->CAM_C_CTL_EN.Bits.PAKG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PAKG_EN;
				}
				bus_width_shift = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?3:1;/* (1<<3)*8, 64 bits */
				switch (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
				{
					case DUAL_OUT_FMT_FG_8:/* FG 8 */
						bit_per_pix = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?12:8;
						break;
					case DUAL_OUT_FMT_FG_10:/* FG 10 */
						bus_width_shift = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?2:1;/* (1<<2)*8, 32 bits */
						bit_per_pix = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?15:10;
						break;
					case DUAL_OUT_FMT_FG_12:/* FG 12 */
						bit_per_pix = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?18:12;
						break;
					default:/* FG 14 */
						bit_per_pix = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?21:14;
						break;
				}
			}
			/* RRZO */
			ptr_out_param->CAM_A_RRZO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
			ptr_out_param->CAM_A_RRZO_CROP.Bits.XOFFSET = 0;
			/* check RRZ_A disabled or RRZ_A only */
			if (ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 || ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
			{
				/* full size output with padding */
				if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
				{
					/* x64p and 128 b */
					ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE = (((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6)*64*bit_per_pix +
						(1<<(bus_width_shift + 3)) - 1)>>(bus_width_shift + 3))*(1<<bus_width_shift) - 1;
				}
				else
				{
					ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*bit_per_pix +
						(1<<(bus_width_shift + 3)) - 1)>>(bus_width_shift + 3))*(1<<bus_width_shift) - 1;
				}
			}
			else
			{
				ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*bit_per_pix)>>3) - 1;
			}
			/* check bus width */
			if (((unsigned int)ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE + 1) & ((1<<bus_width_shift) - 1))
			{
				result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			ptr_out_param->CAM_B_RRZO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
			ptr_out_param->CAM_B_RRZO_CROP.Bits.XOFFSET = 0;
			/* check RRZ_B, C disabled or RRZ_C disabled */
			if (ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 || ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1 ||
				ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
			{
				if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
				{
					/* x64p and 128 b */
					ptr_out_param->CAM_B_RRZO_XSIZE.Bits.XSIZE = (((((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6)*64*bit_per_pix +
						(1<<(bus_width_shift + 3)) - 1)>>(bus_width_shift + 3))*(1<<bus_width_shift) - 1;
				}
				else
				{
					ptr_out_param->CAM_B_RRZO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*bit_per_pix +
						(1<<(bus_width_shift + 3)) - 1)>>(bus_width_shift + 3))*(1<<bus_width_shift) - 1;
				}
			}
			else
			{
				ptr_out_param->CAM_B_RRZO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*bit_per_pix)>>3) - 1;
			}
			/* check bus width */
			if (((unsigned int)ptr_out_param->CAM_B_RRZO_XSIZE.Bits.XSIZE + 1) & ((1<<bus_width_shift) - 1))
			{
				result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			if (dual_mode > 2)
			{
				ptr_out_param->CAM_C_RRZO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
				ptr_out_param->CAM_C_RRZO_CROP.Bits.XOFFSET = 0;
				if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
				{
					/* x64p and 128 b */
					ptr_out_param->CAM_C_RRZO_XSIZE.Bits.XSIZE = (((((unsigned int)ptr_out_param->CAM_C_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6)*64*bit_per_pix +
						(1<<(bus_width_shift + 3)) - 1)>>(bus_width_shift + 3))*(1<<bus_width_shift) - 1;
				}
				else
				{
					ptr_out_param->CAM_C_RRZO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_C_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*bit_per_pix +
						(1<<(bus_width_shift + 3)) - 1)>>(bus_width_shift + 3))*(1<<bus_width_shift) - 1;
				}
				/* check bus width */
				if (((unsigned int)ptr_out_param->CAM_C_RRZO_XSIZE.Bits.XSIZE + 1) & ((1<<bus_width_shift) - 1))
				{
					result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
			/* RRZO_D */
			ptr_out_param->CAM_B_RRZO_CON.Raw = ptr_in_param->CAM_A_RRZO_CON.Raw;
			ptr_out_param->CAM_B_RRZO_CON2.Raw = ptr_in_param->CAM_A_RRZO_CON2.Raw;
			ptr_out_param->CAM_B_RRZO_CON3.Raw = ptr_in_param->CAM_A_RRZO_CON3.Raw;
			ptr_out_param->CAM_B_RRZO_CON4.Raw = ptr_in_param->CAM_A_RRZO_CON4.Raw;
			ptr_out_param->CAM_B_RRZO_STRIDE.Raw = ptr_in_param->CAM_A_RRZO_STRIDE.Raw;
			ptr_out_param->CAM_B_RRZO_CROP.Bits.YOFFSET = ptr_in_param->CAM_A_RRZO_CROP.Bits.YOFFSET;
			ptr_out_param->CAM_B_RRZO_YSIZE.Bits.YSIZE = ptr_in_param->CAM_A_RRZO_YSIZE.Bits.YSIZE;
			/* force bus size disabled */
			ptr_out_param->CAM_A_RRZO_STRIDE.Bits.BUS_SIZE_EN = 0;
			ptr_out_param->CAM_B_RRZO_STRIDE.Bits.BUS_SIZE_EN = 0;
			if (dual_mode > 2)
			{
				/* RRZO_T */
				ptr_out_param->CAM_C_RRZO_CON.Raw = ptr_in_param->CAM_A_RRZO_CON.Raw;
				ptr_out_param->CAM_C_RRZO_CON2.Raw = ptr_in_param->CAM_A_RRZO_CON2.Raw;
				ptr_out_param->CAM_C_RRZO_CON3.Raw = ptr_in_param->CAM_A_RRZO_CON3.Raw;
				ptr_out_param->CAM_C_RRZO_CON4.Raw = ptr_in_param->CAM_A_RRZO_CON4.Raw;
				ptr_out_param->CAM_C_RRZO_STRIDE.Raw = ptr_in_param->CAM_A_RRZO_STRIDE.Raw;
				ptr_out_param->CAM_C_RRZO_CROP.Bits.YOFFSET = ptr_in_param->CAM_A_RRZO_CROP.Bits.YOFFSET;
				ptr_out_param->CAM_C_RRZO_YSIZE.Bits.YSIZE = ptr_in_param->CAM_A_RRZO_YSIZE.Bits.YSIZE;
				/* force bus size disabled */
				ptr_out_param->CAM_C_RRZO_STRIDE.Bits.BUS_SIZE_EN = 0;
			}
			/* RTL */
			if (ptr_in_param->CAM_A_RRZO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_RRZO_BASE_ADDR.Bits.BASE_ADDR)
			{
				if (dual_mode <= 2)
				{
					/* check slave disable */
					if (ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
					{
						/* slave disable, RRZO_A only */
						ptr_out_param->CAM_B_CTL_EN.Bits.RRZ_EN = 0;
						ptr_out_param->CAM_B_CTL_DMA_EN.Bits.RRZO_EN = 0;
						ptr_out_param->CAM_B_CTL_EN.Bits.PAKG_EN = 0;
					}
					else if (ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
					{
						/* master disable, RRZO_B only */
						ptr_out_param->CAM_A_CTL_EN.Bits.RRZ_EN = 0;
						ptr_out_param->CAM_A_CTL_DMA_EN.Bits.RRZO_EN = 0;
						ptr_out_param->CAM_A_CTL_EN.Bits.PAKG_EN = 0;
						/* restore CAM_A_RRZO_XSIZE for driver config sync */
						ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_RRZO_XSIZE.Bits.XSIZE;
					}
					else
					{
						ptr_out_param->CAM_B_RRZO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE + 1;
					}
				}
				else
				{
					/* check RRZ_A enabled */
					if (ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
					{
						/* master disable, RRZO_A off */
						ptr_out_param->CAM_A_CTL_EN.Bits.RRZ_EN = 0;
						ptr_out_param->CAM_A_CTL_DMA_EN.Bits.RRZO_EN = 0;
						ptr_out_param->CAM_A_CTL_EN.Bits.PAKG_EN = 0;
						/* restore CAM_A_RRZO_XSIZE for driver config sync */
						ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_RRZO_XSIZE.Bits.XSIZE;
					}
					if (ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 || ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
					{
						/* slave disable or master disable, RRZO_B off */
						ptr_out_param->CAM_B_CTL_EN.Bits.RRZ_EN = 0;
						ptr_out_param->CAM_B_CTL_DMA_EN.Bits.RRZO_EN = 0;
						ptr_out_param->CAM_B_CTL_EN.Bits.PAKG_EN = 0;
					}
					if (ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
					{
						/* master disable, RRZO_C off */
						ptr_out_param->CAM_C_CTL_EN.Bits.RRZ_EN = 0;
						ptr_out_param->CAM_C_CTL_DMA_EN.Bits.RRZO_EN = 0;
						ptr_out_param->CAM_C_CTL_EN.Bits.PAKG_EN = 0;
					}
					if (0 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
					{
						/* check slave enabled, A */
						if ((0 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2) && (0 == ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2))
						{
							/* master B enable, A+B, A+B+C */
							ptr_out_param->CAM_B_RRZO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE + 1;
						}
						if ((0 == ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1) && (0 == ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2))
						{
							/* check slave enabled, A+B+C, B+C */
							ptr_out_param->CAM_C_RRZO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_B_RRZO_OFST_ADDR.Bits.OFFSET_ADDR +
								(unsigned int)ptr_out_param->CAM_B_RRZO_XSIZE.Bits.XSIZE + 1;
						}
					}
				}
			}
			else
			{
				ptr_out_param->CAM_B_RRZO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
				/* restore stride */
				ptr_out_param->CAM_B_RRZO_STRIDE.Bits.STRIDE = ptr_in_param->CAM_B_RRZO_STRIDE.Bits.STRIDE;
				/* check valid size only WDMA with XOFF and YOFF support */
				if ((int)ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE + 1 > (int)ptr_in_param->CAM_A_RRZO_STRIDE.Bits.STRIDE)
				{
					if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
					{
						result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_in_param->CAM_A_RRZO_STRIDE.Bits.STRIDE>>bus_width_shift)<<bus_width_shift) - 1;
					}
				}
				if ((int)ptr_out_param->CAM_B_RRZO_XSIZE.Bits.XSIZE + 1 > (int)ptr_in_param->CAM_B_RRZO_STRIDE.Bits.STRIDE)
				{
					if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
					{
						result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						ptr_out_param->CAM_B_RRZO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_in_param->CAM_B_RRZO_STRIDE.Bits.STRIDE>>bus_width_shift)<<bus_width_shift) - 1;
					}
				}
				if (dual_mode > 2)
				{
					ptr_out_param->CAM_C_RRZO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
					/* restore stride */
					ptr_out_param->CAM_C_RRZO_STRIDE.Bits.STRIDE = ptr_in_param->CAM_C_RRZO_STRIDE.Bits.STRIDE;
					if ((int)ptr_out_param->CAM_C_RRZO_XSIZE.Bits.XSIZE + 1 > (int)ptr_in_param->CAM_C_RRZO_STRIDE.Bits.STRIDE)
					{
						if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
						{
							result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
						else
						{
							ptr_out_param->CAM_C_RRZO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_in_param->CAM_C_RRZO_STRIDE.Bits.STRIDE>>bus_width_shift)<<bus_width_shift) - 1;
						}
					}
				}
			}
			if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
			{
				ptr_out_param->CAM_A_UFGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
				ptr_out_param->CAM_B_UFGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
				if (dual_mode > 2)
				{
					ptr_out_param->CAM_C_UFGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
				}
				if (ptr_in_param->CAM_A_UFGO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_UFGO_BASE_ADDR.Bits.BASE_ADDR)
				{
					if (ptr_in_param->CAM_A_CTL_EN.Bits.UFEG_EN && ptr_in_param->CAM_A_CTL_DMA_EN.Bits.UFGO_EN)
					{
						/* UFGO, test mode, size not x8 */
						ptr_out_param->CAM_B_UFGO_CON.Raw = ptr_in_param->CAM_A_UFGO_CON.Raw;
						ptr_out_param->CAM_B_UFGO_CON2.Raw = ptr_in_param->CAM_A_UFGO_CON2.Raw;
						ptr_out_param->CAM_B_UFGO_CON3.Raw = ptr_in_param->CAM_A_UFGO_CON3.Raw;
						ptr_out_param->CAM_B_UFGO_CON4.Raw = ptr_in_param->CAM_A_UFGO_CON4.Raw;
						ptr_out_param->CAM_B_UFGO_STRIDE.Raw = ptr_in_param->CAM_A_UFGO_STRIDE.Raw;
						ptr_out_param->CAM_B_UFGO_YSIZE.Bits.YSIZE = ptr_in_param->CAM_A_UFGO_YSIZE.Bits.YSIZE;
						ptr_out_param->CAM_B_UFEG_CON.Raw = ptr_in_param->CAM_A_UFEG_CON.Raw;
						if (dual_mode > 2)
						{
							ptr_out_param->CAM_C_UFGO_CON.Raw = ptr_in_param->CAM_A_UFGO_CON.Raw;
							ptr_out_param->CAM_C_UFGO_CON2.Raw = ptr_in_param->CAM_A_UFGO_CON2.Raw;
							ptr_out_param->CAM_C_UFGO_CON3.Raw = ptr_in_param->CAM_A_UFGO_CON3.Raw;
							ptr_out_param->CAM_C_UFGO_CON4.Raw = ptr_in_param->CAM_A_UFGO_CON4.Raw;
							ptr_out_param->CAM_C_UFGO_STRIDE.Raw = ptr_in_param->CAM_A_UFGO_STRIDE.Raw;
							ptr_out_param->CAM_C_UFGO_YSIZE.Bits.YSIZE = ptr_in_param->CAM_A_UFGO_YSIZE.Bits.YSIZE;
							ptr_out_param->CAM_C_UFEG_CON.Raw = ptr_in_param->CAM_A_UFEG_CON.Raw;
						}
						if (dual_mode <= 2)
						{
							if (1 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
							{
								/* UFGO_A only */
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.UFGO_EN = 0;
								ptr_out_param->CAM_B_CTL_EN.Bits.UFEG_EN = 0;
								ptr_out_param->CAM_A_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
							}
							else if (1 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
							{
								/* UFGO_B only */
								ptr_out_param->CAM_A_CTL_DMA_EN.Bits.UFGO_EN = 0;
								ptr_out_param->CAM_A_CTL_EN.Bits.UFEG_EN = 0;
								ptr_out_param->CAM_B_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
								/* restore CAM_A_UFGO_XSIZE for driver config sync */
								ptr_out_param->CAM_A_UFGO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_UFGO_XSIZE.Bits.XSIZE;
							}
							else
							{
								/* UFGO, test mode, size not x8 */
								ptr_out_param->CAM_A_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
								ptr_out_param->CAM_B_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
								ptr_out_param->CAM_B_UFGO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_UFGO_XSIZE.Bits.XSIZE + 1;
							}
						}
						else
						{
							/* check master disable */
							if (ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
							{
								/* master disable, RRZO_A off */
								ptr_out_param->CAM_A_CTL_DMA_EN.Bits.UFGO_EN = 0;
								ptr_out_param->CAM_A_CTL_EN.Bits.UFEG_EN = 0;
								/* restore CAM_A_UFGO_XSIZE for driver config sync */
								ptr_out_param->CAM_A_UFGO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_UFGO_XSIZE.Bits.XSIZE;
							}
							else
							{
								ptr_out_param->CAM_A_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
							}
							if (ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 || ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
							{
								/* master disable, RRZO_B off */
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.UFGO_EN = 0;
								ptr_out_param->CAM_B_CTL_EN.Bits.UFEG_EN = 0;
							}
							else
							{
								ptr_out_param->CAM_B_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
							}
							if (ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
							{
								/* master disable, RRZO_C off */
								ptr_out_param->CAM_C_CTL_DMA_EN.Bits.UFGO_EN = 0;
								ptr_out_param->CAM_C_CTL_EN.Bits.UFEG_EN = 0;
							}
							else
							{
								ptr_out_param->CAM_C_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_C_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
							}
							if (0 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
							{
								/* check slave enabled, A */
								if ((0 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2) && (0 == ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2))
								{
									/* master B enable, A+B, A+B+C */
									ptr_out_param->CAM_B_UFGO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_UFGO_XSIZE.Bits.XSIZE + 1;
								}
								if ((0 == ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1) && (0 == ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2))
								{
									/* check slave enabled, A+B+C, B+C */
									ptr_out_param->CAM_C_UFGO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_B_UFGO_OFST_ADDR.Bits.OFFSET_ADDR +
										(unsigned int)ptr_out_param->CAM_B_UFGO_XSIZE.Bits.XSIZE + 1;
								}
							}
							if ((0 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1) && (0 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2) &&
								(0 == ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1) && (0 == ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2))
							{
								/* A+B+C, A must compress off */
								ptr_out_param->CAM_A_UFEG_CON.Bits.UFEG_FORCE_PCM = 1;
							}
						}
					}
					else
					{
						if (ptr_in_param->CAM_A_CTL_EN.Bits.UFEG_EN || ptr_in_param->CAM_A_CTL_DMA_EN.Bits.UFGO_EN)
						{
							result = DUAL_MESSAGE_UFGO_CONFIG_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
					}
				}
				else
				{
					/* UFGO */
					ptr_out_param->CAM_A_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
					ptr_out_param->CAM_B_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
					if (dual_mode > 2)
					{
						ptr_out_param->CAM_C_UFGO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_C_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
					}
				}
			}
		}
	}
	/* RMX */
	ptr_out_param->CAM_B_CTL_EN.Bits.RMX_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RMX_EN;
	ptr_out_param->CAM_B_CTL_EN2.Bits.SGG2_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.SGG2_EN;
	ptr_out_param->CAM_B_CTL_EN2.Bits.GSE_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.GSE_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.RMX_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RMX_EN;
		ptr_out_param->CAM_C_CTL_EN2.Bits.SGG2_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.SGG2_EN;
		ptr_out_param->CAM_C_CTL_EN2.Bits.GSE_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.GSE_EN;
	}
	if (ptr_in_param->CAM_A_CTL_EN.Bits.RMX_EN)
	{
		if (0 == ptr_in_param->CAM_A_CTL_EN.Bits.RRZ_EN)
		{
			/* RMX bypass to reduce RAW A RMX_END_X */
			ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_STR_X = 0;
			if ((ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_RMXO > 1) && (dmx_valid[0] & 0x3))
			{
				ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X = dmx_valid[0] + 3 - (dmx_valid[0] & 0x3);
			}
			else
			{
				ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X = dmx_valid[0] - 1;
			}
			ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 0;
			ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
			ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X = (int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1 -
				(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_EDGE = 0xF;
			ptr_out_param->CAM_A_RMX_VSIZE.Bits.RMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
				((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN);
			ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_EDGE = 0xF;
			ptr_out_param->CAM_B_RMX_VSIZE.Bits.RMX_HT = ptr_out_param->CAM_A_RMX_VSIZE.Bits.RMX_HT;
			/* check min size */
			if (left_dmx_max < (int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1 + DUAL_TILE_LOSS_ALL)
			{
				left_dmx_max = (int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1 + DUAL_TILE_LOSS_ALL;
			}
			if (dual_mode <= 2)
			{
				ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;
				ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
				ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
					(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
			}
			else
			{
				ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 0;
				ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
				ptr_out_param->CAM_C_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;
				ptr_out_param->CAM_C_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
				if ((ptr_in_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_RMXO > 1) && (dmx_valid[1] & 0x3))
				{
					ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X = dmx_valid[1] + 3 - (dmx_valid[1] & 0x3);
				}
				else
				{
					ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X = dmx_valid[1] - 1;
				}
				ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_STR_X = (int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X + 1 -
					(int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
				ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_END_X = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X -
					(int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
				ptr_out_param->CAM_C_RMX_CTL.Bits.RMX_EDGE = 0xF;
				ptr_out_param->CAM_C_RMX_VSIZE.Bits.RMX_HT = ptr_out_param->CAM_A_RMX_VSIZE.Bits.RMX_HT;
				/* check min size */
				if (left_dmx_max_d < (int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X +
					(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1 + DUAL_TILE_LOSS_ALL)
				{
					left_dmx_max_d = (int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X +
						(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1 + DUAL_TILE_LOSS_ALL;
				}
			}
		}
		/* check RMX size */
		if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_RMXO < 2)
		{
			if ((((int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1) & 0x1) ||
				(ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X & 0x1) ||
				(((int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X + 1) & 0x1) ||
				((dual_mode > 2) && ((ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_STR_X & 0x1) ||
				(((int)ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_END_X + 1) & 0x1))))
			{
				result = DUAL_MESSAGE_RMX_SIZE_CAL_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		else
		{
			if ((((int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1) & 0x3) ||
				(ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X & 0x1) ||
				(((int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X + 1) & 0x1) ||
				(((int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X - (int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X + 1) & 0x3) ||
				((dual_mode > 2) && ((ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_STR_X & 0x1) ||
				(((int)ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_END_X + 1) & 0x1) ||
				(((int)ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_END_X - (int)ptr_out_param->CAM_C_RMX_CROP.Bits.RMX_STR_X + 1) & 0x3))))
			{
				result = DUAL_MESSAGE_RMX_SIZE_CAL_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
	}
	/* minimize DMX_A_END_X and update all left end */
	if (left_dmx_max & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1))
	{
		left_dmx_max += DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - (left_dmx_max & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1));
	}
	if (left_dmx_max > dmx_valid[0] + dmx_valid[1])
	{
	    result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}
	if (left_dmx_max > (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)
	{
	    result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	}
	else
	{
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X = left_dmx_max - 1;
	}
	/* RRZ A enabled */
	if (ptr_out_param->CAM_A_CTL_EN.Bits.RRZ_EN)
	{
		ptr_out_param->CAM_A_RRZ_IN_IMG.Bits.RRZ_IN_WD = left_dmx_max;
	}
	if (dual_mode > 2)
	{
		/* minimize DMX_B_END_X and update all left end */
		if (left_dmx_max_d & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1))
		{
			left_dmx_max_d += DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - (left_dmx_max_d & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1));
		}
		if (left_dmx_max_d > dmx_valid[0] + dmx_valid[1] + dmx_valid[2])
		{
			result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		if (left_dmx_max_d > (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)
		{
			result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
		}
		else
		{
			ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X = left_dmx_max_d - 1;
		}
		/* RRZ B enabled */
		if (ptr_out_param->CAM_B_CTL_EN.Bits.RRZ_EN)
		{
			ptr_out_param->CAM_B_RRZ_IN_IMG.Bits.RRZ_IN_WD = left_dmx_max_d - (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		}
	}
	/* DMX_A_END_X */
	if (((int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + raw_wd[0] <= (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X) ||
		((int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X >= dmx_valid[0] + dmx_valid[1]))
	{
	    result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}
	/* DMX_B_END_X */
	if (dual_mode > 2)
	{
		/* change DMX_B_END_X */
		if (((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + raw_wd[1] <= (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X) ||
			((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X >= dmx_valid[0] + dmx_valid[1] + dmx_valid[2]))
		{
			result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_dmx_rrz_rmx_af_p2(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param,
									  DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	int dmx_valid[3] = {0, 0, 0};
	int bin_sel_h_size = ((0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
		((int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_E - (int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_S):
		ptr_in_param->SW.TWIN_RAWI_XSIZE)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
		(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN);
	/* following to update */
	int left_loss_af = 0;
	int right_loss_af = 0;
	int left_margin_af = 0;
	int right_margin_af = 0;
	/* upddate FMT_SEL */
	/* update bmx pixel mode */
	/* update dmx pixel mode */
	/* check PIX_BUS_DMXO */
	/* final sync with PIX_BUS_DMXI */
	/* check dmx full size, align with PIX_BUS_DMXI */
	/* shift size by binning modules enable */
	/* check after bin size, align with bin mode */
	/* Error check */
	/* align RCP3 for IMGO, only sync for IMGO_A_XSIZE */
	/* RTL verif or platform */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN)
	{
		if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
		{
			if (ptr_in_param->CAM_A_AF_CON2.Bits.AF_DS_EN && ptr_in_param->CAM_A_AF_CON.Bits.AF_H_GONLY)
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			if (dual_mode <= 2)
			{
				left_loss_af = DUAL_AF_TAPS*2;
				right_loss_af = DUAL_AF_TAPS*2 + 2;
			}
			else
			{
				if (ptr_in_param->CAM_A_AF_CON2.Bits.AF_DS_EN || ptr_in_param->CAM_A_AF_CON.Bits.AF_H_GONLY)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				left_loss_af = DUAL_AF_TAPS;
				right_loss_af = DUAL_AF_TAPS;
			}
			left_margin_af = 0;
			right_margin_af = DUAL_AF_MAX_BLOCK_WIDTH - 2;
			if ((int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH)
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		/* RRZ ofst check */
		/* padding left_loss & right_loss */
		/* update bin_cut_a for DMX to ensure HDR bit-true */
		/* dispatch max buffer size */
		/* minus loss */
		/* with af offset */
		/* cal valid af config*/
		if ((ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE & 0x1) || (ptr_in_param->SW.TWIN_AF_OFFSET & 0x1))
		{
			result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* RTL verif or platform */
		if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
		{
			/* with af offset */
			if (ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
			{
				if (bin_sel_h_size < ptr_in_param->SW.TWIN_AF_OFFSET + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
			else
			{
				result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		/* dispatch dmx size */
		if (dual_mode <= 2)
		{
			dmx_valid[0] = ((((bin_sel_h_size + 1)>>1) + 1)>>1)<<1;
			dmx_valid[1] = bin_sel_h_size - dmx_valid[0];
		}
		else
		{
			dmx_valid[0] = ((((bin_sel_h_size + 2)/3) + 1)>>1)<<1;
			dmx_valid[1] = ((((bin_sel_h_size - dmx_valid[0] + 1)>>1) + 1)>>1)<<1;
			dmx_valid[2] = bin_sel_h_size  - dmx_valid[0] - dmx_valid[1];
		}
		/* DMX */
		/* DMX_D */
		/* BMX */
		/* add BMX_A_SIG_MODE1 & 2 */
		/* BMX_D */
		/* AMX */
		/* add AMX_A_SIG_MODE1 & 2 */
		/* AMX_D */
		/* add AMX_B_SIG_MODE1 & 2 */
		/* RCROP */
		ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN;
		if (dual_mode > 2)
		{
			/* A+B+C */
			ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN;
		}
		if (ptr_in_param->CAM_A_CTL_EN.Bits.RCP_EN)
		{
			/* RCROP */
			/* RCROP_D */
			/* AF & AFO */
			if (((1 != ptr_in_param->CAM_A_CTL_SEL.Bits.SGG_SEL) || (1 != ptr_in_param->CAM_B_CTL_SEL.Bits.SGG_SEL) ||
				(0 == ptr_in_param->CAM_A_CTL_EN.Bits.SGG1_EN) || (0 == ptr_in_param->CAM_A_CTL_EN2.Bits.SGG5_EN)) ||
				((dual_mode > 2) && ((1 != ptr_in_param->CAM_C_CTL_SEL.Bits.SGG_SEL))))
			{
				result = DUAL_MESSAGE_INVALID_AF_SGG_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			/* AF & AF_D */
			/* RTL verif or platform */
			if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
			{
				/* platform */
				if ((ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE != ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE) ||
					((int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH) ||
					((dmx_valid[0] + dmx_valid[1] + ((dual_mode > 2)?dmx_valid[2]:0)) < ptr_in_param->SW.TWIN_AF_OFFSET +
					(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM) ||
					(ptr_in_param->SW.TWIN_AF_BLOCK_XNUM <= 0))
				{
					result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				/* check AF_A, minimize DMX_A_END_X */
				if (dmx_valid[0] <= ptr_in_param->SW.TWIN_AF_OFFSET)
				{
					/* B+C only */
					ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN = false;
					ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN = false;
				}
				else
				{
					/* A+B , cover cross region by right_margin_af */
					ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET;
					/* check AF_A output all */
					if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
						ptr_in_param->SW.TWIN_AF_OFFSET + 2 <= dmx_valid[0] + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
					{
						/* A only */
						ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
					}
					else
					{
						/* B+C, minimize DMX_A_END_X */
						ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = (dmx_valid[0] + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
							2 - ptr_in_param->SW.TWIN_AF_OFFSET)/(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
					}
				}
			}
			else
			{
				/* RTL different base addr */
				ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= 0;
				if (dmx_valid[0] < (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_in_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM)
				{
					ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = dmx_valid[0]/(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
				}
			}
			if (ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN)
			{
				/* check max 128 */
				if (ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
				{
					if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						/* RTL */
						ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
					}
				}
			}
			/* RTL verif or platform */
			if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
			{
				if (ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN)
				{
					if (dual_mode <=2)
					{
						/* A+B */
						if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM > (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM)
						{
							/* A+B */
							ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
								(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
								 (dmx_valid[0] - left_loss_af - left_margin_af);
							ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM -
								(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM;
						}
						else
						{
							/* A only */
							ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = false;
							ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = false;
						}
					}
					else
					{
						/* A+B+C */
						if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM > (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM)
						{
							/* A+B+C */
							if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
								ptr_in_param->SW.TWIN_AF_OFFSET + 2 <= dmx_valid[0] + dmx_valid[1] + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
							{
								/* A+B */
								ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM -
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM;
								ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = false;
								ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN = false;
							}
							else
							{
								/* A+B+C, minimize DMX_B_END_X */
								ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = (dmx_valid[0] + dmx_valid[1] +
									(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 2 -
									ptr_in_param->SW.TWIN_AF_OFFSET)/(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM;
								ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM -
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM - (int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM;
								ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
									(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
									 (dmx_valid[0] + dmx_valid[1] - left_loss_af - left_margin_af);
							}
							ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
								(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
								 (dmx_valid[0] - left_loss_af - left_margin_af);
						}
						else
						{
							/* A only */
							ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = false;
							ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = false;
							ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = false;
							ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN = false;
						}
					}
				}
				else
				{
					/* B+C only */
					if (dual_mode <= 2)
					{
						/* B only */
						ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET - (dmx_valid[0] - left_loss_af - left_margin_af);
						ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
					}
					else
					{
						/* B+C only */
						if (dmx_valid[0] + dmx_valid[1] <= ptr_in_param->SW.TWIN_AF_OFFSET)
						{
							/* C only */
							ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = false;
							ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = false;
							ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET - (dmx_valid[0] + dmx_valid[1] - left_loss_af - left_margin_af);
							ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
						}
						else
						{
							/* B+C only */
							ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET - (dmx_valid[0] - left_loss_af - left_margin_af);
							if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
								ptr_in_param->SW.TWIN_AF_OFFSET + 2 <= dmx_valid[0] + dmx_valid[1] + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
							{
								/* B only */
								ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
								ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = false;
								ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN = false;
							}
							else
							{
								/* B+C, minimize DMX_B_END_X */
								ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = (dmx_valid[0] + dmx_valid[1] +
									(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 2 - ptr_in_param->SW.TWIN_AF_OFFSET)/
									(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
								ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
									(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
									 (dmx_valid[0] + dmx_valid[1] - left_loss_af - left_margin_af);
								ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM - ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM;
							}
						}
					}
				}
				ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_YSTART = ptr_in_param->CAM_A_AF_VLD.Bits.AF_VLD_YSTART;
				ptr_out_param->CAM_B_AF_CON.Raw = ptr_in_param->CAM_A_AF_CON.Raw;
				ptr_out_param->CAM_B_AF_CON2.Raw = ptr_in_param->CAM_A_AF_CON2.Raw;
				ptr_out_param->CAM_B_AF_BLK_0.Raw = ptr_in_param->CAM_A_AF_BLK_0.Raw;
				ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_YNUM = ptr_in_param->CAM_A_AF_BLK_1.Bits.AF_BLK_YNUM;
				ptr_out_param->CAM_B_AF_BLK_2.Raw = ptr_in_param->CAM_A_AF_BLK_2.Raw;
				ptr_out_param->CAM_B_AFO_DRS.Raw = ptr_in_param->CAM_A_AFO_DRS.Raw;
				ptr_out_param->CAM_B_AFO_CON.Raw = ptr_in_param->CAM_A_AFO_CON.Raw;
				ptr_out_param->CAM_B_AFO_CON2.Raw = ptr_in_param->CAM_A_AFO_CON2.Raw;
				ptr_out_param->CAM_B_AFO_CON3.Raw = ptr_in_param->CAM_A_AFO_CON3.Raw;
				ptr_out_param->CAM_B_AFO_CON4.Raw = ptr_in_param->CAM_A_AFO_CON4.Raw;
				if (dual_mode > 2)
				{
					ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_YSTART = ptr_in_param->CAM_A_AF_VLD.Bits.AF_VLD_YSTART;
					ptr_out_param->CAM_C_AF_CON.Raw = ptr_in_param->CAM_A_AF_CON.Raw;
					ptr_out_param->CAM_C_AF_CON2.Raw = ptr_in_param->CAM_A_AF_CON2.Raw;
					ptr_out_param->CAM_C_AF_BLK_0.Raw = ptr_in_param->CAM_A_AF_BLK_0.Raw;
					ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_YNUM = ptr_in_param->CAM_A_AF_BLK_1.Bits.AF_BLK_YNUM;
					ptr_out_param->CAM_C_AF_BLK_2.Raw = ptr_in_param->CAM_A_AF_BLK_2.Raw;
					ptr_out_param->CAM_C_AFO_DRS.Raw = ptr_in_param->CAM_A_AFO_DRS.Raw;
					ptr_out_param->CAM_C_AFO_CON.Raw = ptr_in_param->CAM_A_AFO_CON.Raw;
					ptr_out_param->CAM_C_AFO_CON2.Raw = ptr_in_param->CAM_A_AFO_CON2.Raw;
					ptr_out_param->CAM_C_AFO_CON3.Raw = ptr_in_param->CAM_A_AFO_CON3.Raw;
					ptr_out_param->CAM_C_AFO_CON4.Raw = ptr_in_param->CAM_A_AFO_CON4.Raw;
				}
			}
			else
			{
				/* RTL, diff base_addr */
				ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= 0;
				if (dmx_valid[1] < (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM)
				{
					ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = dmx_valid[1]/(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
				}
				if (dual_mode > 2)
				{
					ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= 0;
					if (dmx_valid[2] < (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM)
					{
						ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = dmx_valid[2]/(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
					}
				}
			}
			if (ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN)
			{
				/* check max 128 */
				if (ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
				{
					if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						/* RTL */
						ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
					}
				}
			}
			if (dual_mode > 2)
			{
				if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
				{
					/* platform */
					if (ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE != ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				if (ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN)
				{
					/* check max 128 */
					if (ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
					{
						if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR)
						{
							result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
						else
						{
							/* RTL */
							ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
						}
					}
				}
			}
			if (ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE < DUAL_AF_BLOCK_BYTE)
			{
				result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
			{
				ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
				ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE - 1;
				/* check stride */
				if ((unsigned int)ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR + (unsigned int)ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1 >
					(unsigned int)ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE)
				{
					if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						/* RTL */
						ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE -
							(unsigned int)ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR)/DUAL_AF_BLOCK_BYTE;
						ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE*(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM - 1;
					}
				}
			}
			if (ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN)
			{
				ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE - 1;
				if ((int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE < DUAL_AF_BLOCK_BYTE)
				{
					result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				/* RTL verif or platform */
				if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
				{
					ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR = ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?
						((int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE):0;
					ptr_out_param->CAM_B_AFO_YSIZE.Raw = ptr_in_param->CAM_A_AFO_YSIZE.Raw;
				}
				if ((unsigned int)ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR + (unsigned int)ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE + 1 >
					(unsigned int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
				{
					if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						/* RTL */
						ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE -
							(unsigned int)ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR)/DUAL_AF_BLOCK_BYTE;
						ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE*(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM  - 1;
					}
				}
				if ((ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?((unsigned int)ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1):0) +
					(unsigned int)ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE + 1 > (unsigned int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
				{
					if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
			}
			if (dual_mode > 2)
			{
				if (ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN)
				{
					ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE - 1;
					if ((int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE < DUAL_AF_BLOCK_BYTE)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					/* RTL verif or platform */
					if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR)
					{
						ptr_out_param->CAM_C_AFO_OFST_ADDR.Bits.OFFSET_ADDR = (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?
							((int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE):0) +
							(ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN?
							((int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*DUAL_AF_BLOCK_BYTE):0);
						ptr_out_param->CAM_C_AFO_YSIZE.Raw = ptr_in_param->CAM_A_AFO_YSIZE.Raw;
					}
					if ((unsigned int)ptr_out_param->CAM_C_AFO_OFST_ADDR.Bits.OFFSET_ADDR + (unsigned int)ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE + 1 >
						(unsigned int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE)
					{
						if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR)
						{
							result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
						else
						{
							/* RTL */
							ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE -
								(unsigned int)ptr_out_param->CAM_C_AFO_OFST_ADDR.Bits.OFFSET_ADDR)/DUAL_AF_BLOCK_BYTE;
							ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE*(int)ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM  - 1;
						}
					}
					if ((ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?((int)ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1):0) +
						(ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN?((int)ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE + 1):0) +
						(int)ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE + 1 > (int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE)
					{
						if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR)
						{
							result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
					}
				}
			}
			/* AFO_A padding */
			if ((false == ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN) &&
				(ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR))
			{
				if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE)
				{
					ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= 0;
					ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
					ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR = DUAL_AF_BLOCK_BYTE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
					ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE - 1;
					ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN = true;
					ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN = true;
				}
			}
			if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
			{
				/* error check A */
				if ((3 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_GONLY))
				{
					if (ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				else if ((3 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_AVG_LVL) ||
					((2 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_AVG_LVL) &&
					(1 == ptr_in_param->CAM_A_AF_CON.Bits.AF_V_GONLY)))
				{
					if (ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				else
				{
					if (ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
			}
			/* AFO_B padding */
			if ((false == ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN) &&
				(ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR))
			{
				if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
				{
		 			ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= 0;
					ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
					ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR = DUAL_AF_BLOCK_BYTE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
					ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE - 1;
					ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = true;
					ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = true;
				}
			}
			if (ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN)
			{
				/* error check B */
				if ((3 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_GONLY))
				{
					if (ptr_out_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				else if ((3 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_AVG_LVL) ||
					((2 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_AVG_LVL) &&
					(1 == ptr_out_param->CAM_B_AF_CON.Bits.AF_V_GONLY)))
				{
					if (ptr_out_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				else
				{
					if (ptr_out_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
			}
			if (dual_mode > 2)
			{
				/* AFO_Cpadding */
				if ((false == ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN) &&
					(ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_C_AFO_BASE_ADDR.Bits.BASE_ADDR))
				{
					if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->CAM_C_AFO_STRIDE.Bits.STRIDE)
					{
		 				ptr_out_param->CAM_C_AF_VLD.Bits.AF_VLD_XSTART= 0;
						ptr_out_param->CAM_C_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
						ptr_out_param->CAM_C_AFO_OFST_ADDR.Bits.OFFSET_ADDR = DUAL_AF_BLOCK_BYTE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
						ptr_out_param->CAM_C_AFO_XSIZE.Bits.XSIZE = DUAL_AF_BLOCK_BYTE - 1;
						ptr_out_param->CAM_C_CTL_EN.Bits.AF_EN = true;
						ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN = true;
					}
				}
				if (ptr_out_param->CAM_C_CTL_DMA_EN.Bits.AFO_EN)
				{
					/* error check B */
					if ((3 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_GONLY))
					{
						if (ptr_out_param->CAM_C_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
						{
							result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
					}
					else if ((3 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_AVG_LVL) ||
						((2 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_AVG_LVL) &&
						(1 == ptr_out_param->CAM_C_AF_CON.Bits.AF_V_GONLY)))
					{
						if (ptr_out_param->CAM_C_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
						{
							result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
					}
					else
					{
						if (ptr_out_param->CAM_C_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
						{
							result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
					}
				}
			}
			/* sync RCP size & AF_IMAGE_WD */
			ptr_out_param->CAM_A_AF_SIZE.Bits.AF_IMAGE_WD = dmx_valid[0] + right_loss_af + right_margin_af;
			if (dual_mode <= 2)
			{
				ptr_out_param->CAM_B_AF_SIZE.Bits.AF_IMAGE_WD = left_loss_af + left_margin_af + dmx_valid[1];
			}
			else
			{
				ptr_out_param->CAM_B_AF_SIZE.Bits.AF_IMAGE_WD = left_loss_af + left_margin_af + dmx_valid[1] + right_loss_af + right_margin_af;
				ptr_out_param->CAM_C_AF_SIZE.Bits.AF_IMAGE_WD = left_loss_af + left_margin_af + dmx_valid[2];
			}
		}
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_dbn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* dbn x2 DMX */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN)
	{
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X<<1;
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X<<1;
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
		if (dual_mode > 2)
		{
			ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X<<1;
			ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
		}
	}
	/* BIN_D_EN */
	ptr_out_param->CAM_B_CTL_EN.Bits.DBN_EN = ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN;
	ptr_out_param->CAM_B_CTL_EN2.Bits.VBN_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.DBN_EN = ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN;
		ptr_out_param->CAM_C_CTL_EN2.Bits.VBN_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_bin(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* binning x2 DMX */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
	{
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X<<1;
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X<<1;
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
		if (ptr_in_param->CAM_A_BIN_CTL.Bits.BIN_LE_INV_CTL)
		{
		    result = DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		ptr_out_param->CAM_B_BIN_CTL.Bits.BIN_LE_INV_CTL = 0;
		if (dual_mode > 2)
		{
			ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X<<1;
			ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
			ptr_out_param->CAM_C_BIN_CTL.Bits.BIN_LE_INV_CTL = 0;
		}
	}
	/* BIN_D_EN */
	ptr_out_param->CAM_B_CTL_EN.Bits.BIN_EN = ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.BIN_EN = ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_scm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* scm x2 DMX */
	if (ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN)
	{
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X<<1;
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X<<1;
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
		if (ptr_in_param->CAM_A_SCM_CFG1.Bits.SCM_LE_INV_CTL)
		{
		    result = DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		ptr_out_param->CAM_B_SCM_CFG1.Bits.SCM_LE_INV_CTL = 0;
		if (dual_mode > 2)
		{
			ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X<<1;
			ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
			ptr_out_param->CAM_C_SCM_CFG1.Bits.SCM_LE_INV_CTL = 0;
		}
	}
	/* SCM_D_EN */
	ptr_out_param->CAM_B_CTL_EN2.Bits.SCM_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN2.Bits.SCM_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_sgm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* OB_D_EN */
	ptr_out_param->CAM_B_CTL_EN.Bits.SGM_EN = ptr_in_param->CAM_A_CTL_EN.Bits.SGM_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.SGM_EN = ptr_in_param->CAM_A_CTL_EN.Bits.SGM_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_sl2f(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* SL2F_A_EN */
	if (ptr_in_param->CAM_A_CTL_EN2.Bits.SL2F_EN)
	{
		int dmx_v_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN)):((unsigned int)(ptr_in_param->CAM_UNI_RAWI_YSIZE.Bits.YSIZE +
			1)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN));
		if ((0 == ptr_in_param->CAM_A_SL2F_RZ.Bits.SL2_HRZ_COMP) || (0 == ptr_in_param->CAM_A_SL2F_RZ.Bits.SL2_VRZ_COMP))
		{
		    result = DUAL_MESSAGE_ZERO_SL_HVRZ_COMP_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		ptr_out_param->CAM_A_SL2F_XOFF.Bits.SL2_X_OFST = (unsigned int)ptr_in_param->CAM_A_SL2F_RZ.Bits.SL2_HRZ_COMP*
			ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_A_SL2F_YOFF.Bits.SL2_Y_OFST = 0;
		ptr_out_param->CAM_A_SL2F_SIZE.Bits.SL2_TPIPE_WD = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
			(unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_A_SL2F_SIZE.Bits.SL2_TPIPE_HT = dmx_v_size;
		ptr_out_param->CAM_B_SL2F_XOFF.Bits.SL2_X_OFST = (unsigned int)ptr_in_param->CAM_A_SL2F_RZ.Bits.SL2_HRZ_COMP*
			ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_B_SL2F_YOFF.Bits.SL2_Y_OFST = 0;
		ptr_out_param->CAM_B_SL2F_SIZE.Bits.SL2_TPIPE_WD = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_B_SL2F_SIZE.Bits.SL2_TPIPE_HT = dmx_v_size;
		if (dual_mode > 2)
		{
			ptr_out_param->CAM_C_SL2F_XOFF.Bits.SL2_X_OFST = (unsigned int)ptr_in_param->CAM_A_SL2F_RZ.Bits.SL2_HRZ_COMP*
				ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_C_SL2F_YOFF.Bits.SL2_Y_OFST = 0;
			ptr_out_param->CAM_C_SL2F_SIZE.Bits.SL2_TPIPE_WD = (unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X -
				(unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X + 1;
			ptr_out_param->CAM_C_SL2F_SIZE.Bits.SL2_TPIPE_HT = dmx_v_size;
		}
	}
	ptr_out_param->CAM_B_CTL_EN2.Bits.SL2F_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.SL2F_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN2.Bits.SL2F_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.SL2F_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_dbs(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* DBS_A_EN */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.DBS_EN)
	{
		if (ptr_in_param->CAM_A_DBS_CTL.Bits.DBS_LE_INV_CTL)
		{
		    result = DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		if (ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
		{
			/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
			ptr_out_param->CAM_B_DBS_CTL.Bits.DBS_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
		}
		else
		{
			ptr_out_param->CAM_B_DBS_CTL.Bits.DBS_LE_INV_CTL = 0;
		}
		if (dual_mode > 2)
		{
			if (ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
			{
				/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
				ptr_out_param->CAM_C_DBS_CTL.Bits.DBS_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
			}
			else
			{
				ptr_out_param->CAM_C_DBS_CTL.Bits.DBS_LE_INV_CTL = 0;
			}
		}
	}
	ptr_out_param->CAM_B_CTL_EN.Bits.DBS_EN = ptr_in_param->CAM_A_CTL_EN.Bits.DBS_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.DBS_EN = ptr_in_param->CAM_A_CTL_EN.Bits.DBS_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_adbs(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* ADBS_A_EN */
	if (ptr_in_param->CAM_A_CTL_EN2.Bits.ADBS_EN)
	{
		if (ptr_in_param->CAM_A_ADBS_CTL.Bits.ADBS_LE_INV_CTL)
		{
		    result = DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		if (ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
		{
			/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
			ptr_out_param->CAM_B_ADBS_CTL.Bits.ADBS_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
		}
		else
		{
			ptr_out_param->CAM_B_ADBS_CTL.Bits.ADBS_LE_INV_CTL = 0;
		}
		if (dual_mode > 2)
		{
			if (ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
			{
				/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
				ptr_out_param->CAM_C_ADBS_CTL.Bits.ADBS_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
			}
			else
			{
				ptr_out_param->CAM_C_ADBS_CTL.Bits.ADBS_LE_INV_CTL = 0;
			}
		}
	}
	ptr_out_param->CAM_B_CTL_EN2.Bits.ADBS_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.ADBS_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN2.Bits.ADBS_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.ADBS_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_obc(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* OB_D_EN */
	ptr_out_param->CAM_B_CTL_EN.Bits.OBC_EN = ptr_in_param->CAM_A_CTL_EN.Bits.OBC_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.OBC_EN = ptr_in_param->CAM_A_CTL_EN.Bits.OBC_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_rmg(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* RMG_D_EN */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.RMG_EN)
	{
		if (ptr_in_param->CAM_A_RMG_HDR_GAIN.Bits.RMG_LE_INV_CTL)
		{
		    result = DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		if (ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
		{
			/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
			ptr_out_param->CAM_B_RMG_HDR_GAIN.Bits.RMG_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
		}
		else
		{
			ptr_out_param->CAM_B_RMG_HDR_GAIN.Bits.RMG_LE_INV_CTL = 0;
		}
		if (dual_mode > 2)
		{
			if (ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
			{
				/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
				ptr_out_param->CAM_C_RMG_HDR_GAIN.Bits.RMG_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
			}
			else
			{
				ptr_out_param->CAM_C_RMG_HDR_GAIN.Bits.RMG_LE_INV_CTL = 0;
			}
		}
	}
	ptr_out_param->CAM_B_CTL_EN.Bits.RMG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RMG_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.RMG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RMG_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_rmm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* RMM_D_EN */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.RMM_EN)
	{
		if (ptr_in_param->CAM_A_RMM_OSC.Bits.RMM_LE_INV_CTL)
		{
		    result = DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		if (ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
		{
			/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
			ptr_out_param->CAM_B_RMM_OSC.Bits.RMM_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
		}
		else
		{
			ptr_out_param->CAM_B_RMM_OSC.Bits.RMM_LE_INV_CTL = 0;
		}
		if (dual_mode > 2)
		{
			if (ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
			{
				/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
				ptr_out_param->CAM_C_RMM_OSC.Bits.RMM_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
			}
			else
			{
				ptr_out_param->CAM_C_RMM_OSC.Bits.RMM_LE_INV_CTL = 0;
			}
		}
	}
	ptr_out_param->CAM_B_CTL_EN.Bits.RMM_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RMM_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.RMM_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RMM_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_bnr(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	if (ptr_in_param->CAM_A_CTL_EN.Bits.BNR_EN)
	{
		/*update size by bin enable */
		int bin_sel_h_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN)):
			((unsigned int)ptr_in_param->SW.TWIN_RAWI_XSIZE>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.SCM_EN));
		int dmx_v_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN)):((unsigned int)(ptr_in_param->CAM_UNI_RAWI_YSIZE.Bits.YSIZE +
			1)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN));
		if (ptr_in_param->CAM_A_BNR_BPC_CON.Bits.BNR_LE_INV_CTL)
		{
		    result = DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		if (ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
		{
			/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
			ptr_out_param->CAM_B_BNR_BPC_CON.Bits.BNR_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
		}
		else
		{
			ptr_out_param->CAM_B_BNR_BPC_CON.Bits.BNR_LE_INV_CTL = 0;
		}
		/* BPC */
		ptr_out_param->CAM_A_BNR_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_A_BNR_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
		ptr_out_param->CAM_A_BNR_BPC_TBLI2.Bits.BPC_XSIZE = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_A_BNR_BPC_TBLI2.Bits.BPC_YSIZE = dmx_v_size - 1;
		ptr_out_param->CAM_A_BNR_PDC_POS.Bits.PDC_XCENTER = (unsigned int)bin_sel_h_size>>1;
		ptr_out_param->CAM_A_BNR_PDC_POS.Bits.PDC_YCENTER = (unsigned int)dmx_v_size>>1;
		/* BPC_D */
		ptr_out_param->CAM_B_BNR_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_B_BNR_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
		ptr_out_param->CAM_B_BNR_BPC_TBLI2.Bits.BPC_XSIZE = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_B_BNR_BPC_TBLI2.Bits.BPC_YSIZE = dmx_v_size - 1;
		ptr_out_param->CAM_B_BNR_PDC_POS.Bits.PDC_XCENTER = (unsigned int)bin_sel_h_size>>1;
		ptr_out_param->CAM_B_BNR_PDC_POS.Bits.PDC_YCENTER = (unsigned int)dmx_v_size>>1;
		if (dual_mode > 2)
		{
			/* BPC_T */
			if (ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
			{
				/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
				ptr_out_param->CAM_C_BNR_BPC_CON.Bits.BNR_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
			}
			else
			{
				ptr_out_param->CAM_C_BNR_BPC_CON.Bits.BNR_LE_INV_CTL = 0;
			}
			/* BPC */
			ptr_out_param->CAM_C_BNR_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_C_BNR_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
			ptr_out_param->CAM_C_BNR_BPC_TBLI2.Bits.BPC_XSIZE = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X -
				(int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_C_BNR_BPC_TBLI2.Bits.BPC_YSIZE = dmx_v_size - 1;
			ptr_out_param->CAM_C_BNR_PDC_POS.Bits.PDC_XCENTER = (unsigned int)bin_sel_h_size>>1;
			ptr_out_param->CAM_C_BNR_PDC_POS.Bits.PDC_YCENTER = (unsigned int)dmx_v_size>>1;
		}
	}
	/* copy */
	ptr_out_param->CAM_B_CTL_EN.Bits.BNR_EN = ptr_in_param->CAM_A_CTL_EN.Bits.BNR_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.BNR_EN = ptr_in_param->CAM_A_CTL_EN.Bits.BNR_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_dcpn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
	if (ptr_in_param->CAM_A_CTL_EN2.Bits.DCPN_EN)
	{
		/*update size by bin enable */
		int dmx_v_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN)):((unsigned int)(ptr_in_param->CAM_UNI_RAWI_YSIZE.Bits.YSIZE +
			1)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN));
		ptr_out_param->CAM_A_DCPN_IN_IMG_SIZE.Bits.DCPN_IN_IMG_W = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
			(unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_A_DCPN_IN_IMG_SIZE.Bits.DCPN_IN_IMG_H = dmx_v_size;
		ptr_out_param->CAM_B_DCPN_IN_IMG_SIZE.Bits.DCPN_IN_IMG_W = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_B_DCPN_IN_IMG_SIZE.Bits.DCPN_IN_IMG_H = dmx_v_size;
		if (dual_mode > 2)
		{
			ptr_out_param->CAM_C_DCPN_IN_IMG_SIZE.Bits.DCPN_IN_IMG_W = (unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X -
				(unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X + 1;
			ptr_out_param->CAM_C_DCPN_IN_IMG_SIZE.Bits.DCPN_IN_IMG_H = dmx_v_size;
		}
	}
	ptr_out_param->CAM_B_CTL_EN2.Bits.DCPN_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.DCPN_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN2.Bits.DCPN_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.DCPN_EN;
	}
	return DUAL_MESSAGE_OK;
}

static DUAL_MESSAGE_ENUM dual_cal_lsc(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	if (ptr_in_param->CAM_A_CTL_EN.Bits.LSC_EN)
	{
		int blkx_start, blkx_end;
		int bit_per_pix = ptr_in_param->CAM_A_LSC_CTL1.Bits.LSC_EXTEND_COEF_MODE?768:512;
		/* LSC */
		blkx_start = (int)(ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X)/(2*(int)ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH);
		if (blkx_start >= ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
		{
			blkx_start = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL;
		}
		blkx_end = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X/(2*(int)ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) + 1;
		ptr_out_param->CAM_A_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
		if (blkx_end > ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
		{
			blkx_end = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL + 1;
			ptr_out_param->CAM_A_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->SW.TWIN_MODE_SDBLK_lWIDTH_ALL;
		}
		ptr_out_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_XNUM = blkx_end - blkx_start - 1;
		ptr_out_param->CAM_A_LSC_TPIPE_OFST.Bits.LSC_TPIPE_OFST_X =
			(unsigned int)((int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X -
			blkx_start*2*(int)ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH)>>1;
		ptr_out_param->CAM_A_LSC_TPIPE_SIZE.Bits.LSC_TPIPE_SIZE_X = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_A_LSCI_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)(blkx_start*bit_per_pix)>>3;
		ptr_out_param->CAM_A_LSCI_XSIZE.Bits.XSIZE = ((unsigned int)((blkx_end - blkx_start)*bit_per_pix)>>3) - 1;
		if ((int)ptr_out_param->CAM_A_LSCI_OFST_ADDR.Bits.OFFSET_ADDR + (int)ptr_out_param->CAM_A_LSCI_XSIZE.Bits.XSIZE + 1 >
			(int)ptr_in_param->CAM_A_LSCI_STRIDE.Bits.STRIDE)
		{
		    result = DUAL_MESSAGE_INVALID_LSCI_STRIDE_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		/* LSC_D */
		blkx_start = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X/(2*(int)ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH);
		if (blkx_start >= ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
		{
			blkx_start = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL;
		}
		blkx_end = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X/(2*(int)ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) + 1;
		ptr_out_param->CAM_B_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
		if (blkx_end > ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
		{
			blkx_end = (int)ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL + 1;
			ptr_out_param->CAM_B_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->SW.TWIN_MODE_SDBLK_lWIDTH_ALL;
		}
		ptr_out_param->CAM_B_LSC_CTL2.Bits.LSC_SDBLK_XNUM = blkx_end - blkx_start - 1;
		ptr_out_param->CAM_B_LSC_TPIPE_OFST.Bits.LSC_TPIPE_OFST_X =
			(unsigned int)((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X - blkx_start*2*(int)ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH)>>1;
		ptr_out_param->CAM_B_LSC_TPIPE_SIZE.Bits.LSC_TPIPE_SIZE_X = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_B_LSCI_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)(blkx_start*bit_per_pix)>>3;
		ptr_out_param->CAM_B_LSCI_XSIZE.Bits.XSIZE = ((unsigned int)((blkx_end - blkx_start)*bit_per_pix)>>3) - 1;
		if ((int)ptr_out_param->CAM_B_LSCI_OFST_ADDR.Bits.OFFSET_ADDR + (int)ptr_out_param->CAM_B_LSCI_XSIZE.Bits.XSIZE + 1 >
			(int)ptr_out_param->CAM_B_LSCI_STRIDE.Bits.STRIDE)
		{
		    result = DUAL_MESSAGE_INVALID_LSCI_STRIDE_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		if (dual_mode > 2)
		{
			/* LSC_T */
			blkx_start = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X/(2*(int)ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH);
			if (blkx_start >= ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
			{
				blkx_start = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL;
			}
			blkx_end = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X/(2*(int)ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) + 1;
			ptr_out_param->CAM_C_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
			if (blkx_end > ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
			{
				blkx_end = (int)ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL + 1;
				ptr_out_param->CAM_C_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->SW.TWIN_MODE_SDBLK_lWIDTH_ALL;
			}
			ptr_out_param->CAM_C_LSC_CTL2.Bits.LSC_SDBLK_XNUM = blkx_end - blkx_start - 1;
			ptr_out_param->CAM_C_LSC_TPIPE_OFST.Bits.LSC_TPIPE_OFST_X =
				(unsigned int)((int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X - blkx_start*2*(int)ptr_in_param->CAM_A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH)>>1;
			ptr_out_param->CAM_C_LSC_TPIPE_SIZE.Bits.LSC_TPIPE_SIZE_X = (int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X -
				(int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X + 1;
			ptr_out_param->CAM_C_LSCI_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)(blkx_start*bit_per_pix)>>3;
			ptr_out_param->CAM_C_LSCI_XSIZE.Bits.XSIZE = ((unsigned int)((blkx_end - blkx_start)*bit_per_pix)>>3) - 1;
			if ((int)ptr_out_param->CAM_C_LSCI_OFST_ADDR.Bits.OFFSET_ADDR + (int)ptr_out_param->CAM_C_LSCI_XSIZE.Bits.XSIZE + 1 >
				(int)ptr_out_param->CAM_C_LSCI_STRIDE.Bits.STRIDE)
			{
				result = DUAL_MESSAGE_INVALID_LSCI_STRIDE_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
	}
	/* assign common config, LSCI */
	ptr_out_param->CAM_B_CTL_EN.Bits.LSC_EN = ptr_in_param->CAM_A_CTL_EN.Bits.LSC_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.LSC_EN = ptr_in_param->CAM_A_CTL_EN.Bits.LSC_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_cpn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
	if (ptr_in_param->CAM_A_CTL_EN2.Bits.CPN_EN)
	{
		/*update size by bin enable */
		int dmx_v_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN)):((unsigned int)(ptr_in_param->CAM_UNI_RAWI_YSIZE.Bits.YSIZE +
			1)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_EN2.Bits.VBN_EN));
		ptr_out_param->CAM_A_CPN_IN_IMG_SIZE.Bits.CPN_IN_IMG_W = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
			(unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_A_CPN_IN_IMG_SIZE.Bits.CPN_IN_IMG_H = dmx_v_size;
		ptr_out_param->CAM_B_CPN_IN_IMG_SIZE.Bits.CPN_IN_IMG_W = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_B_CPN_IN_IMG_SIZE.Bits.CPN_IN_IMG_H = dmx_v_size;
		if (dual_mode > 2)
		{
			ptr_out_param->CAM_C_CPN_IN_IMG_SIZE.Bits.CPN_IN_IMG_W = (unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_END_X -
				(unsigned int)ptr_out_param->CAM_C_DMX_CROP.Bits.DMX_STR_X + 1;
			ptr_out_param->CAM_C_CPN_IN_IMG_SIZE.Bits.CPN_IN_IMG_H = dmx_v_size;
		}
	}
	ptr_out_param->CAM_B_CTL_EN2.Bits.CPN_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.CPN_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN2.Bits.CPN_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.CPN_EN;
	}
	return DUAL_MESSAGE_OK;
}

static DUAL_MESSAGE_ENUM dual_cal_rpg(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	ptr_out_param->CAM_B_CTL_EN.Bits.RPG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RPG_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN.Bits.RPG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RPG_EN;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_rmb(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* QBIN4_EN */
	if (ptr_in_param->CAM_A_CTL_EN2.Bits.RMB_EN)
	{
		if (ptr_in_param->CAM_A_RMB_MODE.Bits.ACC)
		{
			/* BMX */
			ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_STR_X = (unsigned int)ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_STR_X>>
				((int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC);
			ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X = (((unsigned int)ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X + 1)>>
				((int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC)) - 1;
			/* BMX_D */
			ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X = (unsigned int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X>>
				((int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC);
			ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X = (((unsigned int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X + 1)>>
				((int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC)) - 1;
			if (dual_mode > 2)
			{
				/* BMX_T */
				ptr_out_param->CAM_C_BMX_CROP.Bits.BMX_STR_X = (unsigned int)ptr_out_param->CAM_C_BMX_CROP.Bits.BMX_STR_X>>
					((int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC);
				ptr_out_param->CAM_C_BMX_CROP.Bits.BMX_END_X = (((unsigned int)ptr_out_param->CAM_C_BMX_CROP.Bits.BMX_END_X + 1)>>
					((int)ptr_in_param->CAM_A_RMB_MODE.Bits.ACC)) - 1;
			}
		}
	}
	ptr_out_param->CAM_B_CTL_EN2.Bits.RMB_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.RMB_EN;
	if (dual_mode > 2)
	{
		ptr_out_param->CAM_C_CTL_EN2.Bits.RMB_EN = ptr_in_param->CAM_A_CTL_EN2.Bits.RMB_EN;
	}
	return result;
}

bool dual_sel_check_support(int dual_sel)
{
	bool found_flag = false;
	DUAL_SEL_SUPPORT_LIST(DUAL_SEL_CHECK, dual_sel, found_flag,,,,);
	return found_flag;
}

static const char *dual_cal_print_error_message(DUAL_MESSAGE_ENUM n)
{
    TWIN_GET_ERROR_NAME(n);
}

static const char *dual_cal_get_current_file_name(const char *filename)
{
    char *ptr = strrchr((char *)filename, DUAL_FOLDER_SYMBOL_CHAR);
    if (NULL == ptr)
    {
        return filename;
    }
    else
    {
        return (const char *)(ptr + 1);
    }
}
