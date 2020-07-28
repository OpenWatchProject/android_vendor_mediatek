#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "os/api_log.h"

#undef DEBUG_MSG_TAG
#define DEBUG_MSG_TAG       "IF_CORE"

#include "common/api_string.h"
#include "os/api_channel.h"
#include "os/api_msgq.h"
#include "os/api_mem.h"
#include "os/api_task.h"
#include "interface/volte_event.h"
#include "interface/volte_cap.h"
#include "interface/volte_stack.h"
//#include "xdmc.h"

#define VoLTE_STACK_CHANNEL_NAME    "@/rcs_rild"

#define __DEBUG_MEMORY_SIZE__       (0)
#define STACK_RESPONSE_TIMEOUT      (100)

#define VOLTE_NETWORK_INFO_LENGTH (VOLTE_MAX_PLANI_LENGTH)

/* ------------------------ */
/*  for UA API              */
/* ------------------------ */

typedef struct _volte_stack_req_obj {
    pthread_mutex_t             mutex;
    pthread_cond_t              cond;
    int                         result;
    unsigned int                data[4];
} volte_stack_req_obj_t;

typedef struct _volte_stack_obj {
    void                       *channel;

    /* client */
    void                       *msgq;
    void                       *msgq_task;
    int                         reg_id;
    short                       connected;
    short                       closed;
    VoLTE_Stack_Ua_Read_Sip_Fn  ua_read_sip;
    VoLTE_Stack_Ua_Read_Msg_Fn  ua_read_msg;
    void                       *user_data;
    volte_stack_req_obj_t       req;
    pthread_mutex_t             mutex;
    pthread_mutex_t             read_mutex;
    unsigned int                last_msg_type;

    /* server */
    VoLTE_Stack_Read_Msg_Fn     read_msg;
    void                       *read_msg_user_data;
} volte_stack_obj_t;


#include <stdio.h>


