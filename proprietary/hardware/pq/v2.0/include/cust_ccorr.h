#ifndef __CUST_CCORR_H__
#define __CUST_CCORR_H__

#define MDP_CCORR_COEF_CNT (4)

#define PQ_MDP_CCORR_EN_INDEX_RANGE_NUM     (2)
#define PQ_MDP_CCORR_EN_DEFAULT             "0"    // 0: disable, 1: enable, 2:default
#define PQ_MDP_CCORR_EN_STR                 "persist.vendor.sys.pq.mdp.ccorr.en"

extern const unsigned int g_mdp_ccorr_matrix[MDP_CCORR_COEF_CNT][3][3];

#endif
