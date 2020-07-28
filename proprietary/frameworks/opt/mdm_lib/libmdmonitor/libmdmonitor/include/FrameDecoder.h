/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
/**
 * @file FrameDecoder.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c FrameDecoder class used to decode DHL frame to related frame information.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __FRAME_DECODER_H__
#define __FRAME_DECODER_H__
#include <string>
#include <map>
#include <vector>
#include <set>
#include "libmdmonitor.h"
#include "LogCache.h"
namespace libmdmonitor {

typedef enum {
    R8_GPD_DATA = 0,
    R8_BD_DATA = 1,
    R8_RAW_DATA = 2
}R8_DATA_TYPE;

typedef enum {
    PS_MAC_MAJOR = 0,
    PS_MAC_MINOR = 1,
    PS_MAC_RELEASE = 2,
    PS_RLC_MAJOR = 3,
    PS_RLC_MINOR = 4,
    PS_RLC_RELEASE = 5,
    PS_PDCP_MAJOR = 6,
    PS_PDCP_MINOR = 7,
    PS_PDCP_RELEASE = 8,
    PS_RRC_MAJOR = 9,
    PS_RRC_MINOR = 10,
    PS_RRC_RELEASE = 11,
    PS_NAS_MAJOR = 12,
    PS_NAS_MINOR = 13,
    PS_NAS_RELEASE = 14
}PROTOCOL_STACK_VERSION_TYPE;


/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to decode DHL frame to related frame information.
 */
class FrameDecoder
{
public:
    /**
     * @brief The parsing information within an indexed trace or primitive.
     */
    struct FIELD_INFO
    {
        /**
         * @brief Constructor
         */
        FIELD_INFO(): bLoaded(false), offset(0), size(0) {}

        /**
         * @brief Indicate whether the parsing information of the field were loaded from layout description file.
         */
        bool bLoaded;

        /**
         * @brief The offset of the field in the raw buffer.
         */
        size_t offset;

        /**
         * @brief The size in bytes of the field in the raw buffer.
         */
        size_t size;
    };
    /**
     * @brief The parsing information within an indexed trace or primitive.
     */
    struct SIZE_INFO
    {
        /**
         * @brief Constructor
         */
        SIZE_INFO(): bLoaded(false), size(0) {}

        /**
         * @brief Indicate whether the parsing information of the field were loaded from layout description file.
         */
        bool bLoaded;

        /**
         * @brief The size in bytes of the field in the raw buffer.
         */
        size_t size;
    };

    /**
     * @brief The parsing information for pure value. (not field, size)
     */
    struct VALUE_INFO
    {
        /**
         * @brief Constructor
         */
        VALUE_INFO(): bLoaded(false), value(0) {}

        /**
         * @brief Indicate whether the parsing information of the value were loaded from layout description file.
         */
        bool bLoaded;

        /**
         * @brief The value in layout file.
         */
        uint64_t value;
    };

    /**
     * @brief Trace parsing information which contains OTA message offset, trace name string, and a set of fields.
     */
    struct FIELD_TRACE
    {
        /**
         * @brief The offset of OTA message in the trace buffer.
         */
        size_t otaMsgOffset;

        /**
         * @brief The name string of the trace.
         */
        std::string strName;

        /**
         * @brief A set of fields in the trace.
         */
        std::map<std::string, FIELD_INFO> fieldInfo;
    };

    /**
     * @brief OTA parsing information which contains its own name string.
     */
    struct FIELD_OTA
    {
        /**
         * @brief The name string of the OTA message.
         */
        std::string strName;
    };

    /**
     * @brief The information to parse a primitive (containing EM information), consists of the name string of the primitive, and a set of fields.
     */
    struct FIELD_EM
    {
        /**
         * @brief The name string of the primitive.
         */
        std::string strName;

        /**
         * @brief A set of fields in the primitive.
         */
        std::map<std::string, FIELD_INFO> fieldInfo;

