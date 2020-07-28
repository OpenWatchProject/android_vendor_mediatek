#ifndef __MAL_H__
#define __MAL_H__

#include "mal_cfg.h"
#include "datamngr_if.h"
#include "mdmngr_if.h"
#include "nwmngr_if.h"
#include "rds_if.h"
#include "wo_if.h"
#include "rilproxy_if.h"
#include "mal_ril.h"

typedef void (*mal_status_cb_t) (void *arg);

/** APIs **/
int (* mal_status_reg_restart_cb)(mal_ptr_t mal_ptr, mal_status_cb_t cb, void *arg);
mal_ptr_t (* mal_set)(mal_ptr_t mal_ptr, unsigned int num_of_cfg, ...);
mal_ptr_t (* mal_reset)(mal_ptr_t mal_ptr);
int (* rds_set_rcsstat)(rds_ru_set_callstat_req_t *);

#endif
