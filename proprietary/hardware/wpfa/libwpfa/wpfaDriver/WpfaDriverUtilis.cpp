
#include "WpfaDriverUtilis.h"
#include "WpfaControlMsgHandler.h"

extern "C" {
#include "FilterParsingUtilis.h"
}

#define WPFA_D_LOG_TAG "WpfaDriverUtilis"

WpfaDriverUtilis *WpfaDriverUtilis::sInstance = NULL;
Mutex WpfaDriverUtilis::sWpfaDriverUtilisInitMutex;

char ulIpPkt1[] = "45000054bd19400040017ef9c0a82bdc090909090800e146000100010f728b5c0" \
                  "0000000af150e0000000000101112131415161718191a1b1c1d1e1f2021222324" \
                  "25262728292a2b2c2d2e2f3031323334353637";

WpfaDriverUtilis::WpfaDriverUtilis() {
    mtkLogD(WPFA_D_LOG_TAG, "-new()");
    init();
}

void WpfaDriverUtilis::init() {
    mFakeMdTid = FAKE_MD_TID_START;
    mEventId = TEST_EVENT_NONE;
    mtkLogD(WPFA_D_LOG_TAG, "-init()");

}

WpfaDriverUtilis::~WpfaDriverUtilis() {
    mtkLogD(WPFA_D_LOG_TAG, "-del()");
}

WpfaDriverUtilis* WpfaDriverUtilis::getInstance() {
    if (sInstance != NULL) {
       return sInstance;
    } else {
       sWpfaDriverUtilisInitMutex.lock();
       sInstance = new WpfaDriverUtilis();
       if (sInstance == NULL) {
          mtkLogE(WPFA_D_LOG_TAG, "new WpfaDriverUtilis fail");
       }
       sWpfaDriverUtilisInitMutex.unlock();
       return sInstance;
    }
}

void WpfaDriverUtilis::testStartNormal(const sp<WpfaDriverMessage>& msg) {
    mtkLogD(WPFA_D_LOG_TAG, "testStartNormal +");

    if (mReader != NULL) {
        fake_M2A_WPFA_VERSION(msg);

        fake_M2A_REG_DL_FILTER(1);
        fake_M2A_REG_DL_FILTER(2);
        fake_M2A_REG_DL_FILTER(3);
        fake_M2A_REG_DL_FILTER(7);
        fake_M2A_REG_DL_FILTER(8);

        fake_M2A_DEREG_DL_FILTER(3);
        fake_M2A_DEREG_DL_FILTER(2);

        fake_M2A_UL_IP_PKT(1);
    } else {
        mtkLogE(WPFA_D_LOG_TAG, "mReader is NULL, testStartNormal fail");
    }
    mtkLogD(WPFA_D_LOG_TAG, "testStartNormal -");
}

void WpfaDriverUtilis::fake_M2A_WPFA_VERSION(const sp<WpfaDriverMessage>& msg) {
    WpfaCcciDataHeader ccciDataHeader;
    ccci_msg_hdr_t *header = NULL;
    ccci_msg_body_t *body = NULL;

    header = (ccci_msg_hdr_t *) calloc(1, CCCI_HEADER_SIZE);
    if (header == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "OOM *heard");
        return;
    }

    body = (ccci_msg_body_t *) calloc(1, sizeof(ccci_msg_body_t));
    if (body == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "OOM *body");
        free(header);
        return;
    }
    // send response of MSG_A2M_WPFA_VERSION
    header->msg_id = MSG_M2A_WPFA_VERSION;
    header->t_id = msg->getTid();
    header->msg_type = msg->getType();
    header->param_16bit = msg->getParams();

    const wifiproxy_ap_md_filter_ver_t *verMsg =
            (const wifiproxy_ap_md_filter_ver_t *)msg->getData()->getData();

    body->u.version.ap_filter_ver = verMsg->ap_filter_ver;
    body->u.version.md_filter_ver = FILTER_VERSION_1_2;
    body->u.version.dl_buffer_size = verMsg->dl_buffer_size;
    body->u.version.ul_buffer_size = verMsg->ul_buffer_size;

    mtkLogD(WPFA_D_LOG_TAG, "fake_M2A_WPFA_VERSION md_filter_ver=%d", body->u.version.md_filter_ver);

    ccciDataHeader = WpfaCcciDataHeaderEncoder::decodeHeader(header);
    mReader->handleFilterRuleCtrlEvent(ccciDataHeader, WpfaDriverVersionData(body, 1));

    free(body);
    free(header);
}

