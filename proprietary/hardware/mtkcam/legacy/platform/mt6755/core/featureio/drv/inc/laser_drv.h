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
/*
** $Log: lens_drv.h $
 *
*/

#ifndef _LASER_DRV_H_
#define _LASER_DRV_H_

#include <utils/threads.h>

#include <linux/ioctl.h>

using namespace android;

extern "C" {
#include <pthread.h>
#include <semaphore.h>
}


/*******************************************************************************
*
********************************************************************************/
#define LASER_MAGIC 'A'

/*#define LASER_IOCTL_INIT 			_IO(LASER_MAGIC, 0x01)
//#define LASER_IOCTL_XTALKCALB		_IO('p', 0x02)
//#define LASER_IOCTL_OFFCALB		_IO('p', 0x03)
//#define LASER_IOCTL_STOP			_IO('p', 0x05)
//#define LASER_IOCTL_SETXTALK		_IOW('p', 0x06, unsigned int)
//#define LASER_IOCTL_SETOFFSET		_IOW('p', 0x07, int8_t)
#define LASER_IOCTL_GETDATA 		_IOR(LASER_MAGIC, 0x0a, unsigned int)
//#define LASER_IOCTL_GETDATAS 		_IOR('p', 0x0b, RangeData)
//#define LASER_IOCTL_GETDATAS 		_IOR('p', 0x0b, VL6180x_RangeData_t)*/
#define LASER_IOCTL_INIT 			_IO(LASER_MAGIC, 	0x01)
#define LASER_IOCTL_GETOFFCALB		_IOR(LASER_MAGIC, 	0x02, int)
#define LASER_IOCTL_GETXTALKCALB	_IOR(LASER_MAGIC, 	0X03, int)
#define LASER_IOCTL_SETOFFCALB		_IOW(LASER_MAGIC, 	0x04, int)
#define LASER_IOCTL_SETXTALKCALB	_IOW(LASER_MAGIC, 	0x05, int)
#define LASER_IOCTL_GETDATA 		_IOR(LASER_MAGIC, 	0x0a, int)


#define LASER_GOLDEN_TABLE_LEVEL	15

/*******************************************************************************
*
********************************************************************************/
class LaserDrv
{
private:    //// Instanciation outside is disallowed.
    /////////////////////////////////////////////////////////////////////////
    //
    // LaserDrv () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////                       
    LaserDrv();

    /////////////////////////////////////////////////////////////////////////
    //
    // ~LaserDrv () -
    //! \brief
    //
    /////////////////////////////////////////////////////////////////////////    
    ~LaserDrv();

public:     //// Interfaces
    /////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////
    static LaserDrv* getInstance();

    /////////////////////////////////////////////////////////////////////////
    //
    /////////////////////////////////////////////////////////////////////////
    void destroyInstance();

    /////////////////////////////////////////////////////////////////////////
    //
    // init () -
    //! \brief 
    //
    /////////////////////////////////////////////////////////////////////////    
    int init(void);

    /////////////////////////////////////////////////////////////////////////
    //
    // uninit () -
    //! \brief 
    //
    /////////////////////////////////////////////////////////////////////////    
    int uninit(void);

    /////////////////////////////////////////////////////////////////////////
    //
    // checkHwSetting () -
    //! \brief 
    //
    /////////////////////////////////////////////////////////////////////////    
    int checkHwSetting(void);

    /////////////////////////////////////////////////////////////////////////
    //
    // setLaserCaliData () -
    //! \brief 
    //
    /////////////////////////////////////////////////////////////////////////    
    void setLaserGoldenTable(unsigned int *Value);	
	
    /////////////////////////////////////////////////////////////////////////
    //
    // setLaserCaliData () -
    //! \brief 
    //
    /////////////////////////////////////////////////////////////////////////    
    void setLensCalibrationData(int LensDAC_10cm, int LensDAC_50cm);	

    /////////////////////////////////////////////////////////////////////////
    //
    // init () -
    //! \brief 
    //
    /////////////////////////////////////////////////////////////////////////    
    void setLaserCalibrationData(unsigned int OffsetData, unsigned int XTalkData);
	
    /////////////////////////////////////////////////////////////////////////
    //
    // getLaserDac () -
    //! \brief 
    //
    /////////////////////////////////////////////////////////////////////////    
    int getLaserRangeValue(int *RangeValue);	
	
    /////////////////////////////////////////////////////////////////////////
    //
    // getLaserDac () -
    //! \brief 
    //
    /////////////////////////////////////////////////////////////////////////    
    int getLaserCurPos(int *DacValue, int *DistValue);	

	/**
	* @brief Enable AF thread
	* @param [in] a_bEnable set 1 to enable AF thread
	*/
    /////////////////////////////////////////////////////////////////////////
    //
    // EnableLaserThread () -
    // @brief Enable Laser thread
    // @param [in] a_bEnable set 1 to enable Laser thread
    //
    /////////////////////////////////////////////////////////////////////////   	
    int EnableLaserThread(int a_bEnable);

    // Calibration Process
    int getLaserOffsetCalib(int *Value);	
    int setLaserOffsetCalib(int Value);
    int getLaserXTalkCalib(int *Value);	
    int setLaserXTalkCalib(int Value);	
	
private:
	/**
	* @brief AF thread execution function
	*/
    static void* LaserThreadFunc(void *arg);

private:
    //
    int             	m_fd_Laser;
	
    int 		m_Lens10cmPosDAC;
    int 		m_Lens50cmPosDAC;
	
    int		m_LensDistDac[LASER_GOLDEN_TABLE_LEVEL];
    int		m_LaserDistRV[LASER_GOLDEN_TABLE_LEVEL];	
	
    int 		m_LaserCurPos_DAC;
    int 		m_LaserCurPos_RV;	

    mutable Mutex mLock;    

    // thread
protected:    
    signed int			m_bLaserThreadLoop;
    pthread_t                 m_LaserThread;
    Mutex          m_LaserMtx;	

};

#endif  //  _LASER_DRV_H_

