#include <stdio.h>
#include <pthread.h>
#include <linux/param.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include "os/api_mem.h"
#include "os/api_log.h"
#include "os/api_channel.h"
#include "common/api_string.h"

#define CHANNEL_HEADER_LEN      (offsetof(Channel_Data_t, data))

#ifdef __ANDROID__
 #include <cutils/sockets.h>
 #define ANDROID_SOCKETS
#endif

#undef DEBUG_MSG_TAG
#define DEBUG_MSG_TAG       "IF_CH"

#define MAX_CHANNEL_WAKELOCK_TIME        (5)
#define CHANNEL_BUFFER_SIZE     65535   // align SIP_TX_RECV_BUFF_SIZE & SIP_TCP_RECEIVE_BUFFER_SIZE

typedef struct _Channel_Obj {
    int                             fd;
    int                             pipe[2];
    int                             state;
    pthread_mutex_t                 mutex;
    pthread_t                       tid;
    void                           *channels;
    int                             channels_size;
} Channel_Obj_t;

typedef struct _Channel_Argv {
    void                           *user_data;
    Channel_Server_Read_callback_t  serv_read;
    Channel_Client_Read_callback_t  client_read;
    char                           *data;
    int                             data_len;
    int                             data_type;
    Channel_Obj_t                  *channel;
    char                           *name;
} Channel_Argv_t;

typedef struct _Channel_Client {
    Channel_Obj_t                    channel;
    int                              buf_len;
    char                             buf[CHANNEL_BUFFER_SIZE];
} Channel_Client_t;


typedef void * (*Channel_Thread_Fn_t) (void *);


static int
Channel_Thread(pthread_t *tid, Channel_Argv_t *argv, Channel_Thread_Fn_t fn) {
    pthread_attr_t  attr;

    pthread_attr_init (&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (pthread_create(tid, &(attr), fn, (void *)argv) < 0) {
        ErrMsg("Can't create the thread");
        return -1;
    }

    return 0;
}


static void *
Channel_Read_Thread(void *_argv) {
    Channel_Argv_t *argv = (Channel_Argv_t *)_argv;
    Channel_Obj_t  *obj = argv->channel;
    int client_fd = argv->channel->fd;
    Channel_Client_t c;

    memset(&c, 0, sizeof(c));
    while (obj->state > 0) {
        // struct timeval timeout;
        fd_set  iofds;
        int ret = 0;
        int max_fd = client_fd;

        FD_ZERO(&iofds);
        FD_SET(client_fd, &iofds);

        if (max_fd < obj->pipe[0]) {
            max_fd = obj->pipe[0];
        }
        FD_SET(obj->pipe[0], &iofds);

        // timeout.tv_sec  = 1;
        // timeout.tv_usec = 0;

        ret = select(max_fd + 1, &iofds, 0, 0, NULL);
        // DbgMsg("channel, client, select , ret = %d, name = %s", ret, argv->name);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
            ErrMsg("Select fail, ret = %d, errno = %s(%d)", ret, strerror(errno), errno);
            goto free_mem;
        } else if (ret == 0) {
            continue;
        }

        if (FD_ISSET(client_fd, &iofds)) {
            // ret = read(client_fd, c.buf + c.buf_len, sizeof(c.buf)-c.buf_len);
            ret = recv(client_fd, c.buf + c.buf_len, sizeof(c.buf)-c.buf_len, 0);
            // DbgMsg("channel, client, read, ret = %d, name = %s, fd = %d", ret, argv->name, client_fd);
            if (ret > 0) {
                c.buf_len += ret;
                while (c.buf_len >= (int)CHANNEL_HEADER_LEN) {
                    Channel_Data_t recv_data;

                    memcpy(&recv_data, c.buf, CHANNEL_HEADER_LEN);
                    if (recv_data.len == 0) {
                        recv_data.data = 0;
                        argv->client_read(&recv_data, argv->user_data);
                        c.buf_len -= (CHANNEL_HEADER_LEN);
                        memmove(c.buf, c.buf+CHANNEL_HEADER_LEN, c.buf_len);
                    } else if (recv_data.len <= (int)(c.buf_len-CHANNEL_HEADER_LEN)) {
                        recv_data.data = (char *)sip_get_mem(recv_data.len);
                        if (recv_data.data) {
                            memcpy(recv_data.data, c.buf+CHANNEL_HEADER_LEN, recv_data.len);
                            argv->client_read(&recv_data, argv->user_data);
                            if (recv_data.data){
                                sip_free_mem(recv_data.data);
                            }
                        } else {
                            ErrMsg("Can't allocat memory");
                        }
                        c.buf_len -= (CHANNEL_HEADER_LEN+recv_data.len);
                        memmove(c.buf, c.buf+CHANNEL_HEADER_LEN+recv_data.len, c.buf_len);
                    } else {
                        break;
                    }
                }
            } else if (ret == 0) {
                if (sizeof(c.buf)-c.buf_len == 0) {
                    ErrMsg("[Client] ****** channel (%s) buffer full ******", argv->name);
                }
                SysMsg("[Client] Connection is closed, name = %s, fd = %d, channel = %p", argv->name, client_fd, argv->channel);
                pthread_mutex_lock(&(obj->mutex));
                argv->channel->fd = -1;
                shutdown(client_fd, SHUT_RDWR);
                close(client_fd);
                pthread_mutex_unlock(&(obj->mutex));
                argv->client_read(0, argv->user_data);
                goto free_mem;
            } else {
                if (errno != EINTR) {
                    ErrMsg("[Client] Read data error, name = %s, ret = %d, errno = %s(%d)", argv->name, ret, strerror(errno), errno);
                    pthread_mutex_lock(&(obj->mutex));
                    argv->channel->fd = -1;
                    shutdown(client_fd, SHUT_RDWR);
                    close(client_fd);
                    pthread_mutex_unlock(&(obj->mutex));
                    argv->client_read(0, argv->user_data);
                    goto free_mem;
                }
            }
        }
    }

free_mem :
    if (argv->name) sip_free_mem(argv->name);
    argv->name = 0;
    sip_free_mem(argv);
    obj->tid   = 0;
    obj->state = -1;
    pthread_exit(0);
    return 0;
}



