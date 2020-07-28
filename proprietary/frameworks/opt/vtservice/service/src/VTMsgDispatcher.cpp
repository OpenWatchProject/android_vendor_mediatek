#include "VTMsgDispatcher.h"
#include "VTCore.h"
#include "VTCoreHelper.h"

using namespace android;

namespace VTService{

    vt_srv_msg_struct g_vt_srv_msg_hdr;

    VTMsgDispatcher::VTMsgDispatcher(VT_Callback cb) {

        VT_LOGI("[VT] [DISPATCHER] VTMsgDispatcher Constuctor");

        mCb = cb;
        init(cb);
    }

    void VTMsgDispatcher::dispatchMessage(int type, unsigned char * buf, int len) {

        VT_LOGI("[VT] [DISPATCHER] VTMsgDispatcher dispatchMessage");

        sp<AMessage> message = new AMessage(VT_SRV_MESSAGE_DISPATCH, mVTPriMsgHdlr);
        if (len > 0) {
            sp<ABuffer> buff = new ABuffer((size_t)len);
            memcpy(buff->data(), buf, len);
            message->setBuffer("buffer", buff);
        }

        message->setInt32("type", type);
        message->setInt32("len", len);
        message->post();
    }

    void VTMsgDispatcher::init(VT_Callback cb) {

        VT_LOGI("[VT] [DISPATCHER] VTMsgDispatcher init");

        mVTMsgHdlr = new VTMsgHdlr(cb);
        mVTPriMsgHdlr = new VTPriMsgHdlr(mVTMsgHdlr, cb);

        mDispatchLooper = new ALooper();
        mDispatchLooper->setName("IMCB msg dispatcher");
        mDispatchLooper->start(false, false, PRIORITY_AUDIO);
        mDispatchLooper->registerHandler(mVTPriMsgHdlr);

        mHandleLooper = new ALooper();
        mHandleLooper->setName("IMCB msg handler");
        mHandleLooper->start(false, false, PRIORITY_AUDIO);
        mHandleLooper->registerHandler(mVTMsgHdlr);
    }

    VTMsgDispatcher::~VTMsgDispatcher() {
        g_vt_srv_msg_hdr.mDispatcher = NULL;
        mVTPriMsgHdlr->clear();
        mVTPriMsgHdlr = NULL;
        mVTMsgHdlr = NULL;
        mDispatchLooper = NULL;
        mHandleLooper = NULL;
    }

    void VTPriMsgHdlr::onMessageReceived(const sp<AMessage> &msg) {

        VT_LOGI("[VT] [PRI HDLR] VTPriMsgHdlr onMessageReceived");

        switch (msg->what()) {
            case VT_SRV_MESSAGE_DISPATCH:
                {
                    int type, len;
                    sp<ABuffer> buffer;
                    CHECK(msg->findInt32("type", &type));
                    if (type != MSG_ID_WRAP_IMSVT_IMCB_CONFIG_DEINIT_IND &&
                        type != MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_IND &&
                        type != MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CNF &&
                        type != MSG_ID_WRAP_IMSVT_IMCB_MODIFY_SESSION_CANCEL_IND &&
                        type != MSG_ID_WRAP_IMSVT_IMCB_CALL_PROGRESS_IND) {
                        dispatchMessage(msg);
                    } else {
                        CHECK(msg->findInt32("len", &len));
                        CHECK(msg->findBuffer("buffer", &buffer) && buffer.get() != NULL);
                        if (buffer->size() == (unsigned int)len) {
                            VT_LOGD("[VT] [PRI HDLR] handle priority message");
                            mCb(type, buffer->data(), len);
                        }
                    }
                    break;
                }
            default:
                break;
        }
    }

    void VTPriMsgHdlr::dispatchMessage(const sp<AMessage> &msg) {
        VT_LOGI("[VT] [PRI HDLR] VTPriMsgHdlr dispatch normal message");
        int type, len;
        sp<ABuffer> buff;
        sp<AMessage> message = new AMessage(VT_SRV_MESSAGE_HANDLING, mHandler);

        CHECK(msg->findInt32("type", &type));
        CHECK(msg->findInt32("len", &len));
        if (len > 0) {
            CHECK(msg->findBuffer("buffer", &buff) && buff.get() != NULL);
            CHECK(buff->size() == (unsigned int)len);
            sp<ABuffer> newBuff = new ABuffer((size_t)len);
            memcpy(newBuff->data(), buff->data(), len);
            message->setBuffer("buffer", newBuff);
        }

        message->setInt32("type", type);
        message->setInt32("len", len);
        message->post();
    }

    void VTPriMsgHdlr::clear() {
        VT_LOGI("[VT] [PRI HDLR] VTPriMsgHdlr clear");
        mHandler = NULL;
    }

    void VTMsgHdlr::onMessageReceived(const sp<AMessage> &msg) {
        VT_LOGI("[VT] [HDLR] VTMsgHdlr onMessageReceived");
        switch (msg->what()) {
            case VT_SRV_MESSAGE_HANDLING:
                handleMessage(msg);
                break;
            default:
                break;
        }
    }

    void VTMsgHdlr::handleMessage(const sp<AMessage> &msg) {
        VT_LOGI("[VT] [HDLR] VTMsgHdlr handle normal message");
        int type, len;
        sp<ABuffer> buff;
        CHECK(msg->findInt32("type", &type));
        CHECK(msg->findInt32("len", &len));
        if (len > 0) {
            CHECK(msg->findBuffer("buffer", &buff) && buff.get() != NULL);
            CHECK(buff->size() == (unsigned int)len);
            mCb(type, buff->data(), len);
        } else {
            mCb(type, 0, 0);
        }
    }
}

