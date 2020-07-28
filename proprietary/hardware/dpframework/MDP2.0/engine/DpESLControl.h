#ifndef __DP_ESL_CONTROL_H__
#define __DP_ESL_CONTROL_H__

#include "DpDataType.h"
#include "DpCommand.h"
#include "DpLogger.h"
#include "mdp_reg_rdma.h"
#include "mdp_reg_wrot.h"
#include "mdp_reg_wdma.h"

DP_STATUS_ENUM setESLRDMA(DpCommand &command,
                          uint32_t identifier,
                          DpColorFormat colorFormat);

DP_STATUS_ENUM setESLWROT(DpCommand &command,
                          uint32_t identifier,
                          DpColorFormat colorFormat);

DP_STATUS_ENUM setESLWDMA(DpCommand &command,
                          DpColorFormat colorFormat);

#endif  // __DP_ESL_CONTROL_H__