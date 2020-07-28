#include "MyUtils.h"

#define LOG_TAG "DualCal"
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{Dual_cal} "
#include "inc/drv_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(dual_cal);
//EXTERN_DBG_LOG_VARIABLE(twin_drv);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (dual_cal_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (dual_cal_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (dual_cal_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (dual_cal_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (dual_cal_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (dual_cal_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


/* define to replace macro */
#define print_error_message(...) dual_cal_print_error_message(__VA_ARGS__)
#define get_current_file_name(...) dual_cal_get_current_file_name(__VA_ARGS__)
/* func prototype */
static const char *dual_cal_print_error_message(ISP_MESSAGE_ENUM n);
static const char *dual_cal_get_current_file_name(const char *filename);
static ISP_MESSAGE_ENUM dual_init_param(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_dmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_dbs(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_obc(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_bnr(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_lsc(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_bmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_rpg(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_rrz(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
static ISP_MESSAGE_ENUM dual_cal_rmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param);
/* bits field will always calculate by unsigned int */

ISP_MESSAGE_ENUM dual_cal_main(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param,
							   ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param && ptr_out_param)
	{
		result = dual_init_param(ptr_in_param, ptr_out_param);
		if (ISP_MESSAGE_OK == result)
		{
			result = dual_cal_dmx(ptr_in_param, ptr_out_param);
		}
		if (ISP_MESSAGE_OK == result)
		{
			result = dual_cal_dbs(ptr_in_param, ptr_out_param);
		}
		if (ISP_MESSAGE_OK == result)
		{
			result = dual_cal_obc(ptr_in_param, ptr_out_param);
		}
		if (ISP_MESSAGE_OK == result)
		{
			result = dual_cal_bnr(ptr_in_param, ptr_out_param);
		}
		if (ISP_MESSAGE_OK == result)
		{
			result = dual_cal_lsc(ptr_in_param, ptr_out_param);
		}
		if (ISP_MESSAGE_OK == result)
		{
			result = dual_cal_bmx(ptr_in_param, ptr_out_param);
		}
		if (ISP_MESSAGE_OK == result)
		{
			result = dual_cal_rpg(ptr_in_param, ptr_out_param);
		}
		if (ISP_MESSAGE_OK == result)
		{
			result = dual_cal_rrz(ptr_in_param, ptr_out_param);
		}
		if (ISP_MESSAGE_OK == result)
		{
			result = dual_cal_rmx(ptr_in_param, ptr_out_param);
		}
	}
	else
	{
		result = ISP_MESSAGE_NULL_PTR_ERROR;
	}
	return result;
}

static ISP_MESSAGE_ENUM dual_init_param(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		/* copy default RW in to out */
		//DUAL_HW_LUT(DUAL_COPY_RW_REG, ptr_out_param, ptr_in_param,,,,);
                DUAL_IN_OUT_STRUCT_LUT_HW(DUAL_IN_OUT_COPY, ptr_out_param, ptr_in_param,,,,);
        }
	return result;
}

static ISP_MESSAGE_ENUM dual_cal_dmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		int bin_sel_h_size = (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_E - (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_S;
		ptr_out_param->CAM_DMX_CTL.Bits.DMX_EDGE = 0xF;
		/* left side */
		ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X = 0;
		ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X = ((unsigned int)bin_sel_h_size>>1) + (((unsigned int)bin_sel_h_size>>1) & 0x1) +
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0) +
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) +
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN?TILE_LOSS_RRZ:0) - 1;
		/* right side */
		ptr_out_param->CAM_CTL_EN_P1_D.Bits.DMX_D_EN = 1;
		ptr_out_param->CAM_DMX_D_CTL.Bits.DMX_EDGE = 0xF;
		ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X =  ((unsigned int)bin_sel_h_size>>1) + (((unsigned int)bin_sel_h_size>>1) & 0x1) -
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0) -
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) -			
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN?TILE_LOSS_RRZ_L:0);
		ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X = bin_sel_h_size - 1;
		DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DMX_VSIZE.Raw, CAM_DMX_D_VSIZE.Raw);
	}
	return result;
}

