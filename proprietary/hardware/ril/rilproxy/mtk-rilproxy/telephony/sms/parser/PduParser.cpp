/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "PduParser.h"
#include "SmsParserUtils.h"
#include "GsmSmsAddress.h"
#include "SmsHeader.h"
#include "RfxLog.h"

#define RFX_LOG_TAG   "PduParser"

typedef wchar_t WCHAR;
/*****************************************************************************
 * Class PduParser
 *****************************************************************************/
PduParser::PduParser(BYTE *pdu, int length) :
        mPdu(pdu),
        mCur(0),
        mUserDataSeptetPadding(0),
        mUserDataHeader(NULL),
        mUserData(NULL),
        mUserDataLength(0),
        mPduLength(length) {
}

PduParser::~PduParser() {

}

string PduParser::getSCAddress() {
    int len = 0;
    string ret = "";

    // length of SC Address
    len = getByte();

    if (len == 0) {
        // no SC address
        ret = "";
    } else {
        // SC address
        // ret = PhoneNumberUtils::calledPartyBCDToString(mPdu, mCur, len);
    }
    mCur += len;
    return ret;
}

int PduParser::getByte() {
    return mPdu[mCur++] & 0xff;
}

long PduParser::getSCTimestampMillis() {
    mCur += 7;
    return 0;
}

GsmSmsAddress *PduParser::getAddress(/*SmsMessage *parent*/) {
    GsmSmsAddress *address = NULL;

    // "The Address-Length field is an integer representation of
    // the number field, i.e. excludes any semi-octet containing only
    // fill bits."
    // The TOA field is not included as part of this
    int addressLength = mPdu[mCur] & 0xff;
    int lengthBytes = 2 + (addressLength + 1) / 2;
    address = new GsmSmsAddress(mPdu, lengthBytes);
    mCur += lengthBytes;

    return address;
}

int PduParser::constructUserData(bool hasUserDataHeader, bool dataInSeptets) {
    int offset = mCur;
    int userDataLength = mPdu[offset++] & 0xff;
    int headerSeptets = 0;
    int userDataHeaderLength = 0;

    if (hasUserDataHeader) {
        userDataHeaderLength = mPdu[offset++] & 0xff;
        BYTE *udh = new BYTE[userDataHeaderLength];
        memcpy(udh, mPdu + offset, userDataHeaderLength);
        mUserDataHeader = SmsHeader::fromByteArray(udh, userDataHeaderLength/*, this*/);
        delete[] udh;
        offset += userDataHeaderLength;
        int headerBits = (userDataHeaderLength + 1) * 8;
        headerSeptets = headerBits / 7;
        headerSeptets += (headerBits % 7) > 0 ? 1 : 0;
        mUserDataSeptetPadding = (headerSeptets * 7) - headerBits;
    }
    int bufferLen = 0;
    if (dataInSeptets) {
        /*
         * Here we just create the user data length to be the remainder of
         * the pdu minus the user data header, since userDataLength means
         * the number of uncompressed septets.
        */
        bufferLen = mPduLength - offset;
    } else {
        /*
         * userDataLength is the count of octets, so just subtract the
         * user data header.
        */
        bufferLen = userDataLength - (hasUserDataHeader ? (userDataHeaderLength + 1) : 0);
        if (bufferLen < 0) {
            bufferLen = 0;
        }
    }
    mUserDataLength = bufferLen;
    mUserData = new BYTE[bufferLen];
    memcpy(mUserData, mPdu + offset, bufferLen);
    mCur = offset;

    if (dataInSeptets) {
        // Return the number of septets
        int count = userDataLength - headerSeptets;
        // If count < 0, return 0 (means UDL was probably incorrect)
        return count < 0 ? 0 : count;
    } else {
        // Return the number of octets
        return mUserDataLength;
    }
}

int PduParser::getUserDataLength() {
    return mUserDataLength;
}
BYTE* PduParser::getUserData() {
    return mUserData;
}

SmsHeader* PduParser::getUserDataHeader() {
    return mUserDataHeader;
}

