/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_UTILS_CONTROLLERCONTAINER_H_
#define _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_UTILS_CONTROLLERCONTAINER_H_

// MTKCAM
#include <mtkcam/def/BuiltinTypes.h>

// STL
#include <deque>
#include <memory>
#include <mutex>
#include <algorithm> // std::find
#include <functional>
#include <iterator> // std::back_inserter

namespace NSCam {
namespace plugin {


// A container to contain the smart pointer of BaseController and derived classes,
// or any class with these public methods:
//
//      // To check if the specified request number reqNum is belong to the
//      // controller.
//      bool T_Controller::isBelong(MUINT32 reqNum);
//
//
// And caller can specify smart pointer type by the second template argument
// T_SmartPtr. Default we use std::shared_ptr<T_Controller> as the smart pointer
// type name. E.g.:
//
//      ControllerContainer< DerivedController, android::sp<DerivedController> > ctrlContainer;
//
// For different implementations, the container of controller may need be replaced
// , such as using std::vector, or std::list. However, usually std::deque is
// much better for frequently push and pop. Caller can specify the container
// type by the 3-rd template argument. E.g:
//
//      ControllerContainer<
//          DerivedController,
//          android::sp<DerivedController>,
//          std::vector< android::sp<DerivedController> >
//          >
//          ctrlContainer;
//
// Due to thread-safe mechanism, a locker is necessary to avoid race condition.
// The default locker is std::mutex, caller can specify customized locker with
// these two methods:
//
//      void T_Locker::lock(); // lock
//      void T_Locker::unlock(); // unlock
//
// Note:  This class is thread-safe and reentrant class,
//        hence, this class doesn't provide any iterator. It is designed for
//        a multi-thread accessing. Caller who wants to iterate all element or
//        access some elements in this container can invoke
//
//          ControllerContainer::iterate
//
//        with a std::function which receives a call by reference value of the
//        data container.
//
//        E.g.:
//          ControllerContainer<MyController> c;
//
//          // Caller wants iterate all elements in c,
//          // the type of container can be retrieved by ContainerType::T from
//          // ControllerContainer, or if caller already knows(supposed to) the
//          // type of container.
//
//          // retrieve from ControllerContainer<MyController>::ContainerType
//          typedef ControllerContainer<MyController>::ContainerType Data_T1;
//          // or directly declare it because caller knows the container type
//          typedef std::deque< std::shared_ptr<MyController> > Data_T2;
//
//          // iterate all elements
//          c.iterate([](Data_T1& data){ // where Data_T1 and be replaced by Data_T2
//              for (auto&& itr : data) {
//                  // do something with each data (thread-safe)
//                  itr.doSomething();
//              }
//          });
//
template <
    typename T_Controller /* = BaseController */,
    typename T_SmartPtr   = std::shared_ptr<T_Controller>,
    typename T_Container  = std::deque<T_SmartPtr>,
    typename T_Locker     = std::mutex
    >
class ControllerContainer {
public:
    typedef T_Controller    ControllerType;
    typedef T_SmartPtr      SmartPtrType;
    typedef T_Container     ContainerType;
    typedef T_Locker        LockerType;

public:
    // To get the first controller. If the container is empty, returns nullptr.
    //  @return                 The smart pointer of controller, may be nullptr.
    T_SmartPtr                  front() const;

    // To get the last controller. If the container is empty, returns nullptr.
    //  @return                 The smart pointer of controller. Maybe nullptr.
    T_SmartPtr                  back() const;

    // To check if the container is empty or not.
    //  @return                 True for empty, false for not.
    bool                        empty() const;

    // To take the first controller. If the container is empty, returns nullptr.
    //  @return                 The smart pointer of controller. Maybe nullptr.
    T_SmartPtr                  take_first();

    // To take the last controller. If the container is empty, returns nullptr.
    //  @return                 The smart pointer of controller. Maybe nullptr.
    T_SmartPtr                  take_last();

    // To get the first controller which matched of request number, if container
    // is empty, returns nullptr.
    //  @param reqNo            Request number.
    //  @return                 The smart pointer of controller, maybe nullptr.
    T_SmartPtr                  belong_to(MUINT32 reqNo) const;

    // To find the controller by it's address. If not found or empty, returns
    // nullptr.
    //  @param c                Smart ponter of the controller.
    //  @return                 The smart pointer of controller, maybe nullptr.
    T_SmartPtr                  find(const T_SmartPtr& c) const;

    // To remove the controller by it's address. If removed ok return true.
    // If not found or container is empty, returns false.
    //  @param c                Smart pointer of the controller.
    //  @return                 Removed or not.
    bool                        remove(const T_SmartPtr& c);

    // To add a controller from back.
    //  @param c                Controller to be added.
    void                        push_back(const T_SmartPtr& c);

    // To take all the controllers.
    //  @return                 All the controllers.
    T_Container                 take_all();

    // To clear all the controllers.
    void                        clear();

    // To get the size of container.
    size_t                      size() const;

    // To access container thread-safe-ly.
    //  @param func             std::function object, which receives a
    //                          attribute of the T_Container container reference.
    void                        iterate(
                                    std::function<void(T_Container&)>
                                        func
                                    );

//
// Assign operator
//
public:
    ControllerContainer&        operator =  (const ControllerContainer& o);
    ControllerContainer&        operator =  (ControllerContainer&& o);
    ControllerContainer         operator +  (const ControllerContainer& o) const;
    ControllerContainer&        operator += (const ControllerContainer& o);
    ControllerContainer&        operator += (ControllerContainer&& o);
    bool                        operator == (const ControllerContainer& o) const;
    bool                        operator != (const ControllerContainer& o) const;


//
// Attributes
//
private:
    T_Container         m_data;
    mutable T_Locker    m_lock;


public:
    ControllerContainer() = default;
    ControllerContainer(const T_Container& data);
    ControllerContainer(T_Container&& data);
    ControllerContainer(const ControllerContainer&);
    ControllerContainer(ControllerContainer&&);

}; // class ControllerContainer

#include "ControllerContainer.cpp"

}; // namespace plugin
}; // namespace NSCam
#endif // _MTK_HARDWARE_MTKCAM_PIPELINE_EXTENSION_UTILS_CONTROLLERCONTAINER_H_
