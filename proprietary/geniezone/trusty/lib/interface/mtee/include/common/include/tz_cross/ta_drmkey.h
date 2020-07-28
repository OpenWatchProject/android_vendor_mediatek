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

#ifndef __TRUSTZONE_TA_DRMKEY__
#define __TRUSTZONE_TA_DRMKEY__

#define TZ_TA_DRMKEY_UUID   "989850BF-4663-9DCD-394C-07A45F4633D1"

/* Data Structure for DRMKEY TA */
/* You should define data structure used both in REE/TEE here
   N/A for Test TA */

/* Command for Test TA */
#define TZCMD_DRMKEY_INSTALL            0
#define TZCMD_DRMKEY_QUERY              1
#define TZCMD_DRMKEY_GEN_EKKB_PUB       2
#define TZCMD_DRMKEY_GEN_KB_EKKB_EKC    3
#define TZCMD_DRMKEY_GEN_REENC_EKKB     4
#define TZCMD_DRMKEY_INIT_ENV           5
#define TZCMD_DRMKEY_VERIFY_AEK         6
#define TZCMD_DRMKEY_SIGNATURE_OP       7

#endif /* __TRUSTZONE_TA_DRMKEY__ */
