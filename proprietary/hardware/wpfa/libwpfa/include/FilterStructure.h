/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * APlicable license agreements with MediaTek Inc.
 */

#ifndef FILTERSTRUCTURE_H
#define FILTERSTRUCTURE_H

#include <stdint.h>

#define CURRENT_AP_FILTER_VERSION FILTER_VERSION_1_0
#define CURRENT_SHM_CONFIG_MODE   SHM_CONFIG_DL_ONLY

#define MAX_UL_IP_PKT_SIZE (1600)

typedef enum {
    // rx data --> AP to MD
    MSG_A2M_DATA_READY            = 0x1001,  // 4097: AP notify to MD for RX data
    MSG_M2A_REQUEST_DATA          = 0x1002,  // 4098: MD ready to get data from SHM
    MSG_A2M_REQUEST_DATA_ACK      = 0x1003,  // 4099: AP send ACK to MD
    MSG_M2A_REQUEST_DATA_DONE     = 0x1004,  // 4100: MD notify to AP request is done
    // tx data --> MD to AP
    MSG_M2A_DATA_READY            = 0x1005,  // 4101: MD notify to AP for Tx data
    MSG_A2M_REQUEST_DATA          = 0x1006,  // 4102: AP ready to get data from SHM
    MSG_M2A_REQUEST_DATA_ACK      = 0x1007,  // 4103: MD send ACK to AP
    MSG_A2M_REQUEST_DATA_DONE     = 0x1008,  // 4104: AP notify to MD request is done
    // Already Cleared all filters
    MSG_A2M_WPFA_INIT             = 0x2001,  // 8193: AP notify to MD that AP is initialized.
    MSG_A2M_WPFA_VERSION          = 0x2002,  // 8194
    MSG_M2A_WPFA_VERSION          = 0x2003,  // 8195
    // reg the filter
    MSG_M2A_REG_DL_FILTER         = 0x2005,  // 8197: Request to filter registration
    MSG_A2M_REG_REPLY             = 0x2006,  // 8198: Reply the filter registration
    // dereg the filter
    MSG_M2A_DEREG_DL_FILTER       = 0x2007,  // 8199: Request to filter deregistration
    MSG_A2M_DEREG_REPLY           = 0x2008,  // 8199: Reply the filter deregistration
    // UL data --> MD to AP
    MSG_M2A_UL_IP_PKT             = 0x3001,  // 12289: MD UL AP Path Via TTY instead of SHM

    MSG_ID_MAX


} msg_id_enum;

typedef uint16_t wifiProxy_filter_config_e;
enum {
WIFIPROXY_FILTER_TYPE_PROTOCOL          =     (0x0001 << 0),
WIFIPROXY_FILTER_TYPE_ICMP_TYPE         =     (0x0001 << 1),
WIFIPROXY_FILTER_TYPE_ICMP_CODE         =     (0x0001 << 2),
WIFIPROXY_FILTER_TYPE_SRC_IP            =     (0x0001 << 3),
WIFIPROXY_FILTER_TYPE_DST_IP            =     (0x0001 << 4),
WIFIPROXY_FILTER_TYPE_SRC_PORT          =     (0x0001 << 5),
WIFIPROXY_FILTER_TYPE_DST_PORT          =     (0x0001 << 6),
WIFIPROXY_FILTER_TYPE_TCP_FLAGS         =     (0x0001 << 7),
WIFIPROXY_FILTER_TYPE_ICMP_ERR_SRCIP    =     (0x0001 << 8),
WIFIPROXY_FILTER_TYPE_ICMP_ERR_SRC_PORT =     (0x0001 << 9),
WIFIPROXY_FILTER_TYPE_ESP_SPI           =     (0x0001 << 10),
WIFIPROXY_FILTER_TYPE_NON_ESP_MARKER    =     (0x0001 << 11)
};

typedef uint16_t filter_md_ap_ver_enum;
enum {
    FILTER_VERSION_1_0 = 0x0000,
    FILTER_VERSION_1_1 = 0x0001,
    FILTER_VERSION_1_2 = 0x0002,
    FILTER_MAX_VERSION = FILTER_VERSION_1_2
};

typedef uint16_t msg_type_enum;
enum {
    CCCI_CTRL_MSG                = 0x0001,        // CCCI control message, data = NULL
    CCCI_IP_TABLE_MSG            = 0x0002,        // IPTable rule update
    CCCI_UL_IP_PKT_MSG           = 0x0003          // uplink IP package
};

