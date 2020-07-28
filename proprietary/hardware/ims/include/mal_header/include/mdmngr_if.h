#ifndef __MDMNGR_IF_H__
#define __MDMNGR_IF_H__

#include "mal_cfg.h"

typedef enum mal_md_id
{
    MAL_MD_ID_NONE = -1,
    MAL_MD_ID_GSM,
    MAL_MD_ID_C2K
}mal_md_id_t ;

typedef enum mal_sim_id
{
    MAL_SIM_ID_NONE = -1,
    MAL_SIM_ID_0,
    MAL_SIM_ID_1,
    MAL_SIM_ID_2,
    MAL_SIM_ID_3
}mal_sim_id_t;

typedef enum {
    RILD_URC_GSM,
    RILD_URC_C2K,
} mal_md_type_t ;

typedef enum {
//    MAL_SIM_ID_0,
//    MAL_SIM_ID_1,
    MAL_SIM_UNDEF = 2,
} mal_slot_id_t;

/** APIs **/
int (* mal_mdmngr_send_urc)(mal_ptr_t mal_ptr, const char *urc);

#endif
