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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkMhalEng.h

#ifndef _ACDKMHALENG_H_
#define _ACDKMHALENG_H_


#ifndef ACDK_DUMMY_ENG

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>

#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

#include <utils/List.h>
#include <utils/Mutex.h>    // android mutex
using android::Mutex;

#include <vector>
using std::vector;

using NSCam::NSIoPipe::NSCamIOPipe::INormalPipe;
using NSCam::NSIoPipe::NSPostProc::INormalStream;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/aaa/IHalISP.h>

using namespace NS3Av3;

#define ACDK_CAMERA_AAA

namespace NSAcdkMhal {

#if 1
    typedef IHal3A          IHal3A_T;
    typedef IHalISP         IHalISP_T;
#else
#endif

    /**
         *@class AcdkMhalEng
         *@brief This class is the implementation of AcdkMhalBase and also communicates with 3A
       */
    class AcdkMhalEng     : public AcdkMhalBase, public IHal3ACb {
    public :
        typedef struct _acdkBufCtl_t {
            MUINT32     mEnqCount;
            MUINT32     mDeqCount;
            vector<MUINTPTR>    mvBuffer;   ///< IImageBuffer*
            mutable Mutex       mLock;
                        _acdkBufCtl_t()
                            : mEnqCount(0)
                            , mDeqCount(0)
                        {
                        }
        } acdkBufCtl_t;

        typedef enum {
            ACDK_CAPRAW_IDLE = 0,
            ACDK_CAPRAW_READY,
            ACDK_CAPRAW_START,
            ACDK_CAPRAW_MAX
        } acdkCapRawStatus_t;
        typedef struct _acdkCapRawInfo_t {
            acdkCapRawStatus_t  CapStatus;
            MUINT32             CapImgFormat;
            IImageBuffer*       mBufferCap;
            IImageBuffer*       mBufferQv;
                        _acdkCapRawInfo_t()
                            : CapStatus(ACDK_CAPRAW_IDLE)
                            , CapImgFormat(eImgFmt_YUY2)
                            , mBufferCap(NULL)
                            , mBufferQv(NULL)
                        {
                        }
        } acdkCapRawInfo_t;

        /**
                   *@brief AcdkMhalEng constructor
                 */
        AcdkMhalEng();

        /**
                   *@brief AcdkMhalEng destructor
                 */
        ~AcdkMhalEng() {};

        /**
                   *@brief Destory AcdkMhalEng Object
                 */
        virtual void destroyInstance();

        /**
                   *@brief Set current sate to newState
                   *
                   *@param[in] newState : new state
                 */
        virtual MVOID acdkMhalSetState(acdkMhalState_e newState);

        /**
                   *@brief Get current sate of AcdkMhalBase
                 */
        virtual acdkMhalState_e acdkMhalGetState();

        /**
                   *@brief Indicates whether is ready for capture or not
                   *@return
                   *-MFALSE indicates not ready, MTRUE indicates ready
                 */
        virtual MBOOL acdkMhalReadyForCap();

        /**
                   *@brief Initialize function
                   *@note Must call this function right after createInstance and before other functions
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhalInit();

        /**
                   *@brief Uninitialize function
                   *@note Must call this function before destroyInstance
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhalUninit();

        /**
                   *@brief Handle callback
                   *
                   *@param[in] a_type : callback type
                   *@param[in] a_addr1 : return data address
                   *@param[in] a_addr2 : return data address
                   *@param[in] a_dataSize : return data size
                 */
        virtual MVOID  acdkMhalCBHandle(MUINT32 a_type, MUINT8* a_addr1, MUINT8* a_addr2 = 0, MUINT32 const a_dataSize = 0);

