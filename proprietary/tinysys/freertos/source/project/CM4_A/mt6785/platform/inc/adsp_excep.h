/*
 * adsp_excep.h
 *
 *  Created on: 2018/2/2
 *      Author: MTK09865
 */

#ifndef ADSP_EXCEP_H
#define ADSP_EXCEP_H

#define MAX_CHECKED_EXCEP_NUM 19

/*Exception Cause Constants from <xtensa/corebits.h> Header File*/
enum excep_id {
    ID_INSTR_ILLEGAL = 0,
    ID_INSTR_FETCH_ERROR = 2,
    ID_LOAD_STORE_ERROR,
    ID_DIVIDE_BY_ZERO = 6,
    ID_INSTR_PRIVILEGED = 8,
    ID_LOAD_STORE_UNALIGNED,
    ID_PIF_INSTR_DATA_ERROR = 12,
    ID_PIF_LOAD_STORE_DATA_ERROR,
    ID_PIF_INSTR_ADDR_ERROR,
    ID_PIF_LOAD_STORE_ADDR_ERROR,
    ID_ITLB_MISS,
    ID_ITLB_MULTIHIT,
    ID_INSTR_RING,        /*Ring privilege violation on instruction fetch*/
    ID_INSTR_PROHIBITED = 20, /*Cache attribute does not allow instruction fetch*/
    ID_DTLB_MISS = 24,
    ID_DTLB_MULTIHIT,
    ID_LOAD_STORE_RING,
    ID_CACHE_LOAD_PROHIBITED = 28,
    ID_CACHE_STORE_PROHIBITED,
    NR_EXCEP_ID,
};


void adsp_excep_init(void);
void adsp_excep_handler(void);

#endif /*ADSP_EXCEP_H */
