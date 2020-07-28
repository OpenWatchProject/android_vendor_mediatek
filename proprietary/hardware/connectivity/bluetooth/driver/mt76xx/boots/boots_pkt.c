//- vim: set ts=4 sts=4 sw=4 et: --------------------------------------------
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "boots_pkt.h"

//---------------------------------------------------------------------------
#define LOG_TAG "boots_pkt"

#define HCE_INQUIRY_COMPLETE            0x01
#define HCE_INQUIRY_RESULT              0x02
#define HCE_COMMAND_COMPLETE            0x0E
#define HCE_INQUIRY_RESULT_WITH_RSSI    0x22
#define HCE_EXT_INQUIRY_RESULT          0x2F

//---------------------------------------------------------------------------
typedef struct {
    uint8_t pt;     // Packet Type
    uint16_t len;   // Data Length
} packet_info_s;

static const packet_info_s tx_pi[] = {  // TX packet info
    {0x00, 0x0000}, {0x01, 0x0000}, {0x02, 0x0000}, {0x03, 0x0011},
    {0x04, 0x001B}, {0x05, 0x000A}, {0x06, 0x0014}, {0x07, 0x001E},
    {0x08, 0x0013}, {0x09, 0x000A}, {0x0A, 0x0079}, {0x0B, 0x00B7},
    {0x0E, 0x00E0}, {0x0F, 0x0153}, {0x17, 0x001E}, {0x1C, 0x0078},
    {0x1D, 0x00B4}, {0x24, 0x0036}, {0x28, 0x0053}, {0x2A, 0x016F},
    {0x2B, 0x0053}, {0x2E, 0x02A7}, {0x2F, 0x03FD}, {0x36, 0x003C},
    {0x37, 0x005A}, {0x3C, 0x0168}, {0x3D, 0x021C}};

static const packet_info_s rx_pi[] = {  // RX packet info
    {0x03, 0x0011}, {0x04, 0x001B}, {0x0A, 0x0079}, {0x0B, 0x00B7},
    {0x0E, 0x00E0}, {0x0F, 0x0153}, {0x24, 0x0036}, {0x28, 0x0053},
    {0x2A, 0x016F}, {0x2B, 0x0053}, {0x2E, 0x02A7}, {0x2F, 0x03FD}};

typedef enum {
    WR_FLAG_NO,
    WR_FLAG_READ,
    WR_FLAG_WRITE
} wr_flag_e;

typedef struct {
    uint32_t addr;
    uint32_t value;
    wr_flag_e wr_flag;
} efuse_reg_wr_s;
static efuse_reg_wr_s efuse_reg_wr = {0, 0, WR_FLAG_NO};

//---------------------------------------------------------------------------
pkt_list_s *boots_pkt_node_push(uint8_t s_type, uint8_t p_type, void *content,
        size_t len, pkt_list_s *front, pkt_list_s *head)
{
    // Create node
    pkt_list_s *node = (pkt_list_s *)malloc(sizeof(pkt_list_s));

    if (!node) {
        BPRINT_D("%s: List allocate fail", __func__);
        return NULL;
    }

    node->s_type = s_type;
    switch (s_type) {
    case SCRIPT_NONE:
    case SCRIPT_TX:
    case SCRIPT_RX:
    case SCRIPT_WAITRX:
    case SCRIPT_CMD:
        node->p_type = p_type;
        if (len > 0)
            node->u_cnt.data = (uint8_t *)malloc(len);
        if (content) {
            memcpy(node->u_cnt.data, (uint8_t *)content, len);
            node->len = len;
        }
        break;
    case SCRIPT_TITLE:
    case SCRIPT_PROC:
        if (len > 0)
            node->u_cnt.msg = (char *)malloc(len);
        if (content) {
            memcpy(node->u_cnt.msg, (char *)content, len);
            node->len = len;
        }
        break;
    case SCRIPT_LOOP:
        if (content)
            node->u_cnt.loop = *((int *)content);
        break;
    case SCRIPT_TIMEOUT:
        if (content)
            node->u_cnt.timo = *((int *)content);
        break;
    case SCRIPT_WAIT:
        if (content)
            node->u_cnt.wait = *((int *)content);
        break;
    case SCRIPT_USBALT:
        if (content)
            node->u_cnt.usbalt = *((int *)content);
        break;
    case SCRIPT_LOOPEND:
    case SCRIPT_END:
        break;
    default:
        BPRINT_W("Incorrect Script Type: %d", s_type);
        break;
    }
    if (front)
        front->next = node;
    node->next = head ? head : node;

    // return node point
    return node;
}

//---------------------------------------------------------------------------
pkt_list_s *boots_pkt_node_pop(pkt_list_s *list, uint8_t *buf, size_t *len)
{
    pkt_list_s *tmp = NULL;

    if (!list) return NULL;
    //BPRINT_D("%s node: %p, next: %p", __func__, list, list->next);

    if (buf && len) {
        memcpy(buf, list->u_cnt.data, list->len);
        *len = list->len;
    }

    // Remove this node from head
    if (list->next == list) {
        tmp = NULL;
    } else {
        pkt_list_s *tail = NULL;

        tmp = list->next;
        // round circle
        tail = tmp;
        while (tail->next != list) {
            tail = tail->next;
        }
        tail->next = tmp;
    }

    switch (list->s_type) {
    case SCRIPT_NONE:
    case SCRIPT_TX:
    case SCRIPT_RX:
    case SCRIPT_WAITRX:
    case SCRIPT_CMD:
        free(list->u_cnt.data);
        break;
    case SCRIPT_TITLE:
    case SCRIPT_PROC:
        free(list->u_cnt.msg);
        break;
    default:
        break;
    }
    free(list);

    return tmp;
}

//---------------------------------------------------------------------------
void boots_pkt_list_destroy(pkt_list_s *list)
{
    pkt_list_s *cur = list;
    pkt_list_s *next = NULL;

    if (!list) return;

    do {
        if (next) cur = next;
        next = cur->next;
        //BPRINT_D("%s cur: %p, next: %p", __func__, cur, next);
        switch (list->s_type) {
        case SCRIPT_NONE:
        case SCRIPT_TX:
        case SCRIPT_RX:
        case SCRIPT_WAITRX:
        case SCRIPT_CMD:
            free(cur->u_cnt.data);
            break;
        case SCRIPT_TITLE:
        case SCRIPT_PROC:
            free(cur->u_cnt.msg);
            break;
        default:
            break;
        }
        free(cur);
    } while (next != list);
}

//---------------------------------------------------------------------------
size_t boots_pkt_list_amount(pkt_list_s *list)
{
    size_t amount = 1;
    pkt_list_s *tmp = list;

    if (!list) return 0;

    while (tmp->next != list) {
        amount++;
        tmp = tmp->next;
    }
    return amount;
}

