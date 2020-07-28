#define BUFCTRL_MEM_NEW(dstPtr, type, size, memInfo)\
do {\
    memInfo.NPipeAllocMemSum += size; \
    dstPtr = new type;\
} while(0)
#define BUFCTRL_MEM_DEL(dstPtr, size, memInfo)\
do{\
    memInfo.NPipeFreedMemSum += size; \
    delete dstPtr;\
    dstPtr = NULL;\
}while(0)



inline void BufCtrlCenter::initBufCtrl()
{
    int i, j;
    BfcNGlock.lock();
    if (0 == mBfcUsrCnt) {
        for (i=0; i<IQSenSum; i++) {
            mSLinkInfo[i].bInit = MFALSE;
            mNLinkInfo[i].bInit = MFALSE;
            BfcFSM[i].mFsm = BufCtrl_init;
            memset((void*)mSLinkInfo[i].mpUnOrderEnqContainer, 0, sizeof(mSLinkInfo[i].mpUnOrderEnqContainer));
            for (j = 0; j < ePort_max; j++) {
                BUFCTRL_MEM_NEW(mSLinkInfo[i].mpUnOrderEnqContainer[j], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>), NormalPipe::mMemInfo);
                mSLinkInfo[i].mpUnOrderEnqContainer[j]->init();
                mSLinkInfo[i].mNextEnqIdx[j] = 0;
                mSLinkInfo[i].mNextDeqIdx[j] = 0;
            }
            mNLinkInfo[i].mpDeqContainer = NULL;
            BUFCTRL_MEM_NEW(mNLinkInfo[i].mpDeqContainer, QueueMgr<NDeqInfo>(), sizeof(QueueMgr<NDeqInfo>), NormalPipe::mMemInfo);
            mNLinkInfo[i].mpDeqContainer->init();
            mNLinkInfo[i].mvSttMem.clear();
        }
        mBfcNSwitchCnt = 0;
        mBfcSSwitchCnt = 0;
        bSwitch        = MFALSE;
        IQNotify       = NULL;
        mSLinkCfgNum   = 0;
        mSLinkTgSize.w = 0;
        mSLinkTgSize.h = 0;
    }
    mBfcUsrCnt++;
    PIPE_INF("BufCtrlCenter init: mBfcUsrCnt(%d)\n", mBfcUsrCnt);
    BfcNGlock.unlock();
}

inline void BufCtrlCenter::uninitBufCtrl(MUINT32 sensorIdx)
{
    int i, j;
    BfcNGlock.lock();
    if(bSwitch == MFALSE){
        for(i = 0 ; i < IQSenSum ; i++){
            if(mNLinkInfo[i].mpSenIdx == sensorIdx && mNLinkInfo[i].bInit == MTRUE){
                mNLinkInfo[i].bInit = MFALSE;
                BfcFSM[i].mFsm = BufCtrl_init;

                mNLinkInfo[i].mpDeqContainer->uninit();
                mNLinkInfo[i].mpCamIO = NULL;

                mBfcUsrCnt--;

                PIPE_INF("BufCtrlCenter uninit(nlink): index(%d)\n", i);
            }
            if(mSLinkInfo[i].mpSenIdx == sensorIdx && mSLinkInfo[i].bInit == MTRUE){
                mSLinkInfo[i].bInit = MFALSE;

                for(j = 0 ; j < ePort_max ; j++){
                    mSLinkInfo[i].PA_L[j].clear();
                    mSLinkInfo[i].mpUnOrderEnqContainer[j]->uninit();
                    mSLinkInfo[i].mNextEnqIdx[j] = 0;
                    mSLinkInfo[i].mNextDeqIdx[j] = 0;
                }
                mSLinkInfo[i].mpSttIO = NULL;
                mSLinkInfo[i].mpImgHdrMgr = NULL;
                mSLinkInfo[i].mpMInf = NULL;

                mSLinkCfgNum--;

                PIPE_INF("BufCtrlCenter uninit(slink): index(%d)\n", i);
            }
        }
    }
    else{
        mBfcUsrCnt--;
    }
    if(mBfcUsrCnt == 0){
        for(i = 0 ; i < IQSenSum ; i++){
            for(j = 0 ; j < ePort_max ; j++){
                mSLinkInfo[i].mpUnOrderEnqContainer[j]->uninit();
                BUFCTRL_MEM_DEL(mSLinkInfo[i].mpUnOrderEnqContainer[j], sizeof(QueueMgr<QBufInfo>), NormalPipe::mMemInfo);
                mSLinkInfo[i].mpUnOrderEnqContainer[j] = NULL;
            }
            mNLinkInfo[i].mpDeqContainer->uninit();
            BUFCTRL_MEM_DEL(mNLinkInfo[i].mpDeqContainer, sizeof(QueueMgr<NDeqInfo>), NormalPipe::mMemInfo);
            mNLinkInfo[i].mpDeqContainer = NULL;
        }
        bSwitch = MFALSE;
        IQNotify = NULL;
        mBfcNSwitchCnt = 0;
        mBfcSSwitchCnt = 0;
        mSLinkCfgNum   = 0;
        mSLinkTgSize.w = 0;
        mSLinkTgSize.h = 0;
    }
    PIPE_INF("BufCtrlCenter uninit: bSwitch:%d mBfcUsrCnt(%d)\n", bSwitch, mBfcUsrCnt);
    BfcNGlock.unlock();
}

