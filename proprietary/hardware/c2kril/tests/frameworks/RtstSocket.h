/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef __RTST_SOCKET_H__
#define __RTST_SOCKET_H__


/*****************************************************************************
 * Class RtstFd
 *****************************************************************************/
/*
 * Class to hold a fd for read/write
 */
class RtstFd {
// External Method
public:
    // write buffer to socket.
    //
    // RETURNS: the write length or -1 when error
    int write(
        void *buf, // [IN] the buffer need to be write
        int len    // [IN] the lengh need to be write
    ) const;

    // Read data from a socket
    //
    // RETURNS: the read length or -1 when error
    int read(
        void *buf, // [IN] the buffer need to be read
        int len    // [IN] the buffer length
    ) const ;

    // Get the file descripter of the socket
    //
    // RETURNS: fd
    int getFd() const {
        return mFd;
    }

    // Set the file descripter
    void setFd(int fd) {mFd = fd;}

// Constructor / Destructor
public:
    RtstFd(int fd);
    virtual ~RtstFd();

private:
    int mFd;
};

/*****************************************************************************
 * Class RtstSocketPair
 *****************************************************************************/
/*
 * Class to wrap a socket paire
 */
class RtstSocketPair {
// External Method
public:
    // Get the first socket fd
    //
    // RETURNS: the first socket fd
    const RtstFd &getSocket1() const {
        return m_s1;
    }

    // Get the second socket fd
    const RtstFd &getSocket2() const {
        return m_s2;
    }

// Constructor / Destructor
public:
    RtstSocketPair();
    ~RtstSocketPair() {}

// Implementation
private:
    RtstFd m_s1;
    RtstFd m_s2;
};

#endif /* __RTST_SOCKET_H__ */