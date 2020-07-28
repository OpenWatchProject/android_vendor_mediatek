/*------------------------------------------------------------------------------
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
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
 *----------------------------------------------------------------------------*/
#ifndef __CMDL_H__
#define __CMDL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define CMDL_RESULT CMDL_Result_e
#define CMDL_CTL_FLAGS_ENABLE_PROFILING                (1)    ///< [bit 0] Use systrace to profiling. The flags value is NULL
#define CMDL_CTL_FLAGS_DEBUG_BIT_TRUE                  (2)    ///< [bit 1] Check bit true. The flags value is NULL
#define CMDL_CTL_FLAGS_EXTERNAL_MEMORY                 (3)    ///< [bit 2] Use external memory. The flags value defined structure type is "CMDL_HANDLE"
#define CMDL_HANDLE_NULL                            0
#define CMDL_NODE_NULL                              0

typedef struct CmdL_Handle_s* CMDL_HANDLE;
typedef void* CMDL_Node_t;

// defined same as MKErrorType structure in ./src/MKType.h
typedef enum {
    CMDL_OK                         = 0,
    CMDL_OK_DISABLE                 = 1,
    CMDL_OK_HANDLE_FULL             = 2,
    CMDL_ERR                        = 16,
    CMDL_ERR_INVALID_PARAMS         = 17,
    CMDL_ERR_INVALID_HANDLE         = 18,
    CMDL_ERR_INVALID_NODE           = 19,
    CMDL_ERR_BAD_RESOURCE_MAP       = 20,
    CMDL_ERR_BAD_BUFFER_MAP         = 21,
    CMDL_ERR_OUT_OF_MEMORY          = 22,
    CMDL_ERR_COMMAND_RECORD_FAIL    = 23,
    CMDL_ERR_COMMAND_RUN_FAIL       = 24,
    CMDL_ERR_CL_DEVICE_NOT_MATCHED  = 25,
    CMDL_ERR_NOT_FOUND              = 26,
    CMDL_ERR_DUMP_FILE              = 27,
    CMDL_ERR_SET_HOST_PTR_FAIL      = 28,
    CMDL_ERR_COMPARE_SIZE_NOT_MATCH = 29,
    CMDL_ERR_COMPARE_NOT_MATCH      = 30,
} CMDL_Result_e;

typedef enum {
    E_CMDL_DEBUG_LEVEL_FATAL=0,      ///< Fatal
    E_CMDL_DEBUG_LEVEL_ERROR,        ///< Error
    E_CMDL_DEBUG_LEVEL_WARN,         ///< Warn
    E_CMDL_DEBUG_LEVEL_DEBUG,        ///< Debug
    E_CMDL_DEBUG_LEVEL_INFO,         ///< Info
    E_CMDL_DEBUG_LEVEL_VERBOSE,      ///< Verbose
    E_CMDL_DEBUG_LEVEL_MAX,
} CMDL_DebugLevel_e;

// defined same as MKProgramType structure in ./src/MKType.h
typedef enum {
    E_CMDL_RUN_TYPE_INIT     = 0x0001,
    E_CMDL_RUN_TYPE_EXECUTE  = 0x0002,
    E_CMDL_RUN_TYPE_END      = 0x0004,
    E_CMDL_RUN_TYPE_ANY      = 0xFFFF,
} CMDL_RunType_e;

typedef enum {
    E_CMDL_HOST_SOURCE_INPUT = 1,    ///< Host input type
    E_CMDL_HOST_SOURCE_OUTPUT,       ///< Host output type
    E_CMDL_HOST_SOURCE_MAX,
} CMDL_HostSource_e;

typedef enum {
    E_CMDL_CB_EVENT_CL = 0,
    E_CMDL_CB_EVENT_CL_CREATE_BUFFER,
    E_CMDL_CB_EVENT_CL_ENQUEUE_MAP_BUFFER,
    E_CMDL_CB_EVENT_CL_RELEASE_MEM_OBJECT,
    E_CMDL_CB_EVENT_APUSYS = 128,
    E_CMDL_CB_EVENT_MAX,
} CMDL_CallbackEvent_e;

typedef struct CMDL_ClCreateBufferEventParams_s
{
    size_t size;                   ///<[in]
    void* bufferHandle;            ///<[out]
    bool* isProtected;             ///<[out]
} CMDL_ClCreateBufferEventParams_t;

typedef struct CMDL_ClReleaseMemObjectEventParams_s
{
    void* bufferHandle;            ///<[in]
} CMDL_ClReleaseMemObjectEventParams_t;

