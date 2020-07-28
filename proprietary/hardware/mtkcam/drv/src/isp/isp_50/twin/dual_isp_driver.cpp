#include "MyUtils.h"

/* define to replace macro */
#define print_error_message(...) dual_isp_print_error_message(__VA_ARGS__)
#define get_current_file_name(...) dual_isp_get_current_file_name(__VA_ARGS__)
/* func prototype */
#ifndef DUAL_PLATFORM_DRIVER
static const char *dual_isp_print_error_message(DUAL_MESSAGE_ENUM n);
static const char *dual_isp_get_current_file_name(const char *filename);
#endif
/* extern */
extern int dual_cal_main(const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
int dual_convert_input(int dual_sel, const ISP_REG_PTR_STRUCT *ptr_isp_reg_param, DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
						DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config);
int dual_convert_output(int dual_sel, ISP_REG_PTR_STRUCT *ptr_isp_reg_param, const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
						const DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config);
int dual_fprintf_in_config(int count, const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config);
int dual_printf_platform_config(const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config);
int dual_printf_platform_reg(const ISP_REG_PTR_STRUCT *ptr_isp_reg_param);
int dual_fprintf_reg_dump(int dual_sel, const ISP_REG_PTR_STRUCT *ptr_isp_reg_param);

/* return 0: pass, 1: error */
int dual_cal_platform(ISP_REG_PTR_STRUCT *ptr_isp_reg_param, DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
					  DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config)
{
	int result = 0;
    /* check in/out pointer */
	if (ptr_isp_reg_param == NULL || ptr_dual_in_config == NULL || ptr_dual_out_config == NULL)
	{
		return 1;
	}
	/* check supported dual mode */
	if (false == dual_sel_check_support(ptr_dual_in_config->SW.DUAL_SEL))
	{
		return 1;
	}
	/* update isp_reg_ptr_param to dual_isp_in_config & dual_isp_out_config */
	result = dual_convert_input(ptr_dual_in_config->SW.DUAL_SEL, ptr_isp_reg_param, ptr_dual_in_config, ptr_dual_out_config);
	if (0 == result)
	{
		/* run dual cal */
		result = dual_cal_main(ptr_dual_in_config, ptr_dual_out_config);
	}
	if (0 == result)
	{
		result = dual_convert_output(ptr_dual_in_config->SW.DUAL_SEL, ptr_isp_reg_param, ptr_dual_in_config, ptr_dual_out_config);
	}
	if ((0 != result) || ptr_dual_in_config->DEBUG.DUAL_LOG_EN)
	{
		int temp_result = dual_printf_platform_config(ptr_dual_in_config);
		if (temp_result)
		{
			result = temp_result;
		}
	}
	return result;
}

int dual_printf_platform_config(const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config)
{
	int result = dual_fprintf_in_config(0, ptr_dual_in_config);
	return result;
}

int dual_printf_platform_reg(const ISP_REG_PTR_STRUCT *ptr_isp_reg_param)
{
	int result = dual_fprintf_reg_dump(0xF, ptr_isp_reg_param);
	return result;
}

int dual_convert_input(int dual_sel, const ISP_REG_PTR_STRUCT *ptr_isp_reg_param, DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
						DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config)
{
	if (ptr_isp_reg_param && ptr_dual_in_config && ptr_dual_out_config)
	{
		/* update dual_in_config & dual_out_config */
		if (ptr_isp_reg_param->ptr_isp_uni)
		{
			DUAL_CTRL_LIST_HW(DUAL_CONVERT_UNI_TO_DUAL, ptr_isp_reg_param, ptr_dual_in_config, ptr_dual_out_config,,,);
		}
		if ((ptr_isp_reg_param->ptr_isp_reg && (dual_sel & (0x10 + 0x1))) || (ptr_isp_reg_param->ptr_isp_reg_d && (dual_sel & 0x2)) ||
			(ptr_isp_reg_param->ptr_isp_reg_t && (dual_sel & 0x4)))
		{
			if (TWIN_SCENARIO_AF_FAST_P2 == ptr_dual_in_config->SW.TWIN_SCENARIO)
			{
				if (dual_sel & 0x1)
				{
					/* A+B, A+B+C */
					DUAL_REG_A_STRUCT *ptr_isp_reg = ptr_isp_reg_param->ptr_isp_reg;
					/* DUAL_ENGINE_LIST_HW_R_R */
					ptr_dual_in_config->CAM_A_AFO_BASE_ADDR.Raw = ptr_isp_reg->CAM_A_AFO_BASE_ADDR.Raw;
					ptr_dual_in_config->CAM_A_AFO_STRIDE.Raw = ptr_isp_reg->CAM_A_AFO_STRIDE.Raw;
					/* DUAL_ENGINE_LIST_HW_RW_RW */
					/* A to A in */
					ptr_dual_in_config->CAM_A_CTL_EN.Raw = ptr_isp_reg->CAM_A_CTL_EN.Raw;
					ptr_dual_in_config->CAM_A_CTL_EN2.Raw = ptr_isp_reg->CAM_A_CTL_EN2.Raw;
					ptr_dual_in_config->CAM_A_CTL_DMA_EN.Raw = ptr_isp_reg->CAM_A_CTL_DMA_EN.Raw;
					ptr_dual_in_config->CAM_A_CTL_SEL.Raw = ptr_isp_reg->CAM_A_CTL_SEL.Raw;
					ptr_dual_in_config->CAM_A_TG_SEN_GRAB_PXL.Raw = ptr_isp_reg->CAM_A_TG_SEN_GRAB_PXL.Raw;
					ptr_dual_in_config->CAM_A_AF_BLK_1.Raw = ptr_isp_reg->CAM_A_AF_BLK_1.Raw;
					ptr_dual_in_config->CAM_A_AF_VLD.Raw = ptr_isp_reg->CAM_A_AF_VLD.Raw;
					ptr_dual_in_config->CAM_A_AFO_YSIZE.Raw = ptr_isp_reg->CAM_A_AFO_YSIZE.Raw;
					/* A to A out */
					ptr_dual_out_config->CAM_A_CTL_EN.Raw = ptr_isp_reg->CAM_A_CTL_EN.Raw;
					ptr_dual_out_config->CAM_A_CTL_EN2.Raw = ptr_isp_reg->CAM_A_CTL_EN2.Raw;
					ptr_dual_out_config->CAM_A_CTL_DMA_EN.Raw = ptr_isp_reg->CAM_A_CTL_DMA_EN.Raw;
					ptr_dual_out_config->CAM_A_AF_BLK_1.Raw = ptr_isp_reg->CAM_A_AF_BLK_1.Raw;
					ptr_dual_out_config->CAM_A_AF_VLD.Raw = ptr_isp_reg->CAM_A_AF_VLD.Raw;
					ptr_dual_out_config->CAM_A_AFO_OFST_ADDR.Raw = ptr_isp_reg->CAM_A_AFO_OFST_ADDR.Raw;
					ptr_dual_out_config->CAM_A_AFO_XSIZE.Raw = ptr_isp_reg->CAM_A_AFO_XSIZE.Raw;
					ptr_dual_out_config->CAM_A_AFO_YSIZE.Raw = ptr_isp_reg->CAM_A_AFO_YSIZE.Raw;
					/* DUAL_ENGINE_LIST_HW_R_RW */
					/* A to A in */
					ptr_dual_in_config->CAM_A_AF_BLK_0.Raw = ptr_isp_reg->CAM_A_AF_BLK_0.Raw;
					ptr_dual_in_config->CAM_A_AF_BLK_2.Raw = ptr_isp_reg->CAM_A_AF_BLK_2.Raw;
					ptr_dual_in_config->CAM_A_AF_CON.Raw = ptr_isp_reg->CAM_A_AF_CON.Raw;
					ptr_dual_in_config->CAM_A_AF_CON2.Raw = ptr_isp_reg->CAM_A_AF_CON2.Raw;
					ptr_dual_in_config->CAM_A_AFO_DRS.Raw = ptr_isp_reg->CAM_A_AFO_DRS.Raw;
					ptr_dual_in_config->CAM_A_AFO_CON.Raw = ptr_isp_reg->CAM_A_AFO_CON.Raw;
					ptr_dual_in_config->CAM_A_AFO_CON2.Raw = ptr_isp_reg->CAM_A_AFO_CON2.Raw;
					ptr_dual_in_config->CAM_A_AFO_CON3.Raw = ptr_isp_reg->CAM_A_AFO_CON3.Raw;
					ptr_dual_in_config->CAM_A_AFO_CON4.Raw = ptr_isp_reg->CAM_A_AFO_CON4.Raw;
					if (dual_sel & 0x2)
					{
						/* A+B, A+B+C */
						DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
						/* DUAL_ENGINE_LIST_HW_R_R */
						ptr_dual_in_config->CAM_B_AFO_BASE_ADDR.Raw = ptr_isp_reg_d->CAM_B_AFO_BASE_ADDR.Raw;
						ptr_dual_in_config->CAM_B_AFO_STRIDE.Raw = ptr_isp_reg_d->CAM_B_AFO_STRIDE.Raw;
						/* DUAL_ENGINE_LIST_HW_RW_RW */
						/* B to B in */
						/* B to B out */
						ptr_dual_out_config->CAM_B_CTL_EN.Raw = ptr_isp_reg_d->CAM_B_CTL_EN.Raw;
						ptr_dual_out_config->CAM_B_CTL_EN2.Raw = ptr_isp_reg_d->CAM_B_CTL_EN2.Raw;
						ptr_dual_out_config->CAM_B_CTL_DMA_EN.Raw = ptr_isp_reg_d->CAM_B_CTL_DMA_EN.Raw;
						ptr_dual_out_config->CAM_B_AF_BLK_1.Raw = ptr_isp_reg_d->CAM_B_AF_BLK_1.Raw;
						ptr_dual_out_config->CAM_B_AF_VLD.Raw = ptr_isp_reg_d->CAM_B_AF_VLD.Raw;
						ptr_dual_out_config->CAM_B_AFO_OFST_ADDR.Raw = ptr_isp_reg_d->CAM_B_AFO_OFST_ADDR.Raw;
						ptr_dual_out_config->CAM_B_AFO_XSIZE.Raw = ptr_isp_reg_d->CAM_B_AFO_XSIZE.Raw;
						ptr_dual_out_config->CAM_B_AFO_YSIZE.Raw = ptr_isp_reg_d->CAM_B_AFO_YSIZE.Raw;
						/* DUAL_ENGINE_LIST_HW_R_RW */
						/* B to B in */
						ptr_dual_in_config->CAM_B_CTL_SEL.Raw = ptr_isp_reg_d->CAM_B_CTL_SEL.Raw;
						/* B to B out */
						ptr_dual_out_config->CAM_B_AF_BLK_0.Raw = ptr_isp_reg_d->CAM_B_AF_BLK_0.Raw;
						ptr_dual_out_config->CAM_B_AF_BLK_2.Raw = ptr_isp_reg_d->CAM_B_AF_BLK_2.Raw;
						ptr_dual_out_config->CAM_B_AF_CON.Raw = ptr_isp_reg_d->CAM_B_AF_CON.Raw;
						ptr_dual_out_config->CAM_B_AF_CON2.Raw = ptr_isp_reg_d->CAM_B_AF_CON2.Raw;
						ptr_dual_out_config->CAM_B_AFO_DRS.Raw = ptr_isp_reg_d->CAM_B_AFO_DRS.Raw;
						ptr_dual_out_config->CAM_B_AFO_CON.Raw = ptr_isp_reg_d->CAM_B_AFO_CON.Raw;
						ptr_dual_out_config->CAM_B_AFO_CON2.Raw = ptr_isp_reg_d->CAM_B_AFO_CON2.Raw;
						ptr_dual_out_config->CAM_B_AFO_CON3.Raw = ptr_isp_reg_d->CAM_B_AFO_CON3.Raw;
						ptr_dual_out_config->CAM_B_AFO_CON4.Raw = ptr_isp_reg_d->CAM_B_AFO_CON4.Raw;
						/* DUAL_ENGINE_LIST_HW_R_W */
						/* A to B out */
						if (dual_sel & 0x4)
						{
							/* A+B+C */
							DUAL_REG_C_STRUCT *ptr_isp_reg_t = ptr_isp_reg_param->ptr_isp_reg_t;
							/* DUAL_ENGINE_LIST_HW_R_R */
							ptr_dual_in_config->CAM_C_AFO_BASE_ADDR.Raw = ptr_isp_reg_t->CAM_C_AFO_BASE_ADDR.Raw;
							ptr_dual_in_config->CAM_C_AFO_STRIDE.Raw = ptr_isp_reg_t->CAM_C_AFO_STRIDE.Raw;
							/* DUAL_ENGINE_LIST_HW_RW_RW */
							/* C to C in */
							/* C to C out */
							ptr_dual_out_config->CAM_C_CTL_EN.Raw = ptr_isp_reg_t->CAM_C_CTL_EN.Raw;
							ptr_dual_out_config->CAM_C_CTL_EN2.Raw = ptr_isp_reg_t->CAM_C_CTL_EN2.Raw;
							ptr_dual_out_config->CAM_C_CTL_DMA_EN.Raw = ptr_isp_reg_t->CAM_C_CTL_DMA_EN.Raw;
							ptr_dual_out_config->CAM_C_AF_BLK_1.Raw = ptr_isp_reg_t->CAM_C_AF_BLK_1.Raw;
							ptr_dual_out_config->CAM_C_AF_VLD.Raw = ptr_isp_reg_t->CAM_C_AF_VLD.Raw;
							ptr_dual_out_config->CAM_C_AFO_OFST_ADDR.Raw = ptr_isp_reg_t->CAM_C_AFO_OFST_ADDR.Raw;
							ptr_dual_out_config->CAM_C_AFO_XSIZE.Raw = ptr_isp_reg_t->CAM_C_AFO_XSIZE.Raw;
							ptr_dual_out_config->CAM_C_AFO_YSIZE.Raw = ptr_isp_reg_t->CAM_C_AFO_YSIZE.Raw;
							/* DUAL_ENGINE_LIST_HW_R_RW */
							/* C to C in */
							ptr_dual_in_config->CAM_C_CTL_SEL.Raw = ptr_isp_reg_t->CAM_C_CTL_SEL.Raw;
							/* C to C out */
							ptr_dual_out_config->CAM_C_AF_BLK_0.Raw = ptr_isp_reg_t->CAM_C_AF_BLK_0.Raw;
							ptr_dual_out_config->CAM_C_AF_BLK_2.Raw = ptr_isp_reg_t->CAM_C_AF_BLK_2.Raw;
							ptr_dual_out_config->CAM_C_AF_CON.Raw = ptr_isp_reg_t->CAM_C_AF_CON.Raw;
							ptr_dual_out_config->CAM_C_AF_CON2.Raw = ptr_isp_reg_t->CAM_C_AF_CON2.Raw;
							ptr_dual_out_config->CAM_C_AFO_DRS.Raw = ptr_isp_reg_t->CAM_C_AFO_DRS.Raw;
							ptr_dual_out_config->CAM_C_AFO_CON.Raw = ptr_isp_reg_t->CAM_C_AFO_CON.Raw;
							ptr_dual_out_config->CAM_C_AFO_CON2.Raw = ptr_isp_reg_t->CAM_C_AFO_CON2.Raw;
							ptr_dual_out_config->CAM_C_AFO_CON3.Raw = ptr_isp_reg_t->CAM_C_AFO_CON3.Raw;
							ptr_dual_out_config->CAM_C_AFO_CON4.Raw = ptr_isp_reg_t->CAM_C_AFO_CON4.Raw;
							/* DUAL_ENGINE_LIST_HW_R_W */
							/* A to C out */
						}
					}
				}
				else
				{
					/* B+C, B+C+A */
					DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
					/* DUAL_ENGINE_LIST_HW_R_R */
					ptr_dual_in_config->CAM_A_AFO_BASE_ADDR.Raw = ptr_isp_reg_d->CAM_B_AFO_BASE_ADDR.Raw;
					ptr_dual_in_config->CAM_A_AFO_STRIDE.Raw = ptr_isp_reg_d->CAM_B_AFO_STRIDE.Raw;
					/* DUAL_ENGINE_LIST_HW_RW_RW */
					/* B to A in */
					ptr_dual_in_config->CAM_A_CTL_EN.Raw = ptr_isp_reg_d->CAM_B_CTL_EN.Raw;
					ptr_dual_in_config->CAM_A_CTL_EN2.Raw = ptr_isp_reg_d->CAM_B_CTL_EN2.Raw;
					ptr_dual_in_config->CAM_A_CTL_DMA_EN.Raw = ptr_isp_reg_d->CAM_B_CTL_DMA_EN.Raw;
					ptr_dual_in_config->CAM_A_CTL_SEL.Raw = ptr_isp_reg_d->CAM_B_CTL_SEL.Raw;
					ptr_dual_in_config->CAM_A_TG_SEN_GRAB_PXL.Raw = ptr_isp_reg_d->CAM_B_TG_SEN_GRAB_PXL.Raw;
					ptr_dual_in_config->CAM_A_AF_BLK_1.Raw = ptr_isp_reg_d->CAM_B_AF_BLK_1.Raw;
					ptr_dual_in_config->CAM_A_AF_VLD.Raw = ptr_isp_reg_d->CAM_B_AF_VLD.Raw;
					ptr_dual_in_config->CAM_A_AFO_YSIZE.Raw = ptr_isp_reg_d->CAM_B_AFO_YSIZE.Raw;
					/* B to A out */
					ptr_dual_out_config->CAM_A_CTL_EN.Raw = ptr_isp_reg_d->CAM_B_CTL_EN.Raw;
					ptr_dual_out_config->CAM_A_CTL_EN2.Raw = ptr_isp_reg_d->CAM_B_CTL_EN2.Raw;
					ptr_dual_out_config->CAM_A_CTL_DMA_EN.Raw = ptr_isp_reg_d->CAM_B_CTL_DMA_EN.Raw;
					ptr_dual_out_config->CAM_A_AF_BLK_1.Raw = ptr_isp_reg_d->CAM_B_AF_BLK_1.Raw;
					ptr_dual_out_config->CAM_A_AF_VLD.Raw = ptr_isp_reg_d->CAM_B_AF_VLD.Raw;
					ptr_dual_out_config->CAM_A_AFO_OFST_ADDR.Raw = ptr_isp_reg_d->CAM_B_AFO_OFST_ADDR.Raw;
					ptr_dual_out_config->CAM_A_AFO_XSIZE.Raw = ptr_isp_reg_d->CAM_B_AFO_XSIZE.Raw;
					ptr_dual_out_config->CAM_A_AFO_YSIZE.Raw = ptr_isp_reg_d->CAM_B_AFO_YSIZE.Raw;
					/* DUAL_ENGINE_LIST_HW_R_RW */
					/* B to A in */
					ptr_dual_in_config->CAM_A_AF_BLK_0.Raw = ptr_isp_reg_d->CAM_B_AF_BLK_0.Raw;
					ptr_dual_in_config->CAM_A_AF_BLK_2.Raw = ptr_isp_reg_d->CAM_B_AF_BLK_2.Raw;
					ptr_dual_in_config->CAM_A_AF_CON.Raw = ptr_isp_reg_d->CAM_B_AF_CON.Raw;
					ptr_dual_in_config->CAM_A_AF_CON2.Raw = ptr_isp_reg_d->CAM_B_AF_CON2.Raw;
					ptr_dual_in_config->CAM_A_AFO_DRS.Raw = ptr_isp_reg_d->CAM_B_AFO_DRS.Raw;
					ptr_dual_in_config->CAM_A_AFO_CON.Raw = ptr_isp_reg_d->CAM_B_AFO_CON.Raw;
					ptr_dual_in_config->CAM_A_AFO_CON2.Raw = ptr_isp_reg_d->CAM_B_AFO_CON2.Raw;
					ptr_dual_in_config->CAM_A_AFO_CON3.Raw = ptr_isp_reg_d->CAM_B_AFO_CON3.Raw;
					ptr_dual_in_config->CAM_A_AFO_CON4.Raw = ptr_isp_reg_d->CAM_B_AFO_CON4.Raw;
				}
				if ((0x5 == dual_sel) || (0x6 == dual_sel) || (0x16 == dual_sel))
				{
					/* A+C, B+C, B+C+A */
					DUAL_REG_C_STRUCT *ptr_isp_reg_t = ptr_isp_reg_param->ptr_isp_reg_t;
					/* DUAL_ENGINE_LIST_HW_R_R */
					ptr_dual_in_config->CAM_B_AFO_BASE_ADDR.Raw = ptr_isp_reg_t->CAM_C_AFO_BASE_ADDR.Raw;
					ptr_dual_in_config->CAM_B_AFO_STRIDE.Raw = ptr_isp_reg_t->CAM_C_AFO_STRIDE.Raw;
					/* DUAL_ENGINE_LIST_HW_RW_RW */
					/* C to B in */
					/* C to B out */
					ptr_dual_out_config->CAM_B_CTL_EN.Raw = ptr_isp_reg_t->CAM_C_CTL_EN.Raw;
					ptr_dual_out_config->CAM_B_CTL_EN2.Raw = ptr_isp_reg_t->CAM_C_CTL_EN2.Raw;
					ptr_dual_out_config->CAM_B_CTL_DMA_EN.Raw = ptr_isp_reg_t->CAM_C_CTL_DMA_EN.Raw;
					ptr_dual_out_config->CAM_B_AF_BLK_1.Raw = ptr_isp_reg_t->CAM_C_AF_BLK_1.Raw;
					ptr_dual_out_config->CAM_B_AF_VLD.Raw = ptr_isp_reg_t->CAM_C_AF_VLD.Raw;
					ptr_dual_out_config->CAM_B_AFO_OFST_ADDR.Raw = ptr_isp_reg_t->CAM_C_AFO_OFST_ADDR.Raw;
					ptr_dual_out_config->CAM_B_AFO_XSIZE.Raw = ptr_isp_reg_t->CAM_C_AFO_XSIZE.Raw;
					ptr_dual_out_config->CAM_B_AFO_YSIZE.Raw = ptr_isp_reg_t->CAM_C_AFO_YSIZE.Raw;
					/* DUAL_ENGINE_LIST_HW_R_RW */
					/* C to B in */
					ptr_dual_in_config->CAM_B_CTL_SEL.Raw = ptr_isp_reg_t->CAM_C_CTL_SEL.Raw;
					/* C to B out */
					ptr_dual_out_config->CAM_B_AF_BLK_0.Raw = ptr_isp_reg_t->CAM_C_AF_BLK_0.Raw;
					ptr_dual_out_config->CAM_B_AF_BLK_2.Raw = ptr_isp_reg_t->CAM_C_AF_BLK_2.Raw;
					ptr_dual_out_config->CAM_B_AF_CON.Raw = ptr_isp_reg_t->CAM_C_AF_CON.Raw;
					ptr_dual_out_config->CAM_B_AF_CON2.Raw = ptr_isp_reg_t->CAM_C_AF_CON2.Raw;
					ptr_dual_out_config->CAM_B_AFO_DRS.Raw = ptr_isp_reg_t->CAM_C_AFO_DRS.Raw;
					ptr_dual_out_config->CAM_B_AFO_CON.Raw = ptr_isp_reg_t->CAM_C_AFO_CON.Raw;
					ptr_dual_out_config->CAM_B_AFO_CON2.Raw = ptr_isp_reg_t->CAM_C_AFO_CON2.Raw;
					ptr_dual_out_config->CAM_B_AFO_CON3.Raw = ptr_isp_reg_t->CAM_C_AFO_CON3.Raw;
					ptr_dual_out_config->CAM_B_AFO_CON4.Raw = ptr_isp_reg_t->CAM_C_AFO_CON4.Raw;
					/* DUAL_ENGINE_LIST_HW_R_W */
					/* C to B out */
					if (dual_sel & 0x10)
					{
						/* B+C+A */
						DUAL_REG_A_STRUCT *ptr_isp_reg = ptr_isp_reg_param->ptr_isp_reg;
						/* DUAL_ENGINE_LIST_HW_R_R */
						ptr_dual_in_config->CAM_C_AFO_BASE_ADDR.Raw = ptr_isp_reg->CAM_A_AFO_BASE_ADDR.Raw;
						ptr_dual_in_config->CAM_C_AFO_STRIDE.Raw = ptr_isp_reg->CAM_A_AFO_STRIDE.Raw;
						/* DUAL_ENGINE_LIST_HW_RW_RW */
						/* A to C in */
						/* A to C out */
						ptr_dual_out_config->CAM_C_CTL_EN.Raw = ptr_isp_reg->CAM_A_CTL_EN.Raw;
						ptr_dual_out_config->CAM_C_CTL_EN2.Raw = ptr_isp_reg->CAM_A_CTL_EN2.Raw;
						ptr_dual_out_config->CAM_C_CTL_DMA_EN.Raw = ptr_isp_reg->CAM_A_CTL_DMA_EN.Raw;
						ptr_dual_out_config->CAM_C_AF_BLK_1.Raw = ptr_isp_reg->CAM_A_AF_BLK_1.Raw;
						ptr_dual_out_config->CAM_C_AF_VLD.Raw = ptr_isp_reg->CAM_A_AF_VLD.Raw;
						ptr_dual_out_config->CAM_C_AFO_OFST_ADDR.Raw = ptr_isp_reg->CAM_A_AFO_OFST_ADDR.Raw;
						ptr_dual_out_config->CAM_C_AFO_XSIZE.Raw = ptr_isp_reg->CAM_A_AFO_XSIZE.Raw;
						ptr_dual_out_config->CAM_C_AFO_YSIZE.Raw = ptr_isp_reg->CAM_A_AFO_YSIZE.Raw;
						/* DUAL_ENGINE_LIST_HW_R_RW */
						/* A to C in */
						ptr_dual_in_config->CAM_C_CTL_SEL.Raw = ptr_isp_reg->CAM_A_CTL_SEL.Raw;
						/* A to C out */
						ptr_dual_out_config->CAM_C_AF_BLK_0.Raw = ptr_isp_reg->CAM_A_AF_BLK_0.Raw;
						ptr_dual_out_config->CAM_C_AF_BLK_2.Raw = ptr_isp_reg->CAM_A_AF_BLK_2.Raw;
						ptr_dual_out_config->CAM_C_AF_CON.Raw = ptr_isp_reg->CAM_A_AF_CON.Raw;
						ptr_dual_out_config->CAM_C_AF_CON2.Raw = ptr_isp_reg->CAM_A_AF_CON2.Raw;
						ptr_dual_out_config->CAM_C_AFO_DRS.Raw = ptr_isp_reg->CAM_A_AFO_DRS.Raw;
						ptr_dual_out_config->CAM_C_AFO_CON.Raw = ptr_isp_reg->CAM_A_AFO_CON.Raw;
						ptr_dual_out_config->CAM_C_AFO_CON2.Raw = ptr_isp_reg->CAM_A_AFO_CON2.Raw;
						ptr_dual_out_config->CAM_C_AFO_CON3.Raw = ptr_isp_reg->CAM_A_AFO_CON3.Raw;
						ptr_dual_out_config->CAM_C_AFO_CON4.Raw = ptr_isp_reg->CAM_A_AFO_CON4.Raw;
						/* DUAL_ENGINE_LIST_HW_R_W */
						/* A to C out */
					}
				}
			}
			else
			{
				if (dual_sel & 0x1)
				{
					DUAL_REG_A_STRUCT *ptr_isp_reg = ptr_isp_reg_param->ptr_isp_reg;
					/* copy in A to dual A main, A+B, A+B+C */
					DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg,,,,);
					DUAL_ENGINE_LIST_HW_R_R(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg,,,,);
					DUAL_ENGINE_LIST_HW_R_X(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg,,,,);
					DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg,,,,);
					DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_A_A, ptr_dual_out_config, ptr_isp_reg,,,,);
					DUAL_ENGINE_LIST_HW_R_W(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg,,,,);
					DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_A_A, ptr_dual_in_config, ptr_isp_reg,,,,);
					DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_A_A, ptr_dual_out_config, ptr_isp_reg,,,,);
					/* copy in A to dual A main, A+B, A+B+C, A to B */
					DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_A_B, ptr_dual_out_config, ptr_isp_reg,,,,);
					DUAL_ENGINE_LIST_HW_R_W(DUAL_COPY_DUAL_A_B, ptr_dual_out_config, ptr_isp_reg,,,,);
					if (dual_sel & 0x2)
					{
						DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
						/* copy in A/B to dual B sub, A+B, A+B+C */
						DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_B_B, ptr_dual_in_config, ptr_isp_reg_d,,,,);
						DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_B_B, ptr_dual_out_config, ptr_isp_reg_d,,,,);
						DUAL_ENGINE_LIST_HW_R_R(DUAL_COPY_DUAL_B_B, ptr_dual_in_config, ptr_isp_reg_d,,,,);
						DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_B_B, ptr_dual_in_config, ptr_isp_reg_d,,,,);
						DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_B_B, ptr_dual_out_config, ptr_isp_reg_d,,,,);
						if (dual_sel & 0x4)
						{
							DUAL_REG_C_STRUCT *ptr_isp_reg_t = ptr_isp_reg_param->ptr_isp_reg_t;
							/* copy in A/C to dual C sub, A+B+C, A to C */
							DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_A_C, ptr_dual_out_config, ptr_isp_reg,,,,);
							DUAL_ENGINE_LIST_HW_R_W(DUAL_COPY_DUAL_A_C, ptr_dual_out_config, ptr_isp_reg,,,,);
							/* copy in A/C to dual C sub, A+B+C, C to C */
							DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_C_C, ptr_dual_in_config, ptr_isp_reg_t,,,,);
							DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_C_C, ptr_dual_out_config, ptr_isp_reg_t,,,,);
							DUAL_ENGINE_LIST_HW_R_R(DUAL_COPY_DUAL_C_C, ptr_dual_in_config, ptr_isp_reg_t,,,,);
							DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_C_C, ptr_dual_in_config, ptr_isp_reg_t,,,,);
							DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_C_C, ptr_dual_out_config, ptr_isp_reg_t,,,,);
						}
					}
					else
					{
						DUAL_REG_C_STRUCT *ptr_isp_reg_t = ptr_isp_reg_param->ptr_isp_reg_t;
						/* copy in A/C to dual B sub, A+C */
						DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_C_B, ptr_dual_in_config, ptr_isp_reg_t,,,,);
						DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_C_B, ptr_dual_out_config, ptr_isp_reg_t,,,,);
						DUAL_ENGINE_LIST_HW_R_R(DUAL_COPY_DUAL_C_B, ptr_dual_in_config, ptr_isp_reg_t,,,,);
						DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_C_B, ptr_dual_in_config, ptr_isp_reg_t,,,,);
						DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_C_B, ptr_dual_out_config, ptr_isp_reg_t,,,,);
					}
				}
				else
				{
					DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
					/* copy in B to dual A main, B+C */
					DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_B_A, ptr_dual_in_config, ptr_isp_reg_d,,,,);
					DUAL_ENGINE_LIST_HW_R_R(DUAL_COPY_DUAL_B_A, ptr_dual_in_config, ptr_isp_reg_d,,,,);
					DUAL_ENGINE_LIST_HW_R_X(DUAL_COPY_DUAL_B_A, ptr_dual_in_config, ptr_isp_reg_d,,,,);
					DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_B_A, ptr_dual_in_config, ptr_isp_reg_d,,,,);
					DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_B_A, ptr_dual_out_config, ptr_isp_reg_d,,,,);
					DUAL_ENGINE_LIST_HW_R_W(DUAL_COPY_DUAL_B_A, ptr_dual_in_config, ptr_isp_reg_d,,,,);
					DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_B_A, ptr_dual_in_config, ptr_isp_reg_d,,,,);
					DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_B_A, ptr_dual_out_config, ptr_isp_reg_d,,,,);
					/* copy in B/C to dual B, B+C, B to C */
					DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_B_B, ptr_dual_out_config, ptr_isp_reg_d,,,,);
					DUAL_ENGINE_LIST_HW_R_W(DUAL_COPY_DUAL_B_B, ptr_dual_out_config, ptr_isp_reg_d,,,,);
				}
				if ((0x5 == dual_sel) || (0x6 == dual_sel) || (0x16 == dual_sel))
				{
					DUAL_REG_C_STRUCT *ptr_isp_reg_t = ptr_isp_reg_param->ptr_isp_reg_t;
					/* copy in B/C to dual B, B+C, C to B */
					DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_C_B, ptr_dual_in_config, ptr_isp_reg_t,,,,);
					DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_C_B, ptr_dual_out_config, ptr_isp_reg_t,,,,);
					DUAL_ENGINE_LIST_HW_R_R(DUAL_COPY_DUAL_C_B, ptr_dual_in_config, ptr_isp_reg_t,,,,);;
					DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_C_B, ptr_dual_in_config, ptr_isp_reg_t,,,,);
					DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_C_B, ptr_dual_out_config, ptr_isp_reg_t,,,,)
					if (dual_sel & 0x10)
					{
						DUAL_REG_A_STRUCT *ptr_isp_reg = ptr_isp_reg_param->ptr_isp_reg;
						DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
						/* copy in B/C/A to dual C sub, B+C+A, B to C */
						DUAL_ENGINE_LIST_HW_RW_W(DUAL_COPY_DUAL_B_C, ptr_dual_out_config, ptr_isp_reg_d,,,,);
						DUAL_ENGINE_LIST_HW_R_W(DUAL_COPY_DUAL_B_C, ptr_dual_out_config, ptr_isp_reg_d,,,,);
						/* copy in B/C/A to dual C sub, B+C+A, A to C */
						DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_A_C, ptr_dual_in_config, ptr_isp_reg,,,,);
						DUAL_ENGINE_LIST_HW_R_RW(DUAL_COPY_DUAL_A_C, ptr_dual_out_config, ptr_isp_reg,,,,);
						DUAL_ENGINE_LIST_HW_R_R(DUAL_COPY_DUAL_A_C, ptr_dual_in_config, ptr_isp_reg,,,,);
						DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_A_C, ptr_dual_in_config, ptr_isp_reg,,,,);
						DUAL_ENGINE_LIST_HW_RW_RW(DUAL_COPY_DUAL_A_C, ptr_dual_out_config, ptr_isp_reg,,,,);
					}
				}
			}
		}
		else
		{
			return 1;
		}
	}
	return 0;
}

