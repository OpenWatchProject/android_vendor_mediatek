#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>

int unlockpt(int fd);
int grantpt(int fd);
char *ptsname(int fd);

#include "atch.h"
#include <syslog.h>
#include <mtk_log.h>

#ifdef __ANDROID_SOCKET__
 #include <cutils/sockets.h>
#endif

/* debug macro */
//#define ErrMsg(f, a...)   syslog(LOG_ERR,   "[ATCH-ERR] [%s: %d]: " f "\n", &__FILE__[0], __LINE__, ## a)
#define SysMsg(f, a...)   syslog(LOG_INFO,  "[ATCH-SYS] [%s: %d]: " f "\n", &__FILE__[0], __LINE__, ## a)
//#define DbgMsg(f, a...)   syslog(LOG_DEBUG, "[ATCH-DBG] [%s: %d]: " f "\n", &__FILE__[0], __LINE__, ## a)
#define NA_LOG_TAG "Atch"
#define DbgMsg(...) ((void)mtkLogD(NA_LOG_TAG, __VA_ARGS__))
#define ErrMsg(...) ((void)mtkLogE(NA_LOG_TAG, __VA_ARGS__))

#define SYSCHECK(c) do{if((c)<0){ \
                        DbgMsg("system-error: '%s' (code: %d)", strerror(errno), errno);\
                        }\
                    }while(0)

typedef struct _ATChannel {
    int             type;
    int             fd;
    FILE           *fp;
    char           *name;
    char           *buffer;
    int             buffer_len;
    int             buffer_offset;
} ATChannel_t;