inline NSImageio::NSIspio::ICamIOPipe* BufCtrlCenter::GetCamIOPipeInstance(MUINT32 sensorIdx)
{
    NSImageio::NSIspio::ICamIOPipe* pCamIOPipe = NULL;
    MUINT32 idx = 0;

    for(idx = 0 ; idx < IQSenSum ; idx++){
        if(mNLinkInfo[idx].mpSenIdx == sensorIdx){
            pCamIOPipe = mNLinkInfo[idx].mpCamIO;
            break;
        }
    }
    if(idx == IQSenSum){
        PIPE_ERR("unexpected sensorIdx:%d", sensorIdx);
    }

    return pCamIOPipe;
}

inline NSImageio::NSIspio::ICamIOPipe* BufCtrlCenter::GetSttIOPipeInstance(MUINT32 sensorIdx)
{
    NSImageio::NSIspio::ICamIOPipe* pSttIOPipe = NULL;
    MUINT32 idx = 0;

    for(idx = 0 ; idx < IQSenSum ; idx++){
        if(mSLinkInfo[idx].mpSenIdx == sensorIdx){
            pSttIOPipe = mSLinkInfo[idx].mpSttIO;
            break;
        }
    }
    if(idx == IQSenSum){
        PIPE_ERR("unexpected sensorIdx:%d", sensorIdx);
    }

    return pSttIOPipe;
}

