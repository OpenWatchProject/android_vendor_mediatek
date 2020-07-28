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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "GF_HAL"

#include "gf_hal_imp.h"

#define IS_DUMP_ENALED (NULL != __dumpHint)

void
GF_HAL_IMP::__dumpInitInfo()
{
    if(!IS_DUMP_ENALED) {
        return;
    }

    if(__initDumpSize < 1) {
        snprintf(__dumpInitData,
                __DUMP_BUFFER_SIZE,
                "{\n"
                "    \"inputWidth\": %d,\n"
                "    \"inputHeight\": %d,\n"
                "    \"outputWidth\": %d,\n"
                "    \"outputHeight\": %d,\n"
                "    \"workingBuffSize\": %d,\n"
                "    \"gfMode\": %d\n"
                "}",
                m_initInfo.inputWidth,
                m_initInfo.inputHeight,
                m_initInfo.outputWidth,
                m_initInfo.outputHeight,
                m_initInfo.workingBuffSize,
                m_initInfo.gfMode
                );
        __initDumpSize = strlen(__dumpInitData);
    }

    char dumpPath[PATH_MAX];
    genFileName_VSDOF_BUFFER(dumpPath, PATH_MAX, __dumpHint, "GF_INIT_INFO.json");
    FILE *fp = fopen(dumpPath, "w");
    if(NULL == fp) {
        MY_LOGE("Cannot dump GF init info to %s, error: %s", dumpPath, strerror(errno));
        return;
    }

    fwrite(__dumpInitData, 1, __initDumpSize, fp);
    fflush(fp);
    fclose(fp);
}

void
GF_HAL_IMP::__dumpProcInfo()
{
    if(!IS_DUMP_ENALED) {
        return;
    }

    snprintf(__dumpProcData,
             __DUMP_BUFFER_SIZE,
            "{\n"
            "    \"touchTrigger\": %d,\n"
            "    \"touchX\": %d,\n"
            "    \"touchY\": %d,\n"
            "    \"depthValue\": %d,\n"
            "    \"dof\": %d,\n"
            "    \"clearRange\": %d,\n"
            "    \"cOffset\": %.2f,\n"
            "    \"dacInfo\": {\n"
            "        \"min\": %d,\n"
            "        \"max\": %d,\n"
            "        \"cur\": %d\n"
            "    },\n"
            "    \"fdInfo\":{\n"
            "        \"isFd\": %d,\n"
            "        \"ratio\": %.2f\n"
            "    },\n"
            "    \"afInfo\":{\n"
            "        \"afType\": %d,\n"
            "        \"x1\": %d,\n"
            "        \"y1\": %d,\n"
            "        \"x2\": %d,\n"
            "        \"y2\": %d\n"
            "    }\n"
            "}",
            m_procInfo.touchTrigger,
            m_procInfo.touchX,
            m_procInfo.touchY,
            m_procInfo.depthValue,
            m_procInfo.dof,
            m_procInfo.clearRange,
            m_procInfo.cOffset,
            m_procInfo.dacInfo.min,
            m_procInfo.dacInfo.max,
            m_procInfo.dacInfo.cur,
            m_procInfo.fdInfo.isFd,
            m_procInfo.fdInfo.ratio,
            m_procInfo.afInfo.afType,
            m_procInfo.afInfo.x1,
            m_procInfo.afInfo.y1,
            m_procInfo.afInfo.x2,
            m_procInfo.afInfo.y2
            );

    char dumpPath[PATH_MAX];
    genFileName_VSDOF_BUFFER(dumpPath, PATH_MAX, __dumpHint, "GF_PROC_INFO.json");
    FILE *fp = fopen(dumpPath, "w");
    if(NULL == fp) {
        MY_LOGE("Cannot dump GF proc info to %s, error: %s", dumpPath, strerror(errno));
        return;
    }

    fwrite(__dumpProcData, 1, strlen(__dumpProcData), fp);
    fflush(fp);
    fclose(fp);
}

void
GF_HAL_IMP::__dumpWorkingBuffer()
{
    if(!IS_DUMP_ENALED ||
       checkStereoProperty("vendor.STEREO.dump.gf_wb") < 1)
    {
        return;
    }

    char dumpPath[PATH_MAX];
    genFileName_VSDOF_BUFFER(dumpPath, PATH_MAX, __dumpHint, "GF_WORKING_BUFFER");
    FILE *fp = fopen(dumpPath, "wb");
    if(NULL == fp) {
        MY_LOGE("Cannot dump GF working buffer to %s, error: %s", dumpPath, strerror(errno));
        return;
    }

    fwrite(m_initInfo.workingBuffAddr, 1, m_initInfo.workingBuffSize, fp);
    fflush(fp);
    fclose(fp);
}
