#ifndef RT_SPA_PARAM_H_
#define RT_SPA_PARAM_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define RT_SPA_PARAM_VER	(0x0103)

#define RT_SPA_PARAM_COMPATIBLE_MIN	(0x0101)
#define RT_SPA_PARAM_COMPATIBLE_MAX	(0x0109)

struct param_desc {
        const char *name;
        uint32_t offset;
        uint32_t size;
};

enum {
	PAR_START = 0,
	PAR_SINGLE_START = PAR_START,
	PAR_PARAM_VERSION = PAR_SINGLE_START,
	PAR_PARAM_DATE,
	PAR_DUMP_L,
	PAR_DUMP_R,
	PAR_DCB_FILT_EN,
	PAR_FARROW_INTERP_EN,
	PAR_DELAY_EST_EN,
	PAR_DELAY_EST_MU,
	PAR_DELAY_EST_DIFF_SMPLS,
	PAR_DELAY_EST_MAX_DELAY,
	PAR_DELAY_EST_MIN_DELAY,
	PAR_DELAY_EST_BPF_COS_PHI2,
	PAR_DELAY_EST_BPF_SIN_PHI2,
	PAR_TONE_DET_COS_PHI2,
	PAR_TONE_DET_SIN_PHI2,
	PAR_TONE_DET_MU,
	PAR_TONE_DET_LOG2_DEV ,
	PAR_TONE_DET_SGL_TONE_RATIO,
	PAR_TONE_DET_ACT_THRES,
	PAR_TONE_DET_RECOVERY_SMPLS,
	PAR_RX_ACT_RECOVERY_SMPLS,
	PAR_DCR_EST_MU,
	PAR_DCR_EST_CVG_THRES0,
	PAR_DCR_EST_CVG_THRES1,
	PAR_ATC_KP,
	PAR_ATC_KI,
	PAR_ATC_KD,
	PAR_MAX_DCR,
	PAR_MAX_DCR_DMG,
	PAR_MIN_ATC_GAIN,
	PAR_IMP_EST_ORD,
	PAR_IMP_EST_WIN,
	PAR_IMP_EST_MU0_EXP,
	PAR_IMP_EST_MU1_EXP,
	PAR_FRES_DAMAGE_RATIO,
	PAR_FRES_RECOVERY_SMPLS,
	PAR_FRES_STEPSIZE,
	PAR_EXC_RELEASE_RATE,
	PAR_EXC_HOLD_TH,
	PAR_EXC_HOLD_SMPLS,
	PAR_SIG_RELEASE_RATE,
	PAR_SIG_HOLD_TH,
	PAR_MAX_SIG,
	PAR_MAX_EXC,
	PAR_MAX_SIG_DMG,
	PAR_MAX_EXC_DMG,
	PAR_MODE_SEL_DMG,
	PAR_VOLUME,
	PAR_RAMP_SPEED,
	PAR_TX_ACT_THRES,
	PAR_PIL_GAIN,
	PAR_PIL_NORM_FREQ,
	PAR_PIL_ZERO_THRES,
	PAR_BYPASS_NUM,
	PAR_BYPASS_CNT,
	PAR_MUL_ONE,
	PAR_CALIB_DCR,
	PAR_SPK_PRO_ENABLE,
	PAR_EQ_DRC_SWITCH,
	PAR_EST_DELAY,
	PAR_FRES_SET_FRES,
	PAR_T0,
	PAR_PRO_GAIN_SWITCH,
	PAR_ALPHA_SPEAKER,
	PAR_MAX_TEMPERATURE,
	PAR_EQ_DRC_ORD,
	PAR_IMP_EST_SINGLE_TONE_MU1_exp,
	PAR_MONITOR_ON,
	PAR_VI_SENSING_LPF_EN,
	PAR_DECI_RATIO,
	PAR_INPUT_GAIN,
	PAR_VERSION,
	PAR_SINGLE_TONE_UPDATE,
	PAR_SAMPLE_RATE,
	PAR_TUNING_ENABLE,
	PAR_MONITOR_DATA,
	PAR_CHIP_REV,
	PAR_DUMP_ENABLE,
	PAR_FIR_ENABLE,
	PAR_BYPASS_LIB,
	PAR_VO_THR_ERROR,
	PAR_FLAGS,
	PAR_KD_THRES,
	PAR_HP_FILTER_EN,
	PAR_SBDRC_PRE_GAIN,
	PAR_DC_TERM_THRES,
	PAR_DC_TERM_DETECT_SMPLS,
	PAR_DC_TERM_EN,
	PAR_PLMT_RAMP_SPEED,
	PAR_PLMT_LIM_EN,
	PAR_SAFEGUARD_PO_MIN,
	PAR_SAFEGUARD_SLOPE,
	PAR_SAFEGUARD_HOLD_SMPLS,
	PAR_SAFEGUARD_RELEASE_RATE,
	PAR_SAFEGUARD_HOLD_TH,
	PAR_SAFEGUARD_VH,
	PAR_SAFEGUARD_CLIP_THRES,
	PAR_SAFEGUARD_RAMP_DOWN_SPEED,
	PAR_SAFEGUARD_EN,
	PAR_PWR_ACT_THRES,
	PAR_SAFEGUARD_RAMP_UP_SPEED,
	PAR_PLMT_LIMIT_DCR,
	PAR_SAFEGUARD_PEAK_DET_MODE,
	PAR_PLMT_MIN_DCR,
	PAR_VVALIDATION_SIGNAL_RATIO,
	PAR_VVALIDATION_ERROR_FLAG,
	PAR_VVALIDATION_MAX_PWR,
	PAR_VVALIDATION_MIN_PWR,
	PAR_VVALIDATION_PWR_REAL,
	PAR_PCB_TRACE,
	PAR_SINGLE_END = PAR_PCB_TRACE,