inline NSImageio::NSIspio::ICamIOPipe* BufCtrlCenter::PopDeqCamIOInstance(MUINT32 sensorIdx)
{
    NSImageio::NSIspio::ICamIOPipe* pCamIOPipe = NULL;
    MUINT32 curIdx = 0, otherIdx = 0, qSize = 0, qIdx = 0;
    NDeqInfo curDeqInfo, otherDeqInfo;

    for(curIdx = 0 ; curIdx < IQSenSum ; curIdx++){
        if(mNLinkInfo[curIdx].mpSenIdx == sensorIdx){
            otherIdx = (curIdx == 0) ? 1 : 0;
            mNLinkInfo[curIdx].mpDeqContainer->sendCmd(QueueMgr<NDeqInfo>::eCmd_front, (MUINTPTR)&curDeqInfo, 0);

            // another cam not init yet: single cam case, so no need to wait
            if(mNLinkInfo[otherIdx].bInit == MFALSE){
                pCamIOPipe = curDeqInfo.pCamIO;
                mNLinkInfo[curIdx].mpDeqContainer->sendCmd(QueueMgr<NDeqInfo>::eCmd_pop, 0, 0);
                goto EXIT;
            }

            // dual cam case, but not under switch case, so no need to wait
            if(curDeqInfo.bWait == MFALSE){
                pCamIOPipe = curDeqInfo.pCamIO;
                mNLinkInfo[curIdx].mpDeqContainer->sendCmd(QueueMgr<NDeqInfo>::eCmd_pop, 0, 0);
                goto EXIT;
            }

            // dual cam case, need to check whether another cam finish all deque
            mNLinkInfo[otherIdx].mpDeqContainer->sendCmd(QueueMgr<NDeqInfo>::eCmd_getsize, (MUINTPTR)&qSize, 0);
            for(qIdx = 0 ; qIdx < qSize ; qIdx++){
                mNLinkInfo[otherIdx].mpDeqContainer->sendCmd(QueueMgr<NDeqInfo>::eCmd_at, qIdx, (MUINTPTR)&otherDeqInfo);
                if(otherDeqInfo.pCamIO == curDeqInfo.pCamIO){
                    pCamIOPipe = NULL;
                    goto EXIT;
                }
            }
            if(qIdx == qSize){
                pCamIOPipe = curDeqInfo.pCamIO;
                mNLinkInfo[curIdx].mpDeqContainer->sendCmd(QueueMgr<NDeqInfo>::eCmd_pop, 0, 0);
                goto EXIT;
            }
            break;
        }
    }
    if(curIdx == IQSenSum){
        PIPE_ERR("unexpected sensorIdx:%d", sensorIdx);
    }

EXIT:
    return pCamIOPipe;
}
inline void BufCtrlCenter::PushDeqCamIOInstance(MUINT32 sensorIdx, NSImageio::NSIspio::ICamIOPipe* pCamIO, MBOOL bWait)
{
    MUINT32 idx = 0;

    for(idx = 0 ; idx < IQSenSum ; idx++){
        if(mNLinkInfo[idx].mpSenIdx == sensorIdx){
            NDeqInfo deqInfo;
            deqInfo.pCamIO = pCamIO;
            deqInfo.bWait   = bWait;
            mNLinkInfo[idx].mpDeqContainer->sendCmd(QueueMgr<NDeqInfo>::eCmd_push, (MUINTPTR)&deqInfo, 0);
            break;
        }
    }
    if(idx == IQSenSum){
        PIPE_ERR("unexpected sensorIdx:%d", sensorIdx);
    }
}

inline MBOOL BufCtrlCenter::CheckSwitch()
{
    MBOOL bResult = MFALSE;
    this->BfcNGlock.lock();
    bResult = bSwitch;
    this->BfcNGlock.unlock();
    return bResult;
}

inline MUINT32 BufCtrlCenter::GetCfgNum()
{
    return mSLinkCfgNum;
}

inline void BufCtrlCenter::AddSttEnqIdx(SLinkInfo* pslink, MUINT32 portIdx)
{
    pslink->mNextEnqIdx[portIdx] = (pslink->mNextEnqIdx[portIdx] + 1)% ((pslink->mpMInf+portIdx)->size());
}

inline void BufCtrlCenter::AddSttDeqIdx(SLinkInfo* pslink, MUINT32 portIdx)
{
    pslink->mNextDeqIdx[portIdx] = (pslink->mNextDeqIdx[portIdx] + 1)% ((pslink->mpMInf+portIdx)->size());
}

inline void BufCtrlCenter::ResetSttEnqDeqIdx(SLinkInfo* pslink, MUINT32 portIdx)
{
    pslink->mNextEnqIdx[portIdx] = 0;
    pslink->mNextDeqIdx[portIdx] = 0;
}