        /**
                   *@brief Start preview
                   *@note Config and control ISP to start preview
                   *
                   *@param[in] a_pBuffIn : pointer to acdkMhalPrvParam_t data
                   *
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhalPreviewStart(MVOID *a_pBuffIn);

        /**
                   *@brief Stop preview
                   *@note Config and control ISP to stop preview
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhalPreviewStop();

        /**
                   *@brief Start capture
                   *
                   *@param[in] a_pBuffIn
                   *
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhalCaptureStart(MVOID *a_pBuffIn);

        /**
                   *@brief Stop capture
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhalCaptureStop();

        /**
                   *@brief Execute preview process
                   *@note Here is a preview loop
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhalPreviewProc();

        /**
                   *@brief Change state to preCapture state and do related opertion
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhalPreCapture();

        /**
                   *@brief  Execute capture process
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhalCaptureProc();

        /**
                   *@brief  Get shutter time in us
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MUINT32 acdkMhalGetShutTime();

        /**
                   *@brief  Set shutter time
                   *
                   *@param[in] a_time : specific shutter time in us
                   *
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MVOID acdkMhalSetShutTime(MUINT32 a_time);
        /**
                   *@brief  Get AF information sucuess or fail
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MUINT32 acdkMhalGetAFInfo();
        /**
                   *@brief  Get Capture mode raw data
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MUINT32 acdkMhalGetCaptureRaw(AMEM_BUF_INFO* pCapBufInfo, AMEM_BUF_INFO* pQvBufInfo, MUINT32 Format);
    private :

        /**
                   *@brief  Set 3A Parameter
                   *
                   *@param[in] devID : sensor device
                   *
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual MINT32 acdkMhal3ASetParam();

        /**
                   *@brief  Set 3A doNotifyCb
                   *
                   *@param[in] _msgType : callback message type
                   *@param[in] _ext1 : callback info
                   *@param[in] _ext2 : callback info
                   *@param[in] _ext3 : callback info
                   *
                   *@return
                   *-0 indicates success, otherwise indicates fail
                 */
        virtual void doNotifyCb(MINT32  _msgType,
                                MINTPTR _ext1,
                                MINTPTR _ext2,
                                MINTPTR _ext3);

        /**
                   *@brief  Process 3A is magic
                   *
                   *@param[in] Magic : indicate current setting
                   *@param[in] Sof : for check frame latency
                   *
                   *@return
                 */
        void 	onProcess3A(void);

        /**
                   *@brief  Return and increment magic number
                   *
                   *@return New magic number
                 */
        MUINT32 requestMagicNum(void);

        /**
                   *@brief  Create metadata and push to 3AList
                   *
                   *@return Success or fail
                 */
        MBOOL   addFrameMeta(MUINT32 Magic, MBOOL bDummy);

        /******************************************************************************/

        mutable Mutex               mLock;

        INormalPipe*                m_pINormPipe;
        INormalStream*              m_pINormStream;

        PortID                      mPortID;

        MBOOL                       mFocusDone;
        MBOOL                       mFocusSucceed;

        MBOOL                       mRawPostProcSupport;
        MBOOL                       mReadyForCap;
        MUINT32                     mu4SensorDelay;
        MUINT32                     mMagicNum;

        acdkMhalPrvParam_t          mAcdkMhalPrvParam;
        acdkMhalCapParam_t          mAcdkMhalCapParam;
        acdkMhalState_e             mAcdkMhalState;

        acdkBufCtl_t                mImgoRtbc;
        acdkBufCtl_t                mRrzoRtbc;
        acdkBufCtl_t                mWrotoBufCtl;

        acdkCapRawInfo_t            mCapRawInfo;

        #ifdef ACDK_CAMERA_AAA
        IHal3A_T*                   mpAaa;
        IHalISP_T*                  mpISP;

        IMetadata                   mAppMeta;
        IMetadata                   mHalMeta;

        mutable Mutex               mControls3AQueLock;
        std::vector< MetaSet_T >    mControls3AQue; // 3A suggest using vector instead of List
        std::vector< MetaSet_T* >   pControls3AQue;

        #endif
    };
};
#endif

#ifdef ACDK_DUMMY_ENG
namespace NSAcdkMhal {
    /**
         *@class AcdkMhalEng
         *@brief This class is the implementation of AcdkMhalBase and also communicates with 3A
       */
    //class AcdkMhalEng : public AcdkMhalBase,
    //                        public I3ACallBack
    class AcdkMhalEng     : public AcdkMhalBase//, public I3ACallBack
    {
        public :

            /**
                       *@brief AcdkMhalEng constructor
                     */
            AcdkMhalEng() {}

