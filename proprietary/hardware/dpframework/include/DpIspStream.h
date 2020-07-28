#ifndef __DP_ISP_STREAM_H__
#define __DP_ISP_STREAM_H__

#include "DpDataType.h"
#include "DpIspDataType.h"
#include <sys/time.h>
#include <vector>
#include <map>
#include <pthread.h>

using namespace std;

class DpCondition;

#define ISP_MAX_OUTPUT_PORT_NUM     4

class DpMutex;

struct ISP_TPIPE_CONFIG_STRUCT;

typedef std::vector<int32_t> BufIDList;
typedef std::vector<DpJobID> JobIDList;
typedef std::vector<struct timeval> BufTimeList;
typedef std::vector<uint32_t> BufRegList;

typedef struct DpDoneJobItem {
    DpJobID         jobId;
    DP_STATUS_ENUM  jobStatus;
} DpJobItem;

class DpIspStream
{
public:
    enum ISPStreamType
    {
        ISP_IC_STREAM,
        ISP_VR_STREAM,
        ISP_ZSD_STREAM,
        ISP_IP_STREAM,
        ISP_VSS_STREAM,
        ISP_ZSD_SLOW_STREAM,
        WPE_STREAM,
        WPE_STREAM2,
        WPE_ISP_STREAM,
        ISP_CC_STREAM = ISP_VR_STREAM
    };

    enum ISPStreamHint
    {
        HINT_ISP_NEW_FRAME = 0x0,       // need to do tpipe calculation
        HINT_ISP_TPIPE_NO_CHANGE = 0x1, // can skip tpipe calculation
        HINT_ISP_FRAME_MODE = 0x2,      // ISP-only frame mode
        HINT_WPE_FRAME_MODE = HINT_ISP_FRAME_MODE
    };

    static uint32_t queryMultiPortSupport(ISPStreamType type);
    static void queryISPFeatureSupport(std::map<DP_ISP_FEATURE_ENUM, bool> &mapISPFeature);

    DpIspStream(ISPStreamType type);

    ~DpIspStream();

    DP_STATUS_ENUM queueSrcBuffer(void     *pVA,
                                  uint32_t size);

    DP_STATUS_ENUM queueSrcBuffer(void     *pVA,
                                  uint32_t MVA,
                                  uint32_t size);