inline MUINTPTR BufCtrlCenter::GetSttNextEnqPA(SLinkInfo* pslink, MUINT32 portIdx)
{
    list<MUINTPTR>* pImgPA_L;
    list<MUINTPTR>::iterator it;
    MUINT32 idx = 0;
    pImgPA_L = (list<MUINTPTR>*)&pslink->PA_L[portIdx];
    it = pImgPA_L->begin();
    while(idx != pslink->mNextEnqIdx[portIdx]) {
        idx++;
        it++;
    }
    return *it;
}

inline MUINTPTR BufCtrlCenter::GetSttNextDeqPA(SLinkInfo* pslink, MUINT32 portIdx)
{
    list<MUINTPTR>* pImgPA_L;
    list<MUINTPTR>::iterator it;
    MUINT32 idx = 0;
    pImgPA_L = (list<MUINTPTR>*)&pslink->PA_L[portIdx];
    it = pImgPA_L->begin();
    while(idx != pslink->mNextDeqIdx[portIdx]) {
        idx++;
        it++;
    }
    return *it;
}

inline void BufCtrlCenter::SetMaxTgSize(MSize* _pTgSize)
{
    if (mSLinkCfgNum < IQSenSum)
        mSLinkCfgNum++;
    else {
        PIPE_ERR("unexpected CfgNum %d\n", mSLinkCfgNum);
    }
    if (_pTgSize->w * _pTgSize->h > mSLinkTgSize.w * mSLinkTgSize.h) {
        mSLinkTgSize.w = _pTgSize->w;
        mSLinkTgSize.h = _pTgSize->h;
    }
}

inline void BufCtrlCenter::GetMaxTgSize(MSize* _pTgSize)
{
    _pTgSize->w = mSLinkTgSize.w;
    _pTgSize->h = mSLinkTgSize.h;
}

