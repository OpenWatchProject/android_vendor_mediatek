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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MAIN_SECURITY_UTILS_ION_HELPER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MAIN_SECURITY_UTILS_ION_HELPER_H_

#ifndef USING_MTK_ION
#error "Need MTK ION to support secure memory allocation"
#else
#include <ion/ion.h>
#endif

#include <sys/mman.h>

#include <utility>
#include <mutex>

namespace NSCam {
namespace security {

// ------------------------------------------------------------------------

class IonHelper final
{
public:
    IonHelper();
    ~IonHelper();

    /**
     * mmap() is a helper of mmap() system call that creates a new mapping in
     * the virtual address sapce of the calling process from a file descriptor
     * created by ION.
     *
     * @param addr
     *      If addr is NULL, then the kernel chooses the address at which to
     *      create the mapping; this is the most portable method of creating
     *      a  new  mapping.  If addr is not NULL, then the kernel takes it
     *      as a hint about where to place the mapping.
     * @param length
     *      The length argument specifies the length of the mapping.
     * @param prot
     *      describes the desired memory protection of the mapping
     *      (and must not conflict with the open mode of the file).
     *      It is either PROT_NONE or the bitwise OR of one or
     *      more of the following flags:
     *
     *      PROT_EXEC  Pages may be executed.
     *
     *      PROT_READ  Pages may be read.
     *
     *      PROT_WRITE Pages may be written.
     *
     *      PROT_NONE  Pages may not be accessed.
     * @param flags
     *      The flags argument determines whether updates to the mapping are
     *      visible to other processes mapping the same region, and whether
     *      updates are carried through to the underlying file.
     *      This behavior is determined by including exactly one of
     *      the following values in flags:
     *
     *      MAP_SHARED
     *          Share this mapping.
     *          Updates to the mapping are visible to other processes
     *          that map this file, and are carried through to the
     *          underlying file.  The file may not actually be updated
     *          until msync(2) or munmap() is called.
     *
     *      MAP_PRIVATE
     *          Create a private copy-on-write mapping.
     *          Updates to the mapping are not visible to other processes
     *          mapping the same file, and are not carried through to the
     *          underlying file.  It is unspecified whether changes made to
     *          the file after the mmap() call are visible in the mapped region.
     *
     *
     *      Please refers to http://man7.org/linux/man-pages/man2/mmap.2.html
     *      for the reset flags available.
     * @param shared_fd
     * @param offset
     *      The contents of a file mapping are initialized using \p length bytes
     *      starting at \p offset offset in the file (or other object) referred
     *      to by the file descriptor \p shared_fd.
     *      \p offset must be a multiple of the page size as returned by
     *      sysconf(_SC_PAGE_SIZE).
     */
    void* mmap(void *addr, size_t length, int prot, int flags,
            int share_fd, off_t offset = 0);

    /**
     * munmap() is a helper of munmap() system call that deletes the mappings
     * for the specified address range, and causes further references to
     * addresses within the range to generate invalid memory references.
     * The region is also automatically unmapped when the process is terminated.
     * On the other hand, closing the file descriptor does not unmap the region.

     * The address \p addr must be a multiple of the page size.
     * All pages containing a part of the indicated range are unmapped, and sub‐
     * sequent references to these pages will generate SIGSEGV.
     * It is not an error if the indicated range does not contain any mapped pages.
     */
    int munmap(void *addr, size_t length);

    void dumpBuffer(int fd, size_t size, int width, int height,
            size_t stride, int format, bool isSecure = false);

private:
    std::pair<int, std::mutex> mIonDevFd;
    static const char *kDumpPath;
    static const char *kMemSrvName;
    static int64_t sDumpFileCounter;
}; // class SecureCameraProxy

} // namespace security
} // namespace NSCam

#endif // _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_MAIN_SECURITY_UTILS_IONHELPER_H_
