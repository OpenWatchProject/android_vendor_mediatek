#include <string.h>
#include <stdint.h>
//#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utils/Log.h>
#include <assert.h>

#include <arsi_api.h> // implement MTK AURISYS API
#include "inc/parser.h"
#include "inc/smartpa_param.h"

/* nvram */
#include <cutils/properties.h>
#include "Custom_NvRam_LID.h"
#include "libnvram.h"
#include "CFG_SMARTPA_CALIB_File.h"
#include "CFG_SMARTPA_CALIB_Default.h"

#include <tinyalsa/asoundlib.h>

#include <openssl/x509.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>

using namespace Parser;
using namespace std;

#define SMARTPA_LIB_VER	"1.0.0"
#define SMARTPA_LIB_TAG	"[smartpa_param]"

struct map_item {
	string tag;
	struct VariableMapping varMap;
};

static int init_once;
static vector<ObjectRecord> smartpa_parseResult;
static int sig_check_fail;
static map<string, VariableMapping> smartpa_structMapping;

static struct spk_pro_parameter default_param = {
	.param_version = (RT_SPA_PARAM_VER << 16) |
		sizeof(struct spk_pro_parameter),
	.param_date = 0x20190618,
	.dump_l = 7,
	.dump_r = 8,
	.dcb_filt_en = 0,
	.farrow_interp_en = 1,
	.delay_est_en = 0,

	.dcb_filt = {
		.b0 = 16742151,
		.b1 = -16742151,
		.a1 = -16707087,
	},

	.pil_filt_param = {
		.b0 = 42791,
		.b1 = 85582,
		.b2 = 42791,
		.a1 = -31073862,
		.a2 = 14467809,
	},

	.sig_filt_param = {
		.b0 = 15579722,
		.b1 = -31159443,
		.b2 = 15579722,
		.a1 = -31073862,
		.a2 = 14467809,
	},

	.delay_est_mu = 16384,
	.delay_est_diff_smpls = 10,
	.delay_est_max_delay = 16777216,
	.delay_est_min_delay = -16777216,
	.delay_est_bpf_cos_phi2 = 263525,
	.delay_est_bpf_sin_phi2 = 16775146,

	.tone_det_cos_phi2 = 1316325,
	.tone_det_sin_phi2 = 16725497,
	.tone_det_mu = 256,
	.tone_det_log2_dev = 16,
	.tone_det_sgl_tone_ratio = 33554432,
	.tone_det_act_thres = 167772,
	.tone_det_recovery_smpls = 480,
	.rx_act_recovery_smpls = 480,

	.dcr_est_mu = 4388608,
	.dcr_est_cvg_thres0 = 83886,
	.dcr_est_cvg_thres1 = 32,

	.atc_kp = 33554432,
	.atc_ki = 1342177,
	.atc_kd = 1342177,

	.max_dcr = 31140480,
	.max_dcr_dmg = 67108864,
	.min_atc_gain = 4194304,

	.imp_est_ord = 2,
	.imp_est_win = 8,
	.imp_est_mu0_exp = 7,
	.imp_est_mu1_exp = 3,

	.fres_damage_ratio = 33554432,
	.fres_recovery_smpls = 100,
	.fres_stepsize = 21961,

	.exc_release_rate = 16609444,
	.exc_hold_th = 8053063,
	.exc_hold_smpls = 100,
	.sig_release_rate = 16609444,
	.sig_hold_th = 13421773,

	.max_sig = 16102811,
	.max_exc = 16609444,
	.max_sig_dmg = 8388608,
	.max_exc_dmg = 8388608,
	.mode_sel_dmg = 0,
	.volume = 0,
	.ramp_speed = 2796202,

	.tx_act_thres = 167772,
	.pil_gain = 204682,
	.pil_norm_freq = 109805,
	.pil_zero_thres = 1678,
	.bypass_num = 30,
	.bypass_cnt = 0,

	.eq = {
		.Nbands = 10,
		.biquad_coeff = {
			{6987584, 13975169, 6987584, 7735777, 3437345},
			{16343798, -32687596, 16343798, -32676616, 15921360},
			{16916301, -32663639, 15804087,	-32663639, 15943173},
			{16674060, -32853716, 16408972,	-32853716, 16305816},
			{15508272, -12521720, 14198497,	-12521720, 12929553},
			{12725128, -6429774, 2705612,	-12176996, 4400747},
			{15891439, -24479863, 14977159,	-24479863, 14091383},
			{16994942, -31228144, 14437747,	-31258013, 14625604},
			{15861793, -23710973, 14022232,	-23710973, 13106810},
			{16777216, 0, 0, 0, 0},
		},

	},

	.mbdrc = {
		.Nbands = 5,
		.delay = 128,
		.predict_offset = 1,
		.biquad_coeff = {
			{6292, 12584, 6292, -32619899, 15867851},
			{16320078, -32640156, 16320078, -32627569, 15875527},
			{829563, 1659127, 829563, -21449539, 7990579},
			{11580589, -23161179, 11580589, -21498281, 8046861},
			{0,0,0,0,0},
			{0,0,0,0,0},
			{0,0,0,0,0},
			{0,0,0,0,0},
			{0,0,0,0,0},
			{0,0,0,0,0},
		},
		.mode_rms = {0},
		.band_skip = {0},
		.alpha = {
			380106,
			380106,
			380106,
			380106,
			380106,
		},
		.omega = {
			1538,
			1538,
			1538,
			1538,
			1538,
		},
		.At = {
			76720,
			76720,
			76720,
			76720,
			76720,
		},
		.Rt = {
			7688,
			7688,
			7688,
			7688,
			7688,
		},
		.thres0 = {
			-27866353,
			-27866353,
			-27866353,
			-27866353,
			-27866353,
		},
		.thres1 = {
			-167198116,
			-167198116,
			-167198116,
			-167198116,
			-167198116,
		},
		.thres2 = {
			-222930821,
			-222930821,
			-222930821,
			-222930821,
			-222930821,
		},
		.ratio0 = {0},
		.ratio1 = {
			67108864,
			67108864,
			67108864,
			67108864,
			67108864,
		},
		.makeup = {
			41799529,
			41799529,
			41799529,
			41799529,
			41799529,
		},
	},

	.sbdrc = {
		.Nbands = 1,
		.delay = 128,
		.predict_offset = 1,
		.mode_rms = {0},
		.band_skip = {0},
		.alpha ={380106, 0, 0, 0, 0},
		.omega = {1538, 0, 0, 0, 0},
		.At = {76720, 0, 0, 0, 0},
		.Rt = {7688, 0, 0, 0, 0},
		.thres0 = {-27866353, 0, 0, 0, 0},
		.thres1 = {-167198116, 0, 0, 0, 0},
		.thres2 = {-222930821, 0, 0, 0, 0},
		.ratio0 = {0, 0, 0, 0, 0},
		.ratio1 = {67108864, 0, 0, 0, 0},
		.makeup = {41799529, 0, 0, 0, 0},
	},
	.mul_one = 0,
	.calib_y = {
		-11115673,
		10865236,
		5121812,
		-4119360,
		6504144,
	},
	.calib_dcr = 26756096,
	.spk_pro_enable = 1,
	.eq_drc_switch = 1,
	.est_delay = 838860,
	.fres_set_fres = 15847291,
	.pro_gain_switch = 0,
	.alpha_speaker = 240,
	.max_temperature = 100,
	.eq_drc_ord = 1,
	.t0 = 0,
	.imp_est_single_tone_mu1_exp = 6,
	.monitor_on = 0,
	.vi_sensing_lpf_en = 0,
	.deci_ratio = 8,
	.deci_filt_param = {
		.b0 = 502554,
		.b1 = 1005109,
		.b2 = 502554,
		.a1 = -24398159,
		.a2 = 9631161,
	},
	.exc_lpf_param = {
		.b0 = 8388608,
		.b1 = 8388608,
		.a1 = -16646144,
	},
	.input_gain = 0,
	.single_tone_update = 0,
	.sample_rate = 0,
	.tuning_enable = 0,
	.chip_rev = 1,
	.sig_deci_filt = {
		.b0 = 7416374,
		.b1 = -5054104,
		.b2 = 7416374,
		.a1 = -28530943,
		.a2 = 12582359,
	},
	.sig_deci_filt2 = {
		.b0 = 497918,
		.b1 = -810665,
		.b2 = 497918,
		.a1 = -29803015,
		.a2 = 15477616,
	},
	.dump_enable = 0,
	.comp_filt_coef = {493, -1681, 9682, 256060,
		15926054, 356060, 9682, -1681, 493,},
	.fir_enable = 0,
	.bypass_lib = 0,
	.vo_thr_error = 0,
	.flags = 0,
	.kd_thres = 16700000,
	.hp_filter_en = 1,
	.hp_filter = {
		.b0 = 16466454,
		.b1 = -32932909,
		.b2 = 16466454,
		.a1 = -32927265,
		.a2 = 16161336,
	},
	.version = 0x0B22,
	.mbdrc_pre_gain = {0},
	.sbdrc_pre_gain = 0,
	.dc_term_filt = {
		.b0 = 16770626,
		.b1 = 0,
		.a1 = 16777210,
	},
	.dc_term_detect_smpls = 3200,
	.dc_term_thres = 12582912,
	.plmt_ramp_speed = 2097152,
	.plmt_lim_en = 1,
	.safeguard_clip_thres = 65431142,
	.safeguard_slope = 69886080,
	.safeguard_vh = 5238860,
	.safeguard_po_min = 8388608,
	.safeguard_hold_th = 16609444,
	.safeguard_hold_smpls = 100,
	.safeguard_release_rate = 16777048,
	.safeguard_ramp_down_speed = 17476,
	.safeguard_en = 0,
	.pwr_act_thres = 25165824,
	.safeguard_ramp_up_speed = 1747,
	.plmt_limit_dcr = 25165824,
	.safeguard_peak_det_mode = 0,
	.plmt_min_dcr = 16777216,
	.vvalidation_error_flag = 0,
	.vvalidation_max_pwr = 5000,
	.vvalidation_min_pwr = 4300,
	.vvalidation_signal_ratio = 124518,
	.vvalidation_pwr_real = 0,
	.big_data = {
		.data = {
			0, 0, 0,
		},
	},
	.pcb_trace = 0,
};

