/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * File name:  RfxSlotRootController.h
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Define the prototypes of slot root controller class which
 *  is the class of root object of one controller tree for
 *  a slot.
 */

#ifndef __RFX_SLOT_ROOT_CONTROLLER_H__
#define __RFX_SLOT_ROOT_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "utils/Vector.h"
#include "RfxController.h"

using ::android::Vector;
/*****************************************************************************
 * Class RfxSlotRootController
 *****************************************************************************/

class RfxSlotRootController : public RfxController
{
    RFX_DECLARE_CLASS(RfxSlotRootController);

// Constructor / Destructor
public:

    RfxSlotRootController() :
        m_cs_requests(NULL),
        m_ps_requests(NULL) {
    }

    explicit RfxSlotRootController(int slot_id);

    virtual ~RfxSlotRootController();

    void regReqToCsRild(const int *request_id_list, size_t length);

    void regReqToPsRild(const int *request_id_list, size_t length);

    void unregReqToCsRild(const int *request_id_list, size_t length) {
        unregisterRequestInternal(request_id_list, length, m_cs_requests);
    }

    void unregReqToPsRild(const int *request_id_list, size_t length) {
        unregisterRequestInternal(request_id_list, length, m_ps_requests);
    }

protected:

    virtual bool onHandleRequest(const sp<RfxMessage>& message);

    virtual bool onHandleResponse(const sp<RfxMessage>& message);


private:

    void registerRequestInternal(const int *request_id_list, size_t length,
                                 Vector<int> *list);

    void unregisterRequestInternal(const int *request_id_list, size_t length,
                                 Vector<int> *list);

private:

    Vector<int> *m_cs_requests;

    Vector<int> *m_ps_requests;
};


#endif /* __RFX_SLOT_ROOT_CONTROLLER_H__ */

