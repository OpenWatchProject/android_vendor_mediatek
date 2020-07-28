package com.mediatek.engineermode.iotconfig;

import java.util.HashMap;
import java.util.LinkedHashMap;

public class IotConfigConstant {

    //public
    public static final String FK_SS_BOOLCONFIG = "persist.vendor.ss.boolconfig";
    public static final String FK_SS_BOOLVALUE = "persist.vendor.ss.boolvalue";
    public static final String FK_SS_AUID = "persist.vendor.ss.auid";
    public static final String FK_SS_DIGEST_ID = "persist.vendor.ss.digest.id";
    public static final String FK_SS_DIGEST_PWD = "persist.vendor.ss.digest.pwd";
    public static final String FK_SS_MEDIATYPE = "persist.vendor.ss.mediatype";

    //Below-93Modem
    public static final String FK_SS_CONTENTTYPE = "persist.vendor.ss.contenttype";
    public static final String FK_SS_XCAPROOT = "persist.vendor.ss.xcaproot";
    public static final String FK_SS_RELEID_CFU = "persist.vendor.ss.ruleid.cfu";
    public static final String FK_SS_RELEID_CFB = "persist.vendor.ss.ruleid.cfb";
    public static final String FK_SS_RELEID_CFNRY = "persist.vendor.ss.ruleid.cfnry";
    public static final String FK_SS_RELEID_CFNRC = "persist.vendor.ss.ruleid.cfnrc";
    public static final String FK_SS_RELEID_CFNL = "persist.vendor.ss.ruleid.cfnl";
    public static final String FK_SS_XCAPPORT = "persist.vendor.ss.xcapport";
    public static final String FK_SS_ALIVETIMER = "persist.vendor.ss.alivetimer";
    public static final String FK_SS_REQTIMER = "persist.vendor.ss.reqtimer";
    public static final String FK_SS_CDTIMER = "persist.vendor.ss.cdtimer";
    public static final String FK_SS_GBA_BSFSERVERURL = "persist.vendor.ss.gba.bsfserverurl";
    public static final String FK_SS_GBA_ENABLEBATRUSTALL =
            "persist.vendor.ss.gba.enablegbatrustall";
    public static final String FK_SS_GBA_ENABLEBAFORCERUN =
            "persist.vendor.ss.gba.enablegbaforcerun";
    public static final String FK_SS_GBA_GBATYPE = "persist.vendor.ss.gba.gbatype";

    //93Modem
    public static final String FK_SS_XCAP_PROTOCOL_93 = "persist.vendor.ss.xcap.protocol";
    public static final String FK_SS_NAFHOST_93 = "persist.vendor.ss.nafhost";
    public static final String FK_SS_AUID_93 = "persist.vendor.ss.auid";
    public static final String FK_SS_DIGEST_ID_93 = "persist.vendor.ss.digest.id";
    public static final String FK_SS_DIGEST_PWD_93 = "persist.vendor.ss.digest.pwd";
    public static final String FK_SS_PHONE_CONTEXT_93 = "persist.vendor.ss.phone.context";

    public static final String FK_SS_RELEID_CFU_93 = "persist.vendor.ss.ruleid.cfu";
    public static final String FK_SS_RELEID_CFB_93 = "persist.vendor.ss.ruleid.cfb";
    public static final String FK_SS_RELEID_CFNRY_93 = "persist.vendor.ss.ruleid.cfnry";
    public static final String FK_SS_RELEID_CFNRC_93 = "persist.vendor.ss.ruleid.cfnrc";
    public static final String FK_SS_RELEID_CFNL_93 = "persist.vendor.ss.ruleid.cfnl";

    public static final String FK_SS_RELEID_BAOC_93 = "persist.vendor.ss.ruleid.baoc";
    public static final String FK_SS_RELEID_BAOIC_93 = "persist.vendor.ss.ruleid.baoic";
    public static final String FK_SS_RELEID_BAOICxh_93 = "persist.vendor.ss.ruleid.baoicxh";
    public static final String FK_SS_RELEID_BAIC_93 = "persist.vendor.ss.ruleid.baic";
    public static final String FK_SS_RELEID_BAICr_93 = "persist.vendor.ss.ruleid.baicr";