// typedef struct CMDL_CmdType_s
// {
//     //void *pValue;                ///<
// } CMDL_CmdType_t;

//------------------------------------------------------------------------------
/// @brief Callback function to notify getting the event from the CmdL.
/// @param[in] hCmdL: Handle.
/// @param[in] eEvent: Event type. use CMDL_CallbackEvent_e enum value.
/// @param[in] pEventParams: A pointer to structure CMDL_EventParams_t for the notifying event.
/// @param[in] pUserParams: For passing user-defined parameters. it will be NULL because of not USE.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR: Process fail.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
//------------------------------------------------------------------------------
typedef CMDL_RESULT (*CMDL_CallbackFn)(CMDL_HANDLE hCmdL, CMDL_CallbackEvent_e eEvent, void* pEventParams, void* pUserParams);

//------------------------------------------------------------------------------
/// @brief Create.
/// @param[in] eType: Run type defined in enum type CMDL_RunType_e.
/// @param[out] ppNode: Tree node.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_TreeNodeCreate(CMDL_RunType_e eType, CMDL_Node_t* ppNode);

//------------------------------------------------------------------------------
/// @brief Release.
/// @param[in] pNode: Tree node.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_TreeNodeRelease(CMDL_Node_t pNode);

//------------------------------------------------------------------------------
/// @brief Create.
/// @param[in] pstCmd: Command defined in enum type CMDL_CmdType_t.
/// @param[out] ppNode: Cmd node or tree node.
/// @param[out] pBeTreeNode: Is tree node.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_CmdNodeCreate(void* pstCmd, CMDL_Node_t* ppNode, bool *pBeTreeNode);

//------------------------------------------------------------------------------
/// @brief Release.
/// @param[in] pNode: Cmd node.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_NODE: Node are invalied.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_CmdNodeRelease(CMDL_Node_t pNode);

//------------------------------------------------------------------------------
/// @brief Attach node.
/// @param[in] pParentNode: Parent tree node.
/// @param[in] pChildNode: Child tree node.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_TreeNodeAttachNode(CMDL_Node_t pParentNode, CMDL_Node_t pChildNode);

//------------------------------------------------------------------------------
/// @brief Deattach node.
/// @param[in] pParentNode: Parent tree node.
/// @param[in] pChildNode: Child tree node.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_TreeNodeDeAttachNode(CMDL_Node_t pParentNode, CMDL_Node_t pChildNode);

//------------------------------------------------------------------------------
/// @brief Dump Json file.
/// @param[in] pNode: Tree node.
/// @param[in] pFilename: File name.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
/// @return CMDL_ERR_INVALID_NODE: Node are invalied.
/// @return CMDL_ERR_DUMP_FILE: Save file failed.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_TreeNodeDumpJsonFile(CMDL_Node_t pNode, const char* pszFilename);

//------------------------------------------------------------------------------
/// @brief Push node to top.
/// @param[in] pNode: Tree node.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_TreeNodePushStack(CMDL_Node_t pNode);

//------------------------------------------------------------------------------
/// @brief Pop node from top one.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_TreeNodePopStack(void);

//------------------------------------------------------------------------------
/// @brief Get node.
/// @param[out] ppNode: Tree node.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_TreeNodeGetTop(CMDL_Node_t* ppNode);

//------------------------------------------------------------------------------
/// @brief Create handle.
/// @param[out] phCmdL: A handle pointer to retrieve an instance of a created CmdL module.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_CreateHandle(CMDL_HANDLE *phCmdL);

//------------------------------------------------------------------------------
/// @brief Release handle.
/// @param[in] hCmdL: An instance of a created CmdL module.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_ReleaseHandle(CMDL_HANDLE hCmdL);

//------------------------------------------------------------------------------
/// @brief Insert tree node.
/// @param[in] hCmdL: Handle.
/// @param[in] pNode: Tree node.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
/// @return CMDL_ERR_INVALID_NODE: Node are invalied.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_InsertTreeNode(CMDL_HANDLE hCmdL, CMDL_Node_t pNode);

//------------------------------------------------------------------------------
/// @brief Save to file.
/// @param[in] hCmdL: Handle.
/// @param[in] pFilename: File name.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
/// @return CMDL_ERR_INVALID_NODE: Node are invalied.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_SaveToFile(CMDL_HANDLE hCmdL, const char* pszFilename);