static void
volte_sip_stack_client_read(Channel_Data_t *_data, void *priv) {
    volte_stack_obj_t *stack = (volte_stack_obj_t *)priv;

    if (!_data) {
        VoLTE_Stack_Ua_Read_Msg_Fn ua_read_msg = stack->ua_read_msg;
        VoLTE_Stack_Ua_Read_Sip_Fn ua_read_sip = stack->ua_read_sip;

        if (ua_read_msg) {
            ua_read_msg(0, stack->user_data);
        }
        if (ua_read_sip) {
            ua_read_sip(0, stack->user_data);
        }

        stack->closed = 1;
        pthread_mutex_lock(&(stack->req.mutex));
        pthread_cond_signal(&(stack->req.cond));
        pthread_mutex_unlock(&(stack->req.mutex));
        return;
    }

    DbgMsg("[IFChannel] Stack client read, type = %d, size = %d, stack = %p", _data->type, _data->len, stack);

    switch (_data->type) {
        case VOLTE_RSP_SIPTX_CREATE :
        case VOLTE_RSP_SIPTX_DESTROY :
        case VOLTE_RSP_SIPTX_CONNECT :
        case VOLTE_RSP_SIPTX_DISCONNECT :
        case VOLTE_RSP_SIPTX_SEND_SIPMSG :
        case VOLTE_RSP_SIPTX_ABORT_SIPMSG :
        case VOLTE_RSP_SIPTX_IPSEC_SETUP :
        case VOLTE_RSP_SIPTX_IPSEC_CONNECT :
        case VOLTE_RSP_SIPTX_IPSEC_REFRESH :
        case VOLTE_RSP_SIPTX_IPSEC_DELETE :
        case VOLTE_RSP_SIPTX_IPSEC_CLEAR :
        case VOLTE_RSP_SIPTX_RESET_TCP :
        case VOLTE_RSP_SIPTX_IPSEC_CHANGE_TO_REGULAR:
        case VOLTE_RSP_SIPTX_START_KEEP_ALIVE :
        case VOLTE_RSP_SIPTX_STOP_KEEP_ALIVE :
        case VOLTE_RSP_SIPTX_REG_QUERY_IF_EM_TS_EXIST :
        case VOLTE_RSP_SIPTX_IF_REG_READY:
        {
            VoLTE_Stack_Result_t *result = (VoLTE_Stack_Result_t *)_data->data;

            pthread_mutex_lock(&(stack->req.mutex));
            if (_data->len < (int)sizeof(VoLTE_Stack_Result_t)) {
                ErrMsg("type = %d, response size error, size = %d", _data->type, _data->len);
                stack->req.result = 0;
            } else if (result->result == VoLTE_Stack_Result_Success) {
                stack->req.result = 1;
                stack->req.data[0] = result->user_data[0];
                stack->req.data[1] = result->user_data[1];
                stack->req.data[2] = result->user_data[2];
                stack->req.data[3] = result->user_data[3];
            } else {
                stack->req.result = 0;
            }

            pthread_cond_signal(&(stack->req.cond));
            pthread_mutex_unlock(&(stack->req.mutex));

            break;
        }

        default : {
            pthread_mutex_lock(&(stack->read_mutex));
            while (stack->msgq) {
                VoLTE_Stack_Message_t *msg = sip_get_mem(sizeof(VoLTE_Stack_Message_t) + __DEBUG_MEMORY_SIZE__);

                if (!msg) {
                    ErrMsg("Can't allocate the memory");
                    break; /* out-of-while */
                } else {
                msg->type = _data->type;
                msg->flag = _data->flag;
                    msg->len = _data->len;
                if (msg->len > 0) {
                        msg->data = sip_get_mem( msg->len + 1 + __DEBUG_MEMORY_SIZE__);
                    if (!(msg->data)) {
                        ErrMsg("Can't allocate the memory");
                        sip_free_mem(msg);
                            break;  /*out-of-while */
                        } else {
                            (void) memcpy(msg->data, _data->data, msg->len );
                    }
                } else {
                        msg->data = NULL;
                }
                if (MSGQ_Put(stack->msgq, msg) != 0) {
                    ErrMsg("Can't send the message, type = %d, len = %d, last_type = %d", msg->type, msg->len, stack->last_msg_type);
                    if (msg->data) {
                        sip_free_mem(msg->data);
                    }
                    sip_free_mem(msg);
                }
                    break; /* out-of-while */
                }
            }
            pthread_mutex_unlock(&(stack->read_mutex));
            break;
        }
    }

    //TrcMsg("[IFChannel] Stack client read, type = %d, size = %d, stack = %p - completed", _data->type, _data->len, stack);
}


