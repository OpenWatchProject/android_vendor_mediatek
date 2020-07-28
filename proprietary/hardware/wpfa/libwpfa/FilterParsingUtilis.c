#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FilterStructure.h"
#include "RuleRegisterUtilis.h"
#include "FilterParsingUtilis.h"
#include "WpfaCppUtils.h"
#include <mtk_log.h>

#include <stdarg.h>

typedef enum {
WPFA_FILTER_TYPE_ERROR                 =   -1,
WPFA_FILTER_TYPE_ICMP_TYPE_AND_CODE    =   0,
WPFA_FILTER_TYPE_ESP                =   1,
WPFA_FILTER_TYPE_OTHERS             =   2,
WPFA_FILTER_TYPE_MAX                =   255,
} wpfa_filter_type_e;

//Records the handshake result with MD.
filter_md_ap_ver_enum mFilterVersion;

/*************** Example filter section starts ***************/
void initTestingFilterStructue(wifiProxy_filter_reg_t *filter) {     //type d exapmle
    filter->filter_config = 0x0049;
    filter->operation_config = 0x0000;
    filter->ip_ver = WIFIPROXY_FILTER_IP_VER_IPV6;
    filter->protocol = WIFIPROXY_FILTER_PROTOCOL_TCP;
    filter->icmp_type = 0;
    filter->ip_src[0] = 0x08;
    filter->ip_src[1] = 0x08;
    filter->ip_src[2] = 0x08;
    filter->ip_src[3] = 0x08;
    filter->ip_src[4] = 0x08;
    filter->ip_src[5] = 0x08;
    filter->ip_src[6] = 0x08;
    filter->ip_src[7] = 0x08;
    filter->ip_src[8] = 0x08;
    filter->ip_src[9] = 0x08;
    filter->ip_src[10] = 0x08;
    filter->ip_src[11] = 0x08;
    filter->ip_src[12] = 0x08;
    filter->ip_src[13] = 0x08;
    filter->ip_src[14] = 0x08;
    filter->ip_src[15] = 0x08;
    filter->dst_port = 0x0064; //1100100
    filter->dst_port_mask = 0xFFF8;  //1111111111111000    means port 1100000~1100111
}

void initTestingFilterStructue1(wifiProxy_filter_reg_t *filter) {    //type c exapmle   but no protocol  will error
    filter->filter_config = 0x0040;
    filter->operation_config = 0x0000;
    filter->icmp_type = 0;
    filter->ip_ver = WIFIPROXY_FILTER_IP_VER_IPV4;
    filter->dst_port = 0x0064; //1100100
    filter->dst_port_mask = 0xFFF8;  //1111111111111000    means port 1100000~1100111
}

void initTestingFilterStructue2(wifiProxy_filter_reg_t *filter) {    //type c exapmle
    filter->filter_config = 0x0041;
    filter->operation_config = 0x0000;
    filter->icmp_type = 0;
    filter->ip_ver = WIFIPROXY_FILTER_IP_VER_IPV4;
    filter->protocol = WIFIPROXY_FILTER_PROTOCOL_UDP;
    filter->dst_port = 0x0064; //1100100
    filter->dst_port_mask = 0xFFF8; //1111111111111000    means port 1100000~1100111
}

void initTestingFilterStructue3(wifiProxy_filter_reg_t *filter) {    //type b exapmle
    filter->filter_config = 0x0449;
    filter->operation_config = 0x0000;
    filter->ip_ver = WIFIPROXY_FILTER_IP_VER_IPV4;
    filter->protocol = WIFIPROXY_FILTER_PROTOCOL_AH;
    filter->icmp_type = 0;
    filter->ip_src[0] = 0x08;
    filter->ip_src[1] = 0x08;
    filter->ip_src[2] = 0x08;
    filter->ip_src[3] = 0x08;
    filter->dst_port = 0x0064; //1100100
    filter->dst_port_mask = 0xFFF8; //1111111111111000    means port 1100000~1100111
    filter->esp_spi = 0x00000065;
    filter->esp_spi_mask = 0xFFFFFFFF;
}