        /**
         * @brief The offset of peer buffer.
         */
        size_t peerBufferOffset;
        /**
         * @brief The bool value of peer buffer has header.
         */
        bool hasPeerBufferHeader;
    };

    /**
     * @brief IP parsing information which contains its own name string.
     */
    struct FIELD_IPTrace
    {
        /**
         * @brief The name string of the IP trace ID.
         */
        std::string strName;
    };

    //////////// Modem Information  (MD_XXX structure)  ////////////

    /**
     * @brief Modem 3GPP Version Structure
     */
    struct MD_3GPPVERSION
    {
        /**
         * @brief The 3GPP Major Version
         */
        VALUE_INFO majorVersion;

        /**
         * @brief The 3GPP Minor Version
         */
        VALUE_INFO minorVersion;

        /**
         * @brief The 3GPP Release Version
         */
        VALUE_INFO releaseVersion;
    };



    /**
     * @brief Modem Protocol Version Information
     */
    struct MD_ProtocolVersionInfo
    {
        /**
         * @brief The RRC Version
         */
        MD_3GPPVERSION rrcVersion;

        /**
         * @brief The NAS Version
         */
        MD_3GPPVERSION nasVersion;

        /**
         * @brief The MAC Version
         */
        MD_3GPPVERSION macVersion;

        /**
         * @brief The RLC Version
         */
        MD_3GPPVERSION rlcVersion;

        /**
         * @brief The PDCP Version
         */
        MD_3GPPVERSION pdcpVersion;
    };

    /**
     * @brief Modem Time Information
     */
    struct MD_TIMEINFO
    {
        /**
         * @brief The time precision in modem.
         */
        VALUE_INFO timeStampPrecision;
    };

    /**
     * @brief Modem EM filter Information
     */
    struct MD_EMFILTERINFO
    {
        /**
         * @brief The field size in filter (em_filter.bin) for each EM
         */
        VALUE_INFO emFilterFieldLength;

        /**
         * @brief The total EM number in filter (em_filter.bin).
         */
        VALUE_INFO emFilterTotalEMNumber;

        /**
         * @brief The filter offset (in em_filter.bin) for SIM 1~4
         */
        VALUE_INFO emFilterOffset[4];

    };
    /**
     * @brief FrameDecoder destructor.
     */
    virtual ~FrameDecoder() {}

    /**
     * @brief Parse a DHL frame to obtain the frame information.
     *
     * @param dhlFrameType [IN] The DHL frame type. Should be DHL_PRIMITIVE or DHL_INDEX_TRACE.
     * @param pData [IN] The DHL frame buffer. Should not be NULL or false will return.
     * @param len [IN] The length of DHL frame. Should not be 0 or false will return.
     * @param frameInfo [OUT] The obtained frame information.
     *
     * @return true: The frame was successfully parsed. <br>
     *        false: Error.
     */
    bool ParseFrameInfo(unsigned char dhlFrameType, uint64_t rtostime, const unsigned char *pData, size_t len, FRAME_INFO &frameInfo);

    /**
     * @brief Parse a trap to obtain the frame information.
     *
     * @param trapType [IN] The trap type. Should be TRAP_TYPE_OTA, TRAP_TYPE_EM or TRAP_TYPE_VOLTE.
     * @param pData [IN] The DHL frame buffer. Should not be NULL or false will return.
     * @param len [IN] The length of DHL frame. Should not be 0 or false will return.
     * @param frameInfo [OUT] The obtained frame information.
     *
     * @return true: The frame was successfully parsed. <br>
     *        false: Error.
     */
    bool ParseTrapInfo(TRAP_TYPE trapType, uint64_t rtostime, const unsigned char *pData, size_t len, FRAME_INFO &frameInfo);

    /**
     * @brief Whether the information in layout description file support Dynamic EM Filter (DEF) feature.
     *
     * @return true: The information in layout description file is supported DEF. <br>
     *        false: The information in layout description file is not supported DEF.
     */
    bool IsSupportedDEF() const ;