void *
atch_client(int type, char *name) {
    ATChannel_t  *atch = 0;
    int fd = 0;

    if (type <= ATCH_Type_MIN || type >= ATCH_Type_MAX) {
        ErrMsg("Invalid type : %d", type);
        return 0;
    }

    switch (type) {
        case ATCH_Type_RILD : {
            fd = open(name, O_RDWR | O_NONBLOCK);
            if (fd < 0) {
                ErrMsg("could not connect to %s: %s", name, strerror(errno));
                return 0;
            } else {
                struct termios ios;
                tcgetattr(fd, &ios);
                ios.c_lflag = 0; /* disable ECHO, ICANON, etc... */
                ios.c_iflag = 0;
                tcsetattr(fd, TCSANOW, &ios);
            }
            break;
        }
        case ATCH_Type_UART : {
            unsigned int    baud = B115200;
            struct termios  options;

            fd = open(name, O_RDWR | O_NOCTTY | O_NONBLOCK);
            if (fd < 0) {
                ErrMsg("Can't open device = (%s)", name);
                return 0;
            }

            tcgetattr(fd, &options);
            cfmakeraw(&options);
            cfsetospeed(&options, baud);
            if (tcsetattr(fd, TCSAFLUSH, &options) != 0) {
                ErrMsg("setting fd tc");
                close(fd);
                return 0;
            }

            tcflush(fd, TCIFLUSH);
            break;
        }
        case ATCH_Type_TCP  :
            ErrMsg("not support yet");
            return 0;
            break;
        case ATCH_Type_UTCP : {
#ifndef __ANDROID_SOCKET__
            struct sockaddr_un server;

            fd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (fd < 0) {
                ErrMsg("Can't open stream socket (%s)", name);
                return 0;
            }

            server.sun_family = AF_UNIX;
            memset(server.sun_path, '\0', sizeof(server.sun_path));
            strncpy(server.sun_path, name, sizeof(server.sun_path) - 1);

            if (connect(fd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
                close(fd);
                ErrMsg("Can't connect to server side");
                return 0;
            }
#else
            fd = socket_local_client(name, ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
            if (fd < 0) {
                ErrMsg("Can't open stream socket");
                return 0;
            }
#endif
            break;
        }
        default :
            ErrMsg("Invalid type : %d", type);
            return 0;
    }

    atch = malloc(sizeof(ATChannel_t));
    if (!atch) {
        ErrMsg("Can't allocate memory");
        close(fd);
        return 0;
    }

    if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
        DbgMsg("Can't set the FD_CLOEXEC");
    }
    if (fcntl(fd, F_SETFL, 0) < 0) {
        DbgMsg("Can't set the F_SETFL");
    }

    memset(atch, 0, sizeof(ATChannel_t));
    atch->type = type;
    atch->fd   = fd;
    atch->fp   = fdopen(fd, "r+");
    atch->name = strdup(name);
    atch->buffer = 0;
    atch->buffer_len = 0;
    atch->buffer_offset = 0;

    DbgMsg("client connected, type = %d, fd = %d, fp = %p", atch->type, atch->fd, atch->fp);

    return atch;
}



void *
atch_server(int type, char *name) {
    ATChannel_t  *atch = 0;
    int fd = 0;

    switch (type) {
        case ATCH_Type_UART : {
#if 1
            struct termios options;
            char *pts = 0;

            fd = open("/dev/ptmx", O_RDWR | O_NONBLOCK);
            //fd = open("/dev/ptmx", O_RDWR);
            if (fd < 0) {
                ErrMsg("Can't open the ptmx");
                return 0;
            }

            pts = ptsname(fd);
            if (!pts) {
                ErrMsg("Can't get the pts");
                close(fd);
                return 0;
            }

            tcgetattr(fd, &options); //get the parameters
            memset(&options, 0, sizeof(options));

            options.c_cflag = B460800 | CREAD | CLOCAL | CS8 ;
            options.c_iflag = IGNPAR;
            options.c_lflag = 0; /* disable CANON, ECHO*, etc */
            options.c_oflag = 0; /* set raw output */
            options.c_cc[VTIME] = 0;
            options.c_cc[VMIN] = 1;

            tcflush(fd, TCIFLUSH);
            tcsetattr(fd, TCSANOW, &options);

            SYSCHECK(grantpt(fd));
            SYSCHECK(unlockpt(fd));

            unlink(name);
            if (chmod(pts, 0666) < 0) {
                DbgMsg("Can't change the mode");
            }
            symlink(pts, name);

            DbgMsg("pts : (%s), name : (%s)\n", pts, name);
            break;
#else
            ErrMsg("not support yet");
            return 0;
#endif
        }
        case ATCH_Type_TCP  :
            ErrMsg("not support yet");
            return 0;
            break;
        case ATCH_Type_UTCP : {
#ifndef __ANDROID_SOCKET__
            struct sockaddr_un server;

            fd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (fd < 0) {
                ErrMsg("Can't open stream socket (%s)", name);
                return 0;
            }

            server.sun_family = AF_UNIX;
            memset(server.sun_path, '\0', sizeof(server.sun_path));
            strncpy(server.sun_path, name, sizeof(server.sun_path) - 1);
            unlink(name);

            if (bind(fd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
                ErrMsg("Can't create the listen thread");
                close(fd);
                return 0;
            }
#else
            fd = android_get_control_socket(name);
            if (fd < 0) {
                ErrMsg("Can't open stream socket");
                return 0;
            }
#endif

            if (listen(fd, 4) < 0) {
                ErrMsg("Can't listen the connection");
            }

            do {
                struct sockaddr tmp_addr;
                int tmp_addrlen = sizeof(struct sockaddr);
                int cli_fd = 0;

                cli_fd = accept(fd,  (struct sockaddr *) &tmp_addr, (socklen_t *) &tmp_addrlen);
                if (cli_fd < 0) {
                    ErrMsg("accept failed");
                    close(fd);
                    return 0;
                }

                close(fd);
                fd = cli_fd;
            } while (0);

            break;
        }
        default :
            ErrMsg("Invalid type : %d", type);
            return 0;
    }

    atch = malloc(sizeof(ATChannel_t));
    if (!atch) {
        ErrMsg("Can't allocate memory");
        close(fd);
        return 0;
    }

    if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
        DbgMsg("Can't set the FD_CLOEXEC");
    }
    if (fcntl(fd, F_SETFL, 0) < 0) {
        DbgMsg("Can't set the F_SETFL");
    }

    memset(atch, 0, sizeof(ATChannel_t));
    atch->type = type;
    atch->fd   = fd;
    atch->fp   = fdopen(fd, "r+");
    atch->name = strdup(name);
    atch->buffer = 0;
    atch->buffer_len = 0;
    atch->buffer_offset = 0;

    DbgMsg("server connected, type = %d, fd = %d, fp = %p", atch->type, atch->fd, atch->fp);

    return atch;
}


void *
atch_new(int type, int fd) {
    ATChannel_t *atch = 0;

    if (type != ATCH_Type_USER_Client && type != ATCH_Type_USER_Server) {
        ErrMsg("Error type = %d", type);
        return 0;
    }

    atch = malloc(sizeof(ATChannel_t));
    if (!atch) {
        ErrMsg("Can't allocate memory");
        return 0;
    }

    if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
        DbgMsg("Can't set the FD_CLOEXEC");
    }

    memset(atch, 0, sizeof(ATChannel_t));
    atch->type = type;
    atch->fd   = fd;
    atch->name = 0;
    atch->buffer = 0;
    atch->buffer_len = 0;
    atch->buffer_offset = 0;

    if (type == ATCH_Type_USER_Client) {
        atch->fp   = fdopen(fd, "w");
    } else {
        atch->fp   = fdopen(fd, "r");
    }

    DbgMsg("new user atch, type = %d, fd = %d, fp = %p", atch->type, atch->fd, atch->fp);

    return atch;
}