const int WappushPduParser::WAP_PDU_LENGTH_QUOTE = 31;
const int WappushPduParser::WAP_PDU_SHORT_LENGTH_MAX = 30;
const int WappushPduParser::Q_VALUE = 0x00;
const string WappushPduParser::CONTENT_TYPE_B_PUSH_CO = "application/vnd.wap.coc";
const int WappushPduParser::PARAMETER_ID_X_WAP_APPLICATION_ID = 0x2f;
const int WappushPduParser::PDU_TYPE_PUSH = 0x06;
const int WappushPduParser::PDU_TYPE_CONFIRMED_PUSH = 0x07;
map<int, string> WappushPduParser::WELL_KNOWN_MIME_TYPES;
map<int, string> WappushPduParser::WELL_KNOWN_PARAMETERS;
map<int, string> WappushPduParser::WELL_KNOWN_HEADERS;

void WappushPduParser::initWapParaMap() {
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x00, "*/*"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x01, "text/*"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x02, "text/html"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x03, "text/plain"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x04, "text/x-hdml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x05, "text/x-ttml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x06, "text/x-vCalendar"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x07, "text/x-vCard"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x08, "text/vnd.wap.wml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x09, "text/vnd.wap.wmlscript"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0A, "text/vnd.wap.wta-event"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0B, "multipart/*"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0C, "multipart/mixed"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0D, "multipart/form-data"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0E, "multipart/byterantes"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0F, "multipart/alternative"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x10, "application/*"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x11, "application/java-vm"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x12, "application/x-www-form-urlencoded"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x13, "application/x-hdmlc"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x14, "application/vnd.wap.wmlc"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x15, "application/vnd.wap.wmlscriptc"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x16, "application/vnd.wap.wta-eventc"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x17, "application/vnd.wap.uaprof"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x18, "application/vnd.wap.wtls-ca-certificate"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x19, "application/vnd.wap.wtls-user-certificate"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x1A, "application/x-x509-ca-cert"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x1B, "application/x-x509-user-cert"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x1C, "image/*"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x1D, "image/gif"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x1E, "image/jpeg"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x1F, "image/tiff"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x20, "image/png"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x21, "image/vnd.wap.wbmp"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x22, "application/vnd.wap.multipart.*"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x23, "application/vnd.wap.multipart.mixed"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x24, "application/vnd.wap.multipart.form-data"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x25, "application/vnd.wap.multipart.byteranges"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x26, "application/vnd.wap.multipart.alternative"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x27, "application/xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x28, "text/xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x29, "application/vnd.wap.wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x2A, "application/x-x968-cross-cert"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x2B, "application/x-x968-ca-cert"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x2C, "application/x-x968-user-cert"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x2D, "text/vnd.wap.si"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x2E, "application/vnd.wap.sic"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x2F, "text/vnd.wap.sl"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x30, "application/vnd.wap.slc"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x31, "text/vnd.wap.co"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x32, "application/vnd.wap.coc"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x33, "application/vnd.wap.multipart.related"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x34, "application/vnd.wap.sia"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x35, "text/vnd.wap.connectivity-xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x36, "application/vnd.wap.connectivity-wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x37, "application/pkcs7-mime"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x38, "application/vnd.wap.hashed-certificate"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x39, "application/vnd.wap.signed-certificate"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x3A, "application/vnd.wap.cert-response"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x3B, "application/xhtml+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x3C, "application/wml+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x3D, "text/css"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x3E, "application/vnd.wap.mms-message"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x3F, "application/vnd.wap.rollover-certificate"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x40, "application/vnd.wap.locc+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x41, "application/vnd.wap.loc+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x42, "application/vnd.syncml.dm+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x43, "application/vnd.syncml.dm+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x44, "application/vnd.syncml.notification"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x45, "application/vnd.wap.xhtml+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x46, "application/vnd.wv.csp.cir"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x47, "application/vnd.oma.dd+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x48, "application/vnd.oma.drm.message"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x49, "application/vnd.oma.drm.content"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x4A, "application/vnd.oma.drm.rights+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x4B, "application/vnd.oma.drm.rights+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x4C, "application/vnd.wv.csp+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x4D, "application/vnd.wv.csp+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x4E, "application/vnd.syncml.ds.notification"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x4F, "audio/*"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x50, "video/*"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x51, "application/vnd.oma.dd2+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x52, "application/mikey"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x53, "application/vnd.oma.dcd"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x54, "application/vnd.oma.dcdc"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0201, "application/vnd.uplanet.cacheop-wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0202, "application/vnd.uplanet.signal"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0203, "application/vnd.uplanet.alert-wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0204, "application/vnd.uplanet.list-wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0205, "application/vnd.uplanet.listcmd-wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0206, "application/vnd.uplanet.channel-wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0207, "application/vnd.uplanet.provisioning-status-uri"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0208, "x-wap.multipart/vnd.uplanet.header-set"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0209, "application/vnd.uplanet.bearer-choice-wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x020A, "application/vnd.phonecom.mmc-wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x020B, "application/vnd.nokia.syncset+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x020C, "image/x-up-wpng"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0300, "application/iota.mmc-wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0301, "application/iota.mmc-xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0302, "application/vnd.syncml+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0303, "application/vnd.syncml+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0304, "text/vnd.wap.emn+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0305, "text/calendar"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0306, "application/vnd.omads-email+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0307, "application/vnd.omads-file+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0308, "application/vnd.omads-folder+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0309, "text/directory;profile=vCard"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x030A, "application/vnd.wap.emn+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x030B, "application/vnd.nokia.ipdc-purchase-response"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x030C, "application/vnd.motorola.screen3+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x030D, "application/vnd.motorola.screen3+gzip"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x030E, "application/vnd.cmcc.setting+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x030F, "application/vnd.cmcc.bombing+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0310, "application/vnd.docomo.pf"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0311, "application/vnd.docomo.ub"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0312, "application/vnd.omaloc-supl-init"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0313, "application/vnd.oma.group-usage-list+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0314, "application/oma-directory+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0315, "application/vnd.docomo.pf2"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0316, "application/vnd.oma.drm.roap-trigger+wbxml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0317, "application/vnd.sbm.mid2"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0318, "application/vnd.wmf.bootstrap"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x0319, "application/vnc.cmcc.dcd+xml"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x031A, "application/vnd.sbm.cid"));
    WELL_KNOWN_MIME_TYPES.insert(pair<int, string>(0x031B, "application/vnd.oma.bcast.provisioningtrigger"));

    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x00, "Q"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x01, "Charset"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x02, "Level"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x03, "Type"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x07, "Differences"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x08, "Padding"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x09, "Type"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x0E, "Max-Age"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x10, "Secure"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x11, "SEC"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x12, "MAC"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x13, "Creation-date"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x14, "Modification-date"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x15, "Read-date"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x16, "Size"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x17, "Name"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x18, "Filename"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x19, "Start"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x1A, "Start-info"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x1B, "Comment"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x1C, "Domain"));
    WELL_KNOWN_PARAMETERS.insert(pair<int, string>(0x1D, "Path"));

    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x00, "Accept"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x01, "Accept-Charset"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x02, "Accept-Encoding"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x03, "Accept-Language"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x04, "Accept-Ranges"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x05, "Age"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x06, "Allow"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x07, "Authorization"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x08, "Cache-Control"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x09, "Connection"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x0A, "Content-Base"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x0B, "Content-Encoding"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x0C, "Content-Language"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x0D, "Content-Length"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x0E, "Content-Location"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x0F, "Content-MD5"));

    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x10, "Content-Range"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x11, "Content-Type"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x12, "Date"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x13, "Etag"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x14, "Expires"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x15, "From"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x16, "Host"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x17, "If-Modified-Since"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x18, "If-Match"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x19, "If-None-Match"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x1A, "If-Range"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x1B, "If-Unmodified-Since"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x1C, "Location"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x1D, "Last-Modified"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x1E, "Max-Forwards"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x1F, "Pragma"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x20, "Proxy-Authenticate"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x21, "Proxy-Authorization"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x22, "Public"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x23, "Range"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x24, "Referer"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x25, "Retry-After"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x26, "Server"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x27, "Transfer-Encoding"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x28, "Upgrade"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x29, "User-Agent"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x2A, "Vary"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x2B, "Via"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x2C, "Warning"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x2D, "WWW-Authenticate"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x2E, "Content-Disposition"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x2F, "X-Wap-Application-Id"));

    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x30, "X-Wap-Content-URI"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x31, "X-Wap-Initiator-URI"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x32, "Accept-Application"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x33, "Bearer-Indication"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x34, "Push-Flag"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x35, "Profile"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x36, "Profile-Diff"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x37, "Profile-Warning"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x38, "Expect"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x39, "TE"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x3A, "Trailer"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x3B, "Accept-Charset"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x3C, "Accept-Encoding"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x3D, "Cache-Control"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x3E, "Content-Range"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x3F, "X-Wap-Tod"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x40, "Content-ID"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x41, "Set-Cookie"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x42, "Cookie"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x43, "Encoding-Version"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x44, "Profile-Warning"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x45, "Content-Disposition"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x46, "X-WAP-Security"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x47, "Cache-Control"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x48, "Expect"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x49, "X-Wap-Loc-Invocation"));
    WELL_KNOWN_HEADERS.insert(pair<int, string>(0x4A, "X-Wap-Loc-Delivery"));
}

WappushPduParser::WappushPduParser(BYTE* pdu, int length) :
        mWspData(pdu),
        mDataLength(-1),
        mPduLength(length),
        mUnsigned32bit(-1),
        mStringValue("") {
}

bool WappushPduParser::decodeUintvarInteger(int startIndex) {
    int index = startIndex;

    mUnsigned32bit = 0;
    while ((mWspData[index] & 0x80) != 0) {
        if ((index - startIndex) >= 4) {
            return false;
        }
        mUnsigned32bit = (mUnsigned32bit << 7) | (mWspData[index] & 0x7f);
        index++;
    }
    mUnsigned32bit = (mUnsigned32bit << 7) | (mWspData[index] & 0x7f);
    mDataLength = index - startIndex + 1;
    return true;
}

long WappushPduParser::getValue32() {
    return mUnsigned32bit;
}

int WappushPduParser::getDecodedDataLength() {
    return mDataLength;
}

bool WappushPduParser::decodeContentType(int startIndex) {
    int mediaPrefixLength;

    if (decodeValueLength(startIndex) == false) {
        bool found = decodeConstrainedEncoding(startIndex);
        if (found) {
            expandWellKnownMimeType();
        }
        return found;
    }
    int headersLength = (int) mUnsigned32bit;
    mediaPrefixLength = getDecodedDataLength();
    if (decodeIntegerValue(startIndex + mediaPrefixLength) == true) {
        mDataLength += mediaPrefixLength;
        int readLength = mDataLength;
        mStringValue = "";
        expandWellKnownMimeType();
        long wellKnownValue = mUnsigned32bit;
        string mimeType = mStringValue;
        if (readContentParameters(startIndex + mDataLength,
                (headersLength - (mDataLength - mediaPrefixLength)), 0)) {
            mDataLength += readLength;
            mUnsigned32bit = wellKnownValue;
            mStringValue = mimeType;
            return true;
        }
        return false;
    }
    if (decodeExtensionMedia(startIndex + mediaPrefixLength) == true) {
        mDataLength += mediaPrefixLength;
        int readLength = mDataLength;
        expandWellKnownMimeType();
        long wellKnownValue = mUnsigned32bit;
        string mimeType = mStringValue;
        if (readContentParameters(startIndex + mDataLength,
                (headersLength - (mDataLength - mediaPrefixLength)), 0)) {
            mDataLength += readLength;
            mUnsigned32bit = wellKnownValue;
            mStringValue = mimeType;
            return true;
        }
    }
    return false;
}

bool WappushPduParser::decodeValueLength(int startIndex) {
    if ((mWspData[startIndex] & 0xff) > WappushPduParser::WAP_PDU_LENGTH_QUOTE) {
        return false;
    }
    if (mWspData[startIndex] < WappushPduParser::WAP_PDU_LENGTH_QUOTE) {
        mUnsigned32bit = mWspData[startIndex];
        mDataLength = 1;
    } else {
        if (!decodeUintvarInteger(startIndex + 1)) {
            return false;
        }
        mDataLength++;
    }
    return true;
}

bool WappushPduParser::decodeConstrainedEncoding(int startIndex) {
    if (decodeShortInteger(startIndex) == true) {
        mStringValue = "";
        return true;
    }
    return decodeExtensionMedia(startIndex);
}

bool WappushPduParser::decodeExtensionMedia(int startIndex) {
    int index = startIndex;
    mDataLength = 0;
    mStringValue = "";
    int length = mPduLength;
    bool rtrn = index < length;

    while (index < length && mWspData[index] != 0) {
        index++;
    }

    mDataLength = index - startIndex + 1;
    mStringValue = PhoneNumberUtils::stringFromBytes(mWspData, startIndex, mDataLength - 1);
    return rtrn;
}

void WappushPduParser::expandWellKnownMimeType() {
    if (mStringValue.empty()) {
        int binaryContentType = (int) mUnsigned32bit;
        mStringValue = WELL_KNOWN_MIME_TYPES[binaryContentType];
    } else {
        mUnsigned32bit = -1;
    }
}

bool WappushPduParser::decodeIntegerValue(int startIndex) {
    if (decodeShortInteger(startIndex) == true) {
        return true;
    }
    return decodeLongInteger(startIndex);
}

bool WappushPduParser::decodeShortInteger(int startIndex) {
    if ((mWspData[startIndex] & 0x80) == 0) {
        return false;
    }
    mUnsigned32bit = mWspData[startIndex] & 0x7f;
    mDataLength = 1;
    return true;
}

bool WappushPduParser::decodeLongInteger(int startIndex) {
    int lengthMultiOctet = mWspData[startIndex] & 0xff;

    if (lengthMultiOctet > WAP_PDU_SHORT_LENGTH_MAX) {
        return false;
    }
    mUnsigned32bit = 0;
    for (int i = 1; i <= lengthMultiOctet; i++) {
        mUnsigned32bit = (mUnsigned32bit << 8) | (mWspData[startIndex + i] & 0xff);
    }
    mDataLength = 1 + lengthMultiOctet;
    return true;
}

bool WappushPduParser::readContentParameters(int startIndex, int leftToRead, int accumulator) {
    int totalRead = 0;
    if (leftToRead > 0) {
        BYTE nextByte = mWspData[startIndex];
        string value = "";
        string param = "";
        if ((nextByte & 0x80) == 0x00 && nextByte > 31) {  // untyped
            decodeTokenText(startIndex);
            param = mStringValue;
            totalRead += mDataLength;
        } else {  // typed
                if (decodeIntegerValue(startIndex)) {
                    totalRead += mDataLength;
                    int wellKnownParameterValue = (int) mUnsigned32bit;
                    param = WELL_KNOWN_PARAMETERS[wellKnownParameterValue];
                    string hexStr = "";
                    if (param.empty()) {
                        param = "unassigned/0x" + numToHexString(wellKnownParameterValue);
                    }
                    // special case for the "Q" parameter, value is a uintvar
                    if (wellKnownParameterValue == Q_VALUE) {
                        if (decodeUintvarInteger(startIndex + totalRead)) {
                            totalRead += mDataLength;
                            value = numToString(mUnsigned32bit);
                            mContentParameters.insert(pair<string, string>(param, value));
                            return readContentParameters(startIndex + totalRead, leftToRead
                                    - totalRead, accumulator + totalRead);
                        } else {
                            return false;
                        }
                    }
                } else {
                    return false;
                }
            }

            if (decodeNoValue(startIndex + totalRead)) {
                totalRead += mDataLength;
                value = "";
            } else if (decodeIntegerValue(startIndex + totalRead)) {
                totalRead += mDataLength;
                int intValue = (int) mUnsigned32bit;
                value = numToString(intValue);
            } else {
                decodeTokenText(startIndex + totalRead);
                totalRead += mDataLength;
                value = mStringValue;
                if (value.at(0) == '\"') {
                    // quoted string, so remove the quote
                    value = value.substr(1, value.length() - 1);
                }
            }
            mContentParameters.insert(pair<string, string>(param, value));
            return readContentParameters(startIndex + totalRead, leftToRead - totalRead,
                    accumulator + totalRead);
        } else {
            mDataLength = accumulator;
            return true;
        }
    }

bool WappushPduParser::decodeTokenText(int startIndex) {
    int index = startIndex;
    while (mWspData[index] != 0) {
        index++;
    }
    mDataLength = index - startIndex + 1;
    mStringValue = PhoneNumberUtils::stringFromBytes(mWspData, startIndex, mDataLength - 1);
    return true;
}

string WappushPduParser::numToHexString(int number) {
    char temp[128];
    sprintf(temp, "%X", number);
    string ret = temp;
    return ret;
}

string WappushPduParser::numToString(int number) {
    char temp[128];
    sprintf(temp, "%d", number);
    string ret = temp;
    return ret;
}

bool WappushPduParser::decodeNoValue(int startIndex) {
    if (mWspData[startIndex] == 0) {
        mDataLength = 1;
        return true;
    } else {
        return false;
    }
}

string WappushPduParser::getValueString() {
    return mStringValue;
}

void WappushPduParser::decodeHeaders(int startIndex, int headerLength) {
    string headerName = "";
    string headerValue = "";
    int intValues;

    int index = startIndex;
    while (index < startIndex + headerLength) {
        decodeHeaderFieldName(index);
        index += getDecodedDataLength();
        expandWellKnownHeadersName();
        intValues = (int) mUnsigned32bit;
        if (!mStringValue.empty()) {
            headerName = mStringValue;
        } else if (intValues >= 0) {
            headerName = numToString(intValues);
        } else {
            continue;
        }

        decodeHeaderFieldValues(index);
        index += getDecodedDataLength();
        intValues = (int) mUnsigned32bit;
        if (!mStringValue.empty()) {
            headerValue = mStringValue;
        } else if (intValues >= 0) {
            headerValue = numToString(intValues);
        } else {
            continue;
        }

        mHeaders.insert(pair<string, string>(headerName, headerValue));
    }
}

bool WappushPduParser::decodeHeaderFieldName(int startIndex) {
    if (decodeShortInteger(startIndex) == true) {
        mStringValue = "";
        return true;
    } else {
        return decodeTextString(startIndex);
    }
}

bool WappushPduParser::decodeTextString(int startIndex) {
    int index = startIndex;
    while (mWspData[index] != 0) {
        index++;
    }
    mDataLength = index - startIndex + 1;
    if (mWspData[startIndex] == 127) {
        mStringValue = PhoneNumberUtils::stringFromBytes(mWspData, startIndex + 1, mDataLength - 2);
    } else {
        mStringValue = PhoneNumberUtils::stringFromBytes(mWspData, startIndex, mDataLength - 1);
    }
    return true;
}

void WappushPduParser::expandWellKnownHeadersName() {
    if (mStringValue.empty()) {
        int binaryHeadersName = (int) mUnsigned32bit;
        mStringValue = WELL_KNOWN_HEADERS[binaryHeadersName];
    } else {
        mUnsigned32bit = -1;
    }
}

bool WappushPduParser::decodeHeaderFieldValues(int startIndex) {
    BYTE first = mWspData[startIndex];
    if ((first == WappushPduParser::WAP_PDU_LENGTH_QUOTE) &&
            decodeUintvarInteger(startIndex + 1)) {
        mStringValue = "";
        mDataLength++;
        return true;
    }

    if (decodeIntegerValue(startIndex) == true) {
        mStringValue = "";
        return true;
    }
    return decodeTextString(startIndex);
}

bool WappushPduParser::seekXWapApplicationId(int startIndex, int endIndex) {
    int index = startIndex;
    for (index = startIndex; index <= endIndex; ) {
        /**
         * 8.4.1.1  Field name
         * Field name is integer or text.
         */
        if (decodeIntegerValue(index)) {
            int fieldValue = (int) getValue32();

            if (fieldValue == PARAMETER_ID_X_WAP_APPLICATION_ID) {
                mUnsigned32bit = index + 1;
                return true;
            }
        } else {
            if (!decodeTextString(index)) {
                return false;
            }
        }
        index += getDecodedDataLength();
        if (index > endIndex) {
            return false;
        }

        /**
         * 8.4.1.2 Field values
         * Value Interpretation of First Octet
         * 0 - 30 This octet is followed by the indicated number (0 - 30)
                of data octets
         * 31 This octet is followed by a uintvar, which indicates the number
         *      of data octets after it
         * 32 - 127 The value is a text string, terminated by a zero octet
                (NUL character)
         * 128 - 255 It is an encoded 7-bit value; this header has no more data
         */
        BYTE val = mWspData[index];
        if (val <= WAP_PDU_SHORT_LENGTH_MAX) {
            index += mWspData[index] + 1;
        } else if (val == WAP_PDU_LENGTH_QUOTE) {
            if (index + 1 >= endIndex) {
                return false;
            }
            index++;
            if (!decodeUintvarInteger(index)) {
                return false;
            }
            index += getDecodedDataLength();
        } else if (WAP_PDU_LENGTH_QUOTE < val && val <= 127) {
            if (!decodeTextString(index)) {
                return false;
            }
            index += getDecodedDataLength();
        } else {
            index++;
        }
    }
    return false;
}

bool WappushPduParser::decodeXWapApplicationId(int startIndex) {
    if (decodeIntegerValue(startIndex) == true) {
        mStringValue = "";
        return true;
    }
    return decodeTextString(startIndex);
}
