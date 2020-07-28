#ifndef __WO_IF_H__
#define __WO_IF_H__

#include <stddef.h>
#include <stdbool.h>
#include "mal_cfg.h"

typedef struct mal_wo_sim_info_t mal_wo_sim_info_t;

typedef enum mal_wo_sim_type_t {
    MAL_WO_SIM_TYPE_USIM = 0,
    MAL_WO_SIM_TYPE_ISIM = 1,
} mal_wo_sim_type_t;

#define WO_MAX_NUM_PCSCFS    6
#define WO_IPV6_ADDR_BIN_LEN 16

typedef struct mal_wo_pcscfs_t {
    unsigned short num;
    unsigned short v6_mask; /* bit mask => IPV6:1, IPV4:0 */
    unsigned char  addr[WO_MAX_NUM_PCSCFS][WO_IPV6_ADDR_BIN_LEN];
} mal_wo_pcscfs_t;

/** APIs **/
void (* mal_wo_sim_info_free)(mal_ptr_t mal_ptr, mal_wo_sim_info_t* sim_info);
void (* mal_wo_sim_info_set_imsi)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info, const char *imsi, size_t str_len);
void (* mal_wo_sim_info_set_imei)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info, const char *imei, size_t str_len);
void (* mal_wo_sim_info_set_impi)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info, const char *impi, size_t str_len);
void (* mal_wo_sim_info_set_operator)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info, const char *op, size_t str_len);
void (* mal_wo_sim_info_set_ps_capability)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info, bool has_ps_capability);
void (* mal_wo_sim_info_set_sim_type)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info, mal_wo_sim_type_t sim_type);
bool (* mal_wo_sim_ready_notify)(mal_ptr_t mal_ptr, mal_wo_sim_info_t *sim_info);
bool (* mal_wo_sim_rejected_notify)(mal_ptr_t mal_ptr);
mal_wo_sim_info_t* (* mal_wo_sim_info_alloc)(mal_ptr_t mal_ptr, mal_wo_sim_info_t* sim_info);

#endif  /* __WO_IF_H__ */