void
volte_sip_stack_msgq_handler(void *argv) {
    volte_stack_obj_t   *obj = (volte_stack_obj_t *)argv;

    while (1) {
        VoLTE_Stack_Message_t *msg;

        obj->last_msg_type = -1;

        msg = MSGQ_Get(obj->msgq, MSGQ_FLAG_WAIT);
        if (!msg) {
            ErrMsg("volte_sip_stack_msgq_handler : data is NULL");
            sleep(1);
            continue;
        }

        obj->last_msg_type = msg->type;

#ifdef __ANDROID__
        DbgMsg("[IFMsgq] Msg read type = %d (%p), obj = %p, channel = %p (%d/%d)", msg->type, obj->msgq, obj, obj->channel, getpid(), gettid());
#endif
        switch (msg->type) {
            case VOLTE_EVENT_DUMMY : {
                void *task_ptr = obj->msgq_task;
                void *msgq = 0;

                if (msg->data) {
                    sip_free_mem(msg->data);
                }
                sip_free_mem(msg);

#ifdef __ANDROID__
                DbgMsg("channel close, obj = %p, channel = %p (%d/%d)", obj, obj->channel, getpid(), gettid());
#endif
                if (obj->channel) {
                    void *channel = obj->channel;
                    obj->channel = 0;
                    Channel_Close(channel);
                }

#ifdef __ANDROID__
                // DbgMsg("set msgq to NULL, obj = %p (%d/%d)", obj, getpid(), gettid());
#endif
                pthread_mutex_lock(&(obj->read_mutex));
#ifdef __ANDROID__
                // DbgMsg("set msgq to NULL do, obj = %p (%d/%d)", obj, getpid(), gettid());
#endif
                msgq = obj->msgq;
                obj->msgq = 0;
                pthread_mutex_unlock(&(obj->read_mutex));
#ifdef __ANDROID__
                // DbgMsg("set msgq to NULL done, obj = %p (%d/%d)", obj, getpid(), gettid());
#endif
                do {
                    msg = MSGQ_Get(msgq, MSGQ_FLAG_NOWAIT);
                    if (!msg) {
                        break;
                    }
                    if (msg->data) {
                        sip_free_mem(msg->data);
                    }
                    sip_free_mem(msg);
                } while (1);

#ifdef __ANDROID__
                DbgMsg("Message queue is empty (%p)(%p) (%d/%d)", msgq, obj, getpid(), gettid());
#endif
                if (msgq) {
                    MSGQ_Free(msgq);
                }
                // mtk01739 : can't use the voip_task_destroy to kill self-thread
                // if (obj->msgq_task) voip_task_destroy(obj->msgq_task);
                if (task_ptr) {
                    sip_free_mem(task_ptr);
                }
#ifdef __ANDROID__
                DbgMsg("Message queue is empty (%p) done (%d/%d)", obj, getpid(), gettid());
#endif
                sip_free_mem(obj);
                pthread_exit(0);
                return;
            }

            case VOLTE_IND_SIPTX_DISCONNECT : {
                VoLTE_Stack_Ua_Read_Sip_Fn ua_read_sip = obj->ua_read_sip;
                obj->connected = 0;
                if (ua_read_sip) {
                    ua_read_sip(0, obj->user_data);
                }
                break;
            }

            case VOLTE_RSP_SIPTX_IND_SIPMSG : {
                VoLTE_Stack_Sip_Info_t *sip = (VoLTE_Stack_Sip_Info_t *)msg->data;
                VoLTE_Stack_Ua_Read_Sip_Fn ua_read_sip = obj->ua_read_sip;

                if (msg->len < sizeof(VoLTE_Stack_Sip_Info_t)) {
                    ErrMsg("VOLTE_RSP_SIPTX_IND_SIPMSG, message size error, size = %d", msg->len);
                    break;
                }

                if (msg->len < (sip->message_len + sizeof(VoLTE_Stack_Sip_Info_t))) {
                    ErrMsg("VOLTE_RSP_SIPTX_IND_SIPMSG, message size error, size = %d", msg->len);
                    break;
                }

                if (ua_read_sip) {
                    DbgMsg("[DISP-IO] UA Recv sip, obj = %p, channel = %p, conn_id = %d, request_id = %d, len = %d, sip = {%s}", obj, obj->channel, sip->conn_id, sip->request_id, sip->message_len, sip->message);
                    ua_read_sip(sip, obj->user_data);
                } else {
                    ErrMsg("Without the read sip callback, obj = %p", obj);
                    // printf("#@#@ sip->len = %d, {%s}\n", sip->message_len, sip->message);
                }
                break;
            }

            default : {
                VoLTE_Stack_Ua_Read_Msg_Fn ua_read_msg = obj->ua_read_msg;

                if (ua_read_msg) {
                    ua_read_msg(msg, obj->user_data);
                } else {
                    ErrMsg("Without the read msg callback, type = %d", msg->type);
                }
                break;
            }
        }

        //TrcMsg("[IFMsgq] Msg read type = %d - completed (%p)", msg->type, obj->msgq);
        if (msg->data) {
            sip_free_mem(msg->data);
        }
        sip_free_mem(msg);
    }
}