int
atch_close(void *channel) {
    ATChannel_t  *atch = (ATChannel_t *)channel;
    int ret = 0;

    if (!atch) {
        ErrMsg("channel is NULL");
        return -1;
    }

    if (atch->buffer) {
        free(atch->buffer);
        atch->buffer = 0;
    }

    if (atch->name) {
        unlink(atch->name);
        free(atch->name);
        atch->name = 0;
    }

    if (atch->fd >= 0) {
        ret = close(atch->fd);
        atch->fd = 0;
        atch->fp = 0;
        if (ret == 0) {
            free(atch);
            return 0;
        }
        return ret;
    } else {
        atch->fd = 0;
        atch->fp = 0;
        free(atch);
        return 0;
    }
}



int
atch_puts(void *channel, char *buf) {
    ATChannel_t  *atch = (ATChannel_t *)channel;
    int ret = 0;

    if (!atch) {
        ErrMsg("channel is NULL");
        return -1;
    }

    if (!(atch->fp)) {
        ErrMsg("channel fd is NULL");
        return -1;
    }

    if (!(buf)) {
        ErrMsg("buffer is NULL");
        return -1;
    }

    ret = fputs(buf, atch->fp);
    fflush(atch->fp);

    return ret;
}


char *
atch_gets(void *channel, char *buf, int len) {
    ATChannel_t  *atch = (ATChannel_t *)channel;
    char *ret = 0;

    if (!atch) {
        ErrMsg("channel is NULL");
        return 0;
    }

    if (!(atch->fp)) {
        ErrMsg("channel fd is NULL");
        return 0;
    }

    if (!(buf)) {
        ErrMsg("buffer is NULL");
        return 0;
    }

    ret = fgets(buf, len, atch->fp);
    fflush(atch->fp);

    return ret;
}