//---------------------------------------------------------------------------
static pkt_list_s *boots_efuse_write(pkt_list_s *list, uint8_t *buf)
{
    pkt_list_s * pkt = NULL;
    uint8_t efuse[] = {0x01, 0x6F, 0xFC, 0x18,
                               0x01, 0x0D, 0x14, 0x00, 0x01, 0x01,
                               0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00};
    size_t len = 0;
    uint16_t addr = 0;

    boots_pkt_node_pop(list, efuse, &len);
    if (len != sizeof(efuse)) {
        BPRINT_E("Error! %d != %d", (int)len, (int)sizeof(efuse));
    }

    addr = efuse_reg_wr.addr / 16 * 4;
    efuse[10] = addr & 0xFF;
    efuse[11] = (addr & 0xFF00) >> 8;
    memcpy(&efuse[12], &buf[2], 4);
    memcpy(&efuse[16], &buf[8], 4);
    memcpy(&efuse[20], &buf[14], 4);
    memcpy(&efuse[24], &buf[20], 4);

    memcpy(&efuse[12 + efuse_reg_wr.addr % 16], &efuse_reg_wr.value, 1);

    pkt = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, efuse, sizeof(efuse),
            NULL, NULL);

    return pkt;
}

//---------------------------------------------------------------------------
ssize_t boots_file_wr(uint8_t *buf, ssize_t *len)
{
    int fd = 0;
    uint32_t addr = 0;
    uint8_t value = 0;
    uint8_t wr = 0;
    size_t ret = 0; // For compiling error

    /****************************************************
    format:
    CMD: 01 01 FF LEN WR ADDR VALUE FILE_PATH
    EVENT: 04 0E LEN 01 01 FF ERRNO WR ADDR VALUE

    LEN: 1 byte
    WR: 1 byte
    ADDR: 4 bytes
    VALUE: 1 byte
    ERRNO: 2 bytes
    FILE_PATH:
    ****************************************************/

    errno = 0;

    fd = open((char *)&buf[10], O_RDWR);
    if (fd < 0) {
        printf("open file %s failed, errno = %s(%d)\n", &buf[10], strerror(errno), errno);
    } else {
        memcpy(&addr, &buf[5], sizeof(addr));
        wr = buf[4];

        lseek(fd, addr, SEEK_SET);

        if (wr == WR_FLAG_WRITE) {
            value = buf[9];
            ret = write(fd, &value, 1);
        } else {
            ret = read(fd, &value, 1);
        }

        close(fd);
    }

    buf[0] = 0x04;
    buf[1] = 0x0E;
    buf[2] = 0x0B;
    buf[3] = 0x01;
    buf[4] = 0x01;
    buf[5] = 0xFF;
    buf[6] = errno & 0xFF;
    buf[7] = (errno & 0xFF00) >> 8;
    buf[8] = wr;
    memcpy(&buf[9], &addr, sizeof(addr));
    buf[13] = value;
    *len = 14;
    return ret;
}

//--------------------------------------------------------------------------
void boots_pkt_parsing(uint8_t *buf, size_t len)
{
    if (!buf || !len) return;

    if (buf[0] == HCI_EVENT_PKT) {
        if (buf[1] == HCE_COMMAND_COMPLETE && len >= 7) {
            switch (*(uint16_t *)(buf + 4)) {
            case 0x0C03:    // RESET
                BPRINT_I_RAW("(HCI_RESET)");
                break;
            case 0x1009:    // Read BD address
                if (len >= 13)
                    BPRINT_I_RAW("(Read BDADD: 0x%02X-%02X-%02X-%02X-%02X-%02X)",
                            buf[12], buf[11], buf[10], buf[9], buf[8], buf[7]);
                break;
            case 0xFC0D:    // RF test
                if (buf[2] == 20) {   // length is 20 for RF RX results
                #if 0   /* Bus error (core dumped) */
                    uint32_t rxpktcount = *(uint32_t *)&buf[7];
                    float rxerrrate = (float)*(uint32_t *)&buf[11] / 1000000;
                    uint32_t rxbytecount = *(uint32_t *)&buf[15];
                    float biterrrate = (float)*(uint32_t *)&buf[19] / 1000000;
                    BPRINT_I_RAW("\n");
                    BPRINT_I_RAW("\t       RX Packet Count: %d\n", rxpktcount);
                    BPRINT_I_RAW("\t         RX Error Rate: %f%%\n", rxerrrate);
                    BPRINT_I_RAW("\t         RX Byte Count: %d\n", rxbytecount);
                    BPRINT_I_RAW("\t        Bit Error Rate: %f%%", biterrrate);
                #else
                    BPRINT_I_RAW("\n");
                    BPRINT_I_RAW("\t       RX Packet Count: %d\n", (uint32_t)*(uint32_t *)(buf + 7));
                    BPRINT_I_RAW("\t         RX Error Rate: %f%%\n", (float)(*(uint32_t *)(buf + 11) / 1000000));
                    BPRINT_I_RAW("\t         RX Byte Count: %d\n", (uint32_t)*(uint32_t *)(buf + 15));
                    BPRINT_I_RAW("\t        Bit Error Rate: %f%%", (float)(*(uint32_t *)(buf + 19) / 1000000));
                #endif
                }
                break;
            case 0x201F:    // BLE RX test
                BPRINT_I_RAW("\n");
                if (buf[2] >= 6 && buf[6] == 0) {
                    uint16_t pktcount = *(uint16_t *)&buf[7];
                    BPRINT_I_RAW("\t       RX Packet Count: %d\n", pktcount);
                } else {
                    BPRINT_W("status or length is incorrect");
                }
            case 0xFCD1:    // register read
                BPRINT_I_RAW("(register addr: 0x%08X, value: 0x%02X%02X%02X%02X)", efuse_reg_wr.addr, buf[10], buf[9], buf[8], buf[7]);
                break;
            case 0xFF01:    // file read/write
                if (buf[6] == 0 && buf[7] == 0) {
                    if (buf[8] == WR_FLAG_READ)
                        BPRINT_I_RAW("(offset: 0x%02X%02X%02X%02X, value: 0x%02X)", buf[12], buf[11], buf[10], buf[9], buf[13]);
                } else {
                    uint16_t err = buf[6] | (buf[7] << 8);
                    BPRINT_I_RAW("(Error(%d): %s)", err, strerror(err));
                }
                break;
            }
        } else if (buf[1] == HCE_INQUIRY_RESULT || buf[1] == HCE_INQUIRY_RESULT_WITH_RSSI
                || buf[1] == HCE_EXT_INQUIRY_RESULT) {
            BPRINT_I_RAW("(BDADDR:0x%02X-%02X-%02X-%02X-%02X-%02X",
                    buf[9], buf[8], buf[7], buf[6], buf[5], buf[4]);
            if (buf[1] == HCE_INQUIRY_RESULT_WITH_RSSI || buf[1] == HCE_EXT_INQUIRY_RESULT) {
                int8_t rssi = ~buf[17] + 1;
                BPRINT_I_RAW(", RSSI:%s%d", buf[17] > 0x7F ? "-" : "",
                        buf[17] > 0x7F ? rssi : buf[17]);
            }
            if (buf[1] == HCE_EXT_INQUIRY_RESULT) {
                size_t i = 18;
                while (buf[i]) {
                    if (buf[i + 1] == 8 || buf[i + 1] == 9) {
                        char name[256] = {0};
                        memcpy(name, &buf[i + 2], buf[i] - 1);
                        name[buf[i] - 1] = '\0';
                        BPRINT_I_RAW(", Name:%s", name);
                    }
                    i += (buf[i] + 1);
                }
            }
            BPRINT_I_RAW(")");
        } else if (buf[1] == HCE_INQUIRY_COMPLETE) {
            BPRINT_I_RAW("(Inquiry_Complete)");
        } else if (buf[1] == 0xE4 && efuse_reg_wr.wr_flag == WR_FLAG_READ) {
            if ((efuse_reg_wr.addr / 16 * 4) != (buf[9] | (buf[10] << 8))) {
                BPRINT_I_RAW("(Read efuse error!)");
            } else {
                int8_t offset = 9 + ((efuse_reg_wr.addr / 4) % 4) * 6 + 2 + (efuse_reg_wr.addr % 4);

                BPRINT_I_RAW("(efuse addr: 0x%04X, value: 0x%02X)", efuse_reg_wr.addr, buf[offset]);
            }
        }
    }
}