static ISP_MESSAGE_ENUM dual_cal_dbs(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		if (ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN)
		{
			/* update right by left */
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_SIGMA.Raw, CAM_DBS_D_SIGMA.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_BSTBL_0.Raw, CAM_DBS_D_BSTBL_0.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_BSTBL_1.Raw, CAM_DBS_D_BSTBL_1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_BSTBL_2.Raw, CAM_DBS_D_BSTBL_2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_BSTBL_3.Raw, CAM_DBS_D_BSTBL_3.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_CTL.Raw, CAM_DBS_D_CTL.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_CTL_2.Raw, CAM_DBS_D_CTL_2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_SIGMA_2.Raw, CAM_DBS_D_SIGMA_2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_YGN.Raw, CAM_DBS_D_YGN.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_SL_Y12.Raw, CAM_DBS_D_SL_Y12.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_SL_Y34.Raw, CAM_DBS_D_SL_Y34.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_SL_G12.Raw, CAM_DBS_D_SL_G12.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_DBS_SL_G34.Raw, CAM_DBS_D_SL_G34.Raw);
		}
		DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.DBS_EN, CAM_CTL_EN_P1_D.Bits.DBS_D_EN);
	}
	return result;
}

static ISP_MESSAGE_ENUM dual_cal_obc(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		if (ptr_in_param->CAM_CTL_EN_P1.Bits.OB_EN)
		{
			/* update right by left */
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_OFFST0.Raw, CAM_OBC_D_OFFST0.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_OFFST1.Raw, CAM_OBC_D_OFFST1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_OFFST2.Raw, CAM_OBC_D_OFFST2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_OFFST3.Raw, CAM_OBC_D_OFFST3.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_GAIN0.Raw, CAM_OBC_D_GAIN0.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_GAIN1.Raw, CAM_OBC_D_GAIN1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_GAIN2.Raw, CAM_OBC_D_GAIN2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_OBC_GAIN3.Raw, CAM_OBC_D_GAIN3.Raw);
		}
		DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.OB_EN, CAM_CTL_EN_P1_D.Bits.OB_D_EN);
	}
	return result;
}