typedef enum {
    WPFA_REG_ACCPET                      = 0,
    WPFA_IPTABLE_TEMPORARY_NOT_AVALIABLE = -1,
    WPFA_FILTER_DUPLICATE                = -2,
    WPFA_RULE_ERROR                      = -3,
} reg_accept_error_cause_enum;

typedef enum {
    WPFA_DEREG_ACCPET                    = 0,
    WPFA_IPTABLE_NOT_AVALIABLE           = -1,
    WPFA_FILTER_ID_NOT_EXIST             = -11,
} dereg_accept_error_cause_enum;

typedef enum {
WIFIPROXY_FILTER_PROTOCOL_IPV6HOP    =     0,
WIFIPROXY_FILTER_PROTOCOL_ICMP       =     1,
WIFIPROXY_FILTER_PROTOCOL_IPV4       =     4,
WIFIPROXY_FILTER_PROTOCOL_TCP        =     6,
WIFIPROXY_FILTER_PROTOCOL_UDP        =     17,
WIFIPROXY_FILTER_PROTOCOL_IPV6       =     41,
WIFIPROXY_FILTER_PROTOCOL_IPV6_ROUTE =     43,
WIFIPROXY_FILTER_PROTOCOL_IPV6_FRAG  =     44,
WIFIPROXY_FILTER_PROTOCOL_ESP        =     50,
WIFIPROXY_FILTER_PROTOCOL_AH         =     51,
WIFIPROXY_FILTER_PROTOCOL_ICMPV6     =     58,
WIFIPROXY_FILTER_PROTOCOL_IPV6_NONXT =     59,
WIFIPROXY_FILTER_PROTOCOL_IPV6_OPTS  =     60,
WIFIPROXY_FILTER_PROTOCOL_ROHC       =     142,
WIFIPROXY_FILTER_PROTOCOL_RESERVED   =     255
} wifiProxy_filter_protocol_e;

typedef enum {
WIFIPROXY_FILTER_IP_VER_IPV4    =     0,
WIFIPROXY_FILTER_IP_VER_IPV6    =     1
} wifiProxy_filter_ip_ver_e;

typedef int event_id_enum;
enum {
    EVENT_M2A_WPFA_VERSION    = 1,
    EVENT_M2A_REG_DL_FILTER   = 2,
    EVENT_M2A_DEREG_DL_FILTER = 3,
    EVENT_M2A_READ_DATA_PTK   = 4,
    EVENT_M2A_MAX = 5
};

typedef int wpfa_shm_config_enum;
enum {
    SHM_CONFIG_DL_ONLY   = 1,
    SHM_CONFIG_UL_ONLY   = 2,
    SHM_CONFIG_SHARE     = 3
};

/* ######################################################################### */
#pragma pack(1)
/* ######################################################################### */

typedef struct wifiProxy_filter_reg {
    wifiProxy_filter_config_e        filter_config;    /* Enable flag for applied filters */
    uint16_t    operation_config;  /*Operation type for each config wifiProxy_filter_operations_e*/
    uint8_t     priority;        /* filter priority: low priority filter routed through AP */
    uint8_t     ip_ver;        /* IP Versionv=vIPV4 or IPV6 */
    uint8_t     protocol;    /* IP Protocol - IANA IP Protocol numbers */
    uint8_t     icmp_type;    /* IANA ICMP parameter types */
    uint8_t     icmp_code;    /* IANA ICMP code fields */
    uint8_t     ip_src[16];    /* IP Src: 4bytes / 16 bytes (IPV4/IPV6) */
    uint8_t     ip_src_mask;    /* IP mask in CIDR notation */
    uint8_t     ip_dest[16];    /* IP Dest: 4bytes / 16 bytes (IPV4/IPV6) */
    uint8_t     ip_dest_mask;    /* IP mask in CIDR notation */
    uint16_t    src_port;    /* TCP UDP Src Port number */
    uint16_t    src_port_mask;    /* TCP UDP Src Port number mask */
    uint16_t    dst_port;    /* TCP UDP Dst port number */
    uint16_t    dst_port_mask;    /* TCP UDP Dst port number mask */
    uint16_t    tcp_flags;    /* TCP flags */
    uint16_t    tcp_flags_mask;    /* TCP flags mask */
    uint16_t    tcp_flags_operation;    /* TCP flags mask */
    uint8_t     icmp_src_ip[16];    /* ICMP error message (type 3 and 11) */
    uint16_t    icmp_src_port;    /* ICMP error message (type 3 and 11) */
    uint16_t    icmp_src_mask;    /* Mask for src port in ICMP error resp */
    uint32_t    esp_spi;        /* Security parameter Index for ESP packets */
    uint32_t    esp_spi_mask;    /* SPI mask */
} wifiProxy_filter_reg_t;