int dual_convert_output(int dual_sel, ISP_REG_PTR_STRUCT *ptr_isp_reg_param, const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config,
						const DUAL_OUT_CONFIG_STRUCT *ptr_dual_out_config)
{
	/* update dual_in_config & dual_out_config */
	if (ptr_isp_reg_param && ptr_dual_in_config && ptr_dual_out_config)
	{
		/* update dual_in_config & dual_out_config */
		if ((ptr_isp_reg_param->ptr_isp_reg && (dual_sel & (0x10 + 0x1))) || (ptr_isp_reg_param->ptr_isp_reg_d && (dual_sel & 0x2)) ||
			(ptr_isp_reg_param->ptr_isp_reg_t && (dual_sel & 0x4)))
		{
			if (TWIN_SCENARIO_AF_FAST_P2 == ptr_dual_in_config->SW.TWIN_SCENARIO)
			{
				if (dual_sel & 0x1)
				{
					/* copy dual A main to out A, A+B, A+B+C */
					DUAL_REG_A_STRUCT *ptr_isp_reg = ptr_isp_reg_param->ptr_isp_reg;
					/* DUAL_ENGINE_LIST_HW_RW_RW */
					ptr_isp_reg->CAM_A_CTL_EN.Raw = ptr_dual_out_config->CAM_A_CTL_EN.Raw;
					ptr_isp_reg->CAM_A_CTL_EN2.Raw = ptr_dual_out_config->CAM_A_CTL_EN2.Raw;
					ptr_isp_reg->CAM_A_CTL_DMA_EN.Raw = ptr_dual_out_config->CAM_A_CTL_DMA_EN.Raw;
					ptr_isp_reg->CAM_A_AF_BLK_1.Raw = ptr_dual_out_config->CAM_A_AF_BLK_1.Raw;
					ptr_isp_reg->CAM_A_AF_VLD.Raw = ptr_dual_out_config->CAM_A_AF_VLD.Raw;
					ptr_isp_reg->CAM_A_AF_SIZE.Raw = ptr_dual_out_config->CAM_A_AF_SIZE.Raw;
					ptr_isp_reg->CAM_A_AFO_OFST_ADDR.Raw = ptr_dual_out_config->CAM_A_AFO_OFST_ADDR.Raw;
					ptr_isp_reg->CAM_A_AFO_XSIZE.Raw = ptr_dual_out_config->CAM_A_AFO_XSIZE.Raw;
					ptr_isp_reg->CAM_A_AFO_YSIZE.Raw = ptr_dual_out_config->CAM_A_AFO_YSIZE.Raw;
					if (dual_sel & 0x2)
					{
						/* copy dual B sub to out B, A+B, A+B+C */
						DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
						/* DUAL_ENGINE_LIST_HW_RW_RW */
						ptr_isp_reg_d->CAM_B_CTL_EN.Raw = ptr_dual_out_config->CAM_B_CTL_EN.Raw;
						ptr_isp_reg_d->CAM_B_CTL_EN2.Raw = ptr_dual_out_config->CAM_B_CTL_EN2.Raw;
						ptr_isp_reg_d->CAM_B_CTL_DMA_EN.Raw = ptr_dual_out_config->CAM_B_CTL_DMA_EN.Raw;
						ptr_isp_reg_d->CAM_B_AF_BLK_1.Raw = ptr_dual_out_config->CAM_B_AF_BLK_1.Raw;
						ptr_isp_reg_d->CAM_B_AF_VLD.Raw = ptr_dual_out_config->CAM_B_AF_VLD.Raw;
						ptr_isp_reg_d->CAM_B_AF_SIZE.Raw = ptr_dual_out_config->CAM_B_AF_SIZE.Raw;
						ptr_isp_reg_d->CAM_B_AFO_OFST_ADDR.Raw = ptr_dual_out_config->CAM_B_AFO_OFST_ADDR.Raw;
						ptr_isp_reg_d->CAM_B_AFO_XSIZE.Raw = ptr_dual_out_config->CAM_B_AFO_XSIZE.Raw;
						ptr_isp_reg_d->CAM_B_AFO_YSIZE.Raw = ptr_dual_out_config->CAM_B_AFO_YSIZE.Raw;
						/* DUAL_ENGINE_LIST_HW_R_RW */
						ptr_isp_reg_d->CAM_B_AF_BLK_0.Raw = ptr_dual_out_config->CAM_B_AF_BLK_0.Raw;
						ptr_isp_reg_d->CAM_B_AF_BLK_2.Raw = ptr_dual_out_config->CAM_B_AF_BLK_2.Raw;
						ptr_isp_reg_d->CAM_B_AF_CON.Raw = ptr_dual_out_config->CAM_B_AF_CON.Raw;
						ptr_isp_reg_d->CAM_B_AF_CON2.Raw = ptr_dual_out_config->CAM_B_AF_CON2.Raw;
						/* direct copy in A to out B, A+B, A+B+C */
						DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_A_B, ptr_isp_reg_d, ptr_isp_reg,,,,);
						if (dual_sel & 0x4)
						{
							/* copy dual C sub to out C, A+B+C */
							DUAL_REG_C_STRUCT *ptr_isp_reg_t = ptr_isp_reg_param->ptr_isp_reg_t;
							/* DUAL_ENGINE_LIST_HW_RW_RW */
							ptr_isp_reg_t->CAM_C_CTL_EN.Raw = ptr_dual_out_config->CAM_C_CTL_EN.Raw;
							ptr_isp_reg_t->CAM_C_CTL_EN2.Raw = ptr_dual_out_config->CAM_C_CTL_EN2.Raw;
							ptr_isp_reg_t->CAM_C_CTL_DMA_EN.Raw = ptr_dual_out_config->CAM_C_CTL_DMA_EN.Raw;
							ptr_isp_reg_t->CAM_C_AF_BLK_1.Raw = ptr_dual_out_config->CAM_C_AF_BLK_1.Raw;
							ptr_isp_reg_t->CAM_C_AF_VLD.Raw = ptr_dual_out_config->CAM_C_AF_VLD.Raw;
							ptr_isp_reg_t->CAM_C_AF_SIZE.Raw = ptr_dual_out_config->CAM_C_AF_SIZE.Raw;
							ptr_isp_reg_t->CAM_C_AFO_OFST_ADDR.Raw = ptr_dual_out_config->CAM_C_AFO_OFST_ADDR.Raw;
							ptr_isp_reg_t->CAM_C_AFO_XSIZE.Raw = ptr_dual_out_config->CAM_C_AFO_XSIZE.Raw;
							ptr_isp_reg_t->CAM_C_AFO_YSIZE.Raw = ptr_dual_out_config->CAM_C_AFO_YSIZE.Raw;
							/* DUAL_ENGINE_LIST_HW_R_RW */
							ptr_isp_reg_t->CAM_C_AF_BLK_0.Raw = ptr_dual_out_config->CAM_C_AF_BLK_0.Raw;
							ptr_isp_reg_t->CAM_C_AF_BLK_2.Raw = ptr_dual_out_config->CAM_C_AF_BLK_2.Raw;
							ptr_isp_reg_t->CAM_C_AF_CON.Raw = ptr_dual_out_config->CAM_C_AF_CON.Raw;
							ptr_isp_reg_t->CAM_C_AF_CON2.Raw = ptr_dual_out_config->CAM_C_AF_CON2.Raw;
							/* direct copy in A to out C, A+B+C */
							DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_A_C, ptr_isp_reg_t, ptr_isp_reg,,,,);
						}
					}
				}
				else
				{
					/* copy dual A main to out B, B+C */
					DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
					/* DUAL_ENGINE_LIST_HW_RW_RW */
					ptr_isp_reg_d->CAM_B_CTL_EN.Raw = ptr_dual_out_config->CAM_A_CTL_EN.Raw;
					ptr_isp_reg_d->CAM_B_CTL_EN2.Raw = ptr_dual_out_config->CAM_A_CTL_EN2.Raw;
					ptr_isp_reg_d->CAM_B_CTL_DMA_EN.Raw = ptr_dual_out_config->CAM_A_CTL_DMA_EN.Raw;
					ptr_isp_reg_d->CAM_B_AF_BLK_1.Raw = ptr_dual_out_config->CAM_A_AF_BLK_1.Raw;
					ptr_isp_reg_d->CAM_B_AF_VLD.Raw = ptr_dual_out_config->CAM_A_AF_VLD.Raw;
					ptr_isp_reg_d->CAM_B_AF_SIZE.Raw = ptr_dual_out_config->CAM_A_AF_SIZE.Raw;
					ptr_isp_reg_d->CAM_B_AFO_OFST_ADDR.Raw = ptr_dual_out_config->CAM_A_AFO_OFST_ADDR.Raw;
					ptr_isp_reg_d->CAM_B_AFO_XSIZE.Raw = ptr_dual_out_config->CAM_A_AFO_XSIZE.Raw;
					ptr_isp_reg_d->CAM_B_AFO_YSIZE.Raw = ptr_dual_out_config->CAM_A_AFO_YSIZE.Raw;
				}
				if ((0x5 == dual_sel) || (0x6 == dual_sel) || (0x16 == dual_sel))
				{
					/* copy dual B sub to out C, A+C, B+C */
					DUAL_REG_C_STRUCT *ptr_isp_reg_t = ptr_isp_reg_param->ptr_isp_reg_t;
					/* DUAL_ENGINE_LIST_HW_RW_RW */
					ptr_isp_reg_t->CAM_C_CTL_EN.Raw = ptr_dual_out_config->CAM_B_CTL_EN.Raw;
					ptr_isp_reg_t->CAM_C_CTL_EN2.Raw = ptr_dual_out_config->CAM_B_CTL_EN2.Raw;
					ptr_isp_reg_t->CAM_C_CTL_DMA_EN.Raw = ptr_dual_out_config->CAM_B_CTL_DMA_EN.Raw;
					ptr_isp_reg_t->CAM_C_AF_BLK_1.Raw = ptr_dual_out_config->CAM_B_AF_BLK_1.Raw;
					ptr_isp_reg_t->CAM_C_AF_VLD.Raw = ptr_dual_out_config->CAM_B_AF_VLD.Raw;
					ptr_isp_reg_t->CAM_C_AF_SIZE.Raw = ptr_dual_out_config->CAM_B_AF_SIZE.Raw;
					ptr_isp_reg_t->CAM_C_AFO_OFST_ADDR.Raw = ptr_dual_out_config->CAM_B_AFO_OFST_ADDR.Raw;
					ptr_isp_reg_t->CAM_C_AFO_XSIZE.Raw = ptr_dual_out_config->CAM_B_AFO_XSIZE.Raw;
					ptr_isp_reg_t->CAM_C_AFO_YSIZE.Raw = ptr_dual_out_config->CAM_B_AFO_YSIZE.Raw;
					/* DUAL_ENGINE_LIST_HW_R_RW */
					ptr_isp_reg_t->CAM_C_AF_BLK_0.Raw = ptr_dual_out_config->CAM_B_AF_BLK_0.Raw;
					ptr_isp_reg_t->CAM_C_AF_BLK_2.Raw = ptr_dual_out_config->CAM_B_AF_BLK_2.Raw;
					ptr_isp_reg_t->CAM_C_AF_CON.Raw = ptr_dual_out_config->CAM_B_AF_CON.Raw;
					ptr_isp_reg_t->CAM_C_AF_CON2.Raw = ptr_dual_out_config->CAM_B_AF_CON2.Raw;
					if (dual_sel & 0x1)
					{
						DUAL_REG_A_STRUCT *ptr_isp_reg = ptr_isp_reg_param->ptr_isp_reg;
						/* direct copy in A to out C, A+C */
						DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_A_C, ptr_isp_reg_t, ptr_isp_reg,,,,);
					}
					else
					{
						DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
						/* direct copy in B to out C, B+C */
						DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_B_C, ptr_isp_reg_t, ptr_isp_reg_d,,,,);
					}
					if (dual_sel & 0x10)
					{
						/* copy dual C sub to out A, B+C+A */
						DUAL_REG_A_STRUCT *ptr_isp_reg = ptr_isp_reg_param->ptr_isp_reg;
						/* DUAL_ENGINE_LIST_HW_RW_RW */
						ptr_isp_reg->CAM_A_CTL_EN.Raw = ptr_dual_out_config->CAM_C_CTL_EN.Raw;
						ptr_isp_reg->CAM_A_CTL_EN2.Raw = ptr_dual_out_config->CAM_C_CTL_EN2.Raw;
						ptr_isp_reg->CAM_A_CTL_DMA_EN.Raw = ptr_dual_out_config->CAM_C_CTL_DMA_EN.Raw;
						ptr_isp_reg->CAM_A_AF_BLK_1.Raw = ptr_dual_out_config->CAM_C_AF_BLK_1.Raw;
						ptr_isp_reg->CAM_A_AF_VLD.Raw = ptr_dual_out_config->CAM_C_AF_VLD.Raw;
						ptr_isp_reg->CAM_A_AF_SIZE.Raw = ptr_dual_out_config->CAM_C_AF_SIZE.Raw;
						ptr_isp_reg->CAM_A_AFO_OFST_ADDR.Raw = ptr_dual_out_config->CAM_C_AFO_OFST_ADDR.Raw;
						ptr_isp_reg->CAM_A_AFO_XSIZE.Raw = ptr_dual_out_config->CAM_C_AFO_XSIZE.Raw;
						ptr_isp_reg->CAM_A_AFO_YSIZE.Raw = ptr_dual_out_config->CAM_C_AFO_YSIZE.Raw;
						/* DUAL_ENGINE_LIST_HW_R_RW */
						ptr_isp_reg->CAM_A_AF_BLK_0.Raw = ptr_dual_out_config->CAM_C_AF_BLK_0.Raw;
						ptr_isp_reg->CAM_A_AF_BLK_2.Raw = ptr_dual_out_config->CAM_C_AF_BLK_2.Raw;
						ptr_isp_reg->CAM_A_AF_CON.Raw = ptr_dual_out_config->CAM_C_AF_CON.Raw;
						ptr_isp_reg->CAM_A_AF_CON2.Raw = ptr_dual_out_config->CAM_C_AF_CON2.Raw;
						/* direct copy in B to out A, B+C+A */
						DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_B_A, ptr_isp_reg_param->ptr_isp_reg, ptr_isp_reg_param->ptr_isp_reg_d,,,,);
					}
				}
			}
			else
			{
				if (dual_sel & 0x1)
				{
					DUAL_REG_A_STRUCT *ptr_isp_reg = ptr_isp_reg_param->ptr_isp_reg;
					/* copy dual A main to out A, A+B, A+B+C */
					DUAL_ENGINE_LIST_HW_OUT(DUAL_COPY_DUAL_A_A, ptr_isp_reg, ptr_dual_out_config,,,,);
					if (dual_sel & 0x2)
					{
						DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
						/* copy dual B sub to out B, A+B, A+B+C */
						DUAL_ENGINE_LIST_HW_OUT_D(DUAL_COPY_DUAL_B_B, ptr_isp_reg_d, ptr_dual_out_config,,,,);
						/* direct copy in A to out B, A+B, A+B+C */
						DUAL_ENGINE_ARRAY_HW(DUAL_COPY_ARRAY_A_B, ptr_isp_reg_d, ptr_isp_reg,,,,);
						/* direct copy in A to out B, A+B, A+B+C */
						DUAL_ENGINE_LIST_HW_RX_WX(DUAL_COPY_DUAL_A_B, ptr_isp_reg_d, ptr_isp_reg,,,,);
						if (TWIN_SCENARIO_NORMAL == ptr_dual_in_config->SW.TWIN_SCENARIO)
						{
							DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_A_B, ptr_isp_reg_d, ptr_isp_reg,,,,);
						}
						if (dual_sel & 0x4)
						{
							DUAL_REG_C_STRUCT *ptr_isp_reg_t = ptr_isp_reg_param->ptr_isp_reg_t;
							/* copy dual C sub to out C, A+B+C */
							DUAL_ENGINE_LIST_HW_OUT_D(DUAL_COPY_DUAL_C_C, ptr_isp_reg_t, ptr_dual_out_config,,,,);
							/* direct copy in A to out C, A+B+C */
							DUAL_ENGINE_ARRAY_HW(DUAL_COPY_ARRAY_A_C, ptr_isp_reg_t, ptr_isp_reg,,,,);
							/* direct copy in A to out C, A+B+C */
							DUAL_ENGINE_LIST_HW_RX_WX(DUAL_COPY_DUAL_A_C, ptr_isp_reg_t, ptr_isp_reg,,,,);
							if (TWIN_SCENARIO_NORMAL == ptr_dual_in_config->SW.TWIN_SCENARIO)
							{
								DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_A_C, ptr_isp_reg_t, ptr_isp_reg,,,,);
							}
						}
					}
				}
				else
				{
					DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
					/* copy dual A main to out B, B+C */
					DUAL_ENGINE_LIST_HW_OUT(DUAL_COPY_DUAL_A_B, ptr_isp_reg_d, ptr_dual_out_config,,,,);
				}
				if ((0x5 == dual_sel) || (0x6 == dual_sel) || (0x16 == dual_sel))
				{
					DUAL_REG_C_STRUCT *ptr_isp_reg_t = ptr_isp_reg_param->ptr_isp_reg_t;
					/* copy dual B sub to out C, A+C, B+C */
					DUAL_ENGINE_LIST_HW_OUT_D(DUAL_COPY_DUAL_B_C, ptr_isp_reg_t, ptr_dual_out_config,,,,);
					if (dual_sel & 0x1)
					{
						DUAL_REG_A_STRUCT *ptr_isp_reg = ptr_isp_reg_param->ptr_isp_reg;
						/* direct copy in A to out C, A+C */
						DUAL_ENGINE_ARRAY_HW(DUAL_COPY_ARRAY_A_C, ptr_isp_reg_t, ptr_isp_reg,,,,);
						/* direct copy in A to out C, A+C */
						DUAL_ENGINE_LIST_HW_RX_WX(DUAL_COPY_DUAL_A_C, ptr_isp_reg_t, ptr_isp_reg,,,,);
						if (TWIN_SCENARIO_NORMAL == ptr_dual_in_config->SW.TWIN_SCENARIO)
						{
							DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_A_C, ptr_isp_reg_t, ptr_isp_reg,,,,);
						}
					}
					else
					{
						DUAL_REG_B_STRUCT *ptr_isp_reg_d = ptr_isp_reg_param->ptr_isp_reg_d;
						/* direct copy in B to out C, B+C */
						DUAL_ENGINE_ARRAY_HW(DUAL_COPY_ARRAY_B_C, ptr_isp_reg_t, ptr_isp_reg_d,,,,);
						/* direct copy in B to out C, B+C */
						DUAL_ENGINE_LIST_HW_RX_WX(DUAL_COPY_DUAL_B_C, ptr_isp_reg_t, ptr_isp_reg_d,,,,);
						if (TWIN_SCENARIO_NORMAL == ptr_dual_in_config->SW.TWIN_SCENARIO)
						{
							DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_B_C, ptr_isp_reg_t, ptr_isp_reg_d,,,,);
						}
					}
					if (dual_sel & 0x10)
					{
						/* copy dual C sub to out A, B+C+A */
						DUAL_ENGINE_LIST_HW_OUT_D(DUAL_COPY_DUAL_C_A, ptr_isp_reg_param->ptr_isp_reg, ptr_dual_out_config,,,,);
						/* direct copy in B to out A, B+C+A */
						DUAL_ENGINE_ARRAY_HW(DUAL_COPY_ARRAY_B_A, ptr_isp_reg_param->ptr_isp_reg, ptr_isp_reg_param->ptr_isp_reg_d,,,,);
						/* direct copy in B to out A, B+C+A */
						DUAL_ENGINE_LIST_HW_RX_WX(DUAL_COPY_DUAL_B_A, ptr_isp_reg_param->ptr_isp_reg, ptr_isp_reg_param->ptr_isp_reg_d,,,,);
						if (TWIN_SCENARIO_NORMAL == ptr_dual_in_config->SW.TWIN_SCENARIO)
						{
							DUAL_ENGINE_LIST_HW_RX_WX_AF(DUAL_COPY_DUAL_B_A, ptr_isp_reg_param->ptr_isp_reg, ptr_isp_reg_param->ptr_isp_reg_d,,,,);
						}
					}
				}
			}
		}
	}
	return 0;
}