static ISP_MESSAGE_ENUM dual_cal_bnr(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		if (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN)
		{
			int bin_sel_h_size = (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_E - (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_S;
			int dmx_h_size = (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X - (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
			int dmx_v_size = (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_LIN.Bits.LIN_E - (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_LIN.Bits.LIN_S;
			int dmx_d_h_size = (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X - (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
			/* left side */
			ptr_out_param->CAM_BNR_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_BNR_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
			ptr_out_param->CAM_BNR_BPC_TBLI2.Bits.BPC_XSIZE = dmx_h_size - 1;
			ptr_out_param->CAM_BNR_BPC_TBLI2.Bits.BPC_YSIZE = dmx_v_size - 1;
			ptr_out_param->CAM_BNR_PDC_POS.Bits.PDC_XCENTER = (unsigned int)bin_sel_h_size>>1;
			ptr_out_param->CAM_BNR_PDC_POS.Bits.PDC_YCENTER = (unsigned int)dmx_v_size>>1;
			/* right side */
			ptr_out_param->CAM_BNR_D_BPC_TBLI1.Bits.BPC_XOFFSET = ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X;
			ptr_out_param->CAM_BNR_D_BPC_TBLI1.Bits.BPC_YOFFSET = 0;
			ptr_out_param->CAM_BNR_D_BPC_TBLI2.Bits.BPC_XSIZE = dmx_d_h_size - 1;
			ptr_out_param->CAM_BNR_D_BPC_TBLI2.Bits.BPC_YSIZE = dmx_v_size - 1;
			ptr_out_param->CAM_BNR_D_PDC_POS.Bits.PDC_XCENTER = (unsigned int)bin_sel_h_size>>1;
			ptr_out_param->CAM_BNR_D_PDC_POS.Bits.PDC_YCENTER = (unsigned int)dmx_v_size>>1;
			/* copy */
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_CON.Raw, CAM_BNR_D_BPC_CON.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_TH1.Raw, CAM_BNR_D_BPC_TH1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_TH2.Raw, CAM_BNR_D_BPC_TH2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_TH3.Raw, CAM_BNR_D_BPC_TH3.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_TH4.Raw, CAM_BNR_D_BPC_TH4.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_DTC.Raw, CAM_BNR_D_BPC_DTC.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_COR.Raw, CAM_BNR_D_BPC_COR.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_TH1_C.Raw, CAM_BNR_D_BPC_TH1_C.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_TH2_C.Raw, CAM_BNR_D_BPC_TH2_C.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_BPC_TH3_C.Raw, CAM_BNR_D_BPC_TH3_C.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_NR1_CON.Raw, CAM_BNR_D_NR1_CON.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_NR1_CT_CON.Raw, CAM_BNR_D_NR1_CT_CON.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_RSV1.Raw, CAM_BNR_D_RSV1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_RSV2.Raw, CAM_BNR_D_RSV2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_CON.Raw, CAM_BNR_D_PDC_CON.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_L0.Raw, CAM_BNR_D_PDC_GAIN_L0.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_L1.Raw, CAM_BNR_D_PDC_GAIN_L1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_L2.Raw, CAM_BNR_D_PDC_GAIN_L2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_L3.Raw, CAM_BNR_D_PDC_GAIN_L3.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_L4.Raw, CAM_BNR_D_PDC_GAIN_L4.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_R0.Raw, CAM_BNR_D_PDC_GAIN_R0.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_R1.Raw, CAM_BNR_D_PDC_GAIN_R1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_R2.Raw, CAM_BNR_D_PDC_GAIN_R2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_R3.Raw, CAM_BNR_D_PDC_GAIN_R3.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_GAIN_R4.Raw, CAM_BNR_D_PDC_GAIN_R4.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_TH_GB.Raw, CAM_BNR_D_PDC_TH_GB.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_TH_IA.Raw, CAM_BNR_D_PDC_TH_IA.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_TH_HD.Raw, CAM_BNR_D_PDC_TH_HD.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BNR_PDC_SL.Raw, CAM_BNR_D_PDC_SL.Raw);
			/* assign common config, BPCI */
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_OFST_ADDR.Raw, CAM_BPCI_D_OFST_ADDR.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_XSIZE.Raw, CAM_BPCI_D_XSIZE.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_YSIZE.Raw, CAM_BPCI_D_YSIZE.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_STRIDE.Raw, CAM_BPCI_D_STRIDE.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON.Raw, CAM_BPCI_D_CON.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_CON2.Raw, CAM_BPCI_D_CON2.Raw);
		}
		else
		{
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_BPCI_XSIZE.Raw, CAM_BPCI_D_XSIZE.Raw);
		}
		DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.BNR_EN, CAM_CTL_EN_P1_D.Bits.BNR_D_EN);
	}
	return result;
}

static ISP_MESSAGE_ENUM dual_cal_lsc(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		if (ptr_in_param->CAM_CTL_EN_P1.Bits.LSC_EN)
		{
			int blkx_start, blkx_end;
			int bit_per_pix = 512;
			/* left side */
			blkx_start = (ptr_in_param->SW.TWIN_LSC_CROP_OFFX + (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X)/
				(2*(unsigned int)ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH);
			if (blkx_start >= ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
			{
				blkx_start = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL;
			}
			blkx_end = (ptr_in_param->SW.TWIN_LSC_CROP_OFFX + (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X)/
				(2*(unsigned int)ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) + 1;
			ptr_out_param->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
			if (blkx_end > ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
			{
				blkx_end = (unsigned int)ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL + 1;
				ptr_out_param->CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lWIDTH = ptr_in_param->SW.TWIN_MODE_SDBLK_lWIDTH_ALL;
			}
			ptr_out_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_XNUM = blkx_end - blkx_start - 1;
			ptr_out_param->CAM_LSC_TPIPE_OFST.Bits.LSC_TPIPE_OFST_X =
				(unsigned int)(ptr_in_param->SW.TWIN_LSC_CROP_OFFX + (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X -
				blkx_start*2*(unsigned int)ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH)>>1;
			ptr_out_param->CAM_LSC_TPIPE_SIZE.Bits.LSC_TPIPE_SIZE_X = (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X -
				(unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
			ptr_out_param->CAM_LSCI_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)(blkx_start*bit_per_pix)>>3;
			ptr_out_param->CAM_LSCI_XSIZE.Bits.XSIZE = ((unsigned int)((blkx_end - blkx_start)*bit_per_pix)>>3)-1;

			/* right side */
			blkx_start = (ptr_in_param->SW.TWIN_LSC_CROP_OFFX + (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X)/
				(2*(unsigned int)ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH);
			if (blkx_start >= ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
			{
				blkx_start = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL;
			}
			blkx_end = (ptr_in_param->SW.TWIN_LSC_CROP_OFFX + (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X)/
				(2*(unsigned int)ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH) + 1;
			ptr_out_param->CAM_LSC_D_LBLOCK.Bits.LSC_D_SDBLK_lWIDTH = ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH;
			if (blkx_end > ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL)
			{
				blkx_end = ptr_in_param->SW.TWIN_MODE_SDBLK_XNUM_ALL + 1;
				ptr_out_param->CAM_LSC_D_LBLOCK.Bits.LSC_D_SDBLK_lWIDTH = ptr_in_param->SW.TWIN_MODE_SDBLK_lWIDTH_ALL;
			}
			ptr_out_param->CAM_LSC_D_CTL2.Bits.LSC_D_SDBLK_XNUM = blkx_end - blkx_start - 1;
			ptr_out_param->CAM_LSC_D_TPIPE_OFST.Bits.LSC_D_TPIPE_OFST_X =
				(unsigned int)(ptr_in_param->SW.TWIN_LSC_CROP_OFFX + (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X -
				blkx_start*2*(unsigned int)ptr_in_param->CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH)>>1;
			ptr_out_param->CAM_LSC_D_TPIPE_SIZE.Bits.LSC_D_TPIPE_SIZE_X =
				(unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X - (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
			ptr_out_param->CAM_LSCI_D_OFST_ADDR.Bits.OFFSET_ADDR = (unsigned int)(blkx_start*bit_per_pix)>>3;
			ptr_out_param->CAM_LSCI_D_XSIZE.Bits.XSIZE = ((unsigned int)((blkx_end - blkx_start)*bit_per_pix)>>3)-1;
			/* assign common config, LSCI */
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_YSIZE.Raw, CAM_LSCI_D_YSIZE.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_STRIDE.Raw, CAM_LSCI_D_STRIDE.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON.Raw, CAM_LSCI_D_CON.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_CON2.Raw, CAM_LSCI_D_CON2.Raw);
			/* assign common config, LSC */
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL1.Raw, CAM_LSC_D_CTL1.Raw);
			/* only partial update */
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL2.Bits.LSC_SDBLK_WIDTH, CAM_LSC_D_CTL2.Bits.LSC_D_SDBLK_WIDTH);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL2.Bits.LSC_OFLN, CAM_LSC_D_CTL2.Bits.LSC_D_OFLN);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_CTL3.Raw, CAM_LSC_D_CTL3.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_LBLOCK.Bits.LSC_SDBLK_lHEIGHT, CAM_LSC_D_LBLOCK.Bits.LSC_D_SDBLK_lHEIGHT);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_RATIO.Raw, CAM_LSC_D_RATIO.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_TPIPE_OFST.Bits.LSC_TPIPE_OFST_Y, CAM_LSC_D_TPIPE_OFST.Bits.LSC_D_TPIPE_OFST_Y);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_TPIPE_SIZE.Bits.LSC_TPIPE_SIZE_Y, CAM_LSC_D_TPIPE_SIZE.Bits.LSC_D_TPIPE_SIZE_Y);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSC_GAIN_TH.Raw, CAM_LSC_D_GAIN_TH.Raw);
		}
		else
		{
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_LSCI_XSIZE.Raw, CAM_LSCI_D_XSIZE.Raw);
		}
		DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.LSC_EN, CAM_CTL_EN_P1_D.Bits.LSC_D_EN);	
	}
	return result;
}

static ISP_MESSAGE_ENUM dual_cal_bmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		int dmx_h_size = (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X - (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
		int dmx_d_h_size = (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X - (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
		/* left side */
		ptr_out_param->CAM_BMX_CTL.Bits.BMX_EDGE = 0xF;
		ptr_out_param->CAM_BMX_CROP.Bits.BMX_STR_X = 0;
		ptr_out_param->CAM_BMX_CROP.Bits.BMX_END_X = dmx_h_size - (unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0) -
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) -
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN?TILE_LOSS_RRZ:0) - 1;
		ptr_out_param->CAM_BMX_VSIZE.Bits.BMX_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
		/* right side */
		ptr_out_param->CAM_CTL_EN_P1_D.Bits.BMX_D_EN = 1;
		ptr_out_param->CAM_BMX_D_CTL.Bits.BMX_EDGE = 0xF;
		ptr_out_param->CAM_BMX_D_CROP.Bits.BMX_STR_X = (unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN?TILE_LOSS_RRZ_L:0) +
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) +
			(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0);
		ptr_out_param->CAM_BMX_D_CROP.Bits.BMX_END_X = dmx_d_h_size - 1;
		ptr_out_param->CAM_BMX_D_VSIZE.Bits.BMX_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
	}
	return result;
}

static ISP_MESSAGE_ENUM dual_cal_rpg(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		if (ptr_in_param->CAM_CTL_EN_P1.Bits.RPG_EN)
		{
			/* assign common config, RPG */
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_SATU_1.Raw, CAM_RPG_D_SATU_1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_SATU_2.Raw, CAM_RPG_D_SATU_2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_GAIN_1.Raw, CAM_RPG_D_GAIN_1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_GAIN_2.Raw, CAM_RPG_D_GAIN_2.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_OFST_1.Raw, CAM_RPG_D_OFST_1.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RPG_OFST_2.Raw, CAM_RPG_D_OFST_2.Raw);
		}
		DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_CTL_EN_P1.Bits.RPG_EN, CAM_CTL_EN_P1_D.Bits.RPG_D_EN);	
	}
	return result;
}

