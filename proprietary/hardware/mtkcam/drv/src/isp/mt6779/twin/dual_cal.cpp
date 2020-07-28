#include "MyUtils.h"


#define print_error_message(...) dual_cal_print_error_message(__VA_ARGS__)
#define get_current_file_name(...) dual_cal_get_current_file_name(__VA_ARGS__)

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

static DUAL_MESSAGE_ENUM dual_cal_sep_rrz_rmx(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param,
                                      DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

	int bin_sel_h_size = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		                 ((int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_E - (int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_S):
		                 ptr_in_param->SW.TWIN_RAWI_XSIZE;

    // For IMGO_R1 branch
    int imgo_pipe_cut_shift_b;
	int imgo_pipe_cut_shift_a;
    int imgo_pipe_left_loss_b;
	int imgo_pipe_left_loss_a;
    int imgo_pipe_right_loss_b;
	int imgo_pipe_right_loss_a;

    // For DM_R1 branch
    int yuv_pipe_cut_shift_b;
    int yuv_pipe_left_loss_b;
    int yuv_pipe_right_loss_b;

    // For RRZO_R1 branch
    int rrzo_pipe_cut_shift_b;
    int rrzo_pipe_cut_shift_a;
    int rrzo_pipe_left_loss_b;
    int rrzo_pipe_left_loss_a;
    int rrzo_pipe_right_loss_b;
    int rrzo_pipe_right_loss_a;

    // For AFO_R1 branch
    int afo_pipe_cut_shift_b;
    int afo_pipe_left_loss_b;
    int afo_pipe_right_loss_b;
    int afo_pipe_left_margin;
    int afo_pipe_right_margin;

    // For AAO R1 branch
    int aao_pipe_cut_shift_b;
    int aao_pipe_left_loss_b;
    int aao_pipe_right_loss_b;

    // For LTMSO R1 branch
    int ltmso_pipe_cut_shift_b;
    int ltmso_pipe_left_loss_b;
    int ltmso_pipe_right_loss_b;

    // For LCESO R1 branch
    int lceso_pipe_cut_shift_b;
    int lceso_pipe_left_loss_b;
    int lceso_pipe_right_loss_b;

    // For VPU branch
    int vpu_pipe_cut_shift_b;

    // For FLKO R1 branch
    int flko_pipe_cut_shift_b;
    int flko_pipe_left_loss_b;
    int flko_pipe_right_loss_b;

    // For TSFSO & LHSO R1 branch
    int tsfso_lhso_cut_shift_b;

    // For main pipe merge
    int main_pipe_cut_shift;
    int main_pipe_left_loss;
    int main_pipe_right_loss;

    int raw_max_line_wdith[2] = { DUAL_RAW_WD, DUAL_RAW_WD_D };
    int sep_r1_valid_width[2];

    int sep_r1_left_max;

    /* check SEP_R1 full size, align with CAMCTL_PIX_BUS_SEPI */
	//if (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_SEPI > 2)
	//{
	//	if (bin_sel_h_size & 0x3)
	//	{
	//		result = DUAL_MESSAGE_SEP_IN_ALIGN_ERROR;
	//		dual_driver_printf("Error: %s\n", print_error_message(result));
	//		return result;
	//	}
	//}
	//else
	{
		if (bin_sel_h_size & 0x1)
		{
			result = DUAL_MESSAGE_SEP_IN_ALIGN_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
	}

	/* Shift size by binning modules enable */
	bin_sel_h_size = (unsigned int)bin_sel_h_size >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
		                                              (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN +
		                                              (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN);

    /* Upddate R1A, R1B, R1C FMT_SEL and FMT2_SEL */
    ptr_out_param->CAMCTL_R1B_CAMCTL_FMT_SEL.Raw  = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Raw;
    ptr_out_param->CAMCTL_R1B_CAMCTL_FMT2_SEL.Raw = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Raw;
	ptr_out_param->CAMCTL_R1B_CAMCTL_FMT3_SEL.Raw = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT3_SEL.Raw;
	ptr_out_param->CAMCTL_R1B_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R3O = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R3O;
	ptr_out_param->CAMCTL_R1B_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R7O = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R7O;
	ptr_out_param->CAMCTL_R1B_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R8O = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R8O;

    // Calculate IMGO_R1 branch align and loss
    if ((ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL != 4) &&
		(0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL ||
         6 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL ||
		 7 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL ||
		 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL ||
		 2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL))
	{
        imgo_pipe_cut_shift_b  = 0;
        imgo_pipe_left_loss_b  = 0;
        imgo_pipe_right_loss_b = 0;
	}
	else if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN)
	{
        if ((ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAK_R1_EN || ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN) &&
	        ((ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL == 4) || (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL > 0 && ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL < 6)) &&
		    ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL && ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_CRP_R3_EN) ||
		     (2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL && ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_MRG_R3_EN)) &&
		     (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL || 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL))
        {
            imgo_pipe_cut_shift_a  = 1;
            imgo_pipe_left_loss_a  = 0;
            imgo_pipe_right_loss_a = 0;

			if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL == 4)
			{
                if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN ||
                    ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN ||
                    ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN)
				{
                    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
				}
			}

            if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL)
            {
                if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAK_R1_EN)
                {
                    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }
                if (DUAL_OUT_FMT_BAYER_8 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
                {
                    imgo_pipe_cut_shift_a = 1;
                }
                else if (DUAL_OUT_FMT_BAYER_12 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
                {
                    imgo_pipe_cut_shift_a = 2;
                }
                else if (DUAL_OUT_FMT_BAYER_10 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT ||
                         DUAL_OUT_FMT_BAYER_14 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
                {
                    imgo_pipe_cut_shift_a = 3;
                }
                else if ((DUAL_OUT_FMT_BAYER_8_2 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT) ||
    		             (DUAL_OUT_FMT_BAYER_10_2 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT) ||
    		             (DUAL_OUT_FMT_BAYER_12_2 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT) ||
    		             (DUAL_OUT_FMT_BAYER_14_2 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT))
    		    {
    		        imgo_pipe_cut_shift_a = 1;
                }
                else if (DUAL_OUT_FMT_MIPI_10 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
                {
                    imgo_pipe_cut_shift_a = 5;
                }
                else if (DUAL_OUT_FMT_BAYER_15 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
                {
                    imgo_pipe_cut_shift_a = 0;  // 16-bit bus width
                }
                else if (DUAL_OUT_FMT_BAYER_16 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
                {
                    imgo_pipe_cut_shift_a = 0;  // 16-bit bus width
                }
                else if (DUAL_OUT_FMT_BAYER_19 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
                {
                    imgo_pipe_cut_shift_a = 3;  // 32-bit bus width
                }
                else if (DUAL_OUT_FMT_BAYER_24 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
                {
                    imgo_pipe_cut_shift_a = 2;  // 32-bit bus width
                }
                else
                {
                    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }
            }
            else if (1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL)
            {
                if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN)
                {
                    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }

                imgo_pipe_cut_shift_a = 6;
            }

            if (2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL)
            {
                if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_MRG_R3_EN ||
                    0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R12_EN)
                {
                    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
                    dual_driver_printf("Error: %s\n", print_error_message(result));
                    return result;
                }

                if ((1 != ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL) &&
					(DUAL_OUT_FMT_MIPI_10 != ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT))
                {
                    imgo_pipe_cut_shift_a  += (int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R3O;
                }
				imgo_pipe_cut_shift_b  = ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R3O < 1)? 1: (int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R3O;
                imgo_pipe_left_loss_b  = 0;
                imgo_pipe_right_loss_b = 0;
            }
            else
            {
                imgo_pipe_cut_shift_b  = imgo_pipe_cut_shift_a;
                imgo_pipe_left_loss_b  = 0;
                imgo_pipe_right_loss_b = DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_b) - 2;
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
        imgo_pipe_cut_shift_b  = 0;
        imgo_pipe_left_loss_b  = 0;
        imgo_pipe_right_loss_b = 0;
    }

    // Calculate DM_R1 branch align and loss
    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN)
    {
        yuv_pipe_cut_shift_b  = 1;
        yuv_pipe_left_loss_b  = 0;
        yuv_pipe_right_loss_b = 0;
    }
    else
    {
        yuv_pipe_cut_shift_b  = 0;
        yuv_pipe_left_loss_b  = 0;
        yuv_pipe_right_loss_b = 0;
    }

    // Calculate RRZO_R1 and UFGO_R1 branch align and loss
    if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN)
    {
        /* RRZ ofst check */
		if (bin_sel_h_size <= ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST +
		                      ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
		                      (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST? 1: 0))
		{
		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
	    }

        if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN)
        {
			if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN)
		    {
		        if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_RLB_R1_EN)
	    	    {
	    	        result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    	        dual_driver_printf("Error: %s\n", print_error_message(result));
	    	        return result;
	    	    }

				// Mobile RRZ condition
			    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN &&
                    ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN)
                {
                    rrzo_pipe_left_loss_a  = 0;
                    rrzo_pipe_right_loss_a = 0;

			        if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
			        {
				        if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_UFG_R1_EN)
				        {
					        result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
					        dual_driver_printf("Error: %s\n", print_error_message(result));
					        return result;
				        }

					    if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
				        {
			                rrzo_pipe_cut_shift_a = 6;
				        }
				        else
				        {
					        rrzo_pipe_cut_shift_a = 7;
				        }
				    }
				    else
				    {
					    if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAKG_R1_EN)
				        {
					        result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
					        dual_driver_printf("Error: %s\n", print_error_message(result));
					        return result;
				        }

				        if (DUAL_OUT_FMT_FG_8 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
				        {
                            rrzo_pipe_cut_shift_a = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 3: DUAL_RLB_ALIGN_SHIFT;
				        }
				        else if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
				        {
					        rrzo_pipe_cut_shift_a = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 4: 3;
				        }
				        else
				        {
					        rrzo_pipe_cut_shift_a = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 5: 4;
				        }

				        rrzo_pipe_cut_shift_a += (int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R7O;
				    }

					rrzo_pipe_cut_shift_b  = ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R7O < 1)? 1: (int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R7O;
                    rrzo_pipe_left_loss_b  = 0;
                    rrzo_pipe_right_loss_b =  (((unsigned int)((((long long)(DUAL_RLB_ALIGN_PIX - 2) * (bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
					    ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST? 1: 0))) +
					    ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1) / ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1) >> 1) << 1;;
                }
                else
                {
                    rrzo_pipe_cut_shift_b  = 1;
                    rrzo_pipe_left_loss_b  = 0;
                    rrzo_pipe_right_loss_b = 0;

			        if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
			        {
				        if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_UFG_R1_EN)
				        {
					        result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
					        dual_driver_printf("Error: %s\n", print_error_message(result));
					        return result;
				        }

			            if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
				        {
				            /* default 12b */
					        rrzo_pipe_right_loss_b = (((unsigned int)((((long long)(64 - 2) * (bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
						        ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST? 1: 0))) +
						        ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1) / ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1) >> 1) << 1;
				        }
				        else
				        {
					        rrzo_pipe_right_loss_b = (((unsigned int)((((long long)(128 - 2) * (bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
					            ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST? 1: 0))) +
						        ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1) / ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1) >> 1) << 1;
				        }
			        }
			        else
			        {
					    if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAKG_R1_EN)
				        {
					        result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
					        dual_driver_printf("Error: %s\n", print_error_message(result));
					        return result;
				        }

				        if (DUAL_OUT_FMT_FG_8 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
				        {
					        /* min RLB align */
					        rrzo_pipe_right_loss_b = (((unsigned int)(((long long)((ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 16: DUAL_RLB_ALIGN_PIX) - 2) *
					            (bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
						        (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST? 1: 0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1) /
						        ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1) >> 1) << 1;
				        }
				        else if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
				        {
					        rrzo_pipe_right_loss_b = (((unsigned int)(((long long)((ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 32: DUAL_RLB_ALIGN_PIX) - 2) *
					            (bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
						        (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST? 1: 0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1) /
						        ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1) >> 1) << 1;
				        }
				        else
				        {
					        /* new 32b */
					        rrzo_pipe_right_loss_b = (((unsigned int)(((long long)((ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT?32: 8) - 2) *
						        (bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST +
						        (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST? 1: 0))) + ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1) /
						        ptr_in_param->SW.TWIN_RRZ_OUT_WD + 1) >> 1) << 1;
				        }
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
		    if (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R8O > 1)
			{
			    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN)
				{
				    rrzo_pipe_right_loss_b = (((unsigned int)((((long long)(DUAL_RLB_ALIGN_PIX - 2) * (bin_sel_h_size - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
					    ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST + (ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST? 1: 0))) +
					    ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1) / ptr_in_param->SW.TWIN_RRZ_OUT_WD) + 1) >> 1) << 1;
				}
			    else
				{
				    rrzo_pipe_right_loss_b = 2;
				}
			}
		}

        // Merge from DM_R1A_SEL_1
        if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN && 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)
        {
            rrzo_pipe_cut_shift_b  = (rrzo_pipe_cut_shift_b >= yuv_pipe_cut_shift_b)? rrzo_pipe_cut_shift_b: yuv_pipe_cut_shift_b;
            rrzo_pipe_left_loss_b  = (rrzo_pipe_left_loss_b >= yuv_pipe_left_loss_b)? rrzo_pipe_left_loss_b: yuv_pipe_left_loss_b;
            rrzo_pipe_right_loss_b = (rrzo_pipe_right_loss_b >= yuv_pipe_right_loss_b)? rrzo_pipe_right_loss_b: yuv_pipe_right_loss_b;
        }

		rrzo_pipe_right_loss_b += DUAL_TILE_LOSS_RRZ;
		rrzo_pipe_left_loss_b  =  DUAL_TILE_LOSS_RRZ_L;
    }
    else
    {
        rrzo_pipe_cut_shift_b  = 0;
        rrzo_pipe_left_loss_b  = 0;
        rrzo_pipe_right_loss_b = 0;
    }

    // Calculate AFO_R1 branch align and loss
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN && 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_AF_SEL)
    {
        if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN)
        {
            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
            dual_driver_printf("Error: %s\n", print_error_message(result));
            return result;
        }
        else
        {
			if (ptr_in_param->AF_R1A_AF_CON2.Bits.AF_DS_EN && ptr_in_param->AF_R1A_AF_CON.Bits.AF_H_GONLY)
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}

            afo_pipe_cut_shift_b = 1;

			/* AF FAST P1 */
			if (TWIN_SCENARIO_AF_FAST_P1 == ptr_in_param->SW.TWIN_SCENARIO)
			{
				if ((int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}
				else
				{
					afo_pipe_left_loss_b  = DUAL_AF_TAPS * 2;
					afo_pipe_right_loss_b = DUAL_AF_TAPS * 2 + 2;
					afo_pipe_left_margin  = DUAL_AF_LEFT_MARGIN;
					afo_pipe_right_margin = DUAL_AF_RGIHT_MARGIN;
				}
			}
			else
			{
				afo_pipe_left_loss_b  = (ptr_in_param->AF_R1A_AF_CON.Bits.AF_H_GONLY || ptr_in_param->AF_R1A_AF_CON2.Bits.AF_DS_EN)? (DUAL_AF_TAPS * 2): DUAL_AF_TAPS;
				afo_pipe_right_loss_b = ptr_in_param->AF_R1A_AF_CON.Bits.AF_H_GONLY? (DUAL_AF_TAPS * 2): (ptr_in_param->AF_R1A_AF_CON2.Bits.AF_DS_EN? (DUAL_AF_TAPS * 2 + 2): DUAL_AF_TAPS);
				if ((int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH)
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}

                afo_pipe_left_margin  = DUAL_AF_LEFT_MARGIN;
                afo_pipe_right_margin = DUAL_AF_RGIHT_MARGIN;
			}
        }
    }
    else
    {
        afo_pipe_cut_shift_b  = 1;
        afo_pipe_left_loss_b  = DUAL_AF_TAPS * 2;
        afo_pipe_right_loss_b = DUAL_AF_TAPS * 2 + 2;
        afo_pipe_left_margin  = DUAL_AF_LEFT_MARGIN;
        afo_pipe_right_margin = DUAL_AF_RGIHT_MARGIN;
    }

    // Calculate AAO_R1 branch align and loss
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AAO_R1_EN && 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_AA_SEL)
    {
        aao_pipe_cut_shift_b  = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_QBN_R1_EN? (1 + ptr_in_param->QBIN_R1A_QBIN_CTL.Bits.QBIN_ACC): 1;
        aao_pipe_left_loss_b  = 0;
        aao_pipe_right_loss_b = DUAL_ALIGN_PIXEL_MODE(aao_pipe_cut_shift_b) - 2;;
    }
    else
    {
        aao_pipe_cut_shift_b  = 0;
        aao_pipe_left_loss_b  = 0;
        aao_pipe_right_loss_b = 0;
    }

    // Calculate LTMSO_R1 branch align and loss
    if(1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_LTMS_SEL)
    {
        ltmso_pipe_cut_shift_b  = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_QBN_R4_EN? (1 + ptr_in_param->QBIN_R4A_QBIN_CTL.Bits.QBIN_ACC): 1;
        ltmso_pipe_left_loss_b  = 0;
        ltmso_pipe_right_loss_b = DUAL_ALIGN_PIXEL_MODE(ltmso_pipe_cut_shift_b) - 2;;
    }
    else
    {
        ltmso_pipe_cut_shift_b  = 0;
        ltmso_pipe_left_loss_b  = 0;
        ltmso_pipe_right_loss_b = 0;
    }

    // Calculate LCESO_R1 branch align and loss
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_LCESO_R1_EN && 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_LCES_SEL)
    {
        lceso_pipe_cut_shift_b  = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_QBN_R5_EN? (1 + ptr_in_param->QBIN_R5A_QBIN_CTL.Bits.QBIN_ACC): 1;
        lceso_pipe_left_loss_b  = 0;
        lceso_pipe_right_loss_b = DUAL_ALIGN_PIXEL_MODE(lceso_pipe_cut_shift_b) - 2;
    }
    else
    {
        lceso_pipe_cut_shift_b  = 0;
        lceso_pipe_left_loss_b  = 0;
        lceso_pipe_right_loss_b = 0;
    }

    // Calculate FLKO_R1 branch align and loss
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_FLKO_R1_EN && 5 != ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_FLK_SEL)
    {
        flko_pipe_cut_shift_b  = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_QBN_R3_EN? (1 + ptr_in_param->QBIN_R3A_QBIN_CTL.Bits.QBIN_ACC): 1;
        flko_pipe_left_loss_b  = 0;
        flko_pipe_right_loss_b = DUAL_ALIGN_PIXEL_MODE(flko_pipe_cut_shift_b) - 2;;
    }
    else
    {
        flko_pipe_cut_shift_b  = 0;
        flko_pipe_left_loss_b  = 0;
        flko_pipe_right_loss_b = 0;
    }

    /******************************************** Merge point after LTM_R1A ********************************************/
    // Merge RRZO_R1 branch
    main_pipe_cut_shift  = (rrzo_pipe_cut_shift_b >= lceso_pipe_cut_shift_b)? rrzo_pipe_cut_shift_b: lceso_pipe_cut_shift_b;
    main_pipe_left_loss  = (rrzo_pipe_left_loss_b >= lceso_pipe_left_loss_b)? rrzo_pipe_left_loss_b: lceso_pipe_left_loss_b;
    main_pipe_right_loss = (rrzo_pipe_right_loss_b >= lceso_pipe_right_loss_b)? rrzo_pipe_right_loss_b: lceso_pipe_right_loss_b;

    // Merge AFO_R1 branch
    if(1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_AF_SEL &&
       0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R1_SEL)
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= afo_pipe_cut_shift_b)? main_pipe_cut_shift: afo_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= (afo_pipe_left_loss_b + afo_pipe_left_margin))? main_pipe_left_loss: (afo_pipe_left_loss_b + afo_pipe_left_margin);
        main_pipe_right_loss = (main_pipe_right_loss >= (afo_pipe_right_loss_b + afo_pipe_right_margin))? main_pipe_right_loss: (afo_pipe_right_loss_b + afo_pipe_right_margin);
    }

    // Merge IMGO_R1 && CRZO_R1 && CRZO_R2 branch
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN &&
       1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL || 2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL) &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL || 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL))
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= imgo_pipe_cut_shift_b)? main_pipe_cut_shift: imgo_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= imgo_pipe_left_loss_b)? main_pipe_left_loss: imgo_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= imgo_pipe_right_loss_b)? main_pipe_right_loss: imgo_pipe_right_loss_b;
    }

    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN && 0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)
    {
		main_pipe_cut_shift  = (main_pipe_cut_shift >= yuv_pipe_cut_shift_b)? main_pipe_cut_shift: yuv_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= yuv_pipe_left_loss_b)? main_pipe_left_loss: yuv_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= yuv_pipe_right_loss_b)? main_pipe_right_loss: yuv_pipe_right_loss_b;
    }

    if (1) // Always reserve for LTM: ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_LTM_R1_EN
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= 1)? main_pipe_cut_shift: 1;
        main_pipe_left_loss  = main_pipe_left_loss  + 2;
        main_pipe_right_loss = main_pipe_right_loss + 2;
    }

    /******************************************** Merge point after HLR_R1A ********************************************/
    main_pipe_cut_shift  = (main_pipe_cut_shift >= ltmso_pipe_cut_shift_b)? main_pipe_cut_shift: ltmso_pipe_cut_shift_b;
    main_pipe_left_loss  = (main_pipe_left_loss >= ltmso_pipe_left_loss_b)? main_pipe_left_loss: ltmso_pipe_left_loss_b;
    main_pipe_right_loss = (main_pipe_right_loss >= ltmso_pipe_right_loss_b)? main_pipe_right_loss: ltmso_pipe_right_loss_b;

    // Merge IMGO_R1 branch
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN &&
       2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL || 2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL) &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL || 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL))
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= imgo_pipe_cut_shift_b)? main_pipe_cut_shift: imgo_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= imgo_pipe_left_loss_b)? main_pipe_left_loss: imgo_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= imgo_pipe_right_loss_b)? main_pipe_right_loss: imgo_pipe_right_loss_b;
    }

    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_HLR_R1_EN)
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= 1)? main_pipe_cut_shift: 1;
        main_pipe_left_loss  = main_pipe_left_loss + 2;
        main_pipe_right_loss = main_pipe_right_loss + 2;
    }

    /******************************************** Merge point after LSC_R1A ********************************************/
    // Merge AAO_R1 branch
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AAO_R1_EN &&
       1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_AA_SEL)
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= aao_pipe_cut_shift_b)? main_pipe_cut_shift: aao_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= aao_pipe_left_loss_b)? main_pipe_left_loss: aao_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= aao_pipe_right_loss_b)? main_pipe_right_loss: aao_pipe_right_loss_b;
    }

    // Merge AFO_R1 branch
    if(1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_AF_SEL &&
       1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R1_SEL)
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= afo_pipe_cut_shift_b)? main_pipe_cut_shift: afo_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= (afo_pipe_left_loss_b + afo_pipe_left_margin))? main_pipe_left_loss: (afo_pipe_left_loss_b + afo_pipe_left_margin);
        main_pipe_right_loss = (main_pipe_right_loss >= (afo_pipe_right_loss_b + afo_pipe_right_margin))? main_pipe_right_loss: (afo_pipe_right_loss_b + afo_pipe_right_margin);
    }

    // Merge FLKO_R1 branch
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_FLKO_R1_EN && 0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_FLK_SEL)
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= flko_pipe_cut_shift_b)? main_pipe_cut_shift: flko_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= flko_pipe_left_loss_b)? main_pipe_left_loss: flko_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= flko_pipe_right_loss_b)? main_pipe_right_loss: flko_pipe_right_loss_b;
    }

    // Merge IMGO_R1 branch
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN &&
       3 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL || 2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL) &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL || 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL))
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= imgo_pipe_cut_shift_b)? main_pipe_cut_shift: imgo_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= imgo_pipe_left_loss_b)? main_pipe_left_loss: imgo_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= imgo_pipe_right_loss_b)? main_pipe_right_loss: imgo_pipe_right_loss_b;
    }

    /******************************************** Merge point after DGN_R1A ********************************************/
    // Merge TSFSO_R1/LHSO_R1
    tsfso_lhso_cut_shift_b = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_QBN_R2_EN? (1 + ptr_in_param->QBIN_R2A_QBIN_CTL.Bits.QBIN_ACC): 1;;

    main_pipe_cut_shift  = (main_pipe_cut_shift >= tsfso_lhso_cut_shift_b)? main_pipe_cut_shift: tsfso_lhso_cut_shift_b;

    // Merge IMGO_R1 branch
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN &&
       4 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL || 2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL) &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL || 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL))
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= imgo_pipe_cut_shift_b)? main_pipe_cut_shift: imgo_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= imgo_pipe_left_loss_b)? main_pipe_left_loss: imgo_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= imgo_pipe_right_loss_b)? main_pipe_right_loss: imgo_pipe_right_loss_b;
    }

    if(ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_RNR_R1_EN)
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= 1)? main_pipe_cut_shift: 1;
        main_pipe_left_loss  = main_pipe_left_loss + 6;
        main_pipe_right_loss = main_pipe_right_loss + 6;
    }

    /******************************************** Merge point after DGN_R1A_SEL ********************************************/
    // Merge IMGO_R1 branch
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN &&
       5 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL || 2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL) &&
       (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL || 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL))
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= imgo_pipe_cut_shift_b)? main_pipe_cut_shift: imgo_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= imgo_pipe_left_loss_b)? main_pipe_left_loss: imgo_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= imgo_pipe_right_loss_b)? main_pipe_right_loss: imgo_pipe_right_loss_b;
    }

    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_FLKO_R1_EN && 5 != ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_FLK_SEL)
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= flko_pipe_cut_shift_b)? main_pipe_cut_shift: flko_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= flko_pipe_left_loss_b)? main_pipe_left_loss: flko_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= flko_pipe_right_loss_b)? main_pipe_right_loss: flko_pipe_right_loss_b;
    }

    // Merge for FBND_R1
    main_pipe_cut_shift  = (main_pipe_cut_shift <= 3)? (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BPC_R1_EN? 3: main_pipe_cut_shift): main_pipe_cut_shift;
    main_pipe_left_loss  += ((ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_FUS_R1_EN? 4: 0)   /* FUS */ +
                             (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BPC_R1_EN? 4: 0)   /* BPC */ +
                             (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_OBC_R1_EN? 6: 0)); /* OBC */
    main_pipe_right_loss += ((ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_FUS_R1_EN? 4: 0)   /* FUS */ +
                             (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BPC_R1_EN? 4: 0)   /* BPC */ +
                             (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_OBC_R1_EN? 6: 0)); /* OBC */

    // Merge for SEP_R1
    main_pipe_cut_shift  = (main_pipe_cut_shift >= 1)? main_pipe_cut_shift: 1;
    main_pipe_left_loss  += ((ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IRM_R1_EN? 4: 0)    /* IRM */  +
                             (ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IOBC_R1_EN? 8: 0)   /* IOBC */ +
                             (ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IBPC_R1_EN? 4: 0)); /* IBPC */
    main_pipe_right_loss += ((ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IRM_R1_EN? 4: 0)    /* IRM */  +
                             (ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IOBC_R1_EN? 8: 0)   /* IOBC */ +
                             (ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IBPC_R1_EN? 4: 0)); /* IBPC */

    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN && 4 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)
    {
        main_pipe_cut_shift  = (main_pipe_cut_shift >= imgo_pipe_cut_shift_b)? main_pipe_cut_shift: imgo_pipe_cut_shift_b;
        main_pipe_left_loss  = (main_pipe_left_loss >= imgo_pipe_left_loss_b)? main_pipe_left_loss: imgo_pipe_left_loss_b;
        main_pipe_right_loss = (main_pipe_right_loss >= imgo_pipe_right_loss_b)? main_pipe_right_loss: imgo_pipe_right_loss_b;
    }

    int sep_cut_shift_self = (((int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PIX_BUS_SEPI >=
                             (int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_SEPO)?
                            ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PIX_BUS_SEPI:
                            ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_SEPO);
	main_pipe_cut_shift = (main_pipe_cut_shift > sep_cut_shift_self)? main_pipe_cut_shift: sep_cut_shift_self;

    sep_r1_valid_width[0] = (bin_sel_h_size + main_pipe_left_loss - main_pipe_right_loss) / 2 + (DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - 1) -
		                    (((bin_sel_h_size + main_pipe_left_loss - main_pipe_right_loss) / 2 + (DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - 1)) & (DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - 1));
    sep_r1_valid_width[1] = bin_sel_h_size - sep_r1_valid_width[0];

    // setup SEP_R1A
    ptr_out_param->SEP_R1A_SEP_CTL.Bits.SEP_EDGE = 0xF;
    ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X = 0;
    if ((sep_r1_valid_width[0] + main_pipe_right_loss) & (DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - 1))
    {
        ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X = sep_r1_valid_width[0] + main_pipe_right_loss - 1 + DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) -
            ((sep_r1_valid_width[0] + main_pipe_right_loss) & (DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - 1));
    }
    else
    {
        ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X = sep_r1_valid_width[0] + main_pipe_right_loss - 1;
    }

    if (((int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X + raw_max_line_wdith[0] <= (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X) ||
        ((int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X >= sep_r1_valid_width[0] + sep_r1_valid_width[1]))
    {
        result = DUAL_MESSAGE_SEP_SIZE_CAL_ERROR;
        dual_driver_printf("Error: %s\n", print_error_message(result));
        return result;
    }

    // Check min size
    sep_r1_left_max = sep_r1_valid_width[0] + DUAL_TILE_LOSS_ALL;

    // setup SEP_R1B
    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_SEP_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_SEP_R1_EN;
    ptr_out_param->SEP_R1B_SEP_CTL.Bits.SEP_EDGE = 0xF;
    if (sep_r1_valid_width[0] <= main_pipe_left_loss)
    {
        result = DUAL_MESSAGE_SEP_SIZE_CAL_ERROR;
        dual_driver_printf("Error: %s\n", print_error_message(result));
        return result;
    }
    else
    {
        if ((sep_r1_valid_width[0] - main_pipe_left_loss) & (DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - 1))
        {
            ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X = sep_r1_valid_width[0] - main_pipe_left_loss -
                ((sep_r1_valid_width[0] - main_pipe_left_loss) & (DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - 1));
        }
        else
        {
            ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X = sep_r1_valid_width[0] - main_pipe_left_loss;
        }
    }
    /* A+B, SEP_R1B SEP_END_X */
    ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X = sep_r1_valid_width[0] + sep_r1_valid_width[1] - 1;
    if (((int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X + raw_max_line_wdith[1] <= (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X) ||
        ((int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X >= sep_r1_valid_width[0] + sep_r1_valid_width[1]))
    {
        result = DUAL_MESSAGE_SEP_SIZE_CAL_ERROR;
        dual_driver_printf("Error: %s\n", print_error_message(result));
        return result;
    }

    /* MRG_R2 */
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_TSFSO_R1_EN)
    {
	    ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_STR_X = 0;
	    if (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(tsfso_lhso_cut_shift_b) - 1))
	    {
	    	ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1 + DUAL_ALIGN_PIXEL_MODE(tsfso_lhso_cut_shift_b) -
	    		(sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(tsfso_lhso_cut_shift_b) - 1));
	    }
	    else
	    {
	    	ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1;
	    }
	    /* check min size */
	    if (sep_r1_left_max < (int)ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1)
	    {
	    	sep_r1_left_max = (int)ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1;
	    }
	    ptr_out_param->MRG_R2A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R2A_MRG_VSIZE.Bits.MRG_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		    ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		    ((unsigned int)ptr_in_param->SW.TWIN_RAWI_YSIZE >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

	    /* add MRG_SIG_MODE1 & 2 */
	    ptr_out_param->MRG_R2A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
	    ptr_out_param->MRG_R2A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    if ((ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_STR_X & (DUAL_ALIGN_PIXEL_MODE(tsfso_lhso_cut_shift_b) - 1)) ||
	    	(((int)ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(tsfso_lhso_cut_shift_b) - 1)))
	    {
	    	result = DUAL_MESSAGE_MRG_R2_SIZE_CAL_ERROR;
	    	dual_driver_printf("Error: %s\n", print_error_message(result));
	    	return result;
	    }

        /* MRG_R2B */
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_MRG_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_MRG_R2_EN;
	    ptr_out_param->MRG_R2B_MRG_CROP.Bits.MRG_STR_X = (int)ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    ptr_out_param->MRG_R2B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R2B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R2A_MRG_VSIZE.Bits.MRG_HT;

    	/* add MRG_SIG_MODE1 & 2 */
    	ptr_out_param->MRG_R2B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
    	ptr_out_param->MRG_R2B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
    	/* A+B, MRG_R2B MRG_END_X */
    	ptr_out_param->MRG_R2B_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;

	    if ((ptr_out_param->MRG_R2B_MRG_CROP.Bits.MRG_STR_X & (DUAL_ALIGN_PIXEL_MODE(tsfso_lhso_cut_shift_b) - 1)) ||
	    	(((int)ptr_out_param->MRG_R2B_MRG_CROP.Bits.MRG_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(tsfso_lhso_cut_shift_b) - 1)))
	    {
	    	result = DUAL_MESSAGE_MRG_R2_SIZE_CAL_ERROR;
	    	dual_driver_printf("Error: %s\n", print_error_message(result));
	    	return result;
	    }
    }

    /* MRG_R1 */
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AAO_R1_EN && 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_AA_SEL)
    {
	    ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_STR_X = 0;
	    if (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(aao_pipe_cut_shift_b) - 1))
	    {
	    	ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1 + DUAL_ALIGN_PIXEL_MODE(aao_pipe_cut_shift_b) -
	    		(sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(aao_pipe_cut_shift_b) - 1));
	    }
	    else
	    {
	    	ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1;
	    }
	    /* check min size */
	    if (sep_r1_left_max < (int)ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1)
	    {
	    	sep_r1_left_max = (int)ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1;
	    }
	    ptr_out_param->MRG_R1A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R1A_MRG_VSIZE.Bits.MRG_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		    ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		    ((unsigned int)ptr_in_param->SW.TWIN_RAWI_YSIZE >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

	    /* add MRG_SIG_MODE1 & 2 */
	    ptr_out_param->MRG_R1A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
	    ptr_out_param->MRG_R1A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    if ((ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_STR_X & (DUAL_ALIGN_PIXEL_MODE(aao_pipe_cut_shift_b) - 1)) ||
	    	(((int)ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(aao_pipe_cut_shift_b) - 1)))
	    {
	    	result = DUAL_MESSAGE_MRG_R1_SIZE_CAL_ERROR;
	    	dual_driver_printf("Error: %s\n", print_error_message(result));
	    	return result;
	    }
	    /* MRG_R1B */
	    ptr_out_param->MRG_R1B_MRG_CROP.Bits.MRG_STR_X = (int)ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_MRG_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R1_EN;
	    ptr_out_param->MRG_R1B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R1B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R1A_MRG_VSIZE.Bits.MRG_HT;
	    /* add MRG_SIG_MODE1 & 2 */
    	/* update MRG_SIG_MODE1 & 2 */
    	ptr_out_param->MRG_R1B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
    	ptr_out_param->MRG_R1B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
    	/* A+B, MRG_R1B MRG_END_X */
    	ptr_out_param->MRG_R1B_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    if (((int)ptr_out_param->MRG_R1B_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->MRG_R1B_MRG_CROP.Bits.MRG_STR_X) & (DUAL_ALIGN_PIXEL_MODE(aao_pipe_cut_shift_b) - 1))
	    {
	    	result = DUAL_MESSAGE_MRG_R1_SIZE_CAL_ERROR;
	    	dual_driver_printf("Error: %s\n", print_error_message(result));
	    	return result;
	    }
	}

    /* MRG_R4 */
    if(1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_LTMS_SEL)
    {
	    ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_STR_X = 0;
	    if (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(ltmso_pipe_cut_shift_b) - 1))
	    {
		    ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1 + DUAL_ALIGN_PIXEL_MODE(ltmso_pipe_cut_shift_b) -
			    (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(ltmso_pipe_cut_shift_b) - 1));
	    }
	    else
	    {
		    ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1;
	    }
	    /* check min size */
	    if (sep_r1_left_max < (int)ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1)
	    {
		    sep_r1_left_max = (int)ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1;
	    }
	    ptr_out_param->MRG_R4A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R4A_MRG_VSIZE.Bits.MRG_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		    ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		    ((unsigned int)ptr_in_param->SW.TWIN_RAWI_YSIZE >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

	    /* add MRG_SIG_MODE1 & 2 */
	    ptr_out_param->MRG_R4A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
	    ptr_out_param->MRG_R4A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    if ((ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_STR_X & (DUAL_ALIGN_PIXEL_MODE(ltmso_pipe_cut_shift_b) - 1)) ||
		    (((int)ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(ltmso_pipe_cut_shift_b) - 1)))
	    {
		    result = DUAL_MESSAGE_MRG_R4_SIZE_CAL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }

	    /* MRG_R4B */
	    ptr_out_param->MRG_R4B_MRG_CROP.Bits.MRG_STR_X = (int)ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_MRG_R4_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R4_EN;
	    ptr_out_param->MRG_R4B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R4B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R4A_MRG_VSIZE.Bits.MRG_HT;
	    /* add MRG_SIG_MODE1 & 2 */
	    /* update MRG_SIG_MODE1 & 2 */
	    ptr_out_param->MRG_R4B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
	    ptr_out_param->MRG_R4B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    /* A+B, MRG_R4B MRG_END_X */
	    ptr_out_param->MRG_R4B_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    if (((int)ptr_out_param->MRG_R4B_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->MRG_R4B_MRG_CROP.Bits.MRG_STR_X) & (DUAL_ALIGN_PIXEL_MODE(ltmso_pipe_cut_shift_b) - 1))
	    {
		    result = DUAL_MESSAGE_MRG_R4_SIZE_CAL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }
    }

    /* MRG_R5 */
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_LCESO_R1_EN && 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_LCES_SEL)
    {
	    ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_STR_X = 0;
	    if (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(lceso_pipe_cut_shift_b) - 1))
	    {
		    ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1 + DUAL_ALIGN_PIXEL_MODE(lceso_pipe_cut_shift_b) -
			    (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(lceso_pipe_cut_shift_b) - 1));
	    }
	    else
	    {
		    ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1;
	    }
	    /* check min size */
	    if (sep_r1_left_max < (int)ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1)
	    {
		    sep_r1_left_max = (int)ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1;
	    }
	    ptr_out_param->MRG_R5A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R5A_MRG_VSIZE.Bits.MRG_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		    ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		    ((unsigned int)ptr_in_param->SW.TWIN_RAWI_YSIZE >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

	    /* add MRG_SIG_MODE1 & 2 */
	    ptr_out_param->MRG_R5A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
	    ptr_out_param->MRG_R5A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    if ((ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_STR_X & (DUAL_ALIGN_PIXEL_MODE(lceso_pipe_cut_shift_b) - 1)) ||
		    (((int)ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(lceso_pipe_cut_shift_b) - 1)))
	    {
		    result = DUAL_MESSAGE_MRG_R5_SIZE_CAL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }

	    /* MRG_R5B */
	    ptr_out_param->MRG_R5B_MRG_CROP.Bits.MRG_STR_X = (int)ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_MRG_R5_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R5_EN;
	    ptr_out_param->MRG_R5B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R5B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R5A_MRG_VSIZE.Bits.MRG_HT;
	    /* add MRG_SIG_MODE1 & 2 */
		/* update MRG_SIG_MODE1 & 2 */
		ptr_out_param->MRG_R5B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
		ptr_out_param->MRG_R5B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
		/* A+B, MRG_R5B MRG_END_X */
		ptr_out_param->MRG_R5B_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    if (((int)ptr_out_param->MRG_R5B_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->MRG_R5B_MRG_CROP.Bits.MRG_STR_X) & (DUAL_ALIGN_PIXEL_MODE(lceso_pipe_cut_shift_b) - 1))
	    {
		    result = DUAL_MESSAGE_MRG_R5_SIZE_CAL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }
	}

    vpu_pipe_cut_shift_b = 1;

    /* MRG_R6 */
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R6_EN)
    {
	    ptr_out_param->MRG_R6A_MRG_CROP.Bits.MRG_STR_X = 0;
	    if (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(vpu_pipe_cut_shift_b) - 1))
	    {
		    ptr_out_param->MRG_R6A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1 + DUAL_ALIGN_PIXEL_MODE(vpu_pipe_cut_shift_b) -
			    (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(vpu_pipe_cut_shift_b) - 1));
	    }
	    else
	    {
		    ptr_out_param->MRG_R6A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1;
	    }
	    /* check min size */
	    if (sep_r1_left_max < (int)ptr_out_param->MRG_R6A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1)
	    {
		    sep_r1_left_max = (int)ptr_out_param->MRG_R6A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1;
	    }
	    ptr_out_param->MRG_R6A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R6A_MRG_VSIZE.Bits.MRG_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		    ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		    ((unsigned int)ptr_in_param->SW.TWIN_RAWI_YSIZE >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

	    /* add MRG_SIG_MODE1 & 2 */
	    ptr_out_param->MRG_R6A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
	    ptr_out_param->MRG_R6A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    if ((ptr_out_param->MRG_R6A_MRG_CROP.Bits.MRG_STR_X & (DUAL_ALIGN_PIXEL_MODE(vpu_pipe_cut_shift_b) - 1)) ||
		    (((int)ptr_out_param->MRG_R6A_MRG_CROP.Bits.MRG_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(vpu_pipe_cut_shift_b) - 1)))
	    {
		    result = DUAL_MESSAGE_MRG_R6_SIZE_CAL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }

	    /* MRG_R6B */
	    ptr_out_param->MRG_R6B_MRG_CROP.Bits.MRG_STR_X = (int)ptr_out_param->MRG_R6A_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R6_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R6_EN;
	    ptr_out_param->MRG_R6B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R6B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R6A_MRG_VSIZE.Bits.MRG_HT;
	    /* add MRG_SIG_MODE1 & 2 */
		/* update MRG_SIG_MODE1 & 2 */
		ptr_out_param->MRG_R6B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
		ptr_out_param->MRG_R6B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
		/* A+B, MRG_R6B MRG_END_X */
		ptr_out_param->MRG_R6B_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    if (((int)ptr_out_param->MRG_R6B_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->MRG_R6B_MRG_CROP.Bits.MRG_STR_X) & (DUAL_ALIGN_PIXEL_MODE(vpu_pipe_cut_shift_b) - 1))
	    {
		    result = DUAL_MESSAGE_MRG_R6_SIZE_CAL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }
	}

    /* MRG_R10 */
    if(ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_FLKO_R1_EN && 5 != ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_FLK_SEL)
    {
	    ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_STR_X = 0;
	    if (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(flko_pipe_cut_shift_b) - 1))
	    {
		    ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1 + DUAL_ALIGN_PIXEL_MODE(flko_pipe_cut_shift_b) -
			    (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(flko_pipe_cut_shift_b) - 1));
	    }
	    else
	    {
		    ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1;
	    }
	    /* check min size */
	    if (sep_r1_left_max < (int)ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1)
	    {
		    sep_r1_left_max = (int)ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_END_X + DUAL_TILE_LOSS_ALL + 1;
	    }
	    ptr_out_param->MRG_R10A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R10A_MRG_VSIZE.Bits.MRG_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		    ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		    ((unsigned int)ptr_in_param->SW.TWIN_RAWI_YSIZE >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

	    /* add MRG_SIG_MODE1 & 2 */
	    ptr_out_param->MRG_R10A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
	    ptr_out_param->MRG_R10A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    if ((ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_STR_X & (DUAL_ALIGN_PIXEL_MODE(flko_pipe_cut_shift_b) - 1)) ||
		    (((int)ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_END_X + 1) & (DUAL_ALIGN_PIXEL_MODE(flko_pipe_cut_shift_b) - 1)))
	    {
		    result = DUAL_MESSAGE_MRG_R10_SIZE_CAL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }

	    /* MRG_R10B */
	    ptr_out_param->MRG_R10B_MRG_CROP.Bits.MRG_STR_X = (int)ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_MRG_R10_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R10_EN;
	    ptr_out_param->MRG_R10B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    ptr_out_param->MRG_R10B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R10A_MRG_VSIZE.Bits.MRG_HT;
	    /* add MRG_SIG_MODE1 & 2 */
		/* update MRG_SIG_MODE1 & 2 */
		ptr_out_param->MRG_R10B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
		ptr_out_param->MRG_R10B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
		/* A+B, MRG_R10B MRG_END_X */
		ptr_out_param->MRG_R10B_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X -
		    (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	    if (((int)ptr_out_param->MRG_R10B_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->MRG_R10B_MRG_CROP.Bits.MRG_STR_X) & (DUAL_ALIGN_PIXEL_MODE(flko_pipe_cut_shift_b) - 1))
	    {
		    result = DUAL_MESSAGE_MRG_R10_SIZE_CAL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }
	}

    /* AF & AFO */
    if(1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_AF_SEL)
    {
        ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_CRP_R1_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R1_SEL;
		ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_AF_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_AF_SEL;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_CRP_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_CRP_R1_EN;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN;

	    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_CRP_R1_EN)
	    {
		    /* CRP_R1A */
		    ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XSTART = 0;
		    ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND = sep_r1_valid_width[0] + afo_pipe_right_loss_b + afo_pipe_right_margin - 1;
		    ptr_out_param->CRP_R1A_CRP_Y_POS.Bits.CRP_YSTART = 0;
		    ptr_out_param->CRP_R1A_CRP_Y_POS.Bits.CRP_YEND = ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN) - 1;

		    /* CRP_R1B */
		    ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XSTART = sep_r1_valid_width[0] - afo_pipe_left_loss_b - afo_pipe_left_margin - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
			ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XEND = sep_r1_valid_width[0] + sep_r1_valid_width[1] - 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
		    ptr_out_param->CRP_R1B_CRP_Y_POS.Bits.CRP_YSTART = 0;
		    ptr_out_param->CRP_R1B_CRP_Y_POS.Bits.CRP_YEND = ptr_out_param->CRP_R1A_CRP_Y_POS.Bits.CRP_YEND;

         	/* AF & AFO */
		    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN)
		    {
			    /* AF_R1A & AF_R1B */
			    if (((int)ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND + 1 < DUAL_AF_MIN_IN_WIDTH) ||
				    ((int)ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XEND - (int)ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XSTART + 1 < DUAL_AF_MIN_IN_WIDTH))
			    {
				    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				    dual_driver_printf("Error: %s\n", print_error_message(result));
				    return result;
			    }

                /* RTL verif or platform */
			    if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
			    {
				    /* normal */
				    if (TWIN_SCENARIO_NORMAL == ptr_in_param->SW.TWIN_SCENARIO)
				    {
					    /* platform */
					    if ((ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE != ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE) ||
						    ((int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH) ||
						    ((sep_r1_valid_width[0] + sep_r1_valid_width[1]) < ptr_in_param->SW.TWIN_AF_OFFSET +
						    (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE * ptr_in_param->SW.TWIN_AF_BLOCK_XNUM) || (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM <= 0))
					    {
						    result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						    dual_driver_printf("Error: %s\n", print_error_message(result));
						    return result;
					    }
					    /* check AF_A, minimize SEP_R1A_END_X */
					    if (sep_r1_valid_width[0] <= ptr_in_param->SW.TWIN_AF_OFFSET)
					    {
						    /* B+C only */
						    ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_CRP_R1_EN = 0;
						    ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = 0;
						    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = 0;
					    }
					    else
					    {
                            if ((sep_r1_valid_width[0] + DUAL_AF_RGIHT_MARGIN) < (ptr_in_param->SW.TWIN_AF_OFFSET + (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE))
							{
								/* B+C only */
							    ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_CRP_R1_EN = 0;
						        ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = 0;
						        ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = 0;
							}
							else
							{
						        /* A+B , cover cross region by afo_pipe_right_margin */
						        ptr_out_param->AF_R1A_AF_VLD.Bits.AF_VLD_XSTART = ptr_in_param->SW.TWIN_AF_OFFSET;
						        /* check AF_A output all */
						        if (((int)ptr_in_param->SW.TWIN_AF_BLOCK_XNUM * (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE + (int)ptr_in_param->SW.TWIN_AF_OFFSET) <= (int)(sep_r1_valid_width[0] + DUAL_AF_RGIHT_MARGIN))
						        {
							        /* A only */
							        ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
						        }
						        else
						        {
							        /* B+C, minimize SEP_R1A SEP_END_X */
							        ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = (sep_r1_valid_width[0] + DUAL_AF_RGIHT_MARGIN - ptr_in_param->SW.TWIN_AF_OFFSET) / (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE;
						        }

						        /* reduce RCP size */
						        ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND = (int)ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XSTART +
							        (int)ptr_out_param->AF_R1A_AF_VLD.Bits.AF_VLD_XSTART + afo_pipe_right_loss_b +
							        (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM * (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
						        /* protect AF min size */
						        if ((int)ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND + 1 < DUAL_AF_MIN_IN_WIDTH)
						        {
							        ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND = DUAL_AF_MIN_IN_WIDTH - 1;
						        }
					        }
						}
				    }
			    }
			    else
			    {
				    /* RTL different base address */
				    ptr_out_param->AF_R1A_AF_VLD.Bits.AF_VLD_XSTART = 0;
				    if (sep_r1_valid_width[0] < (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE * (int)ptr_in_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM)
				    {
					    ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = sep_r1_valid_width[0] / (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE;
				    }
				    /* reduce RCP size */
				    ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND = (int)ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XSTART +
				        (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM * (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
				    /* protect AF min size */
				    if ((int)ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND + 1 < DUAL_AF_MIN_IN_WIDTH)
				    {
					    ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND = DUAL_AF_MIN_IN_WIDTH - 1;
				    }
			    }

			    /* Normal case */
			    if (TWIN_SCENARIO_NORMAL == ptr_in_param->SW.TWIN_SCENARIO)
			    {
				    if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN)
				    {
					    /* check max 128 */
					    if (ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
					    {
						    if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
						    {
							    result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
							    dual_driver_printf("Error: %s\n", print_error_message(result));
							    return result;
						    }
						    else
						    {
							    /* RTL */
							    ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
						    }
					    }
				    }
				    /* RTL verif or platform */
				    if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
				    {
					    if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN)
					    {
						    /* A+B */
						    if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM > (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM)
						    {
							    /* A+B */
							    ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_XSTART = ptr_in_param->SW.TWIN_AF_OFFSET +
								    (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM * (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE -
								     (sep_r1_valid_width[0] - afo_pipe_left_loss_b - afo_pipe_left_margin);
							    ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM - (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM;
						    }
						    else
						    {
							    /* A only */
							    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_CRP_R1_EN = 0;
							    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = 0;
							    ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = 0;
						    }
					    }
					    else
					    {
						    /* B only */
						    ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_XSTART = ptr_in_param->SW.TWIN_AF_OFFSET - (sep_r1_valid_width[0] - afo_pipe_left_loss_b - afo_pipe_left_margin);
						    ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
					    }

					    ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_YSTART = ptr_in_param->AF_R1A_AF_VLD.Bits.AF_VLD_YSTART;
					    ptr_out_param->AF_R1B_AF_CON.Raw = ptr_in_param->AF_R1A_AF_CON.Raw;
					    ptr_out_param->AF_R1B_AF_CON2.Raw = ptr_in_param->AF_R1A_AF_CON2.Raw;
					    ptr_out_param->AF_R1B_AF_BLK_0.Raw = ptr_in_param->AF_R1A_AF_BLK_0.Raw;
					    ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_YNUM = ptr_in_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_YNUM;
					    ptr_out_param->AF_R1B_AF_BLK_PROT.Raw = ptr_in_param->AF_R1A_AF_BLK_PROT.Raw;
					    ptr_out_param->AFO_R1B_AFO_DRS.Raw = ptr_in_param->AFO_R1A_AFO_DRS.Raw;
					    ptr_out_param->AFO_R1B_AFO_CON.Raw = ptr_in_param->AFO_R1A_AFO_CON.Raw;
					    ptr_out_param->AFO_R1B_AFO_CON2.Raw = ptr_in_param->AFO_R1A_AFO_CON2.Raw;
					    ptr_out_param->AFO_R1B_AFO_CON3.Raw = ptr_in_param->AFO_R1A_AFO_CON3.Raw;
					    ptr_out_param->AFO_R1B_AFO_CON4.Raw = ptr_in_param->AFO_R1A_AFO_CON4.Raw;
				    }
				    else
				    {
					    /* RTL, different base address */
					    ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_XSTART = 0;
					    if ((int)ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XEND - (int)ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XSTART + 1 <
					        (int)ptr_in_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM * (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE)
					    {
						    ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = ((int)ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XEND -
							    (int)ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XSTART + 1) / (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE;
					    }
				    }

				    if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN)
				    {
					    /* check max 128 */
					    if (ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
					    {
						    if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
						    {
							    result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
							    dual_driver_printf("Error: %s\n", print_error_message(result));
							    return result;
						    }
						    else
						    {
							    /* RTL */
							    ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
						    }
					    }
				    }
				    if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN)
				    {
					    if (ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE < DUAL_AF_BLOCK_BYTE)
					    {
						    result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						    dual_driver_printf("Error: %s\n", print_error_message(result));
						    return result;
					    }
					    if (ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN)
					    {
						    ptr_out_param->AFO_R1A_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR = 0;
						    ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE = (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM * DUAL_AF_BLOCK_BYTE - 1;
						    /* check stride */
						    if ((unsigned int)ptr_out_param->AFO_R1A_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR + (unsigned int)ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE + 1 >
							    (unsigned int)ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE)
						    {
							    if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
							    {
								    result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
								    dual_driver_printf("Error: %s\n", print_error_message(result));
								    return result;
							    }
							    else
							    {
								    /* RTL */
								    ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE -
									    (unsigned int)ptr_out_param->AFO_R1A_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR) / DUAL_AF_BLOCK_BYTE;
								    ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE = DUAL_AF_BLOCK_BYTE * (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM - 1;
							    }
						    }
					    }
					    if (ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN)
					    {
						    ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE = (int)ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM * DUAL_AF_BLOCK_BYTE - 1;
						    if ((int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE < DUAL_AF_BLOCK_BYTE)
						    {
							    result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
							    dual_driver_printf("Error: %s\n", print_error_message(result));
							    return result;
						    }
						    /* RTL verif or platform */
						    if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
						    {
							    ptr_out_param->AFO_R1B_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR = ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN?
								    ((int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM * DUAL_AF_BLOCK_BYTE): 0;
							    ptr_out_param->AFO_R1B_AFO_YSIZE.Raw = ptr_in_param->AFO_R1A_AFO_YSIZE.Raw;
						    }
						    if ((unsigned int)ptr_out_param->AFO_R1B_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR + (unsigned int)ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE + 1 >
							    (unsigned int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE)
						    {
							    if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
							    {
								    result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
								    dual_driver_printf("Error: %s\n", print_error_message(result));
								    return result;
							    }
							    else
							    {
								    /* RTL */
								    ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE -
									    (unsigned int)ptr_out_param->AFO_R1B_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR) / DUAL_AF_BLOCK_BYTE;
								    ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE = DUAL_AF_BLOCK_BYTE * (int)ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM  - 1;
							    }
						    }
						    if ((ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN? ((unsigned int)ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE + 1): 0) +
							    (unsigned int)ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE + 1 > (unsigned int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE)
						    {
							    if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
							    {
								    result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
								    dual_driver_printf("Error: %s\n", print_error_message(result));
								    return result;
							    }
						    }
					    }

					    /* AFO_A padding */
					    if ((0 == ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN) &&
						    (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR))
					    {
						    if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1) * DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE)
						    {
							    ptr_out_param->AF_R1A_AF_VLD.Bits.AF_VLD_XSTART = 0;
							    ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
							    ptr_out_param->AFO_R1A_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR = DUAL_AF_BLOCK_BYTE * ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
							    ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE = DUAL_AF_BLOCK_BYTE - 1;
							    ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_CRP_R1_EN = 1;
							    ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = 1;
							    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = 1;
							    ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XSTART = 0;
							    ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND = (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
							    /* protect AF min size */
							    if ((int)ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND + 1 < DUAL_AF_MIN_IN_WIDTH)
							    {
								    ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND = DUAL_AF_MIN_IN_WIDTH - 1;
							    }
						    }
					    }
					    if (ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN)
					    {
						    /* error check A */
						    if ((3 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_GONLY))
						    {
							    if (ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
							    {
								    result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								    dual_driver_printf("Error: %s\n", print_error_message(result));
								    return result;
							    }
						    }
						    else if ((3 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_AVG_LVL) ||
							         ((2 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_AVG_LVL) &&
							          (1 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_GONLY)))
						    {
							    if (ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
							    {
								    result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								    dual_driver_printf("Error: %s\n", print_error_message(result));
								    return result;
							    }
						    }
						    else
						    {
							    if (ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
							    {
								    result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								    dual_driver_printf("Error: %s\n", print_error_message(result));
								    return result;
							    }
						    }
					    }
					    /* AFO_B padding */
					    if ((0 == ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN) &&
						    (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR))
					    {
						    if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1)* DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE)
						    {
					 		    ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_XSTART = 0;
							    ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
							    ptr_out_param->AFO_R1B_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR = DUAL_AF_BLOCK_BYTE * ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
							    ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE = DUAL_AF_BLOCK_BYTE - 1;
							    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_CRP_R1_EN = 1;
							    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = 1;
							    ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = 1;
							    ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XSTART = 0;
							    ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XEND = (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE - 1;
							    /* protect AF min size */
							    if ((int)ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XEND + 1 < DUAL_AF_MIN_IN_WIDTH)
							    {
								    ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XEND = DUAL_AF_MIN_IN_WIDTH - 1;
							    }
						    }
					    }
					    if (ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN)
					    {
						    /* error check B */
						    if ((3 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_GONLY))
						    {
							    if (ptr_out_param->AF_R1B_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
							    {
								    result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								    dual_driver_printf("Error: %s\n", print_error_message(result));
								    return result;
							    }
						    }
						    else if ((3 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_AVG_LVL) ||
							         ((2 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_AVG_LVL) &&
							          (1 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_GONLY)))
						    {
							    if (ptr_out_param->AF_R1B_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
							    {
								    result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								    dual_driver_printf("Error: %s\n", print_error_message(result));
								    return result;
							    }
						    }
						    else
						    {
							    if (ptr_out_param->AF_R1B_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
							    {
								    result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
								    dual_driver_printf("Error: %s\n", print_error_message(result));
								    return result;
							    }
						    }
					    }
				    }
				    /* sync CRP_R1 size & AF_IMAGE_WD */
				    ptr_out_param->AF_R1A_AF_SIZE.Bits.AF_IMAGE_WD = (int)ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND + 1;
				    ptr_out_param->AF_R1B_AF_SIZE.Bits.AF_IMAGE_WD = (int)ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XEND - (int)ptr_out_param->CRP_R1B_CRP_X_POS.Bits.CRP_XSTART + 1;
			    }
            }

			/* minimize sep */
		    if (sep_r1_left_max < (int)ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND + 1 + DUAL_TILE_LOSS_AF_B)
		    {
		        sep_r1_left_max = (int)ptr_out_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND + 1 + DUAL_TILE_LOSS_AF_B;
		    }
	    }
    }

    /* CRP_R3 & MRG_R3 & MRG_R12 & IMGO */
	if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN &&
	    (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAK_R1_EN || ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN) &&
	    ((ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL == 4) || (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL > 0 && ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL < 6)) &&
		((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL && ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_CRP_R3_EN) ||
		 (2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL && ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_MRG_R3_EN)) &&
		(0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL || 1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL))
	{
	    ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_CRP_R3_SEL;

	    int bit_per_pix = 8;
	    int bus_width_shift = 1;/* 2*8, 16 bits */

        /* IMGO */
        if (1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL)
	    {
	        bus_width_shift = 3; /* 2*8, 64 bits */
	        switch (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
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
		    switch (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT)
   	        {
			    case DUAL_OUT_FMT_BAYER_8:
				    bit_per_pix = 8;
					break;
    		    case DUAL_OUT_FMT_BAYER_10:/* bayer 10 */
    			    bit_per_pix = 10;
    			    break;
    		    case DUAL_OUT_FMT_MIPI_10:
    			    bus_width_shift = 3; /* 8*8, 64 bits */
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
    			case DUAL_OUT_FMT_BAYER_15:
    			    bit_per_pix = 16;
    			    break;
    			case DUAL_OUT_FMT_BAYER_16:
    			    bit_per_pix = 16;
    			    break;
    			case DUAL_OUT_FMT_BAYER_19:
    			    bus_width_shift = 2;
    			    bit_per_pix = 20;
    			    break;
    			case DUAL_OUT_FMT_BAYER_24:
    			    bus_width_shift = 2;
    			    bit_per_pix = 24;
    			    break;
    			default:
    			    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		            dual_driver_printf("Error: %s\n", print_error_message(result));
		            return result;
   	        }
   	    }

	    if (2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL)
	    {
			int mrg_r3_end_x;

			if ((1 != ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL) &&
				(DUAL_OUT_FMT_MIPI_10 != ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_IMGO_R1_FMT))
			{
                bus_width_shift += (int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R3O;
			}

	        /* MRG_R3 && MRG_R12 */
			ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R12_EN = 0;
	        ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_STR_X = 0;
		    ptr_out_param->MRG_R3A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	        ptr_out_param->MRG_R3A_MRG_VSIZE.Bits.MRG_HT = (unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN;
	        if (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_a) - 1))
	        {
				/* add MRG_SIG_MODE1 & 2 */
				ptr_out_param->MRG_R3A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
				ptr_out_param->MRG_R3A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;

	            mrg_r3_end_x = sep_r1_valid_width[0] - 1 + DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_a) -
			        (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_a) - 1));

		        ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1;
			    ptr_out_param->MRG_R12B_MRG_CROP.Bits.MRG_STR_X = (int)ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_END_X + 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
			    ptr_out_param->MRG_R12B_MRG_CROP.Bits.MRG_END_X = ptr_out_param->MRG_R12B_MRG_CROP.Bits.MRG_STR_X + (mrg_r3_end_x - (int)ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_END_X) - 1;
			    ptr_out_param->MRG_R12B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
	            ptr_out_param->MRG_R12B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
			    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R12_EN = 1;
				ptr_out_param->MRG_R12B_MRG_CTL.Bits.MRG_EDGE = 0xF;
				ptr_out_param->MRG_R12B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R3A_MRG_VSIZE.Bits.MRG_HT;
				if (((int)ptr_out_param->MRG_R12B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R12B_MRG_CROP.Bits.MRG_STR_X + 1) >= DULL_MRG_R12B_MAX_WD)
	    	    {
	    	        result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    	        dual_driver_printf("Error: %s\n", print_error_message(result));
	    	        return result;
	            }
	        }
	        else
	        {
				/* add MRG_SIG_MODE1 & 2 */
				ptr_out_param->MRG_R3A_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
				ptr_out_param->MRG_R3A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
		        ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1;
		        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R12_EN = 0;

				mrg_r3_end_x = ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_END_X;
	        }

	        if ((ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_STR_X & (DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_a) - 1)) ||
		        (((int)mrg_r3_end_x + 1) & (DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_a) - 1)))
	        {
		        result = DUAL_MESSAGE_MRG_R3_SIZE_CAL_ERROR;
		        dual_driver_printf("Error: %s\n", print_error_message(result));
		        return result;
	        }

			/* IMGO_R1A */
			ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE = DUAL_CALC_BUS_XIZE(mrg_r3_end_x - (int)ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_STR_X + 1, bit_per_pix, bus_width_shift);
	        if ((mrg_r3_end_x - (int)ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_STR_X + 1) & (DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_a) - 1))
	        {
		        result = DUAL_MESSAGE_MRG_R3_SIZE_CAL_ERROR;
		        dual_driver_printf("Error: %s\n", print_error_message(result));
		        return result;
	        }

	        /* MRG_R3B */
	        ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_MRG_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_MRG_R3_EN;
	        ptr_out_param->MRG_R3B_MRG_CROP.Bits.MRG_STR_X = mrg_r3_end_x + 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	        ptr_out_param->MRG_R3B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	        ptr_out_param->MRG_R3B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R3A_MRG_VSIZE.Bits.MRG_HT;

	        /* add MRG_SIG_MODE1 & 2 */
		    /* update MRG_SIG_MODE1 & 2 */
		    ptr_out_param->MRG_R3B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
		    ptr_out_param->MRG_R3B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
		    /* A+B, MRG_R3B MRG_END_X */
		    ptr_out_param->MRG_R3B_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;

			/* IMGO_R1B */
	        ptr_out_param->IMGO_R1B_IMGO_XSIZE.Bits.IMGO_XSIZE = DUAL_CALC_BUS_XIZE((int)ptr_out_param->MRG_R3B_MRG_CROP.Bits.MRG_END_X -
	            (int)ptr_out_param->MRG_R3B_MRG_CROP.Bits.MRG_STR_X + 1, bit_per_pix, bus_width_shift);
	    }
	    else
	    {
	        /* CRP_R3A */
	        ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XSTART = 0;
	        if (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_b) - 1))
	        {
		        ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XEND = sep_r1_valid_width[0] - 1 + DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_b) -
			        (sep_r1_valid_width[0] & (DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_b) - 1));
	        }
	        else
	        {
		        ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XEND = sep_r1_valid_width[0] - 1;
	        }
	        ptr_out_param->CRP_R3A_CRP_Y_POS.Bits.CRP_YSTART = 0;
	        ptr_out_param->CRP_R3A_CRP_Y_POS.Bits.CRP_YEND = ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		        ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		        ((unsigned int)ptr_in_param->SW.TWIN_RAWI_YSIZE >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN))) - 1;

			/* IMGO_R1A */
		    ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE = DUAL_CALC_BUS_XIZE((int)ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XEND -
		        (int)ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XSTART + 1, bit_per_pix, bus_width_shift);

	        if (((int)ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XEND - (int)ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XSTART + 1) & (DUAL_ALIGN_PIXEL_MODE(imgo_pipe_cut_shift_b) - 1))
	        {
		        result = DUAL_MESSAGE_CRP_R3_SIZE_CAL_ERROR;
		        dual_driver_printf("Error: %s\n", print_error_message(result));
		        return result;
	        }

			/* CRP_R3B */
	        ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_CRP_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_CRP_R3_EN;
	        ptr_out_param->CRP_R3B_CRP_X_POS.Bits.CRP_XSTART = (int)ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XEND + 1 - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
		    ptr_out_param->CRP_R3B_CRP_X_POS.Bits.CRP_XEND = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
	        ptr_out_param->CRP_R3B_CRP_Y_POS.Bits.CRP_YSTART = 0;
	        ptr_out_param->CRP_R3B_CRP_Y_POS.Bits.CRP_YEND = ptr_out_param->CRP_R3A_CRP_Y_POS.Bits.CRP_YEND;

            /* IMGO_R1B */
	        ptr_out_param->IMGO_R1B_IMGO_XSIZE.Bits.IMGO_XSIZE = DUAL_CALC_BUS_XIZE((int)ptr_out_param->CRP_R3B_CRP_X_POS.Bits.CRP_XEND -
	            (int)ptr_out_param->CRP_R3B_CRP_X_POS.Bits.CRP_XSTART + 1, bit_per_pix, bus_width_shift);
	    }

	    /* Clone registers */
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_PAK_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAK_R1_EN;
	    ptr_out_param->IMGO_R1B_IMGO_CON.Raw = ptr_in_param->IMGO_R1A_IMGO_CON.Raw;
	    ptr_out_param->IMGO_R1B_IMGO_CON2.Raw = ptr_in_param->IMGO_R1A_IMGO_CON2.Raw;
	    ptr_out_param->IMGO_R1B_IMGO_CON3.Raw = ptr_in_param->IMGO_R1A_IMGO_CON3.Raw;
	    ptr_out_param->IMGO_R1B_IMGO_CON4.Raw = ptr_in_param->IMGO_R1A_IMGO_CON4.Raw;
	    ptr_out_param->IMGO_R1B_IMGO_STRIDE.Raw = ptr_in_param->IMGO_R1A_IMGO_STRIDE.Raw;
	    ptr_out_param->IMGO_R1B_IMGO_CROP.Bits.IMGO_YOFFSET = ptr_in_param->IMGO_R1A_IMGO_CROP.Bits.IMGO_YOFFSET;
	    ptr_out_param->IMGO_R1B_IMGO_YSIZE.Bits.IMGO_YSIZE = ptr_in_param->IMGO_R1A_IMGO_YSIZE.Bits.IMGO_YSIZE;

	    /* Force bus size disabled */
	    ptr_out_param->IMGO_R1A_IMGO_STRIDE.Bits.IMGO_BUS_SIZE_EN = 0;
	    ptr_out_param->IMGO_R1B_IMGO_STRIDE.Bits.IMGO_BUS_SIZE_EN = 0;
	    /* Setup offset address */
	    ptr_out_param->IMGO_R1A_IMGO_OFST_ADDR.Bits.IMGO_OFST_ADDR = 0;

	    /* RTL */
	    if (ptr_in_param->IMGO_R1A_IMGO_BASE_ADDR.Bits.IMGO_BASE_ADDR == ptr_in_param->IMGO_R1B_IMGO_BASE_ADDR.Bits.IMGO_BASE_ADDR)
	    {
		    /* A+B */
		    if ((int)ptr_in_param->IMGO_R1A_IMGO_CROP.Bits.IMGO_XOFFSET * (1 << bus_width_shift) >=
		        (int)ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE + 1)
		    {
			    /* B only */
			    if (2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL)
			    {
			        ptr_out_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_MRG_R3_EN = 0;
			    }
			    else
			    {
			        ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_CRP_R3_EN = 0;
			    }
			    ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAK_R1_EN = 0;
			    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN = 0;
			    /* B only */
			    ptr_out_param->IMGO_R1B_IMGO_OFST_ADDR.Bits.IMGO_OFST_ADDR = 0;
			    ptr_out_param->IMGO_R1B_IMGO_CROP.Bits.IMGO_XOFFSET = (int)ptr_in_param->IMGO_R1A_IMGO_CROP.Bits.IMGO_XOFFSET -
				    (((unsigned int)ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE + 1) >> bus_width_shift);
			    ptr_out_param->IMGO_R1B_IMGO_XSIZE.Bits.IMGO_XSIZE = ptr_in_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE;
		    }
		    else
		    {
			    /* A+B */
			    if ((int)ptr_in_param->IMGO_R1A_IMGO_CROP.Bits.IMGO_XOFFSET * (1 << bus_width_shift) +
			        (int)ptr_in_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE <=
				    (int)ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE)
			    {
				    /* A only */
				    if (2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL)
			        {
			            ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_MRG_R3_EN = 0;
			        }
			        else
			        {
				        ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_CRP_R3_EN = 0;
				    }
				    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_PAK_R1_EN = 0;
				    ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN = 0;
				    /* A only */
				    ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE = ptr_in_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE;
			    }
			    else
			    {
				    /* A+B */
				    ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE -= (int)ptr_in_param->IMGO_R1A_IMGO_CROP.Bits.IMGO_XOFFSET * (1 << bus_width_shift);
				    ptr_out_param->IMGO_R1B_IMGO_CROP.Bits.IMGO_XOFFSET = 0;
				    ptr_out_param->IMGO_R1B_IMGO_OFST_ADDR.Bits.IMGO_OFST_ADDR = (int)ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE + 1;
				    ptr_out_param->IMGO_R1B_IMGO_XSIZE.Bits.IMGO_XSIZE = (int)ptr_in_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE -
					    ((int)ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE + 1);
			    }
		    }
	    }
	    else
	    {
		    ptr_out_param->IMGO_R1B_IMGO_OFST_ADDR.Bits.IMGO_OFST_ADDR = 0;
		    /* restore stride */
		    ptr_out_param->IMGO_R1B_IMGO_STRIDE.Bits.IMGO_STRIDE = ptr_in_param->IMGO_R1B_IMGO_STRIDE.Bits.IMGO_STRIDE;
		    /* check valid size only WDMA with XOFF and YOFF support */
		    if ((int)ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE + 1 > (int)ptr_in_param->IMGO_R1A_IMGO_STRIDE.Bits.IMGO_STRIDE)
		    {
			    ptr_out_param->IMGO_R1A_IMGO_XSIZE.Bits.IMGO_XSIZE = (((unsigned int)ptr_in_param->IMGO_R1A_IMGO_STRIDE.Bits.IMGO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
		    }
		    if ((int)ptr_out_param->IMGO_R1B_IMGO_XSIZE.Bits.IMGO_XSIZE + 1 > (int)ptr_in_param->IMGO_R1B_IMGO_STRIDE.Bits.IMGO_STRIDE)
		    {
			    ptr_out_param->IMGO_R1B_IMGO_XSIZE.Bits.IMGO_XSIZE = (((unsigned int)ptr_in_param->IMGO_R1B_IMGO_STRIDE.Bits.IMGO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
		    }
	    }
	    if (ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN)
	    {

		    if (2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL)
	        {
				if (sep_r1_left_max < (int)ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_END_X + 1 + DUAL_TILE_LOSS_ALL)
		        {
			        sep_r1_left_max = (int)ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_END_X + 1 + DUAL_TILE_LOSS_ALL;
		        }

			}
			else
			{
		        if (sep_r1_left_max < (int)ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XEND + 1 + DUAL_TILE_LOSS_ALL)
		        {
			        sep_r1_left_max = (int)ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XEND + 1 + DUAL_TILE_LOSS_ALL;
		        }
			}
	    }

        if (1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMGO_SEL)
		{
		    int img_r1a_out_wd = 0;
			int img_r1b_out_wd = 0;

			if (2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_IMG_SEL)
	        {
                img_r1a_out_wd = (int)ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R3A_MRG_CROP.Bits.MRG_STR_X + 1;
                if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R12_EN)
                {
                    img_r1a_out_wd += ((int)ptr_out_param->MRG_R12B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R12B_MRG_CROP.Bits.MRG_STR_X + 1);
                }
                img_r1b_out_wd = (int)ptr_out_param->MRG_R3B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R3B_MRG_CROP.Bits.MRG_STR_X + 1;
            }
            else
            {
                img_r1a_out_wd = (unsigned int)ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XEND -
				                 (unsigned int)ptr_out_param->CRP_R3A_CRP_X_POS.Bits.CRP_XSTART + 1;
                img_r1b_out_wd = (unsigned int)ptr_out_param->CRP_R3B_CRP_X_POS.Bits.CRP_XEND -
				                 (unsigned int)ptr_out_param->CRP_R3B_CRP_X_POS.Bits.CRP_XSTART + 1;
            }

			ptr_out_param->UFEO_R1A_UFEO_OFST_ADDR.Bits.UFEO_OFST_ADDR = 0;
			ptr_out_param->UFEO_R1B_UFEO_OFST_ADDR.Bits.UFEO_OFST_ADDR = 0;
			ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN;
			if (ptr_in_param->UFEO_R1A_UFEO_BASE_ADDR.Bits.UFEO_BASE_ADDR == ptr_in_param->UFEO_R1B_UFEO_BASE_ADDR.Bits.UFEO_BASE_ADDR)
			{
				if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN && ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_UFEO_R1_EN)
				{
					/* UFEO, test mode, size not x8 */
					ptr_out_param->UFEO_R1B_UFEO_CON.Raw = ptr_in_param->UFEO_R1A_UFEO_CON.Raw;
					ptr_out_param->UFEO_R1B_UFEO_CON2.Raw = ptr_in_param->UFEO_R1A_UFEO_CON2.Raw;
					ptr_out_param->UFEO_R1B_UFEO_CON3.Raw = ptr_in_param->UFEO_R1A_UFEO_CON3.Raw;
					ptr_out_param->UFEO_R1B_UFEO_CON4.Raw = ptr_in_param->UFEO_R1A_UFEO_CON4.Raw;
					ptr_out_param->UFEO_R1B_UFEO_STRIDE.Raw = ptr_in_param->UFEO_R1A_UFEO_STRIDE.Raw;
					ptr_out_param->UFEO_R1B_UFEO_YSIZE.Bits.UFEO_YSIZE = ptr_in_param->UFEO_R1A_UFEO_YSIZE.Bits.UFEO_YSIZE;
					ptr_out_param->UFE_R1B_UFE_CON.Raw = ptr_in_param->UFE_R1A_UFE_CON.Raw;
					if (0 == ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN)
					{
						/* UFEO_A only */
						ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_UFEO_R1_EN = 0;
						ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN = 0;
						ptr_out_param->UFEO_R1A_UFEO_XSIZE.Bits.UFEO_XSIZE = ((img_r1a_out_wd + 63) >> 6) - 1;
					}
					else if (0 == ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_IMGO_R1_EN)
					{
						/* UFEO_B only */
						ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_UFEO_R1_EN = 0;
						ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN = 0;
					    ptr_out_param->UFEO_R1B_UFEO_XSIZE.Bits.UFEO_XSIZE = ((img_r1b_out_wd + 63) >> 6) - 1;
						/* restore UFEO_R1A_UFEO_XSIZE for driver config sync */
						ptr_out_param->UFEO_R1A_UFEO_XSIZE.Bits.UFEO_XSIZE = ptr_in_param->UFEO_R1A_UFEO_XSIZE.Bits.UFEO_XSIZE;
					}
					else
					{
						/* UFEO, test mode, size not x8 */
						ptr_out_param->UFEO_R1A_UFEO_XSIZE.Bits.UFEO_XSIZE = ((img_r1a_out_wd + 63) >> 6) - 1;
						ptr_out_param->UFEO_R1B_UFEO_XSIZE.Bits.UFEO_XSIZE = ((img_r1b_out_wd + 63) >> 6) - 1;
						ptr_out_param->UFEO_R1B_UFEO_OFST_ADDR.Bits.UFEO_OFST_ADDR = (unsigned int)ptr_out_param->UFEO_R1A_UFEO_XSIZE.Bits.UFEO_XSIZE + 1;
					}
				}
				else
				{
					if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFE_R1_EN || ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_UFEO_R1_EN)
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
				ptr_out_param->UFEO_R1A_UFEO_XSIZE.Bits.UFEO_XSIZE = ((img_r1a_out_wd + 63) >> 6) - 1;
				ptr_out_param->UFEO_R1B_UFEO_XSIZE.Bits.UFEO_XSIZE = ((img_r1b_out_wd + 63) >> 6) - 1;
			}
		}
	}

    /* Setup RRZ_R1 & MRG_R8 */
    if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN)
    {
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN;
	    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN)
	    {
	    	/* Update RRZ_R1 in width */
            int rrz_in_w = ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
	    	                ((int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_E - (int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_S):
	    	                ptr_in_param->SW.TWIN_RAWI_XSIZE) >>
	    	                ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
	    	                 (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN +
	    	                 (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN);

	    	int out_xe_rrz = 0;
	    	int out_shift[3] = {DUAL_RLB_ALIGN_SHIFT, DUAL_RLB_ALIGN_SHIFT, DUAL_RLB_ALIGN_SHIFT};

	    	ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL;
	    	ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_HDS_SEL  = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_HDS_SEL;

	    	if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_RLB_R1_EN)
	    	{
	    	    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    	    return result;
	    	}

            if ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN) || (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN))
            {
	    	    /* check RRZ_R1 alignment */
	    	    if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
	    	    {
	    		    if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
	    		    {
	    				out_shift[0] = 5; /* 2 << 5, x64 */
	    			}
	    			else if (DUAL_OUT_FMT_FG_10 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
	    			{
	    				out_shift[0] = 6; /* 2 << 6, x128 */
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
	    			if (DUAL_OUT_FMT_FG_8 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
	    			{
	    				out_shift[0] = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 3: 0;  /* 2<<3, x16 */
	    			}
	    			else if (DUAL_OUT_FMT_FG_12 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
	    			{
	    				out_shift[0] = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 4: 1;  /* 2<<4, x32 */
	    			}
	    			else if (DUAL_OUT_FMT_FG_10 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
	    			{
	    				/* new 32b */
	    				out_shift[0] = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 4: 2;  /* 2<<4, x32 */
	    			}
	    			else
	    			{
	    				result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
	    				dual_driver_printf("Error: %s\n", print_error_message(result));
	    				return result;
	    			}
	    		}
	    		/* check RRZ_R1 alignment, minmum x8 for RLB monitor */
	    		if (out_shift[0] < DUAL_RLB_ALIGN_SHIFT)
	    		{
	    			out_shift[0] = DUAL_RLB_ALIGN_SHIFT;  /* 2<<2, x8 */
	    		}
	    		out_shift[1] = DUAL_RLB_ALIGN_SHIFT;  /* x8 */
	    	}
	    	/* check MRG_R8 size */
	    	if (ptr_in_param->CAMCTL_R1B_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R8O < 2)
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
	    	/* copy from RRZ_IN_HT & RRZ_D_IN_HT from SEP & SEP_D */
	    	ptr_out_param->RRZ_R1A_RRZ_IN_IMG.Bits.RRZ_IN_HT = (unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >>
	    	                                                    (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN;

	    	if ((rrz_in_w < dual_mode * DUAL_RLB_ALIGN_PIX) || (ptr_in_param->SW.TWIN_RRZ_OUT_WD < dual_mode * DUAL_RLB_ALIGN_PIX) ||
	    		(ptr_in_param->SW.TWIN_RRZ_OUT_WD > DUAL_RRZ_WD_ALL))
	    	{
	    	    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    	    return result;
	    	}

	    	/* Check horizontal oefficient step */
	    	ptr_out_param->RRZ_R1A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = (long long)(rrz_in_w - 1 -
	    		                                                       ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST -
	    		                                                       ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST) * DUAL_RRZ_PREC /
	    		                                                       (ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1);
	    	/* Scaling down error check */
	    	if (ptr_out_param->RRZ_R1A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP < DUAL_RRZ_PREC)
	    	{
	    		result = DUAL_MESSAGE_RRZ_SCALING_UP_ERROR;
	    		dual_driver_printf("Error: %s\n", print_error_message(result));
	    		return result;
	    	}

            if (0 == ptr_in_param->RRZ_R1A_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP)
		    {
	    	    /* Check vertical coefficient step */
	    	    if (ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT <= 1)
	            {
	    	        result = DUAL_MESSAGE_INVALID_RRZ_IN_CROP_HT_ERROR;
	    	        dual_driver_printf("Error: %s\n", print_error_message(result));
	    	        return result;
	            }
	            else
	            {
	    	        /* Recalculate RRZ_R1 RRZ_VERT_STEP */
	    	        ptr_out_param->RRZ_R1A_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = ((long long)ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT - 1) * DUAL_RRZ_PREC /
	    	                                                                  ((long long)ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT - 1);
	    	        if (ptr_out_param->RRZ_R1A_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP < DUAL_RRZ_PREC)
	    	        {
	    		        result = DUAL_MESSAGE_RRZ_SCALING_UP_ERROR;
	    		        dual_driver_printf("Error: %s\n", print_error_message(result));
	    		        return result;
	    	        }
	            }
	        }

	        /* copy coefficient step */
	        ptr_out_param->RRZ_R1B_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = ptr_out_param->RRZ_R1A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
	        ptr_out_param->RRZ_R1B_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = ptr_out_param->RRZ_R1A_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP;
	        ptr_out_param->RRZ_R1B_RRZ_IN_IMG.Bits.RRZ_IN_HT = ptr_out_param->RRZ_R1A_RRZ_IN_IMG.Bits.RRZ_IN_HT;

	        /* configure single_mode */
	        ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
	        ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    	ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    	ptr_out_param->MRG_R8A_MRG_VSIZE.Bits.MRG_HT = ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

	    	ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    	ptr_out_param->MRG_R8B_MRG_VSIZE.Bits.MRG_HT = ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
	    	ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;  /* RRZ_R1B, disable from RRZ_R1C */
	    	ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;

    		/* loop dual mode from right */
    		for (int i = 0; i < 2; i++)
    		{
    			int in_xs_sep = 0;
    			int in_xs_rrz = 0;
    			int in_xe_rrz = 0;
    			int out_xs_rrz = 0;

    			/* config forward backward pos */
    			switch(i)
    			{
    				case 0:
    					/* RRZ_R1B */
    					ptr_out_param->RRZ_R1B_RRZ_IN_IMG.Bits.RRZ_IN_WD = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X -
    					                                                   (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X + 1;
    					/* init pos */
    					in_xs_sep = ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
    					/* Right boundary */
    					in_xs_rrz = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X + DUAL_TILE_LOSS_ALL_L;
    					in_xe_rrz = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X;
    					out_xe_rrz = ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1;
    					break;
    				case 1:
    					/* RRZ_R1A */
    					ptr_out_param->RRZ_R1A_RRZ_IN_IMG.Bits.RRZ_IN_WD = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X -
    						                                               (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X + 1;
	    				/* init pos */
	    				in_xs_sep = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X;
	    				out_xs_rrz = 0;
	    				in_xe_rrz = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X - DUAL_TILE_LOSS_ALL;
	    				break;
	    			default:
	    				break;
	    		}

	    		/* backward & forward calculation */
	    		if (out_xs_rrz < out_xe_rrz)
	    		{
	    			/* backward out_xe_rrz */
	    			long long end_temp = (long long)out_xe_rrz * ptr_out_param->RRZ_R1A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP +
	    				(long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST * DUAL_RRZ_PREC + ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
	    			/* cal pos */
	    			if (end_temp + (long long)DUAL_TILE_LOSS_RRZ_TAP * DUAL_RRZ_PREC < (long long)in_xe_rrz * DUAL_RRZ_PREC)
	    			{
	    				int n = (int)((unsigned long long)(end_temp + DUAL_TILE_LOSS_RRZ_TAP * DUAL_RRZ_PREC) >> 15);
	    				if (n & 0x1)
	    				{
	    					in_xe_rrz = n ;
	    				}
	    				else/* must be even */
	    				{
	    					in_xe_rrz = n + 1;
	    				}
	    			}
	    			else
	    			{
	    				if (in_xe_rrz + 1 < ptr_in_param->SW.TWIN_RRZ_OUT_WD)
	    				{
	    					result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
	    					dual_driver_printf("Error: %s\n", print_error_message(result));
	    					return result;
	    				}
	    			}
	    			/* forward in_xs_rrz */
	    			if (in_xs_rrz < in_xe_rrz)
	    			{
	    				if (in_xs_rrz <= 0)
	    				{
	    					out_xs_rrz = 0;
	    				}
	    				else
	    				{
	    					long long start_temp = (long long)(in_xs_rrz + DUAL_TILE_LOSS_RRZ_L) * DUAL_RRZ_PREC -
	    						(long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST * DUAL_RRZ_PREC - ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
        	    						int n = (int)(start_temp / ptr_out_param->RRZ_R1A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP);
	    					if (((long long)n * ptr_out_param->RRZ_R1A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP) < start_temp)
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
	    						out_xs_rrz = n + DUAL_RRZ_ALIGN(out_shift[i]) - (n & DUAL_RRZ_MASK(out_shift[i]));
	    					}
	    					else
	    					{
	    						out_xs_rrz = n;
	    					}
	    				}
	    			}
	    		}
	    		/* update RRZ width, offset, flag */
	    		if ((out_xs_rrz < out_xe_rrz) && (in_xs_rrz < in_xe_rrz))
	    		{
	    			long long temp_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST * DUAL_RRZ_PREC + ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST +
	    			                        (long long)out_xs_rrz * ptr_out_param->RRZ_R1A_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP - (long long)in_xs_sep * DUAL_RRZ_PREC;
	    			switch (i)
	    			{
	    				case 0:/* to revise for RRZ_D & RMX_D */
	    					/* RRZ_D */
	    					ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = out_xe_rrz - out_xs_rrz + 1;
	    					ptr_out_param->RRZ_R1B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)temp_offset >> 15);
	    					ptr_out_param->RRZ_R1B_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(temp_offset -
	    						(long long)ptr_out_param->RRZ_R1B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST * (1 << 15));
        	    						ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_STR_X = 0;
	    					ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_END_X = out_xe_rrz - out_xs_rrz;
	    					/* check RRZ_D buffer size, must x8 align */
	    					if (ptr_in_param->SW.DUAL_SEL & 0x1)
	    					{
	    						/* A+B, A+C, align from start, C+A, B+A */
	    						ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
	    						ptr_out_param->RRZ_R1A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >>
	    						    (DUAL_RLB_ALIGN_SHIFT + 1)) << (DUAL_RLB_ALIGN_SHIFT + 1);
	    					}
	    					else
	    					{
	    						/* B+C, align from end */
	    						ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = DUAL_RRZ_WD_ALL -
	    							((((unsigned int)ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >> (DUAL_RLB_ALIGN_SHIFT + 1)) << (DUAL_RLB_ALIGN_SHIFT + 1));
	    					}
	    					break;
	    				case 1:
	    					/* RRZ */
	    					ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = out_xe_rrz - out_xs_rrz + 1;
	    					ptr_out_param->RRZ_R1A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)temp_offset >> 15);
	    					ptr_out_param->RRZ_R1A_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(temp_offset -
	    						(long long)ptr_out_param->RRZ_R1A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST * (1 << 15));
        	    						ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_STR_X = 0;
	    					ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_END_X = out_xe_rrz - out_xs_rrz;
	    					/* check min size */
	    					if (sep_r1_left_max < in_xe_rrz + 1 + DUAL_TILE_LOSS_ALL)
	    					{
	    						sep_r1_left_max = in_xe_rrz + 1 + DUAL_TILE_LOSS_ALL;
	    					}
	    					/* check RRZ buffer size, must x8 align */
	    					if (ptr_in_param->SW.DUAL_SEL & 0x1)
	    					{
	    						/* A+B, A+C, align from start, C+A, B+A */
	    						if (((((unsigned int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >>
	    						       (DUAL_RLB_ALIGN_SHIFT + 1)) << (DUAL_RLB_ALIGN_SHIFT + 1)) + (unsigned int)ptr_out_param->RRZ_R1A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST > DUAL_RRZ_WD_ALL)
	    						{
	    							result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    							dual_driver_printf("Error: %s\n", print_error_message(result));
	    							return result;
	    						}
	    					}
	    					else
	    					{
	    						/* B+C, align from end */
	    						if (((((unsigned int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >>
	    						       (DUAL_RLB_ALIGN_SHIFT + 1)) << (DUAL_RLB_ALIGN_SHIFT + 1)) > (unsigned int)ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST)
	    						{
	    							result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    							dual_driver_printf("Error: %s\n", print_error_message(result));
	    							return result;
	    						}
	    						else
	    						{
	    							/* align from end */
	    							ptr_out_param->RRZ_R1A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST -
	    								((((unsigned int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >> (DUAL_RLB_ALIGN_SHIFT + 1)) << (DUAL_RLB_ALIGN_SHIFT + 1));
	    						}
	    					}
	    					break;
	    				default:
	    					break;
	    			}

	    			/* Decrease out_xe to next start pos */
	    			out_xe_rrz = out_xs_rrz - 1;
	    		}
	    		else
	    		{
	    			/* MRG_R8 disabled */
	    			if ((out_xe_rrz > 0) && (out_xe_rrz + 1 < ptr_in_param->SW.TWIN_RRZ_OUT_WD) && (in_xs_rrz <= in_xe_rrz))
	    			{
	    				result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
	    				dual_driver_printf("Error: %s\n", print_error_message(result));
	    				return result;
	    			}
	    			switch (i)
	    			{
	    				case 0:/* to revise for RRZ_D & RMX_D */
	    					ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;  /* RRZ_R1A, diable from RRZ_R1B */
	    					ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = DUAL_RRZ_ALIGN(out_shift[1]);
	    					ptr_out_param->RRZ_R1B_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
	    					ptr_out_param->RRZ_R1B_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
	    					ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_STR_X = 0;
	    					ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_END_X = DUAL_RRZ_MASK(out_shift[1]);
	    					ptr_out_param->MRG_R8B_MRG_VSIZE.Bits.MRG_HT = (unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN;
	    					/* check RRZ buffer size, must x8 align */
	    					if (ptr_in_param->RRZO_R1A_RRZO_BASE_ADDR.Bits.RRZO_BASE_ADDR == ptr_in_param->RRZO_R1B_RRZO_BASE_ADDR.Bits.RRZO_BASE_ADDR)
	    					{
	    						if (ptr_in_param->SW.DUAL_SEL & 0x1)
	    						{
	    							/* A+B, A+C, align from start, B+A, C+A */
	    							ptr_out_param->RRZ_R1A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
	    							ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
	    						}
	    						else
	    						{
	    							/* B+C, align from end */
	    							ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = DUAL_RRZ_WD_ALL;
	    						}
	    					}
	    					else
	    					{
	    						if (ptr_in_param->SW.DUAL_SEL & 0x1)
	    						{
	    							/* A+B, A+C, align from start, B+A, C+A */
	    							ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
	    							ptr_out_param->RRZ_R1A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >>
	    							    (DUAL_RLB_ALIGN_SHIFT + 1)) << (DUAL_RLB_ALIGN_SHIFT + 1);
	    						}
	    						else
	    						{
	    							/* B+C, align from end */
	    							ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = DUAL_RRZ_WD_ALL -
	    								((((unsigned int)ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >> (DUAL_RLB_ALIGN_SHIFT + 1)) << (DUAL_RLB_ALIGN_SHIFT + 1));
	    						}
	    					}
	    					break;
	    				case 1:
	    					ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE2 = 1;/* RRZ_A, disable RRZ_A */
	    					ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = DUAL_RRZ_ALIGN(out_shift[1]);
	    					ptr_out_param->RRZ_R1A_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
	    					ptr_out_param->RRZ_R1A_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
	    					ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_STR_X = 0;
	    					ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_END_X = DUAL_RRZ_MASK(out_shift[1]);
	    					ptr_out_param->MRG_R8A_MRG_VSIZE.Bits.MRG_HT = (unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN;
	    					/* check RRZ buffer size, must x8 align */
	    					if (ptr_in_param->RRZO_R1A_RRZO_BASE_ADDR.Bits.RRZO_BASE_ADDR == ptr_in_param->RRZO_R1B_RRZO_BASE_ADDR.Bits.RRZO_BASE_ADDR)
	    					{
	    						ptr_out_param->RRZ_R1A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST;
	    					}
	    					else
	    					{
	    						if (ptr_in_param->SW.DUAL_SEL & 0x1)
	    						{
	    							/* A+B, A+C, align from start, C+A, B+A */
	    							if (((((unsigned int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >> 3) << 3) +
	    						        (unsigned int)ptr_out_param->RRZ_R1A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST > DUAL_RRZ_WD_ALL)
	    							{
	    								result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    								dual_driver_printf("Error: %s\n", print_error_message(result));
	    								return result;
	    							}
	    						}
	    						else
	    						{
	    							/* B+C, align from end */
	    							if (((((unsigned int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >>
	    							    (DUAL_RLB_ALIGN_SHIFT + 1)) << (DUAL_RLB_ALIGN_SHIFT + 1)) >
	    								(unsigned int)ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST)
	    							{
	    								result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	    								dual_driver_printf("Error: %s\n", print_error_message(result));
	    								return result;
	    							}
	    							else
	    							{
	    								/* align from end */
	    								ptr_out_param->RRZ_R1A_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = ptr_out_param->RRZ_R1B_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST -
	    									((((unsigned int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + DUAL_RLB_ALIGN_PIX - 1) >>
	    									(DUAL_RLB_ALIGN_SHIFT + 1)) << (DUAL_RLB_ALIGN_SHIFT + 1));
	    							}
	    						}
	    					}
	    					break;
	    				default:
	    					break;
	    			}
	    		}
	    	}
	    	if (out_xe_rrz >= 0)
	    	{
	    		result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
	    		dual_driver_printf("Error: %s\n", print_error_message(result));
	    		return result;
	    	}

	    	/* Setup RRZO_R1 */
	    	if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN)
	    	{
	    		int bit_per_pix = 18;
	    		int bus_width_shift = 3;
	    		if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
	    		{
	    			if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_UFG_R1_EN)
	    			{
	    				result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
	    				dual_driver_printf("Error: %s\n", print_error_message(result));
	    				return result;
	    			}
	    			ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_UFG_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_UFG_R1_EN;

	    			bus_width_shift = 4;/* (1<<4)*8, 128 bits */
	    			switch (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
	    			{
	    				case DUAL_OUT_FMT_FG_10:/* FG 10 */
	    					bit_per_pix = 15;
	    					break;
	    				case DUAL_OUT_FMT_FG_12:/* FG 12 */
	    					bit_per_pix = 18;
	    					break;
	    				default:                /* FG 14 */
	    					bit_per_pix = 21;
	    					break;
	    			}
	    		}
	    		else
	    		{
	    			if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAKG_R1_EN)
	    			{
	    				result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
	    				dual_driver_printf("Error: %s\n", print_error_message(result));
	    				return result;
	    			}

	    			ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_PAKG_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAKG_R1_EN;

	    			bus_width_shift = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 2: 1;  /* (1 << 2) * 8, 32 bits */
	    			switch (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RRZO_R1_FMT)
	    			{
	    				case DUAL_OUT_FMT_FG_8:/* FG 8 */
	    					bit_per_pix = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 12: 8;
	    					break;
	    				case DUAL_OUT_FMT_FG_10:/* FG 10 */
	    					bit_per_pix = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 15: 10;
	    					break;
	    				case DUAL_OUT_FMT_FG_12:/* FG 12 */
	    					bit_per_pix = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 18: 12;
	    					break;
	    				default:/* FG 14 */
	    					bit_per_pix = ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PAKG_R1_FG_OUT? 21: 14;
	    					break;
	    			}

	    			if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN &&
	    		        ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN)
	    		    {
	    			    bus_width_shift += (int)ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R7O;
	    			}
	    		}

	    		/* RRZO */
	    		if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN &&
	    		    ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN)
	            {
	    		    int mrg_r7_end_x = (int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
	                /* MRG_R7 && MRG_R11 */
	    		    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN = 0;
	                ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_STR_X = 0;
	                ptr_out_param->MRG_R7A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	                ptr_out_param->MRG_R7A_MRG_VSIZE.Bits.MRG_HT = ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
	                if ((ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1 != 1) && (ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD & (DUAL_ALIGN_PIXEL_MODE(rrzo_pipe_cut_shift_a) - 1)))
	                {
	    				/* add MRG_SIG_MODE1 & 2 */
	    				ptr_out_param->MRG_R7A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
	    				ptr_out_param->MRG_R7A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    				mrg_r7_end_x = (int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1 + DUAL_ALIGN_PIXEL_MODE(rrzo_pipe_cut_shift_a) -
	    		            ((int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD & (DUAL_ALIGN_PIXEL_MODE(rrzo_pipe_cut_shift_a) - 1));

	    	            ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
	    		        ptr_out_param->MRG_R11B_MRG_CROP.Bits.MRG_STR_X = 0;
	    		        ptr_out_param->MRG_R11B_MRG_CROP.Bits.MRG_END_X = mrg_r7_end_x - (int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
	    		        ptr_out_param->MRG_R11B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
	                    ptr_out_param->MRG_R11B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    		        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN = 1;
	    				ptr_out_param->MRG_R11B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	    				ptr_out_param->MRG_R11B_MRG_VSIZE.Bits.MRG_HT = ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
	    				if (((int)ptr_out_param->MRG_R11B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R11B_MRG_CROP.Bits.MRG_STR_X + 1) >= DULL_MRG_R11B_MAX_WD)
	        	        {
	        	            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	        	            dual_driver_printf("Error: %s\n", print_error_message(result));
	        	            return result;
	                    }

	                    if ((ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_STR_X & (DUAL_ALIGN_PIXEL_MODE(rrzo_pipe_cut_shift_a) - 1)) ||
	    	                (((int)mrg_r7_end_x + 1) & (DUAL_ALIGN_PIXEL_MODE(rrzo_pipe_cut_shift_a) - 1)))
	                    {
	    	                result = DUAL_MESSAGE_MRG_R7_SIZE_CAL_ERROR;
	    	                dual_driver_printf("Error: %s\n", print_error_message(result));
	    	                return result;
	                    }
	                }
	                else
	                {
	    				/* add MRG_SIG_MODE1 & 2 */
	    				ptr_out_param->MRG_R7A_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
	    				ptr_out_param->MRG_R7A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	    				ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
	    	            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN = 0;
	                }

	    		    /* RRZO_R1A */
	    		    ptr_out_param->RRZO_R1A_RRZO_OFST_ADDR.Bits.RRZO_OFST_ADDR = 0;
	    		    ptr_out_param->RRZO_R1A_RRZO_CROP.Bits.RRZO_XOFFSET = 0;

	                if (ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1 || ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE2)
	    		    {
	    			    /* full size output with padding */
	    			    if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
	    			    {
	    				    /* x64p and 128 b */
	    					ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE = DUAL_CALC_BUS_XIZE(((((int)ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_END_X -
	    					    (int)ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_STR_X + 1) + 63) >> 6) * 64, bit_per_pix, bus_width_shift);
	    			    }
	    			    else
	    			    {
	    				    ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE = DUAL_CALC_BUS_XIZE((int)ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_END_X -
	    				        (int)ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_STR_X + 1, bit_per_pix, bus_width_shift);
	    			    }
	    		    }
	    		    else
	    		    {
	    			    ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE = (((unsigned int)(mrg_r7_end_x - (int)ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_STR_X + 1) * bit_per_pix) >> 3) - 1;
	    		    }
	    		    /* check bus width */
	    		    if (((unsigned int)ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE + 1) & ((1 << bus_width_shift) - 1))
	    		    {
	    			    result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
	    			    dual_driver_printf("Error: %s\n", print_error_message(result));
	    			    return result;
	    		    }

	                /* MRG_R7B */
	    			ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_STR_X = ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE2? 0: (mrg_r7_end_x + 1 - (int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD);
    	            ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
    	            if (ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_STR_X <= ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_END_X)
    	            {
    	                ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN;
	                    ptr_out_param->MRG_R7B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	                    ptr_out_param->MRG_R7B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R7A_MRG_VSIZE.Bits.MRG_HT;
	                    /* add MRG_SIG_MODE1 & 2 */
    	                /* update MRG_SIG_MODE1 & 2 */
    	                ptr_out_param->MRG_R7B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
    	                ptr_out_param->MRG_R7B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;


	    		        /* RRZO_R1B */
	    		        ptr_out_param->RRZO_R1B_RRZO_OFST_ADDR.Bits.RRZO_OFST_ADDR = 0;
	    		        ptr_out_param->RRZO_R1B_RRZO_CROP.Bits.RRZO_XOFFSET = 0;

                        if (ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1 ||
                            ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_SIG_MODE1 ||
	    			        ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_SIG_MODE2)
	    		        {
	    			        if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
	    			        {
	    				        /* x64p and 128 b */
	                            ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE = DUAL_CALC_BUS_XIZE(((((int)ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_END_X -
	                                (int)ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_STR_X + 1) + 63) >> 6) * 64, bit_per_pix, bus_width_shift);
	    			        }
	    			        else
	    			        {
	    				        ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE = DUAL_CALC_BUS_XIZE((int)ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_END_X -
	    				            (int)ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_STR_X + 1, bit_per_pix, bus_width_shift);
	    			        }
	    		        }
	    		        else
	    		        {
	    			        ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE = (((unsigned int)((int)ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_STR_X + 1)
	    			            * bit_per_pix) >> 3) - 1;
	    		        }
	    		        /* check bus width */
	    		        if (((unsigned int)ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE + 1) & ((1 << bus_width_shift) - 1))
	    		        {
	    			        result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
	    			        dual_driver_printf("Error: %s\n", print_error_message(result));
	    			        return result;
	    		        }
					}
	            }
	            else
	            {
	    		    ptr_out_param->RRZO_R1A_RRZO_OFST_ADDR.Bits.RRZO_OFST_ADDR = 0;
	    		    ptr_out_param->RRZO_R1A_RRZO_CROP.Bits.RRZO_XOFFSET = 0;
	    		    /* check RRZ_A disabled or RRZ_A only */
	    		    if (ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1 || ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE2)
	    		    {
	    			    /* full size output with padding */
	    			    if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
	    			    {
	    				    /* x64p and 128 b */
	    					ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE = DUAL_CALC_BUS_XIZE((((unsigned int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63) >> 6) * 64,
	    					    bit_per_pix, bus_width_shift);
	    			    }
	    			    else
	    			    {
	    					ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD, bit_per_pix, bus_width_shift);
	    			    }
	    		    }
	    		    else
	    		    {
	    			    ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE = (((unsigned int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD * bit_per_pix) >> 3) - 1;
	    		    }
	    		    /* check bus width */
	    		    if (((unsigned int)ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE + 1) & ((1 << bus_width_shift) - 1))
	    		    {
	    			    result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
	    			    dual_driver_printf("Error: %s\n", print_error_message(result));
	    			    return result;
	    		    }

	    		    ptr_out_param->RRZO_R1B_RRZO_OFST_ADDR.Bits.RRZO_OFST_ADDR = 0;
	    		    ptr_out_param->RRZO_R1B_RRZO_CROP.Bits.RRZO_XOFFSET = 0;
	    		    /* check RRZ_B, C disabled or RRZ_C disabled */
	    		    if (ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1 || ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_SIG_MODE1 ||
	    			    ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_SIG_MODE2)
	    		    {
	    			    if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
	    			    {
	    				    /* x64p and 128 b */
	    					ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE = DUAL_CALC_BUS_XIZE((((unsigned int)ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 63) >> 6) * 64,
	    					    bit_per_pix, bus_width_shift);
	    			    }
	    			    else
	    			    {
	    					ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD, bit_per_pix, bus_width_shift);
	    			    }
	    		    }
	    		    else
	    		    {
	    			    ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE = (((unsigned int)ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD * bit_per_pix) >> 3) - 1;
	    		    }
	    		    /* check bus width */
	    		    if (((unsigned int)ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE + 1) & ((1 << bus_width_shift) - 1))
	    		    {
	    			    result = DUAL_MESSAGE_RRZ_XS_XE_CAL_ERROR;
	    			    dual_driver_printf("Error: %s\n", print_error_message(result));
	    			    return result;
	    		    }
	    	    }

	    		/* Copy RRZO resisters */
	    		ptr_out_param->RRZO_R1B_RRZO_CON.Raw = ptr_in_param->RRZO_R1A_RRZO_CON.Raw;
	    		ptr_out_param->RRZO_R1B_RRZO_CON2.Raw = ptr_in_param->RRZO_R1A_RRZO_CON2.Raw;
	    		ptr_out_param->RRZO_R1B_RRZO_CON3.Raw = ptr_in_param->RRZO_R1A_RRZO_CON3.Raw;
	    		ptr_out_param->RRZO_R1B_RRZO_CON4.Raw = ptr_in_param->RRZO_R1A_RRZO_CON4.Raw;
	    		ptr_out_param->RRZO_R1B_RRZO_STRIDE.Raw = ptr_in_param->RRZO_R1A_RRZO_STRIDE.Raw;
	    		ptr_out_param->RRZO_R1B_RRZO_CROP.Bits.RRZO_YOFFSET = ptr_in_param->RRZO_R1A_RRZO_CROP.Bits.RRZO_YOFFSET;
	    		ptr_out_param->RRZO_R1B_RRZO_YSIZE.Bits.RRZO_YSIZE = ptr_in_param->RRZO_R1A_RRZO_YSIZE.Bits.RRZO_YSIZE;
	    		/* force bus size disabled */
	    		ptr_out_param->RRZO_R1A_RRZO_STRIDE.Bits.RRZO_BUS_SIZE_EN = 0;
	    		ptr_out_param->RRZO_R1B_RRZO_STRIDE.Bits.RRZO_BUS_SIZE_EN = 0;
	    		/* RTL */
	    		if (ptr_in_param->RRZO_R1A_RRZO_BASE_ADDR.Bits.RRZO_BASE_ADDR == ptr_in_param->RRZO_R1B_RRZO_BASE_ADDR.Bits.RRZO_BASE_ADDR)
	    		{
    				/* check slave disable */
                    if (ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_STR_X > ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_END_X)
                    {
    					ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN = 0;
    					ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN = 0;
    					ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_PAKG_R1_EN = 0;
                    }
                    else if (ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1)
    				{
    					/* slave disable, RRZO_A only */
    					ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN = 0;
    					ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN = 0;
    					ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN = 0;
    					ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_PAKG_R1_EN = 0;
    				}
    				else if (ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE2)
    				{
    					/* master disable, RRZO_B only */
    					ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN = 0;
    					ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN = 0;
    					ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN = 0;
    					ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN = 0;
    					ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_PAKG_R1_EN = 0;
    					/* restore RRZO_R1A_RRZO_XSIZE for driver config sync */
    					ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE = ptr_in_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE;
    				}
    				else
    				{
    					ptr_out_param->RRZO_R1B_RRZO_OFST_ADDR.Bits.RRZO_OFST_ADDR = (unsigned int)ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE + 1;
    				}
	    		}
	    		else
	    		{
	    			ptr_out_param->RRZO_R1B_RRZO_OFST_ADDR.Bits.RRZO_OFST_ADDR = 0;
	    			/* restore stride */
	    			ptr_out_param->RRZO_R1B_RRZO_STRIDE.Bits.RRZO_STRIDE = ptr_in_param->RRZO_R1B_RRZO_STRIDE.Bits.RRZO_STRIDE;
	    			/* check valid size only WDMA with XOFF and YOFF support */
	    			if ((int)ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE + 1 > (int)ptr_in_param->RRZO_R1A_RRZO_STRIDE.Bits.RRZO_STRIDE)
	    			{
	    				if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
	    				{
	    					result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
	    					dual_driver_printf("Error: %s\n", print_error_message(result));
	    					return result;
	    				}
	    				else
	    				{
	    					ptr_out_param->RRZO_R1A_RRZO_XSIZE.Bits.RRZO_XSIZE = (((unsigned int)ptr_in_param->RRZO_R1A_RRZO_STRIDE.Bits.RRZO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
	    				}
	    			}
	    			if ((int)ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE + 1 > (int)ptr_in_param->RRZO_R1B_RRZO_STRIDE.Bits.RRZO_STRIDE)
	    			{
	    				if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
	    				{
	    					result = DUAL_MESSAGE_RRZO_CONFIG_ERROR;
	    					dual_driver_printf("Error: %s\n", print_error_message(result));
	    					return result;
	    				}
	    				else
	    				{
	    					ptr_out_param->RRZO_R1B_RRZO_XSIZE.Bits.RRZO_XSIZE = (((unsigned int)ptr_in_param->RRZO_R1B_RRZO_STRIDE.Bits.RRZO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
	    				}
	    			}
	    		}
	    		if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_UFEG_SEL)
	    		{
	    		    int rrz_r1a_out_wd = 0;
	    		    int rrz_r1b_out_wd = 0;

	    		    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN &&
	    		        ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN)
	                {
	                    rrz_r1a_out_wd = (int)ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R7A_MRG_CROP.Bits.MRG_STR_X + 1;
	                    if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R11_EN)
	                    {
	                        rrz_r1a_out_wd += ((int)ptr_out_param->MRG_R11B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R11B_MRG_CROP.Bits.MRG_STR_X + 1);
	                    }
						rrz_r1b_out_wd = (ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN? ((int)ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R7B_MRG_CROP.Bits.MRG_STR_X + 1): 0);
	                }
	                else
	                {
	                    rrz_r1a_out_wd = (unsigned int)ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
	                    rrz_r1b_out_wd = (unsigned int)ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
	                }

	    			ptr_out_param->UFGO_R1A_UFGO_OFST_ADDR.Bits.UFGO_OFST_ADDR = 0;
	    			ptr_out_param->UFGO_R1B_UFGO_OFST_ADDR.Bits.UFGO_OFST_ADDR = 0;
	    			if (ptr_in_param->UFGO_R1A_UFGO_BASE_ADDR.Bits.UFGO_BASE_ADDR == ptr_in_param->UFGO_R1B_UFGO_BASE_ADDR.Bits.UFGO_BASE_ADDR)
	    			{
	    				if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_UFG_R1_EN && ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_UFGO_R1_EN)
	    				{
	    					/* UFGO, test mode, size not x8 */
	    					ptr_out_param->UFGO_R1B_UFGO_CON.Raw = ptr_in_param->UFGO_R1A_UFGO_CON.Raw;
	    					ptr_out_param->UFGO_R1B_UFGO_CON2.Raw = ptr_in_param->UFGO_R1A_UFGO_CON2.Raw;
	    					ptr_out_param->UFGO_R1B_UFGO_CON3.Raw = ptr_in_param->UFGO_R1A_UFGO_CON3.Raw;
	    					ptr_out_param->UFGO_R1B_UFGO_CON4.Raw = ptr_in_param->UFGO_R1A_UFGO_CON4.Raw;
	    					ptr_out_param->UFGO_R1B_UFGO_STRIDE.Raw = ptr_in_param->UFGO_R1A_UFGO_STRIDE.Raw;
	    					ptr_out_param->UFGO_R1B_UFGO_YSIZE.Bits.UFGO_YSIZE = ptr_in_param->UFGO_R1A_UFGO_YSIZE.Bits.UFGO_YSIZE;
	    					ptr_out_param->UFG_R1B_UFG_CON.Raw = ptr_in_param->UFG_R1A_UFG_CON.Raw;
    						if (1 == ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1 || (ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_MRG_R7_EN == 0))
    						{
    							/* UFGO_A only */
    							ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_UFGO_R1_EN = 0;
    							ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_UFG_R1_EN = 0;
    							ptr_out_param->UFGO_R1A_UFGO_XSIZE.Bits.UFGO_XSIZE = ((rrz_r1a_out_wd + 63) >> 6) - 1;
    						}
    						else if (1 == ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE2)
    						{
    							/* UFGO_B only */
    							ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_UFGO_R1_EN = 0;
    							ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_UFG_R1_EN = 0;
    							ptr_out_param->UFGO_R1B_UFGO_XSIZE.Bits.UFGO_XSIZE = ((rrz_r1b_out_wd + 63) >> 6) - 1;
    							/* restore UFGO_R1A_UFGO_XSIZE for driver config sync */
    							ptr_out_param->UFGO_R1A_UFGO_XSIZE.Bits.UFGO_XSIZE = ptr_in_param->UFGO_R1A_UFGO_XSIZE.Bits.UFGO_XSIZE;
    						}
    						else
    						{
    							/* UFGO, test mode, size not x8 */
    							ptr_out_param->UFGO_R1A_UFGO_XSIZE.Bits.UFGO_XSIZE = ((rrz_r1a_out_wd + 63) >> 6) - 1;
    							ptr_out_param->UFGO_R1B_UFGO_XSIZE.Bits.UFGO_XSIZE = ((rrz_r1b_out_wd + 63) >> 6) - 1;
    							ptr_out_param->UFGO_R1B_UFGO_OFST_ADDR.Bits.UFGO_OFST_ADDR = (unsigned int)ptr_out_param->UFGO_R1A_UFGO_XSIZE.Bits.UFGO_XSIZE + 1;
	    					}
	    				}
	    				else
	    				{
	    					if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_UFG_R1_EN || ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_UFGO_R1_EN)
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
	    				ptr_out_param->UFGO_R1A_UFGO_XSIZE.Bits.UFGO_XSIZE = ((rrz_r1a_out_wd + 63) >> 6) - 1;
	    				ptr_out_param->UFGO_R1B_UFGO_XSIZE.Bits.UFGO_XSIZE = ((rrz_r1b_out_wd + 63) >> 6) - 1;
	    			}
	    		}
	    	}
	    }
		else
		{
			// RRZ should always turned on
			result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}
    }

	/* Setup MRG_R8 branch */
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_MRG_R8_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_MRG_R8_EN;
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_GSE_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_GSE_R1_EN;
	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_MRG_R8_EN)
	{
		if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN)
		{
			/* RMX bypass to reduce RAW A MRG_END_X */
			ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_STR_X = 0;
			if ((ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R8O > 1) && (sep_r1_valid_width[0] & 0x3))
			{
				ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] + 0x3 - (sep_r1_valid_width[0] & 0x3);
			}
			else
			{
				ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_END_X = sep_r1_valid_width[0] - 1;
			}
			ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
			ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;

			ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_STR_X = (int)ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_END_X + 1 -
			                                                 (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;

			ptr_out_param->MRG_R8A_MRG_CTL.Bits.MRG_EDGE = 0xF;
			ptr_out_param->MRG_R8A_MRG_VSIZE.Bits.MRG_HT = (unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >>
			                                               (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN;

			ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_EDGE = 0xF;
			ptr_out_param->MRG_R8B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R8A_MRG_VSIZE.Bits.MRG_HT;

			/* check min size */
			if (sep_r1_left_max < (int)ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_END_X + 1 + DUAL_TILE_LOSS_ALL)
			{
				sep_r1_left_max = (int)ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_END_X + 1 + DUAL_TILE_LOSS_ALL;
			}
			ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
			ptr_out_param->MRG_R8B_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
			ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_END_X = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X -
				                                             (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
		}
		/* check MRG_R8 size */
		if (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_PIX_BUS_MRG_R8O < 2)
		{
			if ((((int)ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_END_X + 1) & 0x1) ||
				(ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_STR_X & 0x1) ||
				(((int)ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_END_X + 1) & 0x1))
			{
				result = DUAL_MESSAGE_MRG_R8_SIZE_CAL_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
		else
		{
			if ((((int)ptr_out_param->MRG_R8A_MRG_CROP.Bits.MRG_END_X + 1) & 0x3) ||
				(ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_STR_X & 0x1) ||
				(((int)ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_END_X + 1) & 0x1) ||
				(((int)ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R8B_MRG_CROP.Bits.MRG_STR_X + 1) & 0x3))
			{
				result = DUAL_MESSAGE_MRG_R8_SIZE_CAL_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}
		}
	}

    if ((ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_YUVO_R1_EN ||
		 ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_RSSO_R2_EN ||
	     ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_CRZO_R1_EN ||
	     ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZO_R2_EN) &&
		 ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL) ||
		  ((1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL) && ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN)))

    {
        ptr_out_param->CAMCTL_R1B_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_FLC_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_FLC_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_CCM_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_CCM_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_GGM_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_GGM_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_G2C_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_G2C_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_C42_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_C42_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_YNRS_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_YNRS_R1_EN;

        if ((1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL) &&
            ((ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN == 0) || (ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN == 0)))
        {
            ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R13_EN = 0;
	        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN = 0;
            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R13_EN = 0;
            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN = 0;

            /* update YUV pipeline in width */
            int yuv_in_w = ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                    (((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		                      ((int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_E - (int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_S):
		                      ptr_in_param->SW.TWIN_RAWI_XSIZE) >>
		                     ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
                              (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN +
		                      (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN)): ptr_in_param->SW.TWIN_RRZ_OUT_WD);

            if (ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_RRZO_R1_EN == 1)
            {
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_FLC_R1_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_CCM_R1_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_GGM_R1_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_G2C_R1_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_C42_R1_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_YNRS_R1_EN = 0;

                // A only
                if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_YUVO_R1_EN)
                {
				    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRP_R4_EN = 0;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRSP_R1_EN = 0;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R1_EN = 0;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R3_EN = 0;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN = 0;
	                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN = 0;
		    	    ptr_out_param->CAMCTL_R1B_CAMCTL_DMA2_EN.Bits.CAMCTL_YUVO_R1_EN = 0;
		    	    ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_YUVBO_R1_EN = 0;
		    	    ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_YUVCO_R1_EN = 0;
		    	}

                if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_RSSO_R2_EN)
                {
		            if (ptr_in_param->SW.TWIN_RSS_R2_OUT_WD > DUAL_RSS_R2_WD_ALL)
		            {
		                result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		                dual_driver_printf("Error: %s\n", print_error_message(result));
		                return result;
		            }

                    /* setup RSS_R2 IN_WD and IN_HT */
                    ptr_out_param->RSS_R2A_RSS_IN_IMG.Bits.RSS_IN_WD = yuv_in_w;
		            ptr_out_param->RSS_R2A_RSS_IN_IMG.Bits.RSS_IN_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		                ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		            /* check horizontal oefficient step */
                    long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST - (ptr_in_param->SW.TWIN_RSS_R2_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST_LAST;

		    	    // cubic accumulation
		    	    ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP = ((long long)ptr_in_param->SW.TWIN_RSS_R2_OUT_WD * DUAL_RSS_PREC_VAL_CUB_ACC) / crop_in_size;

		    	    /* scaling down error check */
		            if (ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP > DUAL_RSS_PREC_VAL_CUB_ACC)
		            {
		    	        result = DUAL_MESSAGE_RSS_SCALING_UP_ERROR;
		    	        dual_driver_printf("Error: %s\n", print_error_message(result));
		    	        return result;
		            }

                    if (0 == ptr_in_param->RSS_R2A_RSS_VERT_STEP.Bits.RSS_VERT_STEP)
                    {
		                /* Check vertical coefficient step */
		                if (ptr_in_param->SW.TWIN_RSS_R2_IN_CROP_HT <= 1)
	                    {
		                    result = DUAL_MESSAGE_INVALID_RSS_IN_CROP_HT_ERROR;
		                    dual_driver_printf("Error: %s\n", print_error_message(result));
		                    return result;
	                    }
	                    else
	                    {
		                    // cubic accumulation
		    	            ptr_out_param->RSS_R2A_RSS_VERT_STEP.Bits.RSS_VERT_STEP = ((long long)((unsigned int)ptr_in_param->RSS_R2A_RSS_OUT_IMG.Bits.RSS_OUT_HT - 1) * DUAL_RSS_PREC_VAL_CUB_ACC +
		    	                ptr_in_param->SW.TWIN_RSS_R2_IN_CROP_HT - 2) / (ptr_in_param->SW.TWIN_RSS_R2_IN_CROP_HT - 1);
		             	    //if (ptr_out_param->RSS_R2A_RSS_VERT_STEP.Bits.RSS_VERT_STEP > DUAL_RSS_PREC_VAL_CUB_ACC)
		                    //{
		    	            //    result = DUAL_MESSAGE_RSS_SCALING_UP_ERROR;
		    	            //    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            //    return result;
		                    //}
	                    }
	                }

					ptr_out_param->RSS_R2A_RSS_OUT_IMG.Bits.RSS_OUT_WD = ptr_in_param->SW.TWIN_RSS_R2_OUT_WD;
					long long temp = (long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP +
                        (((long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_SUB_OFST * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP) >> DUAL_RSS_CONFIG_BITS);
					ptr_out_param->RSS_R2A_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_RSS_PREC_BITS_CUB_ACC);
		    		ptr_out_param->RSS_R2A_RSS_LUMA_HORI_SUB_OFST.Bits.RSS_LUMA_HORI_SUB_OFST = (int)(temp -
		    			(long long)ptr_out_param->RSS_R2A_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST * DUAL_RSS_PREC_VAL_CUB_ACC);

					ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R3_EN = 0;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN = 0;
		    	    ptr_out_param->CAMCTL_R1B_CAMCTL_DMA2_EN.Bits.CAMCTL_RSSO_R2_EN = 0;
		    	}

                if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZO_R2_EN)
                {
		            if (ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD > DUAL_CRZ_R2_WD_ALL)
		            {
		                result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		                dual_driver_printf("Error: %s\n", print_error_message(result));
		                return result;
		            }

		    	    /* setup CRZ_R2 IN_WD and IN_HT */
		    	    ptr_out_param->CRZ_R2A_CRZ_IN_IMG.Bits.CRZ_IN_WD = yuv_in_w;
		            ptr_out_param->CRZ_R2A_CRZ_IN_IMG.Bits.CRZ_IN_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		                ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		            /* Check horizontal oefficient step */
		            if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		            {
                        long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST_LAST;

		                // 2-tap algorithm
                        ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(crop_in_size - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                            (((unsigned int)ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD  - 1) >> 1)) / (ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD - 1);

		    	    	/* Scaling down error check */
		                if (ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                {
		    	            result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	            dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            return result;
		                }
		    	    }
		    	    else
		    	    {
		    	        long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST_LAST;

		    	        // Source accumulation
		    	        ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC + crop_in_size - 2) / (crop_in_size - 1);

		                /* Scaling down error check */
		                if (ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                {
		    	            result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	            dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            return result;
		                }
		    	    }

                    if (0 == ptr_in_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP)
                    {
		                /* Check vertical coefficient step */
		                if (ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT <= 1)
	                    {
		                    result = DUAL_MESSAGE_INVALID_CRZ_IN_CROP_HT_ERROR;
		                    dual_driver_printf("Error: %s\n", print_error_message(result));
		                    return result;
	                    }
	                    else
	                    {
		                    /* Recalculate CRZ_R2 CRZ_VERT_STEP */
		                    if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_VERT_ALGO)
		                    {
		                        // 2-tap algorithm
                                ptr_out_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                                    (((unsigned int)ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT  - 1) >> 1)) / ((unsigned int)ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1);
		                	    if (ptr_out_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                        {
		    	                    result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                    return result;
		                        }
		                    }
		                    else
		                    {
		    	                // Source accumulation
		    	                ptr_out_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)((unsigned int)ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC +
		    	                    ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT - 2) / (ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT - 1);
		    	                if (ptr_out_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                        {
		    	                    result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                    return result;
		                        }
		                    }
	                    }
	                }

					ptr_out_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD;
					if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
					{
					    long long temp = (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP +
                            (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS);

                        ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_CRZ_PREC_BITS_2_TAP);
		    			ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp -
		    			     (long long)ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP);
					    ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST >> 1;
                        if (0 == (ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST & 0x1))
                        {
                            ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                        }
                        else
                        {
                            ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST + DUAL_CRZ_PREC_VAL_2_TAP;
                        }
                    }
                    else
                    {
                        long long temp = (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
                            (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS);
                        ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_CRZ_PREC_BITS_SRC_ACC);
		    			ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp -
		    			    (long long)ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_SRC_ACC);
                        ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST;
                        ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                    }

                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R2_EN = 0;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN = 0;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRSP_R2_EN = 0;
	                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R3_EN = 0;
		    	    ptr_out_param->CAMCTL_R1B_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZO_R2_EN = 0;
		    	    ptr_out_param->CAMCTL_R1B_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZBO_R2_EN = 0;
		        }

                if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_CRZO_R1_EN)
                {
                    if (ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD > DUAL_CRZ_R1_WD_ALL)
		            {
		                result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		                dual_driver_printf("Error: %s\n", print_error_message(result));
		                return result;
		            }

                    /* setup CRZ_R1 IN_WD and IN_HT */
		    	    ptr_out_param->CRZ_R1A_CRZ_IN_IMG.Bits.CRZ_IN_WD = yuv_in_w;
		            ptr_out_param->CRZ_R1A_CRZ_IN_IMG.Bits.CRZ_IN_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		                ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		            /* Check horizontal oefficient step */
		            if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		            {
                        long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST_LAST;

		                // 2-tap algorithm
                        ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(crop_in_size - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                            (((unsigned int)ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD  - 1) >> 1)) / ((long long)ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD - 1);

		    	    	/* Scaling down error check */
		                if (ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                {
		    	            result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	            dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            return result;
		                }
		    	    }
		    	    else
		    	    {
		    	        long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST_LAST;

		    	        // Source accumulation
		    	        ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC + crop_in_size - 2) / (crop_in_size - 1);

		                /* Scaling down error check */
		                if (ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                {
		    	            result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	            dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            return result;
		                }
		    	    }

                    if (0 == ptr_in_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP)
                    {
		                /* Check vertical coefficient step */
		                if (ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT <= 1)
	                    {
		                    result = DUAL_MESSAGE_INVALID_CRZ_IN_CROP_HT_ERROR;
		                    dual_driver_printf("Error: %s\n", print_error_message(result));
		                    return result;
	                    }
	                    else
	                    {
		                    /* Recalculate CRZ_R1 CRZ_VERT_STEP */
		                    if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_VERT_ALGO)
		                    {
		                        // 2-tap algorithm
                                ptr_out_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                                    (((unsigned int)ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT  - 1) >> 1)) / ((unsigned int)ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1);
		                	    if (ptr_out_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                        {
		    	                    result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                    return result;
		                        }
		                    }
		                    else
		                    {
		    	                // Source accumulation
		    	                ptr_out_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)((unsigned int)ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC +
		    	                    ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT - 2) / (ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT - 1);
		    	                if (ptr_out_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                        {
		    	                    result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                    return result;
		                        }
		                    }
		                }
	                }

					ptr_out_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD;
                    if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
					{
					    long long temp = (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP +
                            (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS);
                        ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_CRZ_PREC_BITS_2_TAP);
		    			ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp -
		    			     (long long)ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP);
					    ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST >> 1;
                        if (0 == (ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST & 0x1))
                        {
                            ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                        }
                        else
                        {
                            ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST + DUAL_CRZ_PREC_VAL_2_TAP;
                        }
                    }
                    else
                    {
                        long long temp = (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
                            (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS);
                        ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_CRZ_PREC_BITS_SRC_ACC);
		    			ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp -
		    			    (long long)ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_SRC_ACC);
                        ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST;
                        ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                    }
		        }

		        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_GGM_R2_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_G2C_R2_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_C42_R2_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R1_EN = 0;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN = 0;
	            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R2_EN = 0;
		    	ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_CRZO_R1_EN = 0;
		        ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_CRZBO_R1_EN = 0;
			}
            else
            {
                ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN = 0;
                ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_FLC_R1_EN = 0;
                ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_CCM_R1_EN = 0;
                ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_GGM_R1_EN = 0;
                ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_G2C_R1_EN = 0;
                ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_C42_R1_EN = 0;
                ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_YNRS_R1_EN = 0;

                // B only
                if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_YUVO_R1_EN)
                {
                    // copy from A to B
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRP_R4_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRP_R4_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRSP_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRSP_R1_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R1_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R3_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN;
                    ptr_out_param->YUVO_R1B_YUVO_CON.Raw = ptr_in_param->YUVO_R1A_YUVO_CON.Raw;
                    ptr_out_param->YUVO_R1B_YUVO_CON2.Raw = ptr_in_param->YUVO_R1A_YUVO_CON2.Raw;
                    ptr_out_param->YUVO_R1B_YUVO_CON3.Raw = ptr_in_param->YUVO_R1A_YUVO_CON3.Raw;
                    ptr_out_param->YUVO_R1B_YUVO_CON4.Raw = ptr_in_param->YUVO_R1A_YUVO_CON4.Raw;
                    ptr_out_param->YUVO_R1B_YUVO_STRIDE.Raw = ptr_in_param->YUVO_R1A_YUVO_STRIDE.Raw;
                    ptr_out_param->YUVO_R1B_YUVO_CROP.Raw = ptr_in_param->YUVO_R1A_YUVO_CROP.Raw;
                    ptr_out_param->YUVO_R1B_YUVO_XSIZE.Raw = ptr_in_param->YUVO_R1A_YUVO_XSIZE.Raw;
                    ptr_out_param->YUVO_R1B_YUVO_YSIZE.Raw = ptr_in_param->YUVO_R1A_YUVO_YSIZE.Raw;
					if (1)
                    {
                        ptr_out_param->YUVBO_R1B_YUVBO_CON.Raw = ptr_in_param->YUVBO_R1A_YUVBO_CON.Raw;
                        ptr_out_param->YUVBO_R1B_YUVBO_CON2.Raw = ptr_in_param->YUVBO_R1A_YUVBO_CON2.Raw;
                        ptr_out_param->YUVBO_R1B_YUVBO_CON3.Raw = ptr_in_param->YUVBO_R1A_YUVBO_CON3.Raw;
                        ptr_out_param->YUVBO_R1B_YUVBO_CON4.Raw = ptr_in_param->YUVBO_R1A_YUVBO_CON4.Raw;
                        ptr_out_param->YUVBO_R1B_YUVBO_STRIDE.Raw = ptr_in_param->YUVBO_R1A_YUVBO_STRIDE.Raw;
                        ptr_out_param->YUVBO_R1B_YUVBO_CROP.Raw = ptr_in_param->YUVBO_R1A_YUVBO_CROP.Raw;
						ptr_out_param->YUVBO_R1B_YUVBO_XSIZE.Raw = ptr_in_param->YUVBO_R1A_YUVBO_XSIZE.Raw;
                        ptr_out_param->YUVBO_R1B_YUVBO_YSIZE.Raw = ptr_in_param->YUVBO_R1A_YUVBO_YSIZE.Raw;
                    }
                    if (1)
                    {
                        ptr_out_param->YUVCO_R1B_YUVCO_CON.Raw = ptr_in_param->YUVCO_R1A_YUVCO_CON.Raw;
                        ptr_out_param->YUVCO_R1B_YUVCO_CON2.Raw = ptr_in_param->YUVCO_R1A_YUVCO_CON2.Raw;
                        ptr_out_param->YUVCO_R1B_YUVCO_CON3.Raw = ptr_in_param->YUVCO_R1A_YUVCO_CON3.Raw;
                        ptr_out_param->YUVCO_R1B_YUVCO_CON4.Raw = ptr_in_param->YUVCO_R1A_YUVCO_CON4.Raw;
                        ptr_out_param->YUVCO_R1B_YUVCO_STRIDE.Raw = ptr_in_param->YUVCO_R1A_YUVCO_STRIDE.Raw;
                        ptr_out_param->YUVCO_R1B_YUVCO_CROP.Raw = ptr_in_param->YUVCO_R1A_YUVCO_CROP.Raw;
						ptr_out_param->YUVCO_R1B_YUVCO_XSIZE.Raw = ptr_in_param->YUVCO_R1A_YUVCO_XSIZE.Raw;
                        ptr_out_param->YUVCO_R1B_YUVCO_YSIZE.Raw = ptr_in_param->YUVCO_R1A_YUVCO_YSIZE.Raw;
                   }

                    // disable pipeline A
				    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRP_R4_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRSP_R1_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R1_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R3_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN = 0;
	                ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN = 0;
		    	    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_YUVO_R1_EN = 0;
		    	    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_YUVBO_R1_EN = 0;
		    	    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_YUVCO_R1_EN = 0;

                    // setup remain parameters
                    ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XSTART = 0;
		            ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XEND = yuv_in_w - 1;
		            ptr_out_param->CRP_R4B_CRP_Y_POS.Bits.CRP_YSTART = 0;
		            ptr_out_param->CRP_R4B_CRP_Y_POS.Bits.CRP_YEND = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                (((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN) - 1):
                        (ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT - 1);

                    /* CRSP_R1B */
                    ptr_out_param->CRSP_R1B_CRSP_OUT_IMG.Bits.CRSP_WD = (int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XEND - (int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XSTART + 1;
                    ptr_out_param->CRSP_R1B_CRSP_OUT_IMG.Bits.CRSP_HT = (int)ptr_out_param->CRP_R4B_CRP_Y_POS.Bits.CRP_YEND - (int)ptr_out_param->CRP_R4B_CRP_Y_POS.Bits.CRP_YSTART + 1;
                    ptr_out_param->CRSP_R1B_CRSP_CROP_X.Bits.CRSP_CROP_XSTART = 0;
                    ptr_out_param->CRSP_R1B_CRSP_CROP_X.Bits.CRSP_CROP_XEND = (int)ptr_out_param->CRSP_R1B_CRSP_OUT_IMG.Bits.CRSP_WD - 1;
                    ptr_out_param->CRSP_R1B_CRSP_CROP_Y.Bits.CRSP_CROP_YSTART = 0;
                    ptr_out_param->CRSP_R1B_CRSP_CROP_Y.Bits.CRSP_CROP_YEND = (int)ptr_out_param->CRSP_R1B_CRSP_OUT_IMG.Bits.CRSP_HT - 1;

                    ptr_out_param->YUVO_R1B_YUVO_STRIDE.Bits.YUVO_BUS_SIZE_EN = 0;
                    ptr_out_param->YUVO_R1B_YUVO_OFST_ADDR.Bits.YUVO_OFST_ADDR = 0;

					ptr_out_param->YUVBO_R1B_YUVBO_STRIDE.Bits.YUVBO_BUS_SIZE_EN = 0;
                    ptr_out_param->YUVBO_R1B_YUVBO_OFST_ADDR.Bits.YUVBO_OFST_ADDR = 0;
					ptr_out_param->YUVCO_R1B_YUVCO_STRIDE.Bits.YUVCO_BUS_SIZE_EN = 0;
                    ptr_out_param->YUVCO_R1B_YUVCO_OFST_ADDR.Bits.YUVCO_OFST_ADDR = 0;
			    }

                if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_RSSO_R2_EN)
                {
		            if (ptr_in_param->SW.TWIN_RSS_R2_OUT_WD > DUAL_RSS_R2_WD_ALL)
		            {
		                result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		                dual_driver_printf("Error: %s\n", print_error_message(result));
		                return result;
		            }

                    /* setup RSS_R2 IN_WD and IN_HT */
                    ptr_out_param->RSS_R2B_RSS_IN_IMG.Bits.RSS_IN_WD = yuv_in_w;
		            ptr_out_param->RSS_R2B_RSS_IN_IMG.Bits.RSS_IN_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		                ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		            /* check horizontal oefficient step */
                    long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST - (ptr_in_param->SW.TWIN_RSS_R2_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST_LAST;

		    	    // cubic accumulation
		    	    ptr_out_param->RSS_R2B_RSS_HORI_STEP.Bits.RSS_HORI_STEP = ((long long)ptr_in_param->SW.TWIN_RSS_R2_OUT_WD * DUAL_RSS_PREC_VAL_CUB_ACC) / crop_in_size;

		    	    /* scaling down error check */
		            if (ptr_out_param->RSS_R2B_RSS_HORI_STEP.Bits.RSS_HORI_STEP > DUAL_RSS_PREC_VAL_CUB_ACC)
		            {
		    	        result = DUAL_MESSAGE_RSS_SCALING_UP_ERROR;
		    	        dual_driver_printf("Error: %s\n", print_error_message(result));
		    	        return result;
		            }

                    if (0 == ptr_in_param->RSS_R2A_RSS_VERT_STEP.Bits.RSS_VERT_STEP)
                    {
		                /* Check vertical coefficient step */
		                if (ptr_in_param->SW.TWIN_RSS_R2_IN_CROP_HT <= 1)
	                    {
		                    result = DUAL_MESSAGE_INVALID_RSS_IN_CROP_HT_ERROR;
		                    dual_driver_printf("Error: %s\n", print_error_message(result));
		                    return result;
	                    }
	                    else
	                    {
		                    // cubic accumulation
		    	            ptr_out_param->RSS_R2B_RSS_VERT_STEP.Bits.RSS_VERT_STEP = ((long long)((unsigned int)ptr_in_param->RSS_R2A_RSS_OUT_IMG.Bits.RSS_OUT_HT - 1) * DUAL_RSS_PREC_VAL_CUB_ACC +
		    	                ptr_in_param->SW.TWIN_RSS_R2_IN_CROP_HT - 2) / (ptr_in_param->SW.TWIN_RSS_R2_IN_CROP_HT - 1);
		             	    //if (ptr_out_param->RSS_R2B_RSS_VERT_STEP.Bits.RSS_VERT_STEP > DUAL_RSS_PREC_VAL_CUB_ACC)
		                    //{
		    	            //    result = DUAL_MESSAGE_RSS_SCALING_UP_ERROR;
		    	            //    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            //    return result;
		                    //}
	                    }
	                }
	                else
	                {
	                    ptr_out_param->RSS_R2B_RSS_VERT_STEP.Bits.RSS_VERT_STEP = ptr_in_param->RSS_R2A_RSS_VERT_STEP.Bits.RSS_VERT_STEP;
	                }

                    // setup remain parameters
					ptr_out_param->RSS_R2B_RSS_OUT_IMG.Bits.RSS_OUT_WD = ptr_in_param->SW.TWIN_RSS_R2_OUT_WD;
					ptr_out_param->RSS_R2B_RSS_OUT_IMG.Bits.RSS_OUT_HT = ptr_in_param->RSS_R2A_RSS_OUT_IMG.Bits.RSS_OUT_HT;
                    long long temp = (long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST * ptr_out_param->RSS_R2B_RSS_HORI_STEP.Bits.RSS_HORI_STEP +
                        (((long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_SUB_OFST * ptr_out_param->RSS_R2B_RSS_HORI_STEP.Bits.RSS_HORI_STEP) >> DUAL_RSS_CONFIG_BITS);
					ptr_out_param->RSS_R2B_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_RSS_PREC_BITS_CUB_ACC);
		    		ptr_out_param->RSS_R2B_RSS_LUMA_HORI_SUB_OFST.Bits.RSS_LUMA_HORI_SUB_OFST = (int)(temp -
		    			(long long)ptr_out_param->RSS_R2B_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST * DUAL_RSS_PREC_VAL_CUB_ACC);

					// copy from A to B
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R3_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN;
	                ptr_out_param->RSSO_R2B_RSSO_CON.Raw = ptr_in_param->RSSO_R2A_RSSO_CON.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_CON2.Raw = ptr_in_param->RSSO_R2A_RSSO_CON2.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_CON3.Raw = ptr_in_param->RSSO_R2A_RSSO_CON3.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_CON4.Raw = ptr_in_param->RSSO_R2A_RSSO_CON4.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_STRIDE.Raw = ptr_in_param->RSSO_R2A_RSSO_STRIDE.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_CROP.Raw = ptr_in_param->RSSO_R2A_RSSO_CROP.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_XSIZE.Raw = ptr_in_param->RSSO_R2A_RSSO_XSIZE.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_YSIZE.Raw = ptr_in_param->RSSO_R2A_RSSO_YSIZE.Raw;

                    // disable pipeline A
			        ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R3_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN = 0;
		            ptr_out_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_RSSO_R2_EN = 0;
		    	}

		    	if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZO_R2_EN)
		    	{
		            if (ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD > DUAL_CRZ_R2_WD_ALL)
		            {
		                result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		                dual_driver_printf("Error: %s\n", print_error_message(result));
		                return result;
		            }

                    /* setup CRZ_R2 IN_WD and IN_HT */
		    	    ptr_out_param->CRZ_R2B_CRZ_IN_IMG.Bits.CRZ_IN_WD = yuv_in_w;
		            ptr_out_param->CRZ_R2B_CRZ_IN_IMG.Bits.CRZ_IN_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		                ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		            /* Check horizontal oefficient step */
		            if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		            {
                        long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST_LAST;

		                // 2-tap algorithm
                        ptr_out_param->CRZ_R2B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(crop_in_size - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                            (((unsigned int)ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD  - 1) >> 1)) / (ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD - 1);

		    	    	/* Scaling down error check */
		                if (ptr_out_param->CRZ_R2B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                {
		    	            result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	            dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            return result;
		                }
		    	    }
		    	    else
		    	    {
		    	        long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST_LAST;

		    	        // Source accumulation
		    	        ptr_out_param->CRZ_R2B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC + crop_in_size - 2) / (crop_in_size - 1);

		                /* Scaling down error check */
		                if (ptr_out_param->CRZ_R2B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                {
		    	            result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	            dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            return result;
		                }
		    	    }

                    if (0 == ptr_in_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP)
                    {
		                /* Check vertical coefficient step */
		                if (ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT <= 1)
	                    {
		                    result = DUAL_MESSAGE_INVALID_CRZ_IN_CROP_HT_ERROR;
		                    dual_driver_printf("Error: %s\n", print_error_message(result));
		                    return result;
	                    }
	                    else
	                    {
		                    /* Recalculate CRZ_R2 CRZ_VERT_STEP */
		                    if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_VERT_ALGO)
		                    {
		                        // 2-tap algorithm
                                ptr_out_param->CRZ_R2B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                                    (((unsigned int)ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT  - 1) >> 1)) / ((unsigned int)ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1);
		                	    if (ptr_out_param->CRZ_R2B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                        {
		    	                    result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                    return result;
		                        }
		                    }
		                    else
		                    {
		    	                // Source accumulation
		    	                ptr_out_param->CRZ_R2B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)((unsigned int)ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC +
		    	                    ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT - 2) / (ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT - 1);
		    	                if (ptr_out_param->CRZ_R2B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                        {
		    	                    result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                    return result;
		                        }
		                    }
	                    }
	                }

                    // setup remain parameters
					ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD;
					ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_HT = ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT;
                    if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
					{
					    long long temp = (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP +
                            (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS);
                        ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_CRZ_PREC_BITS_2_TAP);
		    			ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp -
		    			     (long long)ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP);
					    ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST >> 1;
                        if (0 == (ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST & 0x1))
                        {
                            ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                        }
                        else
                        {
                            ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST + DUAL_CRZ_PREC_VAL_2_TAP;
                        }
                    }
                    else
                    {
                        long long temp = (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * ptr_out_param->CRZ_R2B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
                            (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * ptr_out_param->CRZ_R2B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS);
                        ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_CRZ_PREC_BITS_SRC_ACC);
		    			ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp -
		    			    (long long)ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_SRC_ACC);
                        ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST;
                        ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                    }

                    /* CRSP_R2B */
                    ptr_out_param->CRSP_R2B_CRSP_OUT_IMG.Bits.CRSP_WD = ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD;
                    ptr_out_param->CRSP_R2B_CRSP_OUT_IMG.Bits.CRSP_HT = ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_HT;
                    ptr_out_param->CRSP_R2B_CRSP_CROP_X.Bits.CRSP_CROP_XSTART = 0;
                    ptr_out_param->CRSP_R2B_CRSP_CROP_X.Bits.CRSP_CROP_XEND = (int)ptr_out_param->CRSP_R2B_CRSP_OUT_IMG.Bits.CRSP_WD - 1;
                    ptr_out_param->CRSP_R2B_CRSP_CROP_Y.Bits.CRSP_CROP_YSTART = 0;
                    ptr_out_param->CRSP_R2B_CRSP_CROP_Y.Bits.CRSP_CROP_YEND = (int)ptr_out_param->CRSP_R2B_CRSP_OUT_IMG.Bits.CRSP_HT - 1;

                    // copy from A to B
		    	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R2_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRSP_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRSP_R2_EN;
	                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R3_EN;
	                ptr_out_param->CRZO_R2B_CRZO_CON.Raw = ptr_in_param->CRZO_R2A_CRZO_CON.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_CON2.Raw = ptr_in_param->CRZO_R2A_CRZO_CON2.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_CON3.Raw = ptr_in_param->CRZO_R2A_CRZO_CON3.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_CON4.Raw = ptr_in_param->CRZO_R2A_CRZO_CON4.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_STRIDE.Raw = ptr_in_param->CRZO_R2A_CRZO_STRIDE.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_CROP.Raw = ptr_in_param->CRZO_R2A_CRZO_CROP.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_XSIZE.Raw = ptr_in_param->CRZO_R2A_CRZO_XSIZE.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_YSIZE.Raw = ptr_in_param->CRZO_R2A_CRZO_YSIZE.Raw;
	                if (1)
                    {
                        ptr_out_param->CRZBO_R2B_CRZBO_CON.Raw = ptr_in_param->CRZBO_R2A_CRZBO_CON.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_CON2.Raw = ptr_in_param->CRZBO_R2A_CRZBO_CON2.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_CON3.Raw = ptr_in_param->CRZBO_R2A_CRZBO_CON3.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_CON4.Raw = ptr_in_param->CRZBO_R2A_CRZBO_CON4.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_STRIDE.Raw = ptr_in_param->CRZBO_R2A_CRZBO_STRIDE.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_CROP.Raw = ptr_in_param->CRZBO_R2A_CRZBO_CROP.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_XSIZE.Raw = ptr_in_param->CRZBO_R2A_CRZBO_XSIZE.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_YSIZE.Raw = ptr_in_param->CRZBO_R2A_CRZBO_YSIZE.Raw;
                    }

                    // disable pipeline A
		    	    ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R2_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRSP_R2_EN = 0;
	                ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R3_EN = 0;
		    	    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZO_R2_EN = 0;
		    	    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZBO_R2_EN = 0;
		    	}

                if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_CRZO_R1_EN)
                {
		            if (ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD > DUAL_CRZ_R1_WD_ALL)
		            {
		                result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		                dual_driver_printf("Error: %s\n", print_error_message(result));
		                return result;
		            }

		    	    /* setup CRZ_R1 IN_WD and IN_HT */
		    	    ptr_out_param->CRZ_R1B_CRZ_IN_IMG.Bits.CRZ_IN_WD = yuv_in_w;
		            ptr_out_param->CRZ_R1B_CRZ_IN_IMG.Bits.CRZ_IN_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		                ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		            /* Check horizontal oefficient step */
		            if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		            {
                        long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST_LAST;

		                // 2-tap algorithm
                        ptr_out_param->CRZ_R1B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(crop_in_size - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                            (((unsigned int)ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD  - 1) >> 1)) / ((long long)ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD - 1);

		    	    	/* Scaling down error check */
		                if (ptr_out_param->CRZ_R1B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                {
		    	            result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	            dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            return result;
		                }
		    	    }
		    	    else
		    	    {
		    	        long long crop_in_size = (long long)yuv_in_w - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST_LAST;

		    	        // Source accumulation
		    	        ptr_out_param->CRZ_R1B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC + crop_in_size - 2) / (crop_in_size - 1);

		                /* Scaling down error check */
		                if (ptr_out_param->CRZ_R1B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                {
		    	            result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	            dual_driver_printf("Error: %s\n", print_error_message(result));
		    	            return result;
		                }
		    	    }

                    if (0 == ptr_in_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP)
                    {
		                /* Check vertical coefficient step */
		                if (ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT <= 1)
	                    {
		                    result = DUAL_MESSAGE_INVALID_CRZ_IN_CROP_HT_ERROR;
		                    dual_driver_printf("Error: %s\n", print_error_message(result));
		                    return result;
	                    }
	                    else
	                    {
		                    /* Recalculate CRZ_R1 CRZ_VERT_STEP */
		                    if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_VERT_ALGO)
		                    {
		                        // 2-tap algorithm
                                ptr_out_param->CRZ_R1B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                                    (((unsigned int)ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT  - 1) >> 1)) / ((unsigned int)ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1);
		                	    if (ptr_out_param->CRZ_R1B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                        {
		    	                    result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                    return result;
		                        }
		                    }
		                    else
		                    {
		    	                // Source accumulation
		    	                ptr_out_param->CRZ_R1B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)((unsigned int)ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC +
		    	                    ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT - 2) / (ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT - 1);
		    	                if (ptr_out_param->CRZ_R1B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                        {
		    	                    result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                    return result;
		                        }
		                    }
		                }
	                }
	                else
	                {
	                    ptr_out_param->CRZ_R1B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ptr_in_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP;
	                }

		    	    // setup remain parameters
					ptr_out_param->CRZ_R1B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD;
					ptr_out_param->CRZ_R1B_CRZ_OUT_IMG.Bits.CRZ_OUT_HT = ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT;
                    if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
					{
					    long long temp = (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP +
                            (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS);
                        ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_CRZ_PREC_BITS_2_TAP);
		    			ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp -
		    			     (long long)ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP);
					    ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST >> 1;
                        if (0 == (ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST & 0x1))
                        {
                            ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                        }
                        else
                        {
                            ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST + DUAL_CRZ_PREC_VAL_2_TAP;
                        }
                    }
                    else
                    {
                        long long temp = (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * ptr_out_param->CRZ_R1B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
                            (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * ptr_out_param->CRZ_R1B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS);
                        ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp >> DUAL_CRZ_PREC_BITS_SRC_ACC);
		    			ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp -
		    			    (long long)ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_SRC_ACC);
                        ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST;
                        ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                    }

                    // copy from A to B
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_GGM_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_GGM_R2_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_G2C_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_G2C_R2_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_C42_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_C42_R2_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R1_EN;
                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN;
	                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R2_EN;
	                ptr_out_param->CRZO_R1B_CRZO_CON.Raw = ptr_in_param->CRZO_R1A_CRZO_CON.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_CON2.Raw = ptr_in_param->CRZO_R1A_CRZO_CON2.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_CON3.Raw = ptr_in_param->CRZO_R1A_CRZO_CON3.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_CON4.Raw = ptr_in_param->CRZO_R1A_CRZO_CON4.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_STRIDE.Raw = ptr_in_param->CRZO_R1A_CRZO_STRIDE.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_CROP.Raw = ptr_in_param->CRZO_R1A_CRZO_CROP.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_XSIZE.Raw = ptr_in_param->CRZO_R1A_CRZO_XSIZE.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_YSIZE.Raw = ptr_in_param->CRZO_R1A_CRZO_YSIZE.Raw;
                    if (1)
                    {
                        ptr_out_param->CRZBO_R1B_CRZBO_CON.Raw = ptr_in_param->CRZBO_R1A_CRZBO_CON.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_CON2.Raw = ptr_in_param->CRZBO_R1A_CRZBO_CON2.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_CON3.Raw = ptr_in_param->CRZBO_R1A_CRZBO_CON3.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_CON4.Raw = ptr_in_param->CRZBO_R1A_CRZBO_CON4.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_STRIDE.Raw = ptr_in_param->CRZBO_R1A_CRZBO_STRIDE.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_CROP.Raw = ptr_in_param->CRZBO_R1A_CRZBO_CROP.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_XSIZE.Raw = ptr_in_param->CRZBO_R1A_CRZBO_XSIZE.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_YSIZE.Raw = ptr_in_param->CRZBO_R1A_CRZBO_YSIZE.Raw;
                    }

                    // disable pipeline A
		    	    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_GGM_R2_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_G2C_R2_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_C42_R2_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R1_EN = 0;
                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN = 0;
	                ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R2_EN = 0;
		    	    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_CRZO_R1_EN = 0;
		    	    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_CRZBO_R1_EN = 0;
		        }
			}
        }
        else
        {
            int mrg_r13_valid[3];
            int mrg_r13b_xstart;
            int mrg_r13a_xend;

            if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)
            {
                mrg_r13_valid[0] = sep_r1_valid_width[0];
                mrg_r13_valid[1] = sep_r1_valid_width[1];
            }
            else
            {
                mrg_r13_valid[0] = ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
                mrg_r13_valid[1] = ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
            }

            mrg_r13a_xend = mrg_r13_valid[0] - 1;
	        mrg_r13b_xstart = mrg_r13_valid[0];

	        if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_YUVO_R1_EN)
	        {
	            int crp_r4b_xstart;

	            int bit_per_pixel = 10, bit_per_pixel_uv = 10;
	            int crp_cut_shift = 4, yuvo_plane_num  = 1;

                if ((ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN && (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG != ptr_in_param->PAK_R4A_PAK_CONT.Bits.PAK_YUV_DNG)) ||
				    (ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN && (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG != ptr_in_param->PAK_R5A_PAK_CONT.Bits.PAK_YUV_DNG)))
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	        	    dual_driver_printf("Error: %s\n", print_error_message(result));
	        	    return result;
				}

                if ((ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN && (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT != ptr_in_param->PAK_R4A_PAK_CONT.Bits.PAK_YUV_BIT)) ||
				    (ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN && (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT != ptr_in_param->PAK_R5A_PAK_CONT.Bits.PAK_YUV_BIT)))
				{
					result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	        	    dual_driver_printf("Error: %s\n", print_error_message(result));
	        	    return result;
				}

                /* YUVO */
		    	int bus_width_shift = 2;  /* (2 << 2) * 8, 32 bits */
    	        switch (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_YUVO_R1_FMT)
 	            {
	        	    case DUAL_OUT_FMT_YUV422:
	        		    bit_per_pixel  = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 32: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 20: 16));
	        		    crp_cut_shift  = 3;  // Y: 3, UV: NA
	        		    yuvo_plane_num = 1;
	        			break;
	        	    case DUAL_OUT_FMT_YUV422_2:
						bit_per_pixel    = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 16: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 10: 8));
		    			bit_per_pixel_uv = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 16: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 10: 8));
	        		    crp_cut_shift    = 4;  // Y: 4, UV: 4
	        		    yuvo_plane_num   = 2;
	        			break;
	        	    case DUAL_OUT_FMT_YUV422_3:
	        		    bit_per_pixel    = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 16: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 10: 8));
		    			bit_per_pixel_uv = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 8: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 5: 4));
	        		    crp_cut_shift    = 5;  // Y: 4, UV: 5
	        		    yuvo_plane_num   = 3;
	        			break;
	        	    case DUAL_OUT_FMT_YUV420_2:
	        		    bit_per_pixel    = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 16: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 10: 8));
		    			bit_per_pixel_uv = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 16: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 10: 8));
	        		    crp_cut_shift    = 4;  // Y: 4, UV: 4
	        		    yuvo_plane_num   = 2;
	        			break;
		    		case DUAL_OUT_FMT_YUV420_3:
                        bit_per_pixel    = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 16: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 10: 8));
		    			bit_per_pixel_uv = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 8: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 5: 4));
		    			crp_cut_shift    = 5;  // Y: 4, UV: 5
		    			yuvo_plane_num   = 3;
		    			break;
	        	    case DUAL_OUT_FMT_Y:
	        	        bit_per_pixel  = (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_DNG? 16: (ptr_in_param->PAK_R3A_PAK_CONT.Bits.PAK_YUV_BIT? 10: 8));
	        	        crp_cut_shift  = 4;  // Y: 4, UV: NA
	        	        yuvo_plane_num = 1;
	        	        break;
            		default:
            		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	        	        dual_driver_printf("Error: %s\n", print_error_message(result));
	        	        return result;
   	            }

		    	crp_r4b_xstart = mrg_r13_valid[0] + DUAL_TILE_LOSS_YUV_L;
	    		if (crp_r4b_xstart & (DUAL_ALIGN_PIXEL_MODE(crp_cut_shift) - 1))
	    		{
                    crp_r4b_xstart = crp_r4b_xstart + DUAL_ALIGN_PIXEL_MODE(crp_cut_shift) - (crp_r4b_xstart & (DUAL_ALIGN_PIXEL_MODE(crp_cut_shift) - 1));
		    	}

                mrg_r13a_xend = ((crp_r4b_xstart - 1 + DUAL_TILE_LOSS_YUV) > mrg_r13a_xend)? (crp_r4b_xstart - 1 + DUAL_TILE_LOSS_YUV): mrg_r13a_xend;
		    	mrg_r13b_xstart = (crp_r4b_xstart - DUAL_TILE_LOSS_YUV_L);

                /* CRP_R4A */
		        ptr_out_param->CRP_R4A_CRP_X_POS.Bits.CRP_XSTART = 0;
		        ptr_out_param->CRP_R4A_CRP_X_POS.Bits.CRP_XEND = crp_r4b_xstart - 1;
		        ptr_out_param->CRP_R4A_CRP_Y_POS.Bits.CRP_YSTART = 0;
		        ptr_out_param->CRP_R4A_CRP_Y_POS.Bits.CRP_YEND = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		            (((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN) - 1):
		            (ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT - 1);


                /* CRSP_R1A */
                ptr_out_param->CRSP_R1A_CRSP_OUT_IMG.Bits.CRSP_WD = (int)ptr_out_param->CRP_R4A_CRP_X_POS.Bits.CRP_XEND - (int)ptr_out_param->CRP_R4A_CRP_X_POS.Bits.CRP_XSTART + 1;
                ptr_out_param->CRSP_R1A_CRSP_OUT_IMG.Bits.CRSP_HT = (int)ptr_out_param->CRP_R4A_CRP_Y_POS.Bits.CRP_YEND - (int)ptr_out_param->CRP_R4A_CRP_Y_POS.Bits.CRP_YSTART + 1;
                ptr_out_param->CRSP_R1A_CRSP_CROP_X.Bits.CRSP_CROP_XSTART = 0;
                ptr_out_param->CRSP_R1A_CRSP_CROP_X.Bits.CRSP_CROP_XEND = (int)ptr_out_param->CRSP_R1A_CRSP_OUT_IMG.Bits.CRSP_WD - 1;
                ptr_out_param->CRSP_R1A_CRSP_CROP_Y.Bits.CRSP_CROP_YSTART = 0;
                ptr_out_param->CRSP_R1A_CRSP_CROP_Y.Bits.CRSP_CROP_YEND = (int)ptr_out_param->CRSP_R1A_CRSP_OUT_IMG.Bits.CRSP_HT - 1;

	            /* YUVO_R1A */
		        ptr_out_param->YUVO_R1A_YUVO_STRIDE.Bits.YUVO_BUS_SIZE_EN = 0;
                ptr_out_param->YUVO_R1A_YUVO_OFST_ADDR.Bits.YUVO_OFST_ADDR = 0;
	            ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE = DUAL_CALC_BUS_XIZE(crp_r4b_xstart - (int)ptr_out_param->CRP_R4A_CRP_X_POS.Bits.CRP_XSTART, bit_per_pixel, bus_width_shift);
	            if ((crp_r4b_xstart - (int)ptr_out_param->CRP_R4A_CRP_X_POS.Bits.CRP_XSTART) & (DUAL_ALIGN_PIXEL_MODE(crp_cut_shift) - 1))
	            {
	                result = DUAL_MESSAGE_CRP_R3_SIZE_CAL_ERROR;
	                dual_driver_printf("Error: %s\n", print_error_message(result));
	                return result;
	            }
                if (yuvo_plane_num >= 2)
                {
                    // YUVBO_R1A
                    ptr_out_param->YUVBO_R1A_YUVBO_STRIDE.Bits.YUVBO_BUS_SIZE_EN = 0;
                    ptr_out_param->YUVBO_R1A_YUVBO_OFST_ADDR.Bits.YUVBO_OFST_ADDR = 0;
		            ptr_out_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE = DUAL_CALC_BUS_XIZE(crp_r4b_xstart - (int)ptr_out_param->CRP_R4A_CRP_X_POS.Bits.CRP_XSTART, bit_per_pixel_uv, bus_width_shift);

                    // YUVCO_R1A
                    ptr_out_param->YUVCO_R1A_YUVCO_STRIDE.Bits.YUVCO_BUS_SIZE_EN = 0;
                    ptr_out_param->YUVCO_R1A_YUVCO_OFST_ADDR.Bits.YUVCO_OFST_ADDR = 0;
		    	    ptr_out_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE = DUAL_CALC_BUS_XIZE(crp_r4b_xstart - (int)ptr_out_param->CRP_R4A_CRP_X_POS.Bits.CRP_XSTART, bit_per_pixel_uv, bus_width_shift);
                }

                /* clone registers */
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRP_R4_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRP_R4_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRSP_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRSP_R1_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R1_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R3_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN;
                ptr_out_param->YUVO_R1B_YUVO_CON.Raw = ptr_in_param->YUVO_R1A_YUVO_CON.Raw;
                ptr_out_param->YUVO_R1B_YUVO_CON2.Raw = ptr_in_param->YUVO_R1A_YUVO_CON2.Raw;
                ptr_out_param->YUVO_R1B_YUVO_CON3.Raw = ptr_in_param->YUVO_R1A_YUVO_CON3.Raw;
                ptr_out_param->YUVO_R1B_YUVO_CON4.Raw = ptr_in_param->YUVO_R1A_YUVO_CON4.Raw;
                ptr_out_param->YUVO_R1B_YUVO_STRIDE.Raw = ptr_in_param->YUVO_R1A_YUVO_STRIDE.Raw;
                ptr_out_param->YUVO_R1B_YUVO_CROP.Bits.YUVO_YOFFSET = ptr_in_param->YUVO_R1A_YUVO_CROP.Bits.YUVO_YOFFSET;
                ptr_out_param->YUVO_R1B_YUVO_YSIZE.Bits.YUVO_YSIZE = ptr_in_param->YUVO_R1A_YUVO_YSIZE.Bits.YUVO_YSIZE;
                if (yuvo_plane_num >= 2)
                {
                    // YUVBO_R1B
                    ptr_out_param->YUVBO_R1B_YUVBO_CON.Raw = ptr_in_param->YUVBO_R1A_YUVBO_CON.Raw;
                    ptr_out_param->YUVBO_R1B_YUVBO_CON2.Raw = ptr_in_param->YUVBO_R1A_YUVBO_CON2.Raw;
                    ptr_out_param->YUVBO_R1B_YUVBO_CON3.Raw = ptr_in_param->YUVBO_R1A_YUVBO_CON3.Raw;
                    ptr_out_param->YUVBO_R1B_YUVBO_CON4.Raw = ptr_in_param->YUVBO_R1A_YUVBO_CON4.Raw;
                    ptr_out_param->YUVBO_R1B_YUVBO_STRIDE.Raw = ptr_in_param->YUVBO_R1A_YUVBO_STRIDE.Raw;
                    ptr_out_param->YUVBO_R1B_YUVBO_CROP.Bits.YUVBO_YOFFSET = ptr_in_param->YUVBO_R1A_YUVBO_CROP.Bits.YUVBO_YOFFSET;
                    ptr_out_param->YUVBO_R1B_YUVBO_YSIZE.Bits.YUVBO_YSIZE = ptr_in_param->YUVBO_R1A_YUVBO_YSIZE.Bits.YUVBO_YSIZE;

                    // YUVCO_R1B
                    ptr_out_param->YUVCO_R1B_YUVCO_CON.Raw = ptr_in_param->YUVCO_R1A_YUVCO_CON.Raw;
                    ptr_out_param->YUVCO_R1B_YUVCO_CON2.Raw = ptr_in_param->YUVCO_R1A_YUVCO_CON2.Raw;
                    ptr_out_param->YUVCO_R1B_YUVCO_CON3.Raw = ptr_in_param->YUVCO_R1A_YUVCO_CON3.Raw;
                    ptr_out_param->YUVCO_R1B_YUVCO_CON4.Raw = ptr_in_param->YUVCO_R1A_YUVCO_CON4.Raw;
                    ptr_out_param->YUVCO_R1B_YUVCO_STRIDE.Raw = ptr_in_param->YUVCO_R1A_YUVCO_STRIDE.Raw;
                    ptr_out_param->YUVCO_R1B_YUVCO_CROP.Bits.YUVCO_YOFFSET = ptr_in_param->YUVCO_R1A_YUVCO_CROP.Bits.YUVCO_YOFFSET;
                    ptr_out_param->YUVCO_R1B_YUVCO_YSIZE.Bits.YUVCO_YSIZE = ptr_in_param->YUVCO_R1A_YUVCO_YSIZE.Bits.YUVCO_YSIZE;
                }

                ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XSTART = crp_r4b_xstart - mrg_r13_valid[0];
	            ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XEND = mrg_r13_valid[1] - 1;

	            ptr_out_param->CRP_R4B_CRP_Y_POS.Bits.CRP_YSTART = 0;
	            ptr_out_param->CRP_R4B_CRP_Y_POS.Bits.CRP_YEND = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
	                (((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN) - 1):
	                (ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT - 1);

                /* CRSP_R1B */
                ptr_out_param->CRSP_R1B_CRSP_OUT_IMG.Bits.CRSP_WD = (int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XEND - (int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XSTART + 1;
                ptr_out_param->CRSP_R1B_CRSP_OUT_IMG.Bits.CRSP_HT = ptr_out_param->CRSP_R1A_CRSP_OUT_IMG.Bits.CRSP_HT;
                ptr_out_param->CRSP_R1B_CRSP_CROP_X.Bits.CRSP_CROP_XSTART = 0;
                ptr_out_param->CRSP_R1B_CRSP_CROP_X.Bits.CRSP_CROP_XEND = (int)ptr_out_param->CRSP_R1B_CRSP_OUT_IMG.Bits.CRSP_WD - 1;
                ptr_out_param->CRSP_R1B_CRSP_CROP_Y.Bits.CRSP_CROP_YSTART = 0;
                ptr_out_param->CRSP_R1B_CRSP_CROP_Y.Bits.CRSP_CROP_YEND = (int)ptr_out_param->CRSP_R1B_CRSP_OUT_IMG.Bits.CRSP_HT - 1;

                /* YUVO_R1B */
		        ptr_out_param->YUVO_R1B_YUVO_STRIDE.Bits.YUVO_BUS_SIZE_EN = 0;
                ptr_out_param->YUVO_R1B_YUVO_XSIZE.Bits.YUVO_XSIZE = DUAL_CALC_BUS_XIZE((int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XEND -
                    (int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XSTART + 1, bit_per_pixel, bus_width_shift);
                if (yuvo_plane_num >= 2)
                {
                    ptr_out_param->YUVBO_R1B_YUVBO_STRIDE.Bits.YUVBO_BUS_SIZE_EN = 0;
                    ptr_out_param->YUVBO_R1B_YUVBO_XSIZE.Bits.YUVBO_XSIZE = DUAL_CALC_BUS_XIZE((int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XEND -
                        (int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XSTART + 1, bit_per_pixel_uv, bus_width_shift);

                    ptr_out_param->YUVCO_R1B_YUVCO_STRIDE.Bits.YUVCO_BUS_SIZE_EN = 0;
                    ptr_out_param->YUVCO_R1B_YUVCO_XSIZE.Bits.YUVCO_XSIZE = DUAL_CALC_BUS_XIZE((int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XEND -
                        (int)ptr_out_param->CRP_R4B_CRP_X_POS.Bits.CRP_XSTART + 1, bit_per_pixel_uv, bus_width_shift);
		    	}

	            /* RTL */
	            if (ptr_in_param->YUVO_R1A_YUVO_BASE_ADDR.Bits.YUVO_BASE_ADDR == ptr_in_param->YUVO_R1B_YUVO_BASE_ADDR.Bits.YUVO_BASE_ADDR)
	            {
		    	    /* A+B */
		    	    if ((int)ptr_in_param->YUVO_R1A_YUVO_CROP.Bits.YUVO_XOFFSET * (1 << bus_width_shift) >= (int)ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE + 1)
		    	    {
		    		    /* B only */
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRP_R4_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRSP_R1_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R1_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R3_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN = 0;
	                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN = 0;
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_YUVO_R1_EN = 0;
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_YUVBO_R1_EN = 0;
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_YUVCO_R1_EN = 0;
		    		    /* B only */
		    		    ptr_out_param->YUVO_R1B_YUVO_OFST_ADDR.Bits.YUVO_OFST_ADDR = 0;
		    		    ptr_out_param->YUVO_R1B_YUVO_CROP.Bits.YUVO_XOFFSET = (int)ptr_in_param->YUVO_R1A_YUVO_CROP.Bits.YUVO_XOFFSET -
		    		        (((unsigned int)ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE + 1) >> bus_width_shift);
		    		    ptr_out_param->YUVO_R1B_YUVO_XSIZE.Bits.YUVO_XSIZE = ptr_in_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE;
		    		    if (yuvo_plane_num >= 2)
                        {
                            ptr_out_param->YUVBO_R1B_YUVBO_OFST_ADDR.Bits.YUVBO_OFST_ADDR = 0;
		    		        ptr_out_param->YUVBO_R1B_YUVBO_CROP.Bits.YUVBO_XOFFSET = (int)ptr_in_param->YUVBO_R1A_YUVBO_CROP.Bits.YUVBO_XOFFSET -
		    			        (((unsigned int)ptr_out_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE + 1) >> bus_width_shift);
		    		        ptr_out_param->YUVBO_R1B_YUVBO_XSIZE.Bits.YUVBO_XSIZE = ptr_in_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE;
		    		    }
		    		    if (yuvo_plane_num >= 3)
                        {
                            ptr_out_param->YUVCO_R1B_YUVCO_OFST_ADDR.Bits.YUVCO_OFST_ADDR = 0;
		    		        ptr_out_param->YUVCO_R1B_YUVCO_CROP.Bits.YUVCO_XOFFSET = (int)ptr_in_param->YUVCO_R1A_YUVCO_CROP.Bits.YUVCO_XOFFSET -
		    			        (((unsigned int)ptr_out_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE + 1) >> bus_width_shift);
		    		        ptr_out_param->YUVCO_R1B_YUVCO_XSIZE.Bits.YUVCO_XSIZE = ptr_in_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE;
		    		    }
		    	    }
		    	    else
		    	    {
		    		    /* A+B */
		    		    if ((int)ptr_in_param->YUVO_R1A_YUVO_CROP.Bits.YUVO_XOFFSET * (1 << bus_width_shift) +
		    		        (int)ptr_in_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE <= (int)ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE)
		    		    {
		    		        /* A only */
		    			    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRP_R4_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRSP_R1_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R1_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R3_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R4_EN = 0;
	                        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PAK_R5_EN = 0;
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_DMA2_EN.Bits.CAMCTL_YUVO_R1_EN = 0;
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_YUVBO_R1_EN = 0;
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_YUVCO_R1_EN = 0;
		    			    /* A only */
		    			    ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE = ptr_in_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE;
		    			    if (yuvo_plane_num >= 2)
                            {
                                ptr_out_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE = ptr_in_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE;
		    			    }
		    			    if (yuvo_plane_num >= 3)
                            {
                                ptr_out_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE = ptr_in_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE;
		    			    }
		    		    }
		    		    else
		    		    {
		    			    /* A+B */
		    			    ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE -= (int)ptr_in_param->YUVO_R1A_YUVO_CROP.Bits.YUVO_XOFFSET * (1 << bus_width_shift);
		    			    ptr_out_param->YUVO_R1B_YUVO_CROP.Bits.YUVO_XOFFSET = 0;
		    			    ptr_out_param->YUVO_R1B_YUVO_OFST_ADDR.Bits.YUVO_OFST_ADDR = (int)ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE + 1;
		    			    ptr_out_param->YUVO_R1B_YUVO_XSIZE.Bits.YUVO_XSIZE = (int)ptr_in_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE -
		    			        ((int)ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE + 1);
		    		        if (yuvo_plane_num >= 2)
                            {
                                ptr_out_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE -= (int)ptr_in_param->YUVBO_R1A_YUVBO_CROP.Bits.YUVBO_XOFFSET * (1 << bus_width_shift);
		    			        ptr_out_param->YUVBO_R1B_YUVBO_CROP.Bits.YUVBO_XOFFSET = 0;
		    			        ptr_out_param->YUVBO_R1B_YUVBO_OFST_ADDR.Bits.YUVBO_OFST_ADDR = (int)ptr_out_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE + 1;
		    			        ptr_out_param->YUVBO_R1B_YUVBO_XSIZE.Bits.YUVBO_XSIZE = (int)ptr_in_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE -
		    				        ((int)ptr_out_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE + 1);
                            }
                            if (yuvo_plane_num >= 3)
                            {
                                ptr_out_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE -= (int)ptr_in_param->YUVCO_R1A_YUVCO_CROP.Bits.YUVCO_XOFFSET * (1 << bus_width_shift);
		    			        ptr_out_param->YUVCO_R1B_YUVCO_CROP.Bits.YUVCO_XOFFSET = 0;
		    			        ptr_out_param->YUVCO_R1B_YUVCO_OFST_ADDR.Bits.YUVCO_OFST_ADDR = (int)ptr_out_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE + 1;
		    			        ptr_out_param->YUVCO_R1B_YUVCO_XSIZE.Bits.YUVCO_XSIZE = (int)ptr_in_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE -
		    				        ((int)ptr_out_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE + 1);
                            }
		    		    }
		    	    }
	            }
	            else
	            {
		            ptr_out_param->YUVO_R1B_YUVO_OFST_ADDR.Bits.YUVO_OFST_ADDR = 0;
		            /* restore stride */
		            ptr_out_param->YUVO_R1B_YUVO_STRIDE.Bits.YUVO_STRIDE = ptr_in_param->YUVO_R1B_YUVO_STRIDE.Bits.YUVO_STRIDE;
		            /* check valid size only WDMA with XOFF and YOFF support */
		            if ((int)ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE + 1 > (int)ptr_in_param->YUVO_R1A_YUVO_STRIDE.Bits.YUVO_STRIDE)
		            {
		    	        ptr_out_param->YUVO_R1A_YUVO_XSIZE.Bits.YUVO_XSIZE = (((unsigned int)ptr_in_param->YUVO_R1A_YUVO_STRIDE.Bits.YUVO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
		            }
		            if ((int)ptr_out_param->YUVO_R1B_YUVO_XSIZE.Bits.YUVO_XSIZE + 1 > (int)ptr_in_param->YUVO_R1B_YUVO_STRIDE.Bits.YUVO_STRIDE)
		            {
		    	        ptr_out_param->YUVO_R1B_YUVO_XSIZE.Bits.YUVO_XSIZE = (((unsigned int)ptr_in_param->YUVO_R1B_YUVO_STRIDE.Bits.YUVO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
		            }
		            if (yuvo_plane_num >= 2)
                    {
                        ptr_out_param->YUVBO_R1B_YUVBO_OFST_ADDR.Bits.YUVBO_OFST_ADDR = 0;
		                /* restore stride */
		                ptr_out_param->YUVBO_R1B_YUVBO_STRIDE.Bits.YUVBO_STRIDE = ptr_in_param->YUVBO_R1B_YUVBO_STRIDE.Bits.YUVBO_STRIDE;
		                /* check valid size only WDMA with XOFF and YOFF support */
		                if ((int)ptr_out_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE + 1 > (int)ptr_in_param->YUVBO_R1A_YUVBO_STRIDE.Bits.YUVBO_STRIDE)
		                {
		    	            ptr_out_param->YUVBO_R1A_YUVBO_XSIZE.Bits.YUVBO_XSIZE = (((unsigned int)ptr_in_param->YUVBO_R1A_YUVBO_STRIDE.Bits.YUVBO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
		                }
		                if ((int)ptr_out_param->YUVBO_R1B_YUVBO_XSIZE.Bits.YUVBO_XSIZE + 1 > (int)ptr_in_param->YUVBO_R1B_YUVBO_STRIDE.Bits.YUVBO_STRIDE)
		                {
		    	            ptr_out_param->YUVBO_R1B_YUVBO_XSIZE.Bits.YUVBO_XSIZE = (((unsigned int)ptr_in_param->YUVBO_R1B_YUVBO_STRIDE.Bits.YUVBO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
		                }
                    }
                    if (yuvo_plane_num >= 3)
                    {
                        ptr_out_param->YUVCO_R1B_YUVCO_OFST_ADDR.Bits.YUVCO_OFST_ADDR = 0;
		                /* restore stride */
		                ptr_out_param->YUVCO_R1B_YUVCO_STRIDE.Bits.YUVCO_STRIDE = ptr_in_param->YUVCO_R1B_YUVCO_STRIDE.Bits.YUVCO_STRIDE;
		                /* check valid size only WDMA with XOFF and YOFF support */
		                if ((int)ptr_out_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE + 1 > (int)ptr_in_param->YUVCO_R1A_YUVCO_STRIDE.Bits.YUVCO_STRIDE)
		                {
		    	            ptr_out_param->YUVCO_R1A_YUVCO_XSIZE.Bits.YUVCO_XSIZE = (((unsigned int)ptr_in_param->YUVCO_R1A_YUVCO_STRIDE.Bits.YUVCO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
		                }
		                if ((int)ptr_out_param->YUVCO_R1B_YUVCO_XSIZE.Bits.YUVCO_XSIZE + 1 > (int)ptr_in_param->YUVCO_R1B_YUVCO_STRIDE.Bits.YUVCO_STRIDE)
		                {
		    	            ptr_out_param->YUVCO_R1B_YUVCO_XSIZE.Bits.YUVCO_XSIZE = (((unsigned int)ptr_in_param->YUVCO_R1B_YUVCO_STRIDE.Bits.YUVCO_STRIDE >> bus_width_shift) << bus_width_shift) - 1;
		                }
		    		}
	            }
		    }

            if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_RSSO_R2_EN)
            {
                if (ptr_in_param->SW.TWIN_RSS_R2_OUT_WD > DUAL_RSS_R2_WD_ALL)
		        {
		            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		            dual_driver_printf("Error: %s\n", print_error_message(result));
		            return result;
		        }

                ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R3_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN;

                /* Update RSS_R1 in width */
                int rss_in_w = ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                        (((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		                          ((int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_E - (int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_S):
		                          ptr_in_param->SW.TWIN_RAWI_XSIZE) >>
		                         ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
                                  (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN +
		                          (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN)): ptr_in_param->SW.TWIN_RRZ_OUT_WD);
                int out_xe_rss;
		    	int out_shift[3] = { 1, 1, 1 };

		        /* check RSS_R2 alignment */
		    	//if (DUAL_OUT_FMT_YUV422_8 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_RSSO_R2_FMT)
		    	//{
		    	//	out_shift[0] = 1;  /* 2 << 3, x16 */
		    	//}
		    	//else
		    	//{
		    	//	result = DUAL_MESSAGE_RSSO_CONFIG_ERROR;
		    	//	dual_driver_printf("Error: %s\n", print_error_message(result));
		    	//	return result;
		    	//}

		        /* setup RSS_R2 IN_HT */
		        ptr_out_param->RSS_R2A_RSS_IN_IMG.Bits.RSS_IN_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		            ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		            ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		        /* check horizontal oefficient step */
                long long crop_in_size = (long long)rss_in_w - ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST - (ptr_in_param->SW.TWIN_RSS_R2_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST_LAST;

		    	// cubic accumulation
		    	ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP = ((long long)ptr_in_param->SW.TWIN_RSS_R2_OUT_WD * DUAL_RSS_PREC_VAL_CUB_ACC) / crop_in_size;

		    	/* scaling down error check */
		        if (ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP > DUAL_RSS_PREC_VAL_CUB_ACC)
		        {
		    	    result = DUAL_MESSAGE_RSS_SCALING_UP_ERROR;
		    	    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	    return result;
		        }

                if (0 == ptr_in_param->RSS_R2A_RSS_VERT_STEP.Bits.RSS_VERT_STEP)
                {
		            /* Check vertical coefficient step */
		            if (ptr_in_param->SW.TWIN_RSS_R2_IN_CROP_HT <= 1)
	                {
		                result = DUAL_MESSAGE_INVALID_RSS_IN_CROP_HT_ERROR;
		                dual_driver_printf("Error: %s\n", print_error_message(result));
		                return result;
	                }
	                else
	                {
		                // cubic accumulation
		    	        ptr_out_param->RSS_R2A_RSS_VERT_STEP.Bits.RSS_VERT_STEP = ((long long)((unsigned int)ptr_in_param->RSS_R2A_RSS_OUT_IMG.Bits.RSS_OUT_HT - 1) * DUAL_RSS_PREC_VAL_CUB_ACC +
		    	            ptr_in_param->SW.TWIN_RSS_R2_IN_CROP_HT - 2) / (ptr_in_param->SW.TWIN_RSS_R2_IN_CROP_HT - 1);

		         	    //if (ptr_out_param->RSS_R2A_RSS_VERT_STEP.Bits.RSS_VERT_STEP > DUAL_RSS_PREC_VAL_CUB_ACC)
		                //{
		    	        //    result = DUAL_MESSAGE_RSS_SCALING_UP_ERROR;
		    	        //    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	        //    return result;
		                //}
	                }
	            }

		    	/* loop dual mode from right */
		    	for (int i = 0; i < 2; i++)
		    	{
		    	    int in_xs_mrg = 0;
		    	    int in_xs_rss = 0;
		    	    int in_xe_rss = 0;
		    	    int out_xs_rss = 0;

		    	    /* config forward backward pos */
		    	    switch(i)
		    	    {
		    		    case 0:
		    			    /* RSS_R2B */
		    			    /* init pos */
		    			    in_xs_mrg  = mrg_r13_valid[0];
		    			    /* Right boundary */
		    			    in_xs_rss  = mrg_r13b_xstart + DUAL_TILE_LOSS_YUV_L;
		    			    in_xe_rss  = mrg_r13_valid[0] + mrg_r13_valid[1] - 1;
		    			    out_xe_rss = ptr_in_param->SW.TWIN_RSS_R2_OUT_WD - 1;
		    			    break;
		    		    case 1:
		    			    /* RSS_R2A */
		    			    /* init pos */
		    			    in_xs_mrg  = 0;
		    			    in_xe_rss  = (mrg_r13_valid[0] + mrg_r13_valid[1] - 1) - DUAL_TILE_LOSS_YUV;
		    			    out_xs_rss = 0;
		    			    break;
		    		    default:
    	    				break;
		    	    }

		    	    /* backward & forward calculation */
		    	    if (out_xs_rss < out_xe_rss)
		    	    {
		    		    /* backward out_xe_rss */
		    		    long long end_temp = (long long)out_xe_rss * DUAL_RSS_PREC_VAL_CUB_ACC +
		    		        (long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP +
		    			    (((long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_SUB_OFST * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP) >> DUAL_RSS_CONFIG_BITS);
		    		    /* cal pos */
		    		    if ((end_temp + ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP + 2 * DUAL_RSS_PREC_VAL_CUB_ACC) <
		    		        (long long)in_xe_rss * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP)
		    		    {
		    			    int n = (int)((end_temp + ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP + 2 * DUAL_RSS_PREC_VAL_CUB_ACC) /
		    			        ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP);
		    			    if (((long long)n * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP) ==
		    			        (end_temp + ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP + 2 * DUAL_RSS_PREC_VAL_CUB_ACC))
		    			    {
		    			        n = n - 1;
		    			    }
		    			    if (n & 0x1)
		    			    {
		    			        in_xe_rss = n ;
		    			    }
		    			    else /* must be even */
		    			    {
		    				    in_xe_rss = n + 1;
		    			    }
		    		    }
		    		    else
		    		    {
		    			    if (in_xe_rss + 1 < ptr_in_param->SW.TWIN_RSS_R2_OUT_WD)
		    			    {
		    			        result = DUAL_MESSAGE_RSS_XS_XE_CAL_ERROR;
		    				    dual_driver_printf("Error: %s\n", print_error_message(result));
		    				    return result;
		    			    }
		    		    }
		    		    /* forward in_xs_rss */
		    		    if (in_xs_rss < in_xe_rss)
		    		    {
		    			    if (in_xs_rss <= 0)
		    			    {
		    			        out_xs_rss = 0;
		    			    }
		    			    else
		    			    {
		    				    long long start_temp = (long long)in_xs_rss * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP -
                                    (long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP -
                                    (((long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_SUB_OFST * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP) >> DUAL_RSS_CONFIG_BITS);
                                int n = (int)((unsigned long long)start_temp >> DUAL_RSS_PREC_BITS_CUB_ACC);
                                if (((long long)n * DUAL_RSS_PREC_VAL_CUB_ACC) < start_temp) /* ceiling be careful with value smaller than zero */
                                {
                                    n = n + 1;
                                }
                                n = n + 2;
		    				    if (n < 0)
		    				    {
		    					    n = 0;
		    				    }
		    				    /* update align by pixel mode */
		    				    /* RSS_R2A, run i= 0 only */
		    				    if (n & DUAL_RSS_MASK(out_shift[i]))
		    				    {
		    					    out_xs_rss = n + DUAL_RSS_ALIGN(out_shift[i]) - (n & DUAL_RSS_MASK(out_shift[i]));
		    				    }
		    				    else
		    				    {
		    					    out_xs_rss = n;
		    				    }
		    			    }
		    		    }
		    		}

		    	    /* update RSS width, offset, flag */
		    	    if ((out_xs_rss < out_xe_rss) && (in_xs_rss < in_xe_rss))
		    	    {
		    		    long long temp_offset = (long long)out_xs_rss * DUAL_RSS_PREC_VAL_CUB_ACC +
                                (long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_INT_OFST * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP +
                                (((long long)ptr_in_param->SW.TWIN_RSS_R2_HORI_SUB_OFST * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP) >> DUAL_RSS_CONFIG_BITS) -
                                (long long)in_xs_mrg * ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP;

		    		    switch (i)
		    		    {
		    			    case 0:/* to revise for RSS_R2B */
		    			        /* RSS_R2B */
		    				    ptr_out_param->RSS_R2B_RSS_OUT_IMG.Bits.RSS_OUT_WD = out_xe_rss - out_xs_rss + 1;
		    				    ptr_out_param->RSS_R2B_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST = ((unsigned int)(temp_offset >> DUAL_RSS_PREC_BITS_CUB_ACC));
		    				    ptr_out_param->RSS_R2B_RSS_LUMA_HORI_SUB_OFST.Bits.RSS_LUMA_HORI_SUB_OFST = (int)(temp_offset -
		    					    (long long)ptr_out_param->RSS_R2B_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST * DUAL_RSS_PREC_VAL_CUB_ACC);
		    				    break;
		    			    case 1:
		    				    /* RSS_R2A */
		    				    ptr_out_param->RSS_R2A_RSS_OUT_IMG.Bits.RSS_OUT_WD = out_xe_rss - out_xs_rss + 1;
		    				    ptr_out_param->RSS_R2A_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST = ((unsigned int)(temp_offset >> DUAL_RSS_PREC_BITS_CUB_ACC));
		    				    ptr_out_param->RSS_R2A_RSS_LUMA_HORI_SUB_OFST.Bits.RSS_LUMA_HORI_SUB_OFST = (int)(temp_offset -
		    				        (long long)ptr_out_param->RSS_R2A_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST * DUAL_RSS_PREC_VAL_CUB_ACC);

		    				    mrg_r13a_xend = ((in_xe_rss + DUAL_TILE_LOSS_YUV) > mrg_r13a_xend)? (in_xe_rss + DUAL_TILE_LOSS_YUV): mrg_r13a_xend;
								break;
		    			    default:
		    				    break;
		    		    }

		    		    /* Decrease out_xe to next start pos */
		    		    out_xe_rss = out_xs_rss - 1;
		    	    }
		    	    else
		    	    {
		    		    if ((out_xe_rss > 0) && (out_xe_rss + 1 < ptr_in_param->SW.TWIN_RSS_R2_OUT_WD) && (in_xs_rss <= in_xe_rss))
		    		    {
		    			    result = DUAL_MESSAGE_RSS_XS_XE_CAL_ERROR;
		    			    dual_driver_printf("Error: %s\n", print_error_message(result));
		    			    return result;
		    		    }
		    		    switch (i)
		    		    {
		    			    case 0:/* to revise for RSS_R2B */
		    				    ptr_out_param->RSS_R2B_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST = 0;
		    				    ptr_out_param->RSS_R2B_RSS_LUMA_HORI_SUB_OFST.Bits.RSS_LUMA_HORI_SUB_OFST = 0;
		    				    if (ptr_in_param->RSSO_R2A_RSSO_BASE_ADDR.Bits.RSSO_BASE_ADDR == ptr_in_param->RSSO_R2B_RSSO_BASE_ADDR.Bits.RSSO_BASE_ADDR)
		    				    {
                                    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN = 0;
                                }
                                else
                                {
                                    ptr_out_param->RSS_R2B_RSS_OUT_IMG.Bits.RSS_OUT_WD = DUAL_RSS_ALIGN(out_shift[1]);
                                }
		    				    break;
		    			    case 1:
		    				    ptr_out_param->RSS_R2A_RSS_LUMA_HORI_INT_OFST.Bits.RSS_LUMA_HORI_INT_OFST = 0;
		    				    ptr_out_param->RSS_R2A_RSS_LUMA_HORI_SUB_OFST.Bits.RSS_LUMA_HORI_SUB_OFST = 0;
		    				    if (ptr_in_param->RSSO_R2A_RSSO_BASE_ADDR.Bits.RSSO_BASE_ADDR == ptr_in_param->RSSO_R2B_RSSO_BASE_ADDR.Bits.RSSO_BASE_ADDR)
		    				    {
								    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN = 0;
								}
								else
								{
								    ptr_out_param->RSS_R2A_RSS_OUT_IMG.Bits.RSS_OUT_WD = DUAL_RSS_ALIGN(out_shift[1]);
							    }
		    				    break;
		    			    default:
		    				    break;
		    		    }
		    	    }
		    	}
		        if (out_xe_rss >= 0)
		        {
		    	    result = DUAL_MESSAGE_RSS_XS_XE_CAL_ERROR;
		    	    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	    return result;
		        }

                int bit_per_pixel = 8;
                int bus_cut_shift = 0;

		    	/* RSSO_R2A */
				if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN)
				{
				    ptr_out_param->RSSO_R2A_RSSO_OFST_ADDR.Bits.RSSO_OFST_ADDR = 0;
	    		    ptr_out_param->RSSO_R2A_RSSO_CROP.Bits.RSSO_XOFFSET = 0;
	                ptr_out_param->RSSO_R2A_RSSO_XSIZE.Bits.RSSO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->RSS_R2A_RSS_OUT_IMG.Bits.RSS_OUT_WD, bit_per_pixel, bus_cut_shift);
				}
		        if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN)
	            {
		    		/* RSSO_R2B */
					ptr_out_param->RSSO_R2B_RSSO_OFST_ADDR.Bits.RSSO_OFST_ADDR = 0;
	    		    ptr_out_param->RSSO_R2B_RSSO_CROP.Bits.RSSO_XOFFSET = 0;
		    		ptr_out_param->RSSO_R2B_RSSO_XSIZE.Bits.RSSO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->RSS_R2B_RSS_OUT_IMG.Bits.RSS_OUT_WD, bit_per_pixel, bus_cut_shift);

					/* clone registers */
                    ptr_out_param->RSS_R2B_RSS_HORI_STEP.Bits.RSS_HORI_STEP = ptr_out_param->RSS_R2A_RSS_HORI_STEP.Bits.RSS_HORI_STEP;
	                ptr_out_param->RSS_R2B_RSS_VERT_STEP.Bits.RSS_VERT_STEP = ptr_out_param->RSS_R2A_RSS_VERT_STEP.Bits.RSS_VERT_STEP;
	                ptr_out_param->RSS_R2B_RSS_IN_IMG.Bits.RSS_IN_HT = ptr_out_param->RSS_R2A_RSS_IN_IMG.Bits.RSS_IN_HT;
	                ptr_out_param->RSSO_R2B_RSSO_CON.Raw = ptr_in_param->RSSO_R2A_RSSO_CON.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_CON2.Raw = ptr_in_param->RSSO_R2A_RSSO_CON2.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_CON3.Raw = ptr_in_param->RSSO_R2A_RSSO_CON3.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_CON4.Raw = ptr_in_param->RSSO_R2A_RSSO_CON4.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_STRIDE.Raw = ptr_in_param->RSSO_R2A_RSSO_STRIDE.Raw;
	                ptr_out_param->RSSO_R2B_RSSO_CROP.Bits.RSSO_YOFFSET = ptr_in_param->RSSO_R2A_RSSO_CROP.Bits.RSSO_YOFFSET;
	                ptr_out_param->RSSO_R2B_RSSO_YSIZE.Bits.RSSO_YSIZE = ptr_in_param->RSSO_R2A_RSSO_YSIZE.Bits.RSSO_YSIZE;
				}

	            /* RTL */
	            if (ptr_in_param->RSSO_R2A_RSSO_BASE_ADDR.Bits.RSSO_BASE_ADDR == ptr_in_param->RSSO_R2B_RSSO_BASE_ADDR.Bits.RSSO_BASE_ADDR)
	            {
                    if (0 == ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN)
   	                {
		    			/* B only */
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R3_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN = 0;
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_RSSO_R2_EN = 0;
		    		}
		    		else
		    		{
                        /* A+B */
		    		    if (0 == ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN)
		    		    {
		    			    /* A only */
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R3_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_RSS_R2_EN = 0;
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_DMA2_EN.Bits.CAMCTL_RSSO_R2_EN = 0;
		    			    /* A only */
		    			    ptr_out_param->RSSO_R2A_RSSO_XSIZE.Bits.RSSO_XSIZE = ptr_in_param->RSSO_R2A_RSSO_XSIZE.Bits.RSSO_XSIZE;
		    			}
		    			else
		    			{
		    				/* A + B */
		    		        ptr_out_param->RSSO_R2B_RSSO_OFST_ADDR.Bits.RSSO_OFST_ADDR = (int)ptr_out_param->RSSO_R2A_RSSO_XSIZE.Bits.RSSO_XSIZE + 1;
		    			}
		    		}
		    	}
		    	else
		    	{
		    	    ptr_out_param->RSSO_R2B_RSSO_OFST_ADDR.Bits.RSSO_OFST_ADDR = 0;
		            /* restore stride */
		            ptr_out_param->RSSO_R2B_RSSO_STRIDE.Bits.RSSO_STRIDE = ptr_in_param->RSSO_R2B_RSSO_STRIDE.Bits.RSSO_STRIDE;
		            /* check valid size only WDMA with XOFF and YOFF support */
		            if ((int)ptr_out_param->RSSO_R2A_RSSO_XSIZE.Bits.RSSO_XSIZE + 1 > (int)ptr_in_param->RSSO_R2A_RSSO_STRIDE.Bits.RSSO_STRIDE)
		            {
		    	        ptr_out_param->RSSO_R2A_RSSO_XSIZE.Bits.RSSO_XSIZE = (((unsigned int)ptr_in_param->RSSO_R2A_RSSO_STRIDE.Bits.RSSO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		            }
		            if ((int)ptr_out_param->RSSO_R2B_RSSO_XSIZE.Bits.RSSO_XSIZE + 1 > (int)ptr_in_param->RSSO_R2B_RSSO_STRIDE.Bits.RSSO_STRIDE)
		            {
		    	        ptr_out_param->RSSO_R2B_RSSO_XSIZE.Bits.RSSO_XSIZE = (((unsigned int)ptr_in_param->RSSO_R2B_RSSO_STRIDE.Bits.RSSO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		            }
		    	}
            }

            if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZO_R2_EN)
            {
                if (ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD > DUAL_CRZ_R2_WD_ALL)
		        {
		            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		            dual_driver_printf("Error: %s\n", print_error_message(result));
		            return result;
		        }

                ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R2_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRSP_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRSP_R2_EN;
	            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R3_EN;

                /* Update CRZ_R1 in width */
                int crz_in_w = ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                        (((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		                          ((int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_E - (int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_S):
		                          ptr_in_param->SW.TWIN_RAWI_XSIZE) >>
		                         ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
                                  (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN +
		                          (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN)): ptr_in_param->SW.TWIN_RRZ_OUT_WD);
                int out_xe_crz;
		    	int out_shift[3] = { 1, 1, 1 };

		        /* check CRZ_R2 alignment */
		    	//if (DUAL_OUT_FMT_YUV422_8 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_CRZO_R2_FMT)
		    	//{
		    	//	out_shift[0] = 1;  /* 2 << 3, x16 */
		    	//}
		    	//else
		    	//{
		    	//	result = DUAL_MESSAGE_CRZO_CONFIG_ERROR;
		    	//	dual_driver_printf("Error: %s\n", print_error_message(result));
		    	//	return result;
		    	//}

		        /* setup CRZ_R2 IN_HT */
		        ptr_out_param->CRZ_R2A_CRZ_IN_IMG.Bits.CRZ_IN_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		            ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		            ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		        /* Check horizontal oefficient step */
		        if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		        {
                    long long crop_in_size = (long long)crz_in_w - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST_LAST;

		            // 2-tap algorithm
                    ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(crop_in_size - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                        (((unsigned int)ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD  - 1) >> 1)) / (ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD - 1);

		    		/* Scaling down error check */
		            if (ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		            {
		    	        result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	        dual_driver_printf("Error: %s\n", print_error_message(result));
		    	        return result;
		            }
		    	}
		    	else
		    	{
		    	    long long crop_in_size = (long long)crz_in_w - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST_LAST;

		    	    // Source accumulation
		    	    ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC + crop_in_size - 2) / (crop_in_size - 1);

		            /* Scaling down error check */
		            if (ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		            {
		    	        result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	        dual_driver_printf("Error: %s\n", print_error_message(result));
		    	        return result;
		            }
		    	}

                if (0 == ptr_in_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP)
                {
		            /* Check vertical coefficient step */
		            if (ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT <= 1)
	                {
		                result = DUAL_MESSAGE_INVALID_CRZ_IN_CROP_HT_ERROR;
		                dual_driver_printf("Error: %s\n", print_error_message(result));
		                return result;
	                }
	                else
	                {
		                /* Recalculate CRZ_R2 CRZ_VERT_STEP */
		                if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_VERT_ALGO)
		                {
		                    // 2-tap algorithm
                            ptr_out_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                                (((unsigned int)ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT  - 1) >> 1)) / ((unsigned int)ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1);
		                   	if (ptr_out_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                    {
		    	                result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                return result;
		                    }
		                }
		                else
		                {
		    	            // Source accumulation
		    	            ptr_out_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)((unsigned int)ptr_in_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC +
		    	                ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT - 2) / (ptr_in_param->SW.TWIN_CRZ_R2_IN_CROP_HT - 1);
		    	            if (ptr_out_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                    {
		    	                result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                return result;
		                    }
		                }
		            }
	            }

		    	/* loop dual mode from right */
		    	for (int i = 0; i < 2; i++)
		    	{
		    	    int in_xs_mrg = 0;
		    	    int in_xs_crz = 0;
		    	    int in_xe_crz = 0;
		    	    int out_xs_crz = 0;

		    	    /* config forward backward pos */
		    	    switch(i)
		    	    {
		    		    case 0:
		    			    /* CRZ_R2B */
		    			    /* init pos */
		    			    in_xs_mrg  = mrg_r13_valid[0];
		    			    /* Right boundary */
		    			    in_xs_crz  = mrg_r13b_xstart + DUAL_TILE_LOSS_CRZ_R2_L;
		    			    in_xe_crz  = mrg_r13_valid[0] + mrg_r13_valid[1] - 1;
		    			    out_xe_crz = ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD - 1;
		    			    break;
		    		    case 1:
		    			    /* CRZ_R2A */
		    			    /* init pos */
		    			    in_xs_mrg  = 0;
		    			    in_xe_crz  = (mrg_r13_valid[0] + mrg_r13_valid[1] - 1) - DUAL_TILE_LOSS_CRZ_R2;
		    			    out_xs_crz = 0;
		    			    break;
		    		    default:
    	    				break;
		    	    }

		    	    /* backward & forward calculation */
		    	    if (out_xs_crz < out_xe_crz)
		    	    {
		    	        if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		    	        {
		    	            // 2-tap
		    		        /* backward out_xe_crz */
		    		        long long end_temp = (long long)out_xe_crz * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
		    			        (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP +
		    			        (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS);
		    		        /* cal pos */
		    		        if ((end_temp + (1 + (long long)DUAL_CRZ_TILE_LOSS_2_TAP_UV) * DUAL_CRZ_PREC_VAL_2_TAP) < (long long)in_xe_crz * DUAL_CRZ_PREC_VAL_2_TAP)
		    		        {
		    			        int n = (int)((unsigned long long)(end_temp + (1 + (long long)DUAL_CRZ_TILE_LOSS_2_TAP_UV) * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_PREC_BITS_2_TAP);
		    			        if (n & 0x1)
		    			        {
		    				        in_xe_crz = n ;
		    			        }
		    			        else/* must be even */
		    			        {
		    				        in_xe_crz = n + 1;
		    			        }
		    		        }
		    		        else
		    		        {
		    			        if (in_xe_crz + 1 < ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD)
		    			        {
		    				        result = DUAL_MESSAGE_CRZ_XS_XE_CAL_ERROR;
		    				        dual_driver_printf("Error: %s\n", print_error_message(result));
		    				        return result;
		    			        }
		    		        }
		    		        /* Forward in_xs_crz */
		    		        if (in_xs_crz < in_xe_crz)
		    		        {
		    			        if (in_xs_crz <= 0)
		    			        {
		    				        out_xs_crz = 0;
		    			        }
		    			        else
		    			        {
		    				        long long start_temp = (long long)in_xs_crz * DUAL_CRZ_PREC_VAL_2_TAP -
		    					        (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP -
		    					        (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS);
		    				        int n = (int)(start_temp / ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP);
		    				        if (((long long)n * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) < start_temp)
		    				        {
		    					        n = n + 1;
		    				        }
		    				        if (n < 0)
		    				        {
		    					        n = 0;
		    				        }
		    				        /* update align by pixel mode */
		    				        /* CRZ_R2A, run i= 0 only */
		    				        if (n & DUAL_CRZ_MASK(out_shift[i]))
		    				        {
		    					        out_xs_crz = n + DUAL_CRZ_ALIGN(out_shift[i]) - (n & DUAL_CRZ_MASK(out_shift[i]));
		    				        }
		    				        else
		    				        {
		    					        out_xs_crz = n;
		    				        }
		    			        }
		    		        }
		    		    }
		    		    else
		    		    {
		    		        // source accumulation
		    		        /* backward out_xe_crz */
		    		        long long end_temp = (long long)out_xe_crz * DUAL_CRZ_PREC_VAL_SRC_ACC +
		    			        (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
		    			        (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS);
		    		        /* cal pos */
		    		        if ((end_temp * 2 + ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP + DUAL_CRZ_PREC_VAL_SRC_ACC) <
		    		            ((long long)2 * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP * in_xe_crz))
		    		        {
		    			        int n = (int)((end_temp * 2 + ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP + DUAL_CRZ_PREC_VAL_SRC_ACC) /
		    			            (2 * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP));
                                if (((long long)n * 2 * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) ==
                                    (end_temp * 2 + (long long)ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP + DUAL_CRZ_PREC_VAL_SRC_ACC))
                                {
                                    n = n - 1;
                                }
                                if (n & 0x1)
                                {
                                    in_xe_crz = n;
                                }
                                else
                                {
                                    in_xe_crz = n + 1;
                                }
		    		        }
		    		        else
		    		        {
		    			        if (in_xe_crz + 1 < ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD)
		    			        {
		    				        result = DUAL_MESSAGE_CRZ_XS_XE_CAL_ERROR;
		    				        dual_driver_printf("Error: %s\n", print_error_message(result));
		    				        return result;
		    			        }
		    		        }
		    		        /* Forward in_xs_crz */
		    		        if (in_xs_crz < in_xe_crz)
		    		        {
		    			        if (in_xs_crz <= 0)
		    			        {
		    				        out_xs_crz = 0;
		    			        }
		    			        else
		    			        {

		    			            long long start_temp = (long long)in_xs_crz * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP -
                                        (((unsigned int)ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> 1) -
                                        (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP -
                                        (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS);
                                    int n = (int)((unsigned long long)(start_temp * 2 + DUAL_CRZ_PREC_VAL_SRC_ACC) >> (DUAL_CRZ_PREC_BITS_SRC_ACC + 1));
                                    if (((long long)n * 2 * DUAL_CRZ_PREC_VAL_SRC_ACC) < (start_temp * 2 + DUAL_CRZ_PREC_VAL_SRC_ACC)) /* ceiling be careful with value smaller than zero */
                                    {
                                        n = n + 1;
                                    }
                                    if (n < 0)
                                    {
                                        n = 0;
                                    }
                                    if (n & DUAL_CRZ_MASK(out_shift[i]))
		    				        {
		    					        out_xs_crz = n + DUAL_CRZ_ALIGN(out_shift[i]) - (n & DUAL_CRZ_MASK(out_shift[i]));
		    				        }
		    				        else
		    				        {
		    					        out_xs_crz = n;
		    				        }
		    			        }
		    		        }
		    		    }
		    	    }

		    	    /* update CRZ width, offset, flag */
		    	    if ((out_xs_crz < out_xe_crz) && (in_xs_crz < in_xe_crz))
		    	    {
		    		    if (0 == ptr_in_param->CRZ_R2A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		    		    {
		    		        long long temp_offset = (long long)out_xs_crz * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP  +
                                (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP +
                                (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS) -
                                (long long)in_xs_mrg * DUAL_CRZ_PREC_VAL_2_TAP;

		    		        switch (i)
		    		        {
		    			        case 0:/* to revise for CRZ_R2B */
		    				        /* CRZ_R2B */
		    				        ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = out_xe_crz - out_xs_crz + 1;
		    				        ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp_offset >> DUAL_CRZ_PREC_BITS_2_TAP);
		    				        ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp_offset -
		    					        (long long)ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP);
                                    ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST >> 1;
                                    if (0 == (ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST & 0x1))
                                    {
                                        ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                                    }
                                    else
                                    {
                                        ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST + DUAL_CRZ_PREC_VAL_2_TAP;
                                    }
		    				        break;
		    			        case 1:
		    				        /* CRZ_R2A */
		    				        ptr_out_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = out_xe_crz - out_xs_crz + 1;
		    				        ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp_offset >> DUAL_CRZ_PREC_BITS_2_TAP);
		    				        ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp_offset -
		    					        (long long)ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP);
                                    ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST >> 1;
                                    if (0 == (ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST & 0x1))
                                    {
                                        ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                                    }
                                    else
                                    {
                                        ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST + DUAL_CRZ_PREC_VAL_2_TAP;
                                    }

                                    mrg_r13a_xend = ((in_xe_crz + DUAL_TILE_LOSS_CRZ_R2) > mrg_r13a_xend)? (in_xe_crz + DUAL_TILE_LOSS_CRZ_R2): mrg_r13a_xend;
									break;
		    			        default:
		    				        break;
		    		        }
		    		    }
		    		    else
		    		    {
		    		        long long temp_offset = (long long)out_xs_crz * DUAL_CRZ_PREC_VAL_SRC_ACC +
                                (long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_INT_OFST * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
                                (((long long)ptr_in_param->SW.TWIN_CRZ_R2_HORI_SUB_OFST * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS) -
                                (long long)in_xs_mrg * ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP;

                            switch (i)
		    		        {
		    			        case 0:/* to revise for CRZ_R2B */
		    				        /* CRZ_R2B */
		    				        ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = out_xe_crz - out_xs_crz + 1;
		    				        ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp_offset >> DUAL_CRZ_PREC_BITS_SRC_ACC);
		    				        ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp_offset -
    	    						    (long long)ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_SRC_ACC);
                                    ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST;
                                    ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
		    				        break;
		    			        case 1:
		    				        /* CRZ_R2A */
		    				        ptr_out_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = out_xe_crz - out_xs_crz + 1;
		    				        ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp_offset >> DUAL_CRZ_PREC_BITS_SRC_ACC);
		    				        ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp_offset -
		    					        (long long)ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_SRC_ACC);
                                    ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST;
                                    ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;

                                    mrg_r13a_xend = ((in_xe_crz + DUAL_TILE_LOSS_CRZ_R2) > mrg_r13a_xend)? (in_xe_crz + DUAL_TILE_LOSS_CRZ_R2): mrg_r13a_xend;
									break;
                                default:
                                    break;
		    			    }
		    		    }

		    		    /* Decrease out_xe to next start pos */
		    		    out_xe_crz = out_xs_crz - 1;
		    	    }
		    	    else
		    	    {
		    		    if ((out_xe_crz > 0) && (out_xe_crz + 1 < ptr_in_param->SW.TWIN_CRZ_R2_OUT_WD) && (in_xs_crz <= in_xe_crz))
		    		    {
		    			    result = DUAL_MESSAGE_CRZ_XS_XE_CAL_ERROR;
		    			    dual_driver_printf("Error: %s\n", print_error_message(result));
		    			    return result;
		    		    }
		    		    switch (i)
		    		    {
		    			    case 0:/* to revise for CRZ_R2B */
		    				    ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = 0;
		    				    ptr_out_param->CRZ_R2B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = 0;
		    				    ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = 0;
		    				    ptr_out_param->CRZ_R2B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = 0;

		    				    /* check CRZ buffer size, must x8 align */
		    				    if (ptr_in_param->CRZO_R2A_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR == ptr_in_param->CRZO_R2B_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR)
		    				    {
		    				        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN = 0;
		    				    }
		    				    else
		    				    {
		    				        ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = DUAL_CRZ_ALIGN(out_shift[1]);
		    				    }
		    				    break;
		    			    case 1:
		    				    ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = 0;
		    				    ptr_out_param->CRZ_R2A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = 0;
		    				    ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = 0;
		    				    ptr_out_param->CRZ_R2A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = 0;

		    				    /* check CRZ buffer size, must x8 align */
		    				    if (ptr_in_param->CRZO_R2A_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR == ptr_in_param->CRZO_R2B_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR)
		    				    {
		    					    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN = 0;
		    				    }
		    				    else
		    				    {
		    				        ptr_out_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = DUAL_CRZ_ALIGN(out_shift[1]);
		    				    }
		    				    break;
		    			    default:
		    				    break;
		    		    }
		    	    }
		    	}
		        if (out_xe_crz >= 0)
		        {
		    	    result = DUAL_MESSAGE_CRZ_XS_XE_CAL_ERROR;
		    	    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	    return result;
		        }

                int bit_per_pixel = 8, bit_per_pixel_uv = 8;
                int bus_cut_shift = 1, crzo_plane_num = 1;

                switch (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_CRZO_R2_FMT)
 	            {
 	                case DUAL_OUT_FMT_YUV422:
	        		    bit_per_pixel    = 16;
	        		    bus_cut_shift    = 1;
	        		    crzo_plane_num   = 1;
	        			break;
		    		case DUAL_OUT_FMT_YUV422_2:
		    			bit_per_pixel    = 8;
		    			bit_per_pixel_uv = 8;
		    			bus_cut_shift    = 1;
		    			crzo_plane_num   = 2;
		    			break;
		    		case DUAL_OUT_FMT_YUV420_2:
		    			bit_per_pixel    = 8;
		    			bit_per_pixel_uv = 8;
		    			bus_cut_shift    = 1;
		    			crzo_plane_num   = 2;
                        break;
            		default:
            		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	        	        dual_driver_printf("Error: %s\n", print_error_message(result));
	        	        return result;
   	            }

                /* CRSP_R2A */
                ptr_out_param->CRSP_R2A_CRSP_OUT_IMG.Bits.CRSP_WD = ptr_out_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD;
                ptr_out_param->CRSP_R2A_CRSP_OUT_IMG.Bits.CRSP_HT = ptr_out_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT;
                ptr_out_param->CRSP_R2A_CRSP_CROP_X.Bits.CRSP_CROP_XSTART = 0;
                ptr_out_param->CRSP_R2A_CRSP_CROP_X.Bits.CRSP_CROP_XEND = (int)ptr_out_param->CRSP_R2A_CRSP_OUT_IMG.Bits.CRSP_WD - 1;
                ptr_out_param->CRSP_R2A_CRSP_CROP_Y.Bits.CRSP_CROP_YSTART = 0;
                ptr_out_param->CRSP_R2A_CRSP_CROP_Y.Bits.CRSP_CROP_YEND = (int)ptr_out_param->CRSP_R2A_CRSP_OUT_IMG.Bits.CRSP_HT - 1;

                /* CRSP_R2B */
                ptr_out_param->CRSP_R2B_CRSP_OUT_IMG.Bits.CRSP_WD = ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD;
                ptr_out_param->CRSP_R2B_CRSP_OUT_IMG.Bits.CRSP_HT = ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_HT;
                ptr_out_param->CRSP_R2B_CRSP_CROP_X.Bits.CRSP_CROP_XSTART = 0;
                ptr_out_param->CRSP_R2B_CRSP_CROP_X.Bits.CRSP_CROP_XEND = (int)ptr_out_param->CRSP_R2B_CRSP_OUT_IMG.Bits.CRSP_WD - 1;
                ptr_out_param->CRSP_R2B_CRSP_CROP_Y.Bits.CRSP_CROP_YSTART = 0;
                ptr_out_param->CRSP_R2B_CRSP_CROP_Y.Bits.CRSP_CROP_YEND = (int)ptr_out_param->CRSP_R2B_CRSP_OUT_IMG.Bits.CRSP_HT - 1;

		    	/* CRZO_R2A */
		    	if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN)
		    	{
				    ptr_out_param->CRZO_R2A_CRZO_OFST_ADDR.Bits.CRZO_OFST_ADDR = 0;
	    		    ptr_out_param->CRZO_R2A_CRZO_CROP.Bits.CRZO_XOFFSET = 0;
	                ptr_out_param->CRZO_R2A_CRZO_XSIZE.Bits.CRZO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD, bit_per_pixel, bus_cut_shift);
	                if (crzo_plane_num >= 2)
                    {
                        ptr_out_param->CRZBO_R2A_CRZBO_OFST_ADDR.Bits.CRZBO_OFST_ADDR = 0;
	    		        ptr_out_param->CRZBO_R2A_CRZBO_CROP.Bits.CRZBO_XOFFSET = 0;
                        ptr_out_param->CRZBO_R2A_CRZBO_XSIZE.Bits.CRZBO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->CRZ_R2A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD, bit_per_pixel_uv, bus_cut_shift);
	                }
	            }

		    	if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN)
		    	{
		    		/* setup CRZO_R2B */
		    		ptr_out_param->CRZO_R2B_CRZO_OFST_ADDR.Bits.CRZO_OFST_ADDR = 0;
	    		    ptr_out_param->CRZO_R2B_CRZO_CROP.Bits.CRZO_XOFFSET = 0;
		    		ptr_out_param->CRZO_R2B_CRZO_XSIZE.Bits.CRZO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD, bit_per_pixel, bus_cut_shift);

                    /* Clone registers */
                    ptr_out_param->CRZ_R2B_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ptr_out_param->CRZ_R2A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP;
	                ptr_out_param->CRZ_R2B_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ptr_out_param->CRZ_R2A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP;
	                ptr_out_param->CRZ_R2B_CRZ_IN_IMG.Bits.CRZ_IN_HT = ptr_out_param->CRZ_R2A_CRZ_IN_IMG.Bits.CRZ_IN_HT;
	                ptr_out_param->CRZO_R2B_CRZO_CON.Raw = ptr_in_param->CRZO_R2A_CRZO_CON.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_CON2.Raw = ptr_in_param->CRZO_R2A_CRZO_CON2.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_CON3.Raw = ptr_in_param->CRZO_R2A_CRZO_CON3.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_CON4.Raw = ptr_in_param->CRZO_R2A_CRZO_CON4.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_STRIDE.Raw = ptr_in_param->CRZO_R2A_CRZO_STRIDE.Raw;
	                ptr_out_param->CRZO_R2B_CRZO_CROP.Bits.CRZO_YOFFSET = ptr_in_param->CRZO_R2A_CRZO_CROP.Bits.CRZO_YOFFSET;
	                ptr_out_param->CRZO_R2B_CRZO_YSIZE.Bits.CRZO_YSIZE = ptr_in_param->CRZO_R2A_CRZO_YSIZE.Bits.CRZO_YSIZE;
                    if (crzo_plane_num >= 2)
                    {
                        ptr_out_param->CRZBO_R2B_CRZBO_OFST_ADDR.Bits.CRZBO_OFST_ADDR = 0;
	    		        ptr_out_param->CRZBO_R2B_CRZBO_CROP.Bits.CRZBO_XOFFSET = 0;
                        ptr_out_param->CRZBO_R2B_CRZBO_XSIZE.Bits.CRZBO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->CRZ_R2B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD, bit_per_pixel_uv, bus_cut_shift);

                        ptr_out_param->CRZBO_R2B_CRZBO_CON.Raw = ptr_in_param->CRZBO_R2A_CRZBO_CON.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_CON2.Raw = ptr_in_param->CRZBO_R2A_CRZBO_CON2.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_CON3.Raw = ptr_in_param->CRZBO_R2A_CRZBO_CON3.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_CON4.Raw = ptr_in_param->CRZBO_R2A_CRZBO_CON4.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_STRIDE.Raw = ptr_in_param->CRZBO_R2A_CRZBO_STRIDE.Raw;
	                    ptr_out_param->CRZBO_R2B_CRZBO_CROP.Bits.CRZBO_YOFFSET = ptr_in_param->CRZBO_R2A_CRZBO_CROP.Bits.CRZBO_YOFFSET;
	                    ptr_out_param->CRZBO_R2B_CRZBO_YSIZE.Bits.CRZBO_YSIZE = ptr_in_param->CRZBO_R2A_CRZBO_YSIZE.Bits.CRZBO_YSIZE;
                    }
                }

	            /* RTL */
	            if (ptr_in_param->CRZO_R2A_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR == ptr_in_param->CRZO_R2B_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR)
	            {
		    		/* A+B */
		    		if (0 == ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN)
		    	    {
		    			/* B only */
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R2_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRSP_R2_EN = 0;
	                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R3_EN = 0;
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZO_R2_EN = 0;
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZBO_R2_EN = 0;
		    		}
		    		else
		    		{
                        /* A+B */
		    		    if (0 == ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN)
		    		    {
		    				/* A only */
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R2_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R2_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRSP_R2_EN = 0;
	                        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R3_EN = 0;
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZO_R2_EN = 0;
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZBO_R2_EN = 0;
		    		        /* A only */
		    		        ptr_out_param->CRZO_R2A_CRZO_XSIZE.Bits.CRZO_XSIZE = ptr_in_param->CRZO_R2A_CRZO_XSIZE.Bits.CRZO_XSIZE;
		    		        if (crzo_plane_num >= 2)
                            {
                                ptr_out_param->CRZBO_R2A_CRZBO_XSIZE.Bits.CRZBO_XSIZE = ptr_in_param->CRZBO_R2A_CRZBO_XSIZE.Bits.CRZBO_XSIZE;
                            }
		    			}
		    			else
		    			{
		    				/* A + B */
		    		        ptr_out_param->CRZO_R2B_CRZO_OFST_ADDR.Bits.CRZO_OFST_ADDR = (int)ptr_out_param->CRZO_R2A_CRZO_XSIZE.Bits.CRZO_XSIZE + 1;
		    			    if (crzo_plane_num >= 2)
                            {
		    		            ptr_out_param->CRZBO_R2B_CRZBO_OFST_ADDR.Bits.CRZBO_OFST_ADDR = (int)ptr_out_param->CRZBO_R2A_CRZBO_XSIZE.Bits.CRZBO_XSIZE + 1;
		    			    }
		    			}
		    		}
		    	}
		    	else
		    	{
		    	    ptr_out_param->CRZO_R2B_CRZO_OFST_ADDR.Bits.CRZO_OFST_ADDR = 0;
		            /* restore stride */
		            ptr_out_param->CRZO_R2B_CRZO_STRIDE.Bits.CRZO_STRIDE = ptr_in_param->CRZO_R2B_CRZO_STRIDE.Bits.CRZO_STRIDE;
		            /* check valid size only WDMA with XOFF and YOFF support */
		            if ((int)ptr_out_param->CRZO_R2A_CRZO_XSIZE.Bits.CRZO_XSIZE + 1 > (int)ptr_in_param->CRZO_R2A_CRZO_STRIDE.Bits.CRZO_STRIDE)
		            {
		    	        ptr_out_param->CRZO_R2A_CRZO_XSIZE.Bits.CRZO_XSIZE = (((unsigned int)ptr_in_param->CRZO_R2A_CRZO_STRIDE.Bits.CRZO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		            }
		            if ((int)ptr_out_param->CRZO_R2B_CRZO_XSIZE.Bits.CRZO_XSIZE + 1 > (int)ptr_in_param->CRZO_R2B_CRZO_STRIDE.Bits.CRZO_STRIDE)
		            {
		    	        ptr_out_param->CRZO_R2B_CRZO_XSIZE.Bits.CRZO_XSIZE = (((unsigned int)ptr_in_param->CRZO_R2B_CRZO_STRIDE.Bits.CRZO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		            }

		            if (crzo_plane_num >= 2)
                    {
                        ptr_out_param->CRZBO_R2B_CRZBO_OFST_ADDR.Bits.CRZBO_OFST_ADDR = 0;
		                /* restore stride */
		                ptr_out_param->CRZBO_R2B_CRZBO_STRIDE.Bits.CRZBO_STRIDE = ptr_in_param->CRZBO_R2B_CRZBO_STRIDE.Bits.CRZBO_STRIDE;
		                /* check valid size only WDMA with XOFF and YOFF support */
		                if ((int)ptr_out_param->CRZBO_R2A_CRZBO_XSIZE.Bits.CRZBO_XSIZE + 1 > (int)ptr_in_param->CRZBO_R2A_CRZBO_STRIDE.Bits.CRZBO_STRIDE)
		                {
		    	            ptr_out_param->CRZBO_R2A_CRZBO_XSIZE.Bits.CRZBO_XSIZE = (((unsigned int)ptr_in_param->CRZBO_R2A_CRZBO_STRIDE.Bits.CRZBO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		                }
		                if ((int)ptr_out_param->CRZBO_R2B_CRZBO_XSIZE.Bits.CRZBO_XSIZE + 1 > (int)ptr_in_param->CRZBO_R2B_CRZBO_STRIDE.Bits.CRZBO_STRIDE)
		                {
		    	            ptr_out_param->CRZBO_R2B_CRZBO_XSIZE.Bits.CRZBO_XSIZE = (((unsigned int)ptr_in_param->CRZBO_R2B_CRZBO_STRIDE.Bits.CRZBO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		                }
                    }
		    	}
            }

            if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_CRZO_R1_EN)
            {
                if (ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD > DUAL_CRZ_R1_WD_ALL)
		        {
		            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		            dual_driver_printf("Error: %s\n", print_error_message(result));
		            return result;
		        }

                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_GGM_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_GGM_R2_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_G2C_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_G2C_R2_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_C42_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_C42_R2_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R1_EN;
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN;
   	            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R2_EN;

                /* Update CRZ_R1 in width */
                int crz_in_w = ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		                        (((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		                          ((int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_E - (int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_S):
		                          ptr_in_param->SW.TWIN_RAWI_XSIZE) >>
		                         ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
                                  (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN +
		                          (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN)): ptr_in_param->SW.TWIN_RRZ_OUT_WD);
                int out_xe_crz;
		    	int out_shift[3] = { 1, 1, 1 };

		        /* check CRZ_R1 alignment */
		    	//if (DUAL_OUT_FMT_YUV422_8 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_CRZO_R1_FMT)
		    	//{
		    	//	out_shift[0] = 1;  /* 2 << 3, x16 */
		    	//}
		    	//else
		    	//{
		    	//	result = DUAL_MESSAGE_CRZO_CONFIG_ERROR;
		    	//	dual_driver_printf("Error: %s\n", print_error_message(result));
		    	//	return result;
		    	//}

		        /* setup CRZ_R1 IN_HT */
		        ptr_out_param->CRZ_R1A_CRZ_IN_IMG.Bits.CRZ_IN_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		            ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		            ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		        /* Check horizontal oefficient step */
		        if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		        {
                    long long crop_in_size = (long long)crz_in_w - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST_LAST;

		            // 2-tap algorithm
                    ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(crop_in_size - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                        (((unsigned int)ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD  - 1) >> 1)) / ((long long)ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD - 1);

		    		/* Scaling down error check */
		            if (ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		            {
		    	        result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	        dual_driver_printf("Error: %s\n", print_error_message(result));
		    	        return result;
		            }
		    	}
		    	else
		    	{
		    	    long long crop_in_size = (long long)crz_in_w - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST - (ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST? 1: 0) - ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST_LAST;

		    	    // Source accumulation
		    	    ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC + crop_in_size - 2) / (crop_in_size - 1);

		            /* Scaling down error check */
		            if (ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		            {
		    	        result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	        dual_driver_printf("Error: %s\n", print_error_message(result));
		    	        return result;
		            }
		    	}

                if (0 == ptr_in_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP)
                {
		            /* Check vertical coefficient step */
		            if (ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT <= 1)
	                {
		                result = DUAL_MESSAGE_INVALID_CRZ_IN_CROP_HT_ERROR;
		                dual_driver_printf("Error: %s\n", print_error_message(result));
		                return result;
	                }
	                else
	                {
		                /* Recalculate CRZ_R1 CRZ_VERT_STEP */
		                if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_VERT_ALGO)
		                {
		                    // 2-tap algorithm
                            ptr_out_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)(ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT - 1) * DUAL_CRZ_PREC_VAL_2_TAP +
                                (((unsigned int)ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT  - 1) >> 1)) / ((unsigned int)ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1);
		            	    if (ptr_out_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP < DUAL_CRZ_PREC_VAL_2_TAP)
		                    {
		    	                result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                return result;
		                    }
		                }
		                else
		                {
		    	            // Source accumulation
		    	            ptr_out_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP = ((long long)((unsigned int)ptr_in_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_HT - 1) * DUAL_CRZ_PREC_VAL_SRC_ACC +
		    	                ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT - 2) / (ptr_in_param->SW.TWIN_CRZ_R1_IN_CROP_HT - 1);
		    	            if (ptr_out_param->CRZ_R1A_CRZ_VERT_STEP.Bits.CRZ_VERT_STEP > DUAL_CRZ_PREC_VAL_SRC_ACC)
		                    {
		    	                result = DUAL_MESSAGE_CRZ_SCALING_UP_ERROR;
		    	                dual_driver_printf("Error: %s\n", print_error_message(result));
		    	                return result;
		                    }
		                }
		            }
	            }

		    	/* loop dual mode from right */
		    	for (int i = 0; i < 2; i++)
		    	{
		    	    int in_xs_mrg = 0;
		    	    int in_xs_crz = 0;
		    	    int in_xe_crz = 0;
		    	    int out_xs_crz = 0;

		    	    /* config forward backward pos */
		    	    switch(i)
		    	    {
		    		    case 0:
		    			    /* CRZ_R1B */
		    			    /* init pos */
		    			    in_xs_mrg  = mrg_r13_valid[0];
		    			    /* Right boundary */
		    			    in_xs_crz  = mrg_r13b_xstart + DUAL_TILE_LOSS_CRZ_R1_L;
		    			    in_xe_crz  = mrg_r13_valid[0] + mrg_r13_valid[1] - 1;
		    			    out_xe_crz = ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD - 1;
		    			    break;
		    		    case 1:
		    			    /* CRZ_R1A */
		    			    /* init pos */
		    			    in_xs_mrg  = 0;
		    			    in_xe_crz  = (mrg_r13_valid[0] + mrg_r13_valid[1] - 1) - DUAL_TILE_LOSS_CRZ_R1;
		    			    out_xs_crz = 0;
		    			    break;
		    		    default:
    	    				break;
		    	    }

		    	    /* backward & forward calculation */
		    	    if (out_xs_crz < out_xe_crz)
		    	    {
		    	        if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		    	        {
		    	            // 2-tap
		    		        /* backward out_xe_crz */
		    		        long long end_temp = (long long)out_xe_crz * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
		    			        (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP +
		    			        (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS);
		    		        /* cal pos */
		    		        if ((end_temp + (1 + (long long)DUAL_CRZ_TILE_LOSS_2_TAP_UV) * DUAL_CRZ_PREC_VAL_2_TAP) < (long long)in_xe_crz * DUAL_CRZ_PREC_VAL_2_TAP)
		    		        {
		    			        int n = (int)((unsigned long long)(end_temp + (1 + (long long)DUAL_CRZ_TILE_LOSS_2_TAP_UV) * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_PREC_BITS_2_TAP);
		    			        if (n & 0x1)
		    			        {
		    				        in_xe_crz = n ;
		    			        }
		    			        else/* must be even */
		    			        {
		    				        in_xe_crz = n + 1;
		    			        }
		    		        }
		    		        else
		    		        {
		    			        if (in_xe_crz + 1 < ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD)
		    			        {
		    				        result = DUAL_MESSAGE_CRZ_XS_XE_CAL_ERROR;
		    				        dual_driver_printf("Error: %s\n", print_error_message(result));
		    				        return result;
		    			        }
		    		        }
		    		        /* Forward in_xs_crz */
		    		        if (in_xs_crz < in_xe_crz)
		    		        {
		    			        if (in_xs_crz <= 0)
		    			        {
		    				        out_xs_crz = 0;
		    			        }
		    			        else
		    			        {
		    				        long long start_temp = (long long)in_xs_crz * DUAL_CRZ_PREC_VAL_2_TAP -
		    					        (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP -
		    					        (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS);
		    				        int n = (int)(start_temp / ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP);
		    				        if (((long long)n * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) < start_temp)
		    				        {
		    					        n = n + 1;
		    				        }
		    				        if (n < 0)
		    				        {
		    					        n = 0;
		    				        }
		    				        /* update align by pixel mode */
		    				        /* CRZ_R1A, run i= 0 only */
		    				        if (n & DUAL_CRZ_MASK(out_shift[i]))
		    				        {
		    					        out_xs_crz = n + DUAL_CRZ_ALIGN(out_shift[i]) - (n & DUAL_CRZ_MASK(out_shift[i]));
		    				        }
		    				        else
		    				        {
		    					        out_xs_crz = n;
		    				        }
		    			        }
		    		        }
		    		    }
		    		    else
		    		    {
		    		        // source accumulation
		    		        /* backward out_xe_crz */
		    		        long long end_temp = (long long)out_xe_crz * DUAL_CRZ_PREC_VAL_SRC_ACC +
		    			        (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
		    			        (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS);
		    		        /* cal pos */
		    		        if ((end_temp * 2 + ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP + DUAL_CRZ_PREC_VAL_SRC_ACC) <
		    		            ((long long)2 * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP * in_xe_crz))
		    		        {
		    			        int n = (int)((end_temp * 2 + ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP + DUAL_CRZ_PREC_VAL_SRC_ACC) /
		    			            (2 * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP));
                                if (((long long)n * 2 * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) ==
                                    (end_temp * 2 + (long long)ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP + DUAL_CRZ_PREC_VAL_SRC_ACC))
                                {
                                    n = n - 1;
                                }
                                if (n & 0x1)
                                {
                                    in_xe_crz = n;
                                }
                                else
                                {
                                    in_xe_crz = n + 1;
                                }
		    		        }
		    		        else
		    		        {
		    			        if (in_xe_crz + 1 < ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD)
		    			        {
		    				        result = DUAL_MESSAGE_CRZ_XS_XE_CAL_ERROR;
		    				        dual_driver_printf("Error: %s\n", print_error_message(result));
		    				        return result;
		    			        }
		    		        }
		    		        /* Forward in_xs_crz */
		    		        if (in_xs_crz < in_xe_crz)
		    		        {
		    			        if (in_xs_crz <= 0)
		    			        {
		    				        out_xs_crz = 0;
		    			        }
		    			        else
		    			        {

		    			            long long start_temp = (long long)in_xs_crz * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP -
                                        (((unsigned int)ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> 1) -
                                        (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP -
                                        (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS);
                                    int n = (int)((unsigned long long)(start_temp * 2 + DUAL_CRZ_PREC_VAL_SRC_ACC) >> (DUAL_CRZ_PREC_BITS_SRC_ACC + 1));
                                    if (((long long)n * 2 * DUAL_CRZ_PREC_VAL_SRC_ACC) < (start_temp * 2 + DUAL_CRZ_PREC_VAL_SRC_ACC)) /* ceiling be careful with value smaller than zero */
                                    {
                                        n = n + 1;
                                    }
                                    if (n < 0)
                                    {
                                        n = 0;
                                    }
                                    if (n & DUAL_CRZ_MASK(out_shift[i]))
		    				        {
		    					        out_xs_crz = n + DUAL_CRZ_ALIGN(out_shift[i]) - (n & DUAL_CRZ_MASK(out_shift[i]));
		    				        }
		    				        else
		    				        {
		    					        out_xs_crz = n;
		    				        }
		    			        }
		    		        }
		    		    }
		    	    }

		    	    /* update CRZ width, offset, flag */
		    	    if ((out_xs_crz < out_xe_crz) && (in_xs_crz < in_xe_crz))
		    	    {
		    		    if (0 == ptr_in_param->CRZ_R1A_CRZ_CONTROL.Bits.CRZ_HORI_ALGO)
		    		    {
		    		        long long temp_offset = (long long)out_xs_crz * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP  +
                                (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP +
                                (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * DUAL_CRZ_PREC_VAL_2_TAP) >> DUAL_CRZ_CONFIG_BITS) -
                                (long long)in_xs_mrg * DUAL_CRZ_PREC_VAL_2_TAP;

		    		        switch (i)
		    		        {
		    			        case 0:/* to revise for CRZ_R1B */
		    				        /* CRZ_R1B */
		    				        ptr_out_param->CRZ_R1B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = out_xe_crz - out_xs_crz + 1;
		    				        ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp_offset >> DUAL_CRZ_PREC_BITS_2_TAP);
		    				        ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp_offset -
		    					        (long long)ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP);
                                    ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST >> 1;
                                    if (0 == (ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST & 0x1))
                                    {
                                        ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                                    }
                                    else
                                    {
                                        ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST + DUAL_CRZ_PREC_VAL_2_TAP;
                                    }
		    				        break;
		    			        case 1:
		    				        /* CRZ_R1A */
		    				        ptr_out_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = out_xe_crz - out_xs_crz + 1;
		    				        ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp_offset >> DUAL_CRZ_PREC_BITS_2_TAP);
		    				        ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp_offset -
		    					        (long long)ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_2_TAP);
                                    ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST >> 1;
                                    if (0 == (ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST & 0x1))
                                    {
                                        ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
                                    }
                                    else
                                    {
                                        ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST + DUAL_CRZ_PREC_VAL_2_TAP;
                                    }

                                    mrg_r13a_xend = ((in_xe_crz + DUAL_TILE_LOSS_CRZ_R1) > mrg_r13a_xend)? (in_xe_crz + DUAL_TILE_LOSS_CRZ_R1): mrg_r13a_xend;
									break;
		    			        default:
		    				        break;
		    		        }
		    		    }
		    		    else
		    		    {
		    		        long long temp_offset = (long long)out_xs_crz * DUAL_CRZ_PREC_VAL_SRC_ACC +
                                (long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_INT_OFST * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP +
                                (((long long)ptr_in_param->SW.TWIN_CRZ_R1_HORI_SUB_OFST * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP) >> DUAL_CRZ_CONFIG_BITS) -
                                (long long)in_xs_mrg * ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Bits.CRZ_HORI_STEP;

                            switch (i)
		    		        {
		    			        case 0:/* to revise for CRZ_R1B */
		    				        /* CRZ_R1B */
		    				        ptr_out_param->CRZ_R1B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = out_xe_crz - out_xs_crz + 1;
		    				        ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp_offset >> DUAL_CRZ_PREC_BITS_SRC_ACC);
		    				        ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp_offset -
    	    						    (long long)ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_SRC_ACC);
                                    ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST;
                                    ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;
		    				        break;
		    			        case 1:
		    				        /* CRZ_R1A */
		    				        ptr_out_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = out_xe_crz - out_xs_crz + 1;
		    				        ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = ((unsigned int)temp_offset >> DUAL_CRZ_PREC_BITS_SRC_ACC);
		    				        ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = (int)(temp_offset -
		    					        (long long)ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST * DUAL_CRZ_PREC_VAL_SRC_ACC);
                                    ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST;
                                    ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST;

                                    mrg_r13a_xend = ((in_xe_crz + DUAL_TILE_LOSS_CRZ_R1) > mrg_r13a_xend)? (in_xe_crz + DUAL_TILE_LOSS_CRZ_R1): mrg_r13a_xend;
									break;
                                default:
                                    break;
		    			    }
		    		    }

		    		    /* Decrease out_xe to next start pos */
		    		    out_xe_crz = out_xs_crz - 1;
		    	    }
		    	    else
		    	    {
		    		    if ((out_xe_crz > 0) && (out_xe_crz + 1 < ptr_in_param->SW.TWIN_CRZ_R1_OUT_WD) && (in_xs_crz <= in_xe_crz))
		    		    {
		    			    result = DUAL_MESSAGE_CRZ_XS_XE_CAL_ERROR;
		    			    dual_driver_printf("Error: %s\n", print_error_message(result));
		    			    return result;
		    		    }
		    		    switch (i)
		    		    {
		    			    case 0:/* to revise for CRZ_R1B */
		    				    ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = 0;
		    				    ptr_out_param->CRZ_R1B_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = 0;
		    				    ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = 0;
		    				    ptr_out_param->CRZ_R1B_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = 0;

		    				    /* check CRZ buffer size, must x8 align */
		    				    if (ptr_in_param->CRZO_R1A_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR == ptr_in_param->CRZO_R1B_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR)
		    				    {
		    					    ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN = 0;
		    				    }
		    				    else
		    				    {
		    					    ptr_out_param->CRZ_R1B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = DUAL_CRZ_ALIGN(out_shift[1]);
		    				    }
		    				    break;
		    			    case 1:
		    				    ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_INT_OFST.Bits.CRZ_LUMA_HORI_INT_OFST = 0;
		    				    ptr_out_param->CRZ_R1A_CRZ_LUMA_HORI_SUB_OFST.Bits.CRZ_LUMA_HORI_SUB_OFST = 0;
		    				    ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_INT_OFST.Bits.CRZ_CHRO_HORI_INT_OFST = 0;
		    				    ptr_out_param->CRZ_R1A_CRZ_CHRO_HORI_SUB_OFST.Bits.CRZ_CHRO_HORI_SUB_OFST = 0;

		    				    /* check CRZ buffer size, must x8 align */
		    				    if (ptr_in_param->CRZO_R1A_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR == ptr_in_param->CRZO_R1B_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR)
		    				    {
		    					    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN = 0;
		    				    }
		    				    else
		    				    {
		    					    ptr_out_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD = DUAL_CRZ_ALIGN(out_shift[1]);
		    				    }
		    				    break;
		    			    default:
		    				    break;
		    		    }
		    	    }
		    	}

		        if (out_xe_crz >= 0)
		        {
		    	    result = DUAL_MESSAGE_CRZ_XS_XE_CAL_ERROR;
		    	    dual_driver_printf("Error: %s\n", print_error_message(result));
		    	    return result;
		        }

                int bit_per_pixel = 8, bit_per_pixel_uv = 8;
                int bus_cut_shift = 1, crzo_plane_num = 1;

                switch (ptr_in_param->CAMCTL_R1A_CAMCTL_FMT_SEL.Bits.CAMCTL_CRZO_R1_FMT)
 	            {
	        	    case DUAL_OUT_FMT_YUV422:
	        		    bit_per_pixel  = 16;
	        		    bus_cut_shift  = 1;
	        		    crzo_plane_num = 1;
	        			break;
		    		case DUAL_OUT_FMT_YUV422_2:
		    			bit_per_pixel    = 8;
		    			bit_per_pixel_uv = 8;
		    			bus_cut_shift    = 1;
		    			crzo_plane_num   = 2;
		    			break;
            		default:
            		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	        	        dual_driver_printf("Error: %s\n", print_error_message(result));
	        	        return result;
   	            }

		    	/* CRZO_R1A */
		    	if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN)
		    	{
		    	    ptr_out_param->CRZO_R1A_CRZO_OFST_ADDR.Bits.CRZO_OFST_ADDR = 0;
	    		    ptr_out_param->CRZO_R1A_CRZO_CROP.Bits.CRZO_XOFFSET = 0;
	                ptr_out_param->CRZO_R1A_CRZO_XSIZE.Bits.CRZO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD, bit_per_pixel, bus_cut_shift);
	                if (crzo_plane_num >= 2)
	                {
	                    ptr_out_param->CRZBO_R1A_CRZBO_OFST_ADDR.Bits.CRZBO_OFST_ADDR = 0;
	    		        ptr_out_param->CRZBO_R1A_CRZBO_CROP.Bits.CRZBO_XOFFSET = 0;
	                    ptr_out_param->CRZBO_R1A_CRZBO_XSIZE.Bits.CRZBO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->CRZ_R1A_CRZ_OUT_IMG.Bits.CRZ_OUT_WD, bit_per_pixel_uv, bus_cut_shift);
	                }
	            }

		        if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN)
	            {
		    		/* CRZO_R1B */
		    		ptr_out_param->CRZO_R1B_CRZO_OFST_ADDR.Bits.CRZO_OFST_ADDR = 0;
	    		    ptr_out_param->CRZO_R1B_CRZO_CROP.Bits.CRZO_XOFFSET = 0;
		    		ptr_out_param->CRZO_R1B_CRZO_XSIZE.Bits.CRZO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->CRZ_R1B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD, bit_per_pixel, bus_cut_shift);

                    /* Clone registers */
                    ptr_out_param->CRZ_R1B_CRZ_HORI_STEP.Raw = ptr_out_param->CRZ_R1A_CRZ_HORI_STEP.Raw;
	                ptr_out_param->CRZ_R1B_CRZ_VERT_STEP.Raw = ptr_out_param->CRZ_R1A_CRZ_VERT_STEP.Raw;
	                ptr_out_param->CRZ_R1B_CRZ_IN_IMG.Bits.CRZ_IN_HT = ptr_out_param->CRZ_R1A_CRZ_IN_IMG.Bits.CRZ_IN_HT;
	                ptr_out_param->CRZO_R1B_CRZO_CON.Raw = ptr_in_param->CRZO_R1A_CRZO_CON.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_CON2.Raw = ptr_in_param->CRZO_R1A_CRZO_CON2.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_CON3.Raw = ptr_in_param->CRZO_R1A_CRZO_CON3.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_CON4.Raw = ptr_in_param->CRZO_R1A_CRZO_CON4.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_STRIDE.Raw = ptr_in_param->CRZO_R1A_CRZO_STRIDE.Raw;
	                ptr_out_param->CRZO_R1B_CRZO_CROP.Bits.CRZO_YOFFSET = ptr_in_param->CRZO_R1A_CRZO_CROP.Bits.CRZO_YOFFSET;
	                ptr_out_param->CRZO_R1B_CRZO_YSIZE.Bits.CRZO_YSIZE = ptr_in_param->CRZO_R1A_CRZO_YSIZE.Bits.CRZO_YSIZE;
                    if (crzo_plane_num >= 2)
                    {
                        ptr_out_param->CRZBO_R1B_CRZBO_OFST_ADDR.Bits.CRZBO_OFST_ADDR = 0;
	    		        ptr_out_param->CRZBO_R1B_CRZBO_CROP.Bits.CRZBO_XOFFSET = 0;
                        ptr_out_param->CRZBO_R1B_CRZBO_XSIZE.Bits.CRZBO_XSIZE = DUAL_CALC_BUS_XIZE(ptr_out_param->CRZ_R1B_CRZ_OUT_IMG.Bits.CRZ_OUT_WD, bit_per_pixel_uv, bus_cut_shift);

                        ptr_out_param->CRZBO_R1B_CRZBO_CON.Raw = ptr_in_param->CRZBO_R1A_CRZBO_CON.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_CON2.Raw = ptr_in_param->CRZBO_R1A_CRZBO_CON2.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_CON3.Raw = ptr_in_param->CRZBO_R1A_CRZBO_CON3.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_CON4.Raw = ptr_in_param->CRZBO_R1A_CRZBO_CON4.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_STRIDE.Raw = ptr_in_param->CRZBO_R1A_CRZBO_STRIDE.Raw;
	                    ptr_out_param->CRZBO_R1B_CRZBO_CROP.Bits.CRZBO_YOFFSET = ptr_in_param->CRZBO_R1A_CRZBO_CROP.Bits.CRZBO_YOFFSET;
	                    ptr_out_param->CRZBO_R1B_CRZBO_YSIZE.Bits.CRZBO_YSIZE = ptr_in_param->CRZBO_R1A_CRZBO_YSIZE.Bits.CRZBO_YSIZE;
                    }
                }

	            /* RTL */
	            if (ptr_in_param->CRZO_R1A_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR == ptr_in_param->CRZO_R1B_CRZO_BASE_ADDR.Bits.CRZO_BASE_ADDR)
	            {
		    		/* A+B */
		    	    if (0 == ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN)
		    	    {
		    			/* B only */
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_GGM_R2_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_G2C_R2_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_C42_R2_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BS_R1_EN = 0;
                        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN = 0;
	                    ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_PLNW_R2_EN = 0;
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_CRZO_R1_EN = 0;
		    		    ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_CRZBO_R1_EN = 0;
		    		}
		    		else
		    		{
                        /* A+B */
		    		    if (0 == ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN)
		    		    {
		    				/* A only */
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_GGM_R2_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_G2C_R2_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_C42_R2_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BS_R1_EN = 0;
                            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_CRZ_R1_EN = 0;
	                        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_PLNW_R2_EN = 0;
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_CRZO_R1_EN = 0;
		    		        ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_CRZBO_R1_EN = 0;
		    		        /* A only */
		    		        ptr_out_param->CRZO_R1A_CRZO_XSIZE.Bits.CRZO_XSIZE = ptr_in_param->CRZO_R1A_CRZO_XSIZE.Bits.CRZO_XSIZE;
		    		        if (crzo_plane_num >= 2)
                            {
                                ptr_out_param->CRZBO_R1A_CRZBO_XSIZE.Bits.CRZBO_XSIZE = ptr_in_param->CRZBO_R1A_CRZBO_XSIZE.Bits.CRZBO_XSIZE;
                            }
		    			}
		    			else
		    			{
		    				/* A + B */
		    		        ptr_out_param->CRZO_R1B_CRZO_OFST_ADDR.Bits.CRZO_OFST_ADDR = (int)ptr_out_param->CRZO_R1A_CRZO_XSIZE.Bits.CRZO_XSIZE + 1;
		    			    if (crzo_plane_num >= 2)
                            {
		    		            ptr_out_param->CRZBO_R1B_CRZBO_OFST_ADDR.Bits.CRZBO_OFST_ADDR = (int)ptr_out_param->CRZBO_R1A_CRZBO_XSIZE.Bits.CRZBO_XSIZE + 1;
                            }
		    			}
		    		}
		    	}
		    	else
		    	{
		    	    ptr_out_param->CRZO_R1B_CRZO_OFST_ADDR.Bits.CRZO_OFST_ADDR = 0;
		            /* restore stride */
		            ptr_out_param->CRZO_R1B_CRZO_STRIDE.Bits.CRZO_STRIDE = ptr_in_param->CRZO_R1B_CRZO_STRIDE.Bits.CRZO_STRIDE;
		            /* check valid size only WDMA with XOFF and YOFF support */
		            if ((int)ptr_out_param->CRZO_R1A_CRZO_XSIZE.Bits.CRZO_XSIZE + 1 > (int)ptr_in_param->CRZO_R1A_CRZO_STRIDE.Bits.CRZO_STRIDE)
		            {
		    	        ptr_out_param->CRZO_R1A_CRZO_XSIZE.Bits.CRZO_XSIZE = (((unsigned int)ptr_in_param->CRZO_R1A_CRZO_STRIDE.Bits.CRZO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		            }
		            if ((int)ptr_out_param->CRZO_R1B_CRZO_XSIZE.Bits.CRZO_XSIZE + 1 > (int)ptr_in_param->CRZO_R1B_CRZO_STRIDE.Bits.CRZO_STRIDE)
		            {
		    	        ptr_out_param->CRZO_R1B_CRZO_XSIZE.Bits.CRZO_XSIZE = (((unsigned int)ptr_in_param->CRZO_R1B_CRZO_STRIDE.Bits.CRZO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		            }

		            if (crzo_plane_num >= 2)
                    {
                        ptr_out_param->CRZBO_R1B_CRZBO_OFST_ADDR.Bits.CRZBO_OFST_ADDR = 0;
		                /* restore stride */
		                ptr_out_param->CRZBO_R1B_CRZBO_STRIDE.Bits.CRZBO_STRIDE = ptr_in_param->CRZBO_R1B_CRZBO_STRIDE.Bits.CRZBO_STRIDE;
		                /* check valid size only WDMA with XOFF and YOFF support */
		                if ((int)ptr_out_param->CRZBO_R1A_CRZBO_XSIZE.Bits.CRZBO_XSIZE + 1 > (int)ptr_in_param->CRZBO_R1A_CRZBO_STRIDE.Bits.CRZBO_STRIDE)
		                {
		    	            ptr_out_param->CRZBO_R1A_CRZBO_XSIZE.Bits.CRZBO_XSIZE = (((unsigned int)ptr_in_param->CRZBO_R1A_CRZBO_STRIDE.Bits.CRZBO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		                }
		                if ((int)ptr_out_param->CRZBO_R1B_CRZBO_XSIZE.Bits.CRZBO_XSIZE + 1 > (int)ptr_in_param->CRZBO_R1B_CRZBO_STRIDE.Bits.CRZBO_STRIDE)
		                {
		    	            ptr_out_param->CRZBO_R1B_CRZBO_XSIZE.Bits.CRZBO_XSIZE = (((unsigned int)ptr_in_param->CRZBO_R1B_CRZBO_STRIDE.Bits.CRZBO_STRIDE >> bus_cut_shift) << bus_cut_shift) - 1;
		                }
		            }
		    	}
            }

            /* MRG_R13 && MRG_R14 */
	        ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN = 0;
	        ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X = 0;
	        ptr_out_param->MRG_R13A_MRG_CTL.Bits.MRG_EDGE = 0xF;
	        ptr_out_param->MRG_R13A_MRG_VSIZE.Bits.MRG_HT = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
	            ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
	            ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
		    if (mrg_r13_valid[0] < (mrg_r13a_xend + 1))
	        {
	            /* add MRG_SIG_MODE1 & 2 */
	        	ptr_out_param->MRG_R13A_MRG_CTL.Bits.MRG_SIG_MODE1 = 0;
	        	ptr_out_param->MRG_R13A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	        	ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X = mrg_r13_valid[0] - 1;
	        	ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_STR_X = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		    	    (mrg_r13_valid[0] - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X): 0;
	        	ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_END_X = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		    		(mrg_r13a_xend - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X): (mrg_r13a_xend - mrg_r13_valid[0]);
	        	ptr_out_param->MRG_R14B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
	            ptr_out_param->MRG_R14B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	        	ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN = 1;
	        	ptr_out_param->MRG_R14B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	        	ptr_out_param->MRG_R14B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R13A_MRG_VSIZE.Bits.MRG_HT;
	        	if (((int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_STR_X + 1) >= DULL_MRG_R14B_MAX_WD)
	        	{
	        	    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
	        	    dual_driver_printf("Error: %s\n", print_error_message(result));
	        	    return result;
	            }
	        }
	        else
	        {
	            /* add MRG_SIG_MODE1 & 2 */
	        	ptr_out_param->MRG_R13A_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
	        	ptr_out_param->MRG_R13A_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
	            ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X = mrg_r13_valid[0] - 1;
	            ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN = 0;
	        }

	        /* MRG_R13B */
	        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R13_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_MRG_R13_EN;
		    ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		        (mrg_r13_valid[0] - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X): 0;
	        ptr_out_param->MRG_R13B_MRG_CTL.Bits.MRG_EDGE = 0xF;
	        ptr_out_param->MRG_R13B_MRG_VSIZE.Bits.MRG_HT = ptr_out_param->MRG_R13A_MRG_VSIZE.Bits.MRG_HT;
            /* update MRG_SIG_MODE1 & 2 */
        	ptr_out_param->MRG_R13B_MRG_CTL.Bits.MRG_SIG_MODE1 = 1;
        	ptr_out_param->MRG_R13B_MRG_CTL.Bits.MRG_SIG_MODE2 = 0;
        	/* A+B, MRG_R13B MRG_END_X */
        	ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_END_X = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
	            (mrg_r13_valid[0] + mrg_r13_valid[1] - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X - 1): (mrg_r13_valid[1] - 1);

            if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_RSSO_R2_EN)
		    {
                ptr_out_param->RSS_R2A_RSS_IN_IMG.Bits.RSS_IN_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1;
                if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN)
		    	{
                    ptr_out_param->RSS_R2A_RSS_IN_IMG.Bits.RSS_IN_WD += ((int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_STR_X + 1);
		    	}
                ptr_out_param->RSS_R2B_RSS_IN_IMG.Bits.RSS_IN_WD = (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X + 1;
		    }


            if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA2_EN.Bits.CAMCTL_CRZO_R2_EN)
		    {
                ptr_out_param->CRZ_R2A_CRZ_IN_IMG.Bits.CRZ_IN_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1;
                if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN)
		    	{
                    ptr_out_param->CRZ_R2A_CRZ_IN_IMG.Bits.CRZ_IN_WD += ((int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_STR_X + 1);
		    	}
                ptr_out_param->CRZ_R2B_CRZ_IN_IMG.Bits.CRZ_IN_WD = (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X + 1;
		    }

            if (ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_CRZO_R1_EN)
		    {
                ptr_out_param->CRZ_R1A_CRZ_IN_IMG.Bits.CRZ_IN_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1;
                if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN)
		    	{
                    ptr_out_param->CRZ_R1A_CRZ_IN_IMG.Bits.CRZ_IN_WD += ((int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_STR_X + 1);
		    	}
                ptr_out_param->CRZ_R1B_CRZ_IN_IMG.Bits.CRZ_IN_WD = (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X + 1;
		    }
		}
	}

	/* minimize SEP_R1A SEP_END_X and update all left end */
	if (sep_r1_left_max & (DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - 1))
	{
		sep_r1_left_max += DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - (sep_r1_left_max & (DUAL_ALIGN_PIXEL_MODE(main_pipe_cut_shift) - 1));
	}
	if (sep_r1_left_max > sep_r1_valid_width[0] + sep_r1_valid_width[1])
	{
	    result = DUAL_MESSAGE_SEP_SIZE_CAL_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}
	if (sep_r1_left_max > (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X + 1)
	{
	    result = DUAL_MESSAGE_SEP_SIZE_CAL_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}
	else
	{
		ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X = sep_r1_left_max - 1;
	}
	/* RRZ A enabled */
	if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_RRZ_R1_EN)
	{
		ptr_out_param->RRZ_R1A_RRZ_IN_IMG.Bits.RRZ_IN_WD = sep_r1_left_max;
	}
	/* SEP_R1A SEP_END_X */
	if (((int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X + raw_max_line_wdith[0] <= (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X) ||
		((int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X >= sep_r1_valid_width[0] + sep_r1_valid_width[1]))
	{
	    result = DUAL_MESSAGE_SEP_SIZE_CAL_ERROR;
	    dual_driver_printf("Error: %s\n", print_error_message(result));
	    return result;
	}

    return DUAL_MESSAGE_OK;
}

static DUAL_MESSAGE_ENUM dual_cal_slk_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

    if ((ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN || ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN) && ptr_in_param->DM_R1A_DM_SL_CTL.Bits.DM_SL_EN)
    {
        if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_SLK_R1_EN)
        {
            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
        }

        if ((0 == ptr_in_param->SLK_R1A_SLK_RZ.Bits.SLK_HRZ_COMP) || (0 == ptr_in_param->SLK_R1A_SLK_RZ.Bits.SLK_VRZ_COMP))
        {
            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
        }

        ptr_out_param->SLK_R1A_SLK_XOFF.Bits.SLK_X_OFST = 0;
		if (1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)
		{
		    if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN == 0)
		    {
				ptr_out_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_SLK_R1_EN = 0;

		        ptr_out_param->SLK_R1B_SLK_XOFF.Bits.SLK_X_OFST = 0;
		        ptr_out_param->SLK_R1A_SLK_SIZE.Bits.SLK_TPIPE_WD = ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
		        ptr_out_param->SLK_R1B_SLK_SIZE.Bits.SLK_TPIPE_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
		    }
		    else if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_DM_R1_EN  == 0)
		    {
                ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_SLK_R1_EN = 0;

				ptr_out_param->SLK_R1B_SLK_XOFF.Bits.SLK_X_OFST = 0;
		        ptr_out_param->SLK_R1A_SLK_SIZE.Bits.SLK_TPIPE_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
		        ptr_out_param->SLK_R1B_SLK_SIZE.Bits.SLK_TPIPE_WD = ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
		    }
		    else
		    {
			    ptr_out_param->SLK_R1B_SLK_XOFF.Bits.SLK_X_OFST = (int)ptr_in_param->SLK_R1A_SLK_RZ.Bits.SLK_HRZ_COMP * ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;

			    if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN)
   	            {
                    ptr_out_param->SLK_R1A_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1 +
                        (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_STR_X + 1;
   	            }
   	            else
   	            {
                    ptr_out_param->SLK_R1A_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1;
                }

                ptr_out_param->SLK_R1B_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X + 1;
			}
		}
		else
		{
			ptr_out_param->SLK_R1B_SLK_XOFF.Bits.SLK_X_OFST = (int)ptr_in_param->SLK_R1A_SLK_RZ.Bits.SLK_HRZ_COMP *
			    ((int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X + (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X);

            if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN)
   	        {
                ptr_out_param->SLK_R1A_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1 +
                    (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_STR_X + 1;
   	        }
   	        else
   	        {
                ptr_out_param->SLK_R1A_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1;
            }

            ptr_out_param->SLK_R1B_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X + 1;
        }

		ptr_out_param->SLK_R1A_SLK_SIZE.Bits.SLK_TPIPE_HT  = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		            ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		            ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

        ptr_out_param->DM_R1B_DM_SL_CTL.Bits.DM_SL_EN = ptr_in_param->DM_R1A_DM_SL_CTL.Bits.DM_SL_EN;
		ptr_out_param->SLK_R1B_SLK_RZ.Raw = ptr_in_param->SLK_R1A_SLK_RZ.Raw;
        ptr_out_param->SLK_R1B_SLK_SIZE.Bits.SLK_TPIPE_HT = ptr_out_param->SLK_R1A_SLK_SIZE.Bits.SLK_TPIPE_HT;
    }

    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_SLK_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_SLK_R1_EN;
    return result;
}

static DUAL_MESSAGE_ENUM dual_cal_slk_r2(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

    if ((ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_YNRS_R1_EN || ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_YNRS_R1_EN) && ptr_in_param->YNRS_R1A_YNRS_CON1.Bits.YNRS_SL2_LINK)
    {
        if (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_SLK_R2_EN)
        {
            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
        }

        if ((0 == ptr_in_param->SLK_R2A_SLK_RZ.Bits.SLK_HRZ_COMP) || (0 == ptr_in_param->SLK_R2A_SLK_RZ.Bits.SLK_VRZ_COMP))
        {
            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
        }

        ptr_out_param->SLK_R2A_SLK_XOFF.Bits.SLK_X_OFST = 0;
		if (1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)
		{
		    if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_YNRS_R1_EN  == 0)
		    {
		        ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_SLK_R2_EN = 0;

		        ptr_out_param->SLK_R2B_SLK_XOFF.Bits.SLK_X_OFST = 0;
		        ptr_out_param->SLK_R2A_SLK_SIZE.Bits.SLK_TPIPE_WD = ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
		        ptr_out_param->SLK_R2B_SLK_SIZE.Bits.SLK_TPIPE_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
		    }
		    else if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_YNRS_R1_EN == 0)
		    {
		        ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_SLK_R2_EN = 0;

		        ptr_out_param->SLK_R2B_SLK_XOFF.Bits.SLK_X_OFST = 0;
		        ptr_out_param->SLK_R2A_SLK_SIZE.Bits.SLK_TPIPE_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
		        ptr_out_param->SLK_R2B_SLK_SIZE.Bits.SLK_TPIPE_WD = ptr_out_param->RRZ_R1B_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
		    }
		    else
		    {
			    ptr_out_param->SLK_R2B_SLK_XOFF.Bits.SLK_X_OFST = (int)ptr_in_param->SLK_R2A_SLK_RZ.Bits.SLK_HRZ_COMP * ptr_out_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;

                if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN)
   	            {
   	                ptr_out_param->SLK_R2A_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1 +
   	                    (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_STR_X + 1;
		        }
		        else
		        {
		            ptr_out_param->SLK_R2A_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1;
	            }
                ptr_out_param->SLK_R2B_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X + 1;
            }
		}
		else
		{
			ptr_out_param->SLK_R2B_SLK_XOFF.Bits.SLK_X_OFST = (int)ptr_in_param->SLK_R2A_SLK_RZ.Bits.SLK_HRZ_COMP *
			    ((int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X + (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X);

            if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_MRG_R14_EN)
   	        {
   	            ptr_out_param->SLK_R2A_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1 +
   	                (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R14B_MRG_CROP.Bits.MRG_STR_X + 1;
		    }
		    else
		    {
		        ptr_out_param->SLK_R2A_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13A_MRG_CROP.Bits.MRG_STR_X + 1;
	        }

	        ptr_out_param->SLK_R2B_SLK_SIZE.Bits.SLK_TPIPE_WD = (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_END_X - (int)ptr_out_param->MRG_R13B_MRG_CROP.Bits.MRG_STR_X + 1;
	    }

		ptr_out_param->SLK_R2A_SLK_SIZE.Bits.SLK_TPIPE_HT  = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_DM_R1_SEL)?
		            ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		            ptr_in_param->RRZ_R1A_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;

		ptr_out_param->YNRS_R1B_YNRS_CON1.Bits.YNRS_SL2_LINK = ptr_in_param->YNRS_R1A_YNRS_CON1.Bits.YNRS_SL2_LINK;
		ptr_out_param->SLK_R2B_SLK_RZ.Raw = ptr_in_param->SLK_R2A_SLK_RZ.Raw;
        ptr_out_param->SLK_R2B_SLK_SIZE.Bits.SLK_TPIPE_HT = ptr_out_param->SLK_R2A_SLK_SIZE.Bits.SLK_TPIPE_HT;
    }

    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_SLK_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_SLK_R2_EN;
    return result;
}

static DUAL_MESSAGE_ENUM dual_cal_flk_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

    ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_FLK_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_FLK_SEL;
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_QBN_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_QBN_R3_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_qbin_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* QBIN_R1_EN */
	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_QBN_R1_EN)
	{
		if (ptr_in_param->QBIN_R1A_QBIN_CTL.Bits.QBIN_ACC)
		{
			/* MRG_R1A */
			ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_STR_X >>
				((int)ptr_in_param->QBIN_R1A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R1A_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R1A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
			/* MRG_R1B */
			ptr_out_param->MRG_R1B_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R1B_MRG_CROP.Bits.MRG_STR_X >>
				((int)ptr_in_param->QBIN_R1A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R1B_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R1B_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R1A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
		}
	}

	ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_QBN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_QBN_R1_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_qbin_r2(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* QBIN_R2_EN */
	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_QBN_R2_EN)
	{
		if (ptr_in_param->QBIN_R2A_QBIN_CTL.Bits.QBIN_ACC)
		{
			/* MRG_R2A */
			ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_STR_X >>
				((int)ptr_in_param->QBIN_R2A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R2A_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R2A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
			/* MRG_R2B */
			ptr_out_param->MRG_R2B_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R2B_MRG_CROP.Bits.MRG_STR_X >>
				((int)ptr_in_param->QBIN_R2A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R2B_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R2B_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R2A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
		}
	}

	ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_QBN_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_QBN_R2_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_qbin_r3(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

	/* QBIN_R3_EN */
	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_QBN_R3_EN)
	{
		if (ptr_in_param->QBIN_R3A_QBIN_CTL.Bits.QBIN_ACC)
		{
			/* QBIN_R3A */
			ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_STR_X >>
				((int)ptr_in_param->QBIN_R3A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R10A_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R3A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
			/* QBIN_R3B */
			ptr_out_param->MRG_R10B_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R10B_MRG_CROP.Bits.MRG_STR_X >>
				((int)ptr_in_param->QBIN_R3A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R10B_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R10B_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R3A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
		}
	}

	ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_QBN_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_QBN_R3_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_qbin_r4(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

	/* QBIN_R4_EN */
	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_QBN_R4_EN)
	{
		if (ptr_in_param->QBIN_R4A_QBIN_CTL.Bits.QBIN_ACC)
		{
			/* QBIN_R4A */
			ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_STR_X >>
				((int)ptr_in_param->QBIN_R4A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R4A_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R4A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
			/* QBIN_R4B */
			ptr_out_param->MRG_R4B_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R4B_MRG_CROP.Bits.MRG_STR_X >>
				((int)ptr_in_param->QBIN_R4A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R4B_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R4B_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R4A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
		}
	}

	ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_QBN_R4_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_QBN_R4_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_qbin_r5(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

	/* QBIN_R5_EN */
	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_QBN_R5_EN)
	{
		if (ptr_in_param->QBIN_R5A_QBIN_CTL.Bits.QBIN_ACC)
		{
			/* QBIN_R5A */
			ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_STR_X >>
				((int)ptr_in_param->QBIN_R5A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R5A_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R5A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
			/* QBIN_R5B */
			ptr_out_param->MRG_R5B_MRG_CROP.Bits.MRG_STR_X = (unsigned int)ptr_out_param->MRG_R5B_MRG_CROP.Bits.MRG_STR_X>>
				((int)ptr_in_param->QBIN_R5A_QBIN_CTL.Bits.QBIN_ACC);
			ptr_out_param->MRG_R5B_MRG_CROP.Bits.MRG_END_X = (((unsigned int)ptr_out_param->MRG_R5B_MRG_CROP.Bits.MRG_END_X + 1) >>
				((int)ptr_in_param->QBIN_R5A_QBIN_CTL.Bits.QBIN_ACC)) - 1;
		}
	}

	ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_QBN_R5_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_QBN_R5_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_ltm_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_LTM_R1_EN)
	{
	    int blkx_start;
	    int blkx_end;
	    int cntx_start;
	    int cntx_end;

        if ((ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH == 0) ||
            (ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_HEIGHT == 0))
        {
            result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
        }

		/* LTM_R1A */
		blkx_start = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X / (int)ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;
		if (blkx_start >= (int)ptr_in_param->LTM_R1A_LTM_BLK_NUM.Bits.LTM_BLK_X_NUM)
		{
		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }

		blkx_end = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X / (int)ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;
		if (blkx_end >= (int)ptr_in_param->LTM_R1A_LTM_BLK_NUM.Bits.LTM_BLK_X_NUM)
		{
		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }

		cntx_start = (int)(ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X) % (int)ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;
		cntx_end = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X % (int)ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;

        ptr_out_param->LTM_R1A_LTM_TILE_SIZE.Bits.LTM_TILE_HSIZE = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X + 1;
        ptr_out_param->LTM_R1A_LTM_TILE_SIZE.Bits.LTM_TILE_VSIZE = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		    ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		    ((unsigned int)ptr_in_param->SW.TWIN_RAWI_YSIZE >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));
		ptr_out_param->LTM_R1A_LTM_TILE_NUM.Bits.LTM_TILE_BLK_X_NUM_START = blkx_start;
		ptr_out_param->LTM_R1A_LTM_TILE_NUM.Bits.LTM_TILE_BLK_X_NUM_END = blkx_end;
		ptr_out_param->LTM_R1A_LTM_TILE_CNTX.Bits.LTM_TILE_BLK_X_CNT_START = cntx_start;
		ptr_out_param->LTM_R1A_LTM_TILE_CNTX.Bits.LTM_TILE_BLK_X_CNT_END= cntx_end;

        /* LTM_R1B */
        blkx_start = (int)(ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X) / (int)ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;
        if (blkx_start >= (int)ptr_in_param->LTM_R1A_LTM_BLK_NUM.Bits.LTM_BLK_X_NUM)
		{
		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }

		blkx_end = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X / (int)ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;
		if (blkx_end >= (int)ptr_in_param->LTM_R1A_LTM_BLK_NUM.Bits.LTM_BLK_X_NUM)
		{
		    result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
	    }

		cntx_start = (int)(ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X) % (int)ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;
		cntx_end = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X % (int)ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;

        ptr_out_param->LTM_R1B_LTM_TILE_SIZE.Bits.LTM_TILE_HSIZE = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X + 1;
        ptr_out_param->LTM_R1B_LTM_TILE_SIZE.Bits.LTM_TILE_VSIZE = ptr_out_param->LTM_R1A_LTM_TILE_SIZE.Bits.LTM_TILE_VSIZE;
		ptr_out_param->LTM_R1B_LTM_TILE_NUM.Bits.LTM_TILE_BLK_X_NUM_START = blkx_start;
		ptr_out_param->LTM_R1B_LTM_TILE_NUM.Bits.LTM_TILE_BLK_X_NUM_END = blkx_end;
		ptr_out_param->LTM_R1B_LTM_TILE_CNTX.Bits.LTM_TILE_BLK_X_CNT_START = cntx_start;
		ptr_out_param->LTM_R1B_LTM_TILE_CNTX.Bits.LTM_TILE_BLK_X_CNT_END= cntx_end;

        // Copy from A to B
        ptr_out_param->LTM_R1B_LTM_BLK_NUM.Bits.LTM_BLK_X_NUM = ptr_in_param->LTM_R1A_LTM_BLK_NUM.Bits.LTM_BLK_X_NUM;
        ptr_out_param->LTM_R1B_LTM_BLK_NUM.Bits.LTM_BLK_Y_NUM = ptr_in_param->LTM_R1A_LTM_BLK_NUM.Bits.LTM_BLK_Y_NUM;
        ptr_out_param->LTM_R1B_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH = ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_WIDTH;
        ptr_out_param->LTM_R1B_LTM_BLK_SZ.Bits.LTM_BLK_HEIGHT = ptr_in_param->LTM_R1A_LTM_BLK_SZ.Bits.LTM_BLK_HEIGHT;
        ptr_out_param->LTM_R1B_LTM_TILE_NUM.Bits.LTM_TILE_BLK_Y_NUM_START = ptr_in_param->LTM_R1A_LTM_TILE_NUM.Bits.LTM_TILE_BLK_Y_NUM_START;
        ptr_out_param->LTM_R1B_LTM_TILE_NUM.Bits.LTM_TILE_BLK_Y_NUM_END = ptr_in_param->LTM_R1A_LTM_TILE_NUM.Bits.LTM_TILE_BLK_Y_NUM_END;
        ptr_out_param->LTM_R1B_LTM_TILE_CNTY.Bits.LTM_TILE_BLK_Y_CNT_START = ptr_in_param->LTM_R1A_LTM_TILE_CNTY.Bits.LTM_TILE_BLK_Y_CNT_START;
        ptr_out_param->LTM_R1B_LTM_TILE_CNTY.Bits.LTM_TILE_BLK_Y_CNT_END = ptr_in_param->LTM_R1A_LTM_TILE_CNTY.Bits.LTM_TILE_BLK_Y_CNT_END;

   	}
    // CPU to CCU
   	ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_LTM_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_LTM_R1_EN;

    return result;
}

static DUAL_MESSAGE_ENUM dual_cal_hlr_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_HLR_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_HLR_R1_EN;
	return DUAL_MESSAGE_OK;
}

static DUAL_MESSAGE_ENUM dual_cal_wb_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_WB_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_WB_R1_EN;
	return DUAL_MESSAGE_OK;
}

static DUAL_MESSAGE_ENUM dual_cal_lsc_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_LSC_R1_EN)
	{
		int blkx_start, blkx_end;
		int bit_per_pix = ptr_in_param->LSC_R1A_LSC_CTL1.Bits.LSC_EXTEND_COEF_MODE? 768: 512;
		/* LSC_R1A */
		blkx_start = (int)(ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X) / (2 * (int)ptr_in_param->LSC_R1A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH);
		if (blkx_start >= ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
		{
			blkx_start = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL;
		}
		blkx_end = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X / (2 * (int)ptr_in_param->LSC_R1A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) + 1;
		ptr_out_param->LSC_R1A_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->LSC_R1A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
		if (blkx_end > ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
		{
			blkx_end = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL + 1;
			ptr_out_param->LSC_R1A_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->SW.TWIN_MODE_SDBLK_lWIDTH_ALL;
		}
		ptr_out_param->LSC_R1A_LSC_CTL2.Bits.LSC_SDBLK_XNUM = blkx_end - blkx_start - 1;
		ptr_out_param->LSC_R1A_LSC_TPIPE_OFST.Bits.LSC_TPIPE_OFST_X = (unsigned int)((int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X -
		    blkx_start * 2 * (int)ptr_in_param->LSC_R1A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) >> 1;
		ptr_out_param->LSC_R1A_LSC_TPIPE_SIZE.Bits.LSC_TPIPE_SIZE_X = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X -
		    (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X + 1;
		ptr_out_param->LSCI_R1A_LSCI_OFST_ADDR.Bits.LSCI_OFST_ADDR = (unsigned int)(blkx_start * bit_per_pix) >> 3;
		ptr_out_param->LSCI_R1A_LSCI_XSIZE.Bits.LSCI_XSIZE = ((unsigned int)((blkx_end - blkx_start)* bit_per_pix) >> 3) - 1;
		if ((int)ptr_out_param->LSCI_R1A_LSCI_OFST_ADDR.Bits.LSCI_OFST_ADDR + (int)ptr_out_param->LSCI_R1A_LSCI_XSIZE.Bits.LSCI_XSIZE + 1 >
		    (int)ptr_in_param->LSCI_R1A_LSCI_STRIDE.Bits.LSCI_STRIDE)
		{
		    result = DUAL_MESSAGE_INVALID_LSCI_STRIDE_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}

		/* LSC_R1B */
		blkx_start = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X / (2 * (int)ptr_in_param->LSC_R1A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH);
		if (blkx_start >= ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
		{
			blkx_start = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL;
		}
		blkx_end = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X / (2 * (int)ptr_in_param->LSC_R1A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) + 1;
		ptr_out_param->LSC_R1B_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->LSC_R1A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
		if (blkx_end > ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
		{
			blkx_end = (int)ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL + 1;
			ptr_out_param->LSC_R1B_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->SW.TWIN_MODE_SDBLK_lWIDTH_ALL;
		}
		ptr_out_param->LSC_R1B_LSC_CTL2.Bits.LSC_SDBLK_XNUM = blkx_end - blkx_start - 1;
		ptr_out_param->LSC_R1B_LSC_TPIPE_OFST.Bits.LSC_TPIPE_OFST_X = (unsigned int)((int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X -
		    blkx_start * 2 * (int)ptr_in_param->LSC_R1A_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) >> 1;
		ptr_out_param->LSC_R1B_LSC_TPIPE_SIZE.Bits.LSC_TPIPE_SIZE_X = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X -
			(int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X + 1;
		ptr_out_param->LSCI_R1B_LSCI_OFST_ADDR.Bits.LSCI_OFST_ADDR = (unsigned int)(blkx_start * bit_per_pix) >> 3;
		ptr_out_param->LSCI_R1B_LSCI_XSIZE.Bits.LSCI_XSIZE = ((unsigned int)((blkx_end - blkx_start) * bit_per_pix) >> 3) - 1;
		if ((int)ptr_out_param->LSCI_R1B_LSCI_OFST_ADDR.Bits.LSCI_OFST_ADDR + (int)ptr_out_param->LSCI_R1B_LSCI_XSIZE.Bits.LSCI_XSIZE + 1 >
			(int)ptr_out_param->LSCI_R1B_LSCI_STRIDE.Bits.LSCI_STRIDE)
		{
		    result = DUAL_MESSAGE_INVALID_LSCI_STRIDE_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
	}

	/* assign common config, LSCI */
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_LSC_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_LSC_R1_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_dgn_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_DGN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_DGN_R1_EN;
	return DUAL_MESSAGE_OK;
}

static DUAL_MESSAGE_ENUM dual_cal_rnr_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_RNR_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_RNR_R1_EN;
	return DUAL_MESSAGE_OK;
}

static DUAL_MESSAGE_ENUM dual_cal_fus_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

    ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_DGN_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_DGN_SEL;
    ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_BPC_R2_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_BPC_R2_SEL;
    ptr_out_param->CAMCTL_R1B_CAMCTL_SEL2.Bits.CAMCTL_BPC_R3_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_BPC_R3_SEL;

    if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_FUS_R1_EN)
    {
        // Pipeline A
        int in_sep_xs = (unsigned int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X;
        int in_sep_xe = (unsigned int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X;

        ptr_out_param->FUS_R1A_FUS_SIZE.Bits.FUS_HSIZE = in_sep_xe - in_sep_xs + 1;
        ptr_out_param->FUS_R1A_FUS_SIZE.Bits.FUS_VSIZE = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		    ((unsigned int)ptr_in_param->SEP_R1A_SEP_VSIZE.Bits.SEP_HT >> (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN):
		    ((unsigned int)ptr_in_param->SW.TWIN_RAWI_YSIZE >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

        if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_BPC_R2_SEL == 0 &&
            ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_UNP_R2_EN)
        {
            if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL != 0)
            {
                result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		        dual_driver_printf("Error: %s\n", print_error_message(result));
		        return result;
            }

 	        // RAWI_R2A
 	        int bit_per_pixel = 8;
	        switch(ptr_in_param->CAMCTL_R1A_CAMCTL_FMT3_SEL.Bits.CAMCTL_RAWI_R2_FMT)
	        {
                case 8:
                    bit_per_pixel = 8;
                    break;
                case 9:
                    bit_per_pixel = 10;
                    break;
                case 10:
                    bit_per_pixel = 12;
                    break;
                case 11:
                    bit_per_pixel = 14;
                    break;
                case 16:
                    bit_per_pixel = 10;
                    break;
                default:
                    break;
            }

            int start_bit_offset = (unsigned int)(in_sep_xs * bit_per_pixel) & 0x0F;  /* remainder of 16 */
            int end_bit_offset   = (unsigned int)(start_bit_offset + (in_sep_xe - in_sep_xs + 1) * bit_per_pixel) & 0x0F;

            // UNP_R2A
            ptr_out_param->UNP_R2A_UNP_OFST.Bits.UNP_OFST_STB = start_bit_offset;
            ptr_out_param->UNP_R2A_UNP_OFST.Bits.UNP_OFST_EDB = end_bit_offset;

            // RAWI_R2A
	        ptr_out_param->RAWI_R2A_RAWI_OFST_ADDR.Bits.RAWI_OFST_ADDR = ((in_sep_xs * bit_per_pixel - start_bit_offset) >> 3);
	    	ptr_out_param->RAWI_R2A_RAWI_XSIZE.Bits.RAWI_XSIZE = (((unsigned int)(start_bit_offset + (in_sep_xe - in_sep_xs + 1) * bit_per_pixel + 7) >> 3) - 1) | 0x1;
        }

        // Pipeline B
        in_sep_xs = (unsigned int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
        in_sep_xe = (unsigned int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X;

        ptr_out_param->FUS_R1B_FUS_SIZE.Bits.FUS_HSIZE = in_sep_xe - in_sep_xs + 1;
        ptr_out_param->FUS_R1B_FUS_SIZE.Bits.FUS_VSIZE = ptr_out_param->FUS_R1A_FUS_SIZE.Bits.FUS_VSIZE;

        if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_BPC_R2_SEL == 0 &&
            ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_UNP_R2_EN)
        {
            if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL != 0)
            {
                result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
		        dual_driver_printf("Error: %s\n", print_error_message(result));
		        return result;
            }

 	        // RAWI_R2B
 	        int bit_per_pixel = 8;
	        switch(ptr_in_param->CAMCTL_R1A_CAMCTL_FMT3_SEL.Bits.CAMCTL_RAWI_R2_FMT)
	        {
                case 8:
                    bit_per_pixel = 8;
                    break;
                case 9:
                    bit_per_pixel = 10;
                    break;
                case 10:
                    bit_per_pixel = 12;
                    break;
                case 11:
                    bit_per_pixel = 14;
                    break;
                case 16:
                    bit_per_pixel = 10;
                    break;
                default:
                    break;
            }

            int start_bit_offset = (unsigned int)(in_sep_xs * bit_per_pixel) & 0x0F;  /* remainder of 16 */
            int end_bit_offset   = (unsigned int)(start_bit_offset + (in_sep_xe - in_sep_xs + 1) * bit_per_pixel) & 0x0F;

            // UNP_R2B
            ptr_out_param->UNP_R2B_UNP_OFST.Bits.UNP_OFST_STB = start_bit_offset;
            ptr_out_param->UNP_R2B_UNP_OFST.Bits.UNP_OFST_EDB = end_bit_offset;

            // RAWI_R2B
	        ptr_out_param->RAWI_R2B_RAWI_OFST_ADDR.Bits.RAWI_OFST_ADDR = ((in_sep_xs * bit_per_pixel - start_bit_offset) >> 3);
	    	ptr_out_param->RAWI_R2B_RAWI_XSIZE.Bits.RAWI_XSIZE = (((unsigned int)(start_bit_offset + (in_sep_xe - in_sep_xs + 1) * bit_per_pixel + 7) >> 3) - 1) | 0x1;
            ptr_out_param->RAWI_R2B_RAWI_YSIZE.Bits.RAWI_YSIZE = ptr_in_param->RAWI_R2A_RAWI_YSIZE.Bits.RAWI_YSIZE;
            ptr_out_param->RAWI_R2B_RAWI_STRIDE.Bits.RAWI_STRIDE = ptr_in_param->RAWI_R2A_RAWI_STRIDE.Bits.RAWI_STRIDE;
		}
    }

    /* UNP & RAWI */
    ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_UNP_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_UNP_R2_EN;
    ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_UNP_R3_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_UNP_R3_EN;
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_UNP_R4_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_UNP_R4_EN;
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_FUS_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_FUS_R1_EN;

    /* ZFUS */
    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_ZFUS_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_ZFUS_R1_EN;
    return result;
}

static DUAL_MESSAGE_ENUM dual_cal_bpc_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BPC_R1_EN)
	{
		/*update size by bin enable */
		int bin_sel_h_size = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
			(((unsigned int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_E - (unsigned int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_S) >>
			((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN)):
			((unsigned int)ptr_in_param->SW.TWIN_RAWI_XSIZE >>
			((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

       int sep_v_size = (((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)? ((unsigned int)ptr_in_param->TG_R1A_TG_SEN_GRAB_LIN.Bits.TG_LIN_E - (unsigned int)ptr_in_param->TG_R1A_TG_SEN_GRAB_LIN.Bits.TG_LIN_S):
		                  (unsigned int)(ptr_in_param->RAWI_R2A_RAWI_YSIZE.Bits.RAWI_YSIZE + 1)) >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

		if (ptr_in_param->BPC_R1A_BPC_BPC_CON.Bits.BPC_LE_INV_CTL)
		{
		    result = DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		//if (ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
		//{
		//	/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
		//	ptr_out_param->BPC_R1B_BPC_BPC_CON.Bits.BPC_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
		//}
		//else
		{
			ptr_out_param->BPC_R1B_BPC_BPC_CON.Bits.BPC_LE_INV_CTL = 0;
		}
		/* BPC_R1A */
		ptr_out_param->BPC_R1A_BPC_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X;
		ptr_out_param->BPC_R1A_BPC_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
		ptr_out_param->BPC_R1A_BPC_BPC_TBLI2.Bits.BPC_XSIZE = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X -
			(int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X;
		ptr_out_param->BPC_R1A_BPC_BPC_TBLI2.Bits.BPC_YSIZE = sep_v_size - 1;
		ptr_out_param->BPC_R1A_BPC_PDC_POS.Bits.BPC_PDC_XCENTER = (unsigned int)bin_sel_h_size >> 1;
		ptr_out_param->BPC_R1A_BPC_PDC_POS.Bits.BPC_PDC_YCENTER = (unsigned int)sep_v_size >> 1;
		/* BPC_R1B */
		ptr_out_param->BPC_R1B_BPC_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
		ptr_out_param->BPC_R1B_BPC_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
		ptr_out_param->BPC_R1B_BPC_BPC_TBLI2.Bits.BPC_XSIZE = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X - (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
		ptr_out_param->BPC_R1B_BPC_BPC_TBLI2.Bits.BPC_YSIZE = sep_v_size - 1;
		ptr_out_param->BPC_R1B_BPC_PDC_POS.Bits.BPC_PDC_XCENTER = (unsigned int)bin_sel_h_size >> 1;
		ptr_out_param->BPC_R1B_BPC_PDC_POS.Bits.BPC_PDC_YCENTER = (unsigned int)sep_v_size >> 1;
	}

	/* copy */
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_BPC_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_BPC_R1_EN;
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_OBC_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_OBC_R1_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_bpc_r2(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BPC_R2_EN)
	{
		/*update size by bin enable */
		int bin_sel_h_size = (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
			(((unsigned int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_E - (unsigned int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_S) >>
			((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN)):
			((unsigned int)ptr_in_param->SW.TWIN_RAWI_XSIZE >>
			((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

        int sep_v_size = (((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)? ((unsigned int)ptr_in_param->TG_R1A_TG_SEN_GRAB_LIN.Bits.TG_LIN_E - (unsigned int)ptr_in_param->TG_R1A_TG_SEN_GRAB_LIN.Bits.TG_LIN_S):
		                  (unsigned int)(ptr_in_param->RAWI_R2A_RAWI_YSIZE.Bits.RAWI_YSIZE + 1)) >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN));

		if (ptr_in_param->BPC_R2A_BPC_BPC_CON.Bits.BPC_LE_INV_CTL)
		{
		    result = DUAL_MESSAGE_INVALID_LE_INV_CTL_ERROR;
		    dual_driver_printf("Error: %s\n", print_error_message(result));
		    return result;
		}
		//if (ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X & (DUAL_ZHDR_ALIGN_PIXEL - 1))
		//{
		//	/* 0x1: LE, 0x2: R, 0x4: G, 0x8: B */
		//	ptr_out_param->BPC_R2B_BPC_BPC_CON.Bits.BPC_LE_INV_CTL = DUAL_ZHDR_LE_INV_R + DUAL_ZHDR_LE_INV_B;
		//}
		//else
		{
			ptr_out_param->BPC_R2B_BPC_BPC_CON.Bits.BPC_LE_INV_CTL = 0;
		}
		/* BPC_R2A */
		ptr_out_param->BPC_R2A_BPC_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X;
		ptr_out_param->BPC_R2A_BPC_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
		ptr_out_param->BPC_R2A_BPC_BPC_TBLI2.Bits.BPC_XSIZE = (int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X -
			(int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X;
		ptr_out_param->BPC_R2A_BPC_BPC_TBLI2.Bits.BPC_YSIZE = sep_v_size - 1;
		ptr_out_param->BPC_R2A_BPC_PDC_POS.Bits.BPC_PDC_XCENTER = (unsigned int)bin_sel_h_size >> 1;
		ptr_out_param->BPC_R2A_BPC_PDC_POS.Bits.BPC_PDC_YCENTER = (unsigned int)sep_v_size >> 1;
		/* BPC_R2B */
		ptr_out_param->BPC_R2B_BPC_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
		ptr_out_param->BPC_R2B_BPC_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
		ptr_out_param->BPC_R2B_BPC_BPC_TBLI2.Bits.BPC_XSIZE = (int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X -
			(int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
		ptr_out_param->BPC_R2B_BPC_BPC_TBLI2.Bits.BPC_YSIZE = sep_v_size - 1;
		ptr_out_param->BPC_R2B_BPC_PDC_POS.Bits.BPC_PDC_XCENTER = (unsigned int)bin_sel_h_size >> 1;
		ptr_out_param->BPC_R2B_BPC_PDC_POS.Bits.BPC_PDC_YCENTER = (unsigned int)sep_v_size >> 1;
	}

	/* copy */
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_BPC_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BPC_R2_EN;
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_OBC_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_OBC_R2_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_fbnd_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;
	/* FBND x2 SEP */
	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN)
	{
		ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X = (unsigned int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X << 1;
		ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X = (((unsigned int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X + 1) << 1) - 1;
		ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X = (unsigned int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X << 1;
		ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X = (((unsigned int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X + 1) << 1) - 1;
	}

    /* Copy from A to B */
	ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN;
	//ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN;
	//ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN;
	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_irm_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

    ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_IRM_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IRM_R1_EN;
    return result;
}

static DUAL_MESSAGE_ENUM dual_cal_iobc_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

    ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_IOBC_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IOBC_R1_EN;
    return result;
}

static DUAL_MESSAGE_ENUM dual_cal_ibpc_r1(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

    ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_IBPC_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_IBPC_R1_EN;
    return result;
}

static DUAL_MESSAGE_ENUM dual_cal_ufd_r2(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

    if ((2 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL) &&
        (0 == ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PIX_BUS_RAWI_R2) &&
        (1 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL) &&
		(0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_SEP_SEL) &&
        ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_UFDI_R2_EN &&
        ptr_in_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_RAWI_R2_EN &&
        ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFD_R2_EN &&
		(0 == ptr_in_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_SEP_SEL))
    {
        int au_full_size = ((unsigned int)((unsigned int)ptr_in_param->SW.TWIN_RAWI_XSIZE + (DUAL_UFO_AU_SIZE - 1))) >> DUAL_UFO_AU_BIT;

        // UFD_R2A
        int ufd_xs = (unsigned int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X << (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
                                                                                      ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN);
        int ufd_xe = (((unsigned int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X  + 1) << (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
                                                                                              ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN)) - 1;
        int au_xs  = (unsigned int)ufd_xs >> DUAL_UFO_AU_BIT;
        int au_xe  = (unsigned int)ufd_xe >> DUAL_UFO_AU_BIT;

	    /* check UFD_R2A_BS2_AU_START */
	    if (ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START &&
	        ((int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START < au_full_size) &&
	        (1 == ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE))
	    {
		    ptr_out_param->UFD_R2A_UFD_AU_CON.Bits.UFD_AU_OFST = au_xs;
		    ptr_out_param->UFD_R2A_UFD_AU_CON.Bits.UFD_AU_SIZE = au_xe - au_xs + 1;
		    if (au_xe < (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START)
		    {
			    /* AU offset and size */
			    ptr_out_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE = 0;/* left */
			    ptr_out_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
			    ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_OFST = 0;
			    ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_SIZE = 0;
			    ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			    ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
		    }
		    else if (au_xs < (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START)
		    {
		        ptr_out_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE = 1;/* left + middle */
		        if ((1 == ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE) &&
		            (au_xe >= (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START))
		        {
		            ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_OFST = 0;
			        ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_SIZE =  (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START -
			                                                                (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START;
			        ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			        ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_SIZE = au_xe - (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START + 1;
			        ptr_out_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 1;
		        }
		        else
		        {
		            /* AU offset and size */
			        ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_OFST = 0;
			        ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_SIZE = au_xe - (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START + 1;
			        ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			        ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
			        ptr_out_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
		        }
		    }
		    else
		    {
		        ptr_out_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE = 2;
		        if (1 == ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE)
		        {
		            if (au_xs < (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START)
		            {
		                ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_OFST = au_xs - (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START;
		                if (au_xe < (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START)
		                {
			                ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_SIZE = au_xe - au_xs + 1;
			                ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			                ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
			                ptr_out_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
		                }
		                else
		                {
		                    ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_SIZE = (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START - au_xs;
		                    ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			                ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_SIZE = au_xe - (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START + 1;
			                ptr_out_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 1;
		                }
		            }
		            else
		            {
		                ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_OFST = au_xs - (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START;
			            ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_SIZE = au_xe - au_xs + 1;
			            ptr_out_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 2;
		            }
		        }
		        else
		        {
			        /* AU offset and size */
			        ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_OFST = au_xs - (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START;
			        ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_SIZE = au_xe - au_xs + 1;
			        ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			        ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
			        ptr_out_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
			    }
		    }
	    }
	    else
	    {
		    /* AU offset and size */
		    ptr_out_param->UFD_R2A_UFD_AU_CON.Bits.UFD_AU_OFST  = au_xs;
		    ptr_out_param->UFD_R2A_UFD_AU_CON.Bits.UFD_AU_SIZE  = au_xe - au_xs + 1;
            ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_OFST = 0;
		    ptr_out_param->UFD_R2A_UFD_AU2_CON.Bits.UFD_AU2_SIZE = 0;
		    ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
		    ptr_out_param->UFD_R2A_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
		    ptr_out_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE  = 0;
		    ptr_out_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
	    }

        /* UFD_R2 crop window */
        ptr_out_param->UFD_R2A_UFD_CROP_CON1.Bits.UFD_X_START = ufd_xs - (((unsigned int)au_xs) << DUAL_UFO_AU_BIT);
        ptr_out_param->UFD_R2A_UFD_CROP_CON1.Bits.UFD_X_END = ufd_xe - (((unsigned int)au_xs) << DUAL_UFO_AU_BIT);
        ptr_out_param->UFD_R2A_UFD_SIZE_CON.Bits.UFD_WD = ufd_xe - (((unsigned int)au_xs) << DUAL_UFO_AU_BIT) + 1;

        // update UFDI_R2 && RAWI_R2 input
        ptr_out_param->UFDI_R2A_UFDI_XSIZE.Bits.UFDI_XSIZE = ((((unsigned int)(ufd_xe + DUAL_UFO_AU_SIZE - 1) >> DUAL_UFO_AU_BIT) + 7) >> 3) * 8 - 1;

        int bit_per_pixel = 16;
        switch(ptr_in_param->CAMCTL_R1A_CAMCTL_FMT3_SEL.Bits.CAMCTL_RAWI_R2_FMT)
	    {
            case 8:
                bit_per_pixel = 8;
                break;
            case 9:
                bit_per_pixel = 10;
                break;
            case 10:
                bit_per_pixel = 12;
                break;
            case 11:
                bit_per_pixel = 14;
                break;
            case 16:
                bit_per_pixel = 10;
                break;
            default:
                break;
        }
        ptr_out_param->RAWI_R2A_RAWI_XSIZE.Bits.RAWI_XSIZE = ptr_in_param->RAWI_R2A_RAWI_XSIZE.Bits.RAWI_XSIZE;

        // update SEP_R1 STR_X and END_X
        ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X = 0;
        ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X = (((int)ptr_out_param->UFD_R2A_UFD_CROP_CON1.Bits.UFD_X_END - (int)ptr_out_param->UFD_R2A_UFD_CROP_CON1.Bits.UFD_X_START  + 1) >>
            (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN + ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN)) - 1;

        // UFD_R2B
        ufd_xs = (unsigned int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X << (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
                                                                                  ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN);
        ufd_xe = (((unsigned int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X  + 1) << (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN +
                                                                                          ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN)) - 1;
        au_xs  = (unsigned int)ufd_xs >> DUAL_UFO_AU_BIT;
        au_xe  = (unsigned int)ufd_xe >> DUAL_UFO_AU_BIT;

	    /* check UFD_R2B_BS2_AU_START */
	    if (ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START &&
	        ((int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START < au_full_size) &&
	        (1 == ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE))
	    {
		    ptr_out_param->UFD_R2B_UFD_AU_CON.Bits.UFD_AU_OFST = au_xs;
		    ptr_out_param->UFD_R2B_UFD_AU_CON.Bits.UFD_AU_SIZE = au_xe - au_xs + 1;
		    if (au_xe < (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START)
		    {
			    /* AU offset and size */
			    ptr_out_param->UFD_R2B_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE = 0;/* left */
			    ptr_out_param->UFD_R2B_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
			    ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_OFST = 0;
			    ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_SIZE = 0;
			    ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			    ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
		    }
		    else if (au_xs < (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START)
		    {
		        ptr_out_param->UFD_R2B_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE = 1;/* left + middle */
		        if ((1 == ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE) &&
		            (au_xe >= (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START))
		        {
		            ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_OFST = 0;
			        ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_SIZE =  (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START -
			                                                                (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START;
			        ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			        ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_SIZE = au_xe - (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START + 1;
			        ptr_out_param->UFD_R2B_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 1;
		        }
		        else
		        {
		            /* AU offset and size */
			        ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_OFST = 0;
			        ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_SIZE = au_xe - (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START + 1;
			        ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			        ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
			        ptr_out_param->UFD_R2B_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
		        }
		    }
		    else
		    {
		        ptr_out_param->UFD_R2B_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE = 2;
		        if (1 == ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE)
		        {
		            if (au_xs < (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START)
		            {
		                ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_OFST = au_xs - (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START;
		                if (au_xe < (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START)
		                {
			                ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_SIZE = au_xe - au_xs + 1;
			                ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			                ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
			                ptr_out_param->UFD_R2B_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
		                }
		                else
		                {
		                    ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_SIZE = (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START - au_xs;
		                    ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			                ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_SIZE = au_xe - (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START + 1;
			                ptr_out_param->UFD_R2B_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 1;
		                }
		            }
		            else
		            {
		                ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_OFST = au_xs - (int)ptr_in_param->UFD_R2A_UFD_BS3_AU_CON.Bits.UFD_BS3_AU_START;
			            ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_SIZE = au_xe - au_xs + 1;
			            ptr_out_param->UFD_R2B_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 2;
		            }
		        }
		        else
		        {
			        /* AU offset and size */
			        ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_OFST = au_xs - (int)ptr_in_param->UFD_R2A_UFD_BS2_AU_CON.Bits.UFD_BS2_AU_START;
			        ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_SIZE = au_xe - au_xs + 1;
			        ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
			        ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
			        ptr_out_param->UFD_R2B_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
			    }
		    }
	    }
	    else
	    {
		    /* AU offset and size */
		    ptr_out_param->UFD_R2B_UFD_AU_CON.Bits.UFD_AU_OFST  = au_xs;
		    ptr_out_param->UFD_R2B_UFD_AU_CON.Bits.UFD_AU_SIZE  = au_xe - au_xs + 1;
            ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_OFST = 0;
		    ptr_out_param->UFD_R2B_UFD_AU2_CON.Bits.UFD_AU2_SIZE = 0;
		    ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_OFST = 0;
		    ptr_out_param->UFD_R2B_UFD_AU3_CON.Bits.UFD_AU3_SIZE = 0;
		    ptr_out_param->UFD_R2B_UFD_BS2_AU_CON.Bits.UFD_BOND_MODE  = 0;
		    ptr_out_param->UFD_R2B_UFD_BS3_AU_CON.Bits.UFD_BOND2_MODE = 0;
	    }

        /* UFD_R2 crop window */
        ptr_out_param->UFD_R2B_UFD_CROP_CON1.Bits.UFD_X_START = ufd_xs - (((unsigned int)au_xs) << DUAL_UFO_AU_BIT);
        ptr_out_param->UFD_R2B_UFD_CROP_CON1.Bits.UFD_X_END = ufd_xe - (((unsigned int)au_xs) << DUAL_UFO_AU_BIT);
        ptr_out_param->UFD_R2B_UFD_SIZE_CON.Bits.UFD_WD = ufd_xe - (((unsigned int)au_xs) << DUAL_UFO_AU_BIT) + 1;
        ptr_out_param->UFD_R2B_UFD_SIZE_CON.Bits.UFD_HT = ptr_in_param->UFD_R2A_UFD_SIZE_CON.Bits.UFD_HT;
        ptr_out_param->UFD_R2B_UFD_CROP_CON2.Raw = ptr_in_param->UFD_R2A_UFD_CROP_CON2.Raw;
        ptr_out_param->UFD_R2B_UFD_CON.Raw = ptr_in_param->UFD_R2A_UFD_CON.Raw;
        ptr_out_param->UFD_R2B_UFD_ADDRESS_CON1.Raw = ptr_in_param->UFD_R2A_UFD_ADDRESS_CON1.Raw;
        ptr_out_param->UFD_R2B_UFD_ADDRESS_CON2.Raw = ptr_in_param->UFD_R2A_UFD_ADDRESS_CON2.Raw;
        ptr_out_param->UFD_R2B_UFD_ADDRESS_CON3.Raw = ptr_in_param->UFD_R2A_UFD_ADDRESS_CON3.Raw;

        // update UFDI_R2 && RAWI_R2 input
        ptr_out_param->UFDI_R2B_UFDI_XSIZE.Bits.UFDI_XSIZE = ((((unsigned int)(ufd_xe + DUAL_UFO_AU_SIZE - 1) >> DUAL_UFO_AU_BIT) + 7) >> 3) * 8 - 1;
        ptr_out_param->UFDI_R2B_UFDI_YSIZE.Bits.UFDI_YSIZE = ptr_in_param->UFDI_R2A_UFDI_YSIZE.Bits.UFDI_YSIZE;
        ptr_out_param->UFDI_R2B_UFDI_STRIDE.Bits.UFDI_STRIDE = ptr_in_param->UFDI_R2A_UFDI_STRIDE.Bits.UFDI_STRIDE;
        ptr_out_param->RAWI_R2B_RAWI_XSIZE.Bits.RAWI_XSIZE = ptr_in_param->RAWI_R2A_RAWI_XSIZE.Bits.RAWI_XSIZE;
        ptr_out_param->RAWI_R2B_RAWI_YSIZE.Bits.RAWI_YSIZE = ptr_in_param->RAWI_R2A_RAWI_YSIZE.Bits.RAWI_YSIZE;
        ptr_out_param->RAWI_R2B_RAWI_STRIDE.Bits.RAWI_STRIDE = ptr_in_param->RAWI_R2A_RAWI_STRIDE.Bits.RAWI_STRIDE;

        // update SEP_R1 STR_X and END_X
        ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X = 0;
        ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X = (((int)ptr_out_param->UFD_R2B_UFD_CROP_CON1.Bits.UFD_X_END - (int)ptr_out_param->UFD_R2B_UFD_CROP_CON1.Bits.UFD_X_START  + 1) >>
            (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN + ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN)) - 1;

        ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL;
        ptr_out_param->CAMCTL_R1B_CAMCTL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL;
        ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_SEP_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_SEP_SEL;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_DCPN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_DCPN_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN3.Bits.CAMCTL_UFD_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN3.Bits.CAMCTL_UFD_R2_EN;
    }

    return result;
}

static DUAL_MESSAGE_ENUM dual_cal_rawi_rx(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param, DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

	int in_sep_xs;
	int in_sep_xe;

    if (ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL == 2 &&
        ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_UNP_R2_EN &&
		ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL == 0 &&
		ptr_in_param->CAMCTL_R1A_CAMCTL_FMT2_SEL.Bits.CAMCTL_PIX_BUS_RAWI_R2 == 0)
    {
 	    // RAWI_R2A
        in_sep_xs = (unsigned int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_STR_X;
        in_sep_xe = (unsigned int)ptr_out_param->SEP_R1A_SEP_CROP.Bits.SEP_END_X;

 	    int bit_per_pixel = 8;
	    switch(ptr_in_param->CAMCTL_R1A_CAMCTL_FMT3_SEL.Bits.CAMCTL_RAWI_R2_FMT)
	    {
            case 8:
                bit_per_pixel = 8;
                break;
            case 9:
                bit_per_pixel = 10;
                break;
            case 10:
                bit_per_pixel = 12;
                break;
            case 11:
                bit_per_pixel = 14;
                break;
            case 16:
                bit_per_pixel = 10;
                break;
            default:
                break;
        }

        int start_bit_offset = (unsigned int)(in_sep_xs * bit_per_pixel) & 0x0F;  /* remainder of 16 */
        int end_bit_offset   = (unsigned int)(start_bit_offset + (in_sep_xe - in_sep_xs + 1) * bit_per_pixel) & 0x0F;

        // UNP_R2A
        ptr_out_param->UNP_R2A_UNP_OFST.Bits.UNP_OFST_STB = start_bit_offset;
        ptr_out_param->UNP_R2A_UNP_OFST.Bits.UNP_OFST_EDB = end_bit_offset;

        // RAWI_R2A
	    ptr_out_param->RAWI_R2A_RAWI_OFST_ADDR.Bits.RAWI_OFST_ADDR = ((in_sep_xs * bit_per_pixel - start_bit_offset) >> 3);
	    ptr_out_param->RAWI_R2A_RAWI_XSIZE.Bits.RAWI_XSIZE = (((unsigned int)(start_bit_offset + (in_sep_xe - in_sep_xs + 1) * bit_per_pixel + 7) >> 3) - 1) | 0x1;

        // RAWI_R2B
        in_sep_xs = (unsigned int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_STR_X;
        in_sep_xe = (unsigned int)ptr_out_param->SEP_R1B_SEP_CROP.Bits.SEP_END_X;

        start_bit_offset = (unsigned int)(in_sep_xs * bit_per_pixel) & 0x0F;  /* remainder of 16 */
        end_bit_offset   = (unsigned int)(start_bit_offset + (in_sep_xe - in_sep_xs + 1) * bit_per_pixel) & 0x0F;

        // UNP_R2B
        ptr_out_param->UNP_R2B_UNP_OFST.Bits.UNP_OFST_STB = start_bit_offset;
        ptr_out_param->UNP_R2B_UNP_OFST.Bits.UNP_OFST_EDB = end_bit_offset;

        // RAWI_R2B
	    ptr_out_param->RAWI_R2B_RAWI_OFST_ADDR.Bits.RAWI_OFST_ADDR = ((in_sep_xs * bit_per_pixel - start_bit_offset) >> 3);
	    ptr_out_param->RAWI_R2B_RAWI_XSIZE.Bits.RAWI_XSIZE = (((unsigned int)(start_bit_offset + (in_sep_xe - in_sep_xs + 1) * bit_per_pixel + 7) >> 3) - 1) | 0x1;
        ptr_out_param->RAWI_R2B_RAWI_YSIZE.Bits.RAWI_YSIZE = ptr_in_param->RAWI_R2A_RAWI_YSIZE.Bits.RAWI_YSIZE;
        ptr_out_param->RAWI_R2B_RAWI_STRIDE.Bits.RAWI_STRIDE = ptr_in_param->RAWI_R2A_RAWI_STRIDE.Bits.RAWI_STRIDE;

        /* UNP & RAWI */
        ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL;
        ptr_out_param->CAMCTL_R1B_CAMCTL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL2.Bits.CAMCTL_RAWI_UFO_SEL;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN.Bits.CAMCTL_UNP_R2_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN.Bits.CAMCTL_UNP_R2_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_SEL.Bits.CAMCTL_SEP_SEL = ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_SEP_SEL;
	    ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN;
        ptr_out_param->CAMCTL_R1B_CAMCTL_EN4.Bits.CAMCTL_DCPN_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN4.Bits.CAMCTL_DCPN_R1_EN;

    }

	return result;
}

static DUAL_MESSAGE_ENUM dual_cal_dmx_rrz_rmx_af_p2(int dual_mode, const DUAL_IN_CONFIG_STRUCT *ptr_in_param,
									  DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    DUAL_MESSAGE_ENUM result = DUAL_MESSAGE_OK;

    int bin_sel_h_size = ((0 == ptr_in_param->CAMCTL_R1A_CAMCTL_SEL.Bits.CAMCTL_RAW_SEL)?
		                 ((int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_E - (int)ptr_in_param->TG_R1A_TG_SEN_GRAB_PXL.Bits.TG_PXL_S):
		                 ptr_in_param->SW.TWIN_RAWI_XSIZE) >> ((int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_BIN_R1_EN +
		                 (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_DBN_R1_EN + (int)ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_FBND_R1_EN);

	/* following to update */
	int afo_pipe_left_loss_b  = 0;
	int afo_pipe_right_loss_b = 0;
	int afo_pipe_left_margin  = 0;
	int afo_pipe_right_margin = 0;

	int sep_r1_valid_width[2] = { 0, 0 };

	if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN)
	{
		if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
		{
			if (ptr_in_param->AF_R1A_AF_CON2.Bits.AF_DS_EN && ptr_in_param->AF_R1A_AF_CON.Bits.AF_H_GONLY)
			{
				result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}

			afo_pipe_left_loss_b  = DUAL_AF_TAPS * 2;
			afo_pipe_right_loss_b = DUAL_AF_TAPS * 2 + 2;
			afo_pipe_left_margin  = DUAL_AF_LEFT_MARGIN;
			afo_pipe_right_margin = DUAL_AF_RGIHT_MARGIN;
			if ((int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH)
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
		if ((ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE & 0x1) || (ptr_in_param->SW.TWIN_AF_OFFSET & 0x1))
		{
			result = DUAL_MESSAGE_INVALID_CONFIG_ERROR;
			dual_driver_printf("Error: %s\n", print_error_message(result));
			return result;
		}

		/* RTL verif or platform */
		if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
		{
			/* with af offset */
			if (ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE)
			{
				if (bin_sel_h_size < ptr_in_param->SW.TWIN_AF_OFFSET + (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE)
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
		sep_r1_valid_width[0] = (int)ptr_in_param->CRP_R1A_CRP_X_POS.Bits.CRP_XEND + 1 - afo_pipe_right_loss_b - afo_pipe_right_margin;
		sep_r1_valid_width[1] = bin_sel_h_size - sep_r1_valid_width[0];

		ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN;

		if (ptr_in_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_CRP_R1_EN)
		{
			/* RCROP */
			/* RCROP_D */
			/* AF & AF_D */
			/* RTL verif or platform */
			if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
			{
				/* platform */
				if ((ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE != ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE) ||
					((int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE > DUAL_AF_MAX_BLOCK_WIDTH) ||
					((sep_r1_valid_width[0] + sep_r1_valid_width[1]) < ptr_in_param->SW.TWIN_AF_OFFSET + (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE * ptr_in_param->SW.TWIN_AF_BLOCK_XNUM) ||
					(ptr_in_param->SW.TWIN_AF_BLOCK_XNUM <= 0))
				{
					result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}

				/* check AF_A, minimize DMX_A_END_X */
				if (sep_r1_valid_width[0] <= ptr_in_param->SW.TWIN_AF_OFFSET)
				{
					/* B+C only */
					ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = false;
					ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = false;
				}
				else
				{
					if ((sep_r1_valid_width[0] + DUAL_AF_RGIHT_MARGIN) < (ptr_in_param->SW.TWIN_AF_OFFSET + (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE))
					{
						/* B+C only */
						ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = false;
						ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = false;
					}
					else
					{
						/* A+B , cover cross region by afo_pipe_right_margin */
						ptr_out_param->AF_R1A_AF_VLD.Bits.AF_VLD_XSTART = ptr_in_param->SW.TWIN_AF_OFFSET;
						/* check AF_A output all */
						if (((int)ptr_in_param->SW.TWIN_AF_BLOCK_XNUM * (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE + (int)ptr_in_param->SW.TWIN_AF_OFFSET) <= (int)(sep_r1_valid_width[0] + DUAL_AF_RGIHT_MARGIN))
						{
							/* A only */
							ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
						}
						else
						{
							/* B+C, minimize DMX_A_END_X */
							ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = (sep_r1_valid_width[0] + DUAL_AF_RGIHT_MARGIN - ptr_in_param->SW.TWIN_AF_OFFSET) / (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE;
						}
					}
				}
			}
			else
			{
				/* RTL different base addr */
				ptr_out_param->AF_R1A_AF_VLD.Bits.AF_VLD_XSTART = 0;
				if (sep_r1_valid_width[0] < (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE * (int)ptr_in_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM)
				{
					ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = sep_r1_valid_width[0] / (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE;
				}
			}

			if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN)
			{
				/* check max 128 */
				if (ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
				{
					if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						/* RTL */
						ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
					}
				}
			}
			/* RTL verif or platform */
			if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
			{
				if (ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN)
				{
				    /* A+B */
					if (ptr_in_param->SW.TWIN_AF_BLOCK_XNUM > (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM)
					{
						/* A+B */
						ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_XSTART = ptr_in_param->SW.TWIN_AF_OFFSET +
							(int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM * (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE -
							 (sep_r1_valid_width[0] - afo_pipe_left_loss_b - afo_pipe_left_margin);
						ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM -
							(int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM;
					}
					else
					{
						/* A only */
						ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = false;
						ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = false;
					}
				}
				else
				{
				    /* B only */
					ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_XSTART = ptr_in_param->SW.TWIN_AF_OFFSET - (sep_r1_valid_width[0] - afo_pipe_left_loss_b - afo_pipe_left_margin);
					ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
				}
		        ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_YSTART = ptr_in_param->AF_R1A_AF_VLD.Bits.AF_VLD_YSTART;
				ptr_out_param->AF_R1B_AF_CON.Raw = ptr_in_param->AF_R1A_AF_CON.Raw;
				ptr_out_param->AF_R1B_AF_CON2.Raw = ptr_in_param->AF_R1A_AF_CON2.Raw;
				ptr_out_param->AF_R1B_AF_BLK_0.Raw = ptr_in_param->AF_R1A_AF_BLK_0.Raw;
				ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_YNUM = ptr_in_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_YNUM;
				ptr_out_param->AF_R1B_AF_BLK_PROT.Raw = ptr_in_param->AF_R1A_AF_BLK_PROT.Raw;
				ptr_out_param->AFO_R1B_AFO_DRS.Raw = ptr_in_param->AFO_R1A_AFO_DRS.Raw;
				ptr_out_param->AFO_R1B_AFO_CON.Raw = ptr_in_param->AFO_R1A_AFO_CON.Raw;
				ptr_out_param->AFO_R1B_AFO_CON2.Raw = ptr_in_param->AFO_R1A_AFO_CON2.Raw;
				ptr_out_param->AFO_R1B_AFO_CON3.Raw = ptr_in_param->AFO_R1A_AFO_CON3.Raw;
				ptr_out_param->AFO_R1B_AFO_CON4.Raw = ptr_in_param->AFO_R1A_AFO_CON4.Raw;
			}
			else
			{
				/* RTL, diff base_addr */
				ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_XSTART = 0;
				if (sep_r1_valid_width[1] < (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE * (int)ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM)
				{
					ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = sep_r1_valid_width[1] / (int)ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE;
				}
			}
			if (ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN)
			{
				/* check max 128 */
				if (ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM > 128)
				{
					if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_NUM_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						/* RTL */
						ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = 128;
					}
				}
			}
			if (ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE < DUAL_AF_BLOCK_BYTE)
			{
				result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
				dual_driver_printf("Error: %s\n", print_error_message(result));
				return result;
			}

			if (ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN)
			{
				ptr_out_param->AFO_R1A_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR = 0;
				ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE = (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM * DUAL_AF_BLOCK_BYTE - 1;

				/* check stride */
				if ((unsigned int)ptr_out_param->AFO_R1A_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR + (unsigned int)ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE + 1 >
					(unsigned int)ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE)
				{
					if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						/* RTL */
						ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE -
							(unsigned int)ptr_out_param->AFO_R1A_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR) / DUAL_AF_BLOCK_BYTE;
						ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE = DUAL_AF_BLOCK_BYTE * (int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM - 1;
					}
				}
			}

			if (ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN)
			{
				ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE = (int)ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM * DUAL_AF_BLOCK_BYTE - 1;
				if ((int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE < DUAL_AF_BLOCK_BYTE)
				{
					result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
					dual_driver_printf("Error: %s\n", print_error_message(result));
					return result;
				}

				/* RTL verif or platform */
				if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
				{
					ptr_out_param->AFO_R1B_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR = ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN?
						((int)ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM * DUAL_AF_BLOCK_BYTE):0;
					ptr_out_param->AFO_R1B_AFO_YSIZE.Raw = ptr_in_param->AFO_R1A_AFO_YSIZE.Raw;
				}
				if ((unsigned int)ptr_out_param->AFO_R1B_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR + (unsigned int)ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE + 1 >
					(unsigned int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE)
				{
					if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
					else
					{
						/* RTL */
						ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = ((unsigned int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE -
							(unsigned int)ptr_out_param->AFO_R1B_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR) / DUAL_AF_BLOCK_BYTE;
						ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE = DUAL_AF_BLOCK_BYTE * (int)ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM  - 1;
					}
				}
				if ((ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN? ((unsigned int)ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE + 1): 0) +
					(unsigned int)ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE + 1 > (unsigned int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE)
				{
					if (ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR)
					{
						result = DUAL_MESSAGE_INVALID_AFO_CONFIG_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
			}

			/* AFO_A padding */
			if ((false == ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN) &&
				(ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR))
			{
				if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1) * DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->AFO_R1A_AFO_STRIDE.Bits.AFO_STRIDE)
				{
					ptr_out_param->AF_R1A_AF_VLD.Bits.AF_VLD_XSTART = 0;
					ptr_out_param->AF_R1A_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
					ptr_out_param->AFO_R1A_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR = DUAL_AF_BLOCK_BYTE * ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
					ptr_out_param->AFO_R1A_AFO_XSIZE.Bits.AFO_XSIZE = DUAL_AF_BLOCK_BYTE - 1;
					ptr_out_param->CAMCTL_R1A_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = true;
					ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = true;
				}
			}
			if (ptr_out_param->CAMCTL_R1A_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN)
			{
				/* error check A */
				if ((3 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_GONLY))
				{
					if (ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				else if ((3 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_AVG_LVL) ||
					     ((2 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_AVG_LVL) &&
					      (1 == ptr_in_param->AF_R1A_AF_CON.Bits.AF_V_GONLY)))
				{
					if (ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				else
				{
					if (ptr_in_param->AF_R1A_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
			}
			/* AFO_B padding */
			if ((false == ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN) &&
				(ptr_in_param->AFO_R1A_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR == ptr_in_param->AFO_R1B_AFO_BASE_ADDR.Bits.AFO_BASE_ADDR))
			{
				if ((ptr_in_param->SW.TWIN_AF_BLOCK_XNUM + 1) * DUAL_AF_BLOCK_BYTE <= (int)ptr_in_param->AFO_R1B_AFO_STRIDE.Bits.AFO_STRIDE)
				{
		 			ptr_out_param->AF_R1B_AF_VLD.Bits.AF_VLD_XSTART= 0;
					ptr_out_param->AF_R1B_AF_BLK_1.Bits.AF_BLK_XNUM = 1;
					ptr_out_param->AFO_R1B_AFO_OFST_ADDR.Bits.AFO_OFST_ADDR = DUAL_AF_BLOCK_BYTE * ptr_in_param->SW.TWIN_AF_BLOCK_XNUM;
					ptr_out_param->AFO_R1B_AFO_XSIZE.Bits.AFO_XSIZE = DUAL_AF_BLOCK_BYTE - 1;
					ptr_out_param->CAMCTL_R1B_CAMCTL_EN2.Bits.CAMCTL_AF_R1_EN = true;
					ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN = true;
				}
			}
			if (ptr_out_param->CAMCTL_R1B_CAMCTL_DMA_EN.Bits.CAMCTL_AFO_R1_EN)
			{
				/* error check B */
				if ((3 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_AVG_LVL) && (1 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_GONLY))
				{
					if (ptr_out_param->AF_R1B_AF_BLK_0.Bits.AF_BLK_XSIZE < 32)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				else if ((3 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_AVG_LVL) ||
					     ((2 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_AVG_LVL) &&
					      (1 == ptr_out_param->AF_R1B_AF_CON.Bits.AF_V_GONLY)))
				{
					if (ptr_out_param->AF_R1B_AF_BLK_0.Bits.AF_BLK_XSIZE < 16)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
				else
				{
					if (ptr_out_param->AF_R1B_AF_BLK_0.Bits.AF_BLK_XSIZE < 8)
					{
						result = DUAL_MESSAGE_INVALID_AF_BLK_XSIZE_ERROR;
						dual_driver_printf("Error: %s\n", print_error_message(result));
						return result;
					}
				}
			}
			/* sync RCP size & AF_IMAGE_WD */
			ptr_out_param->AF_R1A_AF_SIZE.Bits.AF_IMAGE_WD = sep_r1_valid_width[0] + afo_pipe_right_loss_b + afo_pipe_right_margin;
			ptr_out_param->AF_R1B_AF_SIZE.Bits.AF_IMAGE_WD = afo_pipe_left_loss_b + afo_pipe_left_margin + sep_r1_valid_width[1];
		}
	}
	return result;
}

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
                /* keep binning fake SEP half size */
                result = dual_cal_sep_rrz_rmx(dual_mode, ptr_in_param, ptr_out_param);
                if (DUAL_MESSAGE_OK == result)
				{
				    result = dual_cal_slk_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
				    result = dual_cal_slk_r2(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
				    result = dual_cal_flk_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
                if (DUAL_MESSAGE_OK == result)
                {
					result = dual_cal_qbin_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					result = dual_cal_qbin_r2(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					result = dual_cal_qbin_r3(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					result = dual_cal_qbin_r4(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					result = dual_cal_qbin_r5(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
				    result = dual_cal_ltm_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_hlr_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_wb_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
				    result = dual_cal_lsc_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_dgn_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_rnr_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_fus_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
				    result = dual_cal_bpc_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
				{
				    result = dual_cal_bpc_r2(dual_mode, ptr_in_param, ptr_out_param);
				}
                if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_fbnd_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_irm_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_iobc_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
				if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_ibpc_r1(dual_mode, ptr_in_param, ptr_out_param);
				}
                if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_ufd_r2(dual_mode, ptr_in_param, ptr_out_param);
				}
                if (DUAL_MESSAGE_OK == result)
                {
					/* restore binning SEP real size finally */
					result = dual_cal_rawi_rx(dual_mode, ptr_in_param, ptr_out_param);
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

bool dual_sel_check_support(int dual_sel)
{
    bool found_flag = false;
    DUAL_SEL_SUPPORT_LIST(DUAL_SEL_CHECK, dual_sel, found_flag,,,,);
    return found_flag;
}