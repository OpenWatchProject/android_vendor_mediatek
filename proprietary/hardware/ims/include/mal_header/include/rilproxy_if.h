#ifndef __RILPROXY_IF_H__
#define __RILPROXY_IF_H__

#include "datamngr_if.h"    // NEED_TO_BE_NOTICED, to be removed from the common part

typedef struct rilproxy_hdr
{
    int sim_id;
}rilproxy_hdr_t;
typedef struct rilproxy_hdr* rilproxy_hdr_ptr_t;

typedef dm_resp_t mal_rilproxy_pcscf_pco_rsp_t;
typedef dm_resp_t* mal_rilproxy_pcscf_pco_rsp_ptr_t;

typedef dm_resp_t mal_rilproxy_last_fail_cause_rsp_t;
typedef dm_resp_t* mal_rilproxy_last_fail_cause_rsp_ptr_t;

/** APIs **/
int (* mal_rilproxy_resp_data_call_info_tlv)(mal_ptr_t mal_ptr, int serial_no, mal_datamngr_data_call_info_req_ptr_t req_ptr, mal_datamngr_data_call_info_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);
int (* mal_rilproxy_resp_pcscf_pco_tlv)(mal_ptr_t mal_ptr, int serial_no, char *pcscf_str, mal_rilproxy_pcscf_pco_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);
int (* mal_rilproxy_resp_last_fail_cause_tlv)(mal_ptr_t mal_ptr, int serial_no, int fail_cause, mal_rilproxy_last_fail_cause_rsp_ptr_t rsp_ptr,
        int num_req_tlv, void *req_tlv, int *num_rsp_tlv, void *rsp_tlv);

#endif //__RILPROXY_IF_H__