static ISP_MESSAGE_ENUM dual_cal_rrz(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		if (ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN)
		{
			if ((ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST > 1 + ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST)
				|| (ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST+ 1 < ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST))
            {
			    int rrz_in_w = (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_E - (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_S;
			    bool RMX_disable = false;
			    bool RMX_D_disable = false;
				if ((rrz_in_w < 8) || (ptr_in_param->SW.TWIN_RRZ_OUT_WD < 8) ||
					(rrz_in_w < 8 + ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST + ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST) ||
					(ptr_in_param->SW.TWIN_RRZ_OUT_WD > DUAL_RRZ_WD_ALL))
				{
					result = ISP_MESSAGE_INVALID_CONFIG_ERROR;
					LOG_ERR("Error: %s\n", print_error_message(result));
					return result;
				}
			    /* left side */
			    ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = (long long)(rrz_in_w - 1 -
				    ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST)*
				    DUAL_RRZ_PREC/((unsigned int)ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1);
			    if (ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP < DUAL_RRZ_PREC)
			    {
				    result = ISP_MESSAGE_RRZ_SCALING_UP_ERROR;
				    LOG_ERR("Error: %s\n", print_error_message(result));
				    return result;
			    }
			    ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD = (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X -
				    (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
			    /* check RMX or RMX_D disabled */
			    if (2*ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST > rrz_in_w)
			    {
				    /* RMX disabled */
				    RMX_disable = true;
				    ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = 4;/* min RRZ_OUT size */
				    ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
				    ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
				    /* RMX_D full out */
				    ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
			    }
			    else if (rrz_in_w < 2*ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST)
			    {
				    /* RMX full out */
				    ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
				    ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST;
				    ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
				    /* RMX_D disabled */
				    RMX_D_disable = true;
				    ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = 4;/* min RRZ_D_OUT size */
				    ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
				    ptr_out_param->CAM_RRZ_D_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
			    }
			    else
			    {
				    long long end_temp = (long long)((unsigned int)ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD - 1 -
					    (unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0) -
					    (unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) - TILE_LOSS_RRZ_TAP -
					    ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST)*DUAL_RRZ_PREC - ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
				    int n = (int)(end_temp/ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP);
				    if ((long long)n*ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP == end_temp)
				    {
					    n -= 1;
				    }
				    if (n < 4)
				    {
					    /* RMX disabled */
					    RMX_disable = true;
					    ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = 4;/* min RRZ_OUT size */
					    ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
					    ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
					    /* RMX_D full out */
					    ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
				    }
				    else
				    {
						/* only x2 align */
						if (n & 01)
						{
							n = ((unsigned int)n>>1)<<1;
						}
						/* RMX enabled */
					    ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = n;
					    ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST;
					    ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
					    if (ptr_in_param->SW.TWIN_RRZ_OUT_WD > (int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD)
					    {
						    ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD -
							    (unsigned int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
					    }
					    else
					    {
						    ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD;
						    /* RMX_D disabled */
						    RMX_D_disable = true;
						    ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = 4;/* min RRZ_D_OUT size */
						    ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = 0;
						    ptr_out_param->CAM_RRZ_D_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = 0;
					    }
				    }
				    if (false == RMX_disable)
				    {
					    /* check xe of left RRZ is valid */
					    if ((long long)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X*DUAL_RRZ_PREC <=
						    (long long)((unsigned int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1)*
						    ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP +
						    (long long)(TILE_LOSS_RRZ_TAP + (ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0) +
						    (ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) +
							(unsigned int)ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST)*DUAL_RRZ_PREC +
						    (long long)ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST)
					    {
						    result = ISP_MESSAGE_RRZ_XE_CAL_ERROR;
						    LOG_ERR("Error: %s\n", print_error_message(result));
						    return result;
					    }
				    }
			    }
			    /* right side */
			    ptr_out_param->CAM_RRZ_D_HORI_STEP.Bits.RRZ_HORI_STEP = ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
			    ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_WD = (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X -
				    (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
			    if (false == RMX_D_disable)
			    {
				    /* RMX_D enabled */
				    long long start_offset;
				    if (false == RMX_disable)/* RMX enabled */
				    {
					    /* RMX enabled */
					    start_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
						    (long long)ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST +
						    (long long)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP -
						    (long long)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X*DUAL_RRZ_PREC;
						/* check xs of right RRZ is valid */
					    if ((long long)((unsigned int)TILE_LOSS_RRZ_TAP_L + (unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0) +
						    (unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0))*DUAL_RRZ_PREC > start_offset)
					    {
						    result = ISP_MESSAGE_RRZ_D_XS_CAL_ERROR;
						    LOG_ERR("Error: %s\n", print_error_message(result));
						    return result;
					    }
				    }
				    else
				    {
					    /* RMX disabled */
					    start_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
						    (long long)ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST -
						    (long long)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X*DUAL_RRZ_PREC;
					    /* check xs of right RRZ is valid */
					    if ((long long)((unsigned int)TILE_LOSS_RRZ_TAP_L + (unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0) +
						    (unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0))*DUAL_RRZ_PREC > start_offset)
					    {
						    result = ISP_MESSAGE_RRZ_D_XS_CAL_ERROR;
						    LOG_ERR("Error: %s\n", print_error_message(result));
						    return result;
					    }
				    }
				    ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)start_offset>>15);
				    ptr_out_param->CAM_RRZ_D_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(start_offset -
					    (long long)ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
			    }
                            /* check disable */
                            /*//mark, unreached
			    if (RMX_disable && RMX_D_disable)
			    {
				    result = ISP_MESSAGE_RRZ_DISABLE_ERROR;
				    LOG_ERR("Error: %s\n", print_error_message(result));
				    return result;
			    }
                            else*/
                            {
			        ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = RMX_D_disable?1:0;
			        ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = RMX_disable?1:0;
                            }
			    /* copy from RRZ_IN_HT & RRZ_D_IN_HT from DMX & DMX_D */
			    ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
			    ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
			    /* cal vertical step */
			    if (ptr_in_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP)
			    {
				    /* RTL just copy */
				    DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_STEP.Raw, CAM_RRZ_VERT_STEP.Raw);
				    DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_STEP.Raw, CAM_RRZ_D_VERT_STEP.Raw);
			    }
			    else
			    {
				    if (ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT <= 1)
				    {
					    result = ISP_MESSAGE_INVALID_RRZ_IN_CROP_HT_ERROR;
					    LOG_ERR("Error: %s\n", print_error_message(result));
					    return result;
				    }
				    else
				    {
					    /* re-cal RRZ_VERT_STEP */
					    ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = (long long)(ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT - 1)*
						    DUAL_RRZ_PREC/((unsigned int)ptr_in_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT - 1);
					    if (ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP < DUAL_RRZ_PREC)
					    {
						    result = ISP_MESSAGE_RRZ_SCALING_UP_ERROR;
						    LOG_ERR("Error: %s\n", print_error_message(result));
						    return result;
					    }
					    /* copy RRZ_D_VERT_STEP by RRZ_VERT_STEP */
					    ptr_out_param->CAM_RRZ_D_VERT_STEP.Bits.RRZ_VERT_STEP = ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP;
				    }
                }
            }
            else
            {
			    int rrz_in_w = (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_E - (unsigned int)ptr_in_param->CAM_TG_SEN_GRAB_PXL.Bits.PXL_S;
			    long long temp_offset;
				if ((rrz_in_w < 8) || (ptr_in_param->SW.TWIN_RRZ_OUT_WD < 8) ||
					(rrz_in_w < 8 + ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST + ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST) ||
					(ptr_in_param->SW.TWIN_RRZ_OUT_WD > DUAL_RRZ_WD_ALL))
				{
					result = ISP_MESSAGE_INVALID_CONFIG_ERROR;
					LOG_ERR("Error: %s\n", print_error_message(result));
					return result;
				}
			    /* left side */
			    ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP = (long long)(rrz_in_w - 1 -
				    ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST - ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST_LAST)*
				    DUAL_RRZ_PREC/(ptr_in_param->SW.TWIN_RRZ_OUT_WD - 1);
			    if (ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP < DUAL_RRZ_PREC)
			    {
				    result = ISP_MESSAGE_RRZ_SCALING_UP_ERROR;
				    LOG_ERR("Error: %s\n", print_error_message(result));
				    return result;
			    }
			    ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD = (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X -
				    (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
				/* only x2 align */
			    ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = (((unsigned int)ptr_in_param->SW.TWIN_RRZ_OUT_WD + 3)>>2)<<1;
			    ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST;
			    ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST;
			    /* check xe of left RRZ is valid */
			    if ((long long)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X*DUAL_RRZ_PREC <=
				    (long long)(ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1)*
				    ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP +
				    (long long)(TILE_LOSS_RRZ_TAP + (ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0) +
					(ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0) +
				    (unsigned int)ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST)*DUAL_RRZ_PREC +
				    (long long)ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST)
			    {
				    result = ISP_MESSAGE_RRZ_LEFT_SHIFT_ERROR;
				    LOG_ERR("Error: %s\n", print_error_message(result));
				    return result;
			    }
			    /* right side */
			    ptr_out_param->CAM_RRZ_D_HORI_STEP.Bits.RRZ_HORI_STEP = ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
			    ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_WD = (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X -
				    (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
			    ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ptr_in_param->SW.TWIN_RRZ_OUT_WD -
					 (unsigned int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD;
			    temp_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
				    (long long)ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST +
				    (long long)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD*ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP -
				    (long long)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X*DUAL_RRZ_PREC;
			    /* check xs of right RRZ is valid */
			    if ((long long)((unsigned int)TILE_LOSS_RRZ_TAP_L + (unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.DBS_EN?TILE_LOSS_DBS:0) +
					(unsigned int)(ptr_in_param->CAM_CTL_EN_P1.Bits.BNR_EN?TILE_LOSS_BNR:0))*DUAL_RRZ_PREC > temp_offset)
			    {
				    result = ISP_MESSAGE_RRZ_RIGHT_SHIFT_ERROR;
				    LOG_ERR("Error: %s\n", print_error_message(result));
				    return result;
			    }
				ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned int)temp_offset>>15);
			    ptr_out_param->CAM_RRZ_D_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(temp_offset -
				    (long long)ptr_out_param->CAM_RRZ_D_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
			    /* copy from RRZ_IN_HT & RRZ_D_IN_HT from DMX & DMX_D */
			    ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
			    ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_HT = ptr_in_param->CAM_DMX_VSIZE.Bits.DMX_HT;
			    /* cal vertical step */
			    if (ptr_in_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP)
			    {
				    /* RTL just copy */
				    DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_STEP.Raw, CAM_RRZ_VERT_STEP.Raw);
				    DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_STEP.Raw, CAM_RRZ_D_VERT_STEP.Raw);
			    }
			    else
			    {
				    if (ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT <= 1)
				    {
					    result = ISP_MESSAGE_INVALID_RRZ_IN_CROP_HT_ERROR;
					    LOG_ERR("Error: %s\n", print_error_message(result));
					    return result;
				    }
				    else
				    {
					    /* re-cal RRZ_VERT_STEP */
					    ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP = (long long)(ptr_in_param->SW.TWIN_RRZ_IN_CROP_HT - 1)*
						    DUAL_RRZ_PREC/((unsigned int)ptr_in_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT - 1);
					    if (ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP < DUAL_RRZ_PREC)
					    {
						    result = ISP_MESSAGE_RRZ_SCALING_UP_ERROR;
						    LOG_ERR("Error: %s\n", print_error_message(result));
						    return result;
					    }
					    /* copy RRZ_D_VERT_STEP by RRZ_VERT_STEP */
					    ptr_out_param->CAM_RRZ_D_VERT_STEP.Bits.RRZ_VERT_STEP = ptr_out_param->CAM_RRZ_VERT_STEP.Bits.RRZ_VERT_STEP;
				    }
			    }
                ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_1 = 0;
			    ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_2 = 0;
            }
            /*  RRZ RRZ_D output width check. if left side expand, offset needs back extension, if right side expand, it's tail is safe,*/
            /*  crop and output width change in RMX */
            if(ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD < 4)
            {
			    long long start_offset = (long long)ptr_in_param->SW.TWIN_RRZ_HORI_INT_OFST*DUAL_RRZ_PREC +
				    (long long)ptr_in_param->SW.TWIN_RRZ_HORI_SUB_OFST - (long long)(4 - (unsigned int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD)*
                    ptr_out_param->CAM_RRZ_HORI_STEP.Bits.RRZ_HORI_STEP;
                ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = 4;
                ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST = ((unsigned long long)start_offset>>15);
				ptr_out_param->CAM_RRZ_HORI_SUB_OFST.Bits.RRZ_HORI_SUB_OFST = (int)(start_offset -
				    (long long)ptr_out_param->CAM_RRZ_HORI_INT_OFST.Bits.RRZ_HORI_INT_OFST*(1<<15));
            }
            if (ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD < 4)
            {
                ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = 4;
            }
			/* check RRZ buffer size, must x4 align */
			ptr_out_param->CAM_RRZ_RLB_AOFST.Bits.RRZ_RLB_AOFST = 0;
			ptr_out_param->CAM_RRZ_D_RLB_AOFST.Bits.RRZ_RLB_AOFST = (((unsigned int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + 3)>>2)<<2;
			if (((((unsigned int)ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD + 3)>>2)<<2) +
				(unsigned int)ptr_out_param->CAM_RRZ_D_RLB_AOFST.Bits.RRZ_RLB_AOFST > DUAL_RRZ_WD_ALL)
			{
				result = ISP_MESSAGE_INVALID_CONFIG_ERROR;
				LOG_ERR("Error: %s\n", print_error_message(result));
				return result;
			}
			/* assign common config, RRZ */
			/* partial update */
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT, CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_HT);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_CTL.Raw, CAM_RRZ_D_CTL.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_INT_OFST.Raw, CAM_RRZ_D_VERT_INT_OFST.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_VERT_SUB_OFST.Raw, CAM_RRZ_D_VERT_SUB_OFST.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_TH.Raw, CAM_RRZ_D_MODE_TH.Raw);
			DUAL_COPY_L2R(ptr_in_param, ptr_out_param, CAM_RRZ_MODE_CTL.Raw, CAM_RRZ_D_MODE_CTL.Raw);
		}
		else
		{
			ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD = (unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_END_X -
				(unsigned int)ptr_out_param->CAM_DMX_CROP.Bits.DMX_STR_X + 1;
			ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD = ptr_out_param->CAM_RRZ_IN_IMG.Bits.RRZ_IN_WD;
			ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_WD = (unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_END_X -
				(unsigned int)ptr_out_param->CAM_DMX_D_CROP.Bits.DMX_STR_X + 1;
			ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD = ptr_out_param->CAM_RRZ_D_IN_IMG.Bits.RRZ_IN_WD;
		}
		ptr_out_param->CAM_CTL_EN_P1_D.Bits.RRZ_D_EN = ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN;	
	}
	return result;
}

static ISP_MESSAGE_ENUM dual_cal_rmx(const ISP_DUAL_IN_CONFIG_STRUCT *ptr_in_param, ISP_DUAL_OUT_CONFIG_STRUCT *ptr_out_param)
{
    ISP_MESSAGE_ENUM result = ISP_MESSAGE_OK;
	if (ptr_in_param->CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE && ptr_in_param->CAM_CTL_EN_P1.Bits.BMX_EN &&
		ptr_in_param->CAM_CTL_EN_P1.Bits.RMX_EN && ptr_in_param->CAM_CTL_EN_P1.Bits.DMX_EN)
	{
		/* left side */
		ptr_out_param->CAM_RMX_CTL.Bits.RMX_EDGE = 0xF;
        if((0 == ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_1) &&
            (0 == ptr_out_param->CAM_RMX_CTL.Bits.RMX_SINGLE_MODE_2) &&
            (ptr_in_param->CAM_CTL_EN_P1.Bits.RRZ_EN))
        {
            if (((int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD + (int)ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD) >
                ptr_in_param->SW.TWIN_RRZ_OUT_WD)
            {
                if (4 == ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD)
                {
                    ptr_out_param->CAM_RMX_CROP.Bits.RMX_STR_X = 2;
                    ptr_out_param->CAM_RMX_CROP.Bits.RMX_END_X = 3;
		            ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_STR_X = 0;
                    ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_END_X = (unsigned int)ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD - 1;
                }
                else if (4 == ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD)
                {
                    ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_STR_X = 0;
                    ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_END_X = 1;
		            ptr_out_param->CAM_RMX_CROP.Bits.RMX_STR_X = 0;
                    ptr_out_param->CAM_RMX_CROP.Bits.RMX_END_X = (unsigned int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
                }
                else
                {
				    result = ISP_MESSAGE_INVALID_CONFIG_ERROR;
				    LOG_ERR("Error: %s\n", print_error_message(result));
				    return result;
                }
            }
            else
            {
                ptr_out_param->CAM_RMX_CROP.Bits.RMX_STR_X = 0;
                ptr_out_param->CAM_RMX_CROP.Bits.RMX_END_X = (unsigned int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
                ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_STR_X = 0;
                ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_END_X = (unsigned int)ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD - 1;
            }
        }
        else
        {
            ptr_out_param->CAM_RMX_CROP.Bits.RMX_STR_X = 0;
            ptr_out_param->CAM_RMX_CROP.Bits.RMX_END_X = (unsigned int)ptr_out_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_WD - 1;
            ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_STR_X = 0;
            ptr_out_param->CAM_RMX_D_CROP.Bits.RMX_END_X = (unsigned int)ptr_out_param->CAM_RRZ_D_OUT_IMG.Bits.RRZ_OUT_WD - 1;
        }
		ptr_out_param->CAM_RMX_VSIZE.Bits.RMX_HT = ptr_in_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
		/* right side */
		ptr_out_param->CAM_CTL_EN_P1_D.Bits.RMX_D_EN = 1;
		ptr_out_param->CAM_RMX_D_CTL.Bits.RMX_EDGE = 0xF;
		ptr_out_param->CAM_RMX_D_VSIZE.Bits.RMX_HT = ptr_in_param->CAM_RRZ_OUT_IMG.Bits.RRZ_OUT_HT;
	}
	return result;
}

static const char *dual_cal_print_error_message(ISP_MESSAGE_ENUM n)
{
    GET_ERROR_NAME(n);
}

static const char *dual_cal_get_current_file_name(const char *filename)
{
    char *ptr = strrchr((char *)filename, FOLDER_SYMBOL_CHAR);
    if (NULL == ptr)
    {
        return filename;
    }
    else
    {
        return (const char *)(ptr + 1);
    }
}
