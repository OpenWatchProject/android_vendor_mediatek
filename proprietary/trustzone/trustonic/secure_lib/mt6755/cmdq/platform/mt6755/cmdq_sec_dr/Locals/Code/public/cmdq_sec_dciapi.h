#ifndef __CMDQ_SEC_DCIAPI_H__
#define __CMDQ_SEC_DCIAPI_H__

#include "dci.h"


/**
 * DCI Command ID's
 * .ecause we need other drivers' utility to execute secure task and tbase dosen't allow driver talk with each other,
 * we use user DCI (linux kernel-sec driver) to dump info only
 */
#define DCI_CMDQ_DUMP   0    // error dump


/**
 * command message.
 *
 * @param len Lenght of the data to process.
 * @param data Data to be processed
 */
typedef struct {
    dciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process */
} dciCmdqCmd_t;


/**
 * Response structure
 */
typedef struct {
    dciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} dciCmdqRsp_t;

typedef struct {
    uint32_t dummy;
} dciCmdqDataParam_t;

/**
 * DCI message
 */
typedef struct {
    union {
        dciCmdqCmd_t cmd;
        dciCmdqRsp_t rsp;
    };

    dciCmdqDataParam_t data;
} dciCmdqMessage_t;


/**
 * Driver UUID. Update accordingly after reserving UUID
 */

#define DRV_CMDQ_SEC_UUID { { 2, 0xb, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }


#endif // __CMDQ_SEC_DCIAPI_H__
