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

#ifndef __MK_PROGRAM_CL_H__
#define __MK_PROGRAM_CL_H__

#include "MKType.h"
#include "MKDebug.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#if defined(__GNUG__) && __GNUG__ >= 8
#pragma GCC diagnostic ignored "-Wcatch-value"
#endif // defined(__GNUG__) && __GNUG__ >= 8
#include "CL/cl_platform.h"
#include "CL/cl.h"
#include "CL/cl_ext.h"
#pragma GCC diagnostic pop

#ifdef __cplusplus
extern "C" {
#endif

// cl_context
MKErrorType MKProgram_AttachCLCreateContext(
    MKProgram mk_program,
    const cl_context_properties *properties,
    cl_uint num_devices,
    const cl_device_id *devices,
    cl_context context);

MKErrorType MKProgram_AttachCLRetainContext(
    MKProgram mk_program,
    cl_context context);

cl_context MKProgram_GetCLContext(
    MKProgram mk_program);

MKErrorType MKProgram_AttachCLReleaseContext(
    MKProgram mk_program,
    cl_context context);

// cl_command_queue
MKErrorType MKProgram_AttachCLCreateCommandQueue(
    MKProgram mk_program,
    cl_context context,
    cl_device_id device_id,
    cl_command_queue_properties properties,
    cl_command_queue command_queue);

MKErrorType MKProgram_AttachCLRetainCommandQueue(
    MKProgram mk_program,
    cl_command_queue command_queue);

MKErrorType MKProgram_AttachCLReleaseCommandQueue(
    MKProgram mk_program,
    cl_command_queue command_queue);

cl_command_queue MKProgram_GetCLCommandQueue(
    MKProgram mk_program);

// cl_program
MKErrorType MKProgram_AttachCLCreateProgramWithBinary(
    MKProgram mk_program,
    cl_context context,
    cl_uint num_devices,
    const cl_device_id *device_list,
    const size_t *lengths,
    const unsigned char **binaries,
    cl_program program);

MKErrorType MKProgram_AttachCLCreateProgramWithSource(
    MKProgram mk_program,
    cl_context context,
    cl_uint count,
    const char **strings,
    const size_t *lengths,
    cl_program program);

MKErrorType MKProgram_AttachCLBuildProgram(
    MKProgram mk_program,
    cl_program program,
    cl_uint num_devices,
    const cl_device_id *device_list,
    const char *options);

MKErrorType MKProgram_AttachCLRetainProgram(
    MKProgram mk_program,
    cl_program program);

MKErrorType MKProgram_AttachCLReleaseProgram(
    MKProgram mk_program,
    cl_program program);

// cl_kernel
MKErrorType MKProgram_AttachCLCreateKernel(
    MKProgram mk_program,
    cl_program program,
    const char *kernel_name,
    cl_kernel kernel);

MKErrorType MKProgram_AttachCLCreateKernelsInProgram(
    MKProgram mk_program,
    cl_program  program,
    cl_uint num_kernels,
    cl_kernel *kernels);

MKErrorType MKProgram_AttachCLSetKernelArg(
    MKProgram mk_program,
    cl_kernel kernel,
    cl_uint index,
    size_t size,
    const void *ptr);

MKErrorType MKProgram_AttachCLRetainKernel(
    MKProgram mk_program,
    cl_kernel kernel);

MKErrorType MKProgram_AttachCLReleaseKernel(
    MKProgram mk_program,
    cl_kernel kernel);

// cl_mem
MKErrorType MKProgram_AttachCLCreateBuffer(
    MKProgram mk_program,
    cl_context context,
    cl_mem_flags flags,
    size_t size,
    void *ptr,
    cl_mem clMem);

MKErrorType MKProgram_AttachCLRetainMemObject(
    MKProgram mk_program,
    cl_mem clMem);

MKErrorType MKProgram_AttachCLReleaseMemObject(
    MKProgram mk_program,
    cl_mem clMem);

// cl_event
MKErrorType MKProgram_AttachCLWaitForEvents(
    MKProgram mk_program,
    cl_uint num_events,
    const cl_event *event_list);

MKErrorType MKProgram_AttachCLRetainEvent(
    MKProgram mk_program,
    cl_event event);

MKErrorType MKProgram_AttachCLReleaseEvent(
    MKProgram mk_program,
    cl_event event);

// svm
MKErrorType MKProgram_AttachCLSVMAlloc(
    MKProgram mk_program,
    cl_context context,
    cl_mem_flags flags,
    size_t size,
    unsigned int alignment,
    void* ptr);

MKErrorType MKProgram_AttachCLSVMFree(
    MKProgram mk_program,
    cl_context context,
    void* ptr);

// cl enqueue
MKErrorType MKProgram_AttachCLEnqueueMarker(
    MKProgram mk_program,
    cl_command_queue command_queue,
    cl_event event);

MKErrorType MKProgram_AttachCLEnqueueMarkerWithWaitList(
    MKProgram mk_program,
    cl_command_queue command_queue,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event event);

MKErrorType MKProgram_AttachCLEnqueueMapBuffer(
    MKProgram mk_program,
    cl_command_queue cmdQueue,
    cl_mem buffer,
    cl_bool blocking_read,
    cl_map_flags map_flags,
    size_t offset,
    size_t cb,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event event,
    void* ptr);

MKErrorType MKProgram_AttachCLEnqueueUnmapMemObject(
    MKProgram mk_program,
    cl_command_queue cmdQueue,
    cl_mem memobj,
    void* ptr,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event event);

MKErrorType MKProgram_AttachCLEnqueueReadBuffer(
    MKProgram mk_program,
    cl_command_queue command_queue,
    cl_mem buffer,
    cl_bool blocking_read,
    size_t offset,
    size_t cb,
    void *ptr,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event event);

MKErrorType MKProgram_AttachCLEnqueueWriteBuffer(
    MKProgram mk_program,
    cl_command_queue command_queue,
    cl_mem buffer,
    cl_bool blocking_read,
    size_t offset,
    size_t cb,
    const void *ptr,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event event);

MKErrorType MKProgram_AttachCLEnqueueNDRangeKernel(
    MKProgram mk_program,
    cl_command_queue command_queue,
    cl_kernel kernel,
    cl_uint work_dim,
    const size_t *global_work_offset,
    const size_t *global_work_size,
    const size_t *local_work_size,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event event);

// flush / finish
MKErrorType MKProgram_AttachCLFlush(
    MKProgram mk_program,
    cl_command_queue commandQueue);

MKErrorType MKProgram_AttachCLFinish(
    MKProgram mk_program,
    cl_command_queue commandQueue);

#ifdef __cplusplus
}
#endif

#endif  /* __MK_PROGRAM_CL_H__ */
