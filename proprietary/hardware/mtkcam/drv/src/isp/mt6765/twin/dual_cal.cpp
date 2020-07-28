#define LOG_TAG "DualCal"

#include "MyUtils.h"


/* define to replace macro */
#define print_error_message(...) dual_cal_print_error_message(__VA_ARGS__)
#define get_current_file_name(...) dual_cal_get_current_file_name(__VA_ARGS__)
#define UNUSED(expr) do { (void)(expr); } while (0)

/* func prototype */
static const char *dual_cal_print_error_message(DUAL_MESSAGE_ENUM n);
static const char *dual_cal_get_current_file_name(const char *filename);
static DUAL_MESSAGE_ENUM dual_cal_dmx_rrz_rmx(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_dmx_rrz_rmx_af_p2(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_dbn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_pbn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_bin(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_scm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_sgm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_sl2f(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_dbs(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_obc(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_rmg(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_rmm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_bnr(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_cac(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_lsc(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_hlr(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_sl2j(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_pmx_sel_0(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_pmx_sel_1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_rpg(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_qbin1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static DUAL_MESSAGE_ENUM dual_cal_qbin4(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
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
					result = dual_cal_cac(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_lsc(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_sl2j(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_hlr(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_pmx_sel_0(dual_mode, ptr_in_param, ptr_out_param);
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
					result = dual_cal_pbn(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_pmx_sel_1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					/* restore binning DMX real size finally */
					result = dual_cal_scm(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_qbin1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
					result = dual_cal_qbin4(dual_mode, ptr_in_param, ptr_out_param);
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
	/* check DMX */
	if ((0 == ptr_in_param->CAM_A_CTL_EN.Bits.DMX_EN) || (1 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL) ||
		(1 < ptr_in_param->CAM_B_CTL_SEL.Bits.DMX_SEL) || (2 != dual_mode))
	{
		result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}
	else
	{
		int dmx_mode_bf = 1;/* default 2 pixel mode, before bin, align dmx, final size */
		int dmx_cut_b = 1;/* default 2 pixel mode, before bin, align dmx, cut */
		int bin_mode_af = 1;/* default 2 pixel mode, after bin, align dmx, final size */
		int bin_cut_a = 1;/* default 2 pixel mode, after bin, align dmx, cut */
		int rcp_cut_a = 1;/* default 2 pixel mode, after bin, align imgo w/ rcp3, cut */
		int bmx_cut_a = 1;/* default 2 pixel mode, after bin, algin bmx, cut */
		int raw_wd[2] = {ptr_in_param->CAM_A_CTL_EN.Bits.CAC_EN?DUAL_CAC_WD:DUAL_RAW_WD,
			ptr_in_param->CAM_A_CTL_EN.Bits.CAC_EN?DUAL_CAC_WD_D:DUAL_RAW_WD_D};
		int dmx_valid[2];
		int dual_bits = 0x0;
		int bin_sel_h_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			((int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_E - (int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_S):
			((int)ptr_in_param->SW.TWIN_RAWI_XSIZE);
		/* select max of AF & RRZ loss */
		int left_loss_dmx = 0;
		int right_loss_dmx = 0;
		/* following to update */
		int left_loss_af = 0;
		int right_loss_af = 0;
		int left_margin_af = 0;
		int right_margin_af = 0;
		/* following to minimize */
		int left_dmx_max = 0;
		/* check height */
		if ((unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT &
			((1<<((int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN +
			((int)ptr_in_param->CAM_A_CTL_EN.Bits.PBN_EN?3:(int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN))) - 1))
		{
			result = DUAL_MESSAGE_DMX_IN_ALIGN_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* upddate FMT_SEL */
		ptr_out_param->CAM_B_CTL_FMT_SEL.Raw = ptr_in_param->CAM_A_CTL_FMT_SEL.Raw;
		ptr_out_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_DMXI = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXI;
		ptr_out_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_DMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO;
		ptr_out_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_BMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_BMXO;
		ptr_out_param->CAM_B_CTL_FMT_SEL.Bits.PIX_BUS_RMXO = ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_RMXO;
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
				bmx_cut_a = 2;/* 4 pixel mode */
				bin_mode_af = 2;
				dmx_mode_bf = 2;
			}
			if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.QBIN4_EN)
			{
				bmx_cut_a += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;/* 4 pixel mode */
				bin_mode_af += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;
				dmx_mode_bf += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;
				dmx_cut_b += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;/* w/o crop */
				bin_cut_a += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;/* w/o crop */
			}
		}
		if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.AMX_EN)
		{
			int bmx_cut_a_tmp = 1;
			int bin_mode_af_tmp = 1;
			int dmx_mode_bf_tmp = 1;
			int dmx_cut_b_tmp = 1;
			int bin_cut_a_tmp = 1;
			if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_BMXO >= 1)
			{
				result = DUAL_MESSAGE_INVALID_PIX_BUS_BMXO_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_BMXO >= 2)
			{
				bmx_cut_a_tmp = 2;/* 4 pixel mode */
				bin_mode_af_tmp = 2;
				dmx_mode_bf_tmp = 2;
			}
			if (ptr_in_param->CAM_A_CTL_EN.Bits.QBIN1_EN)
			{
				bmx_cut_a_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;/* 4 pixel mode */
				bin_mode_af_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;
				dmx_mode_bf_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;
				dmx_cut_b_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;
				bin_cut_a_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;/* w/o crop */
			}
			if (bmx_cut_a < bmx_cut_a_tmp)
			{
				bmx_cut_a = bmx_cut_a_tmp;
			}
			if (bin_mode_af < bin_mode_af_tmp)
			{
				bin_mode_af = bin_mode_af_tmp;
			}
			if (dmx_mode_bf < dmx_mode_bf_tmp)
			{
				dmx_mode_bf = dmx_mode_bf_tmp;
			}
			if (dmx_cut_b < dmx_cut_b_tmp)
			{
				dmx_cut_b = dmx_cut_b_tmp;/* w/o crop */
			}
			if (bin_cut_a < bin_cut_a_tmp)
			{
				bin_cut_a = bin_cut_a_tmp;/* w/o crop */
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
		if (ptr_in_param->CAM_A_CTL_EN.Bits.PBN_EN)
		{
			/* PBN in x16 align without PCP */
			if (0 == ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PCP_EN)
			{
				if (dmx_mode_bf < 4)
				{
					bin_mode_af += 4 - dmx_mode_bf;
					dmx_mode_bf = 4;
				}
				if (dmx_cut_b < 4)
				{
					bin_cut_a += 4 - dmx_cut_b;/* w/o crop */
					dmx_cut_b = 4;
				}
			}
			if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO < 1)
			{
				result = DUAL_MESSAGE_INVALID_PIX_BUS_DMXO_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)
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
			(int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN > (int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO)
		{
			result = DUAL_MESSAGE_INVALID_PIX_BUS_DMXO_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* final sync with PIX_BUS_DMXO */
		if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO >= 2)
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
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN);
		/* check after bin size, align with bin mode */
		if (bin_sel_h_size & (DUAL_ALIGN_PIXEL_MODE(bin_mode_af) - 1))
		{
			result = DUAL_MESSAGE_BMX_OUT_ALIGN_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* Error check */
        if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN && ((0 == ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL) ||
            ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL) && ((0 == ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN) ||
            (0 == ptr_in_param->CAM_A_CTL_EN.Bits.RCP3_EN)))))
		{
			result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* align RCP3 for IMGO, only sync for IMGO_A_XSIZE */
		if (ptr_in_param->CAM_A_CTL_EN.Bits.RCP3_EN && ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN &&
			ptr_in_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN && (2 == ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL))
		{
			if (DUAL_OUT_FMT_BAYER_8 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
			{
				if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
				{
					/* 32b, 8 bits raw 4n */
					if (rcp_cut_a < 2)
					{
						rcp_cut_a = 2;
					}
				}
			}
			else if (DUAL_OUT_FMT_BAYER_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
			{
				if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
				{
					/* 32b, 12 bits raw 8n */
					if (rcp_cut_a < 3)
					{
						rcp_cut_a = 3;
					}
				}
				else
				{
					/* 16b, 12 bits raw 4n */
					if (rcp_cut_a < 2)
					{
						rcp_cut_a = 2;
					}
				}
			}
			else if ((DUAL_OUT_FMT_BAYER_10 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT) ||
				(DUAL_OUT_FMT_BAYER_14 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT))
			{
				if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
				{
					/* 32b, 10 & 14 bits raw 16n */
					if (rcp_cut_a < 4)
					{
						rcp_cut_a = 4;
					}
				}
				else
				{
					/* 16b, 10 & 14 bits raw 8n */
					if (rcp_cut_a < 3)
					{
						rcp_cut_a = 3;
					}
				}
			}
			else if (DUAL_OUT_FMT_MIPI_10 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
			{
				/* 64b, MIPI 10 bits raw 32n */
				if (rcp_cut_a < 5)
				{
					rcp_cut_a = 5;
				}
			}
		}
		/* RTL verif or platform */
		if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
		{
			if (ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN)
			{
				left_loss_af = DUAL_AF_TAPS;
				right_loss_af = DUAL_AF_TAPS;
				left_margin_af = 0;
				/* AF FAST P1 */
				if (TWIN_SCENARIO_AF_FAST_P1 == ptr_in_param->SW.TWIN_SCENARIO)
				{
					right_margin_af = DUAL_AF_MAX_BLOCK_WIDTH - 2;
				}
				else
				{
					if ((int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH)
					{
						result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					right_margin_af = (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 2;
				}
			}
		}
		for (int i=0;i<4;i++)
		{
			int left_loss_check = 0;
			int right_loss_check = 0;
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
					break;
				case 1:/* af */
					left_loss_check = left_loss_af + left_margin_af;
					right_loss_check = right_loss_af + right_margin_af;
					break;
				case 2:/* bmx, amx */
					right_loss_check = DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 2;
					break;
				case 3:/* rrz */
					if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.RRZO_EN)
					{
						if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
						{
							if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
							{
								/* default 12b */
									right_loss_check = (((unsigned int)((((long long)(64 - 2)*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
										ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
										ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							}
							else
							{
									right_loss_check = (((unsigned int)((((long long)(128 - 2)*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
										ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
										ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							}
						}
						else
						{
							if (DUAL_OUT_FMT_FG_8 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
							{
								/* min RLB align */
									right_loss_check = (((unsigned int)(((long long)((ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?16:4) - 2)*
									(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
										(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
										ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							}
							else if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
							{
									right_loss_check = (((unsigned int)(((long long)((ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?32:4) - 2)*
									(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
										(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
										ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							}
							else
							{
								/* new 32b */
									right_loss_check = (((unsigned int)(((long long)((ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?32:8) - 2)*
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
									right_loss_check = (((unsigned int)((((long long)(4 - 2)*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
										ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
										ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							}
							else
							{
								right_loss_check = 2;
							}
						}
					}
					right_loss_check += DUAL_TILE_LOSS_RRZ;
					left_loss_check = DUAL_TILE_LOSS_RRZ_L;
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
		}
		/* padding left_loss & right_loss */
		left_loss_dmx += DUAL_TILE_LOSS_ALL_L;
		right_loss_dmx += DUAL_TILE_LOSS_ALL;
		/* update bin_cut_a for DMX to ensure HDR bit-true */
		if (ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
		{
			/* remarked for af fast mode */
			/* if (ptr_in_param->CAM_A_RMG_HDR_CFG.Bits.RMG_ZHDR_EN) */
			{
				/* DMX_A_STR, DMX_B_END, x4 after bin, x8 before bin */
				if (bin_cut_a < 2)
				{
					bin_cut_a = 2;/* w/o crop */
				}
			}
		}
		if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)
		{
			/* DMX_A_STR, DMX_B_END, x4 after scm, x8 before scm */
			if (bin_cut_a < 2)
			{
				bin_cut_a = 2;/* w/o crop */
			}
		}
		/* minus loss */
		dmx_valid[0] = raw_wd[0] - right_loss_dmx - (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 2);
		dmx_valid[1] = raw_wd[1] - left_loss_dmx - (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 2);
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
		for (int i=0;i<2;i++)
		{
			int bin_sel_h_temp = bin_sel_h_size;
			int even_count = 2 - i;
			bool found_flag = false;
			int even_size[2];
			for (int j=0;j<2;j++)
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
							dual_driver_printf("Error: non-averaged dmx size, %d, desired: %d > valid: %d\r\n", j, even_size[j], dmx_valid[j]);
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
				for (int j=0;j<2;j++)
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
		if (bin_sel_h_size || (dual_bits + 1 != (1 <<2)))
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
			ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X = dmx_valid[0] + right_loss_dmx - 1 +
				DUAL_ALIGN_PIXEL_MODE(bin_cut_a) -
				((dmx_valid[0] + right_loss_dmx) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1));
		}
		else
		{
			ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X = dmx_valid[0] + right_loss_dmx - 1;
		}
		if ((int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + raw_wd[0] <= (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X)
		{
			result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* check min size */
		left_dmx_max = dmx_valid[0] + DUAL_TILE_LOSS_ALL;
		if (ptr_in_param->CAM_A_CTL_EN.Bits.PBN_EN)
		{
			/* PBN in x16 align with PCP */
			if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PCP_EN)
			{
				if ((dmx_valid[0]*(1 + (int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN)*(1 + (int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)) & 0xF)
				{
					if (DUAL_TILE_LOSS_ALL*(1 + (int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN)*(1 + (int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN) +
						((dmx_valid[0]*(1 + (int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN)*(1 + (int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)) & 0xF) < 0x10)
					{
						left_dmx_max = dmx_valid[0] + (((0x10 - ((dmx_valid[0]*(1 + (int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN)*
							(1 + (int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)) & 0xF) + (1<<((int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN +
							(int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + 1)) - 1)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN +
							(int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + 1))<<1);
					}
				}
			}
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
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X = dmx_valid[0] + dmx_valid[1] - 1;
		if ((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + raw_wd[1] <=
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X)
		{
			result = DUAL_MESSAGE_DMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
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
			((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN);
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
		ptr_out_param->CAM_B_CTL_SEL.Bits.QBIN4_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.QBIN4_SEL;
		ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X = (int)ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X + 1 -
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_B_CTL_EN.Bits.BMX_EN = ptr_in_param->CAM_A_CTL_EN.Bits.BMX_EN;
		ptr_out_param->CAM_B_BMX_CTL.Bits.BMX_EDGE = 0xF;
		ptr_out_param->CAM_B_BMX_VSIZE.Bits.BMX_HT = ptr_out_param->CAM_A_BMX_VSIZE.Bits.BMX_HT;
		/* add BMX_B_SIG_MODE1 & 2 */
		ptr_out_param->CAM_B_BMX_CTL.Bits.BMX_SINGLE_MODE_1 = 1;
		ptr_out_param->CAM_B_BMX_CTL.Bits.BMX_SINGLE_MODE_2 = 0;
		ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		if (((int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X + 1 - (int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X) &
			(DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 1))
		{
			result = DUAL_MESSAGE_BMX_SIZE_CAL_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* AMX */
		ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_STR_X = 0;
		ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_END_X = ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X;
		ptr_out_param->CAM_A_AMX_CTL.Bits.AMX_EDGE = 0xF;
		ptr_out_param->CAM_A_AMX_VSIZE.Bits.AMX_HT = ptr_out_param->CAM_A_BMX_VSIZE.Bits.BMX_HT;
		/* add AMX_A_SIG_MODE1 & 2 */
		ptr_out_param->CAM_A_AMX_CTL.Bits.AMX_SINGLE_MODE_1 = 0;
		ptr_out_param->CAM_A_AMX_CTL.Bits.AMX_SINGLE_MODE_2 = 0;
		/* AMX_D */
		ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_STR_X = ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X;
		ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AMX_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.AMX_EN;
		ptr_out_param->CAM_B_AMX_CTL.Bits.AMX_EDGE = 0xF;
		ptr_out_param->CAM_B_AMX_VSIZE.Bits.AMX_HT = ptr_out_param->CAM_B_BMX_VSIZE.Bits.BMX_HT;
		/* add AMX_B_SIG_MODE1 & 2 */
		ptr_out_param->CAM_B_AMX_CTL.Bits.AMX_SINGLE_MODE_1 = 1;
		ptr_out_param->CAM_B_AMX_CTL.Bits.AMX_SINGLE_MODE_2 = 0;
		ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X = ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X;
		/* RCROP */
		ptr_out_param->CAM_B_CTL_EN.Bits.RCP_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RCP_EN;
		ptr_out_param->CAM_B_CTL_EN.Bits.SGG1_EN = ptr_in_param->CAM_A_CTL_EN.Bits.SGG1_EN;
		ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN;
		if (ptr_in_param->CAM_A_CTL_EN.Bits.RCP_EN)
		{
			/* RCROP */
			ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_STR_X = 0;
			ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = dmx_valid[0] + right_loss_af + right_margin_af - 1;
			ptr_out_param->CAM_A_RCP_CROP_CON2.Bits.RCP_STR_Y = 0;
			ptr_out_param->CAM_A_RCP_CROP_CON2.Bits.RCP_END_Y = ((unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
				((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN)) - 1;
			/* RCROP_D */
			ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X = dmx_valid[0] - left_loss_af  - left_margin_af -
				(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_B_RCP_CROP_CON2.Bits.RCP_STR_Y = 0;
			ptr_out_param->CAM_B_RCP_CROP_CON2.Bits.RCP_END_Y = ptr_out_param->CAM_A_RCP_CROP_CON2.Bits.RCP_END_Y;
			ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = dmx_valid[0] + dmx_valid[1] - 1 -
				(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
			/* AF & AFO */
			if (ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN)
			{
				if ((1 != ptr_in_param->CAM_A_CTL_SEL.Bits.SGG_SEL) || (1 != ptr_in_param->CAM_B_CTL_SEL.Bits.SGG_SEL) ||
					(0 == ptr_in_param->CAM_A_CTL_EN.Bits.SGG1_EN) || ((0 == ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SGG5_EN) &&
					ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN))
				{
					result = DUAL_MESSAGE_INVALID_AF_SGG_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				ptr_out_param->CAM_B_CTL_SEL.Bits.RCP_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RCP_SEL;
				ptr_out_param->CAM_B_CTL_SEL.Bits.SGG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.SGG_SEL;
				/* AF & AF_D */
				if (((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < 32) ||
					((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X - (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X + 1 < 32))
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				/* RTL verif or platform */
				if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
				{
					ptr_out_param->CAM_B_SGG1_PGN.Raw = ptr_in_param->CAM_A_SGG1_PGN.Raw;
					ptr_out_param->CAM_B_SGG1_GMRC_1.Raw = ptr_in_param->CAM_A_SGG1_GMRC_1.Raw;
					ptr_out_param->CAM_B_SGG1_GMRC_2.Raw = ptr_in_param->CAM_A_SGG1_GMRC_2.Raw;
								/* AFO size will be possibly different */
					ptr_out_param->CAM_B_CTL_DMA_EN.Bits.SGG5_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SGG5_EN;
					ptr_out_param->CAM_B_SGG5_PGN.Raw = ptr_in_param->CAM_A_SGG5_PGN.Raw;
					ptr_out_param->CAM_B_SGG5_GMRC_1.Raw = ptr_in_param->CAM_A_SGG5_GMRC_1.Raw;
					ptr_out_param->CAM_B_SGG5_GMRC_2.Raw = ptr_in_param->CAM_A_SGG5_GMRC_2.Raw;
					/* normal */
					if (TWIN_SCENARIO_NORMAL == ptr_in_param->SW.TWIN_SCENARIO)
					{
						/* platform */
						if ((ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE != ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE) ||
							((int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH) ||
							((dmx_valid[0] + dmx_valid[1]) < ptr_in_param->SW.TWIN_AF_OFFSET +
							(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM) ||
							(ptr_in_param->SW.TWIN_AF_BLOCK_XNUM <= 0))
						{
							result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
						/* check AF_A no output */
						if ((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < ptr_in_param->SW.TWIN_AF_OFFSET +
							(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE + right_loss_af)
						{
							ptr_out_param->CAM_A_CTL_EN.Bits.RCP_EN = false;
							ptr_out_param->CAM_A_CTL_EN.Bits.SGG1_EN = false;
							ptr_out_param->CAM_A_CTL_DMA_EN.Bits.SGG5_EN = false;
							ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN = false;
							ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN = false;
						}
						else
						{
							ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET;
							/* check AF_A output all */
							if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
								ptr_in_param->SW.TWIN_AF_OFFSET <= dmx_valid[0] + right_margin_af)
							{
								ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
							}
							else
							{
								ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = (dmx_valid[0] + right_margin_af - ptr_in_param->SW.TWIN_AF_OFFSET)/
								(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
							}
							/* reduce RCP size */
							ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART + right_loss_af +
								(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
							/* protect AF min size */
							if ((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < 32)
							{
								ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = 32 - 1;
							}
						}
					}
				}
				else
				{
					ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= 0;
					if (dmx_valid[0] < (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_in_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM)
					{
						ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = dmx_valid[0]/(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
					}
					/* reduce RCP size */
					ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X =
						(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
					/* protect AF min size */
					if ((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < 32)
					{
						ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = 32 - 1;
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
						/* AF_B output all */
						if (dmx_valid[0] + right_margin_af < ptr_in_param->SW.TWIN_AF_OFFSET + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
						{
							ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET - (dmx_valid[0] - left_loss_af - left_margin_af);
							ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
						}
						else
						{
							/* AF_B with output */
							if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM > (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM)
							{
								ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
									 (dmx_valid[0] - left_loss_af - left_margin_af);
								ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM -
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM;
							}
							else
							{
								ptr_out_param->CAM_B_CTL_EN.Bits.RCP_EN = false;
								ptr_out_param->CAM_B_CTL_EN.Bits.SGG1_EN = false;
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.SGG5_EN = false;
								ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = false;
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = false;
							}
						}
						ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_YSTART = ptr_in_param->CAM_A_AF_VLD.Bits.AF_VLD_YSTART;
						ptr_out_param->CAM_B_AF_CON.Raw = ptr_in_param->CAM_A_AF_CON.Raw;
						ptr_out_param->CAM_B_AF_TH_0.Raw = ptr_in_param->CAM_A_AF_TH_0.Raw;
						ptr_out_param->CAM_B_AF_TH_1.Raw = ptr_in_param->CAM_A_AF_TH_1.Raw;
						ptr_out_param->CAM_B_AF_FLT_1.Raw = ptr_in_param->CAM_A_AF_FLT_1.Raw;
						ptr_out_param->CAM_B_AF_FLT_2.Raw = ptr_in_param->CAM_A_AF_FLT_2.Raw;
						ptr_out_param->CAM_B_AF_FLT_3.Raw = ptr_in_param->CAM_A_AF_FLT_3.Raw;
						ptr_out_param->CAM_B_AF_FLT_4.Raw = ptr_in_param->CAM_A_AF_FLT_4.Raw;
						ptr_out_param->CAM_B_AF_FLT_5.Raw = ptr_in_param->CAM_A_AF_FLT_5.Raw;
						ptr_out_param->CAM_B_AF_FLT_6.Raw = ptr_in_param->CAM_A_AF_FLT_6.Raw;
						ptr_out_param->CAM_B_AF_FLT_7.Raw = ptr_in_param->CAM_A_AF_FLT_7.Raw;
						ptr_out_param->CAM_B_AF_FLT_8.Raw = ptr_in_param->CAM_A_AF_FLT_8.Raw;
						ptr_out_param->CAM_B_AF_TH_2.Raw = ptr_in_param->CAM_A_AF_TH_2.Raw;
								ptr_out_param->CAM_B_AF_BLK_0.Raw = ptr_in_param->CAM_A_AF_BLK_0.Raw;
						ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_YNUM = ptr_in_param->CAM_A_AF_BLK_1.Bits.AF_BLK_YNUM;
					}
					else
					{
						ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= 0;
						if ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X - (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X +
							1 < (int)ptr_in_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_in_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM)
						{
							ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X -
								(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X + 1)/(int)ptr_in_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE;
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
					if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
					{
						if (ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE < 16)
						{
							result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
						if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
						{
							ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
							ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*16*
								((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
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
										(unsigned int)ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR)>>(4 + (int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN);
									ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1)*
										(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM - 1;
								}
							}
						}
						if (ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN)
						{
							ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*16*
								((int)ptr_out_param->CAM_B_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
							if (ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE < 16)
							{
								result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
							/* RTL verif or platform */
							if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
							{
								ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR = ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?
									((int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1)):0;
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
										(unsigned int)ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR)>>(4 + (int)ptr_out_param->CAM_B_AF_CON.Bits.AF_EXT_STAT_EN);
									ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_out_param->CAM_B_AF_CON.Bits.AF_EXT_STAT_EN + 1)*
										(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM  - 1;
								}
							}
							if ((ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?(ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1):0) +
								ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE + 1 > ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
							{
								if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
								{
									result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
									dual_driver_printf("Error: %s\n", print_error_message(result));
									return result;
								}
							}
						}
						/* AFO_A padding */
						if ((false == ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN) &&
							(ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR))
						{
							if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) <=
								(int)ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE)
							{
								ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= 0;
								ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
								ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR = 16*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*
									((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1);
								ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
								ptr_out_param->CAM_A_CTL_EN.Bits.RCP_EN = true;
								ptr_out_param->CAM_A_CTL_EN.Bits.SGG1_EN = true;
								ptr_out_param->CAM_A_CTL_DMA_EN.Bits.SGG5_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SGG5_EN;
								ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN = true;
								ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN = true;
								ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_STR_X = 0;
								ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
								/* protect AF min size */
								if ((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < 32)
								{
									ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = 32 - 1;
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
							if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) <=
								(int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
							{
								ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= 0;
								ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
								ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1;
								ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
								ptr_out_param->CAM_B_CTL_EN.Bits.RCP_EN = true;
								ptr_out_param->CAM_B_CTL_EN.Bits.SGG1_EN = true;
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.SGG5_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SGG5_EN;
								ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = true;
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = true;
								ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X = 0;
								ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
								/* protect AF min size */
								if ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X + 1 < 32)
								{
									ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = 32 - 1;
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
					}
					/* sync RCP size & AF_IMAGE_WD */
					ptr_out_param->CAM_A_AF_SIZE.Bits.AF_IMAGE_WD = (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1;
					ptr_out_param->CAM_B_AF_SIZE.Bits.AF_IMAGE_WD = (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X -
						(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X + 1;
				}
				/* minimize dmx */
				if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
				{
					if (left_dmx_max < (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 + DUAL_TILE_LOSS_ALL)
					{
							left_dmx_max = (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 + DUAL_TILE_LOSS_ALL;
					}
				}
			}
		}
		/* RCP3 & IMGO, only 1 pixel mode */
		if (ptr_in_param->CAM_A_CTL_EN.Bits.RCP3_EN && ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN &&
			ptr_in_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN && (2 == ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL) &&
			(0 == ptr_in_param->CAM_A_CTL_SEL.Bits.UFE_SEL))
		{
			int bit_per_pix = 8;
			int bus_width_shift = 1;/* 2*8, 16 bits */
			int dmx_shift = 0;
			ptr_out_param->CAM_B_CTL_SEL.Bits.UFE_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.UFE_SEL;
			ptr_out_param->CAM_B_CTL_SEL.Bits.IMG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL;
			ptr_out_param->CAM_B_CTL_SEL.Bits.RCP3_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL;
			/* IMGO before BIN with 2 pixel mode */
			if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
			{
				dmx_shift = 1;
				bus_width_shift = 2;/* 4*8, 32 bits */
			}
			switch (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
			{
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
				(int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X + 1)*bit_per_pix)>>3) -1;
			if (((int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X -
				(int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X + 1) & (DUAL_ALIGN_PIXEL_MODE(rcp_cut_a) - 1))
			{
				result = DUAL_MESSAGE_RCP3_SIZE_CAL_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			if (((unsigned int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1) &  ((1 << bus_width_shift) - 1))
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
			ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X = ((((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1) << dmx_shift) - 1) -
				((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X << dmx_shift);
			/* IMGO_D */
			ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = ((unsigned int)(((int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X -
				(int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X + 1)*bit_per_pix + (1<<(bus_width_shift + 3)) - 1)>>(bus_width_shift + 3))*
				(1<<bus_width_shift) - 1;
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
			/* RTL */
			if (ptr_in_param->CAM_A_IMGO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_IMGO_BASE_ADDR.Bits.BASE_ADDR)
			{
				ptr_out_param->CAM_B_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = ptr_in_param->CAM_A_IMGO_OFST_ADDR.Bits.OFFSET_ADDR;
				/* A no output */
				if ((int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift) >= (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1)
				{
					/* no A */
					ptr_out_param->CAM_A_CTL_EN.Bits.RCP3_EN = 0;
					ptr_out_param->CAM_A_CTL_EN.Bits.CPG_EN = 0;
					ptr_out_param->CAM_A_CTL_EN.Bits.PAK_EN = 0;
					ptr_out_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN = 0;
					/* B */
					ptr_out_param->CAM_B_IMGO_CROP.Bits.XOFFSET = (int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET -
						(((unsigned int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1)>>bus_width_shift);
					ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE;
				}
				else
				{
					/* B no output */
					if ((int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift) + (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE <=
						(int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE)
					{
						/* no B */
						ptr_out_param->CAM_B_CTL_EN.Bits.RCP3_EN = 0;
						ptr_out_param->CAM_B_CTL_EN.Bits.CPG_EN = 0;
						ptr_out_param->CAM_B_CTL_EN.Bits.PAK_EN = 0;
						ptr_out_param->CAM_B_CTL_DMA_EN.Bits.IMGO_EN = 0;
						/* A */
						ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE = ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE;
					}
					else
					{
						/* A, B */
						ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE -= (int)ptr_in_param->CAM_A_IMGO_CROP.Bits.XOFFSET*(1<<bus_width_shift);
						ptr_out_param->CAM_B_IMGO_CROP.Bits.XOFFSET = 0;
						ptr_out_param->CAM_B_IMGO_OFST_ADDR.Bits.OFFSET_ADDR += (int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1;
						ptr_out_param->CAM_B_IMGO_XSIZE.Bits.XSIZE = (int)ptr_in_param->CAM_A_IMGO_XSIZE.Bits.XSIZE -
							((int)ptr_out_param->CAM_A_IMGO_XSIZE.Bits.XSIZE + 1);
					}
				}
			}
			else
			{
				ptr_out_param->CAM_A_IMGO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
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
			}
			/* pixel mode */
			if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
			{
				ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X = (unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_STR_X>>1;
				ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X = (((unsigned int)ptr_out_param->CAM_A_RCP3_CROP_CON1.Bits.RCP_END_X + 1)>>1) - 1;
				ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X = (unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_STR_X>>1;
				ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X = (((unsigned int)ptr_out_param->CAM_B_RCP3_CROP_CON1.Bits.RCP_END_X + 1)>>1) - 1;
			}
		}
		/* RRZ & RMX */
		ptr_out_param->CAM_B_CTL_EN.Bits.RRZ_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RRZ_EN;
		if (ptr_in_param->CAM_A_CTL_EN.Bits.RRZ_EN)
		{
			/*update size by bin enable */
			int rrz_in_w = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
				(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_E -
				(unsigned int)ptr_in_param-> CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
				(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)):
				((unsigned int)ptr_in_param->SW.TWIN_RAWI_XSIZE>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
				(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN));
			int out_xe=0;
			int out_shift[2] = {0, 0};/* 2<<out_shift[i], [0]: RRZ_A align, [1]: RRZ_B align, default align x2 */
			ptr_out_param->CAM_B_CTL_SEL.Bits.UFEG_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL;
			ptr_out_param->CAM_B_CTL_SEL.Bits.HDS1_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.HDS1_SEL;
			if (0 == ptr_in_param->CAM_UNI_TOP_MOD_EN.Bits.RLB_A_EN)
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
			/* check rrz_a alignment */
			if (ptr_in_param->CAM_B_CTL_DMA_EN.Bits.RRZO_EN)
			{
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
			/* check rrz_a/b alignment, minmum x4 for RLB monitor */
			if (out_shift[0] < 1)
			{
				out_shift[0] = 1;/* 2<<1, x4 */
			}
			out_shift[1] = 1;/* x4 */
			/* copy from RRZ_IN_HT & RRZ_D_IN_HT from DMX & DMX_D */
			ptr_out_param->CAM_A_RRZ_IN_IMG.Bits.RRZ_IN_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
			((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN);
			if ((rrz_in_w < dual_mode*4) || (ptr_in_param->SW.TWIN_RRZ_OUT_WD < dual_mode*4) ||
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
				}
			}
			/* copy vertical step */
			ptr_out_param->CAM_B_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = ptr_out_param->CAM_A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
			ptr_out_param->CAM_B_RRZ_IN_IMG.Bits.RRZ_IN_HT = ptr_out_param->CAM_A_RRZ_IN_IMG.Bits.RRZ_IN_HT;
			/* configure single_mode */
			ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 0;
			ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
			ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;/* disable slave */
			ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
			ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_EDGE = 0xF;
			ptr_out_param->CAM_A_RMX_VSIZE.Bits.RMX_HT = ptr_in_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
			ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_EDGE = 0xF;
			ptr_out_param->CAM_B_RMX_VSIZE.Bits.RMX_HT = ptr_in_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
			/* loop dual mode from right */
			for (int i=0;i<2;i++)
			{
				int in_xs_dmx=0;
				int in_xe_rrz=0;
				int in_xs=0;
				int in_xe=0;
				int out_xs= 0;
				/* config forward backward pos */
				switch (i)
				{
					case 0:
						/* left */
						ptr_out_param->CAM_B_RRZ_IN_IMG.Bits.RRZ_IN_WD = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
							(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1;
						/* init pos */
						in_xs_dmx = ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
						in_xe_rrz = ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X;
						/* right boundary */
						in_xs = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + DUAL_TILE_LOSS_ALL_L;
						in_xe = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X;
						out_xe= ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1;
						break;
					case 1:
						/* right */
						ptr_out_param->CAM_A_RRZ_IN_IMG.Bits.RRZ_IN_WD = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
							(int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1;
						/* init pos */
						in_xs_dmx = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X;
						in_xe_rrz = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X - DUAL_TILE_LOSS_ALL;
						out_xs = 0;
						in_xe = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X - DUAL_TILE_LOSS_ALL;
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
							long long start_temp = (long long)(in_xs + DUAL_TILE_LOSS_RRZ_L)*DUAL_RRZ_PREC -
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
							/* check RRZ buffer size, must x4 align */
							ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_in_param->SW.TWIN_RRZ_OUT_WD -
								(unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 3)>>2)<<2;
							if (((((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 3)>>2)<<2) +
								(unsigned int)ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST > DUAL_RRZ_WD_ALL)
							{
								result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
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
							/* check RRZ buffer size, must x4 align */
							if (((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 3)>>2)<<2) >
								(unsigned int)ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST)
							{
								result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
							else
							{
								ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
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
							ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;/* disable RRZ_B */
							ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = DUAL_RRZ_ALIGN(out_shift[1]);
							ptr_out_param->CAM_B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
							ptr_out_param->CAM_B_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X = DUAL_RRZ_MASK(out_shift[1]);
							/* check RRZ buffer size, must x4 align */
							ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_in_param->SW.TWIN_RRZ_OUT_WD + 3)>>2)<<2;
							if (((((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 3)>>2)<<2) +
								(unsigned int)ptr_out_param->CAM_B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST > DUAL_RRZ_WD_ALL)
							{
								result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
							break;
						case 1:
							ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 1;/* disable RRZ_A */
							ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = DUAL_RRZ_ALIGN(out_shift[0]);
							ptr_out_param->CAM_A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
							ptr_out_param->CAM_A_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_STR_X = 0;
							ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X = DUAL_RRZ_MASK(out_shift[0]);
							/* check RRZ buffer size, must x4 align */
							ptr_out_param->CAM_A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_in_param->SW.TWIN_RRZ_OUT_WD + 3)>>2)<<2;
							if (((((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 3)>>2)<<2) +
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
				/* check RRZ_B disable */
				if (ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
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
				/* check bus width */
				if (((unsigned int)ptr_out_param->CAM_B_RRZO_XSIZE.Bits.XSIZE + 1) & ((1<<bus_width_shift) - 1))
				{
					result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
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
				/* RTL */
				if (ptr_in_param->CAM_A_RRZO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_RRZO_BASE_ADDR.Bits.BASE_ADDR)
				{
					/* check slave disable */
					if (1 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
					{
						/* slave disable, RRZO_A only */
						ptr_out_param->CAM_B_CTL_EN.Bits.RRZ_EN = 0;
						ptr_out_param->CAM_B_CTL_DMA_EN.Bits.RRZO_EN = 0;
						ptr_out_param->CAM_B_CTL_EN.Bits.PAKG_EN = 0;
					}
					else if (1 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
					{
						/* master disable, RRZO_B only */
						ptr_out_param->CAM_A_CTL_EN.Bits.RRZ_EN = 0;
						ptr_out_param->CAM_A_CTL_DMA_EN.Bits.RRZO_EN = 0;
						ptr_out_param->CAM_A_CTL_EN.Bits.PAKG_EN = 0;
					}
					else
					{
						ptr_out_param->CAM_B_RRZO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_RRZO_XSIZE.Bits.XSIZE + 1;
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
				}
				if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
				{
					ptr_out_param->CAM_A_UFEO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
					ptr_out_param->CAM_B_UFEO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
					if (ptr_in_param->CAM_A_UFEO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_UFEO_BASE_ADDR.Bits.BASE_ADDR)
					{
						if (ptr_in_param->CAM_A_CTL_EN.Bits.UFEG_EN && ptr_in_param->CAM_A_CTL_DMA_EN.Bits.UFEO_EN)
						{
							/* UFEO, test mode, size not x8 */
							ptr_out_param->CAM_B_UFEO_CON.Raw = ptr_in_param->CAM_A_UFEO_CON.Raw;
							ptr_out_param->CAM_B_UFEO_CON2.Raw = ptr_in_param->CAM_A_UFEO_CON2.Raw;
							ptr_out_param->CAM_B_UFEO_CON3.Raw = ptr_in_param->CAM_A_UFEO_CON3.Raw;
							ptr_out_param->CAM_B_UFEO_CON4.Raw = ptr_in_param->CAM_A_UFEO_CON4.Raw;
							ptr_out_param->CAM_B_UFEO_STRIDE.Raw = ptr_in_param->CAM_A_UFEO_STRIDE.Raw;
							ptr_out_param->CAM_B_UFEO_YSIZE.Bits.YSIZE = ptr_in_param->CAM_A_UFEO_YSIZE.Bits.YSIZE;
							ptr_out_param->CAM_B_UFE_CON.Raw = ptr_in_param->CAM_A_UFE_CON.Raw;
							if (1 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1)
							{
								/* UFEO_A only */
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.UFEO_EN = 0;
								ptr_out_param->CAM_B_CTL_EN.Bits.UFEG_EN = 0;
								ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
							}
							else if (1 == ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2)
							{
								/* UFEO_B only */
								ptr_out_param->CAM_A_CTL_DMA_EN.Bits.UFEO_EN = 0;
								ptr_out_param->CAM_A_CTL_EN.Bits.UFEG_EN = 0;
								ptr_out_param->CAM_B_UFEO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
							}
							else
							{
								/* UFEO, test mode, size not x8 */
								ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
								ptr_out_param->CAM_B_UFE_CON.Bits.UFE_FORCE_PCM = ptr_in_param->CAM_B_UFE_CON.Bits.UFE_FORCE_PCM;
								ptr_out_param->CAM_B_UFEO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
								ptr_out_param->CAM_B_UFEO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE + 1;
							}
						}
						else
						{
							if (ptr_in_param->CAM_A_CTL_EN.Bits.UFEG_EN || ptr_in_param->CAM_A_CTL_DMA_EN.Bits.UFEO_EN)
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
						ptr_out_param->CAM_A_UFEO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
						ptr_out_param->CAM_B_UFEO_XSIZE.Bits.XSIZE = (((unsigned int)ptr_out_param->CAM_B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63)>>6) - 1;
					}
				}
			}
		}
		/* RMX */
		ptr_out_param->CAM_B_CTL_EN.Bits.RMX_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RMX_EN;
		ptr_out_param->CAM_B_CTL_DMA_EN.Bits.SGG2_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SGG2_EN;
		ptr_out_param->CAM_B_CTL_DMA_EN.Bits.GSE_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.GSE_EN;
		if (ptr_in_param->CAM_A_CTL_EN.Bits.RMX_EN)
		{
			if (0 == ptr_in_param->CAM_A_CTL_EN.Bits.RRZ_EN)
			{
				/* RMX bypass to reduce RAW A RMX_END_X */
				ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_STR_X = 0;
				ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 0;
				ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
				ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 1;
				ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
				if ((ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_RMXO > 1) && (dmx_valid[0] & 0x3))
				{
					ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X = dmx_valid[0] + 3 - (dmx_valid[0] & 0x3);
				}
				else
				{
					ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X = dmx_valid[0] - 1;
				}
				ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X = (int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1 -
					(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
				ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
					(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
				ptr_out_param->CAM_A_RMX_CTL.Bits.RMX_EDGE = 0xF;
				ptr_out_param->CAM_A_RMX_VSIZE.Bits.RMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
					((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN);
				ptr_out_param->CAM_B_RMX_CTL.Bits.RMX_EDGE = 0xF;
				ptr_out_param->CAM_B_RMX_VSIZE.Bits.RMX_HT = ptr_out_param->CAM_A_RMX_VSIZE.Bits.RMX_HT;
				/* check min size */
				if (left_dmx_max < (int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1 + DUAL_TILE_LOSS_ALL)
				{
					left_dmx_max = (int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1 + DUAL_TILE_LOSS_ALL;
				}
			}
			/* check RMX size */
			if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_RMXO < 2)
			{
				if ((ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_STR_X & 0x1) ||
					(((int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1) & 0x1) ||
					(ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X & 0x1) ||
					(((int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X + 1) & 0x1))
				{
					result = DUAL_MESSAGE_RMX_SIZE_CAL_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
			else
			{
				if ((ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_STR_X & 0x3) ||
					(((int)ptr_out_param->CAM_A_RMX_CROP.Bits.RMX_END_X + 1) & 0x3) ||
					(ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X & 0x1) ||
					(((int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X + 1) & 0x1) ||
					(((int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_END_X - (int)ptr_out_param->CAM_B_RMX_CROP.Bits.RMX_STR_X + 1) & 0x3))
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
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_dmx_rrz_rmx_af_p2(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param,
									  DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* check DMX */
	if ((0 == ptr_in_param->CAM_A_CTL_EN.Bits.DMX_EN) || (1 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL) ||
		(1 < ptr_in_param->CAM_B_CTL_SEL.Bits.DMX_SEL) || (2 != dual_mode))
	{
		result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}
	else
	{
		int dmx_mode_bf = 1;/* default 2 pixel mode, before bin, align dmx, final size */
		int dmx_cut_b = 1;/* default 2 pixel mode, before bin, align dmx, cut */
		int bin_mode_af = 1;/* default 2 pixel mode, after bin, align dmx, final size */
		int bin_cut_a = 1;/* default 2 pixel mode, after bin, align dmx, cut */
		int rcp_cut_a = 1;/* default 2 pixel mode, after bin, align imgo w/ rcp3, cut */
		int bmx_cut_a = 1;/* default 2 pixel mode, after bin, algin bmx, cut */
		int raw_wd[2] = {ptr_in_param->CAM_A_CTL_EN.Bits.CAC_EN?DUAL_CAC_WD:DUAL_RAW_WD,
			ptr_in_param->CAM_A_CTL_EN.Bits.CAC_EN?DUAL_CAC_WD_D:DUAL_RAW_WD_D};
		int dmx_valid[2];
		int dual_bits = 0x0;
		int bin_sel_h_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			((int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_E - (int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_S):
			((int)ptr_in_param->SW.TWIN_RAWI_XSIZE);
		/* select max of AF & RRZ loss */
		int left_loss_dmx = 0;
		int right_loss_dmx = 0;
		/* following to update */
		int left_loss_af = 0;
		int right_loss_af = 0;
		int left_margin_af = 0;
		int right_margin_af = 0;
		int dmx_b_str_x = 0;
		/* upddate FMT_SEL */
		/* update bmx pixel mode */
		if (ptr_in_param->CAM_A_CTL_EN.Bits.BMX_EN)
		{
			if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_BMXO >= 2)
			{
				bmx_cut_a = 2;/* 4 pixel mode */
				bin_mode_af = 2;
				dmx_mode_bf = 2;
			}
			if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.QBIN4_EN)
			{
				bmx_cut_a += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;/* 4 pixel mode */
				bin_mode_af += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;
				dmx_mode_bf += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;
				dmx_cut_b += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;/* w/o crop */
				bin_cut_a += (int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC;/* w/o crop */
			}
		}
		if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.AMX_EN)
		{
			int bmx_cut_a_tmp = 1;
			int bin_mode_af_tmp = 1;
			int dmx_mode_bf_tmp = 1;
			int dmx_cut_b_tmp = 1;
			int bin_cut_a_tmp = 1;
			if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_BMXO >= 2)
			{
				bmx_cut_a_tmp = 2;/* 4 pixel mode */
				bin_mode_af_tmp = 2;
				dmx_mode_bf_tmp = 2;
			}
			if (ptr_in_param->CAM_A_CTL_EN.Bits.QBIN1_EN)
			{
				bmx_cut_a_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;/* 4 pixel mode */
				bin_mode_af_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;
				dmx_mode_bf_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;
				dmx_cut_b_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;
				bin_cut_a_tmp += (int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC;/* w/o crop */
			}
			if (bmx_cut_a < bmx_cut_a_tmp)
			{
				bmx_cut_a = bmx_cut_a_tmp;
			}
			if (bin_mode_af < bin_mode_af_tmp)
			{
				bin_mode_af = bin_mode_af_tmp;
			}
			if (dmx_mode_bf < dmx_mode_bf_tmp)
			{
				dmx_mode_bf = dmx_mode_bf_tmp;
			}
			if (dmx_cut_b < dmx_cut_b_tmp)
			{
				dmx_cut_b = dmx_cut_b_tmp;/* w/o crop */
			}
			if (bin_cut_a < bin_cut_a_tmp)
			{
				bin_cut_a = bin_cut_a_tmp;/* w/o crop */
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
		if (ptr_in_param->CAM_A_CTL_EN.Bits.PBN_EN)
		{
			/* PBN in x16 align without PCP */
			if (0 == ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PCP_EN)
			{
				if (dmx_mode_bf < 4)
				{
					bin_mode_af += 4 - dmx_mode_bf;
					dmx_mode_bf = 4;
				}
				if (dmx_cut_b < 4)
				{
					bin_cut_a += 4 - dmx_cut_b;/* w/o crop */
					dmx_cut_b = 4;
				}
			}
		}
		if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)
		{
			dmx_mode_bf += 1;
			dmx_cut_b += 1;
		}
		/* check PIX_BUS_DMXO */
		/* final sync with PIX_BUS_DMXO */
		if (ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO >= 2)
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
		/* shift size by binning modules enable */
		bin_sel_h_size = (unsigned int)bin_sel_h_size>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN);
		/* check after bin size, align with bin mode */
		/* Error check */
		/* align RCP3 for IMGO, only sync for IMGO_A_XSIZE */
		if (ptr_in_param->CAM_A_CTL_EN.Bits.RCP3_EN && ptr_in_param->CAM_A_CTL_EN.Bits.PAK_EN &&
			ptr_in_param->CAM_A_CTL_DMA_EN.Bits.IMGO_EN && (2 == ptr_in_param->CAM_A_CTL_SEL.Bits.IMG_SEL))
		{
			if (DUAL_OUT_FMT_BAYER_8 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
			{
				if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
				{
					/* 32b, 8 bits raw 4n */
					if (rcp_cut_a < 2)
					{
						rcp_cut_a = 2;
					}
				}
			}
			else if (DUAL_OUT_FMT_BAYER_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT)
			{
				if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
				{
					/* 32b, 12 bits raw 8n */
					if (rcp_cut_a < 3)
					{
						rcp_cut_a = 3;
					}
				}
				else
				{
					/* 16b, 12 bits raw 4n */
					if (rcp_cut_a < 2)
					{
						rcp_cut_a = 2;
					}
				}
			}
			else if ((DUAL_OUT_FMT_BAYER_10 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT) ||
				(DUAL_OUT_FMT_BAYER_14 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.IMGO_FMT))
			{
				if ((2 == ptr_in_param->CAM_A_CTL_SEL.Bits.RCP3_SEL) && ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
				{
					/* 32b, 10 & 14 bits raw 16n */
					if (rcp_cut_a < 4)
					{
						rcp_cut_a = 4;
					}
				}
				else
				{
					/* 16b, 10 & 14 bits raw 8n */
					if (rcp_cut_a < 3)
					{
						rcp_cut_a = 3;
					}
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
		/* RTL verif or platform */
		if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
		{
			if (ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN)
			{
				left_loss_af = DUAL_AF_TAPS;
				right_loss_af = DUAL_AF_TAPS;
				left_margin_af = 0;
				right_margin_af = DUAL_AF_MAX_BLOCK_WIDTH - 2;
				if ((int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
		}
		for (int i=0;i<4;i++)
		{
			int left_loss_check = 0;
			int right_loss_check = 0;
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
					break;
				case 1:/* af */
					left_loss_check = left_loss_af + left_margin_af;
					right_loss_check = right_loss_af + right_margin_af;
					break;
				case 2:/* bmx, amx */
					right_loss_check = DUAL_ALIGN_PIXEL_MODE(bmx_cut_a) - 2;
					break;
				case 3:/* rrz */
					if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.RRZO_EN)
					{
						if (ptr_in_param->CAM_A_CTL_SEL.Bits.UFEG_SEL)
						{
							if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
							{
								/* default 12b */
									right_loss_check = (((unsigned int)((((long long)(64 - 2)*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
										ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
										ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							}
							else
							{
									right_loss_check = (((unsigned int)((((long long)(128 - 2)*(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
										ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) +
										ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							}
						}
						else
						{
							if (DUAL_OUT_FMT_FG_8 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
							{
								/* min RLB align */
									right_loss_check = (((unsigned int)(((long long)((ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?16:4) - 2)*
									(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
										(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
										ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							}
							else if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FMT)
							{
									right_loss_check = (((unsigned int)(((long long)((ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?32:4) - 2)*
									(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
										(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
										ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1)>>1)<<1;
							}
							else
							{
								/* new 32b */
									right_loss_check = (((unsigned int)(((long long)((ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.RRZO_FG_MODE?32:8) - 2)*
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
									right_loss_check = (((unsigned int)((((long long)(4 - 2)*
										(bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
										(ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST?1:0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1)/
										ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1)>>1)<<1;
							}
							else
							{
								right_loss_check = 2;
							}
						}
					}
					right_loss_check += DUAL_TILE_LOSS_RRZ;
					left_loss_check = DUAL_TILE_LOSS_RRZ_L;
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
		}
		/* padding left_loss & right_loss */
		left_loss_dmx += DUAL_TILE_LOSS_ALL_L;
		right_loss_dmx += DUAL_TILE_LOSS_ALL;
		/* update bin_cut_a for DMX to ensure HDR bit-true */
		if (ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN)
		{
			/* remarked for af fast mode */
			/* if (ptr_in_param->CAM_A_RMG_HDR_CFG.Bits.RMG_ZHDR_EN) */
			{
				/* DMX_A_STR, DMX_B_END, x4 after bin, x8 before bin */
				if (bin_cut_a < 2)
				{
					bin_cut_a = 2;/* w/o crop */
				}
			}
		}
		if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)
		{
			/* DMX_A_STR, DMX_B_END, x4 after scm, x8 before scm */
			if (bin_cut_a < 2)
			{
				bin_cut_a = 2;/* w/o crop */
			}
		}
		/* minus loss, AF fast mode */
		dmx_valid[0] = raw_wd[0] - right_loss_dmx - (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 2);
		dmx_valid[1] = raw_wd[1] - left_loss_dmx - (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 2);
		/* with af offset */
		if (ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN)
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
		/* dispatch dmx size */
		for (int i=0;i<2;i++)
		{
			int bin_sel_h_temp = bin_sel_h_size;
			int even_count = 2 - i;
			bool found_flag = false;
			int even_size[2] = {0,0};
			for (int j=0;j<2;j++)
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
							dual_driver_printf("Error: non-averaged dmx size, %d, desired: %d > valid: %d\r\n", j, even_size[j], dmx_valid[j]);
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
				for (int j=0;j<2;j++)
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
		if (bin_sel_h_size || (dual_bits + 1 != (1 <<2)))
		{
			result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* DMX */
		/* DMX_D */
		if ((dmx_valid[0] - left_loss_dmx) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1))
		{
			dmx_b_str_x = dmx_valid[0] - left_loss_dmx -
				((dmx_valid[0] - left_loss_dmx) & (DUAL_ALIGN_PIXEL_MODE(bin_cut_a) - 1));
		}
		else
		{
			dmx_b_str_x = dmx_valid[0] - left_loss_dmx;
		}
		/* BMX */
		/* add BMX_A_SIG_MODE1 & 2 */
		/* AMX */
		/* add AMX_A_SIG_MODE1 & 2 */
		/* AMX_D */
		/* add AMX_B_SIG_MODE1 & 2 */
		/* RCROP */
		ptr_out_param->CAM_A_CTL_EN.Raw = ptr_in_param->CAM_A_CTL_EN.Raw;
		ptr_out_param->CAM_A_CTL_DMA_EN.Raw = ptr_in_param->CAM_A_CTL_DMA_EN.Raw;
		ptr_out_param->CAM_A_AF_SIZE.Raw = ptr_in_param->CAM_A_AF_SIZE.Raw;
		ptr_out_param->CAM_A_AF_BLK_1.Raw = ptr_in_param->CAM_A_AF_BLK_1.Raw;
		ptr_out_param->CAM_A_AF_VLD.Raw = ptr_in_param->CAM_A_AF_VLD.Raw;
		ptr_out_param->CAM_A_AFO_OFST_ADDR.Raw = ptr_in_param->CAM_A_AFO_OFST_ADDR.Raw;
		ptr_out_param->CAM_A_AFO_XSIZE.Raw = ptr_in_param->CAM_A_AFO_XSIZE.Raw;
		ptr_out_param->CAM_B_CTL_EN.Raw = ptr_in_param->CAM_B_CTL_EN.Raw;
		ptr_out_param->CAM_B_CTL_DMA_EN.Raw = ptr_in_param->CAM_B_CTL_DMA_EN.Raw;
		ptr_out_param->CAM_B_AF_SIZE.Raw = ptr_in_param->CAM_B_AF_SIZE.Raw;
		ptr_out_param->CAM_B_AF_VLD.Raw = ptr_in_param->CAM_B_AF_VLD.Raw;
		ptr_out_param->CAM_B_AF_CON.Raw = ptr_in_param->CAM_B_AF_CON.Raw;
		ptr_out_param->CAM_B_AF_TH_0.Raw = ptr_in_param->CAM_B_AF_TH_0.Raw;
		ptr_out_param->CAM_B_AF_TH_1.Raw = ptr_in_param->CAM_B_AF_TH_1.Raw;
		ptr_out_param->CAM_B_AF_FLT_1.Raw = ptr_in_param->CAM_B_AF_FLT_1.Raw;
		ptr_out_param->CAM_B_AF_FLT_2.Raw = ptr_in_param->CAM_B_AF_FLT_2.Raw;
		ptr_out_param->CAM_B_AF_FLT_3.Raw = ptr_in_param->CAM_B_AF_FLT_3.Raw;
		ptr_out_param->CAM_B_AF_FLT_4.Raw = ptr_in_param->CAM_B_AF_FLT_4.Raw;
		ptr_out_param->CAM_B_AF_FLT_5.Raw = ptr_in_param->CAM_B_AF_FLT_5.Raw;
		ptr_out_param->CAM_B_AF_FLT_6.Raw = ptr_in_param->CAM_B_AF_FLT_6.Raw;
		ptr_out_param->CAM_B_AF_FLT_7.Raw = ptr_in_param->CAM_B_AF_FLT_7.Raw;
		ptr_out_param->CAM_B_AF_FLT_8.Raw = ptr_in_param->CAM_B_AF_FLT_8.Raw;
		ptr_out_param->CAM_B_AF_TH_1.Raw = ptr_in_param->CAM_B_AF_TH_1.Raw;
		ptr_out_param->CAM_B_AF_BLK_0.Raw = ptr_in_param->CAM_B_AF_BLK_0.Raw;
		ptr_out_param->CAM_B_AF_BLK_1.Raw = ptr_in_param->CAM_B_AF_BLK_1.Raw;
		ptr_out_param->CAM_B_AFO_OFST_ADDR.Raw = ptr_in_param->CAM_B_AFO_OFST_ADDR.Raw;
		ptr_out_param->CAM_B_AFO_XSIZE.Raw = ptr_in_param->CAM_B_AFO_XSIZE.Raw;
		ptr_out_param->CAM_B_AFO_YSIZE.Raw = ptr_in_param->CAM_B_AFO_YSIZE.Raw;
		ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN;
		if (ptr_in_param->CAM_A_CTL_EN.Bits.RCP_EN)
		{
			/* RCROP */
			ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_STR_X = 0;
			ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X = dmx_valid[0] + right_loss_af + right_margin_af - 1;
			/* RCROP_D */
				ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X = dmx_valid[0] - left_loss_af  - left_margin_af - dmx_b_str_x;
				ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X = dmx_valid[0] + dmx_valid[1] - 1 - dmx_b_str_x;
			/* AF & AFO */
			if (ptr_in_param->CAM_A_CTL_EN.Bits.AF_EN)
			{
				/* AF & AF_D */
				if (((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < 32) ||
					((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X - (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X + 1 < 32))
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				/* RTL verif or platform */
				if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
				{
						/* AFO size will be possibly different */
					/* cal valid af config*/
					if ((ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE & 0x1) || (ptr_in_param->SW.TWIN_AF_OFFSET & 0x1))
					{
						result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					/* platform */
					if ((ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE != ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE) ||
						((int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH) ||
						((dmx_valid[0] + dmx_valid[1]) < ptr_in_param->SW.TWIN_AF_OFFSET +
												(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM) ||
												(ptr_in_param->SW.TWIN_AF_BLOCK_XNUM <= 0))
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					/* check AF_A no output */
					if ((int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1 < ptr_in_param->SW.TWIN_AF_OFFSET +
						(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE + right_loss_af)
					{
						ptr_out_param->CAM_A_CTL_EN.Bits.AF_EN = false;
						ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN = false;
					}
					else
					{
						ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET;
						/* check AF_A output all */
						if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE +
							ptr_in_param->SW.TWIN_AF_OFFSET <= dmx_valid[0] + right_margin_af)
						{
							ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
						}
						else
						{
							ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = (dmx_valid[0] + right_margin_af - ptr_in_param->SW.TWIN_AF_OFFSET)/
							(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE;
						}
					}
				}
				else
				{
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
					/* AF_B output all */
					if (dmx_valid[0] + right_margin_af < ptr_in_param->SW.TWIN_AF_OFFSET + (int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE)
					{
						ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET - (dmx_valid[0] - left_loss_af - left_margin_af);
						ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
					}
					else
					{
						/* AF_B with output */
						if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM > (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM)
						{
							ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= ptr_in_param->SW.TWIN_AF_OFFSET +
								(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*(int)ptr_in_param->CAM_A_AF_BLK_0.Bits.AF_BLK_XSIZE -
								 (dmx_valid[0] - left_loss_af - left_margin_af);
							ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM -
								(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM;
						}
						else
						{
							ptr_out_param->CAM_B_CTL_EN.Bits.AF_EN = false;
							ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN = false;
						}
					}
					ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_YSTART = ptr_in_param->CAM_A_AF_VLD.Bits.AF_VLD_YSTART;
					ptr_out_param->CAM_B_AF_CON.Raw = ptr_in_param->CAM_A_AF_CON.Raw;
					ptr_out_param->CAM_B_AF_TH_0.Raw = ptr_in_param->CAM_A_AF_TH_0.Raw;
					ptr_out_param->CAM_B_AF_TH_1.Raw = ptr_in_param->CAM_A_AF_TH_1.Raw;
					ptr_out_param->CAM_B_AF_FLT_1.Raw = ptr_in_param->CAM_A_AF_FLT_1.Raw;
					ptr_out_param->CAM_B_AF_FLT_2.Raw = ptr_in_param->CAM_A_AF_FLT_2.Raw;
					ptr_out_param->CAM_B_AF_FLT_3.Raw = ptr_in_param->CAM_A_AF_FLT_3.Raw;
					ptr_out_param->CAM_B_AF_FLT_4.Raw = ptr_in_param->CAM_A_AF_FLT_4.Raw;
					ptr_out_param->CAM_B_AF_FLT_5.Raw = ptr_in_param->CAM_A_AF_FLT_5.Raw;
					ptr_out_param->CAM_B_AF_FLT_6.Raw = ptr_in_param->CAM_A_AF_FLT_6.Raw;
					ptr_out_param->CAM_B_AF_FLT_7.Raw = ptr_in_param->CAM_A_AF_FLT_7.Raw;
					ptr_out_param->CAM_B_AF_FLT_8.Raw = ptr_in_param->CAM_A_AF_FLT_8.Raw;
					ptr_out_param->CAM_B_AF_TH_2.Raw = ptr_in_param->CAM_A_AF_TH_2.Raw;
							ptr_out_param->CAM_B_AF_BLK_0.Raw = ptr_in_param->CAM_A_AF_BLK_0.Raw;
					ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_YNUM = ptr_in_param->CAM_A_AF_BLK_1.Bits.AF_BLK_YNUM;
				}
				else
				{
					ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= 0;
					if ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X - (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X +
						1 < (int)ptr_in_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE*(int)ptr_in_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM)
					{
						ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = ((int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X -
							(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X + 1)/(int)ptr_in_param->CAM_B_AF_BLK_0.Bits.AF_BLK_XSIZE;
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
				if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
				{
					if (ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE < 16)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN)
					{
						ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
						ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*16*
							((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
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
									(unsigned int)ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR)>>(4 + (int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN);
								ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1)*
									(int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM - 1;
							}
						}
					}
					if (ptr_out_param->CAM_B_CTL_DMA_EN.Bits.AFO_EN)
					{
						ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM*16*
							((int)ptr_out_param->CAM_B_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
						if (ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE < 16)
						{
							result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
							dual_driver_printf("Error: %s\n", print_error_message(result));
							return result;
						}
						/* RTL verif or platform */
						if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
						{
							ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR = ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?
								((int)ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM*16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1)):0;
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
									(unsigned int)ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR)>>(4 + (int)ptr_out_param->CAM_B_AF_CON.Bits.AF_EXT_STAT_EN);
								ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_out_param->CAM_B_AF_CON.Bits.AF_EXT_STAT_EN + 1)*
									(int)ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM  - 1;
							}
						}
						if ((ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN?(ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1):0) +
							ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE + 1 > ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
						{
							if (ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR)
							{
								result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
								dual_driver_printf("Error: %s\n", print_error_message(result));
								return result;
							}
						}
					}
					/* AFO_A padding */
					if ((false == ptr_out_param->CAM_A_CTL_DMA_EN.Bits.AFO_EN) &&
						(ptr_in_param->CAM_A_AFO_BASE_ADDR.Bits.BASE_ADDR == ptr_in_param->CAM_B_AFO_BASE_ADDR.Bits.BASE_ADDR))
					{
						if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) <=
							(int)ptr_in_param->CAM_A_AFO_STRIDE.Bits.STRIDE)
						{
							ptr_out_param->CAM_A_AF_VLD.Bits.AF_VLD_XSTART= 0;
							ptr_out_param->CAM_A_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
							ptr_out_param->CAM_A_AFO_OFST_ADDR.Bits.OFFSET_ADDR = 16*ptr_in_param->SW.TWIN_AF_BLOCK_XNUM*
								((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1);
							ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
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
						if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)*16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) <=
							(int)ptr_in_param->CAM_B_AFO_STRIDE.Bits.STRIDE)
						{
							ptr_out_param->CAM_B_AF_VLD.Bits.AF_VLD_XSTART= 0;
							ptr_out_param->CAM_B_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
							ptr_out_param->CAM_B_AFO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_AFO_XSIZE.Bits.XSIZE + 1;
							ptr_out_param->CAM_B_AFO_XSIZE.Bits.XSIZE = 16*((int)ptr_in_param->CAM_A_AF_CON.Bits.AF_EXT_STAT_EN + 1) - 1;
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
				}
				/* sync RCP size & AF_IMAGE_WD */
				ptr_out_param->CAM_A_AF_SIZE.Bits.AF_IMAGE_WD = (int)ptr_out_param->CAM_A_RCP_CROP_CON1.Bits.RCP_END_X + 1;
				ptr_out_param->CAM_B_AF_SIZE.Bits.AF_IMAGE_WD = (int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_END_X -
					(int)ptr_out_param->CAM_B_RCP_CROP_CON1.Bits.RCP_STR_X + 1;
			}
		}
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_dbn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	/* dbn x2 DMX */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN)
	{
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X<<1;
		ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X<<1;
		ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)<<1) - 1;
	}
	/* BIN_D_EN */
	ptr_out_param->CAM_B_CTL_EN.Bits.DBN_EN = ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN;
	ptr_out_param->CAM_B_CTL_DMA_EN.Bits.VBN_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_pbn(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	if (ptr_in_param->CAM_A_CTL_EN.Bits.PBN_EN)
	{
		ptr_out_param->CAM_A_PBN_VSIZE.Bits.PBN_VSIZE = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
			(unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN;
		ptr_out_param->CAM_B_PBN_VSIZE.Bits.PBN_VSIZE = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
			(unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN;
	}
	/* PBN_D_EN */
	ptr_out_param->CAM_B_CTL_EN.Bits.PBN_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PBN_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_bin(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

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
	}
	/* BIN_D_EN */
	ptr_out_param->CAM_B_CTL_EN.Bits.BIN_EN = ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_scm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	/* scm x2 DMX */
	if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)
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
	}
	/* SCM_D_EN */
	ptr_out_param->CAM_B_CTL_DMA_EN.Bits.SCM_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_sgm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	/* OB_D_EN */
	ptr_out_param->CAM_B_CTL_EN.Bits.SGM_EN = ptr_in_param->CAM_A_CTL_EN.Bits.SGM_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_sl2f(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	/* SL2F_A_EN */
	if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SL2F_EN)
	{
		int dmx_v_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN)):((unsigned int)(ptr_in_param->CAM_UNI_RAWI_YSIZE.Bits.YSIZE +
			1)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN));
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
	}
	ptr_out_param->CAM_B_CTL_DMA_EN.Bits.SL2F_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SL2F_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_sl2j(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	/* SL2J_A_EN */
	if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SL2J_EN)
	{
		int dmx_v_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN)):((unsigned int)(ptr_in_param->CAM_UNI_RAWI_YSIZE.Bits.YSIZE +
			1)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN));
		if ((0 == ptr_in_param->CAM_A_SL2J_RZ.Bits.SL2_HRZ_COMP) || (0 == ptr_in_param->CAM_A_SL2J_RZ.Bits.SL2_VRZ_COMP))
		{
		    result = DUAL_MESSAGE_ZERO_SL_HVRZ_COMP_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		ptr_out_param->CAM_A_SL2J_XOFF.Bits.SL2_X_OFST = (unsigned int)ptr_in_param->CAM_A_SL2J_RZ.Bits.SL2_HRZ_COMP*
			ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_A_SL2J_YOFF.Bits.SL2_Y_OFST = 0;
		ptr_out_param->CAM_A_SL2J_SIZE.Bits.SL2_TPIPE_WD = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
			(unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_A_SL2J_SIZE.Bits.SL2_TPIPE_HT = dmx_v_size;
		ptr_out_param->CAM_B_SL2J_XOFF.Bits.SL2_X_OFST = (unsigned int)ptr_in_param->CAM_A_SL2J_RZ.Bits.SL2_HRZ_COMP*
			ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		ptr_out_param->CAM_B_SL2J_YOFF.Bits.SL2_Y_OFST = 0;
		ptr_out_param->CAM_B_SL2J_SIZE.Bits.SL2_TPIPE_WD = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1;
		ptr_out_param->CAM_B_SL2J_SIZE.Bits.SL2_TPIPE_HT = dmx_v_size;
	}
	ptr_out_param->CAM_B_CTL_DMA_EN.Bits.SL2J_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SL2J_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_dbs(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

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
	}
	ptr_out_param->CAM_B_CTL_EN.Bits.DBS_EN = ptr_in_param->CAM_A_CTL_EN.Bits.DBS_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_obc(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	/* OB_D_EN */
	ptr_out_param->CAM_B_CTL_EN.Bits.OBC_EN = ptr_in_param->CAM_A_CTL_EN.Bits.OBC_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_rmg(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

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
	}
	ptr_out_param->CAM_B_CTL_EN.Bits.RMG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RMG_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_rmm(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

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
	}
	ptr_out_param->CAM_B_CTL_EN.Bits.RMM_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RMM_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_bnr(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	if (ptr_in_param->CAM_A_CTL_EN.Bits.BNR_EN)
	{
		/*update size by bin enable */
		int bin_sel_h_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)):
			((unsigned int)ptr_in_param->SW.TWIN_RAWI_XSIZE>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN));
		int dmx_v_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_LIN.Bits.LIN_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN)):((unsigned int)(ptr_in_param->CAM_UNI_RAWI_YSIZE.Bits.YSIZE +
			1)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN));
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
	}
	/* copy */
	ptr_out_param->CAM_B_CTL_EN.Bits.BNR_EN = ptr_in_param->CAM_A_CTL_EN.Bits.BNR_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_cac(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	if (ptr_in_param->CAM_A_CTL_EN.Bits.CAC_EN)
	{
		/*update size by bin enable */
		int bin_sel_h_size = (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.DMX_SEL)?
			(((unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_E -
			(unsigned int)ptr_in_param->CAM_A_TG_SEN_GRAB_PXL.Bits.PXL_S)>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)):
			((unsigned int)ptr_in_param->SW.TWIN_RAWI_XSIZE>>((int)ptr_in_param->CAM_A_CTL_EN.Bits.BIN_EN +
			(int)ptr_in_param->CAM_A_CTL_EN.Bits.DBN_EN + (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN));
		int caci_x_off_d = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>7;
		int caci_x_size = ((int)(bin_sel_h_size + 128)>>7) + 1;
		/* CACI */
		ptr_out_param->CAM_A_CACI_OFST_ADDR.Bits.OFFSET_ADDR= 0;
		ptr_out_param->CAM_A_CACI_XSIZE.Bits.XSIZE= 4*((((int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X +
			128)>>7) + 1) - 1;
		/* CACI_D */
		ptr_out_param->CAM_B_CACI_OFST_ADDR.Bits.OFFSET_ADDR= 4*caci_x_off_d;
		ptr_out_param->CAM_B_CACI_XSIZE.Bits.XSIZE= 4*(caci_x_size - caci_x_off_d) - 1;
		/* CAC */
		ptr_out_param->CAM_A_CAC_TILE_OFFSET.Bits.CAC_OFFSET_X = 0;
		ptr_out_param->CAM_A_CAC_TILE_SIZE.Bits.CAC_TILE_WIDTH = (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1;
		/* CAC_D */
		ptr_out_param->CAM_B_CAC_TILE_OFFSET.Bits.CAC_OFFSET_X = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X -
			(((int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>7)<<7);
		ptr_out_param->CAM_B_CAC_TILE_SIZE.Bits.CAC_TILE_WIDTH = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
			(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1;
	}
	/* copy */
	ptr_out_param->CAM_B_CTL_EN.Bits.CAC_EN = ptr_in_param->CAM_A_CTL_EN.Bits.CAC_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_lsc(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

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
	}
	/* assign common config, LSCI */
	ptr_out_param->CAM_B_CTL_EN.Bits.LSC_EN = ptr_in_param->CAM_A_CTL_EN.Bits.LSC_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_hlr(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	/* HLR_D_EN */
	ptr_out_param->CAM_B_CTL_DMA_EN.Bits.HLR_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.HLR_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_pmx_sel_0(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	if (0 == ptr_in_param->CAM_A_CTL_SEL.Bits.PMX_SEL)
	{
		ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PCP_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PCP_EN;
		ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PSB_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PSB_EN;
		if (ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN)
		{
			/* PMX */
			ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X;
			/* PMX_D */
			ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X = 0;
			ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
				(int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
		}
		else
		{
			/* PMX */
			ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X;
			/* PMX_D */
			ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X = ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X;
			ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X =  ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X;
		}
		if ((0 == ptr_in_param->CAM_A_CTL_EN.Bits.PMX_EN) && ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PDO_EN)
		{
			result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
		/* PMX */
		ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_STR_X = 0;
		ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_EDGE = ptr_out_param->CAM_A_BMX_CTL.Bits.BMX_EDGE;
		ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT = ptr_out_param->CAM_A_BMX_VSIZE.Bits.BMX_HT;
		ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_SINGLE_MODE_1 = 1;
		ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_SINGLE_MODE_2 = 0;
		/* PMX_D */
		ptr_out_param->CAM_B_PMX_CTL.Bits.PMX_EDGE = ptr_out_param->CAM_B_BMX_CTL.Bits.BMX_EDGE;
		ptr_out_param->CAM_B_PMX_VSIZE.Bits.PMX_HT = ptr_out_param->CAM_B_BMX_VSIZE.Bits.BMX_HT;
		ptr_out_param->CAM_B_PMX_CTL.Bits.PMX_SINGLE_MODE_1 = 1;
		ptr_out_param->CAM_B_PMX_CTL.Bits.PMX_SINGLE_MODE_2 = 0;
		/* PDO */
		ptr_out_param->CAM_A_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
		if ((int)ptr_in_param->CAM_A_PDO_BASE_ADDR.Bits.BASE_ADDR == (int)ptr_in_param->CAM_B_PDO_BASE_ADDR.Bits.BASE_ADDR)
		{
			ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE = 16*((int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>5) - 1;
			ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE + 1;
			ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE = ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE;
			ptr_out_param->CAM_B_PDO_CON.Raw = ptr_in_param->CAM_A_PDO_CON.Raw;
			ptr_out_param->CAM_B_PDO_CON2.Raw = ptr_in_param->CAM_A_PDO_CON2.Raw;
			ptr_out_param->CAM_B_PDO_CON3.Raw = ptr_in_param->CAM_A_PDO_CON3.Raw;
			ptr_out_param->CAM_B_PDO_STRIDE.Raw = ptr_in_param->CAM_A_PDO_STRIDE.Raw;
		}
		else
		{
			/* RTL test PDO without crop */
			ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
			ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE = 2*(int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X + 1;
			ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE = 2*((int)ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X -
				(int)ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X) + 1;
		}
		if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PDO_EN)
		{
			/* Stride check */
			if ((int)ptr_out_param->CAM_A_PDO_OFST_ADDR.Bits.OFFSET_ADDR + (int)ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE +
				1 > (int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE)
			{
				result = DUAL_MESSAGE_INVALID_PDO_STRIDE_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		if (ptr_in_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN)
		{
			ptr_out_param->CAM_B_CTL_EN.Bits.PKP_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PKP_EN;
			if ((int)ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR + (int)ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE +
				1 > (int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE)
			{
				result = DUAL_MESSAGE_INVALID_PDO_STRIDE_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		else
		{
			/* PDO merge by PMX */
			if (ptr_in_param->CAM_A_CTL_EN.Bits.PMX_EN)
			{
				ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE = 16*((int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>4) - 1;
				ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_SINGLE_MODE_1 = 0;
				ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_SINGLE_MODE_2 = 0;
			}
		}
		ptr_out_param->CAM_B_CTL_EN.Bits.PMX_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PMX_EN;
		ptr_out_param->CAM_B_CTL_SEL.Bits.PMX_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.PMX_SEL;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_pmx_sel_1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	if (1 == ptr_in_param->CAM_A_CTL_SEL.Bits.PMX_SEL)
	{
		ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PCP_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PCP_EN;
		ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PSB_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PSB_EN;
		if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PSB_EN)
		{
			if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PCP_EN)
			{
				/* pixel scale */
				int PCP_STR_X = (unsigned int)ptr_in_param->CAM_A_PCP_CROP_CON1.Bits.PCP_STR_X<<((int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO -
					(int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN);
				int PCP_END_X = (((unsigned int)ptr_in_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1)<<((int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO -
					(int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)) - 1;
				/* PCP check */
				if ((PCP_STR_X > PCP_END_X) || (PCP_END_X > (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X) ||
					(2*(int)ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_WD > PCP_END_X - PCP_STR_X + 1) || (ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_WD & 0x1) ||
					(PCP_STR_X & 0x3) || (ptr_in_param->CAM_A_PCP_CROP_CON2.Bits.PCP_STR_Y & 0x1))
				{
					result = DUAL_MESSAGE_PSB_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_STR_X = PCP_STR_X;
				/* PCROP */
				if ((int)PCP_END_X < (int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X)
				{
					/* PSB_A */
					ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X;
					ptr_out_param->CAM_A_PSB_SIZE.Bits.PSB_WD = ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_WD;
					/* PSB_B disabled */
					ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = 0;
					ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
						(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
					ptr_out_param->CAM_B_PSB_SIZE.Bits.PSB_WD = ((unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X + 1 -
						(unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X)>>1;
					if ((unsigned int)ptr_in_param->CAM_A_PDO_BASE_ADDR.Bits.BASE_ADDR == (unsigned int)ptr_in_param->CAM_B_PDO_BASE_ADDR.Bits.BASE_ADDR)
					{
						ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PCP_EN = 0;
						ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PSB_EN = 0;
						ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN = 0;
					}
				}
				else
				{
					if ((unsigned int)PCP_STR_X > (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X)
					{
						/* PSB_A disbaled */
						ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X;
						ptr_out_param->CAM_A_PSB_SIZE.Bits.PSB_WD = ((unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1 -
							(unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_STR_X)>>1;
						if ((unsigned int)ptr_in_param->CAM_A_PDO_BASE_ADDR.Bits.BASE_ADDR == (unsigned int)ptr_in_param->CAM_B_PDO_BASE_ADDR.Bits.BASE_ADDR)
						{
							ptr_out_param->CAM_A_CTL_DMA_EN.Bits.PCP_EN = 0;
							ptr_out_param->CAM_A_CTL_DMA_EN.Bits.PSB_EN = 0;
							ptr_out_param->CAM_A_CTL_DMA_EN.Bits.PDO_EN = 0;
						}
						/* PSB_B */
						ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = PCP_STR_X - (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
						ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = PCP_END_X - (int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
						ptr_out_param->CAM_B_PSB_SIZE.Bits.PSB_WD = ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_WD;
					}
					else
					{
						/* PSB_A */
						ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X;
						ptr_out_param->CAM_A_PSB_SIZE.Bits.PSB_WD = ((unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1 -
							(unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_STR_X)>>1;
						if ((unsigned int)ptr_out_param->CAM_A_PSB_SIZE.Bits.PSB_WD < (unsigned int)ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_WD)
						{
							/* PSB_B */
							ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = (unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1 -
								(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
							ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
								(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
							ptr_out_param->CAM_B_PSB_SIZE.Bits.PSB_WD = (unsigned int)ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_WD -
								(unsigned int)ptr_out_param->CAM_A_PSB_SIZE.Bits.PSB_WD;
						}
						else
						{
							/* PSB_B disabled */
							ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = 0;
							ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
								(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
							ptr_out_param->CAM_B_PSB_SIZE.Bits.PSB_WD = ((unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X + 1 -
								(unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X)>>1;
							if ((unsigned int)ptr_in_param->CAM_A_PDO_BASE_ADDR.Bits.BASE_ADDR == (unsigned int)ptr_in_param->CAM_B_PDO_BASE_ADDR.Bits.BASE_ADDR)
							{
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PCP_EN = 0;
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PSB_EN = 0;
								ptr_out_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN = 0;
							}
						}
					}
				}
				/* pixel mode scale */
				ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_STR_X = (unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_STR_X>>
					((int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO - (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN);
				ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = (((unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1)>>
					((int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO - (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)) - 1;
				ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = (unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X>>
					((int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO - (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN);
				ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (((unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X + 1)>>
					((int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO - (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)) - 1;
				ptr_out_param->CAM_B_PSB_SIZE.Bits.PSB_HT = ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_HT;
				ptr_out_param->CAM_B_PCP_CROP_CON2.Bits.PCP_STR_Y = ptr_in_param->CAM_A_PCP_CROP_CON2.Bits.PCP_STR_Y;
				ptr_out_param->CAM_B_PCP_CROP_CON2.Bits.PCP_END_Y = ptr_in_param->CAM_A_PCP_CROP_CON2.Bits.PCP_END_Y;
				/* PDO */
				ptr_out_param->CAM_A_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
				if ((unsigned int)ptr_in_param->CAM_A_PDO_BASE_ADDR.Bits.BASE_ADDR == (unsigned int)ptr_in_param->CAM_B_PDO_BASE_ADDR.Bits.BASE_ADDR)
				{
					ptr_out_param->CAM_B_PDO_CON.Raw = ptr_in_param->CAM_A_PDO_CON.Raw;
					ptr_out_param->CAM_B_PDO_CON2.Raw = ptr_in_param->CAM_A_PDO_CON2.Raw;
					ptr_out_param->CAM_B_PDO_CON3.Raw = ptr_in_param->CAM_A_PDO_CON3.Raw;
					ptr_out_param->CAM_B_PDO_STRIDE.Raw = ptr_in_param->CAM_A_PDO_STRIDE.Raw;
					/* PDO_D */
					if (ptr_out_param->CAM_A_CTL_DMA_EN.Bits.PDO_EN)
					{
						ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 2*(unsigned int)ptr_out_param->CAM_A_PSB_SIZE.Bits.PSB_WD;
					}
					else
					{
						ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
					}
				}
				else
				{
					ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
					if ((unsigned int)ptr_out_param->CAM_A_PSB_SIZE.Bits.PSB_WD > (unsigned int)ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_WD)
					{
						ptr_out_param->CAM_A_PSB_SIZE.Bits.PSB_WD = ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_WD;
					}
					if ((unsigned int)ptr_out_param->CAM_B_PSB_SIZE.Bits.PSB_WD > (unsigned int)ptr_in_param->CAM_B_PSB_SIZE.Bits.PSB_WD)
					{
						ptr_out_param->CAM_B_PSB_SIZE.Bits.PSB_WD = ptr_in_param->CAM_B_PSB_SIZE.Bits.PSB_WD;
					}
				}
				ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE = 2*(unsigned int)ptr_out_param->CAM_A_PSB_SIZE.Bits.PSB_WD - 1;
				ptr_out_param->CAM_A_PDO_YSIZE.Bits.YSIZE = (unsigned int)ptr_in_param->CAM_A_PSB_SIZE.Bits.PSB_HT - 1;
				ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE = 2*(unsigned int)ptr_out_param->CAM_B_PSB_SIZE.Bits.PSB_WD - 1;
				ptr_out_param->CAM_B_PDO_YSIZE.Bits.YSIZE = ptr_out_param->CAM_A_PDO_YSIZE.Bits.YSIZE;
				if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PDO_EN)
				{
					/* Stride check */
					if ((int)ptr_out_param->CAM_A_PDO_OFST_ADDR.Bits.OFFSET_ADDR + (int)ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE +
						1 > (int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE)
					{
						result = DUAL_MESSAGE_INVALID_PDO_STRIDE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				if (ptr_in_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN)
				{
					ptr_out_param->CAM_B_CTL_EN.Bits.PKP_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PKP_EN;
					if ((int)ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR + (int)ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE +
						1 > (int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE)
					{
						result = DUAL_MESSAGE_INVALID_PDO_STRIDE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
			}
			else
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		else
		{
			ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_STR_X = 0;
			if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PCP_EN)
			{
				/* PCROP */
				if ((unsigned int)ptr_in_param->CAM_A_PDO_BASE_ADDR.Bits.BASE_ADDR == (unsigned int)ptr_in_param->CAM_B_PDO_BASE_ADDR.Bits.BASE_ADDR)
				{
					/* PCROP */
					ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = ((((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)>>
						(3 + 1))<<(3 + 1)) - 1;
					/* PCROP_D */
					ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = (unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1 -
						(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
					ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
						(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
				}
				else
				{
					/* separate mode */
					if (ptr_in_param->CAM_A_PBN_TYPE.Bits.PBN_SEP)
					{
						/* PCROP */
						if ((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>1)<<(3 + 1)))
						{
							ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = ((((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE)>>1)<<(3 + 1)) - 1;
						}
						else
						{
							ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = ((((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)>>
								(3 + 1))<<(3 + 1)) - 1;
						}
						/* PCROP_D */
						if (ptr_in_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN)
						{
							if ((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE>>1)<<(3+1)) +
								(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X)
							{
								ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = ((((unsigned int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE)>>1)<<(3 + 1)) - 1 -
									(((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>(3 + 1))<<(3 + 1));
							}
							else
							{
								ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
									(((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>(3 + 1))<<(3 + 1));
							}
							/* PCROP_D */
							ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X & 0xF;
						}
						else
						{
							if ((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>1)<<(3+1)))
							{
								ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = ((((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE)>>1)<<(3 + 1)) -
									2 - (unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X;
							}
							else
							{
								ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
									(unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X - 1;
							}
						}
						/* PCROP_D */
						ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = (unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1 -
							(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
					}
					else
					{
						/* PCROP */
						if ((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>1)<<3))
						{
							ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = ((((((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE)>>1)<<3)
								>>(3 + 1))<<(3 + 1)) - 1;
						}
						else
						{
							ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = ((((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)>>
								(3 + 1))<<(3 + 1)) - 1;
						}
						/* PCROP_D */
						if (ptr_in_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN)
						{
							if ((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE>>1)<<3) +
								(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X)
							{
								ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = ((((((unsigned int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE)>>1)<<3)
									>>(3 + 1))<<(3 + 1)) - 1 - (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>(3 + 1))<<(3 + 1));
							}
							else
							{
								ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
									(((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>(3 + 1))<<(3 + 1));
							}
							/* PCROP_D */
							ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X & 0xF;
						}
						else
						{
							if ((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>1)<<3))
							{
								ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = ((((((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE)>>1)<<3)
									>>(3 + 1))<<(3 + 1)) - 2 - (unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X;
							}
							else
							{
								ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
									(unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X - 1;
							}
							/* PCROP_D */
							ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = (unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1 -
								(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X;
						}
					}
				}
				/* PCROP */
				ptr_out_param->CAM_A_PCP_CROP_CON2.Bits.PCP_STR_Y = 0;
				ptr_out_param->CAM_A_PCP_CROP_CON2.Bits.PCP_END_Y = ((unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
					(int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN) - 1;
				/* PCROP_D */
				ptr_out_param->CAM_B_PCP_CROP_CON2.Bits.PCP_STR_Y = 0;
				ptr_out_param->CAM_B_PCP_CROP_CON2.Bits.PCP_END_Y = ptr_out_param->CAM_A_PCP_CROP_CON2.Bits.PCP_END_Y;
				/* separate mode */
				if (ptr_in_param->CAM_A_PBN_TYPE.Bits.PBN_SEP)
				{
					/* PMX */
					ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1)>>(3 + 1)) - 1;
					ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
						((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3 - 1);
					/* PMX_D */
					ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X -
						(unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X + 1)>>(3 + 1)) - 1;
				}
				else
				{
					/* PMX */
					ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1)>>3) - 1;
					ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
						((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3);
					/* PMX_D */
					ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X -
						(unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X + 1)>>3) - 1;
				}
				/* pixel mode scale */
				ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X = (((unsigned int)ptr_out_param->CAM_A_PCP_CROP_CON1.Bits.PCP_END_X + 1)>>
					((int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO - (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)) - 1;
				ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X = (unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_STR_X>>
					((int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO - (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN);
				ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X = (((unsigned int)ptr_out_param->CAM_B_PCP_CROP_CON1.Bits.PCP_END_X + 1)>>
					((int)ptr_in_param->CAM_A_CTL_FMT_SEL.Bits.PIX_BUS_DMXO - (int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.SCM_EN)) - 1;
				/* PMX_D */
				ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X = 0;
				/* PDO */
				ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE = 2*(unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X + 1;
				ptr_out_param->CAM_A_PDO_YSIZE.Bits.YSIZE = (unsigned int)ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT - 1;
				/* PDO_D */
				ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE + 1;
				ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE = 2*(unsigned int)ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X + 1;
			}
			else
			{
				if (ptr_in_param->CAM_A_CTL_EN.Bits.PMX_EN)
				{
					if ((unsigned int)ptr_in_param->CAM_A_PDO_BASE_ADDR.Bits.BASE_ADDR == (unsigned int)ptr_in_param->CAM_B_PDO_BASE_ADDR.Bits.BASE_ADDR)
					{
						/* separate mode */
						if (ptr_in_param->CAM_A_PBN_TYPE.Bits.PBN_SEP)
						{
							/* PMX */
							ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)>>(3 + 1)) - 1;
							ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3 - 1);
							/* PMX_D */
							ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X = ((((unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X + 1)<<(3+1)) -
								(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X)>>(3+1);
							ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)>>(3 + 1)) - 1 -
								((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>(3 + 1));
							ptr_out_param->CAM_B_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3 - 1);
						}
						else
						{
							/* PMX */
							ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)>>3) - 1;
							ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3);
							/* PMX_D */
							ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X = ((((unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X + 1)<<3) -
								(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X)>>3;
							ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)>>3) - 1 -
								((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>3);
							ptr_out_param->CAM_B_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3);
						}
					}
					else
					{
						/* separate mode */
						if (ptr_in_param->CAM_A_PBN_TYPE.Bits.PBN_SEP)
						{
							/* PMX */
							if ((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>1)<<(3 + 1)))
							{
								ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = ((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>1) - 1;
							}
							else
							{
								ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)>>(3 + 1)) - 1;
							}
							ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3 - 1);
							/* PMX_D */
							if (ptr_in_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN)
							{
								if ((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE>>1)<<(3+1)) +
									(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X)
								{
									ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE)>>1) - 1 -
										((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>(3 + 1));
								}
								else
								{
									ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)>>(3 + 1)) - 1 -
										((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>(3 + 1));
								}
								/* PMX_D */
								ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X = 0;
							}
							else
							{
								if ((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>1)<<(3+1)))
								{
									ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE)>>1) - 2 -
										(unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X;
								}
								else
								{
									ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)>>(3 + 1)) - 2 -
										(unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X;
								}
								/* PMX_D */
								ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X = (unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X + 1 -
									((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>(3+1));
							}
							ptr_out_param->CAM_B_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3 - 1);
						}
						else
						{
							/* PMX */
							if ((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>1)<<3))
							{
								ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>2)<<1) - 1;/*>>1<<3>>4<<4>>3 */
							}
							else
							{
								ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X + 1)>>3) - 1;
							}
							/* PMX */
							ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3);
							/* PMX_D */
							if (ptr_in_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN)
							{
								if ((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE>>1)<<3) +
									(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X)
								{
									ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE>>2)<<1) - 1 -
										((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>3);/*>>1<<3>>4<<4>>3 */
								}
								else
								{
									ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)>>3) - 1 -
										((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>3);
								}
								/* PMX_D */
								ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X = 0;
							}
							else
							{
								if ((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1 > (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>1)<<3))
								{
									ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE>>2)<<1) - 2 -
										(unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X;/*>>1<<3>>4<<4>>3 */
								}
								else
								{
									ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X = (((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X + 1)>>3) - 2 -
										(unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X;
								}
								/* PMX_D */
								ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X = (unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X + 1 -
									((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>3);
							}
							ptr_out_param->CAM_B_PMX_VSIZE.Bits.PMX_HT = (unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
								((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3);
						}
					}
					/* PDO */
					ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE = 2*(unsigned int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X + 1;
					ptr_out_param->CAM_A_PDO_YSIZE.Bits.YSIZE = (unsigned int)ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT - 1;
					/* PDO_D */
					ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE + 1;
					ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE = 2*((unsigned int)ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X -
						(unsigned int)ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X + 1) - 1;
				}
				else /* no PMX support */
				{
					/* separate mode */
					if (ptr_in_param->CAM_A_PBN_TYPE.Bits.PBN_SEP)
					{
						/* PDO */
						ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE = 2*(((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
							(unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1)>>(3 + 1)) - 1;
						ptr_out_param->CAM_A_PDO_YSIZE.Bits.YSIZE = ((unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
							((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3 - 1)) - 1;
						/* PDO_D */
						ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 2*((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>(3 + 1));
						ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE = 2*(((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
							(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1)>>(3 + 1)) - 1;
					}
					else
					{
						/* PDO */
						ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE = 2*(((unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_END_X -
							(unsigned int)ptr_out_param->CAM_A_DMX_CROP.Bits.DMX_STR_X + 1)>>3) - 1;
						ptr_out_param->CAM_A_PDO_YSIZE.Bits.YSIZE = ((unsigned int)ptr_in_param->CAM_A_DMX_VSIZE.Bits.DMX_HT>>
							((unsigned int)ptr_in_param->CAM_A_CTL_DMA_EN.Bits.VBN_EN + 3)) - 1;
						/* PDO_D */
						ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 2*((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X>>3);
						ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE = 2*(((unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_END_X -
							(unsigned int)ptr_out_param->CAM_B_DMX_CROP.Bits.DMX_STR_X + 1)>>3) - 1;
					}
				}
			}
			/* PMX */
			ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_STR_X = 0;
			ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_EDGE = ptr_out_param->CAM_A_DMX_CTL.Bits.DMX_EDGE;
			ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_SINGLE_MODE_1 = 1;
			ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_SINGLE_MODE_2 = 0;
			/* PMX_D */
			ptr_out_param->CAM_B_PMX_VSIZE.Bits.PMX_HT = ptr_out_param->CAM_A_PMX_VSIZE.Bits.PMX_HT;
			ptr_out_param->CAM_B_PMX_CTL.Bits.PMX_EDGE = ptr_out_param->CAM_B_DMX_CTL.Bits.DMX_EDGE;
			ptr_out_param->CAM_B_PMX_CTL.Bits.PMX_SINGLE_MODE_1 = 1;
			ptr_out_param->CAM_B_PMX_CTL.Bits.PMX_SINGLE_MODE_2 = 0;
			/* PDO */
			ptr_out_param->CAM_A_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
			/* PDO_D */
			ptr_out_param->CAM_B_PDO_YSIZE.Bits.YSIZE = ptr_out_param->CAM_A_PDO_YSIZE.Bits.YSIZE;
			if ((int)ptr_in_param->CAM_A_PDO_BASE_ADDR.Bits.BASE_ADDR == (int)ptr_in_param->CAM_B_PDO_BASE_ADDR.Bits.BASE_ADDR)
			{
				ptr_out_param->CAM_B_PDO_CON.Raw = ptr_in_param->CAM_A_PDO_CON.Raw;
				ptr_out_param->CAM_B_PDO_CON2.Raw = ptr_in_param->CAM_A_PDO_CON2.Raw;
				ptr_out_param->CAM_B_PDO_CON3.Raw = ptr_in_param->CAM_A_PDO_CON3.Raw;
				ptr_out_param->CAM_B_PDO_STRIDE.Raw = ptr_in_param->CAM_A_PDO_STRIDE.Raw;
			}
			else
			{
				ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR = 0;
			}
			if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.PDO_EN)
			{
				/* Stride check */
				if ((int)ptr_out_param->CAM_A_PDO_OFST_ADDR.Bits.OFFSET_ADDR + (int)ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE +
					1 > (int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE)
				{
					result = DUAL_MESSAGE_INVALID_PDO_STRIDE_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
			if (ptr_in_param->CAM_B_CTL_DMA_EN.Bits.PDO_EN)
			{
				ptr_out_param->CAM_B_CTL_EN.Bits.PKP_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PKP_EN;
				if ((int)ptr_out_param->CAM_B_PDO_OFST_ADDR.Bits.OFFSET_ADDR + (int)ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE +
					1 > (int)ptr_in_param->CAM_B_PDO_STRIDE.Bits.STRIDE)
				{
					result = DUAL_MESSAGE_INVALID_PDO_STRIDE_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
			else
			{
				/* merge by PMX */
				if (ptr_in_param->CAM_A_CTL_EN.Bits.PMX_EN)
				{
					ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE = (int)ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE +
						(int)ptr_out_param->CAM_B_PDO_XSIZE.Bits.XSIZE + 1;
					ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_SINGLE_MODE_1 = 0;
					ptr_out_param->CAM_A_PMX_CTL.Bits.PMX_SINGLE_MODE_2 = 0;
					if ((int)ptr_out_param->CAM_A_PDO_XSIZE.Bits.XSIZE + 1 > (int)ptr_in_param->CAM_A_PDO_STRIDE.Bits.STRIDE)
					{
						result = DUAL_MESSAGE_INVALID_PDO_STRIDE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
			}
			if (ptr_in_param->CAM_A_CTL_EN.Bits.PMX_EN)
			{
				if ((int)ptr_out_param->CAM_A_PMX_CROP.Bits.PMX_END_X >= DUAL_PMX_WD_ALL)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				if ((int)ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_END_X >= DUAL_PMX_WD_ALL + (int)ptr_out_param->CAM_B_PMX_CROP.Bits.PMX_STR_X)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
			}
		}
		ptr_out_param->CAM_B_CTL_EN.Bits.PMX_EN = ptr_in_param->CAM_A_CTL_EN.Bits.PMX_EN;
		ptr_out_param->CAM_B_CTL_SEL.Bits.PMX_SEL = ptr_in_param->CAM_A_CTL_SEL.Bits.PMX_SEL;
	}
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_rpg(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	ptr_out_param->CAM_B_CTL_EN.Bits.RPG_EN = ptr_in_param->CAM_A_CTL_EN.Bits.RPG_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_qbin1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	/* QBINI1 */
	if (ptr_in_param->CAM_A_CTL_EN.Bits.QBIN1_EN)
	{
		if (ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC)
		{
			/* AMX */
			ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_STR_X = (unsigned int)ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_STR_X>>
				((int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC);
			ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_END_X = (((unsigned int)ptr_out_param->CAM_A_AMX_CROP.Bits.AMX_END_X + 1)>>
				((int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC)) - 1;
			/* AMX_D */
			ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_STR_X = (unsigned int)ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_STR_X>>
				((int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC);
			ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X = (((unsigned int)ptr_out_param->CAM_B_AMX_CROP.Bits.AMX_END_X + 1)>>
				((int)ptr_in_param->CAM_A_QBN1_MODE.Bits.QBN_ACC)) - 1;
		}
	}
	ptr_out_param->CAM_B_CTL_EN.Bits.QBIN1_EN = ptr_in_param->CAM_A_CTL_EN.Bits.QBIN1_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_qbin4(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
    UNUSED(dual_mode);

	/* QBIN4_EN */
	if (ptr_in_param->CAM_A_CTL_DMA_EN.Bits.QBIN4_EN)
	{
		if (ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC)
		{
			/* BMX */
			ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_STR_X = (unsigned int)ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_STR_X>>
				((int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC);
			ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X = (((unsigned int)ptr_out_param->CAM_A_BMX_CROP.Bits.BMX_END_X + 1)>>
				((int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC)) - 1;
			/* BMX_D */
			ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X = (unsigned int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_STR_X>>
				((int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC);
			ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X = (((unsigned int)ptr_out_param->CAM_B_BMX_CROP.Bits.BMX_END_X + 1)>>
				((int)ptr_in_param->CAM_A_QBN4_MODE.Bits.QBN_ACC)) - 1;
		}
	}
	ptr_out_param->CAM_B_CTL_DMA_EN.Bits.QBIN4_EN = ptr_in_param->CAM_A_CTL_DMA_EN.Bits.QBIN4_EN;
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
