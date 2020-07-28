#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "netagent_io.h"

#define Dbg(f, a...)    printf("D " f " (%s:%d)\n", ## a, __FILE__, __LINE__)
#define Err(f, a...)    printf("E " f " (%s:%d)\n", ## a, __FILE__, __LINE__)

#if 1
void send_if_up(void *netagent_io_obj, unsigned int interface_id, unsigned int addr_type) {
    void *netagent_io_cmd_obj = 0;
    netagent_io_cmd_obj = netagent_io_cmd_ifst_alloc(interface_id, NETAGENT_IO_IFST_UP, addr_type);
    if (netagent_io_cmd_obj == NULL) {
        return;
    }
    netagent_io_send(netagent_io_obj, netagent_io_cmd_obj);
    netagent_io_cmd_free(netagent_io_cmd_obj);
}

void send_if_down(void *netagent_io_obj, unsigned int interface_id) {
    void *netagent_io_cmd_obj = 0;
    netagent_io_cmd_obj = netagent_io_cmd_ifst_alloc(interface_id, NETAGENT_IO_IFST_DOWN, 0);
    if (netagent_io_cmd_obj == NULL) {
        return;
    }
    netagent_io_send(netagent_io_obj, netagent_io_cmd_obj);
    netagent_io_cmd_free(netagent_io_cmd_obj);
}

void send_if_chg(void *netagent_io_obj, unsigned int interface_id, unsigned int addr_type, unsigned char *addr) {
    void *netagent_io_cmd_obj = 0;
    netagent_io_cmd_obj = netagent_io_cmd_ipupdate_alloc(interface_id, addr_type, (unsigned int *)addr, -1);
    if (netagent_io_cmd_obj == NULL) {
        return;
    }
    netagent_io_send(netagent_io_obj, netagent_io_cmd_obj);
    netagent_io_cmd_free(netagent_io_cmd_obj);
}


