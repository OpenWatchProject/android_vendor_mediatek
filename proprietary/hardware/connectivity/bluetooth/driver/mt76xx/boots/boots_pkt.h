//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#ifndef __BOOTS_PKT_H__
#define __BOOTS_PKT_H__

#include "boots.h"

/** Bluetooth Packet Type */
#define HCI_CMD_PKT     0x01
#define HCI_ACL_PKT     0x02
#define HCI_SCO_PKT     0x03
#define HCI_EVENT_PKT   0x04

/** Script Action Type */
#define SCRIPT_NONE     0x00
#define SCRIPT_TITLE    0x01
#define SCRIPT_PROC     0x02
#define SCRIPT_TX       0x03
#define SCRIPT_RX       0x04
#define SCRIPT_WAITRX   0x05
#define SCRIPT_LOOP     0x06
#define SCRIPT_LOOPEND  0x07
#define SCRIPT_TIMEOUT  0x08
#define SCRIPT_WAIT     0x09
#define SCRIPT_USBALT   0x0A
#define SCRIPT_CMD      0x0B
#define SCRIPT_END      0x0C    // script end

/** Script setting */
typedef struct script_set {
    FILE *script;           /** script FD */
    int loop;               /** loop counter */
    long loop_pos;          /** record loop header */
    int timo;               /** timeout */
    int wait;               /** sleep */
} script_set_s;

/** Store packet list */
typedef struct pkt_list {
    uint8_t     s_type;     /** script file type, for SCRIPT_TX/RX/WAITRX please fill packet type */
    uint8_t     p_type;     /** packet type */
    union {
        uint8_t *data;      /** for command, ACL, SCO & event content */
        char    *msg;       /** for title & proc would show message */
        int     loop;       /** for loop tims */
        int     timo;       /** for timeout */
        int     wait;       /** for wait */
        int     usbalt;     /** for USB alternate */
    } u_cnt;
    size_t      len;
    struct pkt_list    *next;
} pkt_list_s;

/** Modular commands */
typedef struct {
    char        *cmd;                                   // Shows command
    pkt_list_s  *(*func)(char *cmd[], size_t len);      // Implementeded functions
    char        *comment;                               // Shows comment
    char        *details;                               // Command details
} boots_cmds_s;

//---------------------------------------------------------------------------
pkt_list_s *boots_pkt_node_push(uint8_t s_type, uint8_t p_type, void *content,
        size_t len, pkt_list_s *front, pkt_list_s *head);
pkt_list_s *boots_pkt_node_pop(pkt_list_s *list, uint8_t *buf, size_t *len);
void boots_pkt_list_destroy(pkt_list_s *list);
size_t boots_pkt_list_amount(pkt_list_s *list);

size_t boots_pkt_handler(uint8_t *buf, size_t len, pkt_list_s **pList);
pkt_list_s *boots_raw_cmd_handler(char *raw[], size_t len);
pkt_list_s *boots_cmd_set_handler(char *raw[], size_t len);
pkt_list_s *boots_script_handler(char script_file[]);

pkt_list_s *boots_script_parse(char script_file[]);

FILE *boots_script_open(char *file);
void boots_script_close(FILE *fd);
void boots_script_loop(FILE * fd, long size);
pkt_list_s *boots_script_get(FILE *fd);

ssize_t boots_file_wr(uint8_t *buf, ssize_t *len);

//---------------------------------------------------------------------------
#endif // __BOOTS_PKT_H__