void *
_volte_sip_stack_create(VoLTE_Stack_Channel_Info_t *info, VoLTE_Stack_Ua_Capability_t *cap, VoLTE_Stack_Callback_t *callback, char *file, int line) {
    Channel_Data_t       data = {0,};
    volte_stack_obj_t   *obj = 0;
    int                  ret = 0;
    int len = 0;
    void *pack = 0;
    int counter = STACK_RESPONSE_TIMEOUT;

    //obj = (volte_stack_obj_t *)sip_get_mem(sizeof(volte_stack_obj_t) + __DEBUG_MEMORY_SIZE__);
    obj = (volte_stack_obj_t *) SIP_get_mem((sizeof(volte_stack_obj_t) + __DEBUG_MEMORY_SIZE__), file, line);
    if (!obj) {
        ErrMsg("Can't allocate the memory");
        goto free_mem;
    }

    obj->msgq = MSGQ_Init(32, "Stack_Client", 0);
    if (!(obj->msgq)) {
        ErrMsg("Can't create the msgq");
        goto free_mem;
    }

    obj->msgq_task = voip_task_create("Stack_Client_Task", 10, 4096, volte_sip_stack_msgq_handler, obj, 0);
    if (!(obj->msgq_task)) {
        ErrMsg("Can't create the msgq task");
        goto free_mem;
    }

    obj->channel = Channel_Create(VoLTE_STACK_CHANNEL_NAME, volte_sip_stack_client_read, (void *)obj);
    if (!(obj->channel)) {
        ErrMsg("Can't create the channel to volte sip stack");
        goto free_mem;
    }

    DbgMsg("create the obj = %p, create the mesgq = %p, create the task = %p, create the channel = %p create the cap = %p (%s:%d)", obj, obj->msgq, obj->msgq_task, obj->channel, cap, file, line);
    obj->reg_id = -1;
    obj->connected = 0;
    obj->closed = 0;
    obj->ua_read_sip = callback->read_sip;
    obj->ua_read_msg = callback->read_msg;
    // obj->ua_read_reg = callback->read_reg;
    obj->user_data   = callback->user_data;
    // volte_sip_stack_req_init(&(obj->req));

    pthread_mutex_init(&(obj->mutex), 0);
    pthread_mutex_init(&(obj->read_mutex), 0);
    pthread_mutex_init(&(obj->req.mutex), 0);
    pthread_cond_init(&(obj->req.cond), 0);


    if (cap) {
        // pack (info+cap)
        //  ------ ---------
        // | info |   cap   |
        //  ------ ---------
        char *p = 0;

        if (VoLTE_STACK_RULE_RSP_OK == volte_pack_ua_cap(cap, (void **)&p, &len)) {
            pack = sip_get_mem(len+sizeof(VoLTE_Stack_Channel_Info_t));
            if (!pack) {
                sip_free_mem(p);
                ErrMsg("insufficient memory space!");
                goto free_mem;
            }
            DbgMsg("Channel_Info size:%d, RuleTbl size:%d", (int)sizeof(VoLTE_Stack_Channel_Info_t), len);
            memset(pack, 0, len+sizeof(VoLTE_Stack_Channel_Info_t));

            DbgMsg("VoLTE_Stack_Channel_Info_t:ua_type:%d, conn_info:%d", info->type, info->conn_info);
            memcpy(pack, info, sizeof(VoLTE_Stack_Channel_Info_t));
            memcpy(((char *)pack)+sizeof(VoLTE_Stack_Channel_Info_t), p, len);

            sip_free_mem(p);
        }
    }

    data.type = VOLTE_REQ_SIPTX_CREATE;
    data.len  = len+sizeof(VoLTE_Stack_Channel_Info_t);
    data.flag = 0;
    data.data = (pack) ? pack : (void *)info;

    // DbgMsg("send the command, waith mutex, obj = %p", obj);
    pthread_mutex_lock(&(obj->mutex));
    obj->req.result = -1;
    DbgMsg("volte_sip_stack_create, send msg, after getting mutex, obj = %p, type = %d", obj, data.type);
    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        ErrMsg("volte_sip_stack_create, Can't send the request to volte sip stack, obj = %p, channel = %p", obj, obj->channel);
        pthread_mutex_unlock(&(obj->mutex));
        goto free_mem;
    }

    if (pack) {
        sip_free_mem(pack);
        pack = 0;
    }

    struct timespec ts;

    pthread_mutex_lock(&(obj->req.mutex));
    ret = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += STACK_RESPONSE_TIMEOUT;
    while (obj->req.result < 0 && obj->closed == 0 && ret == 0 && (counter-->0)) {
        //clock_gettime(CLOCK_REALTIME, &ts);
        //ts.tv_sec += 1;
        ret = pthread_cond_timedwait(&(obj->req.cond), &(obj->req.mutex), &ts);
    }
    pthread_mutex_unlock(&(obj->req.mutex));



    if (obj->closed == 1) {
        ErrMsg("Channel is closed, %p", obj->channel);
        pthread_mutex_unlock(&(obj->mutex));
        goto free_mem;
    } else if (obj->req.result < 0) {
        ErrMsg("Request timeout, %p", obj->channel);
        pthread_mutex_unlock(&(obj->mutex));
        //volte_sip_stack_destroy(obj);
        return 0;
    } else if (obj->req.result == 0) {
        ErrMsg("Can't create the connect for ua, %p", obj->channel);
        pthread_mutex_unlock(&(obj->mutex));
        //volte_sip_stack_destroy(obj);
        return 0;
    }

    SysMsg("volte_sip_stack_create, stack = %p, channel = %p (%s:%d)", obj, obj->channel, file, line);
    pthread_mutex_unlock(&(obj->mutex));

    return obj;

