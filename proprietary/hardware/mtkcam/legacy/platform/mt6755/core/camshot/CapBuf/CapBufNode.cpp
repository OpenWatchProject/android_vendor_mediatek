/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/CBNode"
//
#include <list>
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
using namespace NSCam::Utils;
//
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>
//
#include <utils/Mutex.h>
using namespace android;
//
#include <queue>
using namespace std;
//
#include <mtkcam/camnode/IspSyncControl.h>
//
#include <mtkcam/camshot/CapBufMgr.h>
using namespace NSCamShot;
//
#include <inc/CapBufNode.h>
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")

/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {


/*******************************************************************************
*
********************************************************************************/
class CapBufNodeImp : public CapBufNode
{
    public:
        CapBufNodeImp(Pass1NodeInitCfg const initcfg);
        ~CapBufNodeImp();
        //
        virtual MVOID   destroyInstance(void);
        //
        DECLARE_ICAMTHREADNODE_INTERFACES();
        //
        virtual MBOOL       setCapBufMgr(NSCamShot::CapBufMgr* pCapBufMgr);
        virtual MBOOL       setOneImgBuf(IImageBuffer* pImgBuf_Full, IImageBuffer* pImgBuf_Prv);
        //
    private:
        Mutex                       mLock;
        MUINT32                     muFrameCnt;
        Pass1NodeInitCfg const      mInitCfg;
        list< IImageBuffer* >       mlpDequeFullBuf;
        list< IImageBuffer* >       mlpDequePrvBuf;
        NSCamShot::CapBufMgr*       mpCapBufMgr;
        IImageBuffer*               mpImgBuf_Full;
        IImageBuffer*               mpImgBuf_Prv;
        IspSyncControl*             mpIspSyncCtrl;
};


/*******************************************************************************
*
********************************************************************************/
CapBufNode*
CapBufNode::
createInstance(Pass1NodeInitCfg const initcfg)
{
    CapBufNodeImp* pInstance = new CapBufNodeImp(initcfg);
    return pInstance;
}
//-----------------------------------------------------------------------------
MVOID
CapBufNodeImp::
destroyInstance(void)
{
    delete this;
}
//------------------------------------------------------------------------------
CapBufNodeImp::
CapBufNodeImp(Pass1NodeInitCfg const initcfg)
    : muFrameCnt(0)
    , mInitCfg(initcfg)
    , mpCapBufMgr(NULL)
    , mpImgBuf_Full(NULL)
    , mpImgBuf_Prv(NULL)
    , mpIspSyncCtrl(NULL)
{
    //DATA
    addDataSupport( ENDPOINT_DST, PASS1_FULLRAW );
    addDataSupport( ENDPOINT_DST, PASS1_RESIZEDRAW );
    //NODECAT_BASIC_NOTIFY
    addNotifySupport( PASS1_START_ISP | PASS1_SOF | PASS1_EOF);
}
//-----------------------------------------------------------------------------
CapBufNodeImp::
~CapBufNodeImp()
{
    FUNC_NAME;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
onInit()
{
    FUNC_START;
    muFrameCnt = 0;
    mlpDequeFullBuf.clear();
    mlpDequePrvBuf.clear();
    mpIspSyncCtrl = IspSyncControl::createInstance(getSensorIdx());
    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
onUninit()
{
    FUNC_START;
    //
    if(mpIspSyncCtrl)
    {
        mpIspSyncCtrl->destroyInstance();
        mpIspSyncCtrl = NULL;
    }
    //
    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
onStart()
{
    FUNC_START;
    //
    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
onStop()
{
    FUNC_START;
    syncWithThread(); //wait for jobs done
    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    switch( msg )
    {
        case CONTROL_STOP_PASS1:
            MY_LOGD("pass1 receive stop notify");
            syncWithThread();
            MY_LOGD("pass1 stoped");
            break;
        default:
            break;
    }
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    //should not happen
    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    IImageBuffer* pBuf = static_cast<IImageBuffer *>((MVOID*)buf);
    MBOOL ret = MTRUE;
    {
        Mutex::Autolock _l(mLock);
        if( data == PASS1_FULLRAW )
        {
            list< IImageBuffer* >::iterator iter;
            for(iter = mlpDequeFullBuf.begin() ; iter != mlpDequeFullBuf.end(); iter++)
            {
                if(*iter == pBuf)
                {
                    if(mpCapBufMgr != NULL)
                    {
                        mpCapBufMgr->enqueBuf(pBuf);
                    }
                    mlpDequeFullBuf.erase(iter);
                    break;
                }
            }
            //
            if(iter == mlpDequeFullBuf.end())
            {
                MY_LOGE("Not deque buf(0x%08X)",pBuf);
                ret = MFALSE;
            }
        }
        else if( data == PASS1_RESIZEDRAW )
        {
            list< IImageBuffer* >::iterator iter;
            for(iter = mlpDequePrvBuf.begin() ; iter != mlpDequePrvBuf.end(); iter++)
            {
                if(*iter == pBuf)
                {
                    if(mpCapBufMgr != NULL)
                    {
                        mpCapBufMgr->enqueBuf(pBuf, MFALSE);
                    }
                    mlpDequePrvBuf.erase(iter);
                    break;
                }
            }
            //
            if(iter == mlpDequePrvBuf.end())
            {
                MY_LOGE("Not deque buf(0x%08X)",pBuf);
                ret = MFALSE;
            }
        }
        else
        {
            MY_LOGE("un-support data(%d)",data);
            ret = MFALSE;
        }
    }

    if( !ret )
    {
        MY_LOGE("wrong order: data(%d), buf(0x%X), ext(0x%08X)", data, buf, ext);
    }
    else
    {
        MY_LOGD("enque: data(%d), buf(0x%X), ext(0x%08X)", data, buf, ext);
    }

    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MVOID
CapBufNodeImp::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PATH "/sdcard/camnode/"
#define DUMP_IImageBuffer( type, pbuf, fileExt )               \
    do{                                                        \
        IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
        char filename[256];                                    \
        sprintf(filename, "%s%s_%dx%d_%d.%s",                 \
                DUMP_PATH,                                     \
                #type,                                         \
                buffer->getImgSize().w,buffer->getImgSize().h, \
                muFrameCnt,                                    \
                fileExt                                        \
               );                                              \
        MY_LOGD("dump %s", filename);                           \
        buffer->saveToFile(filename);                          \
    }while(0)

    if(!makePath(DUMP_PATH,0660))
    {
        MY_LOGE("makePath [%s] fail",DUMP_PATH);
        return;
    }

    switch( data )
    {
        case PASS1_FULLRAW:
            DUMP_IImageBuffer( PASS1_FULLRAW, buf, "raw" );
            break;
        case PASS1_RESIZEDRAW:
            DUMP_IImageBuffer( PASS1_RESIZEDRAW, buf, "raw" );
            break;
        default:
            MY_LOGE("not handle this yet data(0x%x)", data);
            break;
    }
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
threadLoopUpdate()
{
    FUNC_START;
    handleNotify( PASS1_SOF, 0, 0 );
    //
    MUINTPTR imgoAddr = 0;
    MUINTPTR rrzoAddr = 0;
    //
    if(isDataConnected(PASS1_RESIZEDRAW))
    {
        if(mpCapBufMgr != NULL)
        {
            IImageBuffer* pImageBuffer = NULL;
            if(mpCapBufMgr->dequeBuf(pImageBuffer, MFALSE))
            {
                if(pImageBuffer != NULL)
                {
                    rrzoAddr = (MUINTPTR)pImageBuffer;
                    //
                    {
                        Mutex::Autolock _l(mLock);
                        mlpDequePrvBuf.push_back(pImageBuffer);
                    }
                }
                else
                {
                    MY_LOGE("Deque NULL prv buffer");
                }
            }
            else
            {
                MY_LOGE("Deque fail");
            }
        }
        else
        if(mpImgBuf_Prv != NULL)
        {
            rrzoAddr = (MUINTPTR)mpImgBuf_Prv;
            //
            {
                Mutex::Autolock _l(mLock);
                mlpDequePrvBuf.push_back(mpImgBuf_Prv);
            }
        }
        else
        {
            MY_LOGE("mpCapBufMgr is NULL");
        }
    }
    //
    if(isDataConnected(PASS1_FULLRAW))
    {
        if(mpCapBufMgr != NULL)
        {
            IImageBuffer* pImageBuffer = NULL;
            if(mpCapBufMgr->dequeBuf(pImageBuffer))
            {
                if(pImageBuffer != NULL)
                {
                    imgoAddr = (MUINTPTR)pImageBuffer;
                    //
                    {
                        Mutex::Autolock _l(mLock);
                        mlpDequeFullBuf.push_back(pImageBuffer);
                    }
                }
                else
                {
                    MY_LOGE("Deque NULL full buffer");
                }
            }
            else
            {
                MY_LOGE("Deque fail");
            }
        }
        else
        if(mpImgBuf_Full != NULL)
        {
            imgoAddr = (MUINTPTR)mpImgBuf_Full;
            //
            {
                Mutex::Autolock _l(mLock);
                mlpDequeFullBuf.push_back(mpImgBuf_Full);
            }
        }
        else
        {
            MY_LOGE("mpCapBufMgr is NULL");
        }
    }
    else
    {
        MY_LOGE("Please connect PASS1_FULLRAW");
        usleep(10*1000);
    }
    //
    if(imgoAddr)
    {
        MUINT32 magicNum;
        MBOOL isRrzo;
        MVOID* pPrivateData;
        MUINT32 privateDataSize;
        //
        if(rrzoAddr)
        {
            mpIspSyncCtrl->queryImgBufInfo(
                                (IImageBuffer*)rrzoAddr,
                                magicNum,
                                isRrzo,
                                pPrivateData,
                                privateDataSize);
            //
            MY_LOGD("rrzoAddr(0x%08X),MN(0x%08X)", rrzoAddr, magicNum);
        }
        mpIspSyncCtrl->queryImgBufInfo(
                            (IImageBuffer*)imgoAddr,
                            magicNum,
                            isRrzo,
                            pPrivateData,
                            privateDataSize);
        //
        MY_LOGD("imgoAddr(0x%X),MN(0x%08X)",
                imgoAddr,
                magicNum);
        //
        handleNotify(PASS1_EOF, -1, magicNum);
        handlePostBuffer(PASS1_FULLRAW, imgoAddr, 0);
        if(rrzoAddr)
            handlePostBuffer(PASS1_RESIZEDRAW, rrzoAddr, 0);
    }
    else
    {
        MY_LOGE("imgoAddr is 0");
    }
    FUNC_END;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
setCapBufMgr(NSCamShot::CapBufMgr* pCapBufMgr)
{
    MY_LOGD("pCapBufMgr(0x%X)",(MUINTPTR)pCapBufMgr);
    if(pCapBufMgr == NULL)
    {
        MY_LOGE("pCapBufMgr is NULL");
        return MFALSE;
    }
    mpCapBufMgr     = pCapBufMgr;
    mpImgBuf_Full   = NULL;
    mpImgBuf_Prv    = NULL;
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
CapBufNodeImp::
setOneImgBuf(IImageBuffer* pImgBuf_Full, IImageBuffer* pImgBuf_Prv)
{
    MY_LOGD("pImgBuf_Full(0x%X), pImgBuf_Prv(0x%X)",(MUINTPTR)pImgBuf_Full, (MUINTPTR)pImgBuf_Prv);
    if(pImgBuf_Full == NULL)
    {
        MY_LOGE("pImgBuf_Full is NULL");
        return MFALSE;
    }
    mpImgBuf_Full   = pImgBuf_Full;
    mpImgBuf_Prv    = pImgBuf_Prv;
    mpCapBufMgr     = NULL;
    return MTRUE;
}
//-----------------------------------------------------------------------------
};  //namespace NSCamNode