int
atch_send(void *channel, char *buf, int len) {
    ATChannel_t  *atch = (ATChannel_t *)channel;
    int ret = 0;
    int offset = 0;

    if (!atch) {
        ErrMsg("channel is NULL");
        return -1;
    }

    if (atch->fd < 0) {
        ErrMsg("fd is negative");
        return -1;
    }

    if (!(buf)) {
        ErrMsg("buffer is NULL");
        return -1;
    }

    if (len <= 0) {
        ErrMsg("len <= 0");
        return -1;
    }

    while (offset < len) {
        ret = write(atch->fd, buf+offset, len-offset);
        if (ret > 0) {
            offset += ret;
        } else {
            break;
        }
    }

    return offset;
}


int
atch_recv(void *channel, char *buf, int len) {
    ATChannel_t  *atch = (ATChannel_t *)channel;

    if (!atch) {
        ErrMsg("channel is NULL");
        return -1;
    }

    if (atch->fd < 0) {
        ErrMsg("fd is negative");
        return -1;
    }

    if (!(buf)) {
        ErrMsg("buffer is NULL");
        return -1;
    }

    if (len <= 0) {
        ErrMsg("len <= 0");
        return -1;
    }

    return read(atch->fd, buf, len);
}


int
atch_getfd(void *channel) {
    ATChannel_t  *atch = (ATChannel_t *)channel;

    if (!atch) {
        ErrMsg("channel is NULL");
        return -1;
    }

    return atch->fd;
}


int
atch_fflush(void *channel) {
    ATChannel_t  *atch = (ATChannel_t *)channel;

    if (!atch) {
        ErrMsg("channel is NULL");
        return -1;
    }

    if (!(atch->fp)) {
        ErrMsg("channel fd is NULL");
        return -1;
    }

    return fflush(atch->fp);
}


int
atch_set2buffer(void *channel, int size) {
    ATChannel_t  *atch = (ATChannel_t *)channel;

    if (!atch) {
        ErrMsg("channel is NULL");
        return -1;
    }

    if (atch->buffer) {
        ErrMsg("buffer is allocate");
        return -1;
    }

    atch->buffer = malloc(size);

    if (!(atch->buffer)) {
        ErrMsg("can't allocate buffer");
        return -1;
    }

    memset(atch->buffer, 0, size); //add memset to clear
    atch->buffer_len = size-1;
    atch->buffer_offset = 0;

    return 0;
}


int
atch_read2buffer(void *channel) {
    ATChannel_t  *atch = (ATChannel_t *)channel;
    int ret = 0;

    if (!atch) {
        ErrMsg("channel is NULL");
        return -1;
    }

    if (!(atch->buffer)) {
        ErrMsg("buffer is NULL");
        return -1;
    }

    if (atch->fd < 0) {
        ErrMsg("fd is negative");
        return -1;
    }

    ret = read(atch->fd, atch->buffer+atch->buffer_offset, atch->buffer_len-atch->buffer_offset);
    if (ret <= 0) {
        return ret;
    }

    atch->buffer_offset += ret;
    atch->buffer[atch->buffer_offset] = 0;

    return ret;
}

char *
atch_getline(void *channel) {
    ATChannel_t  *atch = (ATChannel_t *)channel;
    char *line = 0;
    char *str = 0;

    if (!atch) {
        ErrMsg("channel is NULL");
        return 0;
    }

    if (!(atch->buffer)) {
        ErrMsg("buffer is NULL");
        return 0;
    }

    line = atch->buffer;
    while (*line) {
        char last_char = 0;
        char *ptr = 0;

        ptr = strchr(line, '\n');
        if (!ptr) {
            ptr = strchr(line, '\r');
            if (!ptr) {
                break;
            }
        }

        last_char = *(ptr+1);
        *(ptr+1) = 0;

        str = strdup(line);

        *(ptr+1) = last_char;
        line = ptr+1;
        break;
    }

    if (str) {
        atch->buffer_offset = atch->buffer_offset - (line-atch->buffer);
        if (atch->buffer_offset > 0 && (line != atch->buffer)) {
            memmove(atch->buffer, line, atch->buffer_offset);
        }
        atch->buffer[atch->buffer_offset] = 0;
    }

    return str;
}
