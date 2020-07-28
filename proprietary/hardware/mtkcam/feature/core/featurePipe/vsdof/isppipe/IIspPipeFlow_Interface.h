/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/**
 * @file IIspPipeFlowControler.h
 * @brief flow control interface for IspPipe
 */

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISPPIPE_FLOW_INTERFACE_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_ISPPIPE_FLOW_INTERFACE_H_

// Standard C header file
#include <string>
// Android system/core header file
#include <utils/RefBase.h>
// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/*******************************************************************************
* Class Definition
********************************************************************************/
/**
 * @class INameable
 * @brief interface for that is able to get name
 */
class INameable : public virtual RefBase
{
public:
    /**
    * @brief get the name
     * @return
     * - the name
     */
    virtual const std::string& getName() const = 0;

    virtual ~INameable(){};
};

/**
 * @class IIspPipeConverter
 * @brief convert IIspPipe data to other type
 */
class IIspPipeConverter : public INameable
{
public:
    /**
    * @brief convert buffer id to string
     * @return
     * - the buffer id name
     */
    virtual const std::string& bufferIdtoString(IspPipeBufferID id) const = 0;

    virtual ~IIspPipeConverter(){};

public:
    static const std::string& unknown;
};

/**
 * @class IDataIDToBIDMapProvider
 * @brief provide the map of DataId to BufferIds
 */
using DataIDToBIDMap = KeyedVector<IspPipeDataID, Vector<IspPipeBufferID>>;
class IDataIDToBIDMapProvider : public INameable
{
public:
    /**
    * @brief get the map of DataId to BufferIds
     * @return
     * - the map of DataId to BufferIds
     */
    virtual const DataIDToBIDMap& getMap() const = 0;

    virtual ~IDataIDToBIDMapProvider(){};
};

/**
 * @class EmptyIspPipeConverter
 * @brief empty implementation for IIspPipeConverter that offen uesd on default
 */
class EmptyIspPipeConverter final: public IIspPipeConverter
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IIspPipeConverter Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    const std::string& getName() const override
    {
        static const std::string name = "Empty";
        return name;
    }

    const std::string& bufferIdtoString(IspPipeBufferID id) const override
    {
        return IIspPipeConverter::unknown;
    }
};

/**
 * @class EmptyDataIDToBIDMapProvider
 * @brief empty implementation for IDataIDToBIDMapProvider that offen uesd on default
 */
class EmptyDataIDToBIDMapProvider final: public IDataIDToBIDMapProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IDataIDToBIDMapProvider Public Operator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    const std::string& getName() const override
    {
        static const std::string name = "Empty";
        return name;
    }

    const DataIDToBIDMap& getMap() const override
    {
        static const DataIDToBIDMap map;
        return map;
    }
};


}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam


#endif