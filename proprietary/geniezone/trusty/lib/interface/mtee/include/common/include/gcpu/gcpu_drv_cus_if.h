/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

// *********************************************************************
// Memo
// *********************************************************************
/*
*/

#ifndef _GCPU_DRV_IF_H_
#define _GCPU_DRV_IF_H_

//#include "platform/mt_typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef UINT16 GCPU_SLOT_HANDLE_T;
typedef UINT8  GCPU_INST_HANDLE_T;


#define AES_CMD_SID         (0x02 << 8)
#define MISC_CMD_SID        (0x08 << 8)
#define TDES_CMD_SID        (0x05 << 8)

#define GCPU_AES_D                          ((UINT32)(AES_CMD_SID + 0x00))         ///< AES Decryption Function, parameter struct is AES_Param_DEG
#define GCPU_AES_E                          ((UINT32)(AES_CMD_SID + 0x01))         ///< AES Encryption Function, parameter struct is AES_Param_DEG
#define GCPU_AES_G                          ((UINT32)(AES_CMD_SID + 0x02))         ///< AES Generating Function, parameter struct is AES_Param_DEG
#define GCPU_AES_DPAK                       ((UINT32)(AES_CMD_SID + 0x03))         ///< AES Packet Decryption, parameter struct is AES_Param_PAK
#define GCPU_AES_EPAK                       ((UINT32)(AES_CMD_SID + 0x04))         ///< AES Packet Encryption, parameter struct is AES_Param_PAK
#define GCPU_AES_CMAC                       ((UINT32)(AES_CMD_SID + 0x05))         ///< AES CMAC Algorithm, parameter struct is AES_Param_DEG
#define GCPU_AES_DCBC                       ((UINT32)(AES_CMD_SID + 0x06))         ///< AES Cipher Block Chaining Decryption, parameter struct is AES_Param_CBC
#define GCPU_AES_ECBC                       ((UINT32)(AES_CMD_SID + 0x07))         ///< AES Cipher Block Chaining Encryption, parameter struct is AES_Param_CBC
#define GCPU_AES_CTR                        ((UINT32)(AES_CMD_SID + 0x15))         ///< AES Counter Mode Decryption Function, parameter struct is AES_Param_CTR

#define GCPU_MEM_XOR                        ((UINT32)(MISC_CMD_SID + 0x05))        ///< Memory XOR, parameter struct is MEM_Param_XOR
#define GCPU_SHA_1                          ((UINT32)(MISC_CMD_SID + 0x02))        ///< SHA-1 Algorithm, parameter struct is SHA1_Param
#define GCPU_MD5                            ((UINT32)(MISC_CMD_SID + 0x03))         ///< MD5 Algorithm, parameter struct is MD5_Param
#define GCPU_SHA_256                        ((UINT32)(MISC_CMD_SID + 0X0D))        ///< SHA-256 Algorithm, parameter struct is SHA2_Param
#define GCPU_HMAC                           ((UINT32)(MISC_CMD_SID + 0X0E))        ///< HMAC Algorithm, parameter struct is HMAC_Param

#define GCPU_TDES_D                         ((UINT32)(TDES_CMD_SID + 0x00))         ///< T-DES Decryption, parameter struct is TDES_Param_DE
#define GCPU_TDES_E                         ((UINT32)(TDES_CMD_SID + 0x01))         ///< T-DES Encryption, parameter struct is TDES_Param_DE
#define GCPU_TDES_DMA_D                     ((UINT32)(TDES_CMD_SID + 0x02))         ///< T-DES DMA Decryption, parameter struct is TDES_Param_DMA_DE
#define GCPU_TDES_DMA_E                     ((UINT32)(TDES_CMD_SID + 0x03))         ///< T-DES DMA Encryption, parameter struct is TDES_Param_DMA_DE
#define GCPU_TDES_CBC_D                     ((UINT32)(TDES_CMD_SID + 0x04))         ///< T-DES Cipher Block Chaining Decryption, parameter struct is TDES_Param_CBC_DE
#define GCPU_TDES_CBC_E                     ((UINT32)(TDES_CMD_SID + 0x05))         ///< T-DES Cipher Block Chaining Encryption, parameter struct is TDES_Param_CBC_DE

