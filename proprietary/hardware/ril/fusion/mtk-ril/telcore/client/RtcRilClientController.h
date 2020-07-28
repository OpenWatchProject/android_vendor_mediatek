/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RTC_RIL_CLIENT_CONTROLLER_H__
#define __RTC_RIL_CLIENT_CONTROLLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxController.h"
#include "RfxTimer.h"
#include "RilClient.h"
#include "RilClientConstants.h"
#include "RilClientQueue.h"
/*****************************************************************************
 * Class RfxController
 *****************************************************************************/

using namespace std;

typedef enum {
    CLIENT_TYPE_DEFAULT,
    CLIENT_TYPE_OEM,
    CLIENT_TYPE_MTTS1,
    CLIENT_TYPE_MTTS2,
    CLIENT_TYPE_MTTS3,
    CLIENT_TYPE_MTTS4,
    CLIENT_TYPE_ATCI,
} RilClientType;

struct ClientInformation
{
   int identity;
   char* socketName;
   RilClientType type;
};

class RtcRilClientController : public RfxController {
    RFX_DECLARE_CLASS(RtcRilClientController);

    public:

        RtcRilClientController();
        virtual ~RtcRilClientController();
        static int queryFileDescriptor(int clientId);
        static RilClient* findClientWithId(int clientId);
        static bool onClientRequestComplete(RIL_Token token, RIL_Errno e, void *response,
                size_t responselen, int clientId);
        static bool onClientUnsolicitedResponse(int slotId, int urcId,
            void *response, size_t responselen, UrcDispatchRule rule);
        static int sendResponse (Parcel& parcel, int fd);
        static int blockingWrite(int fd, const void *buffer, size_t len);
        static RilClientQueue* clientHead;
        static int getCdmaSlotId();

    protected:

        virtual void onInit();
        virtual void initRilClient();
        void registerRilClient(RilClient* client);
        void deregisterRilClient(RilClient* client);
        void startListenSocket(ClientInformation information);

};

#endif /* __RP_RADIO_CONTROLLER_H__ */