	PAR_ARRAY_START = 200,
	PAR_CALIB_Y = PAR_ARRAY_START,
	PAR_COMP_FILT_COEF,
	PAR_MBDRC_PRE_GAIN,
	PAR_ARRAY_END = PAR_MBDRC_PRE_GAIN,

	PAR_STRUCT_START = 300,
	PAR_DCB_FILT_B0 = PAR_STRUCT_START,
	PAR_DCB_FILT_B1,
	PAR_DCB_FILT_A1,
	PAR_EXC_LPF_B0,
	PAR_EXC_LPF_B1,
	PAR_EXC_LPF_A1,
	PAR_PIL_FILT_B0,
	PAR_PIL_FILT_B1,
	PAR_PIL_FILT_B2,
	PAR_PIL_FILT_A1,
	PAR_PIL_FILT_A2,
	PAR_SIG_FILT_B0,
	PAR_SIG_FILT_B1,
	PAR_SIG_FILT_B2,
	PAR_SIG_FILT_A1,
	PAR_SIG_FILT_A2,
	PAR_DECI_FILT_B0,
	PAR_DECI_FILT_B1,
	PAR_DECI_FILT_B2,
	PAR_DECI_FILT_A1,
	PAR_DECI_FILT_A2,
	PAR_SIG_DECI_FILT_B0,
	PAR_SIG_DECI_FILT_B1,
	PAR_SIG_DECI_FILT_B2,
	PAR_SIG_DECI_FILT_A1,
	PAR_SIG_DECI_FILT_A2,
	PAR_SIG_DECI_FILT2_B0,
	PAR_SIG_DECI_FILT2_B1,
	PAR_SIG_DECI_FILT2_B2,
	PAR_SIG_DECI_FILT2_A1,
	PAR_SIG_DECI_FILT2_A2,
	PAR_HP_FILTER_COEF,
	PAR_EQ_NBANDS,
	PAR_EQ_FILT_0,
	PAR_EQ_FILT_5,
	PAR_MBDRC_NBANDS,
	PAR_MBDRC_DELAY,
	PAR_MBDRC_PREDICT_OFFSET,
	PAR_MBDRC_FILT_0,
	PAR_MBDRC_FILT_5,
	PAR_MBDRC_MODE_RMS_0,
	PAR_MBDRC_MODE_RMS_1,
	PAR_MBDRC_MODE_RMS_2,
	PAR_MBDRC_MODE_RMS_3,
	PAR_MBDRC_MODE_RMS_4,
	PAR_MBDRC_BAND_SKIP_0,
	PAR_MBDRC_BAND_SKIP_1,
	PAR_MBDRC_BAND_SKIP_2,
	PAR_MBDRC_BAND_SKIP_3,
	PAR_MBDRC_BAND_SKIP_4,
	PAR_MBDRC_ALPHA_0,
	PAR_MBDRC_ALPHA_1,
	PAR_MBDRC_ALPHA_2,
	PAR_MBDRC_ALPHA_3,
	PAR_MBDRC_ALPHA_4,
	PAR_MBDRC_OMEGA_0,
	PAR_MBDRC_OMEGA_1,
	PAR_MBDRC_OMEGA_2,
	PAR_MBDRC_OMEGA_3,
	PAR_MBDRC_OMEGA_4,
	PAR_MBDRC_AT_0,
	PAR_MBDRC_AT_1,
	PAR_MBDRC_AT_2,
	PAR_MBDRC_AT_3,
	PAR_MBDRC_AT_4,
	PAR_MBDRC_RT_0,
	PAR_MBDRC_RT_1,
	PAR_MBDRC_RT_2,
	PAR_MBDRC_RT_3,
	PAR_MBDRC_RT_4,
	PAR_MBDRC_THRES0_0,
	PAR_MBDRC_THRES0_1,
	PAR_MBDRC_THRES0_2,
	PAR_MBDRC_THRES0_3,
	PAR_MBDRC_THRES0_4,
	PAR_MBDRC_THRES1_0,
	PAR_MBDRC_THRES1_1,
	PAR_MBDRC_THRES1_2,
	PAR_MBDRC_THRES1_3,
	PAR_MBDRC_THRES1_4,
	PAR_MBDRC_THRES2_0,
	PAR_MBDRC_THRES2_1,
	PAR_MBDRC_THRES2_2,
	PAR_MBDRC_THRES2_3,
	PAR_MBDRC_THRES2_4,
	PAR_MBDRC_RATIO0_0,
	PAR_MBDRC_RATIO0_1,
	PAR_MBDRC_RATIO0_2,
	PAR_MBDRC_RATIO0_3,
	PAR_MBDRC_RATIO0_4,
	PAR_MBDRC_RATIO1_0,
	PAR_MBDRC_RATIO1_1,
	PAR_MBDRC_RATIO1_2,
	PAR_MBDRC_RATIO1_3,
	PAR_MBDRC_RATIO1_4,
	PAR_MBDRC_MAKEUP_0,
	PAR_MBDRC_MAKEUP_1,
	PAR_MBDRC_MAKEUP_2,
	PAR_MBDRC_MAKEUP_3,
	PAR_MBDRC_MAKEUP_4,
	PAR_SBDRC_NBANDS,
	PAR_SBDRC_DELAY,
	PAR_SBDRC_PREDICT_OFFSET,
	PAR_SBDRC_MODE_RMS,
	PAR_SBDRC_BAND_SKIP,
	PAR_SBDRC_ALPHA,
	PAR_SBDRC_OMEGA,
	PAR_SBDRC_AT,
	PAR_SBDRC_RT,
	PAR_SBDRC_THRES0,
	PAR_SBDRC_THRES1,
	PAR_SBDRC_THRES2,
	PAR_SBDRC_RATIO0,
	PAR_SBDRC_RATIO1,
	PAR_SBDRC_MAKEUP,
	PAR_DC_TERM_FILT_B0,
	PAR_DC_TERM_FILT_B1,
	PAR_DC_TERM_FILT_A0,
	PAR_BIG_DATA,
	PAR_STRUCT_END = PAR_BIG_DATA,
	PAR_END,

