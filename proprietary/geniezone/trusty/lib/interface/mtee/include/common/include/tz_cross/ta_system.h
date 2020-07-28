/*
* Copyright (c) 2015 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/** Commands for TA SYSTEM **/

#ifndef __TRUSTZONE_TA_SYSTEM__
#define __TRUSTZONE_TA_SYSTEM__

/// Special handle for system connect.
/// NOTE: Handle manager guarantee normal handle will have bit31=0.
#define MTEE_SESSION_HANDLE_SYSTEM 0xFFFF1234



/* Session Management */
#define TZCMD_SYS_INIT                0
#define TZCMD_SYS_SESSION_CREATE      1
#define TZCMD_SYS_SESSION_CLOSE       2
#define TZCMD_SYS_IRQ                 3
#define TZCMD_SYS_THREAD_CREATE       4
#define TZCMD_SYS_STUCK               5

#define TZCMD_MEM_APPEND_MULTI_CHUNKMEM      23
#define TZCMD_MEM_SECUREMULTICHUNKMEM_ALLOC  25
#define TZCMD_MEM_SECUREMULTICHUNKMEM_UNREF  28
#define TZCMD_MEM_RELEASE_CHUNKMEM           24
#define TZCMD_MEM_APPEND_MULTI_CHUNKMEM_ION  31
#define TZCMD_MEM_RELEASE_CHUNKMEM_ION       32

#endif /* __TRUSTZONE_TA_SYSTEM__ */