/*! \name TDES command group
* @{
*/
/*!
 * @brief Cmd parameter structure for TDES_D, TDES_E
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * Data value can use pbData or u2DataSlotHandle according to uFlag
 * Result value can use pbResult or u2ResSlotHandle according to uFlag
 * for TDES_D, if u2KeySlotHandle or u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 * for TDES_E, if u2DataSlotHandle is a secure slot, u2ResSlotHandle must be a secure slot
 */
typedef struct _TDES_PARAM_DE
{
  BYTE* pbKey;                     /*!< [IN] Key value DRAM address. Len: same with key length */
  BYTE* pbData;                   /*!< [IN] Data value DRAM address. Len: 64bits */
  BYTE* pbResult;                 /*!< [OUT] Result value DRAM address. Len: 64bits */
  UINT16 u2KeySlotHandle;      /*!< [IN] Key Slot Handle. Slot size should be same with key length*/
  UINT16 u2DatSlotHandle;      /*!< [IN] Data Slot Handle. Slot size should be 64 bits. */
  UINT16 u2ResSlotHandle;      /*!< [OUT] Result Slot Handle. Slot size should be 64 bits. */
  UINT8 uFlag;                       /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key, bit1: data, bit2: result */
  UINT8 uKeyLen;                  /*!< [IN] Key length. 0:64bits, 1:128bits, 2:192bits, 3:40bits */
}TDES_Param_DE;

/*!
 * @brief Cmd parameter structure for TDES_DMA_D, TDES_DMA_E
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * for TDES_DMA_D, u2KeySlotHandle must not be a secure slot
 */
typedef struct _TDES_PARAM_DMA_DE
{
  unsigned long u4SrcSa;                 /*!< [IN] Source DRAM Address */
  unsigned long u4DstSa;                 /*!< [IN] Destination DRAM Address */
  UINT32 u4Len;                     /*!< [IN] data length (in unit of byte) */
  BYTE* pbKey;                     /*!< [IN] Key value DRAM address. Len: same with key length */
  UINT16 u2KeySlotHandle;      /*!< [IN] Key Slot Handle. Slot size should be same with key length*/
  UINT8 uKeyLen;                  /*!< [IN] Key length. 0:64bits, 1:128bits, 2:192bits, 3:40bits */
  UINT8 uFlag;                       /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key */
}TDES_Param_DMA_DE;

/*!
 * @brief Cmd parameter structure for TDES_CBC_D, TDES_CBC_E
 *
 * Key value can use pbKey or u2KeySlotHandle according to uFlag
 * for TDES_CBC_D, u2KeySlotHandle must not be a secure slot
 */
typedef struct _TDES_PARAM_CBC_DE
{
  unsigned long  u4SrcSa;                 /*!< [IN] Source DRAM Address */
  unsigned long  u4DstSa;                 /*!< [IN] Destination DRAM Address */
  UINT32 u4Len;                     /*!< [IN] data length (in unit of byte) */
  BYTE* pbKey;                     /*!< [IN] Key value DRAM address. Len: same with key length */
  BYTE* pbIV;                       /*!< [IN] initial vector for CBC. Len: 64bits */
  BYTE* pbFB;                       /*!< [OUT] Feedback value for the next block. This can be set as the initial value of the next consecutive block. Len: 64bits*/
  UINT16 u2KeySlotHandle;      /*!< [IN] Key Slot Handle. Slot size should be same with key length*/
  UINT8 uKeyLen;                  /*!< [IN] Key length. 0:64bits, 1:128bits, 2:192bits, 3:40bits */
  UINT8 uFlag;                       /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key */
}TDES_Param_CBC_DE;


/*! @} */

/*! \name AES command group
* @{
*/
/*!
 * @brief Cmd parameter structure for AES_D, AES_E, AES_G, AES_CMAC
 *
 * for AES_CMAC, key length must be 128-bit
 * for AES_D and AES_G, if tKeySlotHandle or tDataSlotHandle is a secure slot, tResSlotHandle must be a secure slot
 * for AES_E, if tDataSlotHandle is a secure slot, tResSlotHandle must be a secure slot
 *
 * Key value can use pbKey or tKeySlotHandle according to uFlag
 * Data value can use pbData or tDataSlotHandle according to uFlag
 * Result value can use pbResult or tResSlotHandle according to uFlag
 */