free_mem :
    if (pack) {
        sip_free_mem(pack);
        pack = 0;
    }

    if (obj && obj->msgq_task) {
        VoLTE_Stack_Message_t *msg = sip_get_mem(sizeof(VoLTE_Stack_Message_t) + __DEBUG_MEMORY_SIZE__);
        if (msg) {
            msg->type = VOLTE_EVENT_DUMMY;
            msg->len  = 0;
            msg->flag = 0;
            msg->data = 0;
            if (MSGQ_Put2(obj->msgq, msg, 0x0FFFFFFF) != 0) {
                sip_free_mem(msg);
            }
        }
        sleep(3);
    }
    else
    {
        if (obj) {
            pthread_mutex_lock(&(obj->mutex));
            if(obj->channel)
            {
                DbgMsg("channel close, obj = %p, channel = %p", obj, obj->channel);
                Channel_Close(obj->channel);
                obj->channel = 0;
            }
            pthread_mutex_unlock(&(obj->mutex));
        }
        if (obj && obj->msgq) {
            MSGQ_Free(obj->msgq);
        }
        if (obj) {
            sip_free_mem(obj);
        }
    }

    return 0;
}


static int
volte_sip_stack_send_cmd(void *stack, Channel_Data_t *data) {
    volte_stack_obj_t  *obj = (volte_stack_obj_t *)stack;
    int                 ret = 0;

    if (!obj || !data) {
        ErrMsg("stack(%p) or data(%p) object is NULL", stack, data);
        return VoLTE_Stack_Result_Error;
    }

    TrcMsg("volte_sip_stack_send_cmd, wait mutex, send msg, obj = %p, type = %d", obj, data->type);
    pthread_mutex_lock(&(obj->mutex));
    obj->req.result = -1;
    TrcMsg("volte_sip_stack_send_cmd, send msg, obj = %p, type = %d", obj, data->type);
    ret = Channel_Send(obj->channel, data);
    if (ret < 0) {
        ErrMsg("volte_sip_stack_send_cmd, Can't send the request to volte sip stack, obj = %p, fd = %p", obj, obj->channel);
        pthread_mutex_unlock(&(obj->mutex));
        return VoLTE_Stack_Result_Error;
    }

        struct timespec ts;
        int counter = STACK_RESPONSE_TIMEOUT;

        pthread_mutex_lock(&(obj->req.mutex));
        ret = 0;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += STACK_RESPONSE_TIMEOUT;
        while (obj->req.result < 0 && obj->closed == 0 && ret == 0 && (counter-->0)) {
            //clock_gettime(CLOCK_REALTIME, &ts);
            //ts.tv_sec += 1;
            ret = pthread_cond_timedwait(&(obj->req.cond), &(obj->req.mutex), &ts);
        }
        pthread_mutex_unlock(&(obj->req.mutex));


    if (obj->closed == 1) {
        ErrMsg("volte_sip_stack_send_cmd, channel is disconnect, type = %d", data->type);
        ret = VoLTE_Stack_Result_Disconnected;
    } else if (obj->req.result == 0) {
        ErrMsg("volte_sip_stack_send_cmd, command error, type = %d", data->type);
        ret = VoLTE_Stack_Result_Error;
    } else if (obj->req.result < 0) {
        ErrMsg("volte_sip_stack_send_cmd, command timeout, type = %d", data->type);
        ret = VoLTE_Stack_Result_Timeout;
    } else {
        ret = VoLTE_Stack_Result_Success;
    }
    TrcMsg("volte_sip_stack_send_cmd, send msg, obj = %p, type = %d, completed", obj, data->type);
    pthread_mutex_unlock(&(obj->mutex));

    return ret;
}