void initTestingFilterStructue4(wifiProxy_filter_reg_t *filter) {    //type a exapmle   protocol is wrong, will error.
    filter->filter_config = 0x044F;
    filter->operation_config = 0x0000;
    filter->ip_ver = WIFIPROXY_FILTER_IP_VER_IPV4;
    filter->protocol = WIFIPROXY_FILTER_PROTOCOL_AH;
    filter->icmp_type = 3;
    filter->ip_src[0] = 0x08;
    filter->ip_src[1] = 0x08;
    filter->ip_src[2] = 0x08;
    filter->ip_src[3] = 0x08;
    filter->dst_port = 0x0064; //1100100
    filter->dst_port_mask = 0xFFF8; //1111111111111000    means port 1100000~1100111
    filter->esp_spi = 0x00000065;
    filter->esp_spi_mask = 0xFFFFFFFF;
}

void initTestingFilterStructue5(wifiProxy_filter_reg_t *filter) {    //type a exapmle  only type no code
    filter->filter_config = 0x044B;
    filter->operation_config = 0x0000;
    filter->ip_ver = WIFIPROXY_FILTER_IP_VER_IPV4;
    filter->protocol = WIFIPROXY_FILTER_PROTOCOL_ICMP;
    filter->icmp_type = 3;
    filter->ip_src[0] = 0x08;
    filter->ip_src[1] = 0x08;
    filter->ip_src[2] = 0x08;
    filter->ip_src[3] = 0x08;
    filter->dst_port = 0x0064; //1100100
    filter->dst_port_mask = 0xFFF8; //1111111111111000    means port 1100000~1100111
    filter->esp_spi = 0x00000065;
    filter->esp_spi_mask = 0xFFFFFFFF;
}

void initTestingFilterStructue6(wifiProxy_filter_reg_t *filter) {    //type a exapmle   type & code
    filter->filter_config = 0x044F;
    filter->operation_config = 0x0000;
    filter->ip_ver = WIFIPROXY_FILTER_IP_VER_IPV4;
    filter->protocol = WIFIPROXY_FILTER_PROTOCOL_ICMP;
    filter->icmp_type = 3;
    filter->icmp_code = 11;
    filter->ip_src[0] = 0x08;
    filter->ip_src[1] = 0x08;
    filter->ip_src[2] = 0x08;
    filter->ip_src[3] = 0x08;
    filter->dst_port = 0x0064; //1100100
    filter->dst_port_mask = 0xFFF8; //1111111111111000    means port 1100000~1100111
    filter->esp_spi = 0x00000065;
    filter->esp_spi_mask = 0xFFFFFFFF;
}

void initTestingFilterStructue7(wifiProxy_filter_reg_t *filter) {    //type d exapmle   icmpv4 & src add specified
    filter->filter_config = 0x0009;
    filter->operation_config = 0x0000;
    filter->ip_ver = WIFIPROXY_FILTER_IP_VER_IPV4;
    filter->protocol = WIFIPROXY_FILTER_PROTOCOL_TCP;
    filter->icmp_type = 0;
    //104.16.181.30
    filter->ip_src[0] = 0x68;
    filter->ip_src[1] = 0x10;
    filter->ip_src[2] = 0xB5;
    filter->ip_src[3] = 0x1E;
}

void initTestingFilterStructue8(wifiProxy_filter_reg_t *filter) {    //type d exapmle   icmpv4 & src add specified
    filter->filter_config = 0x0009;
    filter->operation_config = 0x0000;
    filter->ip_ver = WIFIPROXY_FILTER_IP_VER_IPV4;
    filter->protocol = WIFIPROXY_FILTER_PROTOCOL_TCP;
    filter->icmp_type = 0;
    //104.16.182.30
    filter->ip_src[0] = 0x68;
    filter->ip_src[1] = 0x10;
    filter->ip_src[2] = 0xB6;
    filter->ip_src[3] = 0x1E;
}

