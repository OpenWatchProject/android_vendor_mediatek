#define LOG_TAG "rpc_ril"

#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <cutils/sockets.h>
#include "rpc_ril.h"
#include <binder/Parcel.h>
#include <binder/ProcessState.h>
#include <cutils/jstring.h>
#include <cutils/record_stream.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <utils/Log.h>

#define SOCKET_NAME_RPC "rpc"
#define MAX_MESSAGE_LENGTH (1024)

static pthread_mutex_t rpcRequest_Mutex = PTHREAD_MUTEX_INITIALIZER;
static int token = 2000;

extern char16_t * strdup8to16(const char* s, size_t *out_len);

namespace android {

static char *strdupReadString(Parcel &p)
{
    size_t stringlen = 0;
    const char16_t *s16 = NULL;

    s16 = p.readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

static void writeStringToParcel(Parcel &p, const char *s)
{
    char16_t *s16 = NULL;
    size_t s16_len = 0;

    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}
static int rpcBlockWrite(int fd, const void *buffer, unsigned int len)
{
    unsigned int writeOffset = 0;
    const unsigned char *toWrite = NULL;

    toWrite = (const unsigned char *)buffer;

    while(writeOffset < len)
    {
        int written;
        do{
            written = write(fd, toWrite + writeOffset, len - writeOffset);
        } while(written < 0 && errno ==EINTR);

        if(written >=0 )
        {
            writeOffset += written;
        }
        else
        {
            RLOGE("block write error, errno=%d", errno);
            close(fd);
            return -1;
        }
    }
    return 0;
}
/* Send Parcel message to the VRIL, will add header to the message and send them.
 * Input:   fd - socket fd
 *          buffer - message body
 *          len - message length
 * ret  < 0 - error
 *      =0 - OK
 */
static int rpcSendMessage(int fd, const void *buffer, unsigned int len)
{
    int ret = -1;

    if(fd < 0)
    {
        RLOGE("Input socket fd is unavlid.");
        goto error;
    }

    if(len > MAX_MESSAGE_LENGTH)
    {
        RLOGE("Message length is %u and larger than %u", len, MAX_MESSAGE_LENGTH);
        goto error;
    }
#if 0
    header = htonl(len);
    ret = rpcBlockWrite(fd, (void *)&header, sizeof(header));
    if(ret < 0)
    {
        RLOGE("write header error, ret=%d", ret);
        goto error;
    }
    #endif
    RLOGW("buffer = %s",(char*)buffer);
    ret = rpcBlockWrite(fd, buffer, len);
    if(ret < 0)
    {
        RLOGE("write message body error, ret=%d", ret);
        goto error;
    }

    ret = 0;

error:
    return ret;
}

/*Got the socket fd*/
static int connectToSocket(const char *sock_name)
{
    struct timeval tv;
    int s_fdListen = 0;
    int err = -1;

    tv.tv_sec = 3;
    tv.tv_usec = 0;

    s_fdListen = socket_local_client(sock_name, ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
    if (s_fdListen < 0) {
        RLOGE("Failed to get socket %s", sock_name);
        return err;
    }

    RLOGD("!!!!!!!!!!sendRpcRequest fd = %d",s_fdListen);
#if 1
    err = setsockopt(s_fdListen,SOL_SOCKET,SO_SNDTIMEO,(void *)&tv,sizeof(tv));
    if(err < 0)
    {
        RLOGE("set send timeout failer");
        close(s_fdListen);
        return err;
    }
    err = setsockopt(s_fdListen,SOL_SOCKET,SO_RCVTIMEO,(void *)&tv,sizeof(tv));
    if(err < 0)
    {
        RLOGE("set recv timeout failer");
        close(s_fdListen);
        return err;
    }
#endif

    return s_fdListen;
}
/*out_len: int counts, strings counts, raw bytes*/
static int rpcProcessResult(Parcel &p, RIL_RPC_ParaTypes out_types, void ***out_value, int *out_len)
{
    int tmp = 0;
    int numInts = 0;
    int *ret_ibuff = NULL;
    int countStrings = 0;
    char **pStrings = NULL;
    int counts = 0;
    unsigned char *ret_buff = NULL;
    int len = 0;

    if(out_types >= RIL_RPC_PARA_RESERVE )
    {
        RLOGE("Invalid input parameters");
        goto error;
    }
    switch(out_types)
    {
        case RIL_RPC_PARA_NULL:
            return 0;
        case RIL_RPC_PARA_INTS:

            tmp = p.readInt32(&numInts);
            RLOGW("numInts = %d", numInts);
            if(tmp != NO_ERROR || numInts == 0)
            {
                RLOGE("read INT numbers error, count=%d", numInts);
                goto error;
            }
            ret_ibuff = (int *)malloc(numInts * sizeof(int));
            if(ret_ibuff == NULL)
            {
                RLOGE("malloc memory for INTS failure");
                goto error;
            }
            for(int i = 0; i < numInts; i++)
            {
                int t = 0;
                tmp = p.readInt32(&t);
                ret_ibuff[i] = t;
                if(tmp != NO_ERROR)
                {
                    RLOGE("read INT value error");
                    if(ret_ibuff != NULL)
                    {
                        free(ret_ibuff);
                    }
                    goto error;
                }
            }

            *out_value = (void **)ret_ibuff;
            *out_len = numInts;
            break;
        case RIL_RPC_PARA_STRING:
            pStrings = (char **)malloc(sizeof(char *));
            if(pStrings == NULL)
            {
                RLOGE("malloc memory for strings failure");
                goto error;
            }
            pStrings[0] = strdupReadString(p);
            RLOGW("pStrings[0]= %s", pStrings[0]);

            *out_value = (void**)pStrings;
            RLOGW("out_value = %x", (unsigned long)(*out_value));
            len = 1;
            *out_len = len;
            break;
        case RIL_RPC_PARA_STRINGS:
            tmp = p.readInt32(&countStrings);
            RLOGW("countStrings = %d", countStrings);
            if(tmp != NO_ERROR)
            {
                RLOGE("read strings error, count=%d", countStrings);
                goto error;
            }
            if(countStrings <= 0)
            {
                if (*out_value != NULL) {
                    **out_value = NULL;
                }
                *out_len = 0;
            }
            else
            {
                pStrings = (char **)malloc(sizeof(char *) * countStrings);
                if(pStrings == NULL)
                {
                    RLOGE("malloc memory for strings failure");
                    goto error;
                }
                for(int i = 0; i < countStrings; i++)
                {
                    pStrings[i] = strdupReadString(p);
                    RLOGW("pStrings[i]= %s", pStrings[i]);
                }

                *out_value = (void **)pStrings;
                *out_len = countStrings;
            }
            break;
        case RIL_RPC_PARA_RAW:
            tmp = p.readInt32(&counts);
            if(tmp != NO_ERROR)
            {
                RLOGE("read strings error, count=%d", countStrings);
                goto error;
            }

            if(counts <= 0)
            {
                if (*out_value != NULL) {
                    **out_value = NULL;
                }
                *out_len = 0;
            }
            else
            {
                ret_buff = (unsigned char *)malloc(counts);
                if(ret_buff == NULL)
                {
                    RLOGE("malloc memory for RAW failure");
                    goto error;
                }
                tmp = p.read((void *)ret_buff, counts);

                if(tmp != NO_ERROR)
                {
                    RLOGE("read strings error, count=%d", countStrings);
                    if(ret_buff != NULL)
                    {
                        free(ret_buff);
                    }
                    goto error;
                }
                if (*out_value != NULL) {
                    **out_value = ret_buff;
                }
                *out_len = counts;
            }

            break;
        default:
            RLOGE("Not support RIL_RPC_ParaTypes");
            goto error;
    }
    RLOGW("out_value = %x", (unsigned long)(*out_value));
    return 0;

error:
    RLOGW("error\n");
    if (*out_value != NULL) {
        **out_value = NULL;
    }
    *out_len = 0;

    return -1;
}

/*
 * Input para:  request_num -
 *              in_types -
 *              in_len - for ints, it's int numbers
 *                       for strings, it's string numbers
 *                       for raw, it's byte numbers
 *              in_value -
 *              out_types -
 *              out_value -
 *              out_len - same as in_len
 * ret  <0 - fail
 *      = 0 - OK
 */
int sendRpcRequestComm(int request_num, RIL_RPC_ParaTypes in_types, int in_len, void *in_value, RIL_RPC_ParaTypes out_types, int *out_len, void ***out_value)
{
    int s_fd = -1;
    int tmp = -1;
    int i = 0;
    Parcel p_send;
    Parcel p_recv;
    char **p_cur = NULL;
    int n,recv_num;
    char recvbuf[256] = {0};
    fd_set readFds;
    struct timeval tv;
    FD_ZERO(&readFds);
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    int type_recv = 0;
    int ret_token = 0;
    int err;
    void ** out = NULL;//pStrings = (char **)malloc(sizeof(char *));
    int len = 0;

    if(in_types >= RIL_RPC_PARA_RESERVE  \
            || out_types >= RIL_RPC_PARA_RESERVE)
    {
        RLOGE("Invalid input parameters");
        goto error1;
    }

    pthread_mutex_lock(&rpcRequest_Mutex);
    s_fd = connectToSocket(SOCKET_NAME_RPC);
    if(s_fd < 0)
    {
        RLOGE("Connect to RPC socket error");
        goto error;
    }
    FD_SET(s_fd, &readFds);

    RLOGW("request_num = %d", request_num);
    p_send.writeInt32(request_num);
    p_send.writeInt32(token++);

    switch(in_types)
    {
        case RIL_RPC_PARA_INTS:
            for(i = 0; i < in_len; i++)
            {
                p_send.writeInt32(*((int *)in_value + i));
            }
            break;
        case RIL_RPC_PARA_STRING:
            RLOGW("in_value = %s", (char *)in_value);
            writeStringToParcel(p_send, (const char *)in_value);
            break;
        case RIL_RPC_PARA_STRINGS:
            p_send.writeInt32(in_len);
            p_cur = (char **)in_value;

            for(int i = 0; i < in_len; i++)
            {
                RLOGW("p_cur[i] = %s", (char *)p_cur[i]);
                writeStringToParcel(p_send, p_cur[i]);
            }
            break;
        case RIL_RPC_PARA_RAW:
            p_send.write(in_value, in_len);
            break;
        default:
            RLOGE("Unsupported input RIL_RPC_ParaTypes");
            goto error;
            break;
    }
    RLOGW("p_send.data() = %s",(char*)p_send.data());
    tmp = rpcSendMessage(s_fd, p_send.data(), p_send.dataSize());
    if(tmp < 0)
    {
        RLOGE("rpcSendMessage %d error", request_num);
        goto error;
    }

    n = select(s_fd+1,&readFds,NULL,NULL,&tv);
    if (n < 0) {
        RLOGE("sendRpcRequest: select error (%d)", errno);
        close(s_fd);
        pthread_mutex_unlock(&rpcRequest_Mutex);
        return -errno;
    }
    RLOGD("n = %d",n);
    recv_num = recv(s_fd,recvbuf, sizeof(recvbuf),0);
    RLOGD("recv_num = %d",recv_num);
    if(recv_num < 0) {
        RLOGE("recv error");
       // pthread_mutex_unlock(&rpcRequest_Mutex);
        goto error;
    }
    p_recv.setData((unsigned char *)recvbuf,recv_num);

    //p_recv.setData((unsigned char *)recvbuf, recv_num);
    tmp = p_recv.readInt32(&type_recv);
    RLOGW("type_recv = %d", type_recv);
    if(tmp != NO_ERROR)
    {
        RLOGE("invalid ret_request value");
        goto error;
    }
    tmp = p_recv.readInt32(&ret_token);
    RLOGW("ret_token = %d", ret_token);
    if(tmp != NO_ERROR)
    {
        RLOGE("invalid ret_token value");
        goto error;
    }

    tmp = p_recv.readInt32(&err);
    RLOGW("err = %d", err);

    if(tmp != NO_ERROR)
    {
        RLOGE("invalid ret_token value");
        goto error;
    }
    if(err != 0)
        goto error;
    if(rpcProcessResult(p_recv, out_types, &out, &len) != 0)
    {
        RLOGE("Process the received parcel data error");
        goto error;
    }
    RLOGW("len = %d", len);
    if (out != NULL) {
        *out_value = out;
    }
    *out_len = len;
    close(s_fd);
    pthread_mutex_unlock(&rpcRequest_Mutex);
    return 0;

#if 0
    rpc_rs = record_stream_new(s_fd, MAX_MESSAGE_LENGTH);
    if(rpc_rs == NULL)
    {
        RLOGE("create new record stream error");
    }
    for(;;)
    {
    do{
        tmp = record_stream_get_next(rpc_rs, &p_record, &recordlen);
        RLOGW("p_record = %x, recordlen = %d", p_record, recordlen);

        if(tmp == 0 && p_record == NULL)
        {
            RLOGE("No new record stream");
            goto error;
        }
        }while(tmp < 0);
         if(tmp < 0)
        {
            RLOGE("Cant get new stream record");
            goto error;
        }
        else if(tmp == 0)
        {
            int type_recv = 0;
            int ret_token = 0;
            int err;

            p_recv.setData((unsigned char *)p_record, recordlen);
            tmp = p_recv.readInt32(&type_recv);
            RLOGW("type_recv = %d", type_recv);
            if(tmp != NO_ERROR)
            {
                RLOGE("invalid ret_request value");
                //goto error;
            }

            tmp = p_recv.readInt32(&ret_token);
            RLOGW("ret_token = %d", ret_token);
            if(tmp != NO_ERROR)
            {
                RLOGE("invalid ret_token value");
                //goto error;
            }

            tmp = p_recv.readInt32(&err);
            RLOGW("err = %d", err);
            if(tmp != NO_ERROR)
            {
                RLOGE("invalid ret_token value");
                //goto error;
            }
            if(rpcProcessResult(p_recv, out_types, out_value, out_len) != 0)
            {
                RLOGE("Process the received parcel data error");
                goto error;
            }
            break;
        }
    }
#endif

error:
    if(s_fd >= 0) {
        close(s_fd);
    }
    pthread_mutex_unlock(&rpcRequest_Mutex);
    return -1;
error1:
    return -1;
}
int sendRpcRequest(int request_num, int value)
{
    Parcel p_send,p_recv;
    int err;
    int n;
    int send_num, recv_num = 0;
    int type_recv = 0;
    int token = 0;
    char recvbuf[128];
    fd_set readFds;
    int s_fdListen = -1;
    int count = 1;
    struct timeval tv;
    pthread_mutex_lock(&rpcRequest_Mutex);
    FD_ZERO(&readFds);
    tv.tv_sec = 60;
    tv.tv_usec = 0;

    // MTK-START:
    /** If vendor partition(viarild) want to open binder dev node(e.g. Parcel) will be denied for
     * no permission. Should use vndbinder dev node in vendor domain.
     * Using the following way to apply the permission:
     * 1. Initialize vndbinder in vendor
     *     android::ProcessState::initWithDriver("/dev/vndbinder");
     * 2. Add sepolicy rule:
     *     vndbinder_user(module_domain)
     */
    ProcessState::initWithDriver("/dev/vndbinder");
    // MTK-END

    RLOGD("setCPAudioPath before socket_local_client");
    s_fdListen = socket_local_client(SOCKET_NAME_RPC, ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
    if (s_fdListen < 0) {
        RLOGE("Failed to get socket '" SOCKET_NAME_RPC "'" "errno == %d", errno);
        pthread_mutex_unlock(&rpcRequest_Mutex);
        return -1;
    }
    FD_SET(s_fdListen, &readFds);
    RLOGD("!!!!!!!!!!sendRpcRequest fd = %d",s_fdListen);
    err = setsockopt(s_fdListen,SOL_SOCKET,SO_SNDTIMEO,(void *)&tv,sizeof(tv));
    if(err < 0)
        RLOGE("set send timeout failer");
    err = setsockopt(s_fdListen,SOL_SOCKET,SO_RCVTIMEO,(void *)&tv,sizeof(tv));
    if(err < 0)
        RLOGE("set recv timeout failer");
    #if 1
    p_send.writeInt32 (request_num);
    p_send.writeInt32 (token);
    p_send.writeInt32 (count);
    p_send.writeInt32 (value);
    send_num = send(s_fdListen,p_send.data(),p_send.dataSize(),MSG_DONTWAIT);
    if(send_num < 0) {
//c+w
        RLOGE("send error");
        close(s_fdListen);
        pthread_mutex_unlock(&rpcRequest_Mutex);
        return -1;
    }
    n = select(s_fdListen+1,&readFds,NULL,NULL,&tv);
    if (n < 0) {
            RLOGE("sendRpcRequest: select error (%d)", errno);
            close(s_fdListen);
            pthread_mutex_unlock(&rpcRequest_Mutex);
            return -errno;
    }
    RLOGD("n = %d",n);
    recv_num = recv(s_fdListen,recvbuf, sizeof(recvbuf),0);
    if(recv_num < 0) {
//c+w
        RLOGE("recv error");
        close(s_fdListen);
        pthread_mutex_unlock(&rpcRequest_Mutex);
        return -1;
    }
    p_recv.setData((unsigned char *)recvbuf,recv_num);
    type_recv = p_recv.readInt32();
    token = p_recv.readInt32();
    err =  p_recv.readInt32();
    RLOGD("type_recv = %d, token_recv = %d, err = %d",type_recv,token,err);
    #endif
    err = close(s_fdListen);
    RLOGD("sendRpcRequest: err = %d",err);
    pthread_mutex_unlock(&rpcRequest_Mutex);
    return -err;
}
};
