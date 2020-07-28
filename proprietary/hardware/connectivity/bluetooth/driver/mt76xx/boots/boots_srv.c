//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include "boots.h"
#include "boots_pkt.h"
#include "boots_btif.h"
#include "boots_skt.h"
#include "boots_uart.h"
#include "boots_eth.h"

//---------------------------------------------------------------------------
#define LOG_TAG "boots_srv"

//---------------------------------------------------------------------------
static boots_if_s boots_srv_if;
static int cont = 1;
static int uart_speed = 0;
int fd[BOOTS_BTIF_ALL] = {-1};

/****************************************************************************
 *              F U N C T I O N   D E C L A R A T I O N S                   *
 ****************************************************************************/

//---------------------------------------------------------------------------
static void boots_srv_sig_handler(int signum)
{
    BPRINT_D("%s: signum: %d", __func__, signum);
    cont = 0;
}

//---------------------------------------------------------------------------
static void boots_srv_help(void)
{
    printf("Boots(server) - MTK Bluetooth Test Suite ver:%s\n", VERSION);
    printf("Usage:\n");
    printf("    boots_srv <[Input interface] [buadrate]> [BT Interface]\n");
    printf("Input Interface:\n");
    printf("\tCli/Srv connection by socket if you ignore this argument\n");
    printf("\tttyX      Specific UART for data issuer, baudate is for real port\n");
    printf("\t              ex: ./boots_srv ttyGS2\n");
    printf("\t              ex: ./boots_srv ttyUSB0 115200\n");
    printf("\tethX      Specific Eth for data issuer, need client IP\n");
    printf("\t              ex: ./boots_srv eth0 10.1.1.2\n");
    printf("BT Interface:\n");
    printf("\tAuto detect stpbt/HCI, if you ignore this argument\n");
    printf("\tstpbt     MTK stpbt Interface for BT controller, ex: ./boots_srv stpbt\n");
    /* printf("\thci       HCI Interface for BT controller\n"); */
}

//---------------------------------------------------------------------------
static void boots_srv_chk_if(int argc, char **argv)
{
    int i = 0;
#ifdef BOOTS_VERBOSE_MSG
    BPRINT_D("%s: argc = %d", __func__, argc);
    for (i = 0; i < argc; i++)
        BPRINT_D("argv[%d]:%s", i, argv[i]);
    i = 0;
#endif

    while (i < argc) {
        if (!memcmp(argv[i], "stpbt", strlen("stpbt"))) {
            boots_srv_if.btif = BOOTS_BTIF_STPBT;
/*
        } else if (!memcmp(argv[i], "hci", strlen("hci"))) {
            boots_srv_if.btif = BOOTS_BTIF_HCI;
*/
        } else if (!memcmp(argv[i], "tty", strlen("tty"))) {
            boots_srv_if.csif = BOOTS_CSIF_UART;
            snprintf(boots_srv_if.cs, sizeof(boots_srv_if.cs), "/dev/%s", argv[i]);
            BPRINT_D("%s: %s", __func__, boots_srv_if.cs);
            if (i + 1 < argc) {
                uart_speed = atoi(argv[i + 1]);
                BPRINT_D("%s: UART speed: %d", __func__, uart_speed);
                i += 2;
                continue;
            }

        } else if (!memcmp(argv[i], "eth", strlen("eth"))) {
            boots_srv_if.csif = BOOTS_CSIF_ETH;
            memcpy(boots_srv_if.cs, argv[i+1], strlen(argv[i+1]));
            i++;
        }
        i++;
    }

    if (boots_srv_if.csif == BOOTS_IF_NONE)
        boots_srv_if.csif = BOOTS_CSIF_SKT;

    if (boots_srv_if.btif == BOOTS_IF_NONE) {
        if (access(boots_btif[0].p, R_OK) == 0) {
            boots_srv_if.btif = BOOTS_BTIF_STPBT;
        } else {
            boots_srv_if.btif = BOOTS_BTIF_HCI;
        }
    }
}