/*************** example filter section ends ***************/

WPFA_filter_reg_t preProcessFilter(wifiProxy_filter_reg_t rawFilter) {
    int i;
    WPFA_filter_reg_t myFilter;
    myFilter.filter_config = rawFilter.filter_config;
    myFilter.ip_ver = rawFilter.ip_ver;
    myFilter.protocol = rawFilter.protocol;
    myFilter.icmp_type = rawFilter.icmp_type;
    myFilter.icmp_code = rawFilter.icmp_code;
    mtkLogD(FP_LOG_TAG, "sizeof(ip_src) = %lu\n" , (unsigned long) sizeof(myFilter.ip_src));
    for (i = 0 ; i < sizeof(myFilter.ip_src) ; i++) {
        myFilter.ip_src[i] = rawFilter.ip_src[i];
        myFilter.ip_dest[i] = rawFilter.ip_dest[i];
    }
    myFilter.src_port = rawFilter.src_port;
    myFilter.src_port_mask = rawFilter.src_port_mask;
    myFilter.dst_port = rawFilter.dst_port;
    myFilter.dst_port_mask = rawFilter.dst_port_mask;

    myFilter.esp_spi = rawFilter.esp_spi;
    myFilter.esp_spi_mask = rawFilter.esp_spi_mask;

    return myFilter;
}

int checkFilterConfig(wifiProxy_filter_config_e filter_config, wifiProxy_filter_config_e config) {
    //return true if the filter_config has the config
    return ((filter_config & config) == config);
}

int checkIfOnlyHasPortConfig(wifiProxy_filter_config_e filter_config) {
    mtkLogD(FP_LOG_TAG, "checkIfOnlyHasPortConfig: filter_config = %hu, masks = %hu, %hu, %hu\n",
        filter_config, (WIFIPROXY_FILTER_TYPE_SRC_PORT | WIFIPROXY_FILTER_TYPE_DST_PORT),
        WIFIPROXY_FILTER_TYPE_DST_PORT, WIFIPROXY_FILTER_TYPE_SRC_PORT);
    if (filter_config == (WIFIPROXY_FILTER_TYPE_SRC_PORT | WIFIPROXY_FILTER_TYPE_DST_PORT) ||
        filter_config == WIFIPROXY_FILTER_TYPE_DST_PORT ||
        filter_config == WIFIPROXY_FILTER_TYPE_SRC_PORT) {
        return 1;
    }
    return 0;
}

int filterTypeParser(WPFA_filter_reg_t filter) {
    wifiProxy_filter_config_e mConfig = filter.filter_config;
    uint8_t pr = filter.protocol;
    mtkLogD(FP_LOG_TAG, "filterTypeParser: mConfig = %d\n" , mConfig);
    if (checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_ICMP_TYPE) ||
        checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_ICMP_CODE)) {
        if (checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_PROTOCOL) &&
            (pr == WIFIPROXY_FILTER_PROTOCOL_ICMP || pr == WIFIPROXY_FILTER_PROTOCOL_ICMPV6)) {
            mtkLogD(FP_LOG_TAG, "type (a) = %d\n" , WPFA_FILTER_TYPE_ICMP_TYPE_AND_CODE);
            return WPFA_FILTER_TYPE_ICMP_TYPE_AND_CODE;
        }
        else {
            mtkLogD(FP_LOG_TAG, "type (a) protocol error: mConfig: %hu , protocol: %hu\n" ,
                mConfig, pr);
            return WPFA_FILTER_TYPE_ERROR;
        }
    }
    else if (checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_ESP_SPI)) {
        if (checkFilterConfig(mConfig, WIFIPROXY_FILTER_TYPE_PROTOCOL)&&
            (pr == WIFIPROXY_FILTER_PROTOCOL_ESP || pr == WIFIPROXY_FILTER_PROTOCOL_AH)) {
            mtkLogD(FP_LOG_TAG, "type (b) = %d\n" , WPFA_FILTER_TYPE_ESP);
            return WPFA_FILTER_TYPE_ESP;
        }
        else {
            mtkLogD(FP_LOG_TAG, "type (b) protocol error: mConfig: %hu , protocol: %hu\n",
                mConfig, pr);
            return WPFA_FILTER_TYPE_ERROR;
        }
    }
    // Type (c) filter cases can be covered in type (d) filters.
    //else if(checkIfOnlyHasPortConfig(mConfig)){
    //    mtkLogD(FP_LOG_TAG, "type (c) = %d\n" , WPFA_FILTER_TYPE_PORT_RANGE_ONLY);
    //    return WPFA_FILTER_TYPE_PORT_RANGE_ONLY;
    //}
    else {
       mtkLogD(FP_LOG_TAG, "type (d) = %d\n" , WPFA_FILTER_TYPE_OTHERS);
       return WPFA_FILTER_TYPE_OTHERS;
    }

    return WPFA_FILTER_TYPE_ERROR;

}