typedef struct _AES_PARAM_DEG
{
    BYTE* pbKey;                               /*!< [IN] Key value DRAM address. Len: should be same with key length */
    BYTE* pbData;                              /*!< [IN] Data value DRAM address. Len: 128 bits or 160 bits (CMAC 20 bytes Only) */
    BYTE* pbResult;                            /*!< [OUT] Result DRAM address. Len: 128 bits */
    GCPU_SLOT_HANDLE_T tKeySlotHandle;         /*!< [IN] Key Slot Handle. Slot size should be same with key length */
    GCPU_SLOT_HANDLE_T tDataSlotHandle;        /*!< [IN] Data Slot Handle. Slot size should be 128 bits or 256 bits (Only for CMAC 20 Bytes Only) */
    GCPU_SLOT_HANDLE_T tResSlotHandle;         /*!< [IN] Result Slot Handle. Slot size should be 128 bits or 256 bits (Only for CMAC 20 Bytes Only) */
    UINT8 uFlag;                               /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key, bit1: data, bit2: result */
    UINT8 uKeyLen;                             /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
    UINT32 u4DatLen;                           /*!< [IN] Data transfer length (in unit of byte, Only for CMAC, 128 bits or 160 bits) */
} AES_Param_DEG;

/*!
 * @brief Cmd parameter structure for AES_DCBC, AES_ECBC
 *
 * Key value can use pbKey or tKeySlotHandle according to uFlag
 * IV value can use pbIV or tIVSlotHandle according to uFlag
 * XOR value can use pbXOR or tXORSlotHandle according to uFlag
 * tIVSlotHandle and tXORSlotHandle must not be a secure slot
 * for AES_DCBC, tKeySlotHandle must not be a secure slot
 */
typedef struct _AES_PARAM_CBC
{
    unsigned long u4SrcSa;                           /*!< [IN] Source DRAM Address */
    unsigned long u4DstSa;                           /*!< [IN] Destination DRAM Address */
    UINT32 u4DatLen;                          /*!< [IN] Data transfer length (in unit of byte) */
    BYTE* pbKey;                              /*!< [IN] Key value DRAM address. Len: should be same with key length */
    BYTE* pbIV;                               /*!< [IN] Initial vector value DRAM address. Len: 128bits */
    BYTE* pbXOR;                              /*!< [OUT] XOR data value DRAM address. Len: 128bits (Not support CBC CTS mode)*/
    GCPU_SLOT_HANDLE_T tKeySlotHandle;        /*!< [IN] Key Slot Handle. Slot size should be same with key length. */
    GCPU_SLOT_HANDLE_T tIVSlotHandle;         /*!< [IN] Initial vector slot handle. Slot size should be 128-bit. */
    GCPU_SLOT_HANDLE_T tXORSlotHandle;        /*!< [OUT] XOR data slot handle. Feedback value for the next block. This can be set as the initial value of the next consecutive block. Slot size should be 128-bit. */
    UINT8 uFlag;                              /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key, bit1: IV, bit2: XOR */
    UINT8 uKeyLen;                            /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
} AES_Param_CBC;

/*!
 * @brief Cmd parameter structure for AES_CTR
 *
 * Key value can use pbKey or tKeySlotHandle according to uFlag
 * Data value can use pbData or tDataSlotHandle according to uFlag
 * Result value can use pbResult or tResSlotHandle according to uFlag
 */