//------------------------------------------------------------------------------
/// @brief Get buffer size.
/// @param[in] hCmdL: Handle.
/// @param[out] pSize: Buffer size.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_GetBufferSize(CMDL_HANDLE hCmdL, unsigned int* pSize);

//------------------------------------------------------------------------------
/// @brief Save to buffer.
/// @param[in] hCmdL: Handle.
/// @param[in] pBuffer: Buffer.
/// @param[in] size: Buffer size.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
/// @return CMDL_ERR_INVALID_NODE: Node are invalied.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_SaveToBuffer(CMDL_HANDLE hCmdL, const char* pBuffer, unsigned int size);

//------------------------------------------------------------------------------
/// @brief Get tree from file.
/// @param[in] hCmdL: Handle.
/// @param[in] pszFilename: File name.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_LoadByFile(CMDL_HANDLE hCmdL, const char* pszFilename);

//------------------------------------------------------------------------------
/// @brief Get tree load from buffer.
/// @param[in] hCmdL: Handle.
/// @param[in] pBuffer: Buffer.
/// @param[in] size: Buffer size.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_LoadByBuffer(CMDL_HANDLE hCmdL, const char* pBuffer, unsigned int size);

//------------------------------------------------------------------------------
/// @brief Get tree node count.
/// @param[in] hCmdL: Handle.
/// @param[out] pCount: Tree node count.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_GetTreeNodeCount(CMDL_HANDLE hCmdL, unsigned int *pCount);

//------------------------------------------------------------------------------
/// @brief Get tree node.
/// @param[in] hCmdL: Handle.
/// @param[in] i: Take the ith.
/// @param[out] pCount: Tree node count.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_GetTreeNode(CMDL_HANDLE hCmdL, unsigned int i, CMDL_Node_t *ppNode);

//------------------------------------------------------------------------------
/// @brief Register Callback function
/// @param[in] hCmdL: Handle.
/// @param[in] eEvent: Event defined in enum type CMDL_CallbackEvent_e.
/// @param[in] pfEventCallback: Callback function pointer.
/// @param[in] pUserParams: For passing user-defined parameters.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR: Process fail.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_RegisterCallback(CMDL_HANDLE hCmdL, CMDL_CallbackEvent_e eEvent, CMDL_CallbackFn pfEventCallback, void* pUserParams);

//------------------------------------------------------------------------------
/// @brief Unregister Callback function
/// @param[in] hCmdL: Handle.
/// @param[in] eEvent: Event defined in enum type CMDL_CallbackEvent_e.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_UnRegisterCallback(CMDL_HANDLE hCmdL, CMDL_CallbackEvent_e eEvent);

//------------------------------------------------------------------------------
/// @brief Set host memory pointer.
/// @param[in] hCmdL: Handle.
/// @param[in] eSource: Source type defined in enum type CMDL_HostSource_e.
/// @param[in] id: Dimension.
/// @param[in] pData: Data for every dimension.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_SetHostPtr(CMDL_HANDLE hCmdL, CMDL_Node_t pNode, CMDL_HostSource_e eSource, unsigned int id, void* pData);

//------------------------------------------------------------------------------
/// @brief Run.
/// @param[in] hCmdL: Handle.
/// @param[in] eType: Run type defined in enum type CMDL_RunType_e.
/// @param[in] pNode: Tree node.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_Run(CMDL_HANDLE hCmdL, CMDL_RunType_e eType, CMDL_Node_t pNode);

//------------------------------------------------------------------------------
/// @brief Get enable.
/// @param[out] pEnable: enable cmdl feature or not.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_GetEnable(bool *pEnable);

//------------------------------------------------------------------------------
/// @brief Set enable.
/// @param[in] bEnable: enable cmdl feature or not.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_SetEnable(bool bEnable);

//------------------------------------------------------------------------------
/// @brief Set debug level.
/// @param[in] eLevel: Debug level defined in enum type CMDL_DebugLevel_e.
/// @return CMDL_OK: Process success.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_SetDebugLevel(CMDL_DebugLevel_e eLevel);

//------------------------------------------------------------------------------
/// @brief Set debug flags.
/// @param[in] flags: Debug flag using DEBUG_FLAGS defined value.
/// @param[in] pValue: Flag value.
/// @return CMDL_OK: Process success.
/// @return CMDL_ERR_INVALID_PARAMS: Parameters are invalid.
//------------------------------------------------------------------------------
CMDL_RESULT CmdL_SetDebugFlags(unsigned int flags, void* pValue);

#ifdef __cplusplus
}
#endif

#endif///__CMDL_H__
