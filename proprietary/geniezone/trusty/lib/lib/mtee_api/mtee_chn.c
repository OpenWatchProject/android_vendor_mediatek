#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <trusty_std.h>

//#include <mtee_srv.h>
//#include <tz_private/system.h>

#include <lib/mtee/mtee_chn.h>
#include <tz_private/log.h>
#define LOG_TAG "mtee-chn"

//#define MTEE_API_LOG(fmt...) printf("[MTEE API]"fmt)
//#define MTEE_API_ERR(fmt...) printf("[MTEE API][ERR]"fmt)

typedef struct channel_data{
    int channel;
} gz_chan_info;

void release_channel(void* data){
    DBG_LOG("release channel info: %u\n", ((gz_chan_info*)data)->channel);
    free(data);
}

TZ_RESULT MTEE_GetSessionChannel(MTEE_SESSION_HANDLE session, int *channel_p)
{
    TZ_RESULT ret;
    gz_chan_info *chan_info;

    DBG_LOG("GetSessionChannel: session: %u\n", session);
    ret = MTEE_GetSessionUserData(session, (void**)&chan_info);
    if (!ret) { // SUCCESS
        *channel_p = chan_info->channel;
    }
    return ret;
}

TZ_RESULT MTEE_SetSessionChannel(MTEE_SESSION_HANDLE session, int channel)
{
    TZ_RESULT ret;
    gz_chan_info *chan_p;

    DBG_LOG("SetSessionChannel: session: %u, channel: %u\n", session, channel);
    ret = MTEE_GetSessionUserData(session, (void**)&chan_p);
    if (ret) {
        ERR_LOG("check session %u channel fail\n", session);
        return ret;
    }

    if (chan_p) {
        // this session already has a channel
        DBG_LOG("session %u already has channel %u, override it\n", session, chan_p->channel);
    } else {
        chan_p = (gz_chan_info*)malloc(sizeof(gz_chan_info));
    }
    chan_p->channel = channel;

    ret = MTEE_RegisterSessionResource(session, release_channel, (void*)chan_p);
    if (ret) {
        ERR_LOG("session %u register channel callback fail\n", session);
        return ret;
    }
    ret = MTEE_SetSessionUserData(session, (void*)chan_p);
    if (ret) {
        ERR_LOG("session %u set channel fail\n", session);
        return ret;
    }

    return TZ_RESULT_SUCCESS;
}
