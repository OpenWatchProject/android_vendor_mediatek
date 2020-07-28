/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#define LOG_TAG "cctsvr_server"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/poll.h>
#include <unistd.h>
#include <errno.h>          // for errno in OpenFifo()

//#include <utils/Log.h>
#include <cutils/log.h>

#include <pthread.h>

#include "kd_imgsensor_define.h"

#include "cct_feature.h"

#include "cct_op_data.h"
#include "cct_op_handle.h"

#include "cct_server.h"

/********************************************************************************/
#define MY_LOGD(fmt, arg...)        ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        ALOGE("[%s] " fmt, __FUNCTION__, ##arg)
/********************************************************************************/


//save FIFO file
char g_CctSvrPipeInFilename[] = "/mnt/vendor/cct/myfifoin";
char g_CctSvrPipeOutFilename[] = "/mnt/vendor/cct/myfifoout";
#define PIPE_IN_FNAME         (g_CctSvrPipeInFilename)
#define PIPE_OUT_FNAME      (g_CctSvrPipeOutFilename)

typedef enum
{
     LNX_FIFO_IN_MODE = 0
    ,LNX_FIFO_OUT_MODE
} LNX_FIFO_MODE_T;

CctServer::
CctServer(MUINT32 isensor_dev)
{
    MY_LOGD("CctServer constructor +");
    m_bThreadStop = MFALSE;
    m_pCctHd = CctHandle::createInstance((CAMERA_DUAL_CAMERA_SENSOR_ENUM)isensor_dev);
    MY_LOGD("CctServer constructor -");
}


CctServer::
~CctServer()
{
    MY_LOGD("CctServer destructor +");
    m_pCctHd->destroyInstance();
    m_pCctHd = NULL;
    MY_LOGD("CctServer destructor -");
}


MINT32
CctServer::
CCT_ServerCtrl(CCT_SVR_CTL_T Cmd)
{
    pthread_attr_t attr;
    int s;
    void *res;

    MY_LOGD("CCT_ServerCtrl + cmd:%d",Cmd);
    if (Cmd == CCT_SVR_CTL_START_SERVER_THREAD) {
        m_bThreadStop = MFALSE;
#if 0
        int stack_size = 64*1024;
        s = pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, stack_size);
        s = pthread_create(&m_ThreadID, &attr, CCT_ServerThread, (void *)this);
        s = pthread_attr_destroy(&attr);
#else
        s = pthread_create(&m_ThreadID, NULL, CCT_ServerThread, (void *)this);
#endif
    } else if (Cmd == CCT_SVR_CTL_STOP_SERVER_THREAD) {
        m_bThreadStop = MTRUE;
        s = pthread_join(m_ThreadID, &res);

    } else if (Cmd == CCT_SVR_CTL_WAIT_SERVER_THREAD) {
        s = pthread_join(m_ThreadID, &res);
    }
    MY_LOGD("CCT_ServerCtrl -");
    return 0;
}


// mode = 0 for FIFO IN
//      = 1 for FIFO OUT
MINT32 OpenFifo( MINT32 *fd, char *FifoFilename, LNX_FIFO_MODE_T mode )
{
    MINT32 flag;
    MINT32 state;
    umask(0);
#if 1
    // the FIFO needs to be created by the server side. If it is not existed, the client side
    // should wait for the server to create it.
    while ( (state = access(FifoFilename, F_OK)) != 0 );    // Need to add a timeout here
    MY_LOGD("FIFO file %s exist",FifoFilename);
#else
    state = access(FifoFilename, F_OK);
    if (state == 0)
        MY_LOGD("FIFO IN file %s exist",FifoFilename);
    else if(state == -1)
    {
        MY_LOGD("Create FIFO file %s",FifoFilename);
        if(mkfifo(FifoFilename, S_IFIFO|0777)<0)  { // create pipe
            MY_LOGE("mkfifo() failed");
            return -1;
        }
        else
            MY_LOGD("Create FIFO file success");
    }
#endif
    if (mode == LNX_FIFO_IN_MODE)
        flag = O_RDONLY;
    else
        flag = O_WRONLY;
    // open pipe
    *fd = open(FifoFilename, flag, 0);
    if(*fd == -1)
    {
        MY_LOGE("Open FIFO failed");
        return -1;
    }
    MY_LOGD("Open FIFO file %s success",FifoFilename);
    return 0;
}

MINT32 CloseFifo( MINT32 fd, char *FifoFilename )
{
    int state;
    state = close(fd);
    //unlink(FifoFilename);
    if (state == 0)
        MY_LOGD("Close FIFO file %s success", FifoFilename);
    else
        MY_LOGE("Close FIFO file failed, state=%d\n", state);
    return state;
}