inline MBOOL BufCtrlCenter::sendcmdBufCtrl(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE;
    MUINT32 npipe_path;
    NSImageio::NSIspio::ICamIOPipe  *mpTmpIO;
    MUINT32 tmp;
    E_INPUT tgIdx;
    MBOOL offBin, offTwin;
    MUINT32 lockIdx;

    switch (cmd) {

        case BUF_CTRL_CMD_GET_IO_BY_PA:
            {
                // arg1: In:    PortId
                // arg2: In:    PA
                // arg3: Out:   STT imageIO instance pointer
                MUINT32 PortIdx = *(MUINT32*)arg1;
                MUINTPTR PA      = *(MUINTPTR*)arg2;
                MUINT32 i;
                list<MUINTPTR>* pImgPA_L;
                list<MUINTPTR>::iterator it;
                for (i = 0 ; i < IQSenSum ; i++) {
                    pImgPA_L = (list<MUINTPTR>*)&mSLinkInfo[i].PA_L[PortIdx];
                    for(it = pImgPA_L->begin(); it != pImgPA_L->end() ; it++){
                        if (PA == (*it)) {
                            *(SLinkInfo**)arg3 = &mSLinkInfo[i];
                            goto _EXIT_SEND_CMD;
                        }
                    }
                }
                ret = MFALSE;
                PIPE_ERR("error: BufCtrl sendCommand fail: no Buf found Port(%d), PA(0x%08x)", PortIdx, PA);
            }
            break;
        case BUF_CTRL_CMD_SET_SLINK:
            {
                this->BfcSGlock.lock();
                SLinkInfo* pslink = (SLinkInfo*)arg1;
                MUINT32    idx = (mSLinkInfo[0].bInit) ? 1 : 0;
                MUINT32    i;
                list<MUINTPTR>::iterator it;
                // arg1: In: SLinkInfo* to keep
                mSLinkInfo[idx].mpSttIO     = pslink->mpSttIO;
                mSLinkInfo[idx].mpSenIdx    = pslink->mpSenIdx;
                mSLinkInfo[idx].mpImgHdrMgr = pslink->mpImgHdrMgr;
                mSLinkInfo[idx].mpMInf      = pslink->mpMInf;
                mSLinkInfo[idx].mLockIdx    = idx;
                for(i = 0; i < ePort_max; i++){
                    mSLinkInfo[idx].PA_L[i].clear();
                    for(it = pslink->PA_L[i].begin() ; it != pslink->PA_L[i].end() ; it++){
                        mSLinkInfo[idx].PA_L[i].push_back(*it);
                    }
                }
                mSLinkInfo[idx].bInit       = MTRUE;
                this->BfcSGlock.unlock();
            }
            break;

        case BUF_CTRL_CMD_GET_SWITCH_SLINK:
            {
                this->BfcSGlock.lock();
                // arg1: In:    ICamIOPipe*, to specify current ask cam
                // arg2: Out:   CamLinInfo*, to specify the other LinkInfo information
                if (*(NSImageio::NSIspio::ICamIOPipe**)arg1 == mSLinkInfo[0].mpSttIO)
                    *(SLinkInfo**)arg2 = &mSLinkInfo[1];
                else
                    *(SLinkInfo**)arg2 = &mSLinkInfo[0];
                this->BfcSGlock.unlock();
            }
            break;

        case BUF_CTRL_CMD_SWITCH_SLINK:
            {
                this->BfcSGlock.lock();
                // switch CamIO for SPipe
                if(mBfcSSwitchCnt == 0){
                    mBfcSSwitchCnt++;
                }
                else if (mBfcSSwitchCnt == 1){
                    tmp = mSLinkInfo[0].mpSenIdx;
                    mSLinkInfo[0].mpSenIdx = mSLinkInfo[1].mpSenIdx;
                    mSLinkInfo[1].mpSenIdx = tmp;
                    mBfcSSwitchCnt = 0;
                }
                else{
                    PIPE_ERR("unexpected bfc switch value:%d", mBfcSSwitchCnt);
                }
                this->BfcSGlock.unlock();
            }
            break;

        case BUF_CTRL_CMD_GET_CUR_SLINK:
            {
                // arg1: In:    ICamIOPipe*, to specify current ask cam
                // arg2: Out:   CamLinInfo*, to specify current LinkInfo information
                this->BfcSGlock.lock();
                if (*(NSImageio::NSIspio::ICamIOPipe**)arg1 == mSLinkInfo[0].mpSttIO)
                    *(SLinkInfo**)arg2 = &mSLinkInfo[0];
                else
                    *(SLinkInfo**)arg2 = &mSLinkInfo[1];
                this->BfcSGlock.unlock();

            }
            break;

        case BUF_CTRL_CMD_SET_NLINK:
            {
                this->BfcNGlock.lock();
                NLinkInfo* pNlink = (NLinkInfo*)arg1;
                MUINT32    idx    = (mNLinkInfo[0].bInit) ? 1 : 0;
                MUINT32    cam_path = *(MUINT32*)arg2;
                // arg1: In: NLinkInfo* to keep
                mNLinkInfo[idx].mpSenIdx     = pNlink->mpSenIdx;
                mNLinkInfo[idx].mTgIdx       = pNlink->mTgIdx;
                mNLinkInfo[idx].mCurTgIdx    = pNlink->mTgIdx;
                mNLinkInfo[idx].mTgSize      = pNlink->mTgSize;
                mNLinkInfo[idx].mpCamIO      = pNlink->mpCamIO;
                mNLinkInfo[idx].mLockIdx     = idx;
                mNLinkInfo[idx].bInit        = MTRUE;
                mNLinkInfo[idx].mBufCnt      = pNlink->mBufCnt;
                mNLinkInfo[idx].pix_mode_tg  = pNlink->pix_mode_tg;
                mNLinkInfo[idx].raw_fps      = pNlink->raw_fps;
                mNLinkInfo[idx].SrcImgFmt    = pNlink->SrcImgFmt;
                mNLinkInfo[idx].bypass_rrzo  = pNlink->bypass_rrzo;
                mNLinkInfo[idx].rrz_out_w    = pNlink->rrz_out_w;
                mNLinkInfo[idx].offBin       = pNlink->offBin;
                mNLinkInfo[idx].offTwin      = pNlink->offTwin;
                mNLinkInfo[idx].pattern      = pNlink->pattern;
                mNLinkInfo[idx].MIPI_pixrate = pNlink->MIPI_pixrate;
                BfcFSM[idx].mNpipe        = (NSCam::NSIoPipe::NSCamIOPipe::NPIPE_PATH_E)cam_path;
                this->BfcNGlock.unlock();
            }
            break;

        case BUF_CTRL_CMD_GET_SWITCH_NLINK:
            {
                // arg1: In:    ICamIOPipe*, to specify current ask cam
                // arg2: Out:   CamLinInfo*, to specify the other LinkInfo information
                this->BfcNGlock.lock();
                if (*(NSImageio::NSIspio::ICamIOPipe**)arg1 == mNLinkInfo[0].mpCamIO)
                    *(NLinkInfo**)arg2 = &mNLinkInfo[1];
                else
                    *(NLinkInfo**)arg2 = &mNLinkInfo[0];
                this->BfcNGlock.unlock();
            }
            break;

        case BUF_CTRL_CMD_GET_CUR_NLINK:
            {
                // arg1: In:    ICamIOPipe*, to specify current ask cam
                // arg2: Out:   CamLinInfo*, to specify current LinkInfo information
                this->BfcNGlock.lock();
                if (*(NSImageio::NSIspio::ICamIOPipe**)arg1 == mNLinkInfo[0].mpCamIO)
                    *(NLinkInfo**)arg2 = &mNLinkInfo[0];
                else
                    *(NLinkInfo**)arg2 = &mNLinkInfo[1];
                this->BfcNGlock.unlock();
            }
            break;

        case BUF_CTRL_CMD_SWITCH_NLINK:
            {
                this->BfcNGlock.lock();
                // switch CamIO for NPipe
                if(mBfcNSwitchCnt == 0){
                    // change camio instance
                    mpTmpIO                  = mNLinkInfo[1].mpCamIO;
                    mNLinkInfo[1].mpCamIO    = mNLinkInfo[0].mpCamIO;
                    mNLinkInfo[0].mpCamIO    = mpTmpIO;
                    // change lock idx, which is bonding to camio
                    lockIdx                  = mNLinkInfo[1].mLockIdx;
                    mNLinkInfo[1].mLockIdx   = mNLinkInfo[0].mLockIdx;
                    mNLinkInfo[0].mLockIdx   = lockIdx;
                    // change current tg index
                    tgIdx                    = mNLinkInfo[1].mCurTgIdx;
                    mNLinkInfo[1].mCurTgIdx  = mNLinkInfo[0].mCurTgIdx;
                    mNLinkInfo[0].mCurTgIdx  = tgIdx;
                    // change current offBin info
                    offBin                   = mNLinkInfo[1].offBin;
                    mNLinkInfo[1].offBin     = mNLinkInfo[0].offBin;
                    mNLinkInfo[0].offBin     = offBin;
                    // change current offTwin info
                    offTwin                  = mNLinkInfo[1].offTwin;
                    mNLinkInfo[1].offTwin    = mNLinkInfo[0].offTwin;
                    mNLinkInfo[0].offTwin    = offTwin;
                    // change stt buffer
                    vector<IMEM_BUF_INFO> vSttMem;
                    vSttMem.clear();
                    for(MUINT32 i = 0 ; i < mNLinkInfo[1].mvSttMem.size() ; i++){
                        vSttMem.push_back(mNLinkInfo[1].mvSttMem.at(i));
                    }
                    mNLinkInfo[1].mvSttMem.clear();
                    for(MUINT32 i = 0 ; i < mNLinkInfo[0].mvSttMem.size() ; i++){
                        mNLinkInfo[1].mvSttMem.push_back(mNLinkInfo[0].mvSttMem.at(i));
                    }
                    mNLinkInfo[0].mvSttMem.clear();
                    for(MUINT32 i = 0 ; i < vSttMem.size() ; i++){
                        mNLinkInfo[0].mvSttMem.push_back(vSttMem.at(i));
                    }
                    //
                    mBfcNSwitchCnt++;
                    bSwitch                  = (bSwitch)?MFALSE:MTRUE;
                }
                else if(mBfcNSwitchCnt == 1){
                    mBfcNSwitchCnt++;
                }
                else{
                    PIPE_ERR("unexpected bfc switch count:%d", mBfcNSwitchCnt);
                }
                this->BfcNGlock.unlock();
            }
            break;

        default:
            ret = MFALSE;
            break;
    }
    if (ret != MTRUE) {
        PIPE_ERR("error: BufCtrl sendCommand fail: (cmd,arg1,arg2)=(0x%08x,0x%" PRIXPTR ",0x%" PRIXPTR ")", cmd, arg1, arg2);
    }

_EXIT_SEND_CMD:
    return ret;
}