    public static final String FK_SS_GBA_PROTOCOL_93 = "persist.vendor.ss.gba.protocol";
    public static final String FK_SS_BSFHOST_93 = "persist.vendor.ss.bsfhost";
    public static final String FK_SS_BSFURLPATH_93 = "persist.vendor.ss.bsfurlpath";
    public static final String FK_SS_IMEIHEADER_93 = "persist.vendor.ss.imeiheader";
    public static final String FK_SS_GBA_TYPE_93 = "persist.vendor.ss.gba.type";
    public static final String FK_SS_GBA_PORT_93 = "persist.vendor.ss.gba.port";
    public static final String FK_SS_GBA_TIMEOUT_93 = "persist.vendor.ss.gba.timeout";
    public static final String FK_SS_GBA_RETRYTIMES_93 = "persist.vendor.ss.gba.retrytimes";
    public static final String FK_SS_GBA_REALM_93 = "persist.vendor.ss.gba.bsfrealm";

    public static final String FK_SS_UA_PREFIX_93 = "persist.vendor.ss.ua.prefix";
    public static final String FK_SS_XCAP_PORT_93 = "persist.vendor.ss.xcap.port";
    public static final String FK_SS_MEDIATYPE_93 = "persist.vendor.ss.mediatype";
    public static final String FK_SS_URL_ENCODING_93 = "persist.vendor.ss.urlencoding";
    public static final String FK_SS_XCAP_TIMEOUT_93 = "persist.vendor.ss.xcap.timeout";
    public static final String FK_SS_XCAP_RETRYTIMES_93 = "persist.vendor.ss.xcap.retrytimes";

    public static final String FK_SS_DNS_PRIORITY_93 = "persist.vendor.ss.dns.priority";

    //Beyond 93 Modem
    public static final String SS_PREFIX = "persist.vendor.ss.";
    public static final String FK_SS_XCAP_PROTOCOL_95 = SS_PREFIX + "xcap.xcap_protocol";
    public static final String FK_SS_NAFHOST_95 = SS_PREFIX + "xcap.xcap_url";
    public static final String FK_SS_AUID_95 = SS_PREFIX + "xcap.auid";
    public static final String FK_SS_DIGEST_ID_95 = SS_PREFIX + "xcap.digest_id";
    public static final String FK_SS_DIGEST_PWD_95 = SS_PREFIX + "xcap.digest_pwd";
    public static final String FK_SS_PHONE_CONTEXT_95 = SS_PREFIX + "xcap.phone_context";

    public static final String FK_SS_RELEID_CFU_95 = SS_PREFIX + "xcap.ruleid_cfu";
    public static final String FK_SS_RELEID_CFB_95 = SS_PREFIX + "xcap.ruleid_cfb";
    public static final String FK_SS_RELEID_CFNRY_95 = SS_PREFIX + "xcap.ruleid_cfnry";
    public static final String FK_SS_RELEID_CFNRC_95 = SS_PREFIX + "xcap.ruleid_cfnrc";
    public static final String FK_SS_RELEID_CFNL_95 = SS_PREFIX + "xcap.ruleid_cfnl";

    public static final String FK_SS_RELEID_BAOC_95 = SS_PREFIX + "xcap.ruleid_baoc";
    public static final String FK_SS_RELEID_BAOIC_95 = SS_PREFIX + "xcap.ruleid_baoic";
    public static final String FK_SS_RELEID_BAOICxh_95 = SS_PREFIX + "xcap.ruleid_baoicxh";
    public static final String FK_SS_RELEID_BAIC_95 = SS_PREFIX + "xcap.ruleid_baic";
    public static final String FK_SS_RELEID_BAICr_95 = SS_PREFIX + "xcap.ruleid_baicr";

