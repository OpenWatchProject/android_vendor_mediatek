#ifndef __RMC_AGPS_URC_HANDLER_H__
#define __RMC_AGPS_URC_HANDLER_H__

#include "RfxBaseHandler.h"
#include "RfxMessageId.h"
#include "RfxIntsData.h"

#define AGPS_TAG "agps-ril"

class RmcAgpsURCHandler : public RfxBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcAgpsURCHandler);

    public:
        RmcAgpsURCHandler(int slot_id, int channel_id);
        virtual ~RmcAgpsURCHandler();

    protected:
        virtual void onHandleUrc(const sp<RfxMclMessage>& msg);

    private:
        static const char *agps_urc[];

        void handleAgpsUrc(int isConnect);
};

#endif /* __RMC_AGPS_URC_HANDLER_H__ */