    /**
     * @brief Given the primitive message ID, get the name string of the primitve. <br>
     *        Given the OTA global ID, get the name string of the OTA message.
     *
     * @param type [IN] The type of @a msgID. Can be TRAP_TYPE_OTA, TRAP_TYPE_EM, and TRAP_TYPE_VOLTE.
     * @param msgID [IN] Primitive message ID or OTA global ID.
     *
     * @return NULL: @a type or @a msgID does not exist.<br>
     *        !NULL: The name string of primitive or OTA message.
     */
    const char *GetMsgString(TRAP_TYPE type, MSGID msgID) const;

    bool GetTraceIDByString(const char *szTraceName, TRACEID &traceID) const;
    bool GetMsgByString(const char *szMsgName, TRAP_TYPE &trapType, MSGID &msgID) const;
    bool GetEMIDByMsgID(MSGID msgID, EMID &emID) const;

    bool GetTraceFieldInfoByID(TRACEID traceId, const char *szFieldName, size_t &offset, size_t &size) const;
    bool GetTraceFieldVal(TRACEID traceId, const unsigned char *pData, size_t len, const char *szFieldName, bool bSignedVal, int64_t &val) const;
    const unsigned char *GetTraceFieldBuf(TRACEID traceId, const unsigned char *pData, size_t len, const char *szFieldName, size_t elementSize, size_t &retBufSize) const;
    const unsigned char *GetOTARawData(size_t &size, TRACEID traceId, const unsigned char *pData, size_t len) const;

    bool GetEMFieldInfoByID(MSGID msgId, const char *szFieldName, size_t &offset, size_t &size) const;
    bool GetEMFieldVal(MSGID msgId, const unsigned char *pData, size_t len, const char *szFieldName, bool bSignedVal, int64_t &val) const;
    const unsigned char *GetEMFieldBuf(MSGID msgId, const unsigned char *pData, size_t len, const char *szFieldName, size_t elementSize, size_t &retBufSize) const;
    const unsigned char *GetMsgPeerBufferData(size_t &size, MSGID msgId, const unsigned char *pData, size_t len) const;
    static bool SearchLayoutFile(const char *szSearchDir, char *buffer, size_t bufferSize);
    bool GetIPDataWithDataTypeAndIndex(MSGID msgId, R8_DATA_TYPE type, int raw_index, const unsigned char *pData, size_t len, unsigned char **pRetData, size_t &retDataLen) const;
    bool MDInfo_GetProtocolVersion(PROTOCOL_STACK_VERSION_TYPE protocolType, int &version) const;
    int MDInfo_GetTimePrecision(uint64_t &precision);
    int MDInfo_GetEMFilterFeildLength(uint64_t &length);
    int MDInfo_GetEMFilterTotalEMNumber(uint64_t &number);
    int MDInfo_GetEMFilterOffset(unsigned int simIndex, uint64_t &offset);
    /**
     * @brief Get a FrameDecoder instance which will load the layout description file from @a szLayoutDescPath
     *
     * @param szLayoutDescPath [IN] !NULL: The path of the layout description file. <br>
     *                              NULL: FrameDecoder will try to load layout description file from layout desc file.<br>
     *                              which is "/etc/mddb/mdm_layout_desc.dat" defined in MonitorDef.h.
     */
    static FrameDecoder* GetInstance(const char *szLayoutDescPath = NULL);

protected:

    /**
     * @brief Create a FrameDecoder and load the layout description file from @a szLayoutDescPath
     *
     * @param szLayoutDescPath [IN] !NULL: The path of the layout description file. <br>
     *                              NULL: FrameDecoder will try to load layout description file from layout desc file.<br>
     *                              which is "/etc/mddb/mdm_layout_desc.dat" defined in MonitorDef.h.
     */
    FrameDecoder(const char *szLayoutDescPath = NULL);
    FrameDecoder(const FrameDecoder& other);
    FrameDecoder& operator=(const FrameDecoder& other);

