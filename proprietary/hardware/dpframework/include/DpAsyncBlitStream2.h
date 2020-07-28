#ifndef __DP_ASYNC_BLIT_STREAM_2_H__
#define __DP_ASYNC_BLIT_STREAM_2_H__

#include "DpAsyncBlitStream.h"
#include <vector>

using namespace std;

#define PQ_ENHANCE_STEPS        1024

class DpAsyncBlitStream2
{
public:
    static bool queryHWSupport(uint32_t         srcWidth,
                               uint32_t         srcHeight,
                               uint32_t         dstWidth,
                               uint32_t         dstHeight,
                               int32_t          Orientation = 0,
                               DpColorFormat    srcFormat = DP_COLOR_UNKNOWN,
                               DpColorFormat    dstFormat = DP_COLOR_UNKNOWN,
                               DpPqParam        *PqParam = 0,
                               DpRect           *srcCrop = 0);

    DpAsyncBlitStream2();

    ~DpAsyncBlitStream2();

    DP_STATUS_ENUM createJob(uint32_t &jobID, int32_t &fence);

    DP_STATUS_ENUM cancelJob(uint32_t jobID = 0);

    DP_STATUS_ENUM setConfigBegin(uint32_t jobID,
                                  int32_t  enhancePos = PQ_ENHANCE_STEPS,
                                  int32_t  enhanceDir = 0);

    DP_STATUS_ENUM setSrcBuffer(void     *pVABase,
                                uint32_t size,
                                int32_t  fenceFd = -1);

    DP_STATUS_ENUM setSrcBuffer(void     **pVAList,
                                uint32_t *pSizeList,
                                uint32_t planeNumber,
                                int32_t  fenceFd = -1);

    // VA + MVA address interface
    DP_STATUS_ENUM setSrcBuffer(void**   pVAddrList,
                                void**   pMVAddrList,
                                uint32_t *pSizeList,
                                uint32_t planeNumber,
                                int32_t  fenceFd = -1);

    // for ION file descriptor
    DP_STATUS_ENUM setSrcBuffer(int32_t  fileDesc,
                                uint32_t *sizeList,
                                uint32_t planeNumber,
                                int32_t  fenceFd = -1);

    DP_STATUS_ENUM setSrcConfig(int32_t           width,
                                int32_t           height,
                                DpColorFormat     format,
                                DpInterlaceFormat field = eInterlace_None);

    DP_STATUS_ENUM setSrcConfig(int32_t           width,
                                int32_t           height,
                                int32_t           yPitch,
                                int32_t           uvPitch,
                                DpColorFormat     format,
                                DP_PROFILE_ENUM   profile = DP_PROFILE_BT601,
                                DpInterlaceFormat field   = eInterlace_None,
                                DpSecure          secure  = DP_SECURE_NONE,
                                bool              doFlush = true);

    DP_STATUS_ENUM setSrcCrop(int32_t portIndex,
                              DpRect  roi);

    DP_STATUS_ENUM setDstBuffer(int32_t  portIndex,
                                void     *pVABase,
                                uint32_t size,
                                int32_t  fenceFd = -1);

    DP_STATUS_ENUM setDstBuffer(int32_t  portIndex,
                                void     **pVABaseList,
                                uint32_t *pSizeList,
                                uint32_t planeNumber,
                                int32_t  fenceFd = -1);

    // VA + MVA address interface
    DP_STATUS_ENUM setDstBuffer(int32_t  portIndex,
                                void**   pVABaseList,
                                void**   pMVABaseList,
                                uint32_t *pSizeList,
                                uint32_t planeNumber,
                                int32_t  fenceFd = -1);

    // for ION file descriptor
    DP_STATUS_ENUM setDstBuffer(int32_t  portIndex,
                                int32_t  fileDesc,
                                uint32_t *pSizeList,
                                uint32_t planeNumber,
                                int32_t  fenceFd = -1);

    DP_STATUS_ENUM setDstConfig(int32_t           portIndex,
                                int32_t           width,
                                int32_t           height,
                                DpColorFormat     format,
                                DpInterlaceFormat field = eInterlace_None,
                                DpRect            *pROI = 0);

