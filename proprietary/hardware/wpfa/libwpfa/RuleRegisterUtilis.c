#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "FilterStructure.h"
#include "FilterParsingUtilis.h"
#include "RuleRegisterUtilis.h"
#include <mtk_log.h>
// #include "netcap/net_cap.h"
#include "net_cap.h"

 char * const IPTABLES_PATH = "/system/bin/iptables-wrapper-1.0";
 char * const IP6TABLES_PATH = "/system/bin/ip6tables-wrapper-1.0";
 char * const WAIT_FLAG = "-w";
 char * const NEW_CHAIN = "-N";
 char * const NEW_RULE = "-A";
 char * const DELETE_RULE = "-D";
 char * const WPFA_CHAIN = "oem_wpfa";
 char * const OEM_IN_CHAIN = "oem_in";
 char * const PROTOCAL_FLAG = "-p";
 char * const PROTOCAL_TCP = "tcp";
 char * const PROTOCAL_UDP = "udp";
 char * const PROTOCAL_ICMP = "icmp";
 char * const PROTOCAL_ICMPV6 = "icmpv6";
 char * const PROTOCAL_ICMP_TYPE = "--icmp-type";
 char * const PROTOCAL_ICMPV6_TYPE = "--icmpv6-type";
 char * const PROTOCAL_ESP = "esp";
 char * const PROTOCAL_AH = "ah";
 char * const ACTION_SRC = "-s";
 char * const ACTION_DST = "-d";
 char * const ACTION_SRC_PORT = "--sport";
 char * const ACTION_DST_PORT = "--dport";
 char * const ACTION_M = "-m";
 char * const ACTION_ESPSPI = "--espspi";
 char * const ACTION_AHSPI = "--ahspi";
 char * const ACTION_FLAG = "-j";
 char * const ACTION_NFQUEUE = "NFQUEUE";
 char * const ACTION_QNUM = "--queue-num";
 char * const NFQUEUE_QNUM= REGISTER_NFQUEUE_QNUM;
 char * const NFQUEUE_QNUM_ICMP= REGISTER_ICMP_NFQUEUE_QNUM;
 char * const ACTION_BYPASS = "--queue-bypass";

int tryExecIptable(char **args){
    int ret = wpfaExecIptable(args);
    if(ret == WPFA_IPTABLE_EXEC_FALED){
        return MERGE_IPTABLE_ARG_FAILED;
    }
    if(ret == WPFA_IPTABLE_POLL_WAIT_ERR){
        return WPFA_IPTABLE_TEMPORARY_NOT_AVALIABLE;
    }
    return MERGE_IPTABLE_ARG_OK;
}

void dumpMergedRules(char **args){
    char argsOut[(MAX_IPTABLE_ARG_NUM-2)*COMMON_ARG_LENGTH + IP_LENGTH*2];
    int i, j = 0, count = 0;
    for(i = 0; i < MAX_IPTABLE_ARG_NUM ; i++){
        if(args[i] == NULL){
            break;
        }
        for(j = 0 ; j < strlen(args[i]) ; j++){
            if(args[i][j] != '\0'){
                argsOut[count] = args[i][j];
            }
            count++;
        }
        argsOut[count] = ' ';
        count++;
    }
    argsOut[count] = '\0';
    mtkLogD(RR_LOG_TAG, "argsOut = %s" , argsOut);
}