    public static final String FK_SS_GBA_PROTOCOL_95 = SS_PREFIX + "gba.gba_protocol";
    public static final String FK_SS_BSFHOST_95 = SS_PREFIX + "gba.gba_url";
    public static final String FK_SS_BSFURLPATH_95 = SS_PREFIX + "gba.gba_url_path";
    public static final String FK_SS_IMEIHEADER_95 = SS_PREFIX + "gba.imei_header";
    public static final String FK_SS_GBA_TYPE_95 = SS_PREFIX + "gba.gba_type";
    public static final String FK_SS_GBA_PORT_95 = SS_PREFIX + "gba.gba_port";
    public static final String FK_SS_GBA_TIMEOUT_95 = SS_PREFIX + "gba.gba_connection_timeout";
    public static final String FK_SS_GBA_RETRYTIMES_95 = SS_PREFIX + "gba.gba_retry_times";
    public static final String FK_SS_GBA_REALM_95 = SS_PREFIX + "gba.gba_realm";

    public static final String FK_SS_UA_PREFIX_95 = SS_PREFIX + "xcap.user_agent_prefix";
    public static final String FK_SS_UA_MODEL_95 = SS_PREFIX + "xcap.user_agent_model";
    public static final String FK_SS_UA_DEFAULT_95 = SS_PREFIX + "xcap.user_agent_default";
    public static final String FK_SS_XCAP_PORT_95 = SS_PREFIX + "xcap.xcap_port";
    public static final String FK_SS_MEDIATYPE_95 = SS_PREFIX + "xcap.media_type";
    public static final String FK_SS_URL_ENCODING_95 = SS_PREFIX + "xcap.url_encoding";
    public static final String FK_SS_XCAP_TIMEOUT_95 = SS_PREFIX + "xcap.xcap_connection_timeout";
    public static final String FK_SS_XCAP_RETRYTIMES_95 = SS_PREFIX + "xcap.xcap_retry_times";

    public static final String FK_SS_APN_SELECTION_RULE_95 = SS_PREFIX + "cm.apn_selection_rule";
    public static final String FK_SS_PDN_RELEASING_TIMER_95 = SS_PREFIX + "cm.pdn_releasing_timer";
    public static final String FK_SS_PDN_WAIT_TIMER_95 = SS_PREFIX + "cm.pdn_wait_ps_reg_time";
    public static final String FK_SS_PDN_RETRY_TIMES_95 = SS_PREFIX + "cm.pdn_retry_times";

    public static final String FK_SS_PDN_REJECT_CODE_0_95 = SS_PREFIX + "cm.pdn_reject_code_0";
    public static final String FK_SS_PDN_REJECT_CODE_1_95 = SS_PREFIX + "cm.pdn_reject_code_1";
    public static final String FK_SS_PDN_REJECT_CODE_2_95 = SS_PREFIX + "cm.pdn_reject_code_2";
    public static final String FK_SS_PDN_REJECT_CODE_3_95 = SS_PREFIX + "cm.pdn_reject_code_3";
    public static final String FK_SS_PDN_REJECT_CODE_4_95 = SS_PREFIX + "cm.pdn_reject_code_4";
    public static final String FK_SS_PDN_REJECT_CODE_5_95 = SS_PREFIX + "cm.pdn_reject_code_5";
    public static final String FK_SS_PDN_REJECT_CODE_6_95 = SS_PREFIX + "cm.pdn_reject_code_6";
    public static final String FK_SS_PDN_REJECT_CODE_7_95 = SS_PREFIX + "cm.pdn_reject_code_7";
    public static final String FK_SS_PDN_REJECT_CODE_8_95 = SS_PREFIX + "cm.pdn_reject_code_8";
    public static final String FK_SS_PDN_REJECT_CODE_9_95 = SS_PREFIX + "cm.pdn_reject_code_9";
    public static final String FK_SS_PDN_REJECT_CODE_10_95 = SS_PREFIX + "cm.pdn_reject_code_10";
    public static final String FK_SS_PDN_REJECT_CODE_11_95 = SS_PREFIX + "cm.pdn_reject_code_11";
    public static final String FK_SS_PDN_REJECT_CODE_12_95 = SS_PREFIX + "cm.pdn_reject_code_12";
    public static final String FK_SS_PDN_REJECT_CODE_13_95 = SS_PREFIX + "cm.pdn_reject_code_13";
    public static final String FK_SS_PDN_REJECT_CODE_14_95 = SS_PREFIX + "cm.pdn_reject_code_14";
    public static final String FK_SS_PDN_REJECT_CODE_15_95 = SS_PREFIX + "cm.pdn_reject_code_15";

