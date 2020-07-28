/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/
package com.orangelabs.rcs.core.ims.network.sip;

/**
 * Feature tags
 *
 * @author jexa7410
 * @author yplo6403
 */
public class FeatureTags {
    /**
     * OMA IM feature tag
     */
    public final static String FEATURE_OMA_IM = "+g.oma.sip-im";

    /**
     * 3GPP video share feature tag
     */
    public final static String FEATURE_3GPP_VIDEO_SHARE = "+g.3gpp.cs-voice";


    /**
     * video share feature tag, @tct-stack[IOT][ID_RCS_6_10_2]added by fang.wu@tcl.com
     */
        public final static String FEATURE_3GPP_ICSI_MMTEL_VIDEO = "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel.video";


    /**
     * 3GPP image share feature tag
     */
    public final static String FEATURE_3GPP_IMAGE_SHARE = "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.gsma-is\"";

    /**
     * 3GPP image share feature tag
     */
    public final static String FEATURE_3GPP_IM_FT = "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft\"";

    /**
     * 3GPP image share feature tag for RCS 2.0
     */
    public final static String FEATURE_3GPP_IMAGE_SHARE_RCS2 = "+g.3gpp.app_ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.gsma-is\"";

    /**
     * 3GPP location share feature tag
     */
    public final static String FEATURE_3GPP_LOCATION_SHARE = "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush\"";

    /**
     * RCS-e feature tag prefix
     */
    public final static String FEATURE_RCSE = "+g.3gpp.iari-ref";

    /**
     * RCS-e feature tag prefix
     */
    public final static String FEATURE_CPM_RCSE = "+g.3gpp.icsi-ref";

    /**
     * RCS-e feature tag prefix
     */
    public final static String FEATURE_RCSE_OFFLINE = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.vodafone.joyn-offline";

    /**
     * RCS-e image share feature tag
     */
    public final static String FEATURE_RCSE_IMAGE_SHARE = "urn%3Aurn-7%3A3gpp-application.ims.iari.gsma-is";

    /**
     * RCS-e chat feature tag
     */
    public final static String FEATURE_RCSE_CHAT = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im";

    /**
     * RCS-e file transfer feature tag
     */
    public final static String FEATURE_RCSE_FT = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft";

    /**
     * RCS-e file transfer over HTTP feature tag
     */
    public final static String FEATURE_RCSE_FT_HTTP = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp";

    /**
     * RCS-e presence discovery feature tag
     */
    public final static String FEATURE_RCSE_PRESENCE_DISCOVERY = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp";

    /**
     * RCS-e social presence feature tag
     */
    public final static String FEATURE_RCSE_SOCIAL_PRESENCE = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.sp";

    /**
     * RCS-e geolocation push feature tag
     */
    public final static String FEATURE_RCSE_GEOLOCATION_PUSH = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush";

    /**
     * RCS-e file transfer thumbnail feature tag
     */
    public final static String FEATURE_RCSE_FT_THUMBNAIL = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftthumb";

    /**
     * RCS-e file transfer S&F feature tag
     */
    public final static String FEATURE_RCSE_FT_SF = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftstandfw";

    /**
     * RCS-e group chat S&F feature tag
     */
    public final static String FEATURE_RCSE_GC_SF = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fullsfgroupchat";

    /**
     * 3GPP IP call feature tag
     */
    public final static String FEATURE_3GPP_IP_VOICE_CALL = "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"";

    public final static String FEATURE_3GPP_IP_VOICE_CALL_2 = "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel";
    /**
     * RCS-e IP call feature tag
     */
    public final static String FEATURE_RCSE_IP_VOICE_CALL = "+g.gsma.rcs.ipcall";

    /**
     * RCS IP video call feature tag
     */
    public final static String FEATURE_RCSE_IP_VIDEO_CALL = "video";

    /**
     * RCS-e extension feature tag prefix
     */
    public final static String FEATURE_RCSE_EXTENSION = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs";

    /**
     * SIP Automata feature tag
     *
     * <pre>
     * @see RFC 3840 "Indicating User Agent Capabilities in the Session Initiation Protocol (SIP)"
     *
     * The automata tag indicates whether the UA represents an automata (such as a voicemail server,
     * conference server, IVR, or recording device) or a human.
     * </pre>
     */
    public final static String FEATURE_SIP_AUTOMATA = "automata";


 /**
     * M: add feature tag for T-Mobile
     * @{T-Mobile
      */
    /**
     * 3GPP icsi for mmtel feature tag for T-Mobile
     */
    public final static String FEATURE_3GPP_ICSI_MMTEL= "+g.3gpp.icsi_ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"";

    /**
     * 3GPP icsi for content share video feature tag for T-Mobile
     *//*
    public final static String FEATURE_3GPP_ICSI_MMTEL_VIDEO= "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel.video";*/

    /**
     * 3GPP icsi for emergency feature tag for T-Mobile
     */
    public final static String FEATURE_3GPP_ICSI_EMERGENCY= "+g.3gpp.icsi_ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.e-location\"";

    /**
     * 3GPP SMS over IP feature tag for T-Mobile
     */
    public final static String FEATURE_3GPP_SMSIP= "+g.3gpp.smsip";
    /** T-Mobile@} */