size_t boots_pkt_handler_parse(uint8_t *buf, size_t len, pkt_list_s **pList)
{
    size_t i = 0;
    size_t ret = 0;
    pkt_list_s *list = NULL;

    if (pList)
        list = *pList;

    switch (buf[0]) {
    case HCI_CMD_PKT:
        BPRINT_I_RAW("\t  CMD:");
        break;
    case HCI_ACL_PKT:
        BPRINT_I_RAW("\t  ACL:");
        break;
    case HCI_SCO_PKT:
        BPRINT_I_RAW("\t  SCO:");
        break;
    case HCI_EVENT_PKT:
        BPRINT_I_RAW("\tEvent:");
        break;
    default:
        BPRINT_W("Receive unknonw type %02X", buf[0]);
        BPRINT_I_RAW(" Unknown %02X:", buf[0]);
    }

#define SHOW_LEN 17

    // exclude extended
    for (i = 1; i < len && i <= SHOW_LEN; i++) {
        BPRINT_I_RAW(" %02X", buf[i]);
    }
    BPRINT_I_RAW("%s ", len > SHOW_LEN ? "..." : "");
    boots_pkt_parsing(buf, len);    // Parsing
    BPRINT_I_RAW("\n");

    if (len > 2 && buf[0] == HCI_EVENT_PKT) {
        if (buf[1] == HCE_COMMAND_COMPLETE || buf[1] == HCE_INQUIRY_COMPLETE) {
            // means received Command_Complete or Inquiry_Complete
            ret = 0;
        } else if (buf[1] == 0xE4 && buf[2] == 0x1E
                && efuse_reg_wr.wr_flag == WR_FLAG_WRITE) {
            *pList = boots_efuse_write(list, &buf[9]);
            if (*pList) ret = 1;
        } else {
            ret = 1;
        }
    }
    return ret;
}

//--------------------------------------------------------------------------
size_t boots_pkt_handler(uint8_t *buf, size_t len, pkt_list_s **pList)
{
    UNUSED(boots_btif);
    size_t i = 0;
    size_t ret = 0;
    pkt_list_s *list = NULL;
    size_t pkg_len = 0;
    uint8_t *buf_parse = 0;

    if (!buf || !len) return 0;

    if (pList)
        list = *pList;

#define PKG_HDR 3 /* 1 byte event type + 1 byte length */

    buf_parse = &buf[0];

    if (HCI_EVENT_PKT == buf[0]) {
        /* First event */
        pkg_len = buf_parse[2];

        while (len >= pkg_len) {
            if (boots_pkt_handler_parse(buf_parse, pkg_len + PKG_HDR, pList))
                ret = 1;
            len -= (pkg_len + PKG_HDR);
            if (len > PKG_HDR) {
                buf_parse = &buf[pkg_len + PKG_HDR];
                pkg_len = buf_parse[2];
            } else
                break;
        }
    } else
        boots_pkt_handler_parse(buf_parse, len, pList);

    /** For next action */
    if (list) {
        if (list->p_type == HCI_EVENT_PKT && buf[0] == HCI_EVENT_PKT) {
            if (memcmp(buf, list->u_cnt.data, list->len)) {
                BPRINT_I("Received is not match with script");
                BPRINT_I_RAW("\tScript:");
                for (i = 0; i < list->len; i++)
                    BPRINT_I_RAW("%02X ", list->u_cnt.data[i]);
                BPRINT_I_RAW("\n");

                if (list->s_type == SCRIPT_WAITRX) {
                    // waiting specific RX
                    BPRINT_I("Retry to get specific event");
                    return 1;
                }
                boots_pkt_list_destroy(list);
                *pList = NULL;
                return 0;

            }
            *pList = boots_pkt_node_pop(list, NULL, NULL);
        }
        if (*pList) ret = 1;
    }
    return ret;
}