    static std::map<std::string, FrameDecoder*> sDecoderInstances;
    static pthread_mutex_t sDecoderLock;

    /**
     * @brief The offset and size of TraceID in trace buffer.
     */
    FIELD_INFO m_otaTraceID;

    /**
     * @brief The offset and size of SIM index in trace buffer.
     */
    FIELD_INFO m_otaSIMIdx;

    /**
     * @brief The offset and size of primitive ID in primitive buffer.
     */
    FIELD_INFO m_msgID;

    /**
     * @brief The offset and size of SIM index in primitive buffer.
     */
    FIELD_INFO m_msgSIMIdx;

    /**
     * @brief The offset and size of Peer Buffer in primitive peer buffer.
     */
    FIELD_INFO m_msgPeerBufferSize;

    /**
     * @brief The size of peer buffer header.
     */
    SIZE_INFO m_msgPeerBufferHeaderSize;

    /**
     * @brief Mapping Trace ID to its own parsing information.
     *
     * Ex: TraceID:200278199 -> {21, SIBE_PEER_MSG_RRC_SI}
     */
    std::map<TRACEID, FIELD_TRACE> m_mapTrace;

    std::map<std::string, TRACEID> m_mapTraceStrToId;

    /**
     * @brief Mapping OTA Global ID to its own parsing information.
     *
     * Ex: GlobalID:100 -> PROTOCOL_SECTION__GAN_TCP
     */
    std::map<MSGID, FIELD_OTA> m_mapOTA;

    /**
     * @brief Mapping OTA name string to its own Global ID.
     */
	std::map<std::string, MSGID> m_mapOTAStrToId;

    /**
     * @brief Mapping primitive message ID of EM to its own parsing information.
     *
     * Ex: MSGID:15907 -> MSG_ID_EM_CSCE_SERV_CELL_S_STATUS_IND
     */
    std::map<MSGID, FIELD_EM> m_mapEM;

    /**
     * @brief Mapping primitive name string of EM to its own message ID.
     */
	std::map<std::string, MSGID> m_mapEMStrToId;
    /**
     * @brief Mapping primitive message ID of EM to its EM ID.
     *
     * Ex: MSGID : 16583 (MSG_ID_EM_L4C_RAT_CHANGE_IND) -> EMID : 275
     */
    std::map<MSGID, EMID> m_mapEMID;

    /**
     * @brief Mapping IP trace ID to its own parsing information.
     *
     * Ex: trace ID : 813694977 -> UPCM_UL_RAW_DATA
     */
    std::map<TRACEID, FIELD_IPTrace> m_mapIPTrace;

    /**
     * @brief Mapping IP trace name string to its own Global ID.
     */
	std::map<std::string, TRACEID> m_mapIPTraceStrToId;

    /**
     * @brief The SIM info for each SIM index.
     *
     * When the value of SIM index in the raw buffer was fetched, e.g., 186, <br>
     * it is necessary to lookup the table of SIM2, SIM3, or ... to check whether the value, <br>
     * e.g., 186 is in any of them. So that we can determine the frame is from SIM2, SIM3, ..., or otherwise, from SIM1.
     */
    std::vector<std::set<uint64_t> > m_vecSimInfo;
    ////// Modem Information //////
    /**
     * @brief Modem protocol version information
     */
    MD_ProtocolVersionInfo m_modemProtocolVersionInfo;

    /**
     * @brief Modem time information
     */
    MD_TIMEINFO m_modemTimeInfo;

    /**
     * @brief Modem EM filter (em_filter.bin) information.
     */
    MD_EMFILTERINFO m_modemEMFilterInfo;
    /**
     * @brief DHL msg debug log cache for reduce log number.
     */
    LogCache m_logCache;

