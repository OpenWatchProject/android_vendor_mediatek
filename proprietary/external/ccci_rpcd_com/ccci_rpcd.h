/*
* Copyright (C) 2011-2017 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __CCCI_RPCD_H__
#define __CCCI_RPCD_H__

#define CCCI_RPC_MAX_BUFFERS        (2)
#define RPC_API_RESP_ID            0xFFFF0000
#define RPC_PARAM_ERROR            -2


typedef enum
{
    /* 0x3001 ~ 0x3012 are reserved for tc1 */

    IPC_RPC_QUERY_AP_SYS_PROPERTY = 0x400F,
} RPC_CCCI_OP_ID;

typedef struct
{
    unsigned length;
    unsigned index;
}rpc_stream_msg_t;


typedef struct
{
    unsigned int rpc_ops_id;
    unsigned char     buffer[0];
} rpc_stream_buffer_t;

typedef struct RPC_PACKET_INFO_STRUCT
{
    unsigned int Length;
    void         *pData;
} RPC_PACKET_INFO;

typedef struct
{
    unsigned int data[2];
    unsigned int channel;
    unsigned int reserved;
} CCCI_BUFF_T;

typedef struct
{
    CCCI_BUFF_T header;
    rpc_stream_buffer_t payload;
}STREAM_DATA;

typedef enum {
    RPC_BUFF_IDLE = 0,   // current port is not waiting for more data
    RPC_BUFF_WAIT        // current port is waiting for more data to come in
} RPC_BUFF_STATUS;
#define RPC_REQ_BUFFER_MUN        5
#define RPC_BUFFER_SLOT_NUM 5
#define MAX_RPC_PKT_BYTE (3584-128)
#define MAX_RPC_BUF_BYTE 4096
#define CCCI_RPC_REQ_SEND_AGAIN 0x80000000
#define CCCI_RPC_PEER_REQ_SEND_AGAIN(_p) (((((CCCI_BUFF_T*)(_p))->data[0] & CCCI_RPC_REQ_SEND_AGAIN) != 0)? 1: 0)

typedef struct
{
    void*           pRpcBuf;
    unsigned char   rpc_buff_state[RPC_BUFFER_SLOT_NUM];
    unsigned int    rpc_buff_offset[RPC_BUFFER_SLOT_NUM];
    int             bNonAck;
}RPC_INFO;

#endif // __CCCI_RPC_H__