            /**
                       *@brief AcdkMhalEng destructor
                     */
            ~AcdkMhalEng() {};

            /**
                       *@brief Destory AcdkMhalEng Object
                     */
            virtual void destroyInstance() {}

            /**
                       *@brief Set current sate to newState
                       *
                       *@param[in] newState : new state
                     */
            virtual MVOID acdkMhalSetState(acdkMhalState_e newState){}

            /**
                       *@brief Get current sate of AcdkMhalBase
                     */
            virtual acdkMhalState_e acdkMhalGetState(){return (acdkMhalState_e)0;}

            /**
                       *@brief Indicates whether is ready for capture or not
                       *@return
                       *-MFALSE indicates not ready, MTRUE indicates ready
                     */
            virtual MBOOL acdkMhalReadyForCap(){return MTRUE;}

            /**
                       *@brief Initialize function
                       *@note Must call this function right after createInstance and before other functions
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalInit(){return 0;}

            /**
                       *@brief Uninitialize function
                       *@note Must call this function before destroyInstance
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalUninit(){return 0;}

            /**
                       *@brief Handle callback
                       *
                       *@param[in] a_type : callback type
                       *@param[in] a_addr1 : return data address
                       *@param[in] a_addr2 : return data address
                       *@param[in] a_dataSize : return data size
                     */
            virtual MVOID  acdkMhalCBHandle(MUINT32 a_type, MUINT8* a_addr1, MUINT8* a_addr2 = 0, MUINT32 const a_dataSize = 0){}

            /**
                       *@brief Start preview
                       *@note Config and control ISP to start preview
                       *
                       *@param[in] a_pBuffIn : pointer to acdkMhalPrvParam_t data
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalPreviewStart(MVOID *a_pBuffIn){return 0;}

            /**
                       *@brief Stop preview
                       *@note Config and control ISP to stop preview
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalPreviewStop(){return 0;}

            /**
                       *@brief Start capture
                       *
                       *@param[in] a_pBuffIn
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalCaptureStart(MVOID *a_pBuffIn){return 0;}

            /**
                       *@brief Stop capture
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalCaptureStop(){return 0;}

            /**
                       *@brief Execute preview process
                       *@note Here is a preview loop
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalPreviewProc(){return 0;}

            /**
                       *@brief Change state to preCapture state and do related opertion
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalPreCapture(){return 0;}

            /**
                       *@brief  Execute capture process
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhalCaptureProc(){return 0;}

            /**
                       *@brief  Get shutter time in us
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MUINT32 acdkMhalGetShutTime(){return 0;}

            /**
                       *@brief  Set shutter time
                       *
                       *@param[in] a_time : specific shutter time in us
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MVOID acdkMhalSetShutTime(MUINT32 a_time){}
            /**
                       *@brief  Get AF information sucuess or fail
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MUINT32 acdkMhalGetAFInfo(){return 0;}
        private :

            /**
                       *@brief  Set 3A Parameter
                       *
                       *@param[in] devID : sensor device
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkMhal3ASetParam(MINT32 devID, MUINT8 IsFactory){return 0;}

            /**
                       *@brief  Set 3A doNotifyCb
                       *
                       *@param[in] _msgType : callback message type
                       *@param[in] _ext1 : callback info
                       *@param[in] _ext2 : callback info
                       *@param[in] _ext3 : callback info
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual void doNotifyCb(int32_t _msgType,
                                        int32_t _ext1,
                                        int32_t _ext2,
                                        int32_t _ext3){}

            /**
                       *@brief  Set 3A doNotifyCb
                       *
                       *@param[in] _msgType : callback message type
                       *@param[in] _data : callback data
                       *@param[in] _size : callback data size
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual void doDataCb(int32_t _msgType,
                                      void *_data,
                                      uint32_t _size){}

            virtual MUINT32 acdkMhalGetCaptureRaw(AMEM_BUF_INFO* pCapBufInfo, AMEM_BUF_INFO* pQvBufInfo, MUINT32 Format){return 0;}

            /******************************************************************************/
    };
};
#endif

#endif //end AcdkMhalEng.h