    DP_STATUS_ENUM setDstConfig(int32_t           portIndex,
                                int32_t           width,
                                int32_t           height,
                                int32_t           yPitch,
                                int32_t           uvPitch,
                                DpColorFormat     format,
                                DP_PROFILE_ENUM   profile = DP_PROFILE_BT601,
                                DpInterlaceFormat field   = eInterlace_None,
                                DpRect            *pROI   = 0,
                                DpSecure          secure  = DP_SECURE_NONE,
                                bool              doFlush = true);

    DP_STATUS_ENUM setDstROI(int32_t      portIndex,
                             int32_t      width,
                             int32_t      height,
                             const DpRect &roi);

    DP_STATUS_ENUM setConfigEnd();

    DP_STATUS_ENUM setRotate(int32_t portIndex,
                             int32_t rotation);

    //Compatible to 89
    DP_STATUS_ENUM setFlip(int32_t portIndex,
                           int flip);

    DP_STATUS_ENUM setOrientation(int32_t  portIndex,
                                  uint32_t transform);

    DP_STATUS_ENUM setPQParameter(int32_t portIndex,
                                  const DpPqParam &pqParam);

    DP_STATUS_ENUM setDither(int32_t portIndex,
                             bool    enDither);

    DP_STATUS_ENUM setUser(uint32_t eID = 0);

    DP_STATUS_ENUM invalidate(struct timeval *endTime = NULL);

    static int32_t queryPaddingSide(uint32_t transform);

    static DP_STATUS_ENUM queryTheoreticalExecTime(uint32_t    srcWidth,
                        uint32_t          srcHeight,
                        uint32_t          dstWidth,
                        uint32_t          dstHeight,
                        std::vector<uint32_t >* pExecTimeList);

private:
    struct BaseConfig
    {
        DpRect          src;

        DpRect          roi[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         dstWidth[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         dstHeight[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         rotation[ISP_MAX_OUTPUT_PORT_NUM];
        bool            flip[ISP_MAX_OUTPUT_PORT_NUM];
        DpColorFormat   dstFormat[ISP_MAX_OUTPUT_PORT_NUM];
        bool            dstEnabled[ISP_MAX_OUTPUT_PORT_NUM];

        DpRect          crop[ISP_MAX_OUTPUT_PORT_NUM];
        bool            cropEnabled[ISP_MAX_OUTPUT_PORT_NUM];

        BaseConfig();
    };

    struct AsyncBlitJobPair
    {
        int32_t               fenceFD;      // merged fence fd
        uint32_t              jobID;        // job id
        uint32_t              jobID2;       // job id 2

        int32_t               enhancePos;
        int32_t               enhanceRot;
        bool                  enhanceFlip;
        bool                  enhanceSplit;

        struct BaseConfig     config;
    };

    typedef std::vector<struct AsyncBlitJobPair *> JobList;

    struct CropROI
    {
        DpRect  crop;
        DpRect  roi;
        int32_t width;
        int32_t height;
    };

    static void transToRotFlip(uint32_t transform, int32_t &rotation, bool &flip);

    // rotate method 1: aligned to user orientation
    inline void calculateCropROI(struct CropROI &crs, struct CropROI &crs2,
                int32_t rotation, DpColorFormat dstFormat);

    // rotate method 2: fixed to source orientation
    inline void calculateCropROI2(struct CropROI &crs, struct CropROI &crs2,
                int32_t rotation, DpColorFormat dstFormat);

    void _calcCropROI(struct CropROI &crs, struct CropROI &crs2,
                DpColorFormat dstFormat, bool sh2w, bool sw2o, bool dh2w, bool dw2o);

    DP_STATUS_ENUM refreshCropROI(int32_t portIndex);

    DP_STATUS_ENUM refreshCropROIs();

    DpAsyncBlitStream m_stream;
    DpAsyncBlitStream m_stream2;

    // thread safe
    DpMutex           *m_pJobMutex;
    JobList           m_newJobList;
    JobList           m_jobList;

    // *NOT* thread safe
    struct AsyncBlitJobPair *m_pCurJob;
};

#endif  // __DP_ASYNC_BLIT_STREAM_2_H__