	/* for algorithm registers */
	REG_START = 2000,
	REG_GAIN_EST_LMT_GAIN = REG_START,
	REG_ATC_GAIN,
	REG_GAIN_EST_TOT_GAIN,
	REG_SIG_PEAK_PEAK,
	REG_EXC_PEAK_PEAK,
	REG_DELAY_EST_REG_DELAY,
	REG_DCR_EST_REG_DCR,
	REG_DCR_DAMAGE,
	REG_FRES_DAMAGE,
	REG_TONE_DET_ACTIVE,
	REG_TONE_DET_SGL_TONE,
	REG_PIL_GEN_PIL_ACTIVE,
	REG_FRES_REG_FRES,
	REG_IMP_EST_REG_Y,
	REG_IMP_EST_CUR_GAIN,
	REG_DC_TERM_DETECTED,
	REG_VBAT,
	REG_CLIP_GAIN,
	REG_DCR,
	REG_END,
};

#pragma pack(push, 1)

enum {
	BIG_DATA_T_THRES,
	BIG_DATA_X_THRES,
	BIG_DATA_X_MSCALE,
	BIG_DATA_NR,
};

struct big_data {
	int32_t data[BIG_DATA_NR];
};

struct iir_1st_filt_param {
	int64_t b0;
	int64_t b1;
	int64_t a1;
};

struct iir_2nd_filt_param {
	int32_t b0;
	int32_t b1;
	int32_t b2;
	int32_t a1;
	int32_t a2;
};

struct param_eq
{
	int32_t Nbands;
	struct iir_2nd_filt_param biquad_coeff[10];
};

struct param_mbdrc {
	int32_t Nbands;
	int32_t delay;
	int32_t predict_offset;
	struct iir_2nd_filt_param biquad_coeff[10];
	int32_t mode_rms[5];
	int32_t band_skip[5];
	int32_t alpha[5];
	int32_t omega[5];
	int32_t At[5];
	int32_t Rt[5];
	int32_t thres0[5];
	int32_t thres1[5];
	int32_t thres2[5];
	int32_t ratio0[5];
	int32_t ratio1[5];
	int32_t makeup[5];
};