int icmpMergeArgs(wifiProxy_filter_ip_ver_e ipType, WPFA_filter_reg_t filter, int operation){
    wifiProxy_filter_protocol_e protocol = filter.protocol;
    wifiProxy_filter_config_e mConfig = filter.filter_config;
    uint8_t type = filter.icmp_type;
    uint8_t code = filter.icmp_code;
    char typeAndCode[10];
    char *icmpTypeArray;
    char *operationArray;
    char *iptablePathArray;
    char *icmptype;

    if(operation == IPTALBE_NEW_RULE){
        operationArray = NEW_RULE;
    }
    else {
        operationArray = DELETE_RULE;
    }

    if(checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_ICMP_TYPE) &&
        checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_ICMP_CODE)){
        sprintf(typeAndCode,"%hhu/%hhu", type, code);
    }
    else if(checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_ICMP_TYPE)){
        sprintf(typeAndCode,"%hhu", type);
    }
    else{
        mtkLogD(RR_LOG_TAG, "icmpMergeArgs: type error: %d\n", mConfig);
        return MERGE_IPTABLE_ARG_FAILED;
    }

    if(ipType == WIFIPROXY_FILTER_IP_VER_IPV4){
        if(protocol != WIFIPROXY_FILTER_PROTOCOL_ICMP){
            mtkLogD(RR_LOG_TAG, "icmpMergeArgs ipv4: protocol error: %d\n", protocol);
            return MERGE_IPTABLE_ARG_FAILED;
        }
        icmpTypeArray = PROTOCAL_ICMP;
        iptablePathArray = IPTABLES_PATH;
        icmptype = PROTOCAL_ICMP_TYPE;
    }
    else{
        if(protocol != WIFIPROXY_FILTER_PROTOCOL_ICMPV6){
            mtkLogD(RR_LOG_TAG, "icmpMergeArgs ipv6: protocol error: %d\n", protocol);
            return MERGE_IPTABLE_ARG_FAILED;
        }
        icmpTypeArray = PROTOCAL_ICMPV6;
        iptablePathArray = IP6TABLES_PATH;
        icmptype = PROTOCAL_ICMPV6_TYPE;
    }

    //example command: iptables -A OEM_INPUT -p icmp --icmp-type 3/11 -j NFQUEUE --queue-num 80
    char *args[] = {iptablePathArray, WAIT_FLAG, operationArray, WPFA_CHAIN, "-p", icmpTypeArray,
        icmptype, typeAndCode, "-j", "NFQUEUE", "--queue-num", NFQUEUE_QNUM_ICMP ,
        "--queue-bypass", NULL};

    mtkLogD(RR_LOG_TAG, "(a) type:\n");
    dumpMergedRules(args);

    return tryExecIptable(args);

}

int espAhMergeArgs(wifiProxy_filter_ip_ver_e ipType, WPFA_filter_reg_t filter, int operation){
    wifiProxy_filter_protocol_e athuType = filter.protocol;
    uint32_t spi = filter.esp_spi;
    uint32_t spi_mask = filter.esp_spi_mask;
    char spiRange[22];
    char *authTypeArray;
    char *authActionArray;
    char *operationArray;
    char *iptablePathArray;

    //wifiproxy ensure the spi range will be continuous,
    //Therefore we only need to get range.
    uint32_t min_spi = spi & spi_mask;
    uint32_t max_spi = spi | (~spi_mask);

    mtkLogD(RR_LOG_TAG, "spi = %u, spi_mask = %u,  (~spi_mask) = %u\n",
        spi, spi_mask, (~spi_mask));

    sprintf(spiRange,"%u:%u",min_spi, max_spi);

    if(ipType == WIFIPROXY_FILTER_IP_VER_IPV4){
        iptablePathArray = IPTABLES_PATH;
    }
    else{
        iptablePathArray = IP6TABLES_PATH;
    }

    if(operation == IPTALBE_NEW_RULE){
        operationArray = NEW_RULE;
    }
    else if(operation == IPTALBE_DELETE_RULE){
        operationArray = DELETE_RULE;
    }

    if(athuType == WIFIPROXY_FILTER_PROTOCOL_ESP){
        authTypeArray = PROTOCAL_ESP;
        authActionArray = ACTION_ESPSPI;
    }
    else if(athuType == WIFIPROXY_FILTER_PROTOCOL_AH){
        authTypeArray = PROTOCAL_AH;
        authActionArray = ACTION_AHSPI;
    }
    else{
        mtkLogD(RR_LOG_TAG, "espAhMergeArgs : protocol error: %d\n", athuType);
        return MERGE_IPTABLE_ARG_FAILED;
    }

    //example command:
    //ip6tables -I OUTPUT -p esp -m esp --espspi 600 -j NFQUEUE --queue-num 80
    char *args[] = {iptablePathArray, WAIT_FLAG, operationArray, WPFA_CHAIN, "-p", authTypeArray,
        "-m", authTypeArray, authActionArray, spiRange , "-j", "NFQUEUE", "--queue-num",
        NFQUEUE_QNUM , "--queue-bypass", NULL};

    mtkLogD(RR_LOG_TAG, "(b) type:\n");
    dumpMergedRules(args);

    return tryExecIptable(args);

}