// mode = 0 for FIFO IN
//      = 1 for FIFO OUT
MINT32 OpenFifoHost( MINT32 *fd, char *FifoFilename, LNX_FIFO_MODE_T mode )
{
    MINT32 flag;
    MINT32 state;
    MINT32 err;

    umask(0);

    state = access(FifoFilename, F_OK);

    if (state == 0) {
        MY_LOGD("FIFO file %s exist",FifoFilename);
    }
    else if(state == -1)
    {
        MY_LOGD("Create FIFO file %s",FifoFilename);
        if(mkfifo(FifoFilename, S_IFIFO|0777)<0)  { // create pipe
            err = errno;
            MY_LOGE("mkfifo() failed %d",err);
            return -1;
        }
        else
            MY_LOGD("Create FIFO file success\n");
    }

    if (mode == LNX_FIFO_IN_MODE)  // FIFO in
        flag = O_RDONLY | O_NONBLOCK;
    else  // FIFO out
        flag = O_WRONLY | O_NONBLOCK;

    *fd = open(FifoFilename, flag, 0); // open pipe
    if(*fd == -1)
    {
        err = errno;
        if (err == ENXIO) {      /* == 6 ... No such device or address */
            return -2;    // the other end of FIFO not ready yet
        } else {
            MY_LOGE("open FIFO failure - %d",err);
            return -1;
        }
    }
    return 0;
}


MINT32 CloseFifoHost( MINT32 fd, char *FifoFilename )
{
    int state;

    state = close(fd);
    unlink(FifoFilename);
    if (state == 0)
        MY_LOGD("Close FIFO file success\n");
    else
        MY_LOGE("Close FIFO file failed, state=%d\n", state);
    return state;
}


MINT32 ReadCctOp( MUINT32 *Op, MUINT32 *Size, MINT32 fd)
{
    MUINT32 temp[2];
    MINT32 len;

    if ( (len = read(fd, temp, sizeof(temp)) ) < 0 )
        return -1;
    if ( len == sizeof(temp) ) {
        *Op = temp[0];
        *Size = temp[1];
    }

    return len;
}

#if 1
MINT32 ReadCctInData( MUINT32 Size, MUINT8 *inBuf, MUINT32 *RetSize, MINT32 fd)
{
    MINT32 readSize, accReadSize;
    MINT32 retryCount=0;

    accReadSize = 0;
    readSize = 0;
    while (accReadSize < (int)Size && readSize >= 0 ) {
        readSize = read(fd, (inBuf+accReadSize), (Size-accReadSize));
        if (readSize > 0)
            accReadSize += readSize;
        else if (readSize == -1){
            MINT32 err = errno;
            MY_LOGD("ReadCctInData: errno=%d\n",err);
            retryCount++;
            if(err == EAGAIN && retryCount < 100){
                MY_LOGD("ReadCctInData: wait and read again");
                readSize = 0;
                usleep(2000);
            }
        }
    }

    *RetSize = accReadSize;

    if ( readSize < 0 )
        return -1;

    return accReadSize;
}

#else
MINT32 ReadCctInData( MUINT32 Size, MUINT8 *inBuf, MUINT32 *RetSize, MINT32 fd)
{
    MINT32 RSize;
    RSize = read(fd, inBuf, Size);

    if ( RSize < 0 )
        return -1;
    *RetSize = RSize;

    return RSize;
}
#endif

MINT32 WriteCctOutStatus( MUINT32 Op, MINT32 Status, MINT32 Size, MINT32 fd)
{
    MUINT32 temp[3];

    temp[0] = Op;
    temp[1] = Status;
    temp[2] = Size;

    if (write(fd, temp, sizeof(temp)) != sizeof(temp))
        return -1;

    return 0;
}

#if 1
MINT32 WriteCctOutData( MUINT32 Size, MUINT8 *outBuf, MINT32 fd)
{
    if (outBuf == NULL)
    {
        return -1;
    }

    MINT32 outSize, accOutSize;
    MINT32 blockSize;
    MINT32 err;
    MINT32 retryCount=0;

    accOutSize = 0;
    outSize = 0;
    blockSize = 32768;
    while (accOutSize < (int)Size && outSize >= 0 ) {
        if ( blockSize > ((int)Size-accOutSize) )
            blockSize = (Size-accOutSize);
        outSize = write(fd, (outBuf+accOutSize), blockSize);
        if (outSize > 0 )
            accOutSize += outSize;
        else if (outSize < 0) {
            err = errno;
            MY_LOGD("WriteCctOutData: error=%d  accOutSize=%d",err, accOutSize);
            retryCount++;
            if (err == EAGAIN && retryCount < 100) {
                outSize = 0;
                usleep(2000);
            }
        }
    }

    if (accOutSize != (int)Size || outSize < 0)
        return -1;

    return 0;   //Size;
}