//--------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_reset(char *cmd[], size_t len)
{
    uint8_t reset[] = {0x01, 0x03, 0x0C, 0x00};
    pkt_list_s *node = NULL;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h')
        return NULL;

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, reset, sizeof(reset),
            NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_inquiry(char *cmd[], size_t len)
{
    uint8_t inq_mode[] = {0x01, 0x45, 0x0C, 0x01, 0x02};
    uint8_t inq[] = {0x01, 0x01, 0x04, 0x05, 0x33, 0x8B, 0x9E, 2, 0x00};
    pkt_list_s *list = NULL;

    BPRINT_D("%s", __func__);
    if (len > 2 && *(cmd[1] + 1) == 't') {
        inq[7] = strtol(cmd[2], NULL, 16);
    } else if (len > 1) {
        return NULL;
    }
    list = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, inq_mode, sizeof(inq_mode),
            NULL, NULL);
    boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, inq, sizeof(inq), list, list);
    return list;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_enter_dut(char *cmd[], size_t len)
{
    uint8_t reset[] = {0x01, 0x03, 0x0C, 0x00};
    uint8_t scan_enable[] = {0x01, 0x1A, 0x0C, 0x01, 0x03};
    uint8_t event_filter[] = {0x01, 0x05, 0x0C, 0x03, 0x02, 0x00, 0x02};
    uint8_t dut[] = {0x01, 0x03, 0x18, 0x00};
    pkt_list_s *list = NULL;
    pkt_list_s *tmp = NULL;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h')
        return NULL;

    list = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, reset, sizeof(reset),
            NULL, NULL);
    tmp = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, scan_enable,
            sizeof(scan_enable), list, list);
    tmp = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, event_filter,
            sizeof(event_filter), tmp, list);
    boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, dut, sizeof(dut), tmp, list);

    return list;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_read_bd_addr(char *cmd[], size_t len)
{
    uint8_t raddr[] = {0x01, 0x09, 0x10, 0x00};
    pkt_list_s *node = NULL;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h')
        return NULL;

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, raddr, sizeof(raddr),
            NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_write_bd_addr(char *cmd[], size_t len)
{
    uint8_t waddr[] = {0x01, 0x1A, 0xFC, 0x06, 0, 0, 0, 0, 0, 0};
    pkt_list_s *node = NULL;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h')
        return NULL;

    if (len >= 2 && strlen(cmd[1]) == 17) {
        waddr[9] = strtol(cmd[1], NULL, 16);
        waddr[8] = strtol(cmd[1] + 3, NULL, 16);
        waddr[7] = strtol(cmd[1] + 6, NULL, 16);
        waddr[6] = strtol(cmd[1] + 9, NULL, 16);
        waddr[5] = strtol(cmd[1] + 12, NULL, 16);
        waddr[4] = strtol(cmd[1] + 15, NULL, 16);
    /** Don't suppport separated address
    } else if (len == 7) {
        int i = 0;
        for (i = 0; i < 6; i++)
            waddr[9 - i] = strtol(cmd[i + 1], NULL, 16);
    */
    } else {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }
    if (!waddr[4] && !waddr[5] && !waddr[6] && !waddr[7] && !waddr[8] && !waddr[9]) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }
    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, waddr, sizeof(waddr),
            NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_set_power_level(char *cmd[], size_t len)
{
    uint8_t power_level[] = {0x01, 0x17, 0xFC, 0x04, 0x00, 0x00, 0, 0x00};
    pkt_list_s *node = NULL;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (len < 2) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    power_level[6] = strtol(cmd[1], NULL, 16);
    if (power_level[6] > 7 || (power_level[6] == 0 && *cmd[1] != '0')) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }
    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, power_level,
            sizeof(power_level), NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_set_whiten(char *cmd[], size_t len)
{
    uint8_t whiten[] = {0x01, 0x15, 0xFC, 0x01, 0};
    pkt_list_s *node = NULL;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (len < 2) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    whiten[4] = strtol(cmd[1], NULL, 16);
    if (whiten[4] > 1 || (whiten[4] == 0 && *cmd[1] != '0')) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }
    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, whiten, sizeof(whiten),
            NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_rf_tx(char *cmd[], size_t len)
{
    uint8_t rf_test[] = {0x01, 0x0D, 0xFC, 0x17, 0x00, 0x00,
        0,      // pattern
        1,      // hopping:1, fixed channel:0
        0,      // channel if fixed
        0,      // channel if fixed
        0x00, 0x02,
        0,      // packet type
        0, 0,   // packet length
        0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    pkt_list_s *node = NULL;
    size_t i = 1, j = 0;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (!(len % 2)) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'p':       // Pattern
            rf_test[6] = strtol(cmd[i + 1], NULL, 16);
            if (rf_test[6] == 0x0A) {
                rf_test[7] = 0; // fix channel
            } else if (rf_test[6] != 1 && rf_test[6] != 2 && rf_test[6] != 3
                    && rf_test[6] != 4 && rf_test[6] != 9 && rf_test[6] != 0xa) {
                BPRINT_E("%s: Incorrect format: -p %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 'c':       // channel
            rf_test[7] = 0;
            rf_test[8] = strtol(cmd[i + 1], NULL, 10);
            rf_test[9] = rf_test[8];
            if (rf_test[8] > 78) {
                BPRINT_E("%s: Incorrect format: -c %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 't':       // packet type
            rf_test[12] = strtol(cmd[i + 1], NULL, 16);
            while (j < ARRAY_SIZE(tx_pi)) {
                if (tx_pi[j].pt == rf_test[12])
                    break;
                else
                    j++;
            }
            if (j >= ARRAY_SIZE(tx_pi)) {
                BPRINT_E("%s: Incorrect format: -t %s", __func__, cmd[i + 1]);
                return NULL;
            }
            if (*(uint16_t *)&rf_test[13] == 0)
                *(uint16_t *)&rf_test[13] = tx_pi[j].len;
            i += 2;
            BPRINT_D("%02X %02X", rf_test[13], rf_test[14]);
            break;
        case 'l':
            *(uint16_t *)&rf_test[13] = strtol(cmd[i + 1], NULL, 10);
            i += 2;
            BPRINT_D("%02X %02X", rf_test[13], rf_test[14]);
            break;
        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
    }

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, rf_test,
            sizeof(rf_test), NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_rf_rx(char *cmd[], size_t len)
{
    uint8_t rf_test[] = {0x01, 0x0D, 0xFC, 0x17, 0x00,
        0,                      // pattern
        0x0B,                   // rx start:0x0B, rx stop:0xFF
        0x00, 0x00,
        0,                      // channel
        0x00, 0x01,
        3,                      // packet type
        0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0xA5, 0xF0, 0xC3, // address
        0x00, 0x00};
    pkt_list_s *node = NULL;
    size_t i = 1, j = 0;

    BPRINT_D("%s: %s", __func__, cmd[0]);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (!(len % 2)) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    if (!memcmp("rfr_stop", cmd[0], strlen("rfr_stop"))) {
        rf_test[6] = 0xFF;
    }

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'p':       // Pattern
            rf_test[5] = strtol(cmd[i + 1], NULL, 16);
            if (rf_test[5] != 1 && rf_test[5] != 2 && rf_test[5] != 3
                    && rf_test[5] != 4 && rf_test[5] != 9) {
                BPRINT_E("%s: Incorrect format: -p %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 'c':       // channel
            rf_test[9] = strtol(cmd[i + 1], NULL, 10);
            if (rf_test[9] > 78) {
                BPRINT_E("%s: Incorrect format: -c %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 't':       // packet type
            rf_test[12] = strtol(cmd[i + 1], NULL, 16);
            while (j < ARRAY_SIZE(rx_pi)) {
                if (rx_pi[j].pt == rf_test[12])
                    break;
                else
                    j++;
            }
            if (j >= ARRAY_SIZE(rx_pi)) {
                BPRINT_E("%s: Incorrect format: -t %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 'a':
            if (strlen(cmd[i + 1]) == 11) {
                rf_test[21] = strtol(cmd[i + 1], NULL, 16);
                rf_test[22] = strtol(cmd[i + 1] + 3, NULL, 16);
                rf_test[23] = strtol(cmd[i + 1] + 6, NULL, 16);
                rf_test[24] = strtol(cmd[i + 1] + 9, NULL, 16);
            } else {
                BPRINT_E("%s: Incorrect format", __func__);
                return NULL;
            }
            if (!rf_test[21] && !rf_test[22] && !rf_test[23] && !rf_test[24]) {
                BPRINT_E("%s: Incorrect format", __func__);
                return NULL;
            }
            i += 2;
            break;
        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
    }

    if (!rf_test[21] && !rf_test[22] && !rf_test[23] && !rf_test[24]) {
        BPRINT_E("Please input Tester Address");
        return NULL;
    }
    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, rf_test,
            sizeof(rf_test), NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_ble_tx(char *cmd[], size_t len)
{
    uint8_t reset[] = {0x01, 0x03, 0x0C, 0x00};
    uint8_t ble_tx[] = {0x01, 0x1E, 0x20, 0x03, 0, 0x25, 0};
    pkt_list_s *list = NULL;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (!(len % 2)) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'c':   // Channel
            ble_tx[4] = strtol(cmd[i + 1], NULL, 10);
            if (ble_tx[4] > 39) {
                BPRINT_E("%s: Incorrect format: -c %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 'p':   // Pattern
            ble_tx[6] = strtol(cmd[i + 1], NULL, 16);
            if (ble_tx[6] > 2) {
                BPRINT_E("%s: Incorrect format: -p %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
    }

    list = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, reset, sizeof(reset),
            NULL, NULL);
    boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, ble_tx, sizeof(ble_tx), list, list);

    return list;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_ble_rx(char *cmd[], size_t len)
{
    uint8_t reset[] = {0x01, 0x03, 0x0C, 0x00};
    uint8_t ble_rx[] = {0x01, 0x1D, 0x20, 0x01, 0 /* channel */};
    pkt_list_s *list = NULL;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (!(len % 2)) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'c':   // Channel
            ble_rx[4] = strtol(cmd[i + 1], NULL, 10);
            if (ble_rx[4] > 39) {
                BPRINT_E("%s: Incorrect format: -c %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
    }

    list = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, reset, sizeof(reset),
            NULL, NULL);
    boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, ble_rx, sizeof(ble_rx), list, list);

    return list;
}

//--------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_ble_test_stop(char *cmd[], size_t len)
{
    uint8_t ble_stop[] = {0x01, 0x1F, 0x20, 0x00};
    pkt_list_s *node = NULL;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h')
        return NULL;

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, ble_stop, sizeof(ble_stop), NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_ble_etx(char *cmd[], size_t len)
{
    uint8_t reset[] = {0x01, 0x03, 0x0C, 0x00};
    uint8_t ble_etx[] = {0x01, 0x34, 0x20, 0x04, 0, 0, 0, 1};
    pkt_list_s *list = NULL;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (!(len % 2)) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'c':   // Channel
            ble_etx[4] = strtol(cmd[i + 1], NULL, 10);
            if (ble_etx[4] > 39) {
                BPRINT_E("%s: Incorrect format: -c %s", __func__, cmd[i + 1]);
                return NULL;
            }
            break;
        case 'l':   // Packet length in bytes
            ble_etx[5] = strtol(cmd[i + 1], NULL, 10);
            break;
        case 'p':   // Packet Payload
            ble_etx[6] = strtol(cmd[i + 1], NULL, 10);
            if (ble_etx[6] > 0x07) { // So far 0x08-0xFF reserved for future use
                BPRINT_E("%s: Incorrect format: -p %s", __func__, cmd[i + 1]);
                return NULL;
            }
            break;
        case 'y':   // PHY
            ble_etx[7] = strtol(cmd[i + 1], NULL, 10);
            if (ble_etx[7] == 0 || ble_etx[7] > 0x05) { // So far 0 and 0x05-0xFF reserved for future use
                BPRINT_E("%s: Incorrect format: -y %s", __func__, cmd[i + 1]);
                return NULL;
            }
            break;
        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
        i += 2;
    }

    list = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, reset, sizeof(reset),
            NULL, NULL);
    boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, ble_etx, sizeof(ble_etx), list, list);

    return list;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_ble_erx(char *cmd[], size_t len)
{
    uint8_t reset[] = {0x01, 0x03, 0x0C, 0x00};
    uint8_t ble_erx[] = {0x01, 0x33, 0x20, 0x03, 0, 1, 0};
    pkt_list_s *list = NULL;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (!(len % 2)) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'c':   // Channel
            ble_erx[4] = strtol(cmd[i + 1], NULL, 10);
            if (ble_erx[4] > 39) {
                BPRINT_E("%s: Incorrect format: -c %s", __func__, cmd[i + 1]);
                return NULL;
            }
            break;
        case 'y':   // PHY
            ble_erx[5] = strtol(cmd[i + 1], NULL, 10);
            /** So far 0 and 0x04-0xFF reserved for future use
                7668/6632 not support 0x03:LE Coded PHY */
            if (ble_erx[5] == 0 || ble_erx[5] > 0x03) {
                BPRINT_E("%s: Incorrect format: -y %s", __func__, cmd[i + 1]);
                return NULL;
            }
            break;
        case 'm': // Modulation Index
            ble_erx[6] = strtol(cmd[i + 1], NULL, 10);
            if (ble_erx[6] > 0x01) {
                BPRINT_E("%s: Incorrect format: -m %s", __func__, cmd[i + 1]);
                return NULL;
            }
            break;
        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
        i += 2;
    }

    list = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, reset, sizeof(reset),
            NULL, NULL);
    boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, ble_erx, sizeof(ble_erx), list, list);

    return list;
}
//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_tx_tone(char *cmd[], size_t len)
{
    uint8_t txtone[] = {0x01, 0xD5, 0xFC, 0x05, 0, 0, 0, 0, 0};
    pkt_list_s *node = NULL;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (!(len % 2)) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 't':   // Tone Type
            txtone[5] = strtol(cmd[i + 1], NULL, 10);
            if (txtone[5] > 4) {
                BPRINT_E("%s: Incorrect format: -t %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 'c':
            txtone[4] = strtol(cmd[i + 1], NULL, 10);
            if (txtone[4] > 78) {
                BPRINT_E("%s: Incorrect format: -c %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 'm':
            txtone[6] = strtol(cmd[i + 1], NULL, 10);
            if (txtone[6] > 1) {
                BPRINT_E("%s: Incorrect format: -m %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 'd':
            txtone[7] = strtol(cmd[i + 1], NULL, 10);
            if (txtone[7] > 2) {
                BPRINT_E("%s: Incorrect format: -d %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        case 'p':
            txtone[8] = strtol(cmd[i + 1], NULL, 10);
            if (txtone[8] > 1) {
                BPRINT_E("%s: Incorrect format: -p %s", __func__, cmd[i + 1]);
                return NULL;
            }
            i += 2;
            break;
        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
    }

    if (txtone[5] <= 3) {
        // Only for Modulation Tone
        txtone[6] = txtone[7] = txtone[8] = 0;
    } else if (txtone[6] == 1) {
        // Only for BT mode
        txtone[7] = 0;
    }

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, txtone, sizeof(txtone),
            NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_radio_setting(char *cmd[], size_t len)
{
    uint8_t set_radio[] = {0x01, 0x79, 0xFC, 0x08,
                           0x07, 0x80, 0x00, 0x06, 0x07, 0x07, 0x00, 0x00};
    pkt_list_s *node = NULL;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (!(len % 2)) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }
    /**
     * radio setting -b <0-7> -e <0-7> -m <0-7> -s <0-3> -o <0-1>
     */

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'b':
            set_radio[4] = strtol(cmd[i + 1], NULL, 10);
            i += 2;
            break;
        case 'e':
            set_radio[8] = strtol(cmd[i + 1], NULL, 10);
            i += 2;
            break;
        case 'm':
            set_radio[9] = strtol(cmd[i + 1], NULL, 10);
            i += 2;
            break;
        case 's':
            set_radio[10] = strtol(cmd[i + 1], NULL, 10);
            i += 2;
            break;
        case 'o':
            set_radio[11] = strtol(cmd[i + 1], NULL, 10);
            i += 2;
            break;
        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
    }

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, set_radio, sizeof(set_radio),
            NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_tx_power_offset(char *cmd[], size_t len)
{
    uint8_t txpoweroffset[] = {0x01, 0x93, 0xFC, 0x06,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    pkt_list_s *node = NULL;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (!(len % 2)) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }
    /*
    * TX power offset -g0 <0-15> -g1 <0-15> -g2 <0-15>
    *                 -g3 <0-15> -g4 <0-15> -g5 <0-15>
    */

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'g':   // Tone Type
            switch (*(cmd[i] + 2)) {
            case '0':
                txpoweroffset[4] = strtol(cmd[i + 1], NULL, 10);
                i += 2;
                break;
            case '1':
                txpoweroffset[5] = strtol(cmd[i + 1], NULL, 10);
                i += 2;
                break;
            case '2':
                txpoweroffset[6] = strtol(cmd[i + 1], NULL, 10);
                i += 2;
                break;
            case '3':
                txpoweroffset[7] = strtol(cmd[i + 1], NULL, 10);
                i += 2;
                break;
            case '4':
                txpoweroffset[8] = strtol(cmd[i + 1], NULL, 10);
                i += 2;
                break;
            case '5':
                txpoweroffset[9] = strtol(cmd[i + 1], NULL, 10);
                i += 2;
                break;
            default:
                BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
                return NULL;
            }
            break;
        default:
            i++;
        }
    }

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, txpoweroffset, sizeof(txpoweroffset),
            NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_efuse_wr(char *cmd[], size_t len)
{
    uint8_t efuse_r[] = {0x01, 0x6F, 0xFC, 0x0E,
                               0x01, 0x0D, 0x0A, 0x00, 0x02, 0x04,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00};
    uint8_t efuse_w[] = {0x01, 0x6F, 0xFC, 0x18,
                               0x01, 0x0D, 0x14, 0x00, 0x01, 0x01,
                               0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00};
    pkt_list_s *node = NULL;
    uint16_t addr = 0;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (len % 2) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }
    /*
    * efuse -w -a <> -v <>
    * efuse -r -a <>
    */

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'w':
            /* read value first
            efuse[3] = 24;
            efuse[6] = 0x14;
            efuse[8] = 0x01;
            efuse[8] = 0x01;
            */
            efuse_r[3] = 14;      /* length */
            efuse_r[6] = 0x0A;
            efuse_r[8] = 0x02;
            efuse_r[9] = 0x04;
            efuse_reg_wr.wr_flag = WR_FLAG_WRITE;
            i += 1;
            break;
        case 'r':
            efuse_r[3] = 14;      /* length */
            efuse_r[6] = 0x0A;
            efuse_r[8] = 0x02;
            efuse_r[9] = 0x04;
            efuse_reg_wr.wr_flag = WR_FLAG_READ;
            i += 1;
            break;
        case 'a':
            efuse_reg_wr.addr = strtol(cmd[i + 1], NULL, 16);
            addr = efuse_reg_wr.addr / 16 * 4;
            efuse_r[10] = addr & 0xFF;
            efuse_r[11] = (addr & 0xFF00) >> 8;
            efuse_r[12] = (addr + 1) & 0xFF;
            efuse_r[13] = ((addr + 1) & 0xFF00) >> 8;
            efuse_r[14] = (addr + 2) & 0xFF;
            efuse_r[15] = ((addr + 2) & 0xFF00) >> 8;
            efuse_r[16] = (addr + 3) & 0xFF;
            efuse_r[17] = ((addr + 3) & 0xFF00) >> 8;
            i += 2;
            break;
        case 'v':
            efuse_reg_wr.value = strtol(cmd[i + 1], NULL, 16);
            i += 2;
            break;

        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
    }

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, efuse_r, (efuse_r[3] + 4),
            NULL, NULL);
    if (efuse_reg_wr.wr_flag == WR_FLAG_WRITE)
    {
        boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, efuse_w, (efuse_w[3] + 4),
            node, node);
    }
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_register_wr(char *cmd[], size_t len)
{
    uint8_t reg[] = {0x01, 0x00, 0xFC, 0x00,
                               0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00};
    pkt_list_s *node = NULL;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (len % 2) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }
    /*
    * register -w -a <> -v <>
    * register -r -a <>
    */

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'w':
            reg[1] = 0xD0;
            reg[3] = 0x08;     /* length */
            efuse_reg_wr.wr_flag = WR_FLAG_WRITE;
            i += 1;
            break;
        case 'r':
            reg[1] = 0xD1;
            reg[3] = 0x04;     /* length */
            efuse_reg_wr.wr_flag = WR_FLAG_READ;
            i += 1;
            break;
        case 'a':
            efuse_reg_wr.addr = strtol(cmd[i + 1], NULL, 16);
            reg[4] = efuse_reg_wr.addr & 0xFF;
            reg[5] = (efuse_reg_wr.addr & 0xFF00) >> 8;
            reg[6] = (efuse_reg_wr.addr & 0xFF0000) >> 16;
            reg[7] = (efuse_reg_wr.addr & 0xFF000000) >> 24;
            i += 2;
            break;
        case 'v':
            efuse_reg_wr.value = strtol(cmd[i + 1], NULL, 16);
            reg[8] = efuse_reg_wr.value & 0xFF;
            reg[9] = (efuse_reg_wr.value & 0xFF00) >> 8;
            reg[10] = (efuse_reg_wr.value & 0xFF0000) >> 16;
            reg[11] = (efuse_reg_wr.value & 0xFF000000) >> 24;
            i += 2;
            break;

        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
    }

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, reg, (reg[3] + 4),
            NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_file_wr(char *cmd[], size_t len)
{
    uint8_t file[256] = {0x01, 0x01, 0xFF, 0x06,
                          0x00,
                          0x00, 0x00, 0x00, 0x00,
                          0x00,
                          0x00};
    uint32_t addr = 0;
    uint8_t file_path = 0;
    pkt_list_s *node = NULL;
    size_t i = 1;

    BPRINT_D("%s", __func__);
    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (len % 2) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }
    /****************************************************
    format:
    CMD: 01 01 FF LEN WR ADDR VALUE FILE_PATH
    EVENT: 04 0E LEN 01 01 FF ERRNO WR ADDR VALUE

    LEN: 1 byte
    WR: 1 byte
    ADDR: 4 bytes
    VALUE: 1 byte
    ERRNO: 2 bytes
    FILE_PATH:
    ****************************************************/

    while (i + 1 < len) {
        switch (*(cmd[i] + 1)) {
        case 'w':
            file[4] = WR_FLAG_WRITE;
            i += 1;
            break;
        case 'r':
            file[4] = WR_FLAG_READ;
            i += 1;
            break;
        case 'a':
            addr = strtol(cmd[i + 1], NULL, 16);
            file[5] = addr & 0xFF;
            file[6] = (addr & 0xFF00) >> 8;
            file[7] = (addr & 0xFF0000) >> 16;
            file[8] = (addr & 0xFF000000) >> 24;
            i += 2;
            break;
        case 'v':
            file[9] = strtol(cmd[i + 1], NULL, 16);
            i += 2;
            break;
        case 'f':
            strcpy((char *)&file[10], cmd[i + 1]);
            file[3] += strlen((char *)&file[10]) + 1;
            file_path = 1;
            i += 2;
            break;
        default:
            BPRINT_E("%s: Incorrect format: %s", __func__, cmd[i]);
            return NULL;
        }
    }

    if (file_path == 0)
    {
        BPRINT_E("%s: please input the file path and name(-f)", __func__);
        return NULL;
    }

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, file, (file[3] + 4),
            NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
static pkt_list_s *mtk_boots_cmd_sync(char *cmd[], size_t len)
{
    uint8_t access_code[] = {0x01, 0x12, 0xFC, 0x08, 0, 0, 0, 0, 0, 0, 0, 0};
    pkt_list_s *node = NULL;

    if (len == 2 && *(cmd[1] + 1) == 'h') {
        return NULL;
    } else if (len >= 2 && strlen(cmd[1]) == 23) {
        access_code[11] = strtol(cmd[1], NULL, 16);
        access_code[10] = strtol(cmd[1] + 3, NULL, 16);
        access_code[9] = strtol(cmd[1] + 6, NULL, 16);
        access_code[8] = strtol(cmd[1] + 9, NULL, 16);
        access_code[7] = strtol(cmd[1] + 12, NULL, 16);
        access_code[6] = strtol(cmd[1] + 15, NULL, 16);
        access_code[5] = strtol(cmd[1] + 18, NULL, 16);
        access_code[4] = strtol(cmd[1] + 21, NULL, 16);
    } else {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    if (!access_code[4] && !access_code[5] && !access_code[6] && !access_code[7]
            && !access_code[8] && !access_code[9] && !access_code[10] && !access_code[11]) {
        BPRINT_E("%s: Incorrect format", __func__);
        return NULL;
    }

    node = boots_pkt_node_push(SCRIPT_NONE, HCI_CMD_PKT, access_code,
            sizeof(access_code), NULL, NULL);
    return node;
}

//---------------------------------------------------------------------------
#define USAGE_RFTX_PREFACE \
    "RF TX test [-p <XX>] [-c <0-78>] [-t <XX>] [-l <DEC>]"
#define USAGE_RFTX_DETAILS \
    "      [P]attern:\n" \
    "          01:0000      02:1111\n" \
    "          03:1010      04:Pseudo Random Bit Sequence 9(Default)\n" \
    "          09:11110000  0A:Tx single tone\n" \
    "      [C]hannel: 0-78, hopping if ignore this\n" \
    "      Packet [T]ype:\n" \
    "          00:NULL  01:POLL  02:FHS   03:DM1   04:DH1   05:HV1   06:HV2   07:HV3\n" \
    "          08:DV    09:AUX   0A:DM3   0B:DH3   0E:DM5   0F:DH5   17:EV3   1C:EV4\n" \
    "          1D:EV5   24:2DH1  28:3DH1  2A:2DH3  2B:3DH3  2E:2DH5  2F:3DH5  36:2EV3\n" \
    "          37:3EV3  3C:2EV5  3D:3EV5\n" \
    "      [L]ength: Decimal value., If ignore this boots will use the default value\n" \
    "      NOTE: For other parameters command order is Whiten -> RFTX -> Sync -> PowerLvl"

#define USAGE_RFRX_PREFACE \
    "RF RX test start [-p <XX>] [-c <0-78>] [-t <XX>] [-a <XX:XX:XX:XX>]"
#define USAGE_RFRX_STOP_PREFACE \
    "RF RX test stop [-p <XX>] [-c <0-78>] [-t <XX>] [-a <XX:XX:XX:XX>]"
#define USAGE_RFRX_DETAILS \
    "      [P]attern:\n" \
    "          01:0000      02:1111\n" \
    "          03:1010      04:Pseudo Random Bit Sequence 9(Default)\n" \
    "          09:11110000\n" \
    "      [C]hannel: 0-78\n" \
    "      Packet [T]ype:\n" \
    "          03:DM1   04:DH1   0A:DM3   0B:DH3   0E:DM5   0F:DH5\n" \
    "          24:2DH1  28:3DH1  2A:2DH3  2B:3DH3  2E:2DH5  2F:3DH5\n" \
    "      Tester [A]ddress: UAP:LAP1:LAP2:LAP3(Default: 0x00A5F0C3 if ignore it)"
#define USAGE_BLETX_PREFACE \
    "BLE TX test [-c <0-39>] [-p <0-2>]"
#define USAGE_BLETX_DETAILS \
    "      [C]hannel: 0-39\n" \
    "      [P]attern: 0:PRBS9  1:11110000  2:10101010"
#define USAGE_BLEETX_PREFACE \
    "BLE Enhanced TX test [-c <0-39>] [-l <0-255>] [-p <0-7>] [-y <1-5>]"
#define USAGE_BLEETX_DETAILS \
    "      [C]hannel: 0-39\n" \
    "      [L]ength in bytes of packet: 0-255\n" \
    "      [P]ayload of packet:\n" \
    "          0:PRBS9      1:11110000      2:10101010  3:PRBS15\n" \
    "          4:11111111   5:00000000      6:00001111  7:01010101\n" \
    "      Ph[y]:\n" \
    "          1:LE 1M      2:LE 2M     3:LE Coded(S=8) 4:LE Coded(S=2)"
#define USAGE_BLEERX_PREFACE \
    "BLE Enhanced RX test [-c <0-39>] [-y <1-2>] [-m <0-1>]"
#define USAGE_BLEERX_DETAILS \
    "      [C]hannel: 0-39\n" \
    "      Ph[y]: 1:LE 1M   2:LE 2M"
#define USAGE_TXTONE_PREFACE \
    "TX tone test [-t <0-4>] [-c <0-78>] [-m <0/1>] [-d <0-2>] [-p <0/1>]"
#define USAGE_TXTONE_DETAILS \
    "      Tone [T]ype:\n" \
    "          0:SingleTone_DC  1:SingleTone_250K  2:SignleTone_400K\n" \
    "          3:SignleTone_1M  4:ModulationTone\n" \
    "      [C]hannel: 0-78\n" \
    "      The following parameters only for Modulation Tone\n" \
    "      [M]ode: 0:BT  1:BLE\n" \
    "      Mo[d]ulation(Only for BT mode): 0:1M  1:2M  2:3:M\n" \
    "      [P]attern: 0:PRBS9  1:PRBS15"
#define USAGE_RADIOSETTING_PREFACE \
    "Radio setting [-b <0-7>] [-e <0-7>] [-m <0-7>] [-s <0-3>] [-o <0-1>]"
#define USAGE_RADIOSETTING_DETAILS \
    "      [b]: BT TX power level\n" \
    "      [e]: BLE TX power level\n" \
    "      [m]: Max TX power level\n" \
    "      [s]: use for 1db adjust\n" \
    "      [o]: 0 - Default 3db diff, 1 - 0db diff mode"
#define USAGE_TXPOWEROFFSET_PREFACE \
    "TX power offset [-g0 <-16 - 8>] [-g1 <-16 - 8>] [-g2 <-16 - 8>] " \
                    "[-g3 <-16 - 8>] [-g4 <-16 - 8>] [-g5 <-16 - 8>]"
#define USAGE_TXPOWEROFFSET_DETAILS \
    "      [G]roupX:\n" \
    "        Group0: CH0  - CH12\n" \
    "        Group1: CH13 - CH25\n" \
    "        Group2: CH26 - CH38\n" \
    "        Group3: CH39 - CH51\n" \
    "        Group4: CH52 - CH64\n" \
    "        Group5: CH65 - CH78\n" \
    "      <-16 - 8>: unit is 0.25db per step and 0 is default value"
#define USAGE_EFUSE_WR_PREFACE \
    "Read/Write eFuse [-r/w] [-a <HEX:0-0xFFFF>] [-v <HEX:0-0xFF>]"
#define USAGE_EFUSE_WR_DETAILS \
    "      [w]rite: write efuse value\n" \
    "      [r]ead: read efuse value\n" \
    "      [a]ddr: address of efuse\n" \
    "      [v]alue: value of efuse"
#define USAGE_REGISTER_WR_PREFACE \
    "Read/Write Register [-r/w] [-a <HEX:0-0xFFFFFFFF>] [-v <HEX:0-0xFFFFFFFF>]"
#define USAGE_REGISTER_WR_DETAILS \
    "      [w]rite: write register value\n" \
    "      [r]ead: read register value\n" \
    "      [a]ddr: address of register\n" \
    "      [v]alue: value of register"
#define USAGE_FILE_WR_PREFACE \
    "Read/Write File [-r/w] [-a <HEX>] [-v <HEX:0-0xFF>] [-f <file_path>]"
#define USAGE_FILE_WR_DETAILS \
    "      [w]rite: write bin_file value\n" \
    "      [r]ead: read bin_file value\n" \
    "      [a]ddr: offset of bin_file\n" \
    "      [v]alue: value of bin_file\n" \
    "      [f]ile path: full path and name of bin file"

boots_cmds_s commands[] = {
    { "reset", mtk_boots_cmd_reset, "HCI Reset", "" },
    { "inq", mtk_boots_cmd_inquiry, "Inquiry [-t <X * 1.28 sec>]", "" },
    { "dut", mtk_boots_cmd_enter_dut, "Enter DUT mode", "" },
    { "raddr", mtk_boots_cmd_read_bd_addr, "Read BD address", "" },
    { "waddr", mtk_boots_cmd_write_bd_addr, "Write BD address <0E:8D:76:68:00:01>", "  NAP-UAP-LAP" },
    { "pwrlvl", mtk_boots_cmd_set_power_level, "Set power level(0xFC17) for RFTX <0-7>", "" },
    { "radio", mtk_boots_cmd_radio_setting, USAGE_RADIOSETTING_PREFACE, USAGE_RADIOSETTING_DETAILS },
    { "txpwoffset", mtk_boots_cmd_tx_power_offset, USAGE_TXPOWEROFFSET_PREFACE, USAGE_TXPOWEROFFSET_DETAILS },
    { "whiten", mtk_boots_cmd_set_whiten, "Set whiten <0:disable/1:enable>", "" },
    { "sync", mtk_boots_cmd_sync, "Access Code <XX:XX:XX:XX:XX:XX:XX:XX>", "" },
    { "rft", mtk_boots_cmd_rf_tx, USAGE_RFTX_PREFACE, USAGE_RFTX_DETAILS },
    { "rfr", mtk_boots_cmd_rf_rx, USAGE_RFRX_PREFACE, USAGE_RFRX_DETAILS },
    { "rfr_stop", mtk_boots_cmd_rf_rx, USAGE_RFRX_STOP_PREFACE, USAGE_RFRX_DETAILS },
    { "ble_tx", mtk_boots_cmd_ble_tx, USAGE_BLETX_PREFACE, USAGE_BLETX_DETAILS },
    { "ble_rx", mtk_boots_cmd_ble_rx, "BLE RX test [-c <0-39>]", "" },
    { "ble_etx", mtk_boots_cmd_ble_etx, USAGE_BLEETX_PREFACE, USAGE_BLEETX_DETAILS },
    { "ble_erx", mtk_boots_cmd_ble_erx, USAGE_BLEERX_PREFACE, USAGE_BLEERX_DETAILS },
    { "ble_stop", mtk_boots_cmd_ble_test_stop, "Stop BLE test", "" },
    { "txtone", mtk_boots_cmd_tx_tone, USAGE_TXTONE_PREFACE, USAGE_TXTONE_DETAILS },
    { "efuse", mtk_boots_cmd_efuse_wr, USAGE_EFUSE_WR_PREFACE, USAGE_EFUSE_WR_DETAILS },
    { "reg", mtk_boots_cmd_register_wr, USAGE_REGISTER_WR_PREFACE, USAGE_REGISTER_WR_DETAILS },
    { "file", mtk_boots_cmd_file_wr, USAGE_FILE_WR_PREFACE, USAGE_FILE_WR_DETAILS },
    { NULL, NULL, NULL, NULL }
};

//---------------------------------------------------------------------------
pkt_list_s *boots_script_handler(char *script)
{
    pkt_list_s *script_list = NULL;

    if (script == NULL) { // MAY add regular expression
        BPRINT_E("There is no script file");
        return NULL;
    } else {
        script_list = boots_script_parse(script);
        if (!script_list) return NULL;
    }
#if 0
    BPRINT_D("There are %d lines data", boots_pkt_list_amount(script_list));
    pkt_list_s *tmp = script_list;
    size_t num;
    while (tmp) {
        switch (tmp->s_type) {
        case SCRIPT_TX:
        case SCRIPT_RX:
        case SCRIPT_WAITRX:
        case SCRIPT_CMD:
            num = 0;
            BPRINT_D_RAW("[boots_pkt:D] TYPE = %02x <", tmp->s_type);
            while (num < tmp->len) {
                BPRINT_D_RAW(" %02x", tmp->u_cnt.data[num]);
                ++num;
            }
            BPRINT_D_RAW("\n");
            break;
        case SCRIPT_TITLE:
        case SCRIPT_PROC:
            BPRINT_D("TYPE = %02x < %s", tmp->s_type, tmp->u_cnt.msg);
            break;
        case SCRIPT_LOOP:
            BPRINT_D("TYPE = %02x < %d", tmp->s_type, tmp->u_cnt.loop);
            break;
        case SCRIPT_TIMEOUT:
            BPRINT_D("TYPE = %02x < %d", tmp->s_type, tmp->u_cnt.timo);
            break;
        case SCRIPT_WAIT:
            BPRINT_D("TYPE = %02x < %d", tmp->s_type, tmp->u_cnt.wait);
            break;
        case SCRIPT_USBALT:
            BPRINT_D("TYPE = %02x < %d", tmp->s_type, tmp->u_cnt.usbalt);
            break;
        default:
            BPRINT_D("TYPE = %02x", tmp->s_type);
            break;
        }
        tmp = tmp->next;
        if (tmp == script_list) {
            break;
        }
    }
#endif

    return script_list;
}

//---------------------------------------------------------------------------
pkt_list_s *boots_raw_cmd_handler(char *raw[], size_t len)
{
    size_t i = 0;
    uint8_t type = 0;
    pkt_list_s *node = NULL;

    if (!raw || len < 4) {
        BPRINT_E("Incorrect input(len: %d)", (int)len);
        return 0;
    }

    if (!memcmp(raw[0], "CMD", strlen(raw[0])) || !memcmp(raw[0], "cmd", strlen(raw[0]))) {
        size_t rawlen = strtol(raw[3], NULL, 16);
        if (len != rawlen + 4) {
            BPRINT_E("Incorrect command length: %d", (int)rawlen);
            return 0;
        }
        type = 0x01;
        i = 1;
    } else if (!memcmp(raw[0], "ACL", strlen(raw[0])) || !memcmp(raw[0], "acl", strlen(raw[0]))) {
        BPRINT_E("%s: ACL not support yet", __func__);
        return 0;
    } else if (!memcmp(raw[0], "SCO", strlen(raw[0])) || !memcmp(raw[0], "sco", strlen(raw[0]))) {
        BPRINT_E("%s: SCO not support yet", __func__);
        return 0;
    } else {
        BPRINT_E("%s: Incorrect argument", __func__);
        return 0;
    }

    node = boots_pkt_node_push(SCRIPT_NONE, type, NULL, len, NULL, NULL);
    node->u_cnt.data[0] = type;
    while (i < len && raw[i]) {
        node->u_cnt.data[i] = strtol(raw[i], NULL, 16);
        i++;
    }
    node->len = i;
    // TODO check packet length & content
    return node;   // actually raw data length
}

//---------------------------------------------------------------------------
pkt_list_s *boots_cmd_set_handler(char *cmd[], size_t len)
{
    uint8_t i = 0;
    pkt_list_s *list = NULL;

    if (!cmd || !len) return NULL;

    for (i = 0; i < ARRAY_SIZE(commands); i++) {
        if (commands[i].cmd && !memcmp(cmd[0], commands[i].cmd, strlen(cmd[0]))) {
            if ((list = commands[i].func(cmd, len)) == NULL) {
                BPRINT_I("%s\n%s", commands[i].comment, commands[i].details);
                return NULL;
            } else {
                return list;
            }
        }
    }
    BPRINT_I("%s: Command Not Found(%s)", __func__, cmd[0]);

    return NULL;
}

//---------------------------------------------------------------------------