typedef struct _AES_PARAM_CTR
{
    unsigned long u4SrcSa;                          /*!< [IN] Source DRAM Address */
    unsigned long u4DstSa;                          /*!< [IN] Destination DRAM Address */
    UINT32 u4DatLen;                         /*!< [IN] Data transfer length (in unit of byte) */
    BYTE* pbKey;                             /*!< [IN] Key value DRAM address. Len: should be same with key length */
    BYTE* pbCtr;                             /*!< [IN] Enc Key value DRAM address. Len: should be same with key length */
    BYTE* pbCtrResult;                       /*!< [OUT] Ctr Output DRAM address */
    GCPU_SLOT_HANDLE_T tKeySlotHandle;       /*!< [IN] Key Slot Handle. Slot size should be same with key length
                                                                                                 Note: For AESPK_EK_D, AESPK_EK_E, this should be sslot */
    GCPU_SLOT_HANDLE_T tCtrSlotHandle;       /*!< [IN] Encryption Key Slot Handle. Slot size should be same with key length */
    GCPU_SLOT_HANDLE_T tCtrResultSlotHandle; /*!< [IN] Data Slot Handle. Slot size should be 128 bits */
    UINT8 uFlag;                             /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key, bit1: Ctr, bit2: Ctr Result */
    UINT8 uKeyLen;                           /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
} AES_Param_CTR;

/*!
 * @brief Cmd parameter structure for AES_DPAK, AES_EPAK
 *
 * Key value can use pbKey or tKeySlotHandle according to uFlag
 * for AES_DPAK, tKeySlotHandle must not be a secure slot
 */
typedef struct _AES_PARAM_PAK
{
    unsigned long u4SrcSa;                          /*!< [IN] Source DRAM Address */
    unsigned long u4DstSa;                          /*!< [IN] Destination DRAM Address */
    UINT32 u4DatLen;                         /*!< [IN] Data transfer length (in unit of byte) */
    BYTE* pbKey;                             /*!< [IN] Key value DRAM address. Len: should be same with key length */
    GCPU_SLOT_HANDLE_T tKeySlotHandle;       /*!< [IN] Key Slot Handle. Slot size should be same with key length */
    UINT8 uFlag;                             /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. bit0: key */
    UINT8 uKeyLen;                           /*!< [IN] Key Length. 0: 128bit, 1: 192bit, 2: 256bit */
} AES_Param_PAK;

/*! @} */

/*! \name Misc command group
* @{
*/

/*!
 * @brief Cmd parameter structure for SHA-1
 */
typedef struct _SHA1_PARAM
{
    unsigned long u4SrcSa;                         /*!< [IN] Source DRAM Address */
    UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte) */
    UINT64 u8BitCnt;                        /*!< [IN] Bit Count for previouse data, for first packet, it should be zero */
    BOOL   fgFirstPacket;                   /*!< [IN] TRUE: The content contains the first packet. FALSE: The content does not contain the first packet. */
    BOOL   fgLastPacket;                    /*!< [IN] TRUE: The content contains the last packet. FALSE: The content does not contain the last packet. */
    BYTE*  pbIniHash;                       /*!< [IN] Initial Hash value DRAM address. Len: 160 bits. It should not be set for first packet */
    BYTE*  pbResHash;                       /*!< [OUT] Result Hash value DRAM address. Len: 160 bits */
} SHA1_Param;


typedef struct _SHA2_PARAM
{
    unsigned long u4SrcSa;                         /*!< [IN] Source DRAM Address */
    UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte) */
    UINT64 u8BitCnt;                        /*!< [IN] Bit Count for previouse data, for first packet, it should be zero */
    BOOL   fgFirstPacket;                   /*!< [IN] TRUE: The content contains the first packet. FALSE: The content does not contain the first packet. */
    BOOL   fgLastPacket;                    /*!< [IN] TRUE: The content contains the last packet. FALSE: The content does not contain the last packet. */
    BYTE*  pbIniHash;                       /*!< [IN] Initial Hash value DRAM address. Len: 256 bits. It should not be set for first packet */
    BYTE*  pbResHash;                       /*!< [OUT] Result Hash value DRAM address. Len: 256 bits */
} SHA2_Param;

typedef struct _HMAC_PARAM
{
    unsigned long u4SrcSa;                  /*!< [IN] Source DRAM Address , Need 64 byte alignment */
    UINT32 u4DatLen;                        /*!< [IN] msg transfer length (in unit of byte) ,in unit of 512 bit(64 byte)*/
    BYTE* pbKey;                             /*!< [IN] Key value DRAM address. Len: should be same with key length */
    GCPU_SLOT_HANDLE_T u2KeySlotHandle;       /*!< [IN] Key Slot Handle. Slot size should be same with key length */
    UINT8 uFlag;                             /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address.  */
    UINT8 uKeyLen;                           /*!< [IN] Key Length.  Now mt8173 temp always use in 128 bitkey(16byte)  */
    BYTE*  pbResHash;                       /*!< [OUT] Result Hash value DRAM address. Len: 256 bits */
} HMAC_Param;


