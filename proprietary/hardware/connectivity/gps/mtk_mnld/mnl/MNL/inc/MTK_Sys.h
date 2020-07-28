/***********************************************************************
*   This software/firmware and related documentation ("MediaTek Software")
*   are protected under relevant copyright laws. The information contained
*   herein is confidential and proprietary to MediaTek Inc. and/or its licensors.
*
*   Without the prior written permission of MediaTek Inc. and/or its licensors,
*   any reproduction, modification, use or disclosure of MediaTek Software, and
*   information contained herein, in whole or in part, shall be strictly prohibited.
*
*   MediaTek Inc. (C) [2008]. All rights reserved.
*
*************************************************************************/
//*****************************************************************************
// [File] MTK_Sys.h
// [Version] v1.0
// [Revision Date] 2008-03-31
// [Author] YC Chien, yc.chien@mediatek.com, 21558
// [Description] Define the prototypes of operating system dependent functions
//*****************************************************************************

#ifndef MTK_SYS_H
#define MTK_SYS_H

#ifdef __cplusplus
   extern "C" {
#endif

#include "MTK_Type.h"

//#define BEE_HEAP_DEBUG

//*****************************************************************************
// Semaphore Functions
//*****************************************************************************

//*****************************************************************************
// MTK_Sys_Init_Smphr : Initial semaphore
//
// PARAMETER : u4SmphrNum [IN] - semaphore number to be initizlized
//
// RETURN : void

void MTK_Sys_Init_Smphr (MTK_UINT32 u4SmphrNum);


//*****************************************************************************
// MTK_Sys_Delete_Smphr : Delete semaphore
//
// PARAMETER : u4SmphrNum [IN] - semaphore number to be deleted
//
// RETURN : void

void MTK_Sys_Delete_Smphr (MTK_UINT32 u4SmphrNum);


//*****************************************************************************
// MTK_Sys_Reserve_Smphr : Reserve semaphore
//
// PARAMETER : u4SmphrNum [IN] - semaphore number to reserve
//
// RETURN : void

void MTK_Sys_Reserve_Smphr (MTK_UINT32 u4SmphrNum);


//*****************************************************************************
// MTK_Sys_Release_Smphr : Release semaphore
//
// PARAMETER : u4SmphrNum [IN] - semaphore number to release
//
// RETURN : void

void MTK_Sys_Release_Smphr (MTK_UINT32 u4SmphrNum);


//*****************************************************************************
// MTK_Sys_Init_Smphr_For_EPO : Initial semaphore
//
// PARAMETER : u4SmphrNum [IN] - semaphore number to be initizlized
//
// RETURN : void

void MTK_Sys_Init_Smphr_For_EPO (MTK_UINT32 u4SmphrNum);


//*****************************************************************************
// MTK_Sys_Delete_Smphr_For_EPO : Delete semaphore
//
// PARAMETER : u4SmphrNum [IN] - semaphore number to be deleted
//
// RETURN : void

void MTK_Sys_Delete_Smphr_For_EPO(MTK_UINT32 u4SmphrNum);


//*****************************************************************************
// MTK_Sys_Reserve_Smphr_For_EPO : Reserve semaphore
//
// PARAMETER : u4SmphrNum [IN] - semaphore number to reserve
//
// RETURN : void

void MTK_Sys_Reserve_Smphr_For_EPO (MTK_UINT32 u4SmphrNum);


//*****************************************************************************
// MTK_Sys_Release_Smphr_For_EPO : Release semaphore
//
// PARAMETER : u4SmphrNum [IN] - semaphore number to release
//
// RETURN : void

void MTK_Sys_Release_Smphr_For_EPO (MTK_UINT32 u4SmphrNum);


//*****************************************************************************
// Memory Functions
//*****************************************************************************

//*****************************************************************************
// MTK_Sys_Memory_Alloc : Allocate a block of memory
//
// PARAMETER : u4Size [IN] - size of memory (bytes) to be allocated
//
// RETURN : On success, return the pointer to the allocated memory
//          If fail, return NULL

void* MTK_Sys_Memory_Alloc (MTK_UINT32 u4Size);


//*****************************************************************************
// MTK_Sys_Memory_Free : Deallocate memory previosly allocated by MTK_Sys_Memory_Alloc()
//
// PARAMETER : pMemory [IN] - pointer to memory to be freed
//
// RETURN : void

void MTK_Sys_Memory_Free (void *pMemory);


//*****************************************************************************
// File Functions
//*****************************************************************************

//*****************************************************************************
// MTK_Sys_File_Open : Open a file
//
// PARAMETER : szFileName [IN] - name of the file to be opened
//             i4Mode     [IN] - file access mode (read / write / read + write)
//                               0 -- open file for reading (r)
//                               1 -- create file for writing,
//                                    discard previous contents if any (w)
//                               2 -- open or create file for writing at end of file (a)
//                               3 -- open file for reading and writing (r+)
//                               4 -- create file for reading and writing,
//                                    discard previous contents if any (w+)
//                               5 -- open or create file for reading and writing at end of file (a+)
//
// NOTE : For system which treats binary mode and text mode differently,
//        such as Windows / DOS, please make sure to open file in BINARY mode
//
// RETURN : On success, return the file handle
//          If fail, return 0

MTK_FILE MTK_Sys_File_Open (const char *szFileName, MTK_INT32 i4Mode);


//*****************************************************************************
// MTK_Sys_File_Close : Close a file
//
// PARAMETER : hFile [IN] - handle of file to be closed
//
// RETURN : void

void MTK_Sys_File_Close (MTK_FILE hFile);


//*****************************************************************************
// MTK_Sys_File_Read : Read a block of data from file
//
// PARAMETER : hFile    [IN]  - handle of file
//             DstBuf   [OUT] - pointer to data buffer to be read
//             u4Length [IN]  - number of bytes to read
//
// RETURN : Number of bytes read

MTK_UINT32 MTK_Sys_File_Read (MTK_FILE hFile, void *DstBuf, MTK_UINT32 u4Length);


//*****************************************************************************
// MTK_Sys_File_Write : Write a block of data from file
//
// PARAMETER : hFile    [IN] - handle of file
//             SrcBuf   [IN] - pointer to data buffer to be written
//             u4Length [IN] - number of bytes to write
//
// RETURN : Number of bytes written

MTK_UINT32 MTK_Sys_File_Write (MTK_FILE hFile, void *SrcBuf, MTK_UINT32 u4Length);


//*****************************************************************************
// MTK_Sys_File_Seek : Set the position indicator associated with file handle
//                     to a new position defined by adding offset to a reference
//                     position specified by origin
//
// PARAMETER : hFile    [IN] - handle of file
//             u4OffSet [IN] - number of bytes to offset from origin
//             u4Origin [IN] - position from where offset is added
//                             0 -- seek from beginning of file
//                             1 -- seek from current position
//                             2 -- seek from end of file
//
// RETURN : On success, return a zero value
//          Otherwise, return a non-zero value

MTK_INT32 MTK_Sys_File_Seek (MTK_FILE hFile, MTK_UINT32 u4OffSet, MTK_UINT32 u4Origin);

//*****************************************************************************
// MTK_Sys_Debug_Output : Output debug messages of HotStill library
//                        Porting layer can help to log on file
//                        or sent by other interfaces
//
// PARAMETER : buffer [IN] - data pointer
//             length [IN] - size of data
//
// RETURN : On success, return a zero value
//          Otherwise, return a non-zero value

MTK_BEE_INT MTK_Sys_Debug_Output (char* buffer, unsigned int length);


//*****************************************************************************
// Time Functions
//*****************************************************************************

//*****************************************************************************
// MTK_Sys_Time : Get the current system time
//
// PARAMETER : pUTCTime [OUT] - UTC time
//
// RETURN : Success (MTK_TRUE) or Fail (MTK_FAIL)

MTK_BOOL MTK_Sys_Time (MTK_TIME *pUTCTime);

/*****************************************************************************
 * FUNCTION
 *  MTK_Sys_Get_EPO_File_Size
 * DESCRIPTION
 *  Get the file size
 * PARAMETERS
 *  None
 * RETURNS
 *  File size
 *****************************************************************************/
MTK_UINT32 MTK_Sys_Get_EPO_File_Size(MTK_FILE hFile);


#ifdef __cplusplus
   }
#endif

#endif /* MTK_SYS_H */
