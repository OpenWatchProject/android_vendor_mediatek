/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef N3D_DEBUG_UTIL_IMP_H_
#define N3D_DEBUG_UTIL_IMP_H_

#include <n3d_debug_util.h>
#include <vector>
#include <map>
#include <thread>
#include <MTKStereoKernel.h>

#include <vsdof/hal/rapidjson/writer.h>
#include <vsdof/hal/rapidjson/stringbuffer.h>
#include <vsdof/hal/rapidjson/document.h>     // rapidjson's DOM-style API
#include <vsdof/hal/rapidjson/prettywriter.h> // for stringify JSON
#include <vsdof/hal/rapidjson/filewritestream.h>
#include <vsdof/hal/rapidjson/writer.h>
#include <vsdof/hal/extradata_def.h>

#include <mtkcam/feature/stereo/hal/FastLogger.h>

using namespace android;
using namespace StereoHAL;
using namespace NSCam;
using namespace std;
using namespace rapidjson;

class N3DDebugUtilImp: public N3DDebugUtil
{
public:
    N3DDebugUtilImp();
    virtual ~N3DDebugUtilImp();

    virtual void addBuffer(N3DDebugBuffer_T &&buffer);
    virtual size_t getDebugBuffer(void *buffer);
    virtual size_t getDebugBufferSize();

    virtual void resetAll() {}
    virtual void resetFrame() {}

    virtual void setInitInfo(STEREO_KERNEL_SET_ENV_INFO_STRUCT &initInfo);
    virtual void setProcInfo(STEREO_KERNEL_SET_PROC_INFO_STRUCT &procInfo);
    virtual void setResultInfo(STEREO_KERNEL_RESULT_STRUCT &result);

private:
    string __prepareDebugHeader();
    void __addInitInfoToDocument(Document &doc);
    void __addProcToDocument(Document &doc);
    size_t __getDebugBufferSize();

private:
    static Mutex __ioLock;

    const bool  __LOG_ENABLED;
    const bool  __COMPRESS_ENABLED;
    const bool  __DUMP_ENABLED;

    map<string, N3DDebugBuffer_T> __debugBufferMap;
    vector<thread> __compressThreadList;
    char *__headerBuffer = NULL;
    size_t __headerSize = 0;

    //Debug header cache
    Document __initInfoDoc;
    Document __procInfoDoc;
    Document __resultInfoDoc;

    //For debug & benchmark
    size_t __rawBufferSize   = 0;
    size_t __debugBufferSize = 0;

    FastLogger __fastLogger;
};

#endif