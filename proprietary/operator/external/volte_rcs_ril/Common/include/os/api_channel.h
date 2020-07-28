#ifndef __SYSSERV_CHANNEL_H__
#define __SYSSERV_CHANNEL_H__


#define CHANNEL_MAX_CLIENT_CONNECTION       (8)

typedef struct _channel_data_ {
    int         type;
    int         len;
    int         flag;
    void       *data;
} Channel_Data_t;

typedef void (*Channel_Client_Read_callback_t) (Channel_Data_t *data, void *priv);
typedef void (*Channel_Server_Read_callback_t) (Channel_Data_t *data, void *priv, void *channel);

void * Channel_Create(char *name, Channel_Client_Read_callback_t cb, void *data);

int Channel_Close(void *channel);

// int Channel_Send(void *channel, int type, int len, void *data);

int Channel_Send(void *channel, Channel_Data_t *data);

void * Channel_Listen(char *name, Channel_Server_Read_callback_t cb, void *data, int max_channel);

int Channel_Data_Free(void *data);

void * Channel_Get_Item(void *channel, int index);

int Channel_Wait(void *channel);

#endif /* __SYSSERV_CHANNEL_H__ */
