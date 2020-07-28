/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * File name:  RfxClassInfo.h
 * Author: Jun Liu (MTK80064)
 * Description:
 *  Define the prototypes of class info and property pointer.
 *  Implement dynamic access to property of C++ objects.
 */

#ifndef __RFX_CLASS_INFO_H__
#define __RFX_CLASS_INFO_H__

/***************************************************************************** 
 * Include
 *****************************************************************************/
#include <sys/types.h>

/***************************************************************************** 
 * Typedef
 *****************************************************************************/

// Pre-defined classes
class RfxObject;

// Prototype of RfxClassInfo create object function pointer
typedef RfxObject *(*RfxClassInfoCreateObjectFuncPtr)(RfxObject *parent);


/***************************************************************************** 
 * Class RfxClassInfo
 *****************************************************************************/
typedef struct _rfx_class_info_struct
{
    // pointer to the name of the class
    const char *m_class_name;
    // pointer to the parent class info of the class
    const struct _rfx_class_info_struct *m_parent;
    // Function pointer to create a object instance
    RfxClassInfoCreateObjectFuncPtr m_create_object_callback;
} rfx_class_info_struct;

/*
 * Class RfxClassInfo, 
 *  prototype of class information, descript the class name, the parent class, 
 *  and how to create instances of the class
 */
class RfxClassInfo : public _rfx_class_info_struct
{
// Constructor / Destructor
public:
    // Constructor
    RfxClassInfo(
        const char *class_name,
        struct _rfx_class_info_struct *parent,
        RfxClassInfoCreateObjectFuncPtr create_obj_instance_callback) {
        m_class_name = class_name;
        m_parent = parent;
        m_create_object_callback = create_obj_instance_callback;
    }

// Method
public:
    // Return the class name.
    //
    // RETURNS: Return the class name.
    const char *getClassName() const {
        return m_class_name;
    }

    // Return the parent class into.
    //
    // RETURNS: Return the parent class. Return NULL if there is no parent class
    RfxClassInfo *getParent() const {
        return (RfxClassInfo *)m_parent;
    }

    // Check the class whatever inherit from the given class.
    //
    // RETURNS: Return true if the class inherit from the given class.
    //  Otherwise, return false.
    bool isKindOf(
        const RfxClassInfo *classInfo   // [IN] The class to check
    ) const;

    // Create an object instance of the class.
    RfxObject *createObject(
        RfxObject *parent               // [IN] The parent object to create new object
    ) const {
        return m_create_object_callback ? m_create_object_callback(parent) : NULL;
    }

};

#endif /* __RFX_CLASS_INFO_H__ */

