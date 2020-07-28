public:
friend void *MtkOmxVencDequeThread(void *pData);

private:
typedef enum {
    ENCODE_START = 1<<0, // 1: START to Q/DQ Buffer
    ENCODE_FLUSHING = 1<<1, // 1: START + FLUSH, someone is waiting flush done event
    ENCODE_EOS = 1<<2, // 1: EOS received, component should end soon
    ENCODE_FATAL_ERR = 1<<3, // 1: error happened, component should stop now
};
bool flushToEnd;
bool bBsHasEos;
unsigned int mEncodeFlags;
unsigned int mDqFrameCount;
unsigned int mDqBsCount;

inline unsigned int EnquedFrameCount() {return mFrameCount - ((mDqFrameCount > mDqBsCount)? mDqBsCount : mDqFrameCount);}
inline unsigned int EnquedBsCount() {return mBsCount - mDqBsCount;}

pthread_mutex_t mDqFrmLock;
pthread_cond_t mDqFrmCond;
//pthread_mutex_t mDqBsLock;
pthread_mutex_t mDqCvtLock;
DefaultKeyedVector<int, OMX_BUFFERHEADERTYPE*> mFrmBufferTable = DefaultKeyedVector<int, OMX_BUFFERHEADERTYPE*>(0);
//DefaultKeyedVector<int, OMX_BUFFERHEADERTYPE*> mBsBufferTable(0);
DefaultKeyedVector<int, OMX_U8*> mCvtBufferTable = DefaultKeyedVector<int, OMX_U8*>(0);
unsigned int mQueuedBsBuffers;

pthread_t mVencDqThread;
VAL_UINT32_T mVencDqThreadTid;

pthread_cond_t mFlushDoneCond;
pthread_mutex_t mFlushDoneCondLock;

int shouldThreadStop();

void MtkOmxVencDeque();
void deMtkOmxVencDeque();
void HandleDequeError(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
OMX_ERRORTYPE ComponentInitDeque();
OMX_ERRORTYPE ComponentDeinitDeque();

void waitIfNoEnqueued();
OMX_BOOL startFlush(bool end = false);
OMX_BOOL startFlush(OMX_BUFFERHEADERTYPE* pInputBuf, bool end = false);
OMX_BOOL waitFlushDone();
OMX_BOOL notifyFlushDone();
bool isFlushingState();
bool isFlushDoneState();
bool isEosState();
bool isFatalError();
OMX_BOOL startDeque(int i_idx, OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8 *pCvtBuf, int o_idx, OMX_BUFFERHEADERTYPE *pOutputBuf);
OMX_BOOL startDeque(int i_idx, OMX_BUFFERHEADERTYPE *pInputBuf, int o_idx, OMX_BUFFERHEADERTYPE *pOutputBuf);
OMX_BOOL startDeque();
OMX_BOOL doDeque();
OMX_BOOL doDequeFrame(OMX_BUFFERHEADERTYPE **ppInputBuf, OMX_BUFFERHEADERTYPE **ppOutputBuf);
OMX_BOOL tryNotifyFlushDone(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
OMX_BOOL shouldNotifyFlushDone(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
OMX_BOOL requeueFrameBuffers();
OMX_BOOL requeueBsBuffers();
OMX_BOOL doDequeInputFrame(OMX_BUFFERHEADERTYPE **ppInputBuf);
OMX_BOOL doDequeOutputFrame(OMX_BUFFERHEADERTYPE **ppOutputBuf);
OMX_BOOL epilogueDequeInputFrame(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8* pCvtBuf);
OMX_BOOL epilogueDequeOutputFrame(OMX_BUFFERHEADERTYPE *pOutputBuf);

void DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf, void* dqbuf);
void DequeOnGetConvertBuffer(OMX_U8** ppCvtBuf, void* dqbuf);
void DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf, void* dqbuf);