int IptableRuleAdd(WPFA_filter_reg_t filter, int fid) {
    int type = filterTypeParser(filter);
    int result = MERGE_IPTABLE_ARG_FAILED;

    if (findFilterById(fid) == 1) {
        mtkLogD(FP_LOG_TAG, "IptableRuleAdd: error, fid=%d already exist\n", fid);
        return WPFA_FILTER_DUPLICATE;
    }
    switch (type) {
        case WPFA_FILTER_TYPE_ICMP_TYPE_AND_CODE: {
            result = icmpMergeArgs(filter.ip_ver, filter, IPTALBE_NEW_RULE);
            break;
        }
        case WPFA_FILTER_TYPE_ESP: {
            result = espAhMergeArgs(filter.ip_ver, filter, IPTALBE_NEW_RULE);
            break;
        }
        case WPFA_FILTER_TYPE_OTHERS: {
            result = commonMergeArgs(filter.ip_ver, filter, IPTALBE_NEW_RULE);
            break;
        }
        default: {
            return MERGE_IPTABLE_ARG_FAILED;
            break;
        }
    }
    mtkLogD(FP_LOG_TAG, "IptableRuleAdd: result = %d\n", result);
    if (result == MERGE_IPTABLE_ARG_OK) {
        addFilter(fid, filter);
    }
    return result;
}

int IptableRuleDelete(int fid) {
    WPFA_filter_reg_t filter;
    WPFA_filter_reg_t *filterPtr;

    if (findFilterById(fid) != 1) {
        return WPFA_FILTER_ID_NOT_EXIST;
    }
    filterPtr = getFilterFromMap(fid);
    if (filterPtr == NULL) {
        mtkLogD(FP_LOG_TAG, "IptableRuleDelete: filterPtr = NULL\n");
        return WPFA_FILTER_ID_NOT_EXIST;
    }

    filter = *(filterPtr);

    int type = filterTypeParser(filter);
    int result = MERGE_IPTABLE_ARG_FAILED;
    switch (type) {
        case WPFA_FILTER_TYPE_ICMP_TYPE_AND_CODE: {
            result = icmpMergeArgs(filter.ip_ver, filter, IPTALBE_DELETE_RULE);
            break;
        }
        case WPFA_FILTER_TYPE_ESP: {
            result = espAhMergeArgs(filter.ip_ver, filter, IPTALBE_DELETE_RULE);
            break;
        }
        case WPFA_FILTER_TYPE_OTHERS: {
            result = commonMergeArgs(filter.ip_ver, filter, IPTALBE_DELETE_RULE);
            break;
        }
        default: {
            return MERGE_IPTABLE_ARG_FAILED;
            break;
        }
    }
    mtkLogD(FP_LOG_TAG, "IptableRuleDelete: result = %d\n", result);
    if (result == MERGE_IPTABLE_ARG_OK) {
        removeFilter(fid);
    }

    return result;
}