inline MBOOL BufCtrlCenter::updateBufCtrlFSM(E_BufCtrlFSM const op, MUINT32 const npipe_path)
{
    // BufCtrl FSM chart
    // init ---> start --->  set ---> switch_1 ---> switch_2 ---> switch_done
    //             |    ---> suspend  (by NPipe suspend)
    //             |    <--- suspend  (by NPipe resume)
    //             |    <-----------------------------     | (by Switch done CB)
    MBOOL ret = MTRUE;

    Mutex::Autolock lock(BfcFSMlock);

    switch(op){
        case BufCtrl_switch_set:
        case BufCtrl_suspend:
            if(BfcFSM[getBfcIdx(npipe_path)].mFsm != BufCtrl_start)
                ret = MFALSE;
            break;
        case BufCtrl_switch_phase1:
            if(BfcFSM[getBfcIdx(npipe_path)].mFsm != BufCtrl_switch_set)
                ret = MFALSE;
            break;
        case BufCtrl_switch_phase2:
            if(BfcFSM[getBfcIdx(npipe_path)].mFsm != BufCtrl_switch_phase1)
                ret = MFALSE;
            break;
        case BufCtrl_switch_done:
            if(BfcFSM[getBfcIdx(npipe_path)].mFsm != BufCtrl_switch_phase2)
                ret = MFALSE;
            break;
        case BufCtrl_start:
            if(BfcFSM[getBfcIdx(npipe_path)].mFsm != BufCtrl_switch_done &&
                BfcFSM[getBfcIdx(npipe_path)].mFsm != BufCtrl_suspend &&
                BfcFSM[getBfcIdx(npipe_path)].mFsm != BufCtrl_init)
                ret = MFALSE;
            break;
        default:
            ret = MFALSE;
            break;
    }
    if (ret) {
        PIPE_INF("[%d] BufCtrl op update: cur_%d, tar_%d\n", (MUINT32)npipe_path,(MUINT32)BfcFSM[getBfcIdx(npipe_path)].mFsm, (MUINT32)op);
        BfcFSM[getBfcIdx(npipe_path)].mFsm = op;
    }
    else {
        PIPE_ERR("[%d] BufCtrl op error: cur_%d, tar_%d\n", (MUINT32)npipe_path, (MUINT32)BfcFSM[getBfcIdx(npipe_path)].mFsm, (MUINT32)op);
    }

    return ret;
}

inline MUINT32 BufCtrlCenter::getBfcIdx(MUINT32 const npipe_path)
{
    MUINT32 i;
    MUINT32 idx = IQSenSum;
    for (i = 0; i < IQSenSum; i++) {
        if (npipe_path == BfcFSM[i].mNpipe)
            idx = i;
    }
    if (IQSenSum == idx)
        PIPE_ERR("BufCtrl getBfcIdx error!\n");
    return idx;
}
