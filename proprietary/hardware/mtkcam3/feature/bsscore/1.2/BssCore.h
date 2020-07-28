#ifndef __BSSCORE_H__
#define __BSSCORE_H__

#include <mtkcam3/feature/bsscore/IBssCore.h>
#include <MTKBss.h>
#include <mtkcam/utils/hw/IFDContainer.h>

//Transfer FD to BSS_FD
struct MTKBSSFDInfo {
    BssFaceMetadata facedata;
    BssFace         faces[15];
    BssFaceInfo     posInfo[15];
    MTKBSSFDInfo () : facedata{}, faces{}, posInfo{} {};
    ~MTKBSSFDInfo () {};

    void parser(FD_DATATYPE& mtkFace)
    {

#define MFLLBSS_COPY(param1, param2, element) \
        do { \
            param1.element = param2.element; \
        } while (0)
#define MFLLBSS_COPY_ARRAY(param1, param2, array, size) \
        do { \
            for (int i = 0 ; i  < size ; i++) \
                param1.array[i] = param2.array[i]; \
        } while (0)
#define MFLLBSS_COPY_ARRAY2(param1, param2, array, M, N) \
        do { \
            for (int j = 0 ; j  < N ; j++) \
                for (int i = 0 ; i  < M ; i++) \
                    param1.array[i][j] = param2.array[i][j] ;\
        } while (0)

#define MFLLBSS_FACEMETA_COPY(element)              MFLLBSS_COPY(facedata, mtkFace.facedata, element)
#define MFLLBSS_FACEMETA_COPY_ARRAY(array, size)    MFLLBSS_COPY_ARRAY(facedata, mtkFace.facedata, array, size)
#define MFLLBSS_FACEMETA_COPY_ARRAY2(array, M, N)   MFLLBSS_COPY_ARRAY2(facedata, mtkFace.facedata, array, M, N)
        //BssFaceMetadata facedata
        MFLLBSS_FACEMETA_COPY(number_of_faces);
        //
        facedata.faces = &faces[0];
        facedata.posInfo = &posInfo[0];
        MFLLBSS_FACEMETA_COPY_ARRAY(faces_type, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY2(motion, 15, 2);
        //
        MFLLBSS_FACEMETA_COPY(ImgWidth);
        MFLLBSS_FACEMETA_COPY(ImgHeight);
        //
        MFLLBSS_FACEMETA_COPY(gmIndex);
        MFLLBSS_FACEMETA_COPY(gmData);
        MFLLBSS_FACEMETA_COPY(gmSize);
        MFLLBSS_FACEMETA_COPY(genderNum);
        MFLLBSS_FACEMETA_COPY(poseNum);
        MFLLBSS_FACEMETA_COPY(landmarkNum);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyex0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyey0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyex1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyey1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyex0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyey0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyex1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyey1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(nosex, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(nosey, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(mouthx0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(mouthy0, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(mouthx1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(mouthy1, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyeux, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyeuy, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyedx, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(leyedy, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyeux, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyeuy, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyedx, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(reyedy, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(fa_cv, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(fld_rip, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(fld_rop, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY2(YUVsts, 15, 5);
        MFLLBSS_FACEMETA_COPY_ARRAY(fld_GenderLabel, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(fld_GenderInfo, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(GenderLabel, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(oGenderLabel, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(GenderCV, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY(RaceLabel, 15);
        MFLLBSS_FACEMETA_COPY_ARRAY2(RaceCV, 15, 4);
        //
        MFLLBSS_FACEMETA_COPY_ARRAY2(poseinfo, 15, 10);
        //
        MFLLBSS_FACEMETA_COPY(timestamp);
        //
        MFLLBSS_COPY(facedata.CNNFaces, mtkFace.facedata.CNNFaces, PortEnable);
        MFLLBSS_COPY(facedata.CNNFaces, mtkFace.facedata.CNNFaces, IsTrueFace);
        MFLLBSS_COPY(facedata.CNNFaces, mtkFace.facedata.CNNFaces, CnnResult0);
        MFLLBSS_COPY(facedata.CNNFaces, mtkFace.facedata.CNNFaces, CnnResult1);

        //BssFace         faces[15];
        for (size_t f = 0; f < 15 ; f++) {
            MFLLBSS_COPY_ARRAY(faces[f], mtkFace.facedata.faces[f], rect, 4);
            MFLLBSS_COPY(faces[f], mtkFace.facedata.faces[f], score);
            MFLLBSS_COPY(faces[f], mtkFace.facedata.faces[f], id);
            MFLLBSS_COPY_ARRAY(faces[f], mtkFace.facedata.faces[f], left_eye, 2);
            MFLLBSS_COPY_ARRAY(faces[f], mtkFace.facedata.faces[f], right_eye, 2);
            MFLLBSS_COPY_ARRAY(faces[f], mtkFace.facedata.faces[f], mouth, 2);
        }
        //BssFaceInfo     posInfo[15];
        for (size_t p = 0; p < 15 ; p++) {
            MFLLBSS_COPY(posInfo[p], mtkFace.facedata.posInfo[p], rop_dir);
            MFLLBSS_COPY(posInfo[p], mtkFace.facedata.posInfo[p], rip_dir);
        }

#undef MFLLBSS_FACEMETA_COPY_ARRAY2
#undef MFLLBSS_FACEMETA_COPY_ARRAY
#undef MFLLBSS_FACEMETA_COPY
#undef MFLLBSS_COPY_ARRAY2
#undef MFLLBSS_COPY_ARRAY
#undef MFLLBSS_COPY
    }
};

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

    MUINT32 getZipOutSize(IImageBuffer* input, IMetadata* pHalMeta);
    MVOID   doZip(IImageBuffer* input, IImageBuffer* output, MUINT32 ImgOutSize, IMetadata* pHalMeta, MBOOL isFirst);

private:

    MBOOL retrieveGmvInfo(IMetadata* pMetadata, int& x, int& y, MSize& size) const;
    GMV calMotionVector(IMetadata* pMetadata, MBOOL isMain) const;
    MVOID updateBssProcInfo(IImageBuffer* pBuf, BSS_PARAM_STRUCT& p, MINT32 frameNum) const;
    MVOID updateBssIOInfo(IImageBuffer* pBuf, BSS_INPUT_DATA_G& bss_input) const;
    std::shared_ptr<char> getNvramChunk(size_t *bufferSize) const;
    MBOOL appendBSSInput(Vector<RequestPtr>& rvRequests, BSS_INPUT_DATA_G& bss_input, vector<MTKBSSFDInfo>& bss_fddata);
    MBOOL getForceBss(void* param_addr, size_t param_size) const;
    MVOID dumpBssInputData2File(RequestPtr& firstRequest, BSS_PARAM_STRUCT& bss_param, BSS_INPUT_DATA_G& bss_input, BSS_OUTPUT_DATA& bssOutData) const;
    MVOID collectPreBSSExifData(Vector<RequestPtr>& rvReadyRequests, BSS_PARAM_STRUCT& p, BSS_INPUT_DATA_G& bss_input);
    MVOID collectPostBSSExifData(Vector<RequestPtr>& rvReadyRequests, Vector<MINT32>& vNewIndex, BSS_OUTPUT_DATA& bss_output);
    MVOID reorderZipRequest(Vector<RequestPtr>& doBssRequests);

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
    ZipOutData mZipData;
    Vector<IMetadata::Memory> mvZipMemory;
    MBOOL   mIsPackData;
    MBOOL   mIsDownSample;

};// class BssCore

} // namespace BSScore

#endif//__BSSCORE_H__