void dumpFilter(WPFA_filter_reg_t filter) {
    char ip[IP_LENGTH];
    mtkLogD(FP_LOG_TAG, "filter_config = %u\n", filter.filter_config);
    mtkLogD(FP_LOG_TAG, "ip_ver = %hhu\n", filter.ip_ver);
    mtkLogD(FP_LOG_TAG, "protocol = %hhu\n", filter.protocol);
    mtkLogD(FP_LOG_TAG, "icmp_type = %hhu\n", filter.icmp_type);
    mtkLogD(FP_LOG_TAG, "icmp_code = %hhu\n", filter.icmp_code);
    convertIpAddress(ip, filter.ip_src, filter.ip_ver);
    mtkLogD(FP_LOG_TAG, "ip_src = %s\n", ip);
    convertIpAddress(ip, filter.ip_dest, filter.ip_ver);
    mtkLogD(FP_LOG_TAG, "ip_dest = %s\n", ip);
    mtkLogD(FP_LOG_TAG, "src_port = %hu\n", filter.src_port);
    mtkLogD(FP_LOG_TAG, "src_port_mask = %hu\n", filter.src_port_mask);
    mtkLogD(FP_LOG_TAG, "dst_port = %hu\n", filter.dst_port);
    mtkLogD(FP_LOG_TAG, "dst_port_mask = %hu\n", filter.dst_port_mask);
    mtkLogD(FP_LOG_TAG, "esp_spi = %u\n", filter.esp_spi);
    mtkLogD(FP_LOG_TAG, "esp_spi_mask = %u\n", filter.esp_spi_mask);
}

filter_md_ap_ver_enum getUsingFilterVersion() {
    return mFilterVersion;
}

void setUsingFilterVersion(filter_md_ap_ver_enum ver) {
    mFilterVersion = ver;
    if (mFilterVersion != CURRENT_AP_FILTER_VERSION) {
        mtkLogD(FP_LOG_TAG, "ERROR, MD ver not match:%d", ver);
    }
}

int executeFilterReg(wifiproxy_m2a_reg_dl_filter_t m2a_reg_dl_filter) {
    int fid = m2a_reg_dl_filter.fid;
    int result;
    mtkLogD(FP_LOG_TAG, "enter executeFilterReg fid=%d", fid);

    WPFA_filter_reg_t mfilter = preProcessFilter(m2a_reg_dl_filter.reg_hdr);
    dumpFilter(mfilter);
    result = IptableRuleAdd(mfilter, fid);

    mtkLogD(FP_LOG_TAG, "executeFilterReg: fid=%d, Register result = %d\n",
        fid, result);
    dumpAllFilterId();

    return result;
}

int executeFilterDeReg(uint32_t fid) {
    int result;
    mtkLogD(FP_LOG_TAG, "enter executeFilterDeReg fid=%d", fid);

    result = IptableRuleDelete(fid);

    mtkLogD(FP_LOG_TAG, "executeFilterDeReg: fid=%d, DeRegister result = %d\n",
        fid, result);
    dumpAllFilterId();

    return result;
}