void WpfaDriverUtilis::fake_M2A_REG_DL_FILTER(int testingFilterStrId) {
    WpfaCcciDataHeader ccciDataHeader;
    ccci_msg_hdr_t *header = NULL;
    ccci_msg_body_t *body = NULL;

    wifiproxy_m2a_reg_dl_filter_t m2a_reg_dl_filter;

    header = (ccci_msg_hdr_t *) calloc(1, CCCI_HEADER_SIZE);
    if (header == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "OOM *heard");
        return;
    }

    body = (ccci_msg_body_t *) calloc(1, sizeof(ccci_msg_body_t));
    if (body == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "OOM *body");
        free(header);
        return;
    }

    // fake message header
    header->msg_id = MSG_M2A_REG_DL_FILTER;
    header->t_id = fake_MD_Tid();
    header->msg_type = CCCI_IP_TABLE_MSG;
    header->param_16bit = 0;

    // fake message body
    wifiProxy_filter_reg_t *testingMdFilter = (wifiProxy_filter_reg_t *) malloc(sizeof(wifiProxy_filter_reg_t));
    if (testingMdFilter == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "testingMdFilter is NULL, return.");
        free(body);
        free(header);
        return;
    }

    int i = 0;
    testingMdFilter->priority = 0;
    testingMdFilter->icmp_code = 0;
    for (i = 0; i < 16; i++) {
        testingMdFilter->ip_src[i] = 0;
    }

    testingMdFilter->ip_src_mask = 0;

    for (i = 0; i < 16; i++) {
        testingMdFilter->ip_dest[i] = 0;
    }

    testingMdFilter->ip_dest_mask = 0;
    testingMdFilter->src_port = 0;
    testingMdFilter->src_port_mask = 0;
    testingMdFilter->dst_port = 0;
    testingMdFilter->dst_port_mask = 0;
    testingMdFilter->tcp_flags = 0;
    testingMdFilter->tcp_flags_mask = 0;
    testingMdFilter->tcp_flags_operation = 0;

    for (i = 0; i < 16; i++) {
        testingMdFilter->icmp_src_ip[i] = 0;
    }

    testingMdFilter->icmp_src_port = 0;
    testingMdFilter->icmp_src_mask = 0;
    testingMdFilter->esp_spi = 0;
    testingMdFilter->esp_spi_mask = 0;

    switch (testingFilterStrId) {
        case 1: {
            initTestingFilterStructue(testingMdFilter);
            m2a_reg_dl_filter.fid = 1;
            break;
        }
        case 2: {
            initTestingFilterStructue2(testingMdFilter);
            m2a_reg_dl_filter.fid = 2;
            break;
        }
        case 3: {
            initTestingFilterStructue3(testingMdFilter);
            m2a_reg_dl_filter.fid = 8;
            break;
        }
        case 7: {
            initTestingFilterStructue7(testingMdFilter);
            m2a_reg_dl_filter.fid = 4294967295;
            break;
        }
        case 8: {
            initTestingFilterStructue8(testingMdFilter);
            m2a_reg_dl_filter.fid = 4294967290;
            break;
        }
        default: {
            mtkLogE(WPFA_D_LOG_TAG, "can't handle testingFilterStrId=%d", testingFilterStrId);
            free(testingMdFilter);
            free(body);
            free(header);
            return;
        }
    }
    m2a_reg_dl_filter.reg_hdr = *testingMdFilter;

    body->u.reg_filter.reg_hdr = *testingMdFilter;
    body->u.reg_filter.fid = m2a_reg_dl_filter.fid;

    mtkLogD(WPFA_D_LOG_TAG, "fake_M2A_REG_DL_FILTER filterId=%d", body->u.reg_filter.fid);

    ccciDataHeader = WpfaCcciDataHeaderEncoder::decodeHeader(header);
    mReader->handleFilterRuleCtrlEvent(ccciDataHeader, WpfaDriverRegFilterData(body, 1));

    free(testingMdFilter);
    free(body);
    free(header);
}