//---------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    static uint8_t buf[HCI_BUF_SIZE] = {0};
    int btfd = -1;      // Interface file descriptor with BT controller
    ssize_t len = 0;
    struct sigaction sigact;
    fd_set readfs;

    // For root permission
    CHECK_USERID();

    if (argc >= 2 && !memcmp("-h", argv[1], strlen("-h"))) {
        boots_srv_help();
        exit(0);
    }

    sigact.sa_handler = boots_srv_sig_handler;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGINT, &sigact, NULL);
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGQUIT, &sigact, NULL);
    sigaction(SIGKILL, &sigact, NULL);

    boots_srv_chk_if(argc - 1, &argv[1]);

    /** Communication Interface with BT controller */
    if (boots_srv_if.btif != BOOTS_IF_NONE)
        btfd = boots_btif_open(boots_srv_if.btif);
    if (btfd < 0) {
        BPRINT_E("Set interface failed(%d), please check driver and controller", boots_srv_if.btif);
        boots_srv_help();
        exit(1);
    } else {
        // Please follow the boots_if_e
        char *c_inf[] = {"None", "stpbt", "hci", "All", "Socket", "UART", "Ethernet",
            "User", "Tester_UART"};

        BPRINT_I("client <-> %s(%d) <-> %s(%d) <-> BT", c_inf[boots_srv_if.csif],
                boots_srv_if.csif, c_inf[boots_srv_if.btif], boots_srv_if.btif);
    }

    /** open interface to connect upper layer */
    if (boots_srv_if.csif == BOOTS_CSIF_SKT) {
        int skfd_srv = -1;  // Interface file descriptor with host
        int skfd_cli = -1;  // Interface file descriptor with host
        struct sockaddr_un skaddr_srv;
        struct sockaddr_un skaddr_cli;
        socklen_t sklen_srv = {0};
        socklen_t sklen_cli = {0};

        if (boots_sk_create(&skfd_srv, &skaddr_srv, &sklen_srv, BOOTS_SRVSK_NAME, 0)) {
            BPRINT_E("%s: Create server socket failed", __func__);
            goto BTOFF;
        }

        BPRINT_I("Local relaying...");
        while (cont && btfd >= 0) {
            FD_ZERO(&readfs);
            memset(buf, 0, sizeof(buf));

            if (btfd >= 0)
                FD_SET(btfd, &readfs);
            if (skfd_srv >= 0)
                FD_SET(skfd_srv, &readfs);

            if (select(MAX(btfd, skfd_srv) + 1, &readfs, NULL, NULL, NULL) > 0) {
                // Process the event from BT controller, read
                if (FD_ISSET(btfd, &readfs)) {
                    len = boots_btif_read(btfd, buf, sizeof(buf));
                    if (len > 0 && skfd_cli >= 0) {
                        boots_sk_send(skfd_cli, (void *)buf, len, &skaddr_cli, BOOTS_CLISK_NAME);
                    } else if (len > 0) {
                        boots_pkt_handler(buf, len, NULL);
                    } else if (len < 0) {
                        BPRINT_E("%s: Read data from chip failed", __func__);
                        btfd = -1;
                        break;
                    }
                }
                // Process the command from host, write
                if (FD_ISSET(skfd_srv, &readfs)) {
                    // connect socket, server
                    if (skfd_cli == -1 &&
                            boots_sk_create(&skfd_cli, &skaddr_cli, &sklen_cli, BOOTS_CLISK_NAME, 1)) {
                        BPRINT_E("%s: Connect client socket failed", __func__);
                        boots_sk_close(&skfd_srv);
                        goto BTOFF;
                    }
                    len = boots_sk_recv(skfd_srv, buf, sizeof(buf), 0, NULL, NULL);

                    if (len == 4 && buf[0] == 's' && buf[1] == 'e' && buf[2] == 'n' && buf[3] == 'd') {
                        // client socket end
                        boots_sk_close(&skfd_cli);
                        skfd_cli = -1;
                    } else if (buf[0] == 0x01 && buf[1] == 0x01 && buf[2] == 0xFF) {
                        // read/write file
                        boots_file_wr(buf, &len);
                        boots_sk_send(skfd_cli, (void *)buf, len, &skaddr_cli, BOOTS_CLISK_NAME);
                    // write it to BT chip
                    } else if (!boots_btif_write(btfd, buf, len)) {
                        BPRINT_E("Write data to chip failed, please check controller");
                        btfd = -1;
                        break;
                    }
                }
            }
        }
        boots_sk_close(&skfd_cli);
        boots_sk_close(&skfd_srv);

    } else if (boots_srv_if.csif == BOOTS_CSIF_UART) {
        int uartfd = -1;

        uartfd = boots_uart_init(boots_srv_if.cs, uart_speed);
        if (uartfd < 0) {
            BPRINT_E("%s: Open serial port:%s failed(%d)", __func__, boots_srv_if.cs, uartfd);
            goto BTOFF;
        }

        BPRINT_I("With UART(%s) Relaying...", boots_srv_if.cs);
        while (cont && btfd >= 0) {
            FD_ZERO(&readfs);
            memset(buf, 0, sizeof(buf));

            if (btfd >= 0)
                FD_SET(btfd, &readfs);
            if (uartfd >= 0)
                FD_SET(uartfd, &readfs);

            if (select(MAX(btfd, uartfd) + 1, &readfs, NULL, NULL, NULL) > 0) {
                // Process the event from BT controller, read
                if (FD_ISSET(btfd, &readfs)) {
                    len = boots_btif_read(btfd, buf, sizeof(buf));
                    if (len > 0) {
                        boots_uart_write(uartfd, buf, len);
                        boots_pkt_handler(buf, len, NULL);
                    } else if (len < 0) {
                        BPRINT_E("%s: Read data from chip failed", __func__);
                        btfd = -1;
                        break;
                    }
                }
                // Process the command from host, write
                if (FD_ISSET(uartfd, &readfs)) {
                    len = boots_uart_read(uartfd, buf, sizeof(buf));
                    if (len > 0) {
                        if (!boots_btif_write(btfd, buf, len)) {
                            BPRINT_E("%s: Write command to chip failed", __func__);
                        }
                        boots_pkt_handler(buf, len, NULL);
                    }
                }
                fsync(btfd);
                fsync(uartfd);
            } else {
                BPRINT_D("%s(%d)", strerror(errno), errno);
            }
        }
        close(uartfd);
    } else if (boots_srv_if.csif == BOOTS_CSIF_ETH) {
        int skfd_srv = -1;  // Interface file descriptor with host
        int skfd_cli = -1;  // Interface file descriptor with host
        struct sockaddr_in skaddr_srv;
        struct sockaddr_in skaddr_cli;

        if (boots_eth_create(&skfd_srv, &skaddr_srv, 0)) {
            BPRINT_E("%s: Create server socket failed", __func__);
            goto BTOFF;
        }

        BPRINT_I("Ethernet relaying...");
        while (cont && btfd >= 0) {
            FD_ZERO(&readfs);
            memset(buf, 0, sizeof(buf));

            if (btfd >= 0)
                FD_SET(btfd, &readfs);
            if (skfd_srv >= 0)
                FD_SET(skfd_srv, &readfs);

            if (select(MAX(btfd, skfd_srv) + 1, &readfs, NULL, NULL, NULL) > 0) {
                // Process the event from BT controller, read
                if (FD_ISSET(btfd, &readfs)) {
                    len = boots_btif_read(btfd, buf, sizeof(buf));
                    if (len > 0 && skfd_cli >= 0) {
                        skaddr_cli.sin_family = AF_INET;
                        skaddr_cli.sin_port = ETH_UDP_SRV_PORT;
                        skaddr_cli.sin_addr.s_addr = inet_addr(boots_srv_if.cs);
                        boots_eth_send(skfd_cli, (void *)buf, len, &skaddr_cli);
                    } else if (len > 0) {
                        boots_pkt_handler(buf, len, NULL);
                    } else if (len < 0) {
                        BPRINT_E("%s: Read data from chip failed", __func__);
                        btfd = -1;
                        break;
                    }
                }
                // Process the command from host, write
                if (FD_ISSET(skfd_srv, &readfs)) {
                    // connect socket, server
                    if (skfd_cli == -1 &&
                            boots_eth_create(&skfd_cli, &skaddr_cli, 1)) {
                        BPRINT_E("%s: Connect client socket failed", __func__);
                        boots_eth_close(&skfd_srv);
                        goto BTOFF;
                    }
                    len = boots_eth_recv(skfd_srv, buf, sizeof(buf), 0);

                    if (len == 4 && buf[0] == 's' && buf[1] == 'e' && buf[2] == 'n' && buf[3] == 'd') {
                        // client socket end
                        boots_eth_close(&skfd_cli);
                        skfd_cli = -1;
                    } else if (buf[0] == 0x01 && buf[1] == 0x01 && buf[2] == 0xFF) {
                        // read/write file
                        boots_file_wr(buf, &len);
                        boots_eth_send(skfd_cli, (void *)buf, len, &skaddr_cli);
                    // write it to BT chip
                    } else if (!boots_btif_write(btfd, buf, len)) {
                        BPRINT_E("%s: Write command to chip failed", __func__);
                        btfd = -1;
                        break;
                    }
                }
            }
        }
        boots_eth_close(&skfd_cli);
        boots_eth_close(&skfd_srv);
    }

BTOFF:
    boots_btif_close(boots_srv_if.btif);
    BPRINT_I("Service STOP!");
    return 0;
}

//---------------------------------------------------------------------------
