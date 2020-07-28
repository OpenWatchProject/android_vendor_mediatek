/****************************************************************************
*
* Copyright (c) 2014 Wi-Fi Alliance
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/


/*
 * File: mtk_pf.c
 *       This is the main program for Port Forwarding Agent.
 *
 */
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/select.h>

#include "wfa_debug.h"
#include "wfa_main.h"
#include "wfa_types.h"
#include "wfa_miscs.h"
#include "wfa_sock.h"

#define WFA_ENV_AGENT_IPADDR "WFA_ENV_AGENT_IPADDR"

char gRespStr[WFA_BUFF_512];

int gSock = -1, gCaSockfd = -1;
int gtgTransac;
char gnetIf[32] = "any";
long          itimeout = 0;

unsigned short wfa_defined_debug = WFA_DEBUG_ERR | WFA_DEBUG_WARNING | WFA_DEBUG_INFO;
unsigned short dfd_lvl = WFA_DEBUG_DEFAULT | WFA_DEBUG_ERR | WFA_DEBUG_INFO;

/*
 * the output format can be redefined for file output.
 */

int main(int argc, char *argv[])
{
    int nfds;
    struct sockaddr_in servAddr;
    unsigned short servPort, myport;
    char *servIP=NULL, *tstr=NULL;
    int bytesRcvd;
    fd_set sockSet;
    int i, nbytes;
    int tmsockfd;
    int maxfdn1;
    BYTE xcCmdBuf[WFA_BUFF_4K];
    BYTE caCmdBuf[WFA_BUFF_4K];
    char respStr[WFA_BUFF_512];

    if(argc < 3)
        {
            DPRINT_ERR(WFA_ERR, "Usage: %s <control interface> <local control agent port>\n", argv[0]);
            exit(1);
        }

    myport = atoi(argv[2]);

    if(argc > 3)
        {
            if(argc < 5)
                {
                    DPRINT_ERR(WFA_ERR, "Usage: %s <control interface> <local forwarded port> <CA IP ADDRESS> <CA PORT>\n", argv[0]);
                    exit(1);
                }
            servIP = argv[3];
            if(isIpV4Addr(argv[3])== WFA_FAILURE)
                return WFA_FAILURE;
            if(isNumber(argv[4])== WFA_FAILURE)
                return WFA_FAILURE;
            servPort = atoi(argv[4]);
            if(argc > 5)
                {
                    FILE *logfile;
                    int fd;
                    logfile = fopen(argv[5],"a");
                    if(logfile != NULL)
                        {
                            fd = fileno(logfile);
                            DPRINT_INFO(WFA_OUT,"redirecting the output to %s\n",argv[5]);
                            dup2(fd,1);
                            dup2(fd,2);
                        }
                    else
                        {
                            DPRINT_ERR(WFA_ERR, "Cant open the log file continuing without redirecting\n");
                        }
                }
        }
    else
        {
            if((tstr = getenv("WFA_ENV_AGENT_IPADDR")) == NULL)
                {
                    DPRINT_ERR(WFA_ERR, "Environment variable WFA_ENV_AGENT_IPADDR not set or specify DUT IP/PORT\n");
                    exit(1);
                }
            if(isIpV4Addr(tstr)== WFA_FAILURE)
                return WFA_FAILURE;
            servIP= tstr;
            if((tstr = getenv("WFA_ENV_AGENT_PORT")) == NULL)
                {
                    DPRINT_ERR(WFA_ERR, "Environment variable WFA_ENV_AGENT_PORT not set or specify DUT IP/PORT\n");
                    exit(1);
                }
            if(isNumber(tstr)== WFA_FAILURE)
                return WFA_FAILURE;
            servPort = atoi(tstr);
        }

    tmsockfd = wfaCreateTCPServSock(myport);

    maxfdn1 = tmsockfd + 1;

    FD_ZERO(&sockSet);
    if(gSock == -1)
        {
            if ((gSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
                {
                    DPRINT_ERR(WFA_ERR, "socket() failed: %i", errno);
                    exit(1);
                }

            memset(&servAddr, 0, sizeof(servAddr));
            servAddr.sin_family      = AF_INET;
            servAddr.sin_addr.s_addr = inet_addr(servIP);
            servAddr.sin_port        = htons(servPort);

            if (connect(gSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
                {
                    DPRINT_ERR(WFA_ERR, "connect() failed: %i", errno);
                    exit(1);
                }

        }
	DPRINT_INFO(WFA_OUT, "Port Forwarding Agent started successfully \n");

    for(;;)
        {
            FD_ZERO(&sockSet);
            FD_SET(tmsockfd, &sockSet);
            maxfdn1 = tmsockfd + 1;

            if(gCaSockfd != -1)
                {
                    FD_SET(gCaSockfd, &sockSet);
                    if(maxfdn1 < gCaSockfd)
                        maxfdn1 = gCaSockfd +1;
                }

            if(gSock != -1)
                {
                    FD_SET(gSock, &sockSet);
                    if(maxfdn1 < gSock)
                        maxfdn1 = gSock +1;
                }

            if((nfds = select(maxfdn1, &sockSet, NULL, NULL, NULL)) < 0)
                {
                    if(errno == EINTR)
                        continue;
                    else
                        DPRINT_WARNING(WFA_WNG, "select error %i", errno);
                }

            DPRINT_INFO(WFA_OUT, "\nnew event \n");
            if(FD_ISSET(tmsockfd, &sockSet))
                {
                    gCaSockfd = wfaAcceptTCPConn(tmsockfd);
                    DPRINT_INFO(WFA_OUT, "accept new connection\n");
                    continue;
                }

            if(gCaSockfd > 0 && FD_ISSET(gCaSockfd, &sockSet))
                {
                    memset(xcCmdBuf, 0, WFA_BUFF_4K);
                    memset(gRespStr, 0, WFA_BUFF_512);

                    nbytes = wfaCtrlRecv(gCaSockfd, xcCmdBuf);
                    if(nbytes <=0)
                        {
                            shutdown(gCaSockfd, SHUT_WR);
                            close(gCaSockfd);
                            gCaSockfd = -1;
                            continue;
                        }

                    DPRINT_INFO(WFA_OUT, "message %s %i\n", xcCmdBuf, nbytes);

                    if(gSock == -1)
                        {
                            if ((gSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
                                {
                                    DPRINT_ERR(WFA_ERR, "socket() failed: %i", errno);
                                    exit(1);
                                }

                            memset(&servAddr, 0, sizeof(servAddr));
                            servAddr.sin_family      = AF_INET;
                            servAddr.sin_addr.s_addr = inet_addr(servIP);
                            servAddr.sin_port        = htons(servPort);

                            if (connect(gSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
                                {
                                    DPRINT_ERR(WFA_ERR, "connect() failed: %i", errno);
                                    exit(1);
                                }
                        }

                    /*
                     * send to CA.
                     */
                    if(send(gSock, xcCmdBuf, nbytes, 0) != nbytes)
                        {
                            DPRINT_WARNING(WFA_WNG, "Incorrect sending ...\n");
                            continue;
                        }

                    DPRINT_INFO(WFA_OUT, "sent to CA\n");
                } /* done with gCaSockfd */

            if(gSock > 0 && FD_ISSET(gSock, &sockSet))
                {
                    DPRINT_INFO(WFA_OUT, "received from CA\n");
                    sleep(1);
                    memset(respStr, 0, WFA_BUFF_128);
                    memset(caCmdBuf, 0, WFA_BUFF_4K);
                    if ((bytesRcvd = recv(gSock, caCmdBuf, WFA_BUFF_4K, 0)) <= 0)
                        {
                            DPRINT_WARNING(WFA_WNG, "recv() failed or connection closed prematurely");
                            continue;
                        }

                    for(i = 0; i< bytesRcvd; i++)
                        printf("%x ", caCmdBuf[i]);
                    printf("\n");

                    wfaCtrlSend(gCaSockfd, (BYTE *)caCmdBuf, bytesRcvd);
                } /* if(gCaSock */

        } /* for */

    close(gSock);
    exit(0);
}