    public static final String FK_SS_DNS_PRIORITY_95 = SS_PREFIX + "http.dns_server_ipv4v6_priority";
    public static final String FK_SS_XCAP_CACHE_LEVEL_95 = SS_PREFIX + "xcap.cache_level";
    public static final String FK_SS_CONFIG_RESET_95 = SS_PREFIX + "reset";
    public static final String FK_SS_CONFIG_RESET_DONE_95 = SS_PREFIX + "resetdone";

    public static final String BOOLEANTYPE = "boolean";
    public static final String STRINGTYPE = "String";
    public static final String INTEGERTYPE = "int";

    // Below 93
    public static final String[] mXcapBoolCfgBelow93 = new String[] {
            "Not support XCAP", "Use http protocol",
            "Handle 409 exception", "Fill complete forward to",
            "Use namespace prefix", "Need to re-reg ims",
            "Need to append country code", "Support media tag",
            "Update whole CLIR", "Query whole Simserv", "Disable etag",
            "http error to unknown host", "Use xcap PDN",
            "Use internet PDN", "Noreply timer inside CF rule",
            "Support time slot", "Save whole node",
            "Set CFNL when set CFNRc", "Not support CFNL setting"};

    public static final LinkedHashMap<String, String> mXcapItemsBelow93 = new LinkedHashMap<String, String>() {
            {
                put("Element content type", FK_SS_CONTENTTYPE);
                put("AUID", IotConfigConstant.FK_SS_AUID);
                put("XCAP Root", FK_SS_XCAPROOT);
                put("RuleID CFU", FK_SS_RELEID_CFU);
                put("RuleID CFB", FK_SS_RELEID_CFB);
                put("RuleID CFNRy", FK_SS_RELEID_CFNRY);
                put("RuleID CFNRc", FK_SS_RELEID_CFNRC);
                put("RuleID CFNL", FK_SS_RELEID_CFNL);
                put("Digest user ID", FK_SS_DIGEST_ID);
                put("Digest Password", FK_SS_DIGEST_PWD);
                put("XCAP port", FK_SS_XCAPPORT);
                put("Media type",FK_SS_MEDIATYPE);
                put("Data keep alive timer", FK_SS_ALIVETIMER);
                put("Data connection time out", FK_SS_REQTIMER);
                put("Disconnect data PDN timer", FK_SS_CDTIMER);
                put("Gba Bsf server Url", FK_SS_GBA_BSFSERVERURL);
                put("Gba trust all", FK_SS_GBA_ENABLEBATRUSTALL);
                put("Gba force run", FK_SS_GBA_ENABLEBAFORCERUN);
                put("Gba type", FK_SS_GBA_GBATYPE);
            }
    };