int
volte_sip_stack_destroy(void *stack) {
    volte_stack_obj_t  *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t  data = {0,};
    int             ret;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return VoLTE_Stack_Result_Error;
    }

    obj->ua_read_msg = 0;
    obj->ua_read_sip = 0;

    data.type = VOLTE_REQ_SIPTX_DESTROY;
    data.len  = 0;
    data.flag = 0;
    data.data = 0;

    ret = volte_sip_stack_send_cmd(stack, &data);
    if (ret != VoLTE_Stack_Result_Success) {
        ErrMsg("destroy channel error, ret = %d", ret);
    }

    SysMsg("volte_sip_stack_destroy, stack = %p, channel = %p", stack, obj->channel);

    if (obj->msgq_task) {
        VoLTE_Stack_Message_t *msg = sip_get_mem(sizeof(VoLTE_Stack_Message_t) + __DEBUG_MEMORY_SIZE__);
        if (msg) {
            msg->type = VOLTE_EVENT_DUMMY;
            msg->len  = 0;
            msg->flag = 0;
            msg->data = 0;

            DbgMsg("volte_sip_stack_destroy, send the dummy event, obj->msgq = %p", obj->msgq);

            if (obj->msgq) {
                if (MSGQ_Put2(obj->msgq, msg, 0x0FFFFFFF) != 0) {
                    sip_free_mem(msg);
                }
            } else {
                ErrMsg("volte_sip_stack_destroy, msgq is NULL");
                sip_free_mem(msg);
            }
        } else {
            ErrMsg("volte_sip_stack_destroy, msg is NULL");
        }
    } else {
        ErrMsg("volte_sip_stack_destroy, msgq_task is NULL");
    }

    return ret;
}

int
volte_sip_stack_disconnect(void *stack) {
    volte_stack_obj_t  *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t  data = {0,};
    int ret = 0;

    if (!obj) {
        return VoLTE_Stack_Result_Error;
    } else if (!(obj->connected)) {
        ErrMsg("Stack is not connected");
        return VoLTE_Stack_Result_Error;
    }

    data.type = VOLTE_REQ_SIPTX_DISCONNECT;
    data.len  = 0;
    data.flag = 0;
    data.data = 0;

    ret = volte_sip_stack_send_cmd(stack, &data);

    obj->connected = 0;

    SysMsg("volte_sip_stack_disconnect, stack = %p, channel = %p", stack, obj->channel);

    return ret;
}


