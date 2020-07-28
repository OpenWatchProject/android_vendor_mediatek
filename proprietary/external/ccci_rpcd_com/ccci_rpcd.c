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

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/vfs.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <wchar.h>
#include <linux/ioctl.h>
#include <ctype.h>
//#include <cutils/properties.h>
#include "hardware/ccci_intf.h"
//#include <hardware_legacy/power.h>
#include <assert.h>
//#define RPC_WAKE_LOCK_NAME "ccci_rpc"
//#define RPC_WAKE_LOCK() acquire_wake_lock(PARTIAL_WAKE_LOCK, RPC_WAKE_LOCK_NAME)
//#define RPC_WAKE_UNLOCK() release_wake_lock(RPC_WAKE_LOCK_NAME)

//#include <cutils/log.h>
#include "ccci_rpcd_platform.h"
#include "ccci_rpcd.h"


//#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "ccci_rpcd",__VA_ARGS__)
//#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "ccci_rpcd",__VA_ARGS__)
//#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "ccci_rpcd",__VA_ARGS__)
//#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "ccci_rpcd",__VA_ARGS__)
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "ccci_rpcd",__VA_ARGS__)

static int          stream_support = 0;
static int          DeviceFd = 0;
static unsigned int RPC_MAX_BUF_SIZE = 2048;
static unsigned int RPC_MAX_ARG_NUM = 6;
static int          md_id = 0;
static RPC_INFO     g_RpcInfo;


static int RPC_GetPackInfo(RPC_PACKET_INFO* pPackInfo, unsigned char* pData)
{
    unsigned int PackNum = *((unsigned int*)pData);
    unsigned int Index = 0;
    unsigned int i;

    if(PackNum > RPC_MAX_ARG_NUM)
        return false;

    Index = sizeof(unsigned int);
    for(i = 0; i < PackNum; i++)
    {
        pPackInfo[i].Length = *((unsigned int*)(pData + Index));
        Index += sizeof(unsigned int);
        pPackInfo[i].pData = (pData + Index);
        //4 byte alignment
        Index += ((pPackInfo[i].Length+3)>>2)<<2;
    }

    if(Index > RPC_MAX_BUF_SIZE)
        return false;

    return true;
}

// 5 = CCCI header + Operation ID
unsigned int g_bak[5];

/*
 * @brief Prepare a packet buffer for sending to MD
 * @param
 *     pData [in] A pointer to argument data for sending
 *     data_to_send [in] Size in bytes of argument data to send
 *     ccci_src [in] The pointer to the CCCI header for every sub-packet
 *     op_id [in] Operation ID currently used
 *     again [in] A flag means if we need to set "send again indicator"
 * @return
 *     On success, a pointer to arguments data is returned.
 *     On error, NULL is returned.
 */
void *RPC_PreparePktEx(unsigned char *pData, unsigned int data_to_send, CCCI_BUFF_T *ccci_src, unsigned int op_id, unsigned int again)
{
    char *bak_ptr = NULL;
    STREAM_DATA *stream = NULL;

    assert(data_to_send <= MAX_RPC_PKT_BYTE);
    assert(pData != NULL && ccci_src != NULL);
    assert(pData - (sizeof(CCCI_BUFF_T) + sizeof(unsigned int)) >= ccci_src);
        assert(sizeof(g_bak) == (sizeof(CCCI_BUFF_T) + sizeof(unsigned int)));
    // move pointer forward to fill in CCCI header, this will replace orignal data there, so we backup them first
    bak_ptr = (char *)(pData - (sizeof(CCCI_BUFF_T) + sizeof(unsigned int)));
    // backup partial data
    memcpy((void*)g_bak, bak_ptr, sizeof(g_bak));
    stream = (STREAM_DATA *)bak_ptr;
    // copy CCCI header from the very fist header of all sub-packets
    if (again)
        stream->header.data[0] = ccci_src->data[0] | CCCI_RPC_REQ_SEND_AGAIN;
    else
        stream->header.data[0] = ccci_src->data[0] & ~CCCI_RPC_REQ_SEND_AGAIN;
    stream->header.data[1] = data_to_send + sizeof(CCCI_BUFF_T) + sizeof(unsigned int);;
    stream->header.channel = ccci_src->channel;
    stream->header.reserved = ccci_src->reserved;
    stream->payload.rpc_ops_id = op_id;

    //LOGD("RPC_PreparePktEx() CCCI_H(0x%X)(0x%X)(0x%X)(0x%X), OP ID = 0x%X",
    //     stream->header.data[0], stream->header.data[1], stream->header.channel, stream->header.reserved,
    //     stream->payload.OperateID);

    return (void*)stream;
}