    public static final HashMap<String, String> mXcapItemTypesBelow93 = new HashMap<String, String>() {
            {
                put("Element content type", STRINGTYPE);
                put("AUID", STRINGTYPE);
                put("XCAP Root", STRINGTYPE);
                put("RuleID CFU", STRINGTYPE);
                put("RuleID CFB", STRINGTYPE);
                put("RuleID CFNRy", STRINGTYPE);
                put("RuleID CFNRc", STRINGTYPE);
                put("RuleID CFNL", STRINGTYPE);
                put("Digest user ID", STRINGTYPE);
                put("Digest Password", STRINGTYPE);
                put("XCAP port", INTEGERTYPE);
                put("Media type", INTEGERTYPE);
                put("Data keep alive timer", INTEGERTYPE);
                put("Data connection time out", INTEGERTYPE);
                put("Disconnect data PDN timer", INTEGERTYPE);
                put("Gba Bsf server Url", STRINGTYPE);
                put("Gba trust all", BOOLEANTYPE);
                put("Gba force run", BOOLEANTYPE);
                put("Gba type", STRINGTYPE);
        }
    };

    // 93
    public static final String[] mXcapBoolCfgFor93 = new String[] {
            "Support Media Tag",
            "Support CFNL", "Enable TMPI",
            "Enable XCAP Cache", "Support Put CF Root",
            "Support Timer In CFNRY", "Support Put CLIR Root",
            "Need Quotation Mark", "SSL Trust", "Gzip Support",
            "Set CFNRC with CFNL", "Use SIP Forwarding Number",
            "Allow No Actions in Call Barring", "Clear Cache After Put",
            "Remove non-3GPP actions",
            "Use First XUI", "Use Saved XUI",
            "Use TMPI as GBA User Name", "Support Adding Rules",
            "User Agent String Customization", "Exclusive Call Barring",
            "Support Put CW Root", "Support Adding No Reply Timer"};

    public static final LinkedHashMap<String, String> mXcapItemsFor93 = new LinkedHashMap<String, String>() {
            {
                put("XCAP Protocol", FK_SS_XCAP_PROTOCOL_93);
                put("XCAP Root", FK_SS_NAFHOST_93);
                put("AUID", FK_SS_AUID_93);
                put("Digest User ID", FK_SS_DIGEST_ID_93);
                put("Digest User Password", FK_SS_DIGEST_PWD_93);
                put("Phone Context Domain", FK_SS_PHONE_CONTEXT_93);
                put("Rule ID for CFU", FK_SS_RELEID_CFU_93);
                put("Rule ID for CFB", FK_SS_RELEID_CFB_93);
                put("Rule ID for CFNRY", FK_SS_RELEID_CFNRY_93);
                put("Rule ID for CFNRC", FK_SS_RELEID_CFNRC_93);
                put("Rule ID for CFNL", FK_SS_RELEID_CFNL_93);
                put("Rule ID for BAOC", FK_SS_RELEID_BAOC_93);
                put("Rule ID for BAOIC", FK_SS_RELEID_BAOIC_93);
                put("Rule ID for BAOICxh", FK_SS_RELEID_BAOICxh_93);
                put("Rule ID for BAIC", FK_SS_RELEID_BAIC_93);
                put("Rule ID for BAICr", FK_SS_RELEID_BAICr_93);

                put("GBA Protocol", FK_SS_GBA_PROTOCOL_93);
                put("GBA BSF URL", FK_SS_BSFHOST_93);
                put("GBA BSF Path", FK_SS_BSFURLPATH_93);
                put("IMEI Header", FK_SS_IMEIHEADER_93);
                put("GBA Type", FK_SS_GBA_TYPE_93);
                put("GBA Port", FK_SS_GBA_PORT_93);
                put("GBA Connection Timeout", FK_SS_GBA_TIMEOUT_93);
                put("GBA Retry Times", FK_SS_GBA_RETRYTIMES_93);
                put("GBA REALM", FK_SS_GBA_REALM_93);

                put("User Agent Prefix", FK_SS_UA_PREFIX_93);
                put("XCAP Port", FK_SS_XCAP_PORT_93);
                put("Media Type", FK_SS_MEDIATYPE_93);
                put("URL Encoding", FK_SS_URL_ENCODING_93);
                put("XCAP Connection Timeout", FK_SS_XCAP_TIMEOUT_93);
                put("XCAP Retry Times", FK_SS_XCAP_RETRYTIMES_93);

                put("DNS IPv4/v6 Preference", FK_SS_DNS_PRIORITY_93);
            }
    };

