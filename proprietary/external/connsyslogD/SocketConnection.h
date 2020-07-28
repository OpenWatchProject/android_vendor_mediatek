/******************************************************************************
 *
 * Filename:
 * ---------
 *   socketconnection.h
 *
 * Description:
 * ------------
 *   socketconnection declaration
 *
 *   This class defines a socket server to communicate with client MTKLogger
 *   All MTKLogger's control commands, and modem logger's status feedback
 *   commands are transferred by this socket.
 *
 *   Besides, the class will also connect to MTKLogger's socket server to send
 *   modem logger start and other data.
 *
 *****************************************************************************/

#include <sys/socket.h>
#include <cutils/sockets.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "GlbDefine.h"


#ifndef SOCKETCONNECTION_H_
#define SOCKETCONNECTION_H_

typedef struct __CLIENT {
    int fd;
    struct sockaddr addr;
} CLIENT;

class socketconnection {
private:
    int mSockListenId;
    pthread_t mThread;
    pthread_mutex_t mlock;
    pthread_mutex_t mMsglock;

    int clientConnect;
    timeval timeout;

    CLIENT  client[10];


    static void *threadStart(void *obj);
    bool commandHandler(int fd);
    void runListener();

public:
    socketconnection();
    virtual ~socketconnection();
    void startListening();
    bool sendCommandToClient(const int msgid, const char *msgdata);
    void stop();
    bool shouldtimeout;
};

#endif /* SOCKETCONNECTION_H_ */