/*
 * @brief Determine the prepare data has done
 * @param
 *     pStream [in] A pointer returned from RPC_PreparePktEx()
 * @return
 *     None
 */
void RPC_PreparePktDone(void *pStream)
{
    assert(pStream != NULL);
    // Restore backuped data
    memcpy(pStream, (void*)g_bak, sizeof(g_bak));
}

static bool RPC_WriteToMD(int DeviceFd, int BufIndex, RPC_PACKET_INFO* pPacketSrc, unsigned int PacketNum)
{
    bool bRet = false;
    int ret = 0;
    unsigned char* pData;
    unsigned int DataLength = 0, AlignLength;
    unsigned int i;
    rpc_stream_buffer_t *pRpcBuf;
    rpc_stream_msg_t stream_msg;

    int data_len = 0;
    int data_sent = 0;
    STREAM_DATA *buffer_slot;
    CCCI_BUFF_T *ccci_h = NULL;
    void *pkt_ptr = NULL;
    int pkt_size = 0;
    int data_to_send = 0;
    if(!stream_support) {
        pRpcBuf = (rpc_stream_buffer_t *)((char *)g_RpcInfo.pRpcBuf + (RPC_MAX_BUF_SIZE + sizeof(rpc_stream_msg_t))*BufIndex);
    } else {
        buffer_slot = (STREAM_DATA *)((char *)g_RpcInfo.pRpcBuf + (RPC_MAX_BUF_SIZE + sizeof(STREAM_DATA))*BufIndex);
        pRpcBuf = &buffer_slot->payload;
        DataLength += sizeof(CCCI_BUFF_T);
        DataLength += sizeof(unsigned int); // size of operate ID field
    }
    pRpcBuf->rpc_ops_id = RPC_API_RESP_ID | pRpcBuf->rpc_ops_id;
    pData = (unsigned char* )pRpcBuf->buffer;

    *((unsigned int*)pData) = PacketNum;

    pData += sizeof(unsigned int);
    DataLength += sizeof(unsigned int);

    for(i = 0; i < PacketNum; i++)
    {
        if((DataLength + 2*sizeof(unsigned int) + pPacketSrc[i].Length) > RPC_MAX_BUF_SIZE)
        {
            LOGE("RPCD_WriteToMD: Stream buffer full!!\r\n");
            goto _Exit;
        }
        *((unsigned int*)pData) = pPacketSrc[i].Length;
        pData += sizeof(unsigned int);
        DataLength += sizeof(unsigned int);

        //4 byte aligned
        AlignLength = ((pPacketSrc[i].Length + 3) >> 2) << 2;
        DataLength += AlignLength;

        if(pData != pPacketSrc[i].pData)
            memcpy(pData, pPacketSrc[i].pData, pPacketSrc[i].Length);

        pData += AlignLength;
    }

    stream_msg.length = DataLength;
    stream_msg.index = BufIndex;

    if(!stream_support) {
        msync(pRpcBuf, RPC_MAX_BUF_SIZE, MS_SYNC);
        ret = ioctl(DeviceFd, CCCI_RPC_IOCTL_SEND, &stream_msg);
        if(ret < 0) {
            LOGE("WriteToMD: [error]fail send RPC stream: %d \n", errno);
            return bRet;
        }
    } else {
        // data length excluding CCCI header and OP ID
        data_len = DataLength - sizeof(CCCI_BUFF_T) - sizeof(unsigned int);
        ccci_h = &buffer_slot->header;
                ccci_h->channel++; //Rx->Tx

        /* No fragment is needed */
        if (data_len <= MAX_RPC_PKT_BYTE) {
            pData = (unsigned char *)buffer_slot;
            // Clear "send again indicator"
            ccci_h->data[0] = ccci_h->data[0] & ~CCCI_RPC_REQ_SEND_AGAIN;
                        ccci_h->data[1] = DataLength;
            ret = write(DeviceFd, pData, DataLength);
            if (ret != (int)DataLength) {
                LOGE("Failed to write only one RPC packet(%d)!! (%d/%d)\n", DataLength, ret, errno);
                return bRet;
            }
            LOGD("Write %d bytes to slot %d, CCCI_H(0x%X)(0x%X)(0x%X)(0x%X)\n",
                 ret, BufIndex, ccci_h->data[0], ccci_h->data[1], ccci_h->channel, ccci_h->reserved);
        } else {
            /* Data fragment is needed */
            //LOGD("Big packet, need fragment.");
            pData = (unsigned char *)(&buffer_slot->payload.buffer);
            while ((unsigned int)(data_sent + sizeof(CCCI_BUFF_T) + sizeof(unsigned int)) < DataLength) {
                /* Moret than 2 packets to send */
                /* Each packet includes CCCI header, OP id, and data */
                if ((data_len - data_sent) > MAX_RPC_PKT_BYTE) {
                    data_to_send = MAX_RPC_PKT_BYTE;
                    pkt_ptr = RPC_PreparePktEx(pData, data_to_send, ccci_h, pRpcBuf->rpc_ops_id, 1);
                } else {
                    /* The last packet */
                    data_to_send = data_len - data_sent;
                    pkt_ptr = RPC_PreparePktEx(pData, data_to_send, ccci_h, pRpcBuf->rpc_ops_id, 0);
                }
                // Add CCCI header and operation ID size to packet size, be aware of that OP_ID is not cosindered as payload, so not counted in MAX_RPC_PKT_BYTE
                pkt_size = data_to_send + sizeof(CCCI_BUFF_T) + sizeof(unsigned int);
                // write size = data + CCCI header + OP ID
                ret = write(DeviceFd, pkt_ptr, pkt_size);
                if (ret != pkt_size) {
                    LOGE("Failed to write RPC packet !! (%d)\n", errno);
                    break;
                } else {
                    CCCI_BUFF_T *dst_ccci_h = (CCCI_BUFF_T *)pkt_ptr;
                    LOGD("Write %d bytes to slot %d, CCCI_H(0x%X)(0x%X)(0x%X)(0x%X)\n",
                         ret, BufIndex,
                         dst_ccci_h->data[0], dst_ccci_h->data[1], dst_ccci_h->channel, dst_ccci_h->reserved);
                }
                RPC_PreparePktDone(pkt_ptr);
                data_sent += data_to_send;
                pData += data_to_send;
            };
        }
        if (ret < 0) {
            LOGE("WriteToMD: [error]fail send RPC stream: %d \n", ret);
            return bRet;
        }
        //LOGD("write to MD %d\n", DataLength);
    }
    bRet = true;

_Exit:
    return bRet;
}