    public static final HashMap<String, String> mXcapItemTypesFor93 = new HashMap<String, String>() {
        {
            put("XCAP Protocol", STRINGTYPE);
            put("XCAP Root", STRINGTYPE);
            put("AUID", STRINGTYPE);
            put("Digest User ID", STRINGTYPE);
            put("Digest User Password", STRINGTYPE);
            put("Phone Context Domain", STRINGTYPE);
            put("Rule ID for CFU", STRINGTYPE);
            put("Rule ID for CFB", STRINGTYPE);
            put("Rule ID for CFNRY", STRINGTYPE);
            put("Rule ID for CFNRC", STRINGTYPE);
            put("Rule ID for CFNL", STRINGTYPE);
            put("Rule ID for BAOC", STRINGTYPE);
            put("Rule ID for BAOIC", STRINGTYPE);
            put("Rule ID for BAOICxh", STRINGTYPE);
            put("Rule ID for BAIC", STRINGTYPE);
            put("Rule ID for BAICr", STRINGTYPE);

            put("GBA Protocol", STRINGTYPE);
            put("GBA BSF URL", STRINGTYPE);
            put("GBA BSF Path", STRINGTYPE);
            put("IMEI Header", STRINGTYPE);
            put("GBA Type", STRINGTYPE);
            put("GBA Port", INTEGERTYPE);
            put("GBA Connection Timeout", INTEGERTYPE);
            put("GBA Retry Times", INTEGERTYPE);
            put("GBA REALM", STRINGTYPE);

            put("User Agent Prefix", STRINGTYPE);
            put("XCAP Port", INTEGERTYPE);
            put("Media Type", INTEGERTYPE);
            put("URL Encoding", INTEGERTYPE);
            put("XCAP Connection Timeout", INTEGERTYPE);
            put("XCAP Retry Times", INTEGERTYPE);

            put("DNS IPv4/v6 Preference", INTEGERTYPE);
        }
    };

    // Beyond 93
    public static final String[] mXcapBoolCfgBeyond93 = new String[] {
            "Support Media Tag",
            "Support CFNL", "Enable TMPI",
            "Support Put CF Root",
            "Support Timer In CFNRY", "Support Put CLIR Root",
            "Need Quotation Mark", "SSL Trust",
            "Gzip Support",
            "Set CFNRC with CFNL", "Use SIP Forwarding Number",
            "Allow No Actions in Call Barring", "Remove non-3GPP actions",
            "Use First XUI", "Use Saved XUI",
            "Use TMPI as GBA User Name", "Support Adding Rules",
            "User Agent String Customization", "Exclusive Call Barring",
            "Support Put CW Root", "Support Adding No Reply Timer",
            "Accept Network Activated PDN", "Accept using Public WiFi"};

    public static final String[] mXcapBoolPropertyBeyond93 = new String[] {
            "xcap.support_media_tag",
            "xcap.support_cfnl", "gba.enable_tmpi_header",
            "xcap.support_put_cf_root",
            "xcap.timer_inside_cfnry", "xcap.support_clir_root",
            "xcap.attribute_need_quotation_mark", "http.support_always_trust_ssl",
            "http.support_gzip",
            "xcap.set_cfnrc_with_cfnl", "xcap.forwarding_number_use_sip_uri",
            "xcap.support_cb_no_action_allow", "xcap.remove_invalid_actions",
            "xcap.use_first_xui_element", "xcap.use_saved_xui",
            "gba.supportTMPIasUserName", "xcap.support_adding_unprovisioned_rule",
            "xcap.enable_custom_user_agent_string", "xcap.exclusive_cb",
            "xcap.support_put_cw_root", "xcap.support_adding_no_reply_timer",
            "cm.accept_network_act_pdn", "cm.accept_public_wifi"};

