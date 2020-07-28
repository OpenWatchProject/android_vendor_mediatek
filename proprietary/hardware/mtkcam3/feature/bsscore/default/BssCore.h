#ifndef __BSSCORE_H__
#define __BSSCORE_H__

#include <mtkcam3/feature/bsscore/IBssCore.h>
#include <MTKBss.h>
#include <mtkcam/utils/hw/IFDContainer.h>

namespace BSScore{

    static const int MFC_GMV_CONFX_TH = 25;
    static const int MFC_GMV_CONFY_TH = 25;
    static const int MAX_GMV_CNT = MAX_FRAME_NUM;

    struct GMV{
        MINT32 x = 0;
        MINT32 y = 0;
    };

class BssCore : public IBssCore {

public:
    BssCore();
    ~BssCore();
    MVOID Init(MINT32 SensorIndex);
    MBOOL doBss(Vector<RequestPtr>& rvReadyRequests, Vector<MUINT32>& BSSOrder, int& frameNumToSkip);
    const std::map<MINT32, MINT32>& getExifDataMap();

private:

protected:

private:
    mutable Mutex mNvramChunkLock;
    MINT32  mSensorIndex;
    MINT32  mDebugLevel;
    MINT32  mMfnrQueryIndex;
    MINT32  mMfnrDecisionIso;
    MINT32  mDebugDump;
    MINT32  mEnableBSSOrdering;
    MINT32  mDebugDrop;
    MINT32  mDebugLoadIn;
    std::map<MINT32, MINT32> mExifData;

};// class BssCore

} // namespace BSScore

#endif//__BSSCORE_H__