static const struct map_item map_items[] = {
	{ string("PARAM_VERSION"), {1, offsetof(spk_pro_parameter, param_version)}},
	{ string("PARAM_DATE"), {1, offsetof(spk_pro_parameter, param_date)}},
	{ string("DUMP_L"), {1, offsetof(spk_pro_parameter, dump_l)}},
	{ string("DUMP_R"), {1, offsetof(spk_pro_parameter, dump_r)}},
	{ string("DCB_FILT_EN"), {1, offsetof(spk_pro_parameter, dcb_filt_en)}},
	{ string("FARROW_INTERP_EN"), {1, offsetof(spk_pro_parameter, farrow_interp_en)}},
	{ string("DELAY_EST_EN"), {1, offsetof(spk_pro_parameter, delay_est_en)}},
	{ string("DELAY_EST_MU"), {1, offsetof(spk_pro_parameter, delay_est_mu)}},
	{ string("DELAY_EST_DIFF_SMPLS"), {1, offsetof(spk_pro_parameter, delay_est_diff_smpls)}},
	{ string("DELAY_EST_MAX_DELAY"), {1, offsetof(spk_pro_parameter, delay_est_max_delay)}},
	{ string("DELAY_EST_MIN_DELAY"), {1, offsetof(spk_pro_parameter, delay_est_min_delay)}},
	{ string("DELAY_EST_BPF_COS_PHI2"), {1, offsetof(spk_pro_parameter, delay_est_bpf_cos_phi2)}},
	{ string("DELAY_EST_BPF_SIN_PHI2"), {1, offsetof(spk_pro_parameter, delay_est_bpf_sin_phi2)}},
	{ string("TONE_DET_COS_PHI2"), {1, offsetof(spk_pro_parameter, tone_det_cos_phi2)}},
	{ string("TONE_DET_SIN_PHI2"), {1, offsetof(spk_pro_parameter, tone_det_sin_phi2)}},
	{ string("TONE_DET_MU"), {1, offsetof(spk_pro_parameter, tone_det_mu)}},
	{ string("TONE_LOG2_DEV"), {1, offsetof(spk_pro_parameter, tone_det_log2_dev)}},
	{ string("TONE_DET_SGL_TONE_RATIO"), {1, offsetof(spk_pro_parameter, tone_det_sgl_tone_ratio)}},
	{ string("TONE_DET_ACT_THRES"), {1, offsetof(spk_pro_parameter, tone_det_act_thres)}},
	{ string("TONE_DET_RECOVERY_SMPLS"), {1, offsetof(spk_pro_parameter, tone_det_recovery_smpls)}},
	{ string("RX_ACT_RECOVERY_SMPLS"), {1 , offsetof(spk_pro_parameter, rx_act_recovery_smpls)}},
	{ string("DCR_EST_MU"), {1, offsetof(spk_pro_parameter, dcr_est_mu)}},
	{ string("DCR_EST_CVG_THRES0"), {1, offsetof(spk_pro_parameter, dcr_est_cvg_thres0)}},
	{ string("DCR_EST_CVG_THRES1"), {1, offsetof(spk_pro_parameter, dcr_est_cvg_thres1)}},
	{ string("ATC_KP"), {1, offsetof(spk_pro_parameter, atc_kp)}},
	{ string("ATC_KI"), {1, offsetof(spk_pro_parameter, atc_ki)}},
	{ string("ATC_KD"), {1, offsetof(spk_pro_parameter, atc_kd)}},
	{ string("MAX_DCR"), {1, offsetof(spk_pro_parameter, max_dcr)}},
	{ string("MAX_DCR_DMG"), {1, offsetof(spk_pro_parameter, max_dcr_dmg)}},
	{ string("MIN_ATC_GAIN"), {1, offsetof(spk_pro_parameter, min_atc_gain)}},
	{ string("IMP_EST_ORD"), {1, offsetof(spk_pro_parameter, imp_est_ord)}},
	{ string("IMP_EST_WIN"), {1, offsetof(spk_pro_parameter, imp_est_win)}},
	{ string("IMP_EST_MU0_EXP"), {1, offsetof(spk_pro_parameter, imp_est_mu0_exp)}},
	{ string("IMP_EST_MU1_EXP"), {1, offsetof(spk_pro_parameter, imp_est_mu1_exp)}},
	{ string("FRES_DAMAGE_RATIO"), {1, offsetof(spk_pro_parameter, fres_damage_ratio)}},
	{ string("FRES_RECOVERY_SMPLS"), {1, offsetof(spk_pro_parameter, fres_recovery_smpls)}},
	{ string("FRES_STEPSIZE"), {1, offsetof(spk_pro_parameter, fres_stepsize)}},
	{ string("EXC_RELEASE_RATE"), {1, offsetof(spk_pro_parameter, exc_release_rate)}},
	{ string("EXC_HOLD_TH"), {1, offsetof(spk_pro_parameter, exc_hold_th)}},
	{ string("EXC_HOLD_SMPLS"), {1, offsetof(spk_pro_parameter, exc_hold_smpls)}},
	{ string("SIG_RELEASE_RATE"), {1, offsetof(spk_pro_parameter, sig_release_rate)}},
	{ string("SIG_HOLD_TH"), {1, offsetof(spk_pro_parameter, sig_hold_th)}},
	{ string("MAX_SIG"), {1, offsetof(spk_pro_parameter, max_sig)}},
	{ string("MAX_EXC"), {1, offsetof(spk_pro_parameter, max_exc)}},
	{ string("MAX_SIG_DMG"), {1, offsetof(spk_pro_parameter, max_sig_dmg)}},
	{ string("MAX_EXC_DMG"), {1, offsetof(spk_pro_parameter, max_exc_dmg)}},
	{ string("MODE_SEL_DMG"), {1, offsetof(spk_pro_parameter, mode_sel_dmg)}},
	{ string("VOLUME"), {1, offsetof(spk_pro_parameter, volume)}},
	{ string("RAMP_SPEED"), {1, offsetof(spk_pro_parameter, ramp_speed)}},
	{ string("TX_ACT_THRES"), {1, offsetof(spk_pro_parameter, tx_act_thres)}},
	{ string("PIL_GAIN"), {1, offsetof(spk_pro_parameter, pil_gain)}},
	{ string("PIL_NORM_FREQ"), {1, offsetof(spk_pro_parameter, pil_norm_freq)}},
	{ string("PIL_ZERO_THRES"), {1, offsetof(spk_pro_parameter, pil_zero_thres)}},
	{ string("BYPASS_NUM"), {1, offsetof(spk_pro_parameter, bypass_num)}},
	{ string("BYPASS_CNT"), {1, offsetof(spk_pro_parameter, bypass_cnt)}},
	{ string("MUL_ONE"), {1, offsetof(spk_pro_parameter, mul_one)}},
	{ string("CALIB_DCR"), {1, offsetof(spk_pro_parameter, calib_dcr)}},
	{ string("SPK_PRO_ENABLE"), {1, offsetof(spk_pro_parameter, spk_pro_enable)}},
	{ string("EQ_DRC_SWITCH"), {1, offsetof(spk_pro_parameter, eq_drc_switch)}},
	{ string("EST_DELAY"), {1, offsetof(spk_pro_parameter, est_delay)}},
	{ string("FRES_SET_FRES"), {1, offsetof(spk_pro_parameter, fres_set_fres)}},
	{ string("T0"), {1, offsetof(spk_pro_parameter, t0)}},
	{ string("PRO_GAIN_SWITCH"), {1, offsetof(spk_pro_parameter, pro_gain_switch)}},
	{ string("ALPHA_SPEAKER"), {1, offsetof(spk_pro_parameter, alpha_speaker)}},
	{ string("MAX_TEMPERATURE"), {1, offsetof(spk_pro_parameter, max_temperature)}},
	{ string("EQ_DRC_ORD"), {1, offsetof(spk_pro_parameter, eq_drc_ord)}},
	{ string("IMP_EST_IS_SINGLE_TONE_MU1_EXP"), {1, offsetof(spk_pro_parameter, imp_est_single_tone_mu1_exp)}},
	{ string("MONITOR_ON"), {1, offsetof(spk_pro_parameter, monitor_on)}},
	{ string("VI_SENSING_LPF_EN"), {1, offsetof(spk_pro_parameter, vi_sensing_lpf_en)}},
	{ string("DECI_RATIO"), {1, offsetof(spk_pro_parameter, deci_ratio)}},
	{ string("INPUT_GAIN"), {1, offsetof(spk_pro_parameter, input_gain)}},
	{ string("PARAM_VERSION"), {1, offsetof(spk_pro_parameter, param_version)}},
	{ string("SINGLE_TONE_UPDATE"), {1, offsetof(spk_pro_parameter, single_tone_update)}},
	{ string("SAMPLE_RATE"), {1, offsetof(spk_pro_parameter, sample_rate)}},
	{ string("TUNING_ENABLE"), {1, offsetof(spk_pro_parameter, tuning_enable)}},
	{ string("MONITOR_DATA"), {1, offsetof(spk_pro_parameter, monitor_data)}},
	{ string("CHIP_REV"), {1, offsetof(spk_pro_parameter, chip_rev)}},
	{ string("DUMP_ENABLE"), {1, offsetof(spk_pro_parameter, dump_enable)}},
	{ string("FIR_ENABLE"), {1, offsetof(spk_pro_parameter, fir_enable)}},
	{ string("BYPASS_LIB"), {1, offsetof(spk_pro_parameter, bypass_lib)}},
	{ string("VO_THR_ERROR"), {1, offsetof(spk_pro_parameter, vo_thr_error)}},
	{ string("FLAGS"), {1, offsetof(spk_pro_parameter, flags)}},
	{ string("KD_THRES"), {1, offsetof(spk_pro_parameter, kd_thres)}},
	{ string("HP_FILTER_EN"), {1, offsetof(spk_pro_parameter, hp_filter_en)}},
	{ string("SBDRC_PRE_GAIN"), {1, offsetof(spk_pro_parameter, sbdrc_pre_gain)}},
	{ string("DC_TERM_THRES"), {1, offsetof(spk_pro_parameter, dc_term_thres)}},
	{ string("DC_TERM_DETECT_SMPLS"), {1, offsetof(spk_pro_parameter, dc_term_detect_smpls)}},
	{ string("DC_TERM_EN"), {1, offsetof(spk_pro_parameter, dc_term_en)}},
	{ string("PLMT_RAMP_SPEED"), {1, offsetof(spk_pro_parameter, plmt_ramp_speed)}},
	{ string("PLMT_LIM_EN"), {1, offsetof(spk_pro_parameter, plmt_lim_en)}},
	{ string("SAFEGUARD_PO_MIN"), {1, offsetof(spk_pro_parameter, safeguard_po_min)}},
	{ string("SAFEGUARD_SLOPE"), {1, offsetof(spk_pro_parameter, safeguard_slope)}},
	{ string("SAFEGUARD_HOLD_SMPLS"), {1, offsetof(spk_pro_parameter, safeguard_hold_smpls)}},
	{ string("SAFEGUARD_RELEASE_RATE"), {1, offsetof(spk_pro_parameter, safeguard_release_rate)}},
	{ string("SAFEGUARD_HOLD_TH"), {1, offsetof(spk_pro_parameter, safeguard_hold_th)}},
	{ string("SAFEGUARD_VH"), {1, offsetof(spk_pro_parameter, safeguard_vh)}},
	{ string("SAFEGUARD_CLIP_THRES"), {1, offsetof(spk_pro_parameter, safeguard_clip_thres)}},
	{ string("SAFEGUARD_RAMP_DOWN_SPEED"), {1, offsetof(spk_pro_parameter, safeguard_ramp_down_speed)}},
	{ string("SAFEGUARD_EN"), {1, offsetof(spk_pro_parameter, safeguard_en)}},
	{ string("PWR_ACT_THRES"), {1, offsetof(spk_pro_parameter, pwr_act_thres)}},
	{ string("SAFEGUARD_RAMP_UP_SPEED"), {1, offsetof(spk_pro_parameter, safeguard_ramp_up_speed)}},
	{ string("PLMT_LIMIT_DCR"), {1, offsetof(spk_pro_parameter, plmt_limit_dcr)}},
	{ string("SAFEGUARD_PEAK_DET_MODE"), {1, offsetof(spk_pro_parameter, safeguard_peak_det_mode)}},
	{ string("VVALIDATION_SIGNAL_RATIO"), {1, offsetof(spk_pro_parameter, vvalidation_signal_ratio)}},
	{ string("VVALIDATION_ERROR_FLAG"), {1, offsetof(spk_pro_parameter, vvalidation_error_flag)}},	
	{ string("VVALIDATION_MAX_PWR"), {1, offsetof(spk_pro_parameter, vvalidation_max_pwr)}},
	{ string("VVALIDATION_MIN_PWR"), {1, offsetof(spk_pro_parameter, vvalidation_min_pwr)}},
	{ string("VVALIDATION_PWR_REAL"), {1, offsetof(spk_pro_parameter, vvalidation_pwr_real)}}, 
	{ string("PCB_TRACE"), {1, offsetof(spk_pro_parameter, pcb_trace)}},
	{ string("CALIB_Y"), {5, offsetof(spk_pro_parameter, calib_y)}},
	{ string("COMP_FILT_COEF"), {9, offsetof(spk_pro_parameter, comp_filt_coef)}},
	{ string("MBDRC_PRE_GAIN"), {5, offsetof(spk_pro_parameter, mbdrc_pre_gain)}},
	{ string("DCB_FILT_B0"), {2, offsetof(spk_pro_parameter, dcb_filt.b0)}},
	{ string("DCB_FILT_B1"), {2, offsetof(spk_pro_parameter, dcb_filt.b1)}},
	{ string("DCB_FILT_A1"), {2, offsetof(spk_pro_parameter, dcb_filt.a1)}},
	{ string("EXC_LPF_B0"), {1, offsetof(spk_pro_parameter, exc_lpf_param.b0)}},
	{ string("EXC_LPF_B1"), {1, offsetof(spk_pro_parameter, exc_lpf_param.b1)}},
	{ string("EXC_LPF_A1"), {1, offsetof(spk_pro_parameter, exc_lpf_param.a1)}},
	{ string("PIL_FILT_B0"), {1, offsetof(spk_pro_parameter, pil_filt_param.b0)}},
	{ string("PIL_FILT_B1"), {1, offsetof(spk_pro_parameter, pil_filt_param.b1)}},
	{ string("PIL_FILT_B2"), {1, offsetof(spk_pro_parameter, pil_filt_param.b2)}},
	{ string("PIL_FILT_A1"), {1, offsetof(spk_pro_parameter, pil_filt_param.a1)}},
	{ string("PIL_FILT_A2"), {1, offsetof(spk_pro_parameter, pil_filt_param.a2)}},
	{ string("SIG_FILT_B0"), {1, offsetof(spk_pro_parameter, sig_filt_param.b0)}},
	{ string("SIG_FILT_B1"), {1, offsetof(spk_pro_parameter, sig_filt_param.b1)}},
	{ string("SIG_FILT_B2"), {1, offsetof(spk_pro_parameter, sig_filt_param.b2)}},
	{ string("SIG_FILT_A1"), {1, offsetof(spk_pro_parameter, sig_filt_param.a1)}},
	{ string("SIG_FILT_A2"), {1, offsetof(spk_pro_parameter, sig_filt_param.a2)}},
	{ string("DECI_FILT_B0"), {1, offsetof(spk_pro_parameter, deci_filt_param.b0)}},
	{ string("DECI_FILT_B1"), {1, offsetof(spk_pro_parameter, deci_filt_param.b1)}},
	{ string("DECI_FILT_B2"), {1, offsetof(spk_pro_parameter, deci_filt_param.b2)}},
	{ string("DECI_FILT_A1"), {1, offsetof(spk_pro_parameter, deci_filt_param.a1)}},
	{ string("DECI_FILT_A2"), {1, offsetof(spk_pro_parameter, deci_filt_param.a2)}},
	{ string("SIG_DECI_FILT_B0"), {1, offsetof(spk_pro_parameter, sig_deci_filt.b0)}},
	{ string("SIG_DECI_FILT_B1"), {1, offsetof(spk_pro_parameter, sig_deci_filt.b1)}},
	{ string("SIG_DECI_FILT_B2"), {1, offsetof(spk_pro_parameter, sig_deci_filt.b2)}},
	{ string("SIG_DECI_FILT_A1"), {1, offsetof(spk_pro_parameter, sig_deci_filt.a1)}},
	{ string("SIG_DECI_FILT_A2"), {1, offsetof(spk_pro_parameter, sig_deci_filt.a2)}},
	{ string("SIG_DECI_FILT2_B0"), {1, offsetof(spk_pro_parameter, sig_deci_filt2.b0)}},
	{ string("SIG_DECI_FILT2_B1"), {1, offsetof(spk_pro_parameter, sig_deci_filt2.b1)}},
	{ string("SIG_DECI_FILT2_B2"), {1, offsetof(spk_pro_parameter, sig_deci_filt2.b2)}},
	{ string("SIG_DECI_FILT2_A1"), {1, offsetof(spk_pro_parameter, sig_deci_filt2.a1)}},
	{ string("SIG_DECI_FILT2_A2"), {1, offsetof(spk_pro_parameter, sig_deci_filt2.a2)}},
	{ string("HP_FILTER"), {5, offsetof(spk_pro_parameter, hp_filter)}},
	{ string("EQ_NBANDS"), {1, offsetof(spk_pro_parameter, eq.Nbands)}},
	{ string("EQ_BIQUAD_COEFF_0"), {25, offsetof(spk_pro_parameter, eq.biquad_coeff[0])}},
	{ string("EQ_BIQUAD_COEFF_5"), {25, offsetof(spk_pro_parameter, eq.biquad_coeff[5])}},
	{ string("MBDRC_NBANDS"), {1, offsetof(spk_pro_parameter, mbdrc.Nbands)}},
	{ string("MBDRC_DELAY"), {1, offsetof(spk_pro_parameter, mbdrc.delay)}},
	{ string("MBDRC_PREDICT_OFFSET"), {1, offsetof(spk_pro_parameter, mbdrc.predict_offset)}},
	{ string("MBDRC_BIQUAD_COEFF_0"), {25, offsetof(spk_pro_parameter, mbdrc.biquad_coeff[0])}},
	{ string("MBDRC_BIQUAD_COEFF_5"), {25, offsetof(spk_pro_parameter, mbdrc.biquad_coeff[5])}},
	{ string("MBDRC_MODE_RMS"), {5, offsetof(spk_pro_parameter, mbdrc.mode_rms)}},
	{ string("MBDRC_BAND_SKIP"), {5, offsetof(spk_pro_parameter, mbdrc.band_skip)}},
	{ string("MBDRC_ALPHA"), {5, offsetof(spk_pro_parameter, mbdrc.alpha)}},
	{ string("MBDRC_OMEGA"), {5, offsetof(spk_pro_parameter, mbdrc.omega)}},
	{ string("MBDRC_AT"), {5, offsetof(spk_pro_parameter, mbdrc.At)}},
	{ string("MBDRC_RT"), {5, offsetof(spk_pro_parameter, mbdrc.Rt)}},
	{ string("MBDRC_THRES0"), {5, offsetof(spk_pro_parameter, mbdrc.thres0)}},
	{ string("MBDRC_THRES1"), {5, offsetof(spk_pro_parameter, mbdrc.thres1)}},
	{ string("MBDRC_THRES2"), {5, offsetof(spk_pro_parameter, mbdrc.thres2)}},
	{ string("MBDRC_RATIO0"), {5, offsetof(spk_pro_parameter, mbdrc.ratio0)}},
	{ string("MBDRC_RATIO1"), {5, offsetof(spk_pro_parameter, mbdrc.ratio1)}},
	{ string("MBDRC_MAKEUP"), {5, offsetof(spk_pro_parameter, mbdrc.makeup)}},
	{ string("SBDRC_NBANDS"), {1, offsetof(spk_pro_parameter, sbdrc.Nbands)}},
	{ string("SBDRC_DELAY"), {1, offsetof(spk_pro_parameter, sbdrc.delay)}},
	{ string("SBDRC_PREDICT_OFFSET"), {1, offsetof(spk_pro_parameter, sbdrc.predict_offset)}},
	{ string("SBDRC_MODE_RMS"), {1, offsetof(spk_pro_parameter, sbdrc.mode_rms)}},
	{ string("SBDRC_BAND_SKIP"), {5, offsetof(spk_pro_parameter, sbdrc.band_skip)}},
	{ string("SBDRC_ALPHA"), {1, offsetof(spk_pro_parameter, sbdrc.alpha)}},
	{ string("SBDRC_OMEGA"), {1, offsetof(spk_pro_parameter, sbdrc.omega)}},
	{ string("SBDRC_AT"), {1, offsetof(spk_pro_parameter, sbdrc.At)}},
	{ string("SBDRC_RT"), {1, offsetof(spk_pro_parameter, sbdrc.Rt)}},
	{ string("SBDRC_THRES0_0"), {1, offsetof(spk_pro_parameter, sbdrc.thres0)}},
	{ string("SBDRC_THRES1_0"), {1, offsetof(spk_pro_parameter, sbdrc.thres1)}},
	{ string("SBDRC_THRES2_0"), {1, offsetof(spk_pro_parameter, sbdrc.thres2)}},
	{ string("SBDRC_RATIO0_0"), {1, offsetof(spk_pro_parameter, sbdrc.ratio0)}},
	{ string("SBDRC_RATIO1_0"), {1, offsetof(spk_pro_parameter, sbdrc.ratio1)}},
	{ string("SBDRC_MAKEUP_0"), {1, offsetof(spk_pro_parameter, sbdrc.makeup)}},
	{ string("DC_TERM_FILT_B0"), {2, offsetof(spk_pro_parameter, dc_term_filt.b0)}},
	{ string("DC_TERM_FILT_B1"), {2, offsetof(spk_pro_parameter, dc_term_filt.b1)}},
	{ string("DC_TERM_FILT_A1"), {2, offsetof(spk_pro_parameter, dc_term_filt.a1)}},
	{ string("BIG_DATA_DATA"), {3, offsetof(spk_pro_parameter, big_data)}},
};

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

