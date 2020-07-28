#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "../include/debug.h"
#include "../include/rcs_proxy_session_handler.h"


/* callback for the SIP message receiving from DISPatcher */

/*****************************************************************************
 * FUNCTION
 *  rcse_rds_conn_init
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  void
 *****************************************************************************/
void rcse_rds_conn_init() {
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    rds_ru_set_callstat_req_t data;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (rds_set_rcsstat == 0) {
        ErrMsg("rcs_rds_conn_init, no MAL lib!");
        return;
    }
    DbgMsg("rcs_rds_conn_init start");

    memset(&data, 0, sizeof(rds_ru_set_callstat_req_t));
    data.call_stat = WOS_RESET;
    data.call_type = WOS_IPME;
    rds_set_rcsstat(&data);

    DbgMsg("rcs_rds_conn_init end");

    return;
}
/*****************************************************************************
 * FUNCTION
 *  rcse_rds_conn_active
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  void
 *****************************************************************************/
void rcse_rds_conn_active() {
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    rds_ru_set_callstat_req_t data;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (rds_set_rcsstat == 0) {
        ErrMsg("rcs_rds_conn_active, no MAL lib!");
        return;
    }
    DbgMsg("rcs_rds_conn_active start");

    memset(&data, 0, sizeof(rds_ru_set_callstat_req_t));
    data.call_stat = WOS_ACT_CALL;
    data.call_type = WOS_IPME;
    rds_set_rcsstat(&data);

    DbgMsg("rcs_rds_conn_active end");

    return;
}

/*****************************************************************************
 * FUNCTION
 *  rcse_rds_conn_inactive
 * DESCRIPTION
 *
 * PARAMETERS
 *
 * RETURNS
 *  void
 *****************************************************************************/
 void rcse_rds_conn_inactive() {
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    rds_ru_set_callstat_req_t data;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (rds_set_rcsstat == 0) {
        ErrMsg("rcse_rds_conn_inactive, no MAL lib!");
        return;
    }
    DbgMsg("rcse_rds_conn_inactive start");

    memset(&data, 0, sizeof(rds_ru_set_callstat_req_t));
    data.call_stat = WOS_END_CALL;
    data.call_type = WOS_IPME;
    rds_set_rcsstat(&data);

    DbgMsg("rcs_rds_conn_inactive end");

    return;
}