    public static final LinkedHashMap<String, String> mXcapItemsBeyond93 = new LinkedHashMap<String, String>() {
            {
                put("XCAP Protocol", FK_SS_XCAP_PROTOCOL_95);
                put("XCAP Root", FK_SS_NAFHOST_95);
                put("AUID", FK_SS_AUID_95);
                put("Digest User ID", FK_SS_DIGEST_ID_95);
                put("Digest User Password", FK_SS_DIGEST_PWD_95);
                put("Phone Context Domain", FK_SS_PHONE_CONTEXT_95);
                put("Rule ID for CFU", FK_SS_RELEID_CFU_95);
                put("Rule ID for CFB", FK_SS_RELEID_CFB_95);
                put("Rule ID for CFNRY", FK_SS_RELEID_CFNRY_95);
                put("Rule ID for CFNRC", FK_SS_RELEID_CFNRC_95);
                put("Rule ID for CFNL", FK_SS_RELEID_CFNL_95);
                put("Rule ID for BAOC", FK_SS_RELEID_BAOC_95);
                put("Rule ID for BAOIC", FK_SS_RELEID_BAOIC_95);
                put("Rule ID for BAOICxh", FK_SS_RELEID_BAOICxh_95);
                put("Rule ID for BAIC", FK_SS_RELEID_BAIC_95);
                put("Rule ID for BAICr", FK_SS_RELEID_BAICr_95);

                put("GBA Protocol", FK_SS_GBA_PROTOCOL_95);
                put("GBA BSF URL", FK_SS_BSFHOST_95);
                put("GBA BSF Path", FK_SS_BSFURLPATH_95);
                put("IMEI Header", FK_SS_IMEIHEADER_95);
                put("GBA Type", FK_SS_GBA_TYPE_95);
                put("GBA Port", FK_SS_GBA_PORT_95);
                put("GBA Connection Timeout", FK_SS_GBA_TIMEOUT_95);
                put("GBA Retry Times", FK_SS_GBA_RETRYTIMES_95);
                put("GBA REALM", FK_SS_GBA_REALM_95);

                put("User Agent Prefix", FK_SS_UA_PREFIX_95);
                put("User Agent Model", FK_SS_UA_MODEL_95);
                put("User Agent default String", FK_SS_UA_DEFAULT_95);
                put("XCAP Cache Level", FK_SS_XCAP_CACHE_LEVEL_95);
                put("XCAP Port", FK_SS_XCAP_PORT_95);
                put("Media Type", FK_SS_MEDIATYPE_95);
                put("URL Encoding", FK_SS_URL_ENCODING_95);
                put("XCAP Connection Timeout", FK_SS_XCAP_TIMEOUT_95);
                put("XCAP Retry Times", FK_SS_XCAP_RETRYTIMES_95);

                put("APN Selection Rule", FK_SS_APN_SELECTION_RULE_95);
                put("PDN Releasing Timer", FK_SS_PDN_RELEASING_TIMER_95);
                put("PDN wait PS Registered time", FK_SS_PDN_WAIT_TIMER_95);
                put("PDN Retry Times", FK_SS_PDN_RETRY_TIMES_95);
                put("PDN REJECT CODE 0 FOR CSFB", FK_SS_PDN_REJECT_CODE_0_95);
                put("PDN REJECT CODE 1 FOR CSFB", FK_SS_PDN_REJECT_CODE_1_95);
                put("PDN REJECT CODE 2 FOR CSFB", FK_SS_PDN_REJECT_CODE_2_95);
                put("PDN REJECT CODE 3 FOR CSFB", FK_SS_PDN_REJECT_CODE_3_95);
                put("PDN REJECT CODE 4 FOR CSFB", FK_SS_PDN_REJECT_CODE_4_95);
                put("PDN REJECT CODE 5 FOR CSFB", FK_SS_PDN_REJECT_CODE_5_95);
                put("PDN REJECT CODE 6 FOR CSFB", FK_SS_PDN_REJECT_CODE_6_95);
                put("PDN REJECT CODE 7 FOR CSFB", FK_SS_PDN_REJECT_CODE_7_95);
                put("PDN REJECT CODE 8 FOR CSFB", FK_SS_PDN_REJECT_CODE_8_95);
                put("PDN REJECT CODE 9 FOR CSFB", FK_SS_PDN_REJECT_CODE_9_95);
                put("PDN REJECT CODE 10 FOR CSFB", FK_SS_PDN_REJECT_CODE_10_95);
                put("PDN REJECT CODE 11 FOR CSFB", FK_SS_PDN_REJECT_CODE_11_95);
                put("PDN REJECT CODE 12 FOR CSFB", FK_SS_PDN_REJECT_CODE_12_95);
                put("PDN REJECT CODE 13 FOR CSFB", FK_SS_PDN_REJECT_CODE_13_95);
                put("PDN REJECT CODE 14 FOR CSFB", FK_SS_PDN_REJECT_CODE_14_95);
                put("PDN REJECT CODE 15 FOR CSFB", FK_SS_PDN_REJECT_CODE_15_95);

                put("DNS IPv4/v6 Preference", FK_SS_DNS_PRIORITY_95);
            }
    };