void *
Channel_Create(char *name, Channel_Client_Read_callback_t cb, void *data) {
    Channel_Argv_t *argv = 0;
    Channel_Obj_t  *obj = 0;
    int ret = 0;
    int client_fd = 0;
    struct sockaddr_un server;

#ifdef VOLTE_UNIT_TEST
    int msg_printed = 0;
#endif /* VOLTE_UNIT_TEST */

    obj = (Channel_Obj_t *)sip_get_mem(sizeof(Channel_Obj_t));
    if (!obj) {
        ErrMsg("Can't allocate the memory");
        return 0;
    }

    client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&server, 0, sizeof(struct sockaddr_un));
    server.sun_family = AF_UNIX;
    strncpy(server.sun_path, name, sizeof(server.sun_path));
    server.sun_path[0] = 0;
    //strcpy(&(server.sun_path[1]), name);

    if (client_fd < 0) {
        ErrMsg("Can't open stream socket");
        sip_free_mem(obj);
        return 0;
    }

 #ifdef VOLTE_UNIT_TEST
    while (connect(client_fd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        if(!msg_printed) {
            ErrMsg("Can't connect to server side. Retrying...");
            msg_printed = 1;
        }

        usleep(500000);
    }
#else /* VOLTE_UNIT_TEST */
    if (connect(client_fd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        close(client_fd);
        sip_free_mem(obj);
        ErrMsg("Can't connect to server side,%s %d", strerror(errno), errno);
        return 0;
    }
#endif /* VOLTE_UNIT_TEST */

    argv = (Channel_Argv_t *)sip_get_mem(sizeof(Channel_Argv_t));
    if (!argv) {
        close(client_fd);
        sip_free_mem(obj);
        ErrMsg("Can't allocat memory");
        return 0;
    }

    pthread_mutex_init(&(obj->mutex), 0);
    obj->fd = client_fd;
    obj->state = 1;
    if (pipe(obj->pipe)==-1)
    {
        ErrMsg("pipe failed: %s %d", strerror(errno), errno);
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        sip_free_mem(obj);
        if (argv->name) sip_free_mem(argv->name);
        sip_free_mem(argv);
        return 0;
    }

    if (fcntl(obj->pipe[0], F_SETFL, O_NONBLOCK) == -1)
    {
        ErrMsg("fcntl failed: %s %d", strerror(errno), errno);
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        sip_free_mem(obj);
        if (argv->name) sip_free_mem(argv->name);
        sip_free_mem(argv);
        return 0;
   }

    argv->client_read = cb;
    argv->user_data   = data;
    argv->channel     = obj;
    argv->name        = voip_strdup(name);

    ret = Channel_Thread(&(obj->tid), argv, Channel_Read_Thread);
    if (ret != 0) {
        pthread_mutex_lock(&(obj->mutex));
        obj->fd = -1;
        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
        pthread_mutex_unlock(&(obj->mutex));
        if (argv->name) sip_free_mem(argv->name);
        sip_free_mem(argv);
        sip_free_mem(obj);
        ErrMsg("Can't create the read thread");
        return 0;
    }

    SysMsg("** channel create (%p) (%d)", obj, obj->fd);

    return obj;
}


int
Channel_Close(void *channel) {
    Channel_Obj_t *obj = (Channel_Obj_t *)channel;
    pthread_t tid = 0;
    int fd = 0;

    if (!obj) {
        ErrMsg("Channel object is NULL");
        return -1;
    }

    pthread_mutex_lock(&(obj->mutex));
    fd = obj->fd;
    if (fd > 0) {
        obj->fd = -1;
        DbgMsg("channel close: close fd = %d", fd);
        shutdown(fd, SHUT_RDWR);
        close(fd);
    } else {
        ErrMsg("channel fd (%d) <= 0, channel = %p", fd, channel);
    }
    pthread_mutex_unlock(&(obj->mutex));

    if (obj->pipe[1]) {
        write(obj->pipe[1], " ", 1);
    }

    tid = obj->tid;
    if (tid) {
        obj->state = 0;
        while (obj->state >= 0 && obj->tid) {
            usleep(100000);
        }
        // pthread_kill(tid, SIGQUIT);
    }
    close(obj->pipe[0]);
    close(obj->pipe[1]);

    SysMsg("** channel close (%p) (%d)", obj, fd);
    sip_free_mem(obj);

    return 0;
}


int
Channel_Data_Free(void *data) {
    sip_free_mem(data);
    return 0;
}


static int
fsend(int fd, char *data, int len) {
    int size = 0, ret = 0;;
    int write_size = 0;
    int local_errno;

    if (fd <= 0) {
        return -1;
    }

    while (size < len) {
        write_size = len-size;

        ret = send(fd, data+size, write_size, 0);
        local_errno = errno;
        // ret = write(fd, data+size, write_size);
        if (ret > 0) {
            size += ret;
        } else {
            if (errno == EINTR) {
                continue;
            }
            else if (errno == EPIPE) {
                ;
            }
            ErrMsg("Can't write the data to channel, error = (%d), fd = (%d)", local_errno, fd);
            break;
        }
    }
    return size;
}


int
Channel_Send(void *channel, Channel_Data_t *data) {
    Channel_Obj_t *obj = (Channel_Obj_t *)channel;
    int ret = 0;
    int id = 0;

    if (!obj) {
        ErrMsg("Channel object is NULL");
        return -1;
    }

    pthread_mutex_lock(&(obj->mutex));
    id = obj->fd;
    if (id <= 0) {
        ErrMsg("channel fd (%d) <= 0, channel = %p", id, channel);
        pthread_mutex_unlock(&(obj->mutex));
        return -1;
    }

    do {
        ret = fsend(id, (char *)data, CHANNEL_HEADER_LEN);
        if (ret != CHANNEL_HEADER_LEN) {
            ret = -1;
            break;
        }

        if (data->len && data->data) {
            ret = fsend(id, (char *)data->data, data->len);
            if (ret != data->len) {
                ret = -1;
                break;
            }
        } else if (data->len || data->data) {
            ErrMsg("Channel send fail, length or data is null");
            ret = -1;
            break;
        }
        ret = 0;
    } while (0);
    pthread_mutex_unlock(&(obj->mutex));

    return ret;
}



#if 0
static void *
Channel_Command_Thread(void *_argv) {
    Channel_Argv_t *argv = (Channel_Argv_t *)_argv;

    if (argv->serv_read) {
        argv->serv_read(argv->data_type, argv->data_len, argv->data, argv->user_data, argv->channel);
    }

    if (argv->data) sip_free_mem(argv->data);
    sip_free_mem(argv);
    return 0;
}
#endif


static void *
Channel_Listen_Thread(void *_argv) {
    Channel_Argv_t *argv = (Channel_Argv_t *)_argv;
    Channel_Client_t *clients = argv->channel->channels;
    int serv_fd = argv->channel->fd;
    int max_client = argv->channel->channels_size;

    if (serv_fd < 0) {
        ErrMsg("serv_fd < 0");
        return 0;
    }

    SysMsg("Max connection is %d, name = %s", max_client, argv->name);
    if (0 > listen(serv_fd, 5))
    {
        ErrMsg("Listen failed on %d: %s %d", serv_fd, strerror(errno), errno);
        return 0;
    }

    while (1) {
        // struct timeval timeout;
        fd_set  iofds;
        int i = 0;
        int ret = 0;
        int max_fd = serv_fd;
        int local_errno;
        int bad_fd_handle = 0;

        // DbgMsg("channel, server, name = %s, select start, argv = %p", argv->name, argv);

        FD_ZERO(&iofds);
        FD_SET(serv_fd, &iofds);

        // timeout.tv_sec  = 1;
        // timeout.tv_usec = 0;

        for (i=0 ; i<max_client ; ++i) {
            if (clients[i].channel.fd >= 0) {
                if (clients[i].channel.fd > max_fd) {
                    max_fd = clients[i].channel.fd;
                }
                FD_SET(clients[i].channel.fd, &iofds);
            }
        }
        //DbgMsg("channel, server, name = %s, select get max_fd = %d, argv = %p", argv->name, max_fd, argv);

        ret = select(max_fd + 1, &iofds, 0, 0, NULL);
        local_errno = errno;
        //DbgMsg("channel, server, name = %s, select ret = %d, argv = %p", argv->name, ret, argv);
        if (ret < 0) {
            if (local_errno == EINTR) {
                continue;
            }

            ErrMsg("Select fail, ret = %d, errno = %s(%d)", ret, strerror(errno), errno);

            if (local_errno == 9) {
                //Bad file descriptor(9)
                for (i=0 ; i<max_client ; ++i) {
                    Channel_Client_t *ccc = &(clients[i]);
                    if (ccc->channel.fd >= 0) {
                        if (FD_ISSET(ccc->channel.fd, &iofds)) {
                            int tmp_client_fd = ccc->channel.fd;
                            SysMsg("Bad file descriptor [name:%s, fd:%d, idx:%d]", argv->name, ccc->channel.fd, i);
                            ccc->channel.fd = -1;
                            shutdown(tmp_client_fd, SHUT_RDWR);
                            close(tmp_client_fd);
                            argv->serv_read(0, argv->user_data, &(ccc->channel));
                            bad_fd_handle = 1;
                            break;
                        }
                    }
                }
            }

            if (bad_fd_handle == 1) {
                continue;
            } else {
                goto free_mem;
            }
        } else if (ret == 0) {
            continue;
        }

        //DbgMsg("channel, server, name = %s, check fd, argv = %p", argv->name, argv);
        if (FD_ISSET(serv_fd, &iofds)) {
            int tmp_fd = 0;
            int tmp_index = -1;
            struct sockaddr_in tmp_addr = {0};
            socklen_t tmp_addrlen = sizeof(struct sockaddr);

            //DbgMsg("accept the connection, name = %s, argv = %p", argv->name, argv);
            tmp_fd = accept(serv_fd, (void *)&tmp_addr, (socklen_t *) &tmp_addrlen);
            if (tmp_fd < 0) {
                ErrMsg("accept error, name = %s, errno = %s(%d)", argv->name, strerror(errno), errno);
                continue;
            }

            for (i=0 ; i<max_client ; ++i) {
                if (clients[i].channel.fd < 0) {
                    pthread_mutex_init(&(clients[i].channel.mutex), 0);
                    clients[i].channel.fd = tmp_fd;
                    tmp_index = i;
                    break;
                }
            }

            if (tmp_index < 0) {
                shutdown(tmp_fd, SHUT_RDWR);
                close(tmp_fd);
                ErrMsg("Connection pool is full, name = %s", argv->name);
            } else {
                SysMsg("Connection from %s:%d [name:%s, fd:%d, idx:%d]", inet_ntoa(tmp_addr.sin_addr), ntohs(tmp_addr.sin_port), argv->name, tmp_fd, tmp_index);
            }
        }

        for (i=0 ; i<max_client ; ++i) {
            Channel_Client_t *ccc = &(clients[i]);

            if (ccc->channel.fd >= 0) {
                //DbgMsg("channel, server, name = %s, check fd[%d] = %d", argv->name, i, ccc->channel.fd);
                if (FD_ISSET(ccc->channel.fd, &iofds)) {
                    // ret = read(ccc->channel.fd, ccc->buf + ccc->buf_len, sizeof(ccc->buf) - ccc->buf_len);
                    // DbgMsg("channel, server, name = %s, read , fd = %d (idx:%d)", argv->name, ccc->channel.fd, i);
                    ret = recv(ccc->channel.fd, ccc->buf + ccc->buf_len, sizeof(ccc->buf) - ccc->buf_len, 0);
                    //DbgMsg("channel, server, name = %s, read ret = %d, fd = %d (idx:%d)", argv->name, ret, ccc->channel.fd, i);
                    if (ret > 0) {
                        ccc->buf_len += ret;
                        while (ccc->buf_len >= (int)CHANNEL_HEADER_LEN) {
                            Channel_Data_t recv_data;

                            memcpy(&recv_data, ccc->buf, CHANNEL_HEADER_LEN);
                            if (recv_data.len == 0) {
                                recv_data.data = 0;
                                argv->serv_read(&recv_data, argv->user_data, &(ccc->channel));
                                ccc->buf_len -= (CHANNEL_HEADER_LEN);
                                memmove(ccc->buf, ccc->buf+CHANNEL_HEADER_LEN, ccc->buf_len);
                            } else if (recv_data.len <= (int)(ccc->buf_len-CHANNEL_HEADER_LEN)) {
                                recv_data.data = (char *)sip_get_mem(recv_data.len+1);
                                if (recv_data.data) {
                                    memcpy(recv_data.data, ccc->buf+CHANNEL_HEADER_LEN, recv_data.len);
                                    argv->serv_read(&recv_data, argv->user_data, &(ccc->channel));
                                    sip_free_mem(recv_data.data);
                                } else {
                                    ErrMsg("Can't allocate memory");
                                }
                                ccc->buf_len -= (CHANNEL_HEADER_LEN+recv_data.len);
                                memmove(ccc->buf, ccc->buf+CHANNEL_HEADER_LEN+recv_data.len, ccc->buf_len);
                            } else {
                                break;
                            }
                        }
                    } else if (ret == 0) {
                        int tmp_client_fd = ccc->channel.fd;

                        if (sizeof(ccc->buf) - ccc->buf_len == 0) {
                            ErrMsg("****** channel (%s) buffer full ******", argv->name);
                        }
                        SysMsg("Client disconnect [name:%s, fd:%d, idx:%d]", argv->name, ccc->channel.fd, i);
                        ccc->channel.fd = -1;
                        shutdown(tmp_client_fd, SHUT_RDWR);
                        close(tmp_client_fd);
                        argv->serv_read(0, argv->user_data, &(ccc->channel));
                    } else {
                        if (errno != EINTR) {
                            int tmp_client_fd = ccc->channel.fd;

                            ErrMsg("Read data errorm [name:%s, fd:%d, idx:%d], ret = %d, errno = %s(%d)", argv->name, ccc->channel.fd, i, ret, strerror(errno), errno);
                            ccc->channel.fd = -1;
                            shutdown(tmp_client_fd, SHUT_RDWR);
                            close(tmp_client_fd);
                            argv->serv_read(0, argv->user_data, &(ccc->channel));
                        }
                    }
                }
                //DbgMsg("channel, server, name = %s, check fd[%d] = %d done", argv->name, i, ccc->channel.fd);
            }
        }
        //DbgMsg("channel, server, name = %s, select done, argv = %p", argv->name, argv);
        //DbgMsg("channel, server, name = %s, select done 2, argv = %p", argv->name, argv);
    }

    ErrMsg("Channel_Listen_Thread exit, name = %s", argv->name);

    close(serv_fd);

free_mem :
    if (argv->name) sip_free_mem(argv->name);
    argv->name = 0;
    sip_free_mem(argv);

    return 0;
}


void *
Channel_Get_Item(void *channel, int index) {
    Channel_Obj_t *obj = (Channel_Obj_t *)channel;
    Channel_Client_t *clients = 0;

    if (!channel) {
        ErrMsg("Channel object is NULL");
        return 0;
    }

    clients = obj->channels;

    if (index < 0 || index >= obj->channels_size) {
        ErrMsg("Out of the range");
        return 0;
    }

    return &(clients[index]);
}


void *
Channel_Listen(char *name, Channel_Server_Read_callback_t cb, void *data, int max_channel) {
    Channel_Argv_t *argv = 0;
    Channel_Obj_t *obj = 0;
    struct sockaddr_un server;
    int ret = 0;
    int serv_fd = 0;

    argv = (Channel_Argv_t *)sip_get_mem(sizeof(Channel_Argv_t));
    if (!argv) {
        ErrMsg("Can't allocat memory");
        goto free_mem;
    }

    obj = (Channel_Obj_t *)sip_get_mem(sizeof(Channel_Obj_t));
    if (!obj) {
        ErrMsg("Can't allocate the memory");
        goto free_mem;
    }

    obj->channels_size = max_channel;
    obj->channels = (Channel_Client_t *)sip_get_mem(sizeof(Channel_Client_t)*(obj->channels_size));
    if (!obj->channels) {
        ErrMsg("Can't allocate the memory");
        goto free_mem;
    }
    memset(obj->channels, 0, sizeof(Channel_Client_t)*(obj->channels_size));

#ifdef ANDROID_SOCKETS
    serv_fd = android_get_control_socket(name);
    if (serv_fd < 0) {
        ErrMsg("Can't open stream socket (listen), name = %s, errno = %d(%s)", name, errno, strerror(errno));
        goto free_mem;
    } else {
        SysMsg("server open stream socket (listen), name = %s", name);
    }

#else
    serv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, name);

    if (serv_fd < 0) {
        ErrMsg("Can't open stream socket (listen)");
        goto free_mem;
    }

    {
        int val = 1;
        setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

        unlink(name);
    }

    if (bind(serv_fd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
        close(serv_fd);
        ErrMsg("Can't bind the %s, errmsg = %s(%d)", name, strerror(errno), errno);
        goto free_mem;
    }
#endif

    pthread_mutex_init(&(obj->mutex), 0);
    obj->fd = serv_fd;

    argv->serv_read = cb;
    argv->user_data = data;
    argv->channel   = obj;
    argv->name      = voip_strdup(name);

    {
        Channel_Client_t *clients = obj->channels;
        int i = 0;

        for (i=0 ; i<obj->channels_size ; ++i) {
            clients[i].channel.fd = -1;
        }
    }


    ret = Channel_Thread(&(obj->tid), argv, Channel_Listen_Thread);
    if (ret != 0) {
        ErrMsg("Can't create the listen thread");
        goto free_mem;
    }

    return obj;

free_mem :
    if (obj && obj->channels) sip_free_mem(obj->channels);
    if (obj) sip_free_mem(obj);
    if (argv) {
        if (argv->name) sip_free_mem(argv->name);
        sip_free_mem(argv);
    }

    return 0;
}


int
Channel_Wait(void *channel) {
    Channel_Obj_t *obj = (Channel_Obj_t *)channel;

    if (!channel) {
        ErrMsg("Channel object is NULL");
        return 0;
    }

    pthread_join(obj->tid, 0);

    return 0;
}


#if defined _UNIT_

#include <unistd.h>

#define UT_CHECK(f)     do { if ((f)) { printf("\tPass @ %d\n",  __LINE__); break; } else { printf("\tError @ %d\n", __LINE__); goto test_end; } } while (0)

int test1() {
    int ret = 0;

    UT_CHECK( ret == 0 );

    return 0;

test_end:
    return -1;
}


int main() {

    test1();

    return 0;
}

#endif
