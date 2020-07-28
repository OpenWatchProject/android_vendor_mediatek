#ifndef __DPBLOCK2NV12CONVERT_H__
#define __DPBLOCK2NV12CONVERT_H__
#include <stdio.h>
#include <ion.h>

class DpConvertBlk_NV12 {
public:
    #define MAX_FHD_BLK_SIZE (1920 * 1088 * 3 >> 1)
    DpConvertBlk_NV12();
    ~DpConvertBlk_NV12();

    bool isSupportBlk() { return m_rdma_support_blk; }
    bool isSkipB2R() { return m_skip_b2r; }
    int allocInternalBuffer(bool isSecure);
    void setSrcBufferInfo(void *pSourceYBuffer,
                          size_t bufferWidth,
                          size_t bufferHeight,
                          bool secure);
    int convertBlk_NV12();
    void *mapInternalBufferVA(void);
    int flushInternalBuffer();
    int getInternalBufferFD();
    static DpConvertBlk_NV12 *getInstance();
    static void destoryInstance();
private:
    bool   m_rdma_support_blk;
    bool   m_skip_b2r;
    size_t m_width;
    size_t m_height;

    // block buffer info
    unsigned char *m_pSourceBuffer;  // normal block buffer va
    int m_sourceBufferHandle; // secure block buffer handle


    // internal buffer info
    int m_internalBufferFD; //ion shareFd
    int m_ionHandle;  //ion file fd
    ion_user_handle_t m_internalBufferHandle;    //ion handle
    void *m_internalBufferVA;

    bool m_isSecure;

    // internal secure buffer info
    int m_internalBufferSecureHandle;  // secure buffer handle
    #define MEM_TAG "mdp internal secure buffer tag"
    #define TZCMD_CONVERT_NV12BLK_NV12 (140)
    #define TZ_TA_MDP_UUID "tz_ta_mdp_convert_blk_nv12_uuid"
};


#endif