void convertIpAddress(char *dst, uint8_t *ip, wifiProxy_filter_ip_ver_e ipType){
    if(ipType == WIFIPROXY_FILTER_IP_VER_IPV4){
        sprintf(dst, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
    }
    else{
        sprintf(dst, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                 ip[0], ip[1],
                 ip[2], ip[3],
                 ip[4], ip[5],
                 ip[6], ip[7],
                 ip[8], ip[9],
                 ip[10], ip[11],
                 ip[12], ip[13],
                 ip[14], ip[15]);
    }
}


int commonMergeArgs(wifiProxy_filter_ip_ver_e ipType, WPFA_filter_reg_t filter, int operation){
    wifiProxy_filter_config_e mConfig = filter.filter_config;
    char srcIp[IP_LENGTH];
    char dstIp[IP_LENGTH];
    char srcPortRange[COMMON_ARG_LENGTH];
    char dstPortRange[COMMON_ARG_LENGTH];
    char *operationArray;
    char *iptablePathArray;

    if(operation == IPTALBE_NEW_RULE){
        operationArray = NEW_RULE;
    }
    else {
        operationArray = DELETE_RULE;
    }

    if(ipType == WIFIPROXY_FILTER_IP_VER_IPV4){
        iptablePathArray = IPTABLES_PATH;
    }
    else{
        iptablePathArray = IP6TABLES_PATH;
    }

    char *args[MAX_IPTABLE_ARG_NUM];
    args[0] = iptablePathArray;
    args[1] = WAIT_FLAG;
    args[2] = operationArray;
    args[3] = WPFA_CHAIN;
    int i = 4;
    if(checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_PROTOCOL)){
        args[i] = PROTOCAL_FLAG;
        i++;
        switch(filter.protocol){
            case WIFIPROXY_FILTER_PROTOCOL_ICMP: {
                if(ipType != WIFIPROXY_FILTER_IP_VER_IPV4){
                    mtkLogD(RR_LOG_TAG, "failed in commonMergeArgs: PROTOCOL=WIFIPROXY_FILTER_PROTOCOL_ICMP, but ipType = %d",
                        ipType);
                    return MERGE_IPTABLE_ARG_FAILED;
                }
                args[i] = PROTOCAL_ICMP;
                i++;
                break;
            }
            case WIFIPROXY_FILTER_PROTOCOL_TCP: {
                args[i] = PROTOCAL_TCP;
                i++;
                break;
            }
            case WIFIPROXY_FILTER_PROTOCOL_UDP: {
                args[i] = PROTOCAL_UDP;
                i++;
                break;
            }
            case WIFIPROXY_FILTER_PROTOCOL_ICMPV6: {
                if(ipType != WIFIPROXY_FILTER_IP_VER_IPV6){
                    mtkLogD(RR_LOG_TAG, "failed in commonMergeArgs: PROTOCOL=WIFIPROXY_FILTER_PROTOCOL_ICMPV6, but ipType = %d",
                        ipType);
                    return MERGE_IPTABLE_ARG_FAILED;
                }
                args[i] = PROTOCAL_ICMPV6;
                i++;
                break;
            }
            case WIFIPROXY_FILTER_PROTOCOL_IPV4: {
                if(ipType != WIFIPROXY_FILTER_IP_VER_IPV4){
                    mtkLogD(RR_LOG_TAG, "failed in commonMergeArgs: PROTOCOL=WIFIPROXY_FILTER_PROTOCOL_IPV4, but ipType = %d",
                        ipType);
                    return MERGE_IPTABLE_ARG_FAILED;
                }
                char tempProtocol[5];
                sprintf(tempProtocol, "%hhu", WIFIPROXY_FILTER_PROTOCOL_IPV4);
                args[i] = tempProtocol;
                i++;
                break;
            }
            case WIFIPROXY_FILTER_PROTOCOL_IPV6: {
                if(ipType != WIFIPROXY_FILTER_IP_VER_IPV6){
                    mtkLogD(RR_LOG_TAG, "failed in commonMergeArgs: PROTOCOL=WIFIPROXY_FILTER_PROTOCOL_IPV6, but ipType = %d",
                        ipType);
                    return MERGE_IPTABLE_ARG_FAILED;
                }
                char tempProtocol[5];
                sprintf(tempProtocol, "%hhu", WIFIPROXY_FILTER_PROTOCOL_IPV6);
                args[i] = tempProtocol;
                i++;
                break;
            }
            default: {
                mtkLogD(RR_LOG_TAG, "protocol error:%d, not support", filter.protocol);
                return MERGE_IPTABLE_ARG_FAILED;
                break;
            }
        }
    }
    if(checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_SRC_IP)){
        args[i] = ACTION_SRC;
        i++;
        convertIpAddress(srcIp, filter.ip_src, ipType);
        args[i] = srcIp;
        i++;
    }
    if(checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_DST_IP)){
        args[i] = ACTION_DST;
        i++;
        convertIpAddress(dstIp, filter.ip_dest, ipType);
        args[i] = dstIp;
        i++;
    }
    if(checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_SRC_PORT)){
        if(!checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_PROTOCOL)){
            mtkLogD(RR_LOG_TAG, "failed in commonMergeArgs: no protocol specified");
            return MERGE_IPTABLE_ARG_FAILED;
        }
        if(filter.protocol != WIFIPROXY_FILTER_PROTOCOL_TCP &&
            filter.protocol != WIFIPROXY_FILTER_PROTOCOL_UDP){
            mtkLogD(RR_LOG_TAG, "failed in commonMergeArgs: protocol not allowed");
            return MERGE_IPTABLE_ARG_FAILED;
        }
        args[i] = ACTION_SRC_PORT;
        i++;

        //wifiproxy ensure the port range will be continuous,
        //Therefore we only need to get range.
        uint16_t min_port = filter.src_port & filter.src_port_mask;//spi & spi_mask;
        uint16_t max_port = filter.src_port | (~filter.src_port_mask);//spi | (~spi_mask);

        mtkLogD(RR_LOG_TAG, "src_port = %u, src_port_mask = %u,  (~src_port_mask) = %u",
            filter.src_port, filter.src_port_mask, (~filter.src_port_mask));

        sprintf(srcPortRange,"%u:%u",min_port, max_port);
        args[i] = srcPortRange;
        i++;
    }
    if(checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_DST_PORT)){
        if(!checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_PROTOCOL)){
            mtkLogD(RR_LOG_TAG, "failed in commonMergeArgs: no protocol specified");
            return MERGE_IPTABLE_ARG_FAILED;
        }
        if(filter.protocol != WIFIPROXY_FILTER_PROTOCOL_TCP &&
            filter.protocol != WIFIPROXY_FILTER_PROTOCOL_UDP){
            mtkLogD(RR_LOG_TAG, "failed in commonMergeArgs: protocol not allowed");
            return MERGE_IPTABLE_ARG_FAILED;
        }
        args[i] = ACTION_DST_PORT;
        i++;

        //wifiproxy ensure the port range will be continuous,
        //Therefore we only need to get range.
        uint16_t min_port = filter.dst_port & filter.dst_port_mask;//spi & spi_mask;
        uint16_t max_port = filter.dst_port | (~filter.dst_port_mask);//spi | (~spi_mask);

        mtkLogD(RR_LOG_TAG, "dst_port = %u, dst_port_mask = %u,  (~dst_port_mask) = %u",
            filter.dst_port, filter.dst_port_mask, (~filter.dst_port_mask));

        sprintf(dstPortRange,"%u:%u",min_port, max_port);
        args[i] = dstPortRange;
        i++;
    }
    args[i] = ACTION_FLAG;
    i++;
    args[i] = ACTION_NFQUEUE;
    i++;
    args[i] = ACTION_QNUM;
    i++;
    args[i] = NFQUEUE_QNUM;
    i++;
    args[i] = ACTION_BYPASS;
    i++;
    args[i] = NULL;

    mtkLogD(RR_LOG_TAG, "(d) type: total args num = %d\n" , i);

    //Example commands:
    //ip6tables -I OUTPUT -p tcp -s [ip] -d [ip] -sport [port_min:port_max] -dport
    //[port_min:port_max] -j NFQUEUE --queue-num 80 --queue-bypass
    dumpMergedRules(args);

    return tryExecIptable(args);

}


