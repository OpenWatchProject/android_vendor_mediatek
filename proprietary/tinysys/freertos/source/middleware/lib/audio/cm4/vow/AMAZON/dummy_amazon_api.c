#if AMAZON_HOTWORD_DUMMY_LIB

#include "pryon_lite.h"
DLLDECL PryonLiteError PryonLite_GetEngineAttributes(PryonLiteEngineAttributes* engineAttributes)
{
    return PRYON_LITE_ERROR_OK;
}

DLLDECL PryonLiteError PryonLite_GetModelAttributes(const void *model, size_t sizeofModel, PryonLiteModelAttributes* modelAttributes)
{
    return PRYON_LITE_ERROR_OK;
}

DLLDECL PryonLiteError PryonLiteDecoder_Initialize(const PryonLiteDecoderConfig* config, PryonLiteSessionInfo* result, PryonLiteDecoderHandle *pHandle)
{
    return PRYON_LITE_ERROR_OK;
}

DLLDECL PryonLiteError PryonLiteDecoder_PushAudioSamples(PryonLiteDecoderHandle handle, const short *samples,
                                                         int sampleCount)
{
    return PRYON_LITE_ERROR_OK;
}

DLLDECL PryonLiteError PryonLiteDecoder_Destroy(PryonLiteDecoderHandle *pHandle)
{
    return PRYON_LITE_ERROR_OK;
}

DLLDECL int PryonLiteDecoder_IsDecoderInitialized(PryonLiteDecoderHandle handle)
{
    return PRYON_LITE_ERROR_OK;
}

DLLDECL PryonLiteError PryonLiteDecoder_SetDetectionThreshold(PryonLiteDecoderHandle handle, const char* keyword, int detectThreshold)
{
    return PRYON_LITE_ERROR_OK;
}


#endif // AMAZON_HOTWORD_DUMMY_LIB
