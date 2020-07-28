 /*
 * Copyright (c) 2016 Samsung Electronics Ltd.
 * All Rights Reserved.
 * Confidential and Proprietary - Samsung Electronics Ltd.
 */

#ifndef REMOSAIC_ITF_H_
#define REMOSAIC_ITF_H_

#ifdef __cplusplus
extern "C" {
#endif

enum e_remosaic_bayer_order
{
 	BAYER_BGGR = 0,
	BAYER_GBRG = 1,
	BAYER_GRBG = 2,
	BAYER_RGGB = 3,
};

struct st_remosaic_param {
	int16_t wb_r_gain;		//capture WB R Gain
	int16_t wb_gr_gain;		//capture WB GR Gain
	int16_t wb_gb_gain;		//capture WB GB Gain
	int16_t wb_b_gain;		//capture WB B Gain
};

enum {
	RET_OK = 0,
	RET_NG = -1,
};


/*
	void remosaic_init :
	Create REMOSAIC Daemon & Init Daemon
	return none
*/
void remosaic_init(int32_t img_w, int32_t img_h, 
			e_remosaic_bayer_order bayer_order, int32_t pedestal);

/*
	int32_t remosaic_gainmap_gen :
	Make gain map for remosaic alogrithm, call just 1 time
	return result OK, NG
*/
int32_t remosaic_gainmap_gen(void* eep_buf_addr, size_t eep_buf_size);

/*
	int32_t remosaic_process_param_set :
	Set parameter for 4pixel remosaic algorithm
	return none	
*/
void remosaic_process_param_set(struct st_remosaic_param* p_param);
			
/*
	int32_t remosaic_process :
	run 4pixel remosaic algorithm
	return result OK, NG
*/
int32_t remosaic_process(uint8_t* src_buf_addr, size_t src_buf_size, 
					uint8_t* dst_buf_addr, size_t dst_buf_size);

/*
	int32_t remosaic_deinit :
	free all resource and destroy daemon
	return none
*/
void remosaic_deinit();

/*
	int32_t remosaic_debug :
	debug interface
		opt 0 : display remosaic daemon state
	return none
*/
void remosaic_debug(int opt);

#ifdef __cplusplus
}
#endif

#endif // REMOSAIC_ITF_H_