static map<string, VariableMapping> initVarMapping(void)
{
	map<string, VariableMapping> m;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(map_items); i++)
		m[map_items[i].tag] = map_items[i].varMap;
	return m;
}

#ifdef __DEBUG
static void print_config_param(const struct spk_pro_parameter *param)
{
	int i;

	ALOGE(SMARTPA_LIB_TAG "%s++\n", __func__);
	ALOGE(SMARTPA_LIB_TAG "dump_l = %d\n", param->dump_l);
	ALOGE(SMARTPA_LIB_TAG "dump_r = %d\n", param->dump_r);
	ALOGE(SMARTPA_LIB_TAG "dcb_filt_en = %d\n", param->dcb_filt_en);
	ALOGE(SMARTPA_LIB_TAG "farrow_interp_en = %d\n", param->farrow_interp_en);
	ALOGE(SMARTPA_LIB_TAG "delay_est_en = %d\n", param->delay_est_en);
	/* iir_1st */
#ifdef __LP64__
	ALOGE(SMARTPA_LIB_TAG "iir_1st b0 = %ld\n", param->dcb_filt.b0);
	ALOGE(SMARTPA_LIB_TAG "iir_1st b1 = %ld\n", param->dcb_filt.b1);
	ALOGE(SMARTPA_LIB_TAG "iir_1st a1 = %ld\n", param->dcb_filt.a1);
#else
	ALOGE(SMARTPA_LIB_TAG "iir_1st b0 = %lld\n", param->dcb_filt.b0);
	ALOGE(SMARTPA_LIB_TAG "iir_1st b1 = %lld\n", param->dcb_filt.b1);
	ALOGE(SMARTPA_LIB_TAG "iir_1st a1 = %lld\n", param->dcb_filt.a1);
#endif /* #ifdef __LP64__ */
	/* pil_filt */
	ALOGE(SMARTPA_LIB_TAG "pil_filt b0 = %d\n", param->pil_filt_param.b0);
	ALOGE(SMARTPA_LIB_TAG "pil_filt b1 = %d\n", param->pil_filt_param.b1);
	ALOGE(SMARTPA_LIB_TAG "pil_filt b2 = %d\n", param->pil_filt_param.b2);
	ALOGE(SMARTPA_LIB_TAG "pil_filt a1 = %d\n", param->pil_filt_param.a1);
	ALOGE(SMARTPA_LIB_TAG "pil_filt a2 = %d\n", param->pil_filt_param.a2);
	/* sig_filt */
	ALOGE(SMARTPA_LIB_TAG "sig_filt b0 = %d\n", param->sig_filt_param.b0);
	ALOGE(SMARTPA_LIB_TAG "sig_filt b1 = %d\n", param->sig_filt_param.b1);
	ALOGE(SMARTPA_LIB_TAG "sig_filt b2 = %d\n", param->sig_filt_param.b2);
	ALOGE(SMARTPA_LIB_TAG "sig_filt a1 = %d\n", param->sig_filt_param.a1);
	ALOGE(SMARTPA_LIB_TAG "sig_filt a2 = %d\n", param->sig_filt_param.a2);
	/* general */
	ALOGE(SMARTPA_LIB_TAG "delay_est_mu = %d\n", param->delay_est_mu);
	ALOGE(SMARTPA_LIB_TAG "delay_est_diff_smpls = %d\n", param->delay_est_diff_smpls);
	ALOGE(SMARTPA_LIB_TAG "delay_est_max_delay = %d\n", param->delay_est_max_delay);
	ALOGE(SMARTPA_LIB_TAG "delay_est_min_delay = %d\n", param->delay_est_min_delay);
	ALOGE(SMARTPA_LIB_TAG "delay_est_bpf_cos_phi2 = %d\n", param->delay_est_bpf_cos_phi2);
	ALOGE(SMARTPA_LIB_TAG "delay_est_bpf_sin_phi2 = %d\n", param->delay_est_bpf_sin_phi2);
	ALOGE(SMARTPA_LIB_TAG "tone_det_cos_phi2 = %d\n", param->tone_det_cos_phi2);
	ALOGE(SMARTPA_LIB_TAG "tone_det_sin_phi2 = %d\n", param->tone_det_sin_phi2);
	ALOGE(SMARTPA_LIB_TAG "tone_det_mu = %d\n", param->tone_det_mu);
	ALOGE(SMARTPA_LIB_TAG "tone_det_log2_dev = %d\n", param->tone_det_log2_dev);
	ALOGE(SMARTPA_LIB_TAG "tone_det_sgl_tone_ratio = %d\n", param->tone_det_sgl_tone_ratio);
	ALOGE(SMARTPA_LIB_TAG "tone_det_act_thres = %d\n", param->tone_det_act_thres);
	ALOGE(SMARTPA_LIB_TAG "tone_det_recovery_smpls = %d\n", param->tone_det_recovery_smpls);
	ALOGE(SMARTPA_LIB_TAG "rx_act_recovery_smpls = %d\n", param->rx_act_recovery_smpls);
	ALOGE(SMARTPA_LIB_TAG "dcr_est_mu = %d\n", param->dcr_est_mu);
	ALOGE(SMARTPA_LIB_TAG "dcr_est_cvg_thres0 = %d\n", param->dcr_est_cvg_thres0);
	ALOGE(SMARTPA_LIB_TAG "dcr_est_cvg_thres1 = %d\n", param->dcr_est_cvg_thres1);
	ALOGE(SMARTPA_LIB_TAG "atc_kp = %d\n", param->atc_kp);
	ALOGE(SMARTPA_LIB_TAG "atc_ki = %d\n", param->atc_ki);
	ALOGE(SMARTPA_LIB_TAG "atc_kd = %d\n", param->atc_kd);
	ALOGE(SMARTPA_LIB_TAG "max_dcr_dmg = %d\n", param->max_dcr_dmg);
	ALOGE(SMARTPA_LIB_TAG "min_atc_gain = %d\n", param->min_atc_gain);
	ALOGE(SMARTPA_LIB_TAG "imp_est_ord = %d\n", param->imp_est_ord);
	ALOGE(SMARTPA_LIB_TAG "imp_est_win = %d\n", param->imp_est_win);
	ALOGE(SMARTPA_LIB_TAG "imp_est_mu0_exp = %d\n", param->imp_est_mu0_exp);
	ALOGE(SMARTPA_LIB_TAG "imp_est_mu1_exp = %d\n", param->imp_est_mu1_exp);
	ALOGE(SMARTPA_LIB_TAG "fres_damage_ratio = %d\n", param->fres_damage_ratio);
	ALOGE(SMARTPA_LIB_TAG "fres_recovery_smpls = %d\n", param->fres_recovery_smpls);
	ALOGE(SMARTPA_LIB_TAG "fres_stepsize = %d\n", param->fres_stepsize);
	ALOGE(SMARTPA_LIB_TAG "exc_release_rate = %d\n", param->exc_release_rate);
	ALOGE(SMARTPA_LIB_TAG "exc_hold_th = %d\n", param->exc_hold_th);
	ALOGE(SMARTPA_LIB_TAG "exc_hold_smpls = %d\n", param->exc_hold_smpls);
	ALOGE(SMARTPA_LIB_TAG "sig_release_rate = %d\n", param->sig_release_rate);
	ALOGE(SMARTPA_LIB_TAG "sig_hold_th = %d\n", param->sig_hold_th);
	ALOGE(SMARTPA_LIB_TAG "max_sig = %d\n", param->max_sig);
	ALOGE(SMARTPA_LIB_TAG "max_exc = %d\n", param->max_exc);
	ALOGE(SMARTPA_LIB_TAG "max_sig_dmg = %d\n", param->max_sig_dmg);
	ALOGE(SMARTPA_LIB_TAG "max_exc_dmg = %d\n", param->max_exc_dmg);
	ALOGE(SMARTPA_LIB_TAG "mode_sel_dmg = %d\n", param->mode_sel_dmg);
	ALOGE(SMARTPA_LIB_TAG "volume = %d\n", param->volume);
	ALOGE(SMARTPA_LIB_TAG "ramp_speed = %d\n", param->ramp_speed);
	ALOGE(SMARTPA_LIB_TAG "tx_act_thres = %d\n", param->tx_act_thres);
	ALOGE(SMARTPA_LIB_TAG "pil_gain = %d\n", param->pil_gain);
	ALOGE(SMARTPA_LIB_TAG "pil_norm_freq = %d\n", param->pil_norm_freq);
	ALOGE(SMARTPA_LIB_TAG "pil_zero_thres = %d\n", param->pil_zero_thres);
	ALOGE(SMARTPA_LIB_TAG "bypass_num = %d\n", param->bypass_num);
	ALOGE(SMARTPA_LIB_TAG "bypass_cnt = %d\n", param->bypass_cnt);
	/* EQ part */
	ALOGE(SMARTPA_LIB_TAG "eq Nbands = %d\n", param->eq.Nbands);
	for (i = 0; i < 10; i++) {
		ALOGE(SMARTPA_LIB_TAG,
			"eq biquad_coeff[%d] = %d, %d, %d, %d, %d\n",
			i,
			param->eq.biquad_coeff[i].b0,
			param->eq.biquad_coeff[i].b1,
			param->eq.biquad_coeff[i].b2,
			param->eq.biquad_coeff[i].a1,
			param->eq.biquad_coeff[i].a2);
	}

	/* MBDRC part */
	ALOGE(SMARTPA_LIB_TAG "mbdrc Nbands = %d\n", param->mbdrc.Nbands);
	ALOGE(SMARTPA_LIB_TAG "mbdrc delay = %d\n", param->mbdrc.delay);
	ALOGE(SMARTPA_LIB_TAG "mbdrc predict_offset = %d\n", param->mbdrc.predict_offset);
	for (i = 0; i < 10; i++) {
		ALOGE(SMARTPA_LIB_TAG,
			"mbdrc biquad_coeff[%d] = %d, %d, %d, %d, %d\n",
			i,
			param->mbdrc.biquad_coeff[i].b0,
			param->mbdrc.biquad_coeff[i].b1,
			param->mbdrc.biquad_coeff[i].b2,
			param->mbdrc.biquad_coeff[i].a1,
			param->mbdrc.biquad_coeff[i].a2);
	}
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc mode_rms[%d] = %d\n", i, param->mbdrc.mode_rms[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc band_skip[%d] = %d\n", i, param->mbdrc.band_skip[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc alpha[%d] = %d\n", i, param->mbdrc.alpha[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc omega[%d] = %d\n", i, param->mbdrc.omega[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc At[%d] = %d\n", i, param->mbdrc.At[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc Rt[%d] = %d\n", i, param->mbdrc.Rt[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc thres0[%d] = %d\n", i, param->mbdrc.thres0[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc thres1[%d] = %d\n", i, param->mbdrc.thres1[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc thres2[%d] = %d\n", i, param->mbdrc.thres2[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc ratio0[%d] = %d\n", i, param->mbdrc.ratio0[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc ratio1[%d] = %d\n", i, param->mbdrc.ratio1[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "mbdrc makeup[%d] = %d\n", i, param->mbdrc.makeup[i]);
	/* SBDRC part */
	ALOGE(SMARTPA_LIB_TAG "sbdrc Nbands = %d\n", param->sbdrc.Nbands);
	ALOGE(SMARTPA_LIB_TAG "sbdrc delay = %d\n", param->sbdrc.delay);
	ALOGE(SMARTPA_LIB_TAG "sbdrc predict_offset = %d\n", param->sbdrc.predict_offset);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc mode_rms[%d] = %d\n", i, param->sbdrc.mode_rms[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc band_skip[%d] = %d\n", i, param->sbdrc.band_skip[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc alpha[%d] = %d\n", i, param->sbdrc.alpha[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc omega[%d] = %d\n", i, param->sbdrc.omega[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc At[%d] = %d\n", i, param->sbdrc.At[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc Rt[%d] = %d\n", i, param->sbdrc.Rt[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc thres0[%d] = %d\n", i, param->sbdrc.thres0[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc thres1[%d] = %d\n", i, param->sbdrc.thres1[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc thres2[%d] = %d\n", i, param->sbdrc.thres2[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc ratio0[%d] = %d\n", i, param->sbdrc.ratio0[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc ratio1[%d] = %d\n", i, param->sbdrc.ratio1[i]);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "sbdrc makeup[%d] = %d\n", i, param->sbdrc.makeup[i]);
	/* General part */
	ALOGE(SMARTPA_LIB_TAG "mul_one = %d\n", param->mul_one);
	for (i = 0; i < 5; i++)
		ALOGE(SMARTPA_LIB_TAG "calib_y[%d] = %d\n", i, param->calib_y[i]);
	ALOGE(SMARTPA_LIB_TAG "calib_dcr = %d\n", param->calib_dcr);
	ALOGE(SMARTPA_LIB_TAG "spk_pro_enable = %d\n", param->spk_pro_enable);
	ALOGE(SMARTPA_LIB_TAG "eq_drc_switch = %d\n", param->eq_drc_switch);
	ALOGE(SMARTPA_LIB_TAG "est_delay = %d\n", param->est_delay);
	ALOGE(SMARTPA_LIB_TAG "fres_set_fres = %d\n", param->fres_set_fres);
	ALOGE(SMARTPA_LIB_TAG "t0 = %d\n", param->t0);
	ALOGE(SMARTPA_LIB_TAG "pro_gain_switch = %d\n", param->pro_gain_switch);
	ALOGE(SMARTPA_LIB_TAG "alpha_speaker = %d\n", param->alpha_speaker);
	ALOGE(SMARTPA_LIB_TAG "max_temperature = %d\n", param->max_temperature);
	ALOGE(SMARTPA_LIB_TAG "eq_drc_ord = %d\n", param->eq_drc_ord);
	ALOGE(SMARTPA_LIB_TAG "imp_est_single_tone_mu1_exp = %d\n", param->imp_est_single_tone_mu1_exp);
	ALOGE(SMARTPA_LIB_TAG "monitor_on = %d\n", param->monitor_on);
	ALOGE(SMARTPA_LIB_TAG "vi_sensing_lpf_en = %d\n", param->vi_sensing_lpf_en);
	ALOGE(SMARTPA_LIB_TAG "deci_ratio; = %d\n", param->deci_ratio);
	ALOGE(SMARTPA_LIB_TAG "deci_filt_param b0 = %d\n", param->deci_filt_param.b0);
	ALOGE(SMARTPA_LIB_TAG "deci_filt_param b1 = %d\n", param->deci_filt_param.b1);
	ALOGE(SMARTPA_LIB_TAG "deci_filt_param b2 = %d\n", param->deci_filt_param.b2);
	ALOGE(SMARTPA_LIB_TAG "deci_filt_param a1 = %d\n", param->deci_filt_param.a1);
	ALOGE(SMARTPA_LIB_TAG "deci_filt_param a2 = %d\n", param->deci_filt_param.a2);
#ifdef __LP64__
	ALOGE(SMARTPA_LIB_TAG "exc_lpf_param b0 = %ld\n", param->exc_lpf_param.b0);
	ALOGE(SMARTPA_LIB_TAG "exc_lpf_param b1 = %ld\n", param->exc_lpf_param.b1);
	ALOGE(SMARTPA_LIB_TAG "exc_lpf_param a1 = %ld\n", param->exc_lpf_param.a1);
#else
	ALOGE(SMARTPA_LIB_TAG "exc_lpf_param b0 = %lld\n", param->exc_lpf_param.b0);
	ALOGE(SMARTPA_LIB_TAG "exc_lpf_param b1 = %lld\n", param->exc_lpf_param.b1);
	ALOGE(SMARTPA_LIB_TAG "exc_lpf_param a1 = %lld\n", param->exc_lpf_param.a1);
#endif /* __LP64__ */
	ALOGE(SMARTPA_LIB_TAG "input_gain = %d\n", param->input_gain);
	ALOGE(SMARTPA_LIB_TAG "version = %d\n", param->version);
	ALOGE(SMARTPA_LIB_TAG "single_tone_update = %d\n", param->single_tone_update);
	ALOGE(SMARTPA_LIB_TAG "sample_rate = %d\n", param->sample_rate);
	ALOGE(SMARTPA_LIB_TAG "tuning_enable = %d\n", param->tuning_enable);
	ALOGE(SMARTPA_LIB_TAG "chip_rev = %d\n", param->chip_rev);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt b0 = %d\n", param->sig_deci_filt.b0);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt b1 = %d\n", param->sig_deci_filt.b1);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt b2 = %d\n", param->sig_deci_filt.b2);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt a1 = %d\n", param->sig_deci_filt.a1);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt a2 = %d\n", param->sig_deci_filt.a2);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt2 b0 = %d\n", param->sig_deci_filt2.b0);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt2 b1 = %d\n", param->sig_deci_filt2.b1);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt2 b2 = %d\n", param->sig_deci_filt2.b2);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt2 a1 = %d\n", param->sig_deci_filt2.a1);
	ALOGE(SMARTPA_LIB_TAG "sig_deci_filt2 a2 = %d\n", param->sig_deci_filt2.a2);
	ALOGE(SMARTPA_LIB_TAG "dump_enable = %d\n", param->dump_enable);
	ALOGE(SMARTPA_LIB_TAG "comp_filt_coef[0] = %d\n", param->comp_filt_coef[0]);
	ALOGE(SMARTPA_LIB_TAG "comp_filt_coef[1] = %d\n", param->comp_filt_coef[1]);
	ALOGE(SMARTPA_LIB_TAG "comp_filt_coef[2] = %d\n", param->comp_filt_coef[2]);
	ALOGE(SMARTPA_LIB_TAG "comp_filt_coef[3] = %d\n", param->comp_filt_coef[3]);
	ALOGE(SMARTPA_LIB_TAG "comp_filt_coef[4] = %d\n", param->comp_filt_coef[4]);
	ALOGE(SMARTPA_LIB_TAG "comp_filt_coef[5] = %d\n", param->comp_filt_coef[5]);
	ALOGE(SMARTPA_LIB_TAG "comp_filt_coef[6] = %d\n", param->comp_filt_coef[6]);
	ALOGE(SMARTPA_LIB_TAG "comp_filt_coef[7] = %d\n", param->comp_filt_coef[7]);
	ALOGE(SMARTPA_LIB_TAG "comp_filt_coef[8] = %d\n", param->comp_filt_coef[8]);
	ALOGE(SMARTPA_LIB_TAG "fir_enable = %d\n", param->fir_enable);
	ALOGD(SMARTPA_LIB_TAG "bypass_lib = %d\n", param->bypass_lib);
	ALOGE(SMARTPA_LIB_TAG "vo_thr_error = %d\n", param->vo_thr_error);
	ALOGE(SMARTPA_LIB_TAG "flags = %x\n", param->flags);
	ALOGE(SMARTPA_LIB_TAG "%s--\n", __func__);
}
#endif /* #ifdef __DEBUG */

static int32_t smartpa_get_nv_calib_dcr(void)
{
	int nvram_ready_retry = 0;
	char nvram_init_val[PROPERTY_VALUE_MAX] = {0};
	F_ID smartpa_calib_fd;
	int file_lid = AP_CFG_CUSTOM_FILE_SMARTPA_CALIB_LID;
	int size, num, result = 0;
	ap_nvram_smartpa_calib_config_struct val;

	ALOGD(SMARTPA_LIB_TAG "%s ++\n", __func__);
	memset(&val, 0, sizeof(ap_nvram_smartpa_calib_config_struct));
	while (nvram_ready_retry++ < 10) {
		property_get("vendor.service.nvram_init", nvram_init_val, NULL);
		if (!strcmp(nvram_init_val, "Ready") ||
			!strcmp(nvram_init_val, "Pre_Ready")) {
			result = 1;
			break;
		}
		/* if not ready, wait for 100ms to retry again */
		usleep(100 * 1000);
	}
	if (!result) {
		ALOGE(SMARTPA_LIB_TAG "nvram ready over retry cnt\n");
		goto direct_default;
	}
	smartpa_calib_fd = NVM_GetFileDesc(file_lid, &size, &num, ISREAD);
	result = read(smartpa_calib_fd.iFileDesc, &val, size * num);
	NVM_CloseFileDesc(smartpa_calib_fd);
	if (result == size * num)
		goto bypass_default;
	ALOGE(SMARTPA_LIB_TAG "result size is not match, use default\n");
direct_default:
	memcpy(&val, &smartpa_calib_ConfigDefault, sizeof(val));
bypass_default:
	ALOGD(SMARTPA_LIB_TAG "calib_dcr = %d\n", val.calib_dcr_val);
	ALOGD(SMARTPA_LIB_TAG "%s --\n", __func__);
	return val.calib_dcr_val;
}

static void smartpa_get_initial_coeff(void)
{
	struct mixer *mixer;
	struct mixer_ctl *ctl;
	int ret;

	/* 0 means default card */
	mixer = mixer_open(0);
	if (!mixer) {
		ALOGE(SMARTPA_LIB_TAG "mixer open fail\n");
		return;
	}
	ctl = mixer_get_ctl_by_name(mixer, "T0_SEL");
	if (!ctl) {
		ALOGE(SMARTPA_LIB_TAG "mixer_ctl open fail\n");
		goto mixer_ctl_fail;
	}
	ret = mixer_ctl_get_value(ctl, 0);
	if (ret < 0)
		goto mixer_ctl_fail;
	ALOGD(SMARTPA_LIB_TAG "t0 coeff [%d]\n", ret);
	default_param.t0 = ret + 22;
	ctl = mixer_get_ctl_by_name(mixer, "Chip_Rev");
	if (!ctl) {
		ALOGE(SMARTPA_LIB_TAG "mixer_ctl open fail\n");
		goto mixer_ctl_fail;
	}
	ret = mixer_ctl_get_value(ctl, 0);
	if (ret < 0)
		goto mixer_ctl_fail;
	ALOGD(SMARTPA_LIB_TAG "chip_rev [%d]\n", ret);
	default_param.chip_rev = ret;
mixer_ctl_fail:
	mixer_close(mixer);
}

#define PUBKEY_LEN	(140)
#define SIG_LEN		(128)
#define DIG_LEN		(SHA256_DIGEST_LENGTH)
/* Current Support Header version ver 1.0 */
#define MIN_HEADER_VER	(0x0100)

static const uint8_t def_pub_key[PUBKEY_LEN] = {
	0x30, 0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xca, 0x46, 0x23, 0xf8, 0xac,
	0x77, 0xcd, 0x0c, 0xa7, 0x3d, 0xbe, 0x27, 0xd7, 0x1d, 0xdc, 0x48, 0x57,
	0x12, 0xfc, 0x39, 0x78, 0xf5, 0x49, 0x99, 0x4e, 0x03, 0x90, 0x3e, 0x6d,
	0xa7, 0xdd, 0x9d, 0x66, 0x78, 0x3d, 0xe8, 0x83, 0x16, 0x15, 0x15, 0x03,
	0x4a, 0x20, 0x99, 0xc1, 0x75, 0x6a, 0xfe, 0x37, 0xae, 0x89, 0x8d, 0xb7,
	0xf5, 0x51, 0xb6, 0xf0, 0xca, 0xd5, 0x9b, 0xd2, 0x91, 0xdb, 0xf9, 0x01,
	0x78, 0x02, 0x8e, 0xdf, 0x23, 0xcb, 0x52, 0x43, 0xb1, 0x8d, 0xde, 0x1a,
	0x7d, 0x0f, 0xce, 0xd8, 0x65, 0xc7, 0x57, 0x04, 0xd8, 0xf6, 0x54, 0xee,
	0x15, 0x62, 0xb1, 0x07, 0x81, 0xd4, 0xf6, 0x08, 0xe7, 0x53, 0xa7, 0xad,
	0x7f, 0x5f, 0x58, 0x62, 0xd2, 0xee, 0xb5, 0x40, 0x9d, 0x0b, 0x83, 0x07,
	0x30, 0xd1, 0x13, 0xba, 0x43, 0x25, 0x56, 0xc7, 0x1f, 0x34, 0xed, 0x80,
	0x6f, 0x50, 0xe7, 0x02, 0x03, 0x01, 0x00, 0x01,
};

static int32_t smartpa_param_sig_output(const uint8_t *msg, int msg_len, const uint8_t *pubkey,
					int pubkey_len, const uint8_t *sig, int sig_len)
{
	BIO *pub_key_bio;
	RSA *rsa_pub_key;
	uint8_t dig_buf[DIG_LEN];
	int ret;

	if (!msg || !pubkey || !sig || pubkey_len < PUBKEY_LEN || sig_len < SIG_LEN)
		return -EINVAL;
	/* msg digest */
	SHA256(msg, msg_len, dig_buf);
	/* RSA verify proceduer */
	pub_key_bio = BIO_new_mem_buf(pubkey, pubkey_len);
	if (!pub_key_bio) {
		ALOGE(SMARTPA_LIB_TAG "bio pub_key fail\n");
		ret = -EINVAL;
		goto invalid_bio;
	}
	rsa_pub_key = d2i_RSAPublicKey_bio(pub_key_bio, NULL);
	if (!rsa_pub_key) {
		ALOGE(SMARTPA_LIB_TAG "pub_key fail\n");
		ret = -EINVAL;
		goto invalid_key;
	}
	ret = RSA_verify(NID_sha256, dig_buf, DIG_LEN, sig, sig_len, rsa_pub_key);
	if (ret <= 0) {
		ALOGE(SMARTPA_LIB_TAG "rsa_verify result [%d]\n", ret);
		ret = -EINVAL;
		goto verify_err;
	}
	return 0;
verify_err:
	RSA_free(rsa_pub_key);
invalid_key:
	BIO_free(pub_key_bio);
invalid_bio:
	return ret;
}

static const uint32_t crc32_table[] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
	0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
	0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
	0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
	0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
	0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
	0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
	0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
	0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
	0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
	0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
	0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
	0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
	0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
	0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
	0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
	0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
	0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
	0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
	0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
	0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
	0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
	0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
	0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
	0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
	0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
	0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
	0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
	0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
	0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
	0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
	0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
	0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
	0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
	0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
	0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
	0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
	0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
	0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
};

static unsigned int crc32(const uint8_t *buf, int len, uint32_t init)
{
	unsigned int crc = init;

	while (len--) {
		crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *buf) & 255];
		buf++;
	}
	return crc;
}

static int32_t smartpa_param_sig_check(const string_buf_t *param_file_path)
{
	uint8_t *msg;
	uint8_t dig_buf[DIG_LEN], f_dig_buf[DIG_LEN];
	struct sec_header *header;
	size_t msg_len;
	char sig_file_path[200];
	uint32_t crc;
	int fd, pfd, ret;

	ALOGD(SMARTPA_LIB_TAG "%s ++\n", __func__);
	ALOGD(SMARTPA_LIB_TAG "file path = %s\n", param_file_path->p_string);
	snprintf(sig_file_path, sizeof(sig_file_path), "%s.sig", param_file_path->p_string);

	fd = open(sig_file_path, O_RDONLY);
	if (fd < 0) {
		ALOGE(SMARTPA_LIB_TAG "could open param sig file\n");
		return -ENOENT;
	}
	ret = lseek(fd, 0, SEEK_END);
	if (ret < (int)sizeof(struct sec_header)) {
		ALOGE(SMARTPA_LIB_TAG "param sig size less than header\n");
		ret = -EINVAL;
		goto invalid_data;
	}
	msg_len = ret;
	lseek(fd, 0, SEEK_SET);
	msg = (uint8_t *)malloc(msg_len);
	if (!msg) {
		ALOGE(SMARTPA_LIB_TAG "alloc msg buffer fail\n");
		ret =  -ENOMEM;
		goto invalid_data;
	}
	ret = read(fd, msg, msg_len);
	header = (struct sec_header *)msg;
	if (header->MSHV > MIN_HEADER_VER) {
		ALOGE(SMARTPA_LIB_TAG "not supported header version\n");
		ret = -EINVAL;
		goto invalid_sig;
	}
	/* user info sig */
	ret = smartpa_param_sig_output(msg + 160, 224, def_pub_key, PUBKEY_LEN,
				       header->uinfo_sig, SIG_LEN);
	if (ret < 0) {
		ALOGE(SMARTPA_LIB_TAG "user info sig fail\n");
		goto invalid_sig;
	}
	/* header info sig */
	ret = smartpa_param_sig_output(msg, 160, header->upub_key, PUBKEY_LEN,
				       header->hinfo_sig, SIG_LEN);
	if (ret < 0) {
		ALOGE(SMARTPA_LIB_TAG "user info sig fail\n");
		goto invalid_sig;
	}
	/* check size */
	if (header->TBS != (sizeof(*header) + header->EHO + header->EHS + header->EDS) ||
		header->TBS != msg_len || header->EDS < DIG_LEN) {
		ALOGE(SMARTPA_LIB_TAG "data size is not match\n");
		ret = -EINVAL;
		goto invalid_sig;
	}
	/* check data crc */
	crc = crc32(msg + sizeof(*header) + header->EHO + header->EHS, header->EDS, -1);
	if (crc != header->CRC) {
		ALOGE(SMARTPA_LIB_TAG "data crc is not match\n");
		ret = -EINVAL;
		goto invalid_sig;
	}
	memcpy(f_dig_buf, msg + sizeof(*header) + header->EHO + header->EHS, DIG_LEN);
	pfd = open(param_file_path->p_string, O_RDONLY);
	if (!pfd) {
		ALOGE(SMARTPA_LIB_TAG "open param file fail\n");
		ret = -ENOENT;
		goto invalid_sig;
	}
	ret = lseek(pfd, 0, SEEK_END);
	msg_len = ret;
	free(msg);
	msg = (uint8_t *)malloc(msg_len);
	if (!msg) {
		ALOGE(SMARTPA_LIB_TAG "alloc param msg buffer fail\n");
		ret = -ENOMEM;
		goto param_msg_fail;
	}
	lseek(pfd, 0, SEEK_SET);
	read(pfd, msg, msg_len);
	SHA256(msg, msg_len, dig_buf);
	if (memcmp(dig_buf, f_dig_buf, DIG_LEN) != 0) {
		ALOGE(SMARTPA_LIB_TAG "param digest not match\n");
		ret = -EINVAL;
		goto param_msg_fail;
	}
	ret = 0;
	ALOGD(SMARTPA_LIB_TAG "%s --\n", __func__);
param_msg_fail:
	close(pfd);
invalid_sig:
	free(msg);
invalid_data:
	close(fd);
	return ret;
}

static status_t smartpa_load_param(
	const string_buf_t	*product_name,
	const string_buf_t	*param_file_path,
	const debug_log_fp_t	debug_log_fp)
{
	int ret;

	smartpa_structMapping = initVarMapping();


	if (product_name == NULL || param_file_path == NULL ||
		debug_log_fp == NULL) {
		ALOGE(SMARTPA_LIB_TAG "input parameter invalid\n");
		return BAD_VALUE;
	}

	/* do sig check for dat file */
	ret = smartpa_param_sig_check(param_file_path);
	if (ret < 0) {
		ALOGE(SMARTPA_LIB_TAG "sig check fail [%d]\n", ret);
		sig_check_fail = 1;
		return NO_ERROR;
	} else
		sig_check_fail = 0;

	if (!init_once) {
		smartpa_get_initial_coeff();
		init_once = 1;
	}

	smartpa_parseResult.clear();
	debug_log_fp(SMARTPA_LIB_TAG "param_path = %s\n", param_file_path->p_string);
	debug_log_fp(SMARTPA_LIB_TAG "param size = %d\n", sizeof(struct spk_pro_parameter));
	if (Parser::readFile(string(param_file_path->p_string), smartpa_structMapping, smartpa_parseResult))
		debug_log_fp(SMARTPA_LIB_TAG "parse file OK\n");
	else
		debug_log_fp(SMARTPA_LIB_TAG "parse file Fail\n");

	return NO_ERROR;
}

static status_t smartpa_parsing_param_file_by_custom_info(
	const arsi_task_config_t *p_arsi_task_config,
	const arsi_lib_config_t  *p_arsi_lib_config,
	const string_buf_t       *product_name,
	const string_buf_t       *param_file_path,
	const string_buf_t	 *custom_info,
	data_buf_t               *p_param_buf,
	const debug_log_fp_t      debug_log_fp)
{
	struct spk_pro_parameter config_param;
	int ret;

	if (p_arsi_task_config == NULL || p_arsi_lib_config == NULL ||
		product_name == NULL || param_file_path == NULL ||
		p_param_buf == NULL || debug_log_fp == NULL ||
		custom_info == NULL) {
		ALOGE(SMARTPA_LIB_TAG "input parameter invalid\n");
		return BAD_VALUE;
	}
	if (p_param_buf->memory_size < sizeof(struct spk_pro_parameter)) {
		ALOGE(SMARTPA_LIB_TAG "data size invalid\n");
		return BAD_VALUE;
	}
	if (!init_once) {
		smartpa_get_initial_coeff();
		init_once = 1;
	}
	memset(&config_param, 0, sizeof(struct spk_pro_parameter));

	if (!sig_check_fail) {
		if (Parser::applyParseRecord((void *)&config_param, &default_param,
			sizeof(struct spk_pro_parameter), smartpa_structMapping,
			smartpa_parseResult, p_arsi_task_config->audio_mode))
			debug_log_fp(SMARTPA_LIB_TAG "apply param OK, mode %d\n", p_arsi_task_config->audio_mode);
		else
			debug_log_fp(SMARTPA_LIB_TAG "apply param fail\n");
	}

	ret = smartpa_get_nv_calib_dcr();
	if (!ret && config_param.spk_pro_enable) {
		debug_log_fp(SMARTPA_LIB_TAG "amp not calibrated\n");
		config_param.pro_gain_switch = 3;
		config_param.volume = -167772160;
	} else
		config_param.calib_dcr = ret;
#ifdef __DEBUG
	print_config_param(&config_param);
#endif /* #ifdef __DEBUG */
	memcpy(p_param_buf->p_buffer, &config_param, sizeof(struct spk_pro_parameter));
	p_param_buf->data_size = sizeof(struct spk_pro_parameter);
	debug_log_fp(SMARTPA_LIB_TAG "data_size = %d\n", p_param_buf->data_size);
	return NO_ERROR;
}

static status_t smartpa_query_param_buf_size_by_custom_info(
	const arsi_task_config_t *p_arsi_task_config,
	const arsi_lib_config_t  *p_arsi_lib_config,
	const string_buf_t       *product_name,
	const string_buf_t       *param_file_path,
	const string_buf_t	 *custom_info,
	uint32_t                 *p_param_buf_size,
	const debug_log_fp_t      debug_log_fp)
{
	if (p_arsi_task_config == NULL || p_arsi_lib_config == NULL ||
		product_name == NULL || param_file_path == NULL ||
		p_param_buf_size == NULL || debug_log_fp == NULL ||
		custom_info == NULL) {
		ALOGE(SMARTPA_LIB_TAG "input parameter invalid\n");
		return BAD_VALUE;
	}
	*p_param_buf_size = sizeof(struct spk_pro_parameter);
	debug_log_fp(SMARTPA_LIB_TAG "buf_size = %d\n", *p_param_buf_size);
	return NO_ERROR;
}

static status_t smartpa_get_lib_version(string_buf_t *version_buf)
{
	/* 1. copy version sting to char buf */
	snprintf(version_buf->p_string,
		 version_buf->memory_size, "%s", SMARTPA_LIB_VER);
	/* 2. put string size into string_size variable */
	version_buf->string_size = strlen(version_buf->p_string);
	return NO_ERROR;
}

/* used */
void dynamic_link_arsi_assign_lib_fp(AurisysLibInterface *lib)
{
    lib->arsi_get_lib_version = smartpa_get_lib_version;
    lib->arsi_query_param_buf_size_by_custom_info = smartpa_query_param_buf_size_by_custom_info;
    lib->arsi_parsing_param_file_by_custom_info = smartpa_parsing_param_file_by_custom_info;
    lib->arsi_load_param = smartpa_load_param;
}

int mtk_smartpa_param_init(struct SmartPAParamOps *mSmartPAParam)
{
    mSmartPAParam->queryParamSize = smartpa_query_param_buf_size_by_custom_info;
    mSmartPAParam->parsingParamFile = smartpa_parsing_param_file_by_custom_info;
    mSmartPAParam->loadParam = smartpa_load_param;
	return 0;
}