#else
MINT32 WriteCctOutData( MUINT32 Size, MUINT8 *outBuf, MINT32 fd)
{
    MINT32 OutSize = write(fd, outBuf, Size);

    if (OutSize != Size)
        return -1;

    return 0;   //Size;
}
#endif


extern MBOOL gbCaptureCallback;

void
CctServer::
CCT_CaptureCallback(void *arg, MINT32 arg_size)
{
    (void*) arg;
    MY_LOGD("CCT_CaptureCallback start");

    MUINT32 Op, Size, retSize, outBufSize;
    MUINT8 *inBuf, *outBuf;
    MINT32 Status;
    MINT32 fdin, fdout;
    MINT32 RdLen;
    MINT32 dtype = 0, ret;

    if ( OpenFifo(&fdin, PIPE_IN_FNAME, LNX_FIFO_IN_MODE) == -1 ) {
        MY_LOGE("Open input FIFO failure: %s", PIPE_IN_FNAME);
        return;
    }
    MY_LOGD("Open input FIFO success");

    if ( OpenFifo(&fdout, PIPE_OUT_FNAME, LNX_FIFO_OUT_MODE) == -1 ) {
        MY_LOGE("Open output FIFO failure: %s", PIPE_OUT_FNAME);
        CloseFifo(fdin, (char*)PIPE_IN_FNAME);
        return;
    }
    MY_LOGD("Open output FIFO success");

    gbCaptureCallback = MTRUE;
    WriteCctOutStatus( 0xF1F1, 0, arg_size, fdout );
    WriteCctOutData( arg_size, (MUINT8*) arg, fdout);

    //while ( tthis->m_bThreadStop == MFALSE ) {
    while (1) {
        RdLen = ReadCctOp( &Op, &Size, fdin );
        if ( RdLen == 8 ){
            if(Op == 0xE2E2) {
                MY_LOGD("capture done");
                break;
            }
            else {
                MY_LOGE("capture failed: op=%d, size=%d", Op, Size);
                break;
            }
        } else if (RdLen < 0) {
            MINT32 err = errno;
            if (err != EAGAIN)
                MY_LOGD("Read CCT Op in error %d",err);
            usleep(5000);
        } else if (RdLen > 0 ) {
            MY_LOGE("Read CCT Op incomplete");
        }
    }
    gbCaptureCallback = MFALSE;


    CloseFifo(fdin, PIPE_IN_FNAME);
    CloseFifo(fdout, PIPE_OUT_FNAME);
    fdin = -1;
    fdout = -1;

    MY_LOGD("CCT_CaptureCallback done");
    return;
}