    public static final HashMap<String, String> mXcapItemTypesBeyond93 = new HashMap<String, String>() {
        {
            put("XCAP Protocol", STRINGTYPE);
            put("XCAP Root", STRINGTYPE);
            put("AUID", STRINGTYPE);
            put("Digest User ID", STRINGTYPE);
            put("Digest User Password", STRINGTYPE);
            put("Phone Context Domain", STRINGTYPE);
            put("Rule ID for CFU", STRINGTYPE);
            put("Rule ID for CFB", STRINGTYPE);
            put("Rule ID for CFNRY", STRINGTYPE);
            put("Rule ID for CFNRC", STRINGTYPE);
            put("Rule ID for CFNL", STRINGTYPE);
            put("Rule ID for BAOC", STRINGTYPE);
            put("Rule ID for BAOIC", STRINGTYPE);
            put("Rule ID for BAOICxh", STRINGTYPE);
            put("Rule ID for BAIC", STRINGTYPE);
            put("Rule ID for BAICr", STRINGTYPE);

            put("GBA Protocol", STRINGTYPE);
            put("GBA BSF URL", STRINGTYPE);
            put("GBA BSF Path", STRINGTYPE);
            put("IMEI Header", STRINGTYPE);
            put("GBA Type", STRINGTYPE);
            put("GBA Port", INTEGERTYPE);
            put("GBA Connection Timeout", INTEGERTYPE);
            put("GBA Retry Times", INTEGERTYPE);
            put("GBA REALM", STRINGTYPE);

            put("User Agent Prefix", STRINGTYPE);
            put("User Agent Model", STRINGTYPE);
            put("User Agent default String", STRINGTYPE);
            put("XCAP Cache Level", INTEGERTYPE);
            put("XCAP Port", INTEGERTYPE);
            put("Media Type", INTEGERTYPE);
            put("URL Encoding", INTEGERTYPE);
            put("XCAP Connection Timeout", INTEGERTYPE);
            put("XCAP Retry Times", INTEGERTYPE);

            put("APN Selection Rule", INTEGERTYPE);
            put("PDN Releasing Timer", INTEGERTYPE);
            put("PDN wait PS Registered time", INTEGERTYPE);
            put("PDN Retry Times", INTEGERTYPE);
            put("PDN REJECT CODE 0 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 1 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 2 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 3 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 4 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 5 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 6 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 7 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 8 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 9 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 10 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 11 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 12 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 13 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 14 FOR CSFB", INTEGERTYPE);
            put("PDN REJECT CODE 15 FOR CSFB", INTEGERTYPE);

            put("DNS IPv4/v6 Preference", INTEGERTYPE);
        }
    };

    public static boolean isNumeric(String s) {
        if (s != null && !"".equals(s.trim()))
            return s.matches("^[0-9]*$");
        else
            return false;
    }
}