int initIptablesChain(){
    //Note: to use iptable-wrapper-1.0 to create chain,
    // -w is needed and needs the chain name started with "oem_"

    //Init ome_wpfa chain to contain filter rules.
    char *args1[]={IPTABLES_PATH, "-w", "-N", WPFA_CHAIN, NULL};
    char *args2[]={IPTABLES_PATH, "-w", "-F", WPFA_CHAIN, NULL};
    char *args3[]={IP6TABLES_PATH, "-w", "-N", WPFA_CHAIN, NULL};
    char *args4[]={IP6TABLES_PATH, "-w", "-F", WPFA_CHAIN, NULL};

    tryExecIptable(args1);
    tryExecIptable(args2);
    tryExecIptable(args3);
    tryExecIptable(args4);

    //Add ome_wpfa into oem_in to start filtering
    char *args5[]={IPTABLES_PATH, "-w", "-D", OEM_IN_CHAIN, "-j", WPFA_CHAIN, NULL};
    char *args6[]={IPTABLES_PATH, "-w", "-A", OEM_IN_CHAIN, "-j", WPFA_CHAIN, NULL};
    char *args7[]={IP6TABLES_PATH, "-w", "-D", OEM_IN_CHAIN, "-j", WPFA_CHAIN, NULL};
    char *args8[]={IP6TABLES_PATH, "-w", "-A", OEM_IN_CHAIN, "-j", WPFA_CHAIN, NULL};

    tryExecIptable(args5);
    tryExecIptable(args6);
    tryExecIptable(args7);
    tryExecIptable(args8);

    return MERGE_IPTABLE_ARG_OK;
}