int dual_fprintf_in_config(int count, const DUAL_IN_CONFIG_STRUCT *ptr_dual_in_config)
{
	if (ptr_dual_in_config)
	{
		int dual_sel = ptr_dual_in_config->SW.DUAL_SEL;
		int dual_mode = DUAL_MODE_CAL(ptr_dual_in_config->SW.DUAL_SEL);
		if (dual_mode > 1)
		{
#ifdef DUAL_PLATFORM_DRIVER
			uart_printf("%s %s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_DEBUG_LIST(DUAL_UART_PRINT_TXT, uart_printf, ptr_dual_in_config,,,,);
			DUAL_SW_LIST(DUAL_UART_PRINT_SW_TXT, uart_printf, ptr_dual_in_config,,,,);
			DUAL_CTRL_LIST_HW(DUAL_UART_PRINT_TXT_HW, uart_printf, ptr_dual_in_config,,,,);
			DUAL_ENGINE_LIST_HW_IN(DUAL_UART_PRINT_TXT_ENGINE, uart_printf, ptr_dual_in_config, dual_sel,,,);
			DUAL_ENGINE_LIST_HW_IN_D(DUAL_UART_PRINT_TXT_ENGINE_D, uart_printf, ptr_dual_in_config, dual_sel,,,);
			if (dual_mode > 2)
			{
				DUAL_ENGINE_LIST_HW_IN_T(DUAL_UART_PRINT_TXT_ENGINE_T, uart_printf, ptr_dual_in_config,,,,);
			}
			uart_printf("%s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_END);
#else
			FILE *fpt_log=NULL;
			char full_name[DUAL_MAX_COMMAND_LENGTH];
			dual_sprintf(full_name, sizeof(full_name), "%s%s%s%d%s", DUAL_DEFAULT_PATH, DUAL_FOLDER_SYMBOL_STR,
				DUAL_IN_CONFIG_FILENAME, count, DUAL_IN_CONFIG_EXTNAME);
			dual_fopen(fpt_log, full_name, "wb");
			if (NULL == fpt_log)
			{
				dual_sprintf(full_name, sizeof(full_name), "%s%d%s",
					DUAL_IN_CONFIG_FILENAME, count, DUAL_IN_CONFIG_EXTNAME);
				dual_fopen(fpt_log, full_name, "wb");
				if (NULL == fpt_log)
				{
					dual_driver_printf("Error: %s\n", print_error_message(DUAL_MESSAGE_FILE_OPEN_ERROR));
					return 1;
				}
			}
			printf("Output file: %s\n", full_name);
			fprintf(fpt_log, "%s %s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_DEBUG_LIST(DUAL_LOG_PRINT_TXT, fpt_log, fprintf, ptr_dual_in_config,,,);
			DUAL_SW_LIST(DUAL_LOG_PRINT_SW_TXT, fpt_log, fprintf, ptr_dual_in_config,,,);
			DUAL_CTRL_LIST_HW(DUAL_LOG_PRINT_TXT_HW, fpt_log, fprintf, ptr_dual_in_config,,,);
			DUAL_ENGINE_LIST_HW_IN(DUAL_LOG_PRINT_TXT_ENGINE, fpt_log, fprintf, ptr_dual_in_config, dual_sel,,);
			DUAL_ENGINE_LIST_HW_IN_D(DUAL_LOG_PRINT_TXT_ENGINE_D, fpt_log, fprintf, ptr_dual_in_config, dual_sel,,);
			if (dual_mode > 2)
			{
				DUAL_ENGINE_LIST_HW_IN_T(DUAL_LOG_PRINT_TXT_ENGINE_T, fpt_log, fprintf, ptr_dual_in_config,,,);
			}
			/* input can check flag before printf */
			fprintf(fpt_log, "%s %s\n", DUAL_IN_DUMP_HEADER, DUAL_DUMP_END);
			fclose(fpt_log);
#endif
		}
	}
	return 0;
}

int dual_fprintf_reg_dump(int dual_sel, const ISP_REG_PTR_STRUCT *ptr_isp_reg_param)
{
	if (ptr_isp_reg_param)
	{
		if ((ptr_isp_reg_param->ptr_isp_reg && (dual_sel & (0x10 + 0x1))) || (ptr_isp_reg_param->ptr_isp_reg_d && (dual_sel & 0x2)) ||
			(ptr_isp_reg_param->ptr_isp_reg_t && (dual_sel & 0x4)))
		{
#ifdef DUAL_PLATFORM_DRIVER
			uart_printf("%s %s %s\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_CTRL_LIST_HW(DUAL_UART_REG_TXT_HW, uart_printf, ptr_isp_reg_param->ptr_isp_uni, DUAL_ISP_BASE_UNI,,,);
			/* can check flag before dump */
			if (dual_sel & (0x10 + 0x1))
			{
				/* A+B, A+C, A+B+C */
				DUAL_ENGINE_LIST_HW_ALL(DUAL_UART_REG_TXT_ENGINE, uart_printf, ptr_isp_reg_param->ptr_isp_reg, DUAL_ISP_BASE_HW,,,);
				DUAL_ENGINE_ARRAY_HW(DUAL_UART_REG_TXT_ARRAY, uart_printf, ptr_isp_reg_param->ptr_isp_reg, DUAL_ISP_BASE_HW,,,);
				if (dual_sel & 0x2)
				{
					/* A+B, A+B+C */
					DUAL_ENGINE_LIST_HW_D_ALL(DUAL_UART_REG_TXT_ENGINE_D, uart_printf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,,);
					DUAL_ENGINE_ARRAY_HW(DUAL_UART_REG_TXT_ARRAY_D, uart_printf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,,);
				}
			}
			else
			{
				/* B+C */
				DUAL_ENGINE_LIST_HW_ALL(DUAL_UART_REG_TXT_ENGINE_D, uart_printf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,,);
				DUAL_ENGINE_ARRAY_HW(DUAL_UART_REG_TXT_ARRAY_D, uart_printf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,,);
			}
			if (dual_sel & 0x4)
			{
				/* A+B+C, B+C */
				DUAL_ENGINE_LIST_HW_D_ALL(DUAL_UART_REG_TXT_ENGINE_T, uart_printf, ptr_isp_reg_param->ptr_isp_reg_t, DUAL_ISP_BASE_HW_T,,,);
				DUAL_ENGINE_ARRAY_HW(DUAL_UART_REG_TXT_ARRAY_T, uart_printf, ptr_isp_reg_param->ptr_isp_reg_t, DUAL_ISP_BASE_HW_T,,,);
			}
			uart_printf("%s %s\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_END);
#else
			FILE *fpt_log=NULL;
			char full_name[DUAL_MAX_COMMAND_LENGTH];
			dual_sprintf(full_name, sizeof(full_name), "%s%s%s", DUAL_DEFAULT_PATH, DUAL_FOLDER_SYMBOL_STR,
				DUAL_REG_DUMP_FILENAME);
			dual_fopen(fpt_log, full_name, "wb");
			if (NULL == fpt_log)
			{
				dual_sprintf(full_name, sizeof(full_name), "%s", DUAL_REG_DUMP_FILENAME);
				dual_fopen(fpt_log, full_name, "wb");
				if (NULL == fpt_log)
				{
					dual_driver_printf("Error: %s\n", print_error_message(DUAL_MESSAGE_FILE_OPEN_ERROR));
					return 1;
				}
			}
			printf("Output file: %s\n", full_name);
			fprintf(fpt_log, "%s %s %s\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_START, DUAL_DUMP_VERSION);
			DUAL_CTRL_LIST_HW(DUAL_LOG_REG_TXT_HW, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_uni, DUAL_ISP_BASE_UNI,,);
			/* can check flag before dump */
			if (dual_sel & (0x10 + 0x1))
			{
				/* A+B, A+C, A+B+C */
				DUAL_ENGINE_LIST_HW_ALL(DUAL_LOG_REG_TXT_ENGINE, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg, DUAL_ISP_BASE_HW,,);
				DUAL_ENGINE_ARRAY_HW(DUAL_LOG_REG_TXT_ARRAY, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg, DUAL_ISP_BASE_HW,,);
				if (dual_sel & 0x2)
				{
					/* A+B, A+B+C */
					DUAL_ENGINE_LIST_HW_D_ALL(DUAL_LOG_REG_TXT_ENGINE_D, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,);
					DUAL_ENGINE_ARRAY_HW(DUAL_LOG_REG_TXT_ARRAY_D, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,);
				}
			}
			else
			{
				/* B+C */
				DUAL_ENGINE_LIST_HW_ALL(DUAL_LOG_REG_TXT_ENGINE_D, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,);
				DUAL_ENGINE_ARRAY_HW(DUAL_LOG_REG_TXT_ARRAY_D, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg_d, DUAL_ISP_BASE_HW_D,,);
			}
			if (dual_sel & 0x4)
			{
				/* A+B+C, B+C */
				DUAL_ENGINE_LIST_HW_D_ALL(DUAL_LOG_REG_TXT_ENGINE_T, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg_t, DUAL_ISP_BASE_HW_T,,);
				DUAL_ENGINE_ARRAY_HW(DUAL_LOG_REG_TXT_ARRAY_T, fpt_log, fprintf, ptr_isp_reg_param->ptr_isp_reg_t, DUAL_ISP_BASE_HW_T,,);
			}
			/* clone to 3 engines */
			fprintf(fpt_log, "%s %s\n", DUAL_REG_DUMP_HEADER, DUAL_DUMP_END);
			fclose(fpt_log);
#endif
		}
	}
	return 0;
}

#ifndef DUAL_PLATFORM_DRIVER
static const char *dual_isp_print_error_message(DUAL_MESSAGE_ENUM n)
{
    TWIN_GET_ERROR_NAME(n);
}

static const char *dual_isp_get_current_file_name(const char *filename)
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
#endif