int
volte_sip_stack_reg_bind(void *stack, int id) {
    volte_stack_obj_t *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t data = {0,};
    VoLTE_Event_Reg_Item_t item;
    int ret = 0;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return VoLTE_Stack_Result_Error;
    }

    item.id  = id;
    data.type = VOLTE_REQ_SIPTX_REG_ADD_BIND;
    data.len  = sizeof(VoLTE_Event_Reg_Item_t);
    data.flag = 0;
    data.data = &item;

    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        ErrMsg("Can't send the sip message to volte sip stack");
        return VoLTE_Stack_Result_Error;
    } else {
    obj->reg_id = id;
    return VoLTE_Stack_Result_Success;
}
}


int
volte_sip_stack_reg_unbind(void *stack) {
    volte_stack_obj_t *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t data = {0,};
    VoLTE_Event_Reg_Item_t item;
    int ret = 0;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return VoLTE_Stack_Result_Error;
    } else if (obj->reg_id < 0) {
        ErrMsg("stack object unbind");
        return VoLTE_Stack_Result_Error;
    }

    item.id   = obj->reg_id;
    data.type = VOLTE_REQ_SIPTX_REG_DEL_BIND;
    data.len  = sizeof(VoLTE_Event_Reg_Item_t);
    data.flag = 0;
    data.data = &item;

    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        ErrMsg("Can't send the sip message to volte sip stack");
        return VoLTE_Stack_Result_Error;
    } else {
    obj->reg_id = -1;
    return VoLTE_Stack_Result_Success;
}
}


int
volte_sip_stack_reg_capability(void *stack, int set, char *capability) {
    volte_stack_obj_t *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t data = {0,};
    VoLTE_Event_Reg_Capability_t cap;
    int ret = 0;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return VoLTE_Stack_Result_Error;
    } else if (obj->reg_id < 0) {
        ErrMsg("stack object unbind");
        return VoLTE_Stack_Result_Error;
    }

    cap.id = obj->reg_id;
    cap.type = VoLTE_Event_Capability_ICSI;
    cap.rat_type = VoLTE_Event_Network_Type_Unknown;
    snprintf(cap.capability, sizeof(cap.capability)-1, "%s", capability);
    if (set) {
        data.type = VOLTE_REQ_SIPTX_REG_ADD_CAPABILITY;
    } else {
        data.type = VOLTE_REQ_SIPTX_REG_DEL_CAPABILITY;
    }
    data.len  = sizeof(VoLTE_Event_Reg_Capability_t);
    data.flag = 0;
    data.data = &cap;

    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        ErrMsg("Can't send the sip message to volte sip stack");
        return VoLTE_Stack_Result_Error;
    } else {
    return VoLTE_Stack_Result_Success;
}
}


int
volte_sip_stack_reg_state(void *stack) {
    volte_stack_obj_t *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t data = {0,};
    VoLTE_Event_Reg_Item_t reg;
    int ret = 0;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return VoLTE_Stack_Result_Error;
    } else if (obj->reg_id < 0) {
        ErrMsg("stack object unbind");
        return VoLTE_Stack_Result_Error;
    }

    reg.id = obj->reg_id;
    data.type = VOLTE_REQ_SIPTX_REG_GET_STATE;
    data.len  = sizeof(VoLTE_Event_Reg_Item_t);
    data.flag = 0;
    data.data = &reg;

    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        ErrMsg("Can't send the sip message to volte sip stack");
        return VoLTE_Stack_Result_Error;
    } else {
    return VoLTE_Stack_Result_Success;
}
}


int
volte_sip_stack_send_sip(void *stack, VoLTE_Stack_Sip_Info_t *sip) {
    volte_stack_obj_t  *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t  data = {0,};
    int             ret;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return VoLTE_Stack_Result_Error;
    } else if (!sip) {
        ErrMsg("sip object is NULL");
        return VoLTE_Stack_Result_Error;
    }

    data.type = VOLTE_REQ_SIPTX_SEND_SIPMSG;
    data.len  = sip->message_len + sizeof(VoLTE_Stack_Sip_Info_t);
    data.flag = 0;
    data.data = (void *)sip;

    // DbgMsg("*SIP*, send the sip to ua, obj = %p, channel = %p, sip = {%s}", obj, obj->channel, sip->message);
    ret = volte_sip_stack_send_cmd(stack, &data);
    return ret;
}


