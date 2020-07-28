#ifndef __RMC_AGPS_REQUEST_HANDLER_H__
#define __RMC_AGPS_REQUEST_HANDLER_H__

#include <telephony/mtk_ril.h>
#include "RfxBaseHandler.h"

#define AGPS_TAG "agps-ril"

class RmcAgpsRequestHandler : public RfxBaseHandler {
    RFX_DECLARE_HANDLER_CLASS(RmcAgpsRequestHandler);

    public:
        RmcAgpsRequestHandler(int slot_id, int channel_id);
        virtual ~RmcAgpsRequestHandler();

    protected:
        virtual void onHandleRequest(const sp<RfxMclMessage>& msg);

    private:
        void requestAgpsConnind(const sp<RfxMclMessage>& msg);
};

#endif /* __RMC_AGPS_REQUEST_HANDLER_H__ */