    /*M : TCT GSM IOT patch */
        //@tct-stack yuxin.li@tcl.com IOT add +g.oma.sip-im tag in contact header
        public final static String FEATURE_RCSE_CHAT_OMA = "+g.oma.sip-im";
        /*@*/
        /*M: MTK integrated pathc */
        public final static String FEATURE_RCSE_INTEGERATED_IM = "+g.3gpp.iari_ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.joyn.intmsg\"";
        /*@*/

    //CPM Feature tags

    public final static String FEATURE_RCSE_CPM_SESSION = "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\"";

    public final static String FEATURE_RCSE_CPM_FT = "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer\"";

    public final static String FEATURE_RCSE_PAGER_MSG = "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg\"";

    public final static String FEATURE_RCSE_LARGE_MSG = "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg\"";

    public final static String FEATURE_CPM_SESSION = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session";

    public final static String FEATURE_CPM_FT = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer";

    public final static String FEATURE_CPM_MSG = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg";

    public final static String FEATURE_CPM_LARGE_MSG = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg";

    public final static String FEATURE_CPM_URN_SESSION = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session";

    public final static String FEATURE_CPM_URN_FT = "urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer";

    public final static String FEATURE_CPM_URN_MSG = "urn:urn-7:3gpp-service.ims.icsi.oma.cpm.msg";

    public final static String FEATURE_CPM_URN_LARGE_MSG = "urn:urn-7:3gpp-service.ims.icsi.oma.cpm.largemsg";

    public final static String FEATURE_CPM_URN_PREFERRED_SESSION = "urn:urn-7:3gpp-service.ims.icsi.oma.cpm.session";

    public final static String FEATURE_CPM_URN_PREFERRED_FT = "urn:urn-7:3gpp-service.ims.icsi.oma.cpm.filetransfer";

    public final static String FEATURE_CPM_URN_PREFERRED_GROUP_SESSION = "urn:urn-7:3gpp-service.ims.icsi.oma.cpm.session.group";

    public final static String FEATURE_CPM_URN_PREFERRED_GROUP_FT = "urn:urn-7:3gpp-service.ims.icsi.oma.cpm.filetransfer.group";



    /* Public account */
    public final static String FEATURE_CMCC_PUBLIC_ACCOUNT = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.publicmsg";

    public final static String FEATURE_CMCC_IARI_PUBLIC_ACCOUNT = "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.publicmsg\"";

    public final static String FEATURE_CMCC_URN_PUBLIC_ACCOUNT = "urn:urn-7:3gpp-application.ims.iari.rcs.mnc000.mcc460.publicmsg";

    /* Group chat management */
    public final static String FEATURE_CMCC_GROUP_MANAGE = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.gpmanage";

    public final static String FEATURE_CMCC_IARI_GROUP_MANAGE = "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.gpmanage;vs=1\"";

    public final static String FEATURE_CMCC_URN_GROUP_MANAGE = "urn:urn-7:3gpp-application.ims.iari.rcs.mnc000.mcc460.gpmanage;vs=1";

    /* Emoticon */
    public final static String FEATURE_CMCC_EMOTICON = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.vemotico";

    public final static String FEATURE_CMCC_IARI_EMOTICON = "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.vemoticon\";version=1_0";

    public final static String FEATURE_CMCC_URN_EMOTICON = "urn:urn-7:3gpp-application.ims.iari.rcs.mnc000.mcc460.vemoticon;version=1_0";

    /* Cloud file */
    public final static String FEATURE_CMCC_CLOUD_FILE = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.cloudfile";

    public final static String FEATURE_CMCC_IARI_CLOUD_FILE = "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.cloudfile\";version=1_0";

    public final static String FEATURE_CMCC_URN_CLOUD_FILE = "urn:urn-7:3gpp-application.ims.iari.rcs.mnc000.mcc460.cloudfile;version=1_0";

    /* Card message */
    public final static String FEATURE_CMCC_CARD_MSG = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.cardmsg";

    public final static String FEATURE_CMCC_IARI_CARD_MSG = "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.mnc000.mcc460.cardmsg\";version=1_0";

    public final static String FEATURE_CMCC_URN_CARD_MSG = "urn:urn-7:3gpp-application.ims.iari.rcs.mnc000.mcc460.cardmsg;version=1_0";

    //burn after reading feature tags
    public final static String FEATURE_CPM_BURNED_MSG = "barCycle";

    /**
     * 3GPP feature tag prefix
     */
    public final static String FEATURE_3GPP = "+g.3gpp.icsi-ref";


    /**
     * RCS-e IARI extension feature tag prefix
     */
    public final static String FEATURE_RCSE_IARI_EXTENSION = "urn%3Aurn-7%3A3gpp-application.ims.iari.rcs";

    /**
     * 3GPP RCS extension feature tag
     */
    public final static String FEATURE_3GPP_EXTENSION = "urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.rcs.extension";

    /**
     * 3GPP RCS service extension
     */
    public final static String FEATURE_3GPP_SERVICE_EXTENSION = "urn:urn-7:3gpp-service.ims.icsi.gsma.rcs.extension";

}