/*************** testing section starts ***************/
void testingBasicFunctions() {
    mtkLogD(FP_LOG_TAG, "\ntry time: 0\n");
    wifiProxy_filter_reg_t *testingMdFilter = (wifiProxy_filter_reg_t *) malloc(sizeof(wifiProxy_filter_reg_t));
    initTestingFilterStructue(testingMdFilter);
    WPFA_filter_reg_t myFilter = preProcessFilter(*testingMdFilter);
    //dumpFilter(myFilter);
    IptableRuleAdd(myFilter, 0);

    mtkLogD(FP_LOG_TAG, "\ntry time: 1\n");
    initTestingFilterStructue1(testingMdFilter);
    myFilter = preProcessFilter(*testingMdFilter);
    IptableRuleAdd(myFilter, 1);

    mtkLogD(FP_LOG_TAG, "\ntry time: 2\n");
    initTestingFilterStructue2(testingMdFilter);
    myFilter = preProcessFilter(*testingMdFilter);
    IptableRuleAdd(myFilter, 2);

    mtkLogD(FP_LOG_TAG, "\ntry time: 3\n");
    initTestingFilterStructue3(testingMdFilter);
    myFilter = preProcessFilter(*testingMdFilter);
    IptableRuleAdd(myFilter, 3);

    mtkLogD(FP_LOG_TAG, "\ntry time: 4\n");
    initTestingFilterStructue4(testingMdFilter);
    myFilter = preProcessFilter(*testingMdFilter);
    IptableRuleAdd(myFilter, 4);

    mtkLogD(FP_LOG_TAG, "\ntry time: 5\n");
    initTestingFilterStructue5(testingMdFilter);
    myFilter = preProcessFilter(*testingMdFilter);
    IptableRuleAdd(myFilter, 5);

    mtkLogD(FP_LOG_TAG, "\ntry time: 6\n");
    initTestingFilterStructue6(testingMdFilter);
    myFilter = preProcessFilter(*testingMdFilter);
    IptableRuleAdd(myFilter, 6);

    dumpAllFilterId();

    //try delete///
    mtkLogD(FP_LOG_TAG, "\ntry delete for no. 5\n");
    IptableRuleDelete(5);

    dumpAllFilterId(testingMdFilter);

    free(testingMdFilter);
}

void testingFilter() {
    initialRuleContainer();

    mtkLogD(FP_LOG_TAG, "size of raw filter = %lu\n" ,
            (unsigned long) sizeof(wifiProxy_filter_reg_t));
    wifiproxy_m2a_reg_dl_filter_t m2a_reg_dl_filter;
    wifiproxy_m2a_dereg_dl_filter_t m2a_dereg_dl_filter_t;

    mtkLogD(FP_LOG_TAG, "\ntry time: 1\n");
    wifiProxy_filter_reg_t *testingMdFilter = (wifiProxy_filter_reg_t *) malloc(sizeof(wifiProxy_filter_reg_t));
    initTestingFilterStructue(testingMdFilter);
    m2a_reg_dl_filter.fid = 1;
    m2a_reg_dl_filter.reg_hdr = *testingMdFilter;
    executeFilterReg(m2a_reg_dl_filter);
    //dumpAllFilterId();

    mtkLogD(FP_LOG_TAG, "\ntry time: 2\n");
    initTestingFilterStructue2(testingMdFilter);
    m2a_reg_dl_filter.fid = 3;
    m2a_reg_dl_filter.reg_hdr = *testingMdFilter;
    executeFilterReg(m2a_reg_dl_filter);
    //dumpAllFilterId();

    mtkLogD(FP_LOG_TAG, "\ntry time: 3\n");
    initTestingFilterStructue3(testingMdFilter);
    m2a_reg_dl_filter.fid = 8;
    m2a_reg_dl_filter.reg_hdr = *testingMdFilter;
    executeFilterReg(m2a_reg_dl_filter);
    //dumpAllFilterId();

    mtkLogD(FP_LOG_TAG, "\ntry time: 4\n");
    initTestingFilterStructue7(testingMdFilter);
    m2a_reg_dl_filter.fid = 4294967295;
    m2a_reg_dl_filter.reg_hdr = *testingMdFilter;
    executeFilterReg(m2a_reg_dl_filter);

    mtkLogD(FP_LOG_TAG, "\ntry time: 5\n");
    initTestingFilterStructue8(testingMdFilter);
    m2a_reg_dl_filter.fid = 4294967290;
    m2a_reg_dl_filter.reg_hdr = *testingMdFilter;
    executeFilterReg(m2a_reg_dl_filter);

    executeFilterDeReg(3);
    free(testingMdFilter);

}
/*************** testing section Ends ***************/