// filter registration
typedef struct {
    wifiProxy_filter_reg_t      reg_hdr;
    uint32_t fid;
} wifiproxy_m2a_reg_dl_filter_t;

// filter deregistration
typedef struct {
     uint32_t fid;
} wifiproxy_m2a_dereg_dl_filter_t;

// filter registration responce
typedef struct {
    uint32_t fid;
    int32_t error_cause;
} wifiproxy_a2m_reg_reply_t;

// filter deregistration responce
typedef  wifiproxy_a2m_reg_reply_t    wifiproxy_a2m_dereg_reply_t;

typedef struct {
    filter_md_ap_ver_enum ap_filter_ver;
    filter_md_ap_ver_enum md_filter_ver;
    uint32_t dl_buffer_size;
    uint32_t ul_buffer_size;
} wifiproxy_ap_md_filter_ver_t;

// UL IP Packets
typedef struct {
    uint8_t  pkt[MAX_UL_IP_PKT_SIZE];
} wifiproxy_m2a_ul_ip_pkt_t;

typedef struct ccci_msg_hdr_t {
    uint16_t msg_id;
    uint16_t t_id;
    uint16_t msg_type;    // 1 for CCCI control message;  2 for IPTable rule update
    uint16_t param_16bit; // reserve to add parameter for respective msg
} ccci_msg_hdr_t;

typedef struct ccci_msg_body_t{
    union {
        wifiproxy_m2a_reg_dl_filter_t       reg_filter;
        wifiproxy_m2a_dereg_dl_filter_t     dereg_filter;
        wifiproxy_a2m_reg_reply_t           reg_reply_id;
        wifiproxy_a2m_dereg_reply_t         dereg_reply_id;
        wifiproxy_ap_md_filter_ver_t        version;
    } u;
} ccci_msg_body_t;

typedef struct ccci_msg_t {
    ccci_msg_hdr_t hdr;
    ccci_msg_body_t body;
} ccci_msg_t;

typedef struct ccci_msg_send_t {
    union {
        uint8_t         buffer[sizeof(ccci_msg_t)];
        ccci_msg_t      msg;
    };
    uint16_t            buffer_size;
} ccci_msg_send_t;

typedef struct ccci_msg_ul_ip_pkt_body_t{
    union {
        wifiproxy_m2a_ul_ip_pkt_t           ul_ip_pkt;
    } u;
} ccci_msg_ul_ip_pkt_body_t;

typedef struct ccci_msg_ul_ip_pkt_t {
    ccci_msg_hdr_t hdr;
    ccci_msg_ul_ip_pkt_body_t body;
} ccci_msg_ul_ip_pkt_t;


typedef struct WPFA_filter_reg {   // parameters that WPFA cares.
    wifiProxy_filter_config_e filter_config;  /* Enable flag for applied filters */
    uint8_t            ip_ver;                /* IP Versionv=vIPV4 or IPV6 */
    uint8_t            protocol;              /* IP Protocol ??IANA IP Protocol numbers */
    uint8_t            icmp_type;             /* IANA ICMP parameter types */
    uint8_t            icmp_code;             /* IANA ICMP code fields */
    uint8_t            ip_src[16];            /* IP Src: 4bytes / 16 bytes (IPV4/IPV6) */

    uint8_t            ip_dest[16];           /* IP Dest: 4bytes / 16 bytes (IPV4/IPV6) */
    uint16_t           src_port;              /* TCP UDP Src Port number */
    uint16_t           src_port_mask;         /* TCP UDP Src Port number mask */
    uint16_t           dst_port;              /* TCP UDP Dst port number */
    uint16_t           dst_port_mask;         /* TCP UDP Dst port number mask */

    uint32_t           esp_spi;               /* Security parameter Index for ESP packets */
    uint32_t           esp_spi_mask;          /* SPI mask */
} WPFA_filter_reg_t;

typedef struct WPFA_iptable_rule {   //Maintain in WPFA, for future delete.
    WPFA_filter_reg_t filter;
    uint32_t fid;
} WPFA_iptable_rule_t;

/* ######################################################################### */
#pragma pack(0)
/* ######################################################################### */

#endif // FILTERSTRUCTURE_H