/*!
 * @brief Cmd parameter structure for MD5
 */
typedef struct _MD5_PARAM
{
    unsigned long u4SrcSa;                          /*!< [IN] Source DRAM Address */
    UINT32 u4DatLen;                        /*!< [IN] Data transfer length (in unit of byte) */
    UINT64 u8BitCnt;                        /*!< [IN] Bit Count for previouse data, for first packet, it should be zero */
    BOOL fgFirstPacket;                    /*!< [IN] TRUE: The content contains the first packet. FALSE: The content does not contain the first packet. The initial value has to be set. */
    BOOL   fgLastPacket;                    /*!< [IN] TRUE: The content contains the last packet. FALSE: The content does not contain the last packet. */
    BYTE* pbIniHash;                        /*!< [IN] Initial Hash value DRAM address. Len: 128 bits */
    BYTE* pbResHash;                       /*!< [OUT] Result Hash value DRAM address. Len: 128 bits */
}MD5_Param;

/*!
 * @brief Cmd parameter structure for MEM_XOR
 */
typedef struct _MEM_PARAM_XOR
{
    UINT32 u4DataSz;                         /*!< [IN] Data size. */
    UINT8* pbData;                           /*!< [IN] Data value DRAM address. */
    UINT8* pbXORValue;                       /*!< [IN] XOR value DRAM address. */
    GCPU_SLOT_HANDLE_T tDataSlotHandle;      /*!< [IN/OUT] Data slot handle. Slot size should be 128bits. */
    UINT8  uFlag;                            /*!< [IN] Slot handle flag. 1: using slot handle, 0: using DRAM address. */
} MEM_Param_XOR;

// ************************************************************************************************************
// GCPU API
// ************************************************************************************************************

typedef enum
{
    GCPU_RET_OK = 0,
    GCPU_RET_UNEXPECT = 1,
    GCPU_RET_INVALID_HANDLE = 2,
    GCPU_RET_PARAM_WRONG = 3,
    GCPU_RET_OS_OPERA_FAIL = 4,
    GCPU_RET_NO_INIT = 5,
    GCPU_RET_OVER_LIMIT = 6,
    GCPU_RET_OPERATE_FORBID = 7,
    GCPU_RET_NO_MEM = 8,
    GCPU_RET_CMD_FAIL = 9,
    GCPU_RET_UNIMPLEMENT = 10,
    GCPU_RET_FAIL = 11,
    GCPU_RET_INVALID_ARG = 12,
    GCPU_RET_NO_RES = 13,
    GCPU_RET_NO_INST = 14,
    GCPU_RET_MEM_MAP_FAIL = 15,
    GCPU_RET_HW_FAIL = 16,
    GCPU_RET_UNKNOWN_FAIL = 17
} GCPU_RET_T;

typedef enum
{
    GCPU_CMD_MODE_SYNC = 0,
    GCPU_CMD_MODE_ASYNC
} GCPU_CMD_MODE_T;

/*
*  GCPU callback function prototype
*  - Used for Async command execute
*
* @param fgResult TRUE: Success. FALSE: Fail
* @param u4Cmd Execute command
* @param pvUserPrivate user private data
*/
typedef void (*GCPU_FUNC_CB)(GCPU_RET_T eResult, UINT32 u4Cmd, void* pvUserPrivate);

typedef struct _GCPU_CREATE_INST_PARAM_T
{
    GCPU_FUNC_CB pfCbFunc;        ///< [IN] Callback function (NOTE: You should provide this if you want to use GCPU_ExeCmd with GCPU_CMD_MODE_ASYNC)
    void *pvUserPrivate;          ///< [IN] User private data for callback function
    const char *pcUserName;       ///< [IN] (NOTE: Can't be NULL) User name for debug, support max size == GCPU_USERNAME_SIZE
} GCPU_CREATE_INST_PARAM_T;