static void *
task_stub(void *arg) {
    void    *netagent_io_obj = arg;
    void    *netagent_io_cmd_obj = 0;

    while (1) {
        netagent_io_cmd_obj = netagent_io_recv(netagent_io_obj);
        if (netagent_io_cmd_obj) {
            netagent_io_cmd_e cmd;
            netagent_io_addr_type_e type;
            unsigned int addr_v4;
            unsigned int addr_v6[4];

            if (netagent_io_get_cmd_type(netagent_io_cmd_obj, &cmd) == NETAGENT_IO_RET_SUCCESS) {
                switch (cmd) {
                    case NETAGENT_IO_CMD_IFUP : {
                        if (netagent_io_get_addr_type(netagent_io_cmd_obj, &type) == NETAGENT_IO_RET_SUCCESS) {
                            unsigned int if_id = 0;
                            char *reason = NULL;
                            netagent_io_get_if_id(netagent_io_cmd_obj, &if_id);
                            netagent_io_get_ip_change_reason(netagent_io_cmd_obj, &reason);
                            switch (type) {
                                case NETAGENT_IO_ADDR_TYPE_IPv4 :
                                    if (netagent_io_get_addr_v4(netagent_io_cmd_obj, &addr_v4) == NETAGENT_IO_RET_SUCCESS) {
                                        unsigned char *ptr = (unsigned char *)&addr_v4;
                                        Dbg("### get ifup event with v4 address = %d.%d.%d.%d, if = %d, reason = %s", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), if_id, reason);
                                    } else {
                                        Err("get v4 address fail");
                                    }
                                    send_if_up(netagent_io_obj, if_id, type);
                                    break;
                                case NETAGENT_IO_ADDR_TYPE_IPv6 :
                                    if (netagent_io_get_addr_v6(netagent_io_cmd_obj, addr_v6) == NETAGENT_IO_RET_SUCCESS) {
                                        unsigned char *ptr = (unsigned char *)&addr_v6;
                                        Dbg("### get ifup event with v6 address %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X, if = %d, reason = %s",
                                          *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7), *(ptr+8), *(ptr+9), *(ptr+10), *(ptr+11), *(ptr+12), *(ptr+13), *(ptr+14), *(ptr+15), if_id, reason);
                                    } else {
                                        Err("get v6 address fail");
                                    }
                                    send_if_up(netagent_io_obj, if_id, type);
                                    break;
                                case NETAGENT_IO_ADDR_TYPE_IPv4v6:
                                    if (netagent_io_get_addr_v4(netagent_io_cmd_obj, &addr_v4) == NETAGENT_IO_RET_SUCCESS) {
                                        unsigned char *ptr = (unsigned char *)&addr_v4;
                                        Dbg("### get ifup event with v4 address = %d.%d.%d.%d, if = %d, reason = %s", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), if_id, reason);
                                    } else {
                                        Err("get v4 address fail");
                                    }
                                    if (netagent_io_get_addr_v6(netagent_io_cmd_obj, addr_v6) == NETAGENT_IO_RET_SUCCESS) {
                                        unsigned char *ptr = (unsigned char *)&addr_v6;
                                        Dbg("### get ifup event with v6 address %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X, if = %d, reason = %s",
                                          *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7), *(ptr+8), *(ptr+9), *(ptr+10), *(ptr+11), *(ptr+12), *(ptr+13), *(ptr+14), *(ptr+15), if_id, reason);
                                    } else {
                                        Err("get v6 address fail");
                                    }
                                    send_if_up(netagent_io_obj, if_id, type);
                                    break;
                                default:
                                    Err("can't handle type=%d", type);
                                    break;
                            }

                        } else {
                            Err("get type fail");
                        }
                        break;
                    }
                    case NETAGENT_IO_CMD_IFDOWN : {
                        unsigned int if_id = 0;
                        char *reason = NULL;
                        netagent_io_get_if_id(netagent_io_cmd_obj, &if_id);
                        netagent_io_get_ip_change_reason(netagent_io_cmd_obj, &reason);
                        Dbg("### get ifdown, if = %d, reason = %s", if_id, reason);
                        send_if_down(netagent_io_obj, if_id);
                        break;
                    }
                    case NETAGENT_IO_CMD_IFCHG :{
                        if (netagent_io_get_addr_type(netagent_io_cmd_obj, &type) == NETAGENT_IO_RET_SUCCESS) {
                            unsigned int if_id = 0;
                            char *reason = NULL;
                            netagent_io_get_if_id(netagent_io_cmd_obj, &if_id);
                            netagent_io_get_ip_change_reason(netagent_io_cmd_obj, &reason);

                            if (type & NETAGENT_IO_ADDR_TYPE_IPv4) {
                                if (netagent_io_get_addr_v4(netagent_io_cmd_obj, &addr_v4) == NETAGENT_IO_RET_SUCCESS) {
                                    unsigned char *ptr = (unsigned char *)&addr_v4;
                                    Dbg("### get ifchg event with v4 address = %d.%d.%d.%d, if = %d, reason = %s", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), if_id, reason);
                                } else {
                                    Err("get v4 address fail");
                                }
                                {
                                    unsigned char addr_v6[4] = {0xAA, 0xBB, 0xCC, 0xDD};
                                    send_if_chg(netagent_io_obj, if_id, NETAGENT_IO_ADDR_TYPE_IPv4, addr_v6);
                                }
                            }

                            if (type & NETAGENT_IO_ADDR_TYPE_IPv6) {
                                if (netagent_io_get_addr_v6(netagent_io_cmd_obj, addr_v6) == NETAGENT_IO_RET_SUCCESS) {
                                    unsigned char *ptr = (unsigned char *)&addr_v6;
                                    Dbg("### get ifchg event with v6 address %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X, if = %d, reason = %s",
                                      *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5), *(ptr+6), *(ptr+7), *(ptr+8), *(ptr+9), *(ptr+10), *(ptr+11), *(ptr+12), *(ptr+13), *(ptr+14), *(ptr+15), if_id, reason);
                                } else {
                                    Err("get v6 address fail");
                                }
                                {
                                    unsigned char addr_v6[16] = {0xFE, 0x80, 0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD};
                                    send_if_chg(netagent_io_obj, if_id, NETAGENT_IO_ADDR_TYPE_IPv6, addr_v6);
                                }
                            }

                        } else {
                            Err("get type fail");
                        }
                        break;
                    }
                    default:
                        Err("can't handle cmd=%d", cmd);
                        break;
                }
            } else {
                Err("get cmd fail");
            }

            netagent_io_cmd_free(netagent_io_cmd_obj);

        } else {
            Err("recv fail");
        }
    }
    return NULL;
}
#endif

int netagent_io_test(void *o, char *cmd);

int main(int argc, char *argv[]) {

    void    *netagent_io_obj = 0;
    pthread_t           tid;
    pthread_attr_t      attr;

    netagent_io_obj = netagent_io_init();
    if (!netagent_io_obj) {
        Err("init fail");
        return 0;
    }

    // create recv thread
    {
        pthread_attr_init (&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&tid, &attr, task_stub, netagent_io_obj);
    }


    netagent_io_test(netagent_io_obj, "AT+EIF=1,\"TEST\",\"reset\",1");
    netagent_io_test(netagent_io_obj, "AT+EIF=1,\"TEST\",\"up\",1");
    netagent_io_test(netagent_io_obj, "AT+EIF=2,\"TEST\",\"up\",1");

    sleep(3);
    netagent_io_test(netagent_io_obj, "AT+EIF=1,\"TEST\",\"ipchg\",1");
    sleep(3);
    netagent_io_test(netagent_io_obj, "AT+EIF=1,\"TEST\",\"down\",1");

#if 0
    // send ra test
    {
        void *netagent_io_cmd_obj = 0;
        unsigned int interface_id = 1;
        netagent_io_ra_e flag = NETAGENT_IO_NO_RA_INITIAL;

        netagent_io_cmd_obj = netagent_io_cmd_ra_alloc(interface_id, flag);

        if (netagent_io_send(netagent_io_obj, netagent_io_cmd_obj) != NETAGENT_IO_RET_SUCCESS) {
            Err("send cmd fail");
        }

        netagent_io_cmd_free(netagent_io_cmd_obj);
    }
#endif
    printf("******************** done ********************\r\n");
    while (1) {
        sleep(100);
    }

    if (netagent_io_deinit(netagent_io_obj) != NETAGENT_IO_RET_SUCCESS) {
        Err("deinit fail");
    }

    return 0;

}