void WpfaDriverUtilis::fake_M2A_DEREG_DL_FILTER(uint32_t filterId) {
    WpfaCcciDataHeader ccciDataHeader;
    ccci_msg_hdr_t *header = NULL;
    ccci_msg_body_t *body = NULL;

    wifiproxy_m2a_reg_dl_filter_t m2a_reg_dl_filter;

    header = (ccci_msg_hdr_t *) calloc(1, CCCI_HEADER_SIZE);
    if (header == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "OOM *heard");
        return;
    }

    body = (ccci_msg_body_t *) calloc(1, sizeof(ccci_msg_body_t));
    if (body == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "OOM *body");
        free(header);
        return;
    }
    // fake message header
    header->msg_id = MSG_M2A_DEREG_DL_FILTER;
    header->t_id = fake_MD_Tid();
    header->msg_type = CCCI_IP_TABLE_MSG;
    header->param_16bit = 0;

    body->u.dereg_reply_id.fid = filterId;
    body->u.dereg_reply_id.error_cause = 0;

    mtkLogD(WPFA_D_LOG_TAG, "fake_M2A_DEREG_DL_FILTER filterId=%d", body->u.dereg_reply_id.fid);

    ccciDataHeader = WpfaCcciDataHeaderEncoder::decodeHeader(header);
    mReader->handleFilterRuleCtrlEvent(ccciDataHeader, WpfaDriverDeRegFilterData(body, 1));

    free(body);
    free(header);

}

void WpfaDriverUtilis::fake_M2A_UL_IP_PKT(int IpPktId) {
    int i= 0;
    const uint8_t maxTryCnt = 10;
    uint8_t tryCnt = 0;

    WpfaCcciDataHeader ccciDataHeader;
    ccci_msg_hdr_t *header = NULL;
    ccci_msg_ul_ip_pkt_body_t *body = NULL;

    header = (ccci_msg_hdr_t *) calloc(1, CCCI_HEADER_SIZE);
    if (header == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "OOM *heard");
        return;
    }

    body = (ccci_msg_ul_ip_pkt_body_t *) calloc(1, sizeof(ccci_msg_ul_ip_pkt_body_t));
    if (body == NULL) {
        mtkLogE(WPFA_D_LOG_TAG, "OOM *body");
        free(header);
        return;
    }

    // send response of MSG_M2A_UL_IP_PKT
    header->msg_id = MSG_M2A_UL_IP_PKT;
    header->t_id = 999;
    header->msg_type = CCCI_UL_IP_PKT_MSG;

    if (IpPktId == 1) {
        header->param_16bit = 84;
        for (i = 0; i < 84; i++) {
            body->u.ul_ip_pkt.pkt[i] = (uint8_t)ulIpPkt1[i];
            //mtkLogD(WPFA_D_LOG_TAG, "u.ul_ip_pkt.pkt[%d]=%c",i, body->u.ul_ip_pkt.pkt[i]);
        }
    } else {
        mtkLogE(WPFA_D_LOG_TAG, "unknown IpPktId=%d", IpPktId);
    }
    mtkLogD(WPFA_D_LOG_TAG, "fake_M2A_UL_IP_PKT IpPktId=%d", IpPktId);

    for (tryCnt = 0; tryCnt < maxTryCnt; tryCnt++) {
        mtkLogD(WPFA_D_LOG_TAG, "tryCnt=%d", tryCnt);
        ccciDataHeader = WpfaCcciDataHeaderEncoder::decodeHeader(header);
        mReader->handleFilterRuleCtrlEvent(ccciDataHeader, WpfaDriverULIpPkt(body, 1));
        if (mEventId == TEST_EVENT_PKT_SEND_TO_KERNEL_SUCCESS) {
            // sleep to wait Wi-Fi ready
            usleep(10000 * 1000); /* 60 sec */
        } else {
            // sleep to wait Wi-Fi ready
            usleep(60000 * 1000); /* 60 sec */
        }
    }

    free(body);
    free(header);
}

uint16_t WpfaDriverUtilis::fake_MD_Tid() {
    uint16_t newTid = mFakeMdTid;
    newTid = newTid + 1;
    if (newTid >= FAKE_MD_TID_END) {
        newTid = FAKE_MD_TID_START;
    }
    mFakeMdTid = newTid;
    //mtkLogD(WPFA_D_LOG_TAG, "fake_MD_Tid() newTid=%d", mFakeMdTid);
    return mFakeMdTid;
}