typedef struct _GCPU_CMD_PARAM_T
{
    UINT32 u4CmdId;               ///< [IN] GCPU command id
    GCPU_CMD_MODE_T tCmdMode;     ///< [IN] Command mode
    void *pvParam;                ///< [IN] Parameter for corresponding command id
    UINT32 u4ParamSize;           ///< [IN] Parameter size
} GCPU_CMD_PARAM_T;

/**
 * Create a new GCPU instance
 * -This will be called before GCPU_ExeCmd
 *
 * @param inParam Setting for the new GCPU instance
 * @param outHandle Handle for the new GCPU instance.
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_CreateInstance (GCPU_CREATE_INST_PARAM_T *inParam, GCPU_INST_HANDLE_T *outHandle);

/**
 * Executing a GCPU command
 *
 * @param inHandle Handle of GCPU instance
 * @param inCmd Executed command information
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_ExeCmd (GCPU_INST_HANDLE_T inHandle, GCPU_CMD_PARAM_T *inCmd);

/**
 * Desroty a GCPU instance
 *
 * @param inHandle Handle of GCPU instance
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_DestoryInstance (GCPU_INST_HANDLE_T inHandle);

/**
 * Allocate a secure slot
 * -The secure slot is a protected space which could be accessed by GCPU, RISC could only write data
 *
 * @param inUserName User name for debug (NOTE: Can't be NULL) 
 * @param inReqByte Required size (in unit of bytes)
 * @param outSlotHandle Handle for the allocated secure slot
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_AllocSecSlot (const char *inUserName, UINT32 inReqByte, GCPU_SLOT_HANDLE_T *outSlotHandle);

/**
 * Free a secure slot
 *
 * @param inSlotHandle Handle of secure slot
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_FreeSecSlot (GCPU_SLOT_HANDLE_T inSlotHandle);

/**
 * Write data into a secure slot
 * - Wrap GCPU_ExeCmd with GCPU_SETKEY2SLOT and GCPU_CMD_MODE_SYNC for convenience
 *
 * @param inHandle Handle of GCPU instance
 * @param inSlotHandle Handle of secure slot
 * @param inCpyByte Copy size (in unit of bytes)
 * @param inSrcData Pointer to the source of data
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_SetSecSlotData(GCPU_INST_HANDLE_T inHandle, GCPU_SLOT_HANDLE_T inSlotHandle, UINT32 inCpyByte, void *inSrcData);

/**
 * Allocate a non-secure slot
 * -The non-secure slot is a virtual slot resource created by secure dram (MTEE_AllocMem)
 * -RISC could read/write a non-secure slot
 * -Note that some command ex. GCPU_SETKEY2SLOT must use secure slot only
 *
 * @param inUserName User name for debug (NOTE: Can't be NULL) 
 * @param inReqByte Required size (in unit of bytes)
 * @param outSlotHandle Handle for the allocated non-secure slot
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_AllocNsecSlot (const char *inUserName, UINT32 inReqByte, GCPU_SLOT_HANDLE_T *outSlotHandle);

/**
 * Free a non-secure slot
 *
 * @param inSlotHandle Handle of non-secure slot
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_FreeNsecSlot (GCPU_SLOT_HANDLE_T inSlotHandle);

/**
 * Write data into a non-secure slot
 *
 * @param inSlotHandle Handle of non-secure slot
 * @param inCpyByte Copy size (in unit of bytes)
 * @param inSrcData Pointer to the source of data
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_SetNsecSlotData (GCPU_SLOT_HANDLE_T inSlotHandle, UINT32 inCpyByte, void *inSrcData);

/**
 * Read data from a non-secure slot
 *
 * @param inSlotHandle Handle of non-secure slot
 * @param inCpyByte Copy size (in unit of bytes)
 * @param inDstData Pointer to the destination buffer
 * @return GCPU_RET_OK if success
 */
GCPU_RET_T GCPU_GetNsecSlotData (GCPU_SLOT_HANDLE_T inSlotHandle, UINT32 inCpyByte, void *inDstData);


#ifdef __cplusplus
}
#endif

#endif //#ifndef _GCPU_DRV_IF_H_

