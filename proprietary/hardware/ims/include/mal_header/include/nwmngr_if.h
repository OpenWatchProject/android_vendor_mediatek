#ifndef __NWMNGR_IF_H__
#define __NWMNGR_IF_H__

#include "mal_cfg.h"

/* ------------------------------------ */
/*  Network Manager                     */
/* ------------------------------------ */

typedef struct _nwmngr_ps_reg_state {
    int   sim_id;
    char raw_data[128];
} nwmngr_ps_reg_state_t;

typedef struct _nwmngr_data_reg_state_ {
    int status;
    int state;
    char lac[8];
    char ci[12];
    int act;
} nwmngr_data_reg_state_resp_t;

typedef struct _nwmngr_wfc_supported_ {
    int sim_id;
    int is_wfc_supported;
} nwmngr_wfc_supported_t;

#define NWMNGR_MAX_SIM (4)
#define NWMNGR_SIM_ID_INVALID(sim_id) ((sim_id) < 0 || (sim_id) >= NWMNGR_MAX_SIM)

typedef struct {
    int sim_id;
    unsigned int rat_type;
    unsigned int serial_no;
} nw_trigger_get_cell_info_t;

/** APIs **/
int (* mal_nwmgnr_set_wfc_supported)(mal_ptr_t mal_ptr, int is_wfc_supported);


#endif /* __NWMNGR_IF_H__ */
