/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
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

#ifndef __MT_DRV_ERROR_H__
#define __MT_DRV_ERROR_H__

#define DRV_FWK_API_OK 0x0 /**< Returns on successful execution of a function. */
#define DRV_FWK_API_IPCH_WRONG_CMD 0x1 /** IPCH command is not supported */
#define DRV_FWK_API_IPCH_CLIENT_NOT_EXISTED 0x2 /** IPCH client is not existed */
#define DRV_FWK_API_MAP_TASK_BUFFER_FAILED 0x3 /** IPCH fail to map client buffer */
#define DRV_FWK_API_MAP_HARDWARE_FAILED 0x4 /** Fail to map hardware region */
#define DRV_FWK_API_INVALIDATE_PARAMETERS 0x5 /** Parameters are not valid */
#define DRV_FWK_API_WRONG_CALL_FLOW 0x6 /** Call direction from secure driver to tee driver is not supported */

#endif //__MT_DRV_ERROR_H__

