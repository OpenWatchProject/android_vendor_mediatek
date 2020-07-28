#ifndef PARSER_FACTORY_H
#define PARSER_FACTORY_H

#include "types.h"
#include "IParser.h"

#define GOOGLE_STEREO_INFO 0
#define SEGMENT_MASK_INFO 1
#define STEREO_BUFFER_INFO 2
#define STEREO_CONFIG_INFO 3
#define STEREO_DEPTH_INFO 4
#define STEREO_CAPTURE_INFO 5

namespace stereo {

class ParserFactory {

public:
    /**
     * Get parser instance by information type.
     * @param infoType
     *            data info type
     * @param info
     *            data info
     * @param standardBuffer
     *            standard data buffer
     * @param extendedBuffer
     *            extended data buffer
     * @param customizedBuffer
     *            customized data buffer
     * @return Parser
     */
    static IParser* getParserInstance(
        int infoType, void *info, const StereoBuffer_t &standardBuffer,
        const StereoBuffer_t &extendedBuffer, BufferMapPtr customizedBuffer);
};

}

#endif