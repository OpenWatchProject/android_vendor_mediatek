#ifndef _MT_L2C_H_
#define _MT_L2C_H_

#define L3_SHARE_EN	9
#define L3_SHARE_PRE_EN	8

enum options {
	BORROW_L2,
	RETURN_L2,
	BORROW_NONE
};

struct _l2c_share_info {
	uint32_t share_cluster_num;
	uint32_t cluster_borrow;
	uint32_t cluster_return;
};

void config_L2_size(void);

#endif
