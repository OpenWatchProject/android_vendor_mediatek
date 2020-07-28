#ifndef _VTMSGDISPATCHER_H
#define _VTMSGDISPATCHER_H

#include "VTDef.h"

using namespace android;

namespace VTService {

struct VTMsgHdlr : public AHandler {
 public:
  VTMsgHdlr(VT_Callback cb) {
      mCb = cb;
  };
  virtual ~VTMsgHdlr(){};
 protected:
  virtual void onMessageReceived(const sp<AMessage> &msg);
  void handleMessage(const sp<AMessage> &msg);
 private:
  VT_Callback  mCb;
};

struct VTPriMsgHdlr : public AHandler {
 public:
  VTPriMsgHdlr(sp<VTMsgHdlr> handler, VT_Callback cb) {
      mCb = cb;
      mHandler = handler;
  };
  virtual ~VTPriMsgHdlr(){};
  void clear();

 protected:
  void onMessageReceived(const sp<AMessage> &msg);

 private:
  VT_Callback  mCb;
  sp<VTMsgHdlr> mHandler;
  void dispatchMessage(const sp<AMessage> &msg);
};

class VTMsgDispatcher : public RefBase {
 public:
  VTMsgDispatcher(VT_Callback cb);
  virtual ~VTMsgDispatcher();
  void dispatchMessage(int type, unsigned char* buf, int len);

 private:
  void init(VT_Callback cb);

  sp<VTPriMsgHdlr>           mVTPriMsgHdlr;
  sp<VTMsgHdlr>              mVTMsgHdlr;
  sp<ALooper>                mDispatchLooper;
  sp<ALooper>                mHandleLooper;
  VT_Callback                mCb;

};

typedef struct {
  sp<VTMsgDispatcher>                 mDispatcher;
}vt_srv_msg_struct;

}

#endif