typedef struct spk_pro_parameter {
	int32_t param_version;
	int32_t param_date;
	int32_t dump_l;
	int32_t dump_r;
	int32_t dcb_filt_en;
	int32_t farrow_interp_en;
	int32_t delay_est_en;
	int32_t delay_est_mu;
	int32_t delay_est_diff_smpls;
	int32_t delay_est_max_delay;
	int32_t delay_est_min_delay;
	int32_t delay_est_bpf_cos_phi2;
	int32_t delay_est_bpf_sin_phi2;
	int32_t tone_det_cos_phi2;
	int32_t tone_det_sin_phi2;
	int32_t tone_det_mu;
	int32_t tone_det_log2_dev ;
	int32_t tone_det_sgl_tone_ratio;
	int32_t tone_det_act_thres;
	int32_t tone_det_recovery_smpls;
	int32_t rx_act_recovery_smpls;
	int32_t dcr_est_mu;
	int32_t dcr_est_cvg_thres0;
	int32_t dcr_est_cvg_thres1;
	int32_t atc_kp;
	int32_t atc_ki;
	int32_t atc_kd;
	int32_t max_dcr;
	int32_t max_dcr_dmg;
	int32_t min_atc_gain;
	int32_t imp_est_ord;
	int32_t imp_est_win;
	int32_t imp_est_mu0_exp;
	int32_t imp_est_mu1_exp;
	int32_t fres_damage_ratio;
	int32_t fres_recovery_smpls;
	int32_t fres_stepsize;
	int32_t exc_release_rate;
	int32_t exc_hold_th;
	int32_t exc_hold_smpls;
	int32_t sig_release_rate;
	int32_t sig_hold_th;
	int32_t max_sig;
	int32_t max_exc;
	int32_t max_sig_dmg;
	int32_t max_exc_dmg;
	int32_t mode_sel_dmg;
	int32_t volume;
	int32_t ramp_speed;
	int32_t tx_act_thres;
	int32_t pil_gain;
	int32_t pil_norm_freq;
	int32_t pil_zero_thres;
	int32_t bypass_num;
	int32_t bypass_cnt;
	int32_t mul_one;
	int32_t calib_dcr;
	int32_t spk_pro_enable;
	int32_t eq_drc_switch;
	int32_t est_delay;
	int32_t fres_set_fres;
	int32_t t0;
	int32_t pro_gain_switch;
	int32_t alpha_speaker;
	int32_t max_temperature;
	int32_t eq_drc_ord;
	int32_t imp_est_single_tone_mu1_exp;
	int32_t monitor_on;
	int32_t vi_sensing_lpf_en;
	int32_t deci_ratio;
	int32_t input_gain;
	int32_t version;
	int32_t single_tone_update;
	int32_t sample_rate;
	int32_t tuning_enable;
	int32_t monitor_data;
	int32_t chip_rev;
	int32_t dump_enable;
	int32_t fir_enable;
	int32_t bypass_lib;
	int32_t vo_thr_error;
	int32_t flags;
	int32_t kd_thres;
	int32_t hp_filter_en;
	int32_t sbdrc_pre_gain;
	int32_t dc_term_thres;
	int32_t dc_term_detect_smpls;
	int32_t dc_term_en;
	int32_t plmt_ramp_speed;
	int32_t plmt_lim_en;
	int32_t safeguard_po_min;
	int32_t safeguard_slope;
	int32_t safeguard_hold_smpls;
	int32_t safeguard_release_rate;
	int32_t safeguard_hold_th;
	int32_t safeguard_vh;
	int32_t safeguard_clip_thres;
	int32_t safeguard_ramp_down_speed;
	int32_t safeguard_en;
	int32_t pwr_act_thres;
	int32_t safeguard_ramp_up_speed;
	int32_t plmt_limit_dcr;
	int32_t safeguard_peak_det_mode;
	int32_t plmt_min_dcr;
	int32_t vvalidation_signal_ratio;
	int32_t vvalidation_error_flag;
	int32_t vvalidation_max_pwr;
	int32_t vvalidation_min_pwr;
	int32_t vvalidation_pwr_real;
	int32_t pcb_trace;

	int32_t calib_y[5];
	int32_t comp_filt_coef[9];
	int32_t mbdrc_pre_gain[5];

	struct iir_1st_filt_param dcb_filt;
	struct iir_1st_filt_param exc_lpf_param;
	struct iir_2nd_filt_param pil_filt_param;
	struct iir_2nd_filt_param sig_filt_param;
	struct iir_2nd_filt_param deci_filt_param;
	struct iir_2nd_filt_param sig_deci_filt;
	struct iir_2nd_filt_param sig_deci_filt2;
	struct iir_2nd_filt_param hp_filter;
	struct param_eq eq;
	struct param_mbdrc mbdrc;
	struct param_mbdrc sbdrc;
	struct iir_1st_filt_param dc_term_filt;
	struct big_data big_data;

} spk_prot_algo_param_t;

#pragma pack(pop)

#endif /* RT_SPA_PARAM_H_ */