    DP_STATUS_ENUM queueSrcBuffer(void     **pVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    DP_STATUS_ENUM queueSrcBuffer(uint32_t *pMVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    DP_STATUS_ENUM queueSrcBuffer(void     **pVAList,
                                  uint32_t *pMVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    // for ION file descriptor
    DP_STATUS_ENUM queueSrcBuffer(int32_t  fileDesc,
                                  uint32_t *sizeList,
                                  uint32_t planeNum);

    DP_STATUS_ENUM dequeueSrcBuffer();

    /**
     * Description:
     *     Set source buffer configuration information
     *
     * Parameter:
     *     srcFormat: Source buffer format
     *     srcWidth: Source buffer width
     *     srcHeight: Source buffer height
     *     srcPitch: Source buffer pitch
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setSrcConfig(DpColorFormat srcFormat,
                                int32_t       srcWidth,
                                int32_t       srcHeight,
                                int32_t       srcPitch,
                                bool          doFlush = true);


    DP_STATUS_ENUM setSrcConfig(int32_t           width,
                                int32_t           height,
                                int32_t           YPitch,
                                int32_t           UVPitch,
                                DpColorFormat     format,
                                DP_PROFILE_ENUM   profile = DP_PROFILE_BT601,
                                DpInterlaceFormat field   = eInterlace_None,
                                DpRect            *pROI   = 0,
                                bool              doFlush = true,
                                DpSecure          secure = DP_SECURE_NONE);

    /**
     * Description:
     *     Set source buffer crop window information
     *
     * Parameter:
     *     XStart: Source crop X start coordinate
     *     XSubpixel: Source crop X subpixel coordinate
     *     YStart: Source crop Y start coordinate
     *     YSubpixel: Source crop Y subpixel coordinate
     *     cropWidth: Source crop window width
     *     cropHeight: Source crop window height
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setSrcCrop(int32_t XStart,
                              int32_t XSubpixel,
                              int32_t YStart,
                              int32_t YSubpixel,
                              int32_t cropWidth,
                              int32_t cropHeight);

    DP_STATUS_ENUM setSrcCrop(int32_t portIndex,
                              int32_t XStart,
                              int32_t XSubpixel,
                              int32_t YStart,
                              int32_t YSubpixel,
                              int32_t cropWidth,
                              int32_t cropHeight);

    DP_STATUS_ENUM setSrcCrop(int32_t portIndex,
                              int32_t XStart,
                              int32_t XSubpixel,
                              int32_t YStart,
                              int32_t YSubpixel,
                              int32_t cropWidth,
                              int32_t cropWidthSubpixel,
                              int32_t cropHeight,
                              int32_t cropHeightSubpixel);

    DP_STATUS_ENUM queueDstBuffer(int32_t  portIndex,
                                  void     **pVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    DP_STATUS_ENUM queueDstBuffer(int32_t  portIndex,
                                  uint32_t *pMVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    DP_STATUS_ENUM queueDstBuffer(int32_t  portIndex,
                                  void     **pVAList,
                                  uint32_t *pMVAList,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    // for ION file descriptor
    DP_STATUS_ENUM queueDstBuffer(int32_t  portIndex,
                                  int32_t  fileDesc,
                                  uint32_t *pSizeList,
                                  int32_t  planeNum);

    /**
     * Description:
     *     Acquire a destination buffer for HW processing
     *
     * Parameter:
     *     port: Output port index
     *     base: buffer virtual base address
     *     waitBuf: true for the buffer is ready;
     *              else return immediately
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM dequeueDstBuffer(int32_t portIndex,
                                    void    **pVABase,
                                    bool    waitBuf = true);

    /**
     * Description:
     *     Set destination buffer configuration information
     *
     * Parameter:
     *     format: Destination buffer format
     *     width: Destination buffer width
     *     height: Destination buffer height
     *     pitch: Destination buffer pitch
     *     port: Destination port number
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setDstConfig(int32_t       portIndex,
                                DpColorFormat dstFormat,
                                int32_t       dstWidth,
                                int32_t       dstHeight,
                                int32_t       dstPitch,
                                bool          doFlush = true);


    DP_STATUS_ENUM setDstConfig(int32_t           portIndex,
                                int32_t           width,
                                int32_t           height,
                                int32_t           YPitch,
                                int32_t           UVPitch,
                                DpColorFormat     format,
                                DP_PROFILE_ENUM   profile = DP_PROFILE_BT601,
                                DpInterlaceFormat field   = eInterlace_None,
                                DpRect            *pROI   = 0,
                                bool              doFlush = true,
                                DpSecure          secure = DP_SECURE_NONE);


    /**
     * Description:
     *     Set port desired rotation angles
     *
     * Parameter:
     *     portIndex: Port index number
     *     rotation: Desired rotation angle
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setRotation(int32_t portIndex,
                               int32_t rotation);

    /**
     * Description:
     *     Set port desired flip status
     *
     * Parameter:
     *    portIndex: Port index number
     *    flipStatus: Desired flip status
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setFlipStatus(int32_t portIndex,
                                 bool    flipStatus);


    /**
     * Description:
     *     Set extra parameter for ISP
     *
     * Parameter:
     *     extraPara: ISP extra parameters
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM setParameter(ISP_TPIPE_CONFIG_STRUCT &extraPara, uint32_t hint = 0);

    DP_STATUS_ENUM setParameter(ISP_TPIPE_CONFIG_STRUCT &extraPara, ISP2MDP_STRUCT *isp2mdp, uint32_t hint);

    DP_STATUS_ENUM setPortType(int32_t           portIndex,
                               uint32_t          portType = 0,
                               DpJPEGEnc_Config* JPEGEnc_cfg = NULL);

    DP_STATUS_ENUM setSharpness(int32_t portIndex,
                                int32_t gain);

    DP_STATUS_ENUM setDither(int32_t portIndex,
                             bool    enDither);

    DP_STATUS_ENUM setPortMatrix(int32_t portIndex, DpColorMatrix *matrix);

    uint32_t getPqID();

    DP_STATUS_ENUM setPQParameter(int32_t portIndex, const DpPqParam &pParam);

    DP_STATUS_ENUM setPQParameter(int32_t portIndex, const DpPqParam *pParam);

    /**
     * Description:
     *     Start ISP stream processing (non-blocking)
     *
     * Parameter:
     *     None
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM startStream(struct timeval *endTime = NULL);

    /**
     * Description:
     *     Stop ISP stream processing
     *
     * Parameter:
     *     None
     *
     * Return Value:
     *     Return DP_STATUS_RETURN_SUCCESS if the API succeeded,
     *     else the API will return the error code.
     */
    DP_STATUS_ENUM stopStream();

    DP_STATUS_ENUM dequeueFrameEnd(uint32_t *pValueList = NULL);

    DP_STATUS_ENUM getJPEGFilesize(uint32_t* filesize);

    DP_STATUS_ENUM setCallback(void (*callback)(void *param), void * data = NULL);

    static DP_STATUS_ENUM initPQService();

private:
    typedef struct BaseConfig
    {
        // src
        int32_t         srcWidth;
        int32_t         srcHeight;
        int32_t         srcYPitch;
        int32_t         srcUVPitch;
        DpColorFormat   srcFormat;
        DP_PROFILE_ENUM srcProfile;
        DpSecure        srcSecure;
        bool            srcFlush;

        // dst
        int32_t         dstWidth[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         dstHeight[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         dstYPitch[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         dstUVPitch[ISP_MAX_OUTPUT_PORT_NUM];
        DpColorFormat   dstFormat[ISP_MAX_OUTPUT_PORT_NUM];
        DP_PROFILE_ENUM dstProfile[ISP_MAX_OUTPUT_PORT_NUM];
        DpSecure        dstSecure[ISP_MAX_OUTPUT_PORT_NUM];
        bool            dstFlush[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         dstEnabled[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         dstPlane[ISP_MAX_OUTPUT_PORT_NUM];

        // crop
        int32_t         multiSrcXStart[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         multiSrcXSubpixel[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         multiSrcYStart[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         multiSrcYSubpixel[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         multiCropWidth[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         multiCropWidthSubpixel[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         multiCropHeight[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         multiCropHeightSubpixel[ISP_MAX_OUTPUT_PORT_NUM];

        // target offset
        int32_t         targetXStart[ISP_MAX_OUTPUT_PORT_NUM];
        int32_t         targetYStart[ISP_MAX_OUTPUT_PORT_NUM];

        int32_t         rotation[ISP_MAX_OUTPUT_PORT_NUM];
        bool            flipStatus[ISP_MAX_OUTPUT_PORT_NUM];
        bool            ditherStatus[ISP_MAX_OUTPUT_PORT_NUM];
        DpPqConfig      PqConfig[ISP_MAX_OUTPUT_PORT_NUM];

        BaseConfig();

    }BaseConfig;


    typedef struct IspConfig: BaseConfig
    {
        bool            frameChange;
        bool            tpipeChange;
        uint32_t        dstPortType[ISP_MAX_OUTPUT_PORT_NUM];
        DpJPEGEnc_Config jpegEnc_cfg;
        int32_t         srcBuffer;
        int32_t         dstBuffer[ISP_MAX_OUTPUT_PORT_NUM];
        bool            multiCropChange[ISP_MAX_OUTPUT_PORT_NUM];
        DpPqParam       pqParam[ISP_MAX_OUTPUT_PORT_NUM];
        void (*callback)(void *param);
        void            *callbackData;
        IspConfig();

    } IspConfig;

    DP_STATUS_ENUM detectFrameChange(IspConfig &config);

    DP_STATUS_ENUM detectCropChange(int32_t portIndex);

    DP_STATUS_ENUM configPQParameter(int32_t portIndex);

    ISPStreamType           m_streamType;
    DpStream                *m_pStream;
    DpChannel               *m_pChannel;
    int32_t                 m_channelID;

    DpMutex                 *m_pListMutex;
    DpMutex                 *m_pBufferMutex;
    DpMutex                 *m_pConfigMutex;

	std::vector<JobIDList>		m_jobIDsList;
    std::vector<DpDoneJobItem> m_frameDoneList;
    typedef std::vector<IspConfig> IspConfigList;
    IspConfigList           m_ispConfigList;

    // Source information
    DpBasicBufferPool       *m_pSrcPool;
    //int32_t                 m_srcBuffer;    // Only used in MDP1.0
    BufIDList               m_srcBufferList;
    BufTimeList             m_srcBufferStartTimeList;

    // Destination information
    DpBasicBufferPool       *m_pDstPool[ISP_MAX_OUTPUT_PORT_NUM];
    //int32_t                 m_dstBuffer[ISP_MAX_OUTPUT_PORT_NUM];   // Only used in MDP1.0
    BufIDList               m_dstBufferList[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_sharpness[ISP_MAX_OUTPUT_PORT_NUM];

    // Crop information
    //bool                    m_cropChange;   // Only used in MDP1.0
    //int32_t                 m_srcXStart;    // Only used in MDP1.0
    //int32_t                 m_srcXSubpixel; // Only used in MDP1.0
    //int32_t                 m_srcYStart;    // Only used in MDP1.0
    //int32_t                 m_srcYSubpixel; // Only used in MDP1.0
    //int32_t                 m_cropWidth;    // Only used in MDP1.0
    //int32_t                 m_cropHeight;   // Only used in MDP1.0

    uint32_t                m_jpegEnc_filesize;

    //ISP_TPIPE_CONFIG_STRUCT *m_pParameter;

    uint32_t                m_PqID[ISP_MAX_OUTPUT_PORT_NUM];
    int32_t                 m_pqSupport;
    DpColorMatrix           *m_pDstMatrix[ISP_MAX_OUTPUT_PORT_NUM];

    //Cmd read register
    BufRegList              m_regValueList;
    BufRegList              m_regCountList;
    uint32_t                m_PABuffer[MAX_NUM_READBACK_PA_BUFFER];
    bool                    m_allocatePA;
    uint64_t                m_configFrameCount;
    uint64_t                m_dequeueFrameCount;
    DpMutex                 *m_pRegMutex;

    //PQ AAL readback
    BufRegList              m_DREPABufferSizeList;
    BufRegList              m_DREPABufferStartIndexList;
    uint32_t                m_DREPABuffer[MAX_NUM_READBACK_PA_BUFFER];
    DpMutex                 *m_pDRERegMutex;
    BufRegList              m_numReadbackEngineList;

    IspConfig               m_prevConfig;
    IspConfig               m_curConfig;

    DpCondition             *m_pJobCond;
    pthread_t               m_thread;
    bool                    m_abortJobs;

    DP_STATUS_ENUM waitSubmit();
    DP_STATUS_ENUM waitComplete();

	static void* waitCompleteLoop(void* para);
	void createThread();

};

#endif  // __DP_ISP_STREAM_H__