/***************************************************************************
 * Support lib Section
 ***************************************************************************/

/* tc1 lib --------------------------------------------------------------------------------------- */
#define TC1_SUPPORT_LIB_PATH	"/vendor/lib/libccci_tc1_srv.so"
static void *tc1_support_lib = NULL;
static int tc1_lib_ready = 0;
int (*tc1_rpc_srv_entry)(int ops_id, RPC_PACKET_INFO *packet_info, int *packet_num, int buf[]);
int (*tc_lib_init)(void);

int load_support_lib(void)
{
    const char *error;

    /* Tc1 section */
    tc1_support_lib = dlopen(TC1_SUPPORT_LIB_PATH, RTLD_NOW);
    if(NULL == tc1_support_lib){
        LOGD("TC1 lib not support for current project!(%s)\n", dlerror());
        return -1;
    }
    error = dlerror(); /* to clear previous error msg */
    tc_lib_init = dlsym(tc1_support_lib, "tc1_lib_init");
    error = dlerror();
    if (NULL != error) {
        LOGE("Load tc1_lib_init api fail!!(%s)\n", error);
        dlclose(tc1_support_lib);
        return -1;
    }
    tc1_rpc_srv_entry = dlsym(tc1_support_lib, "tc1_rpc_srv_entry");
    error = dlerror();
    if (NULL != error) {
        LOGE("Load tc1_rpc_srv_entry api fail!!(%s)\n", error);
        dlclose(tc1_support_lib);
        return -1;
    }
    if (tc_lib_init() < 0) {
    	LOGE("tc1_rpc_lib init fail!!\n");
        dlclose(tc1_support_lib);
        return -1;
    }
    tc1_lib_ready = 1;

    return 0;
}

