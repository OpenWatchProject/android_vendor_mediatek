/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SwJpegUtility_H__
#define __SwJpegUtility_H__


#include <stdio.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <fcntl.h>
#include <sys/mman.h>


//#include "SkImageDecoder.h"
#include "SwStream.h"
#include "SwType.h"

extern "C" {
    #include "jpeglib_alpha.h"
    #include "jerror_alpha.h"
}

#include <setjmp.h>

/* Our error-handling struct.
 *
*/
struct swjpeg_error_mgr : jpeg_error_mgr_ALPHA {
    jmp_buf fJmpBuf;
};


void swjpeg_error_exit(j_common_ptr_ALPHA cinfo);

///////////////////////////////////////////////////////////////////////////
/* Our source struct for directing jpeg to our stream object.
*/
struct swjpeg_source_mgr : jpeg_source_mgr_ALPHA {
    //swjpeg_source_mgr(SwStream* stream, SkImageDecoder* decoder, bool ownStream);
    swjpeg_source_mgr(SwStream* stream, bool ownStream);
    ~swjpeg_source_mgr();

    SwStream*   fStream;
    void*       fMemoryBase;
    size_t      fMemoryBaseSize;
    bool        fUnrefStream;
    //SkImageDecoder* fDecoder;
    enum {
        kBufferSize = 1024
    };
    char    fBuffer[kBufferSize];
};

/////////////////////////////////////////////////////////////////////////////
/* Our destination struct for directing decompressed pixels to our stream
 * object.
 */
struct swjpeg_destination_mgr : jpeg_destination_mgr_ALPHA {
    swjpeg_destination_mgr(SwEncStream* stream);

    SwEncStream*  fStream;

    enum {
        kBufferSize = 1024
    };
    uint8_t fBuffer[kBufferSize];
};

#endif