void *
CctServer::
CCT_ServerThread(void *arg)
{
    CctServer *tthis;
    CctHandle *pCctCmdHandle;
    MUINT32 Op, Size, retSize, outBufSize;
    MUINT8 *inBuf, *outBuf;
    MINT32 Status;
    MINT32 fdin, fdout;
    MINT32 RdLen;
    MINT32 dtype = 0, ret;

    MY_LOGD("CCT_ServerThread: Start");

#if 0
    Status = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (Status != 0) {
        MY_LOGD("CCT_ServerThread: cannot be set to cancelable");
        printf("CCT_ServerThread: cannot be set to cancelable\n");
    }

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif

    tthis = (CctServer *) arg;

    auto capCB = CctServer::CCT_CaptureCallback;
    pCctCmdHandle = tthis->m_pCctHd;
    pCctCmdHandle->init(capCB);

    if ( OpenFifoHost(&fdin, PIPE_IN_FNAME, LNX_FIFO_IN_MODE) == -1 ) {
        MY_LOGD("CCT_ServerThread: Open input FIFO failure");
        printf("Open Input FIFO faulure \n");
        return arg;
    }

    MY_LOGD("CCT_ServerThread: Open input FIFO success");

    MY_LOGD("CCT_ServerThread: Open output FIFO %s ...",PIPE_OUT_FNAME);
#if 0
    if ( OpenFifoHost(&fdout, PIPE_OUT_FNAME, LNX_FIFO_OUT_MODE) == -1 ) {
        MY_LOGD("CCT_ServerThread: Open output FIFO failure");
        printf("Open Output FIFO faulure \n");
        return arg;
    }
#elif 0
    while (OpenFifoHost(&fdout, PIPE_OUT_FNAME, LNX_FIFO_OUT_MODE) == -1 && tthis->m_bThreadStop == MFALSE) {
        MY_LOGD("Open output FIFO failure");
        sleep(1);
    }
#else
    do {
        ret = OpenFifoHost(&fdout, PIPE_OUT_FNAME, LNX_FIFO_OUT_MODE);
        if (ret != 0) {
            if (ret != -2)
                MY_LOGD("Open output FIFO failure");
            sleep(1);
        }
    } while (ret != 0 && tthis->m_bThreadStop == MFALSE) ;
#endif
    if (tthis->m_bThreadStop == MFALSE) {
        MY_LOGD("CCT_ServerThread: Open output FIFO success");
    }

    while ( tthis->m_bThreadStop == MFALSE ) {
        RdLen = ReadCctOp( &Op, &Size, fdin );
        if ( RdLen == 8 ) {
            MY_LOGD("[CCT Server]Op:%x  Size:%x\n",Op, Size);
            //printf("[CCT Server]Op:%x  Size:%x\n",Op, Size);
            if (Size > 0)
                inBuf = new MUINT8 [Size];
            else
                inBuf = NULL;

            if (Size > 0)
                while ( (RdLen = ReadCctInData( Size, inBuf, &retSize, fdin )) == 0 ); // Need to add a timeout here
            else if (Size == 0) {
                RdLen = 0;
                retSize = 0;
            }

//            outBufSize = pCctCmdHandle->cct_GetCctOutBufSize((CCT_OP_ID)Op, dtype);

            MINT32 statusAndSize = pCctCmdHandle->cct_GetCctOutBufSize((CCT_OP_ID)Op, RdLen, inBuf);
            MY_LOGD("[CCT Server]Output size:%d\n",statusAndSize);
            if (statusAndSize > 0)
            {
                outBufSize = static_cast<MUINT32>(statusAndSize);
                outBuf = new MUINT8 [outBufSize];
            }
            else
                outBuf = NULL;

            if (RdLen < 0) {
                // read CCT In data failure
                MY_LOGD("Read CCT in data failure \n");
            } else if ( RdLen != (int)Size ) {
                MY_LOGD("Read CCT in data incomplete \n");
            } else {
                if (RdLen > 3)
                    //printf("[CCT Server]inBuf data:%x %x %x %x\n",inBuf[0], inBuf[1], inBuf[2], inBuf[3]);
                    if(inBuf != NULL)
                    {
                        MY_LOGD("[CCT Server]inBuf data:%x %x %x %x\n",inBuf[0], inBuf[1], inBuf[2], inBuf[3]);
                    }

                if ( retSize == Size ) {

                    Status = pCctCmdHandle->cct_OpDispatch((CCT_OP_ID)Op, Size, inBuf, outBufSize, outBuf, &retSize);

                    MY_LOGD("[CCT Server]CCT cmd done status %x RetSize %x",Status, retSize);

                    WriteCctOutStatus( Op, Status, retSize, fdout );
                    MY_LOGD("[CCT Server]Output CCT cmd status %x and retSize %x",Status, retSize);

                    if ( Status == 0 ) {
                        if (retSize > 0) {
                            Status = WriteCctOutData( retSize, outBuf, fdout );
                            MY_LOGD("[CCT Server]CCT data out done status %x RetSize %x",Status, retSize);
                        }
                    } else {
                        // Cct Op handling error
                        MY_LOGD("CCT Op handling error \n");
                    }

                } else {
                    // input data incorrect
                    MY_LOGD("CCT input data incorrect \n");
                }
            }

            if (inBuf)
                delete [] inBuf;
            if (outBuf)
                delete [] outBuf;
        } else if (RdLen < 0) {
            MINT32 err = errno;
            if (err != EAGAIN)      /* == 11 ... Try again */
                MY_LOGD("Read CCT Op in error %d\n",err);
            //else
                //MY_LOGD("No CCT Op in %d\n",err);
            usleep(5000);
        } else if (RdLen > 0 ) {
            MY_LOGD("Read CCT Op incomplete \n");
        }
    }

    CloseFifoHost(fdin, PIPE_IN_FNAME);
    CloseFifoHost(fdout, PIPE_OUT_FNAME);
    fdin = -1;
    fdout = -1;

    MY_LOGD("CCT_ServerThread: End");
    return arg;

}