static int tc1_srv_check(int ops_id, RPC_PACKET_INFO *packet_info, int *packet_num, int buf[])
{
    if (tc1_lib_ready)
        return tc1_rpc_srv_entry(ops_id, packet_info, packet_num, buf);

    return -1;
}


static int exit_signal = 0;
void signal_treatment(int param)
{
    /*
     * this signal catching design does NOT work...
     * set property ctl.stop will send SIGKILL to ccci_rpcd(check service_stop_or_reset() in init.c),
     * but SIGKILL is not catchable.
     * kill pid will send SIGTERM to ccci_rpcd, we can catch this signal, but the process is just
     * terminated, and no time for us to check exit_signal in main().
     * per system team's comment, kernel will free all resource (memory get from malloc, etc.),
     * so we do NOT need to take care of these.
     */
    LOGD("signal number=%d\n", param);
    switch (param) {
    case SIGPIPE:
    case SIGHUP:
    case SIGINT:
    case SIGTERM:
    case SIGUSR1:
    case SIGUSR2:
    case SIGALRM:
    case SIGKILL:
    default:
        exit_signal = param;
        break;
    }
}

int main(int argc, char *argv[])
{
    int ReqBufIndex;
    rpc_stream_buffer_t *pRpcBuf;
    int PacketNum = 0;
    int RetVal;
    char dev_node[32];
    int  using_old_ver = 0;
    RPC_PACKET_INFO *PackInfo;
    CCCI_BUFF_T *ccci_h = NULL;
    char pkt_buff[MAX_RPC_BUF_BYTE] = {0};
    STREAM_DATA *stream = NULL; // data packet received from MD
    STREAM_DATA *buffer_slot = NULL; // local buffer slot
    char *p_rpc_buff = NULL;
    char property_val[PROPERTY_VALUE_MAX] = {0};
    int tmp_buff[2];

    LOGD("ccci_rpcd Ver:v2.00, CCCI Ver:%d", ccci_get_version());
    //Check if input parameter is valid
    if(argc != 2) {
        md_id = 0;
        LOGE("[Warning]Parameter number not correct,use old version!\n");
        using_old_ver = 1;
        snprintf(dev_node, 32, "/dev/ccci_rpc");
    } else {
        if(strcmp(argv[1],"0")==0) {
            snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_CCCI_RPC, MD_SYS1));
            md_id = 0;
        } else if(strcmp(argv[1],"1")==0) {
            snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_CCCI_RPC, MD_SYS2));
            md_id =1;
        } else if(strcmp(argv[1],"4")==0) {
            snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_CCCI_RPC, MD_SYS5));
            md_id =4;
        } else {
            LOGD("Invalid md sys id(%d)!\n", md_id);
            return -1;
        }
    }
    if(md_id==0 || md_id==1) {
        if(ccci_get_version() == ECCCI || ccci_get_version() == EDSDA || ccci_get_version() == ECCCI_FSM)
            stream_support = 1;
        else
            stream_support = 0;
    } else if(md_id == 4) {
        stream_support = 1;
    }

    DeviceFd = open(dev_node, O_RDWR);
    if(DeviceFd == -1)
    {
        LOGE("Main: open ccci_rpc fail\r\n");
        return -1;
    }

    if(!stream_support) {
        g_RpcInfo.pRpcBuf = mmap(NULL, sizeof(rpc_stream_buffer_t), PROT_READ | PROT_WRITE, MAP_SHARED, DeviceFd, 0);
    } else {
        int alloc_length = (sizeof(STREAM_DATA) + RPC_MAX_BUF_SIZE) * RPC_BUFFER_SLOT_NUM;
        g_RpcInfo.pRpcBuf = malloc(alloc_length);
        memset(g_RpcInfo.pRpcBuf, 0, alloc_length);
    }
    PackInfo = malloc(sizeof(RPC_PACKET_INFO) * RPC_MAX_ARG_NUM);
    if(g_RpcInfo.pRpcBuf  == NULL)
    {
        LOGE("Main: mmap buffer fail\r\n");
        return -1;
    }

    LOGD("register signal hadler\n");
    if(signal(SIGHUP, signal_treatment)==SIG_ERR)
        LOGE("can't catch SIGHUP\n");
    if(signal(SIGPIPE, signal_treatment)==SIG_ERR)
        LOGE("can't catch SIGPIPE\n");
    if(signal(SIGINT, signal_treatment)==SIG_ERR)
        LOGE("can't catch SIGINT\n");
    if(signal(SIGUSR1, signal_treatment)==SIG_ERR)
        LOGE("can't catch SIGUSR1\n");
    if(signal(SIGUSR2, signal_treatment)==SIG_ERR)
        LOGE("can't catch SIGUSR2\n");
    if(signal(SIGTERM, signal_treatment)==SIG_ERR)
        LOGE("can't catch SIGTERM\n");
    if(signal(SIGALRM, signal_treatment)==SIG_ERR)
        LOGE("can't catch SIGALRM\n");

    load_support_lib();

    while(exit_signal == 0)
    {
        PacketNum = 0;
retry:
        if(!stream_support) {
            ReqBufIndex = ioctl(DeviceFd, CCCI_RPC_IOCTL_GET_INDEX, 0);
            RPC_WAKE_LOCK();

            if(ReqBufIndex < 0 || ReqBufIndex > RPC_REQ_BUFFER_MUN)
            {
                LOGE("Main: [error]fail get CCCI_RPC buffer index: %d \n", errno);
                RetVal = RPC_PARAM_ERROR;
                PackInfo[PacketNum].Length = sizeof(unsigned int);
                PackInfo[PacketNum++].pData = (void*) &RetVal;
                goto _Next;
            }

            pRpcBuf = (rpc_stream_buffer_t *)((char *)g_RpcInfo.pRpcBuf + (RPC_MAX_BUF_SIZE + sizeof(rpc_stream_buffer_t))*ReqBufIndex);
        } else {
            while (1) {
                memset(pkt_buff, 0, MAX_RPC_BUF_BYTE);
                // add an extra integer as MD consider OP_ID as not part of the "payload"
                RetVal = read(DeviceFd, pkt_buff, (MAX_RPC_PKT_BYTE+sizeof(CCCI_BUFF_T)+sizeof(unsigned int)));
                if (RetVal <= 0) {
                    LOGE("Failed to read from RPC device (%d) !! errno = %d", RetVal, errno);
                    goto retry;
                } else {
                    LOGD("Read %d bytes from RPC device", RetVal);
                }
                RPC_WAKE_LOCK();
                stream = (STREAM_DATA *)pkt_buff;
                ccci_h = (CCCI_BUFF_T *)&stream->header;
                ReqBufIndex = ccci_h->reserved;
                LOGD("Read %d bytes from slot %d, CCCI_H(0x%X)(0x%X)(0x%X)(0x%X)",
                     RetVal, ReqBufIndex,
                ccci_h->data[0], ccci_h->data[1], ccci_h->channel, ccci_h->reserved);

                buffer_slot = (STREAM_DATA *)((char *)g_RpcInfo.pRpcBuf + (RPC_MAX_BUF_SIZE + sizeof(STREAM_DATA))*ReqBufIndex);
                p_rpc_buff = (char *)buffer_slot;
                /******************************************
                 *
                 *  FSM description for re-sent mechanism
                 *   (ccci_rpc_buff_state == CCCI_RPC_BUFF_IDLE) ==> initial status & end status
                 *   (ccci_rpc_buff_state == CCCI_RPC_BUFF_WAIT) ==> need to receive again
                 *
                 ******************************************/
                if (!CCCI_RPC_PEER_REQ_SEND_AGAIN(ccci_h)) {
                    if (g_RpcInfo.rpc_buff_state[ReqBufIndex] == RPC_BUFF_IDLE) {
                        /* copy data memory and CCCI header */
                        memcpy(p_rpc_buff, ccci_h, ccci_h->data[1]);
                        /* don't need to update FS_Address */
                    } else if (g_RpcInfo.rpc_buff_state[ReqBufIndex] == RPC_BUFF_WAIT) {
                        /* copy data memory and NULL, excluding CCCI header, OP id */
                        if (ccci_h->data[1] < (sizeof(CCCI_BUFF_T) + sizeof(unsigned int))) {
                            /* data send from modem abnormal */
                            assert(0);
                        } else {
                            memcpy(p_rpc_buff + g_RpcInfo.rpc_buff_offset[ReqBufIndex],
                                 stream->payload.buffer,
                                 ccci_h->data[1] - sizeof(CCCI_BUFF_T) - sizeof(unsigned int));
                            /* update CCCI header info */
                            memcpy(p_rpc_buff, ccci_h, sizeof(CCCI_BUFF_T));
                        }
                    } else {
                        /* No such rpc_buff_state state */
                        assert(0);
                    }
                    g_RpcInfo.rpc_buff_state[ReqBufIndex] = RPC_BUFF_IDLE;
                    g_RpcInfo.rpc_buff_offset[ReqBufIndex] = 0;
                } else {
                    if (g_RpcInfo.rpc_buff_state[ReqBufIndex] == RPC_BUFF_IDLE) {
                        /* only "OP id" and "data" size and "CCCI header" */
                        unsigned int length = ccci_h->data[1];
                        memcpy(p_rpc_buff, ccci_h, length);
                        g_RpcInfo.rpc_buff_offset[ReqBufIndex] += length;
                    } else if (g_RpcInfo.rpc_buff_state[ReqBufIndex] == RPC_BUFF_WAIT) {
                        /* only "data" size, excluding CCCI header and OP id */
                        unsigned int length = ccci_h->data[1] - sizeof(CCCI_BUFF_T) - sizeof(unsigned int);
                        memcpy(p_rpc_buff + g_RpcInfo.rpc_buff_offset[ReqBufIndex],
                             stream->payload.buffer,
                             length);    /* CCCI_HEADER + RPC_OP_ID */
                        g_RpcInfo.rpc_buff_offset[ReqBufIndex] += length;
                    } else {
                        /* No such ccci_rpc_buff_state state */
                        assert(0);
                    }
                    g_RpcInfo.rpc_buff_state[ReqBufIndex] = RPC_BUFF_WAIT;
                }
                if (g_RpcInfo.rpc_buff_state[ReqBufIndex] == RPC_BUFF_IDLE)
                    break;
                RPC_WAKE_UNLOCK();
            }
            pRpcBuf = &buffer_slot->payload;
        }
        //LOGD("Main: operation ID = %x\n", pRpcBuf->OperateID);
        if(!RPC_GetPackInfo(PackInfo, pRpcBuf->buffer))
        {
            LOGE("Main: Fail to get packet info!! \r\n");
            RetVal = RPC_PARAM_ERROR;
            PackInfo[PacketNum].Length = sizeof(unsigned int);
            PackInfo[PacketNum++].pData = (void*) &RetVal;
            goto _Next;
        }

        switch(pRpcBuf->rpc_ops_id)
        {
        case IPC_RPC_QUERY_AP_SYS_PROPERTY:
            {
            int property_len;
            char *property_name = (char*)PackInfo[0].pData;

            property_name[PackInfo[0].Length] = 0;
            RetVal = mtk_property_get(property_name, property_val, NULL);
            LOGD("Main: IPC_RPC_QUERY_AP_SYS_PROPERTY, key<%s>, value<%s>, %d\n", property_name, property_val, RetVal);
            property_len = strlen(property_val) + 1;
            PackInfo[PacketNum].Length = sizeof(int);
            PackInfo[PacketNum++].pData = (void*) &RetVal;
            PackInfo[PacketNum].Length = property_len;
            PackInfo[PacketNum++].pData = &property_val;
            }
            break;

        default:
            if (tc1_srv_check(pRpcBuf->rpc_ops_id, PackInfo, &PacketNum, tmp_buff) == 0)
            	break;
            else {
                LOGE("Main: Unknow RPC Operation ID (0x%x)\n", pRpcBuf->rpc_ops_id);
                RetVal = RPC_PARAM_ERROR;
                PackInfo[PacketNum].Length = sizeof(int);
                PackInfo[PacketNum++].pData = (void*) &RetVal;
                break;
            }
        }
_Next:
        if(!RPC_WriteToMD(DeviceFd, ReqBufIndex, PackInfo, PacketNum))
        {
            LOGE("Main: fail to write packet!!\r\n");
//            return -1;
        }
        RPC_WAKE_UNLOCK();
    }
    LOGD("ccci_rpcd exit, free buffer\n");
    close(DeviceFd);
    free(PackInfo);
    if(stream_support)
        free(g_RpcInfo.pRpcBuf);
    dlclose(tc1_support_lib);
    return 0;
}