    /**
     * @brief Prototype of parse info collector for layout description file.
     *
     * Each line in layout description file will be divided into several tokens by comma.<br>
     * Then, the array of tokens will be passed into the callback function for information collection.
     *
     * @param pToken [IN] The array of tokens. Will not be NULL.
     * @param nTokenCount [IN] The number of tokens. The value is greater than 0.
     * @param pDecoder [IN] The pointer to @c FrameDecoder instance. Will not be NULL.
     * @param collectorParam [IN] The parameter for collector. May be NULL.
     *
     * @return true: Success.<br>
     *        false: Error.
     */
    typedef bool (*InfoCollector)(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

    /**
     * @brief The information for parsing sections in layout description file.
     */
    struct SectionParsingInfo {
        /**
         * @brief The pattern to indicate the begining of section.
         */
        const char *szStrPatten;

        /**
         * @brief Information collector.
         */
        InfoCollector collector;

        /**
         * @brief Parameters for info collector.
         */
        void *collectorParam;
    };

    /**
     * @brief Table of information about parsing sections.
     */
    static SectionParsingInfo m_sectionParsingInfo[];

    /**
     * @brief Collector for OTA info.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify.
     * @param nTokenCount [IN] The number of tokens.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool OTAInfoCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

    /**
     * @brief Collector for OTA Global name string and ID.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool GlobalIDCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

    /**
     * @brief Collector for EM related primitive information.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool MsgInfoCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

    /**
     * @brief Collector for SIM information.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool SIMInfoCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

    /**
     * @brief Collector for IP packet trace ID.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool IPInfoTraceIDCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

     /**
     * @brief Collector for peer buffer offset of primitive information.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool PeerInfoCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);
    /**
     * @brief Collector for Peer buffer info.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool PeerInfoWithoutHeaderCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);
    /**
     * @brief Collector for Mapping table between msg id and EM id.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool MsgEMIDMappingCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

    /**
     * @brief Collector for EM filter info.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool EMFilterInfoCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

    /**
     * @brief Collector for modem time info.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool TimeInfoCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

    /**
     * @brief Collector for modem protocol stack version info.
     *
     * @param pToken[] [IN] Array of tokens. Allow to modify. Should not be NULL or false will return.
     * @param nTokenCount [IN] The number of tokens. Should not be 0 or false will return.
     * @param pDecoder [IN] The pointer to the frame decoder for storing the information.<br>
     *                      Should not be NULL or false will return.
     * @param collectorParam [IN] The parameters for the collector.
     *
     * @return true: The tokens were successfully analyzed and the information were stored in @a pDecoder.<br>
     *        false: Errors.
     */
    static bool ProtocolVersionInfoCollector(char *pToken[], size_t nTokenCount, FrameDecoder *pDecoder, void *collectorParam);

    /**
     * @brief Extract tokens from a line in layout description file by comma.
     *
     * @param szLine [IN] The line in layout description file to be processed. Should not be NULL or false will return.
     * @param pToken[] [OUT] The extracted tokens. Should not be NULL or false will return.
     * @param nTokenCount [OUT] The number of tokens extracted. May be 0.
     *
     * @return true: Success.<br>
     *        false: Bad parameters.
     */
    bool ExtractTokens(char *szLine, char *pToken[], size_t &nTokenCount);

    /**
     * @brief Lookup @a m_sectionParsingInfo table to find the section parsing info whose pattern matchs @a szSectionPattern.
     *
     * @param szSectionPattern [IN] The pattern of section to lookup. Should not be NULL or false will return.
     *
     * @return NULL: No section parsing info found.<br>
     *        !NULL: The pointer to the entry of section parsing info.
     */
    SectionParsingInfo *GetSectionParsingInfo(const char *szSectionPattern);

    /**
     * @brief Load layout description file from a given path.
     *
     * @param szPath [IN] The path of layout description file. Should not be NULL or false will return.
     *
     * @return true: The layout description file was successfully loaded and parsed.<br>
     *        false: Errors.
     */
    bool LoadFromDESC(const char *szPath);

    /**
     * @brief Get the value from raw data buffer @a pData.
     *
     * @param pData [IN] The data buffer. Should not be NULL or false will return.
     * @param len [IN] The length of data buffer. The length should be greater than @a nBytes or false will return.
     * @param nBytes [IN] The size of value in bytes to fetch. Should be one of 1/2/4/8 or false will return.
     * @param bSignedVal [IN] true: The fetched value is signed.<br>
     *                       false: The fetched value is unsigned.
     * @param val [OUT] The fetched value.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool GetValueFromBuffer(const unsigned char *pData, size_t len, size_t nBytes, bool bSignedVal, int64_t &val) const;

    /**
     * @brief A wrapper of @b GetValueFromBuffer to get trace ID from trace data buffer.
     *
     * @param pData [IN] The trace data buffer. Should not be NULL or false will return.
     * @param len [IN] The length of trace data buffer. Should not be 0 or false will return.
     * @param val [OUT] The trace ID.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool GetOTATraceID(const unsigned char *pData, size_t len, TRACEID &val) const;

    /**
     * @brief A wrapper of @b GetValueFromBuffer to get the value at SIM Idx from trace data buffer
     *
     * @param pData [IN] The trace data buffer. Should not be NULL or false will return.
     * @param len [IN] The length of trace data buffer. Should not be 0 or false will return.
     * @param val [OUT] The value at SIM Idx in trace data buffer.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool GetOTASIMIdx(const unsigned char *pData, size_t len, uint32_t &val) const;

    /**
     * @brief A wrapper of @b GetValueFromBuffer to get primitive message ID from primitive data buffer
     *
     * @param pData [IN] The primitive data buffer. Should not be NULL or false will return.
     * @param len [IN] The length of primitive data buffer. Should not be 0 or false will return.
     * @param val [OUT] The primitive message ID.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool GetMsgID(const unsigned char *pData, size_t len, MSGID &val) const;

    /**
     * @brief A wrapper of @b GetValueFromBuffer to get the value at SIM Idx from primitive data buffer
     *
     * @param pData [IN] The primitive data buffer. Should not be NULL or false will return.
     * @param len [IN] The length of primitive data buffer. Should not be 0 or false will return.
     * @param val [OUT] The value at SIM Idx in primitive data buffer.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool GetMsgSIMIdx(const unsigned char *pData, size_t len, uint32_t &val) const;

    /**
     * @brief A wrapper of @b GetValueFromBuffer to get the value at peer buffer size from primitive data buffer
     *
     * @param msgID [IN] Primitive message ID or OTA global ID..
     * @param pData [IN] The primitive data buffer. Should not be NULL or false will return.
     * @param len [IN] The length of primitive data buffer. Should not be 0 or false will return.
     * @param val [OUT] The value at peer buffer size in primitive data buffer.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool GetMsgPeerBufSize(MSGID msgId, const unsigned char *pData, size_t len, uint32_t &val) const;


    /**
     * @brief Get OTA global ID from trace data buffer.
     *
     * @param traceID [IN] The trace ID.
     * @param pData [IN] The trace data buffer. Should not be NULL or false will return.
     * @param len [IN] The length of trace data buffer. Should not be 0 or false will return.
     * @param globalID [OUT] The OTA global ID.
     * @param bSearchMagic [IN] true: Search OTA header in trace data buffer to find OTA offset.<br>
     *                         false: Use the OTA offset defined in layout description file.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool GetOTAGlobalID(TRACEID traceID, const unsigned char *pData, size_t len, MSGID &globalID, bool bSearchMagic = false) const;
    bool GetOTAOffset(TRACEID traceID, const unsigned char *pData, size_t len, uint64_t &otaMsgOffset, bool bSearchMagic) const;
    bool GetMsgPeerBufferOffset(MSGID msgId, uint64_t &peerBufferOffset) const;
};

} //namespace libmdmonitor {
#endif
