#ifndef __LIBATCH_ATCH_H__
#define __LIBATCH_ATCH_H__

enum ATCH_Type_e {
    ATCH_Type_MIN           = 0,
    ATCH_Type_UART          = 1,
    ATCH_Type_TCP           = 2,
    ATCH_Type_UTCP          = 3,
    ATCH_Type_USER_Client   = 4,
    ATCH_Type_USER_Server   = 5,
    ATCH_Type_RILD          = 6,
    ATCH_Type_MAX
};


void * atch_client(int type, char *name);

void * atch_server(int type, char *name);

void * atch_new(int type, int fd);

int atch_close(void *channel);

int atch_send(void *channel, char *buf, int len);

int atch_recv(void *channel, char *buf, int len);

int atch_puts(void *channel, char *buf);

char *atch_gets(void *channel, char *buf, int len);

int atch_getfd(void *channel);

int atch_fflush(void *channel);

int atch_set2buffer(void *channel, int size);

int atch_read2buffer(void *channel);

char *atch_getline(void *channel);

#endif /* __LIBATCH_ATCH_H__ */