int
volte_sip_stack_send_msg(void *stack, VoLTE_Stack_Message_t *msg) {
    volte_stack_obj_t *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t data ={0,};
    int ret = 0;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return VoLTE_Stack_Result_Error;
    }

    // TrcMsg("[VoLTEStack] Send Msg, type = %d, len = %d, channel = %p", msg->type, msg->len, obj->channel);
    data.type = msg->type;
    data.len  = msg->len;
    data.flag = msg->flag;
    data.data = msg->data;

    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        ErrMsg("Can't send the sip message to volte sip stack");
        return VoLTE_Stack_Result_Error;
    } else {
        return VoLTE_Stack_Result_Success;
    }
}


int volte_sip_stack_restore(void *stack) {
    volte_stack_obj_t *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t data ={0,};
    VoLTE_Event_Reg_Item_t reg;
    int ret = 0;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return VoLTE_Stack_Result_Error;
    } else if (obj->reg_id < 0) {
        ErrMsg("stack object unbind");
        return VoLTE_Stack_Result_Error;
    }
    SysMsg("volte_sip_stack_restore, stack = %p, channel = %p, connected = %d",
        stack, obj->channel, obj->connected);

    reg.id = obj->reg_id;
    data.type = VOLTE_REQ_SIPTX_REG_RESTORE;
    data.len = sizeof(VoLTE_Event_Reg_Item_t);
    data.flag = 0;
    data.data = &reg;

    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        SysMsg("Can't send the restore to volte sip stack");
        return VoLTE_Stack_Result_Error;
    } else {
        SysMsg("volte_sip_stack_restore return success");
        return VoLTE_Stack_Result_Success;
    }
}

int volte_sip_stack_ind_external_ua_state(void *stack, int id, int ua_type, int ua_state) {
    volte_stack_obj_t *obj = (volte_stack_obj_t *)stack;
    Channel_Data_t data ={0,};
    VoLTE_Event_External_UA_State_t external_ua_state;
    int ret = 0;

    if (!obj) {
        ErrMsg("stack object is NULL");
        return VoLTE_Stack_Result_Error;
    } else if (obj->reg_id < 0) {
        ErrMsg("stack object unbind");
        return VoLTE_Stack_Result_Error;
    }
    SysMsg("volte_sip_stack_ind_external_ua_state, stack = %p, id = %d, ua_type = %d, ua_state = %d",
        stack, id, ua_type, ua_state);

    external_ua_state.id = id;
    external_ua_state.ua_type = ua_type;
    external_ua_state.ua_state = ua_state;

    data.type = VOLTE_IND_SIPTX_REG_EXTERNAL_UA_STATE;
    data.len = sizeof(VoLTE_Event_External_UA_State_t);
    data.flag = 0;
    data.data = &external_ua_state;

    ret = Channel_Send(obj->channel, &data);
    if (ret < 0) {
        SysMsg("Can't send the ua state indication to volte sip stack");
        return VoLTE_Stack_Result_Error;
    } else {
        SysMsg("volte_sip_stack_ind_external_ua_state return success");
        return VoLTE_Stack_Result_Success;
    }
}


int
volte_sip_info_free(VoLTE_Stack_Sip_Info_t *sip) {
    sip_free_mem(sip);
    return 0;
}


/* for RCS UA, 3rd-party UA */
int
volte_sip_stack_init(void) {
    return 0;
}


#if defined _UNIT_
#include <stdio.h>
#include <base/utest.h>

int
main(void) {
    printf("%s - %s\r\n", __DATE__, __TIME__);
    UTST_Run(0);

    return 0;
}

#elif defined _EXE_


int main(void)
{
    return 0;
}

#endif



