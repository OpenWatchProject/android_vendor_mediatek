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
#if defined(__CC_TEMPLATE_DECLARE___) || defined(__CC_CLASSTYPE___)
#   error "ControllerContainer template macro has been defined before it's"\
          "supposed to be defined timimg."
#endif

#define __CC_TEMPLATE_DECLARE___ \
    template<typename T_Controller, typename T_SmartPtr, typename T_Container, typename T_Locker>

#define __CC_CLASSTYPE___ \
    ControllerContainer<T_Controller, T_SmartPtr, T_Container, T_Locker>

__CC_TEMPLATE_DECLARE___
T_SmartPtr __CC_CLASSTYPE___::front() const
{
    std::lock_guard<T_Locker> __l(m_lock);
    if (m_data.empty())
        return nullptr;
    return m_data.front();
}


__CC_TEMPLATE_DECLARE___
T_SmartPtr __CC_CLASSTYPE___::back() const
{
    std::lock_guard<T_Locker> __l(m_lock);
    if (m_data.empty())
        return nullptr;
    return m_data.back();
}


__CC_TEMPLATE_DECLARE___
bool __CC_CLASSTYPE___::empty() const
{
    std::lock_guard<T_Locker> __l(m_lock);
    return m_data.empty();
}


__CC_TEMPLATE_DECLARE___
T_SmartPtr __CC_CLASSTYPE___::take_first()
{
    std::lock_guard<T_Locker> __l(m_lock);
    if (m_data.empty())
        return nullptr;
    auto r = m_data.front();
    m_data.pop_front();
    return r;
}


__CC_TEMPLATE_DECLARE___
T_SmartPtr __CC_CLASSTYPE___::take_last()
{
    std::lock_guard<T_Locker> __l(m_lock);
    if (m_data.empty())
        return nullptr;
    auto r = m_data.back();
    m_data.pop_back();
    return r;
}


__CC_TEMPLATE_DECLARE___
T_SmartPtr __CC_CLASSTYPE___::belong_to(MUINT32 reqNo) const
{
    std::lock_guard<T_Locker> __l(m_lock);
    for (auto&& itr : m_data) {
        if (itr.get() && itr->isBelong(reqNo)) {
            return itr;
        }
    }
    return nullptr;
}


__CC_TEMPLATE_DECLARE___
T_SmartPtr __CC_CLASSTYPE___::find(const T_SmartPtr& c) const
{
    std::lock_guard<T_Locker> __l(m_lock);
    auto itr = std::find(m_data.begin(), m_data.end(), c);
    if (itr == m_data.end())
        return nullptr;
    return itr;
}

__CC_TEMPLATE_DECLARE___
bool __CC_CLASSTYPE___::remove(const T_SmartPtr& c)
{
    std::lock_guard<T_Locker> __l(m_lock);
    auto itr = std::find(m_data.begin(), m_data.end(), c);
    if (itr == m_data.end()) // not found
        return false;
    m_data.erase(itr);
    return true;
}


__CC_TEMPLATE_DECLARE___
void __CC_CLASSTYPE___::push_back(const T_SmartPtr& c)
{
    std::lock_guard<T_Locker> __l(m_lock);
    m_data.push_back(c);
}


__CC_TEMPLATE_DECLARE___
T_Container __CC_CLASSTYPE___::take_all()
{
    T_Container r;
    {
        std::lock_guard<T_Locker> __l(m_lock);
        r = std::move(m_data);
    }
    return r;
}


__CC_TEMPLATE_DECLARE___
void __CC_CLASSTYPE___::clear()
{
    std::lock_guard<T_Locker> __l(m_lock);
    m_data.clear();
}


__CC_TEMPLATE_DECLARE___
size_t __CC_CLASSTYPE___::size() const
{
    std::lock_guard<T_Locker> __l(m_lock);
    return m_data.size();
}



__CC_TEMPLATE_DECLARE___
void __CC_CLASSTYPE___::iterate(
        std::function<void(T_Container&)> func)
{
    std::lock_guard<T_Locker> __l(m_lock);
    func(m_data);
}


__CC_TEMPLATE_DECLARE___
__CC_CLASSTYPE___& __CC_CLASSTYPE___::operator = (const __CC_CLASSTYPE___& o)
{
    if (this != &o) {
        std::lock_guard<T_Locker> __l1(m_lock);
        std::lock_guard<T_Locker> __l2(o.m_lock);
        m_data = o.m_data;
    }
    return *this;
}


__CC_TEMPLATE_DECLARE___
__CC_CLASSTYPE___& __CC_CLASSTYPE___::operator = (__CC_CLASSTYPE___&& o)
{
    if (this != &o) {
        std::lock(m_lock, o.m_lock);
        std::lock_guard<T_Locker> __l1(m_lock, std::adopt_lock);
        std::lock_guard<T_Locker> __l2(o.m_lock, std::adopt_lock);

        m_data = std::move(o.m_data);
    }
    return *this;
}


__CC_TEMPLATE_DECLARE___
__CC_CLASSTYPE___ __CC_CLASSTYPE___::operator + (const __CC_CLASSTYPE___& o) const
{
    T_Container data;

    if (this != &o) {
        // self
        {
            std::lock_guard<T_Locker> __l(m_lock);
            std::copy(m_data.begin(), m_data.end(), std::back_inserter(data));
        }
        // other
        {
            std::lock_guard<T_Locker> __l(o.m_lock);
            std::copy(o.m_data.begin(), o.m_data.end(), std::back_inserter(data));
        }
    }
    else {
        std::lock_guard<T_Locker> __l1(m_lock);

        // copy self twice
        std::copy(m_data.begin(), m_data.end(),
                std::back_inserter(data));
        std::copy(m_data.begin(), m_data.end(),
                std::back_inserter(data));
    }

    return ControllerContainer(std::move(data));
}


__CC_TEMPLATE_DECLARE___
__CC_CLASSTYPE___& __CC_CLASSTYPE___::operator += (const __CC_CLASSTYPE___& o)
{
    if (this != &o) {
        std::lock(m_lock, o.m_lock);
        std::lock_guard<T_Locker> __l1(m_lock, std::adopt_lock);
        std::lock_guard<T_Locker> __l2(o.m_lock, std::adopt_lock);
        // copy from other
        std::copy(o.m_data.begin(), o.m_data.end(),
                std::back_inserter(m_data));
    }
    else {
        // copy self
        std::lock_guard<T_Locker> __l1(m_lock);
        std::copy(m_data.begin(), m_data.end(),
                std::back_inserter(m_data));
    }

    return *this;
}


__CC_TEMPLATE_DECLARE___
__CC_CLASSTYPE___& __CC_CLASSTYPE___::operator += (__CC_CLASSTYPE___&& o)
{
    if (this != &o) {
        std::lock(m_lock, o.m_lock);
        std::lock_guard<T_Locker> lk1(m_lock,   std::adopt_lock);
        std::lock_guard<T_Locker> lk2(o.m_lock, std::adopt_lock);
        // move
        std::move(o.m_data.begin(), o.m_data.end(),
                std::back_inserter(m_data));
    }
    return *this;
}


__CC_TEMPLATE_DECLARE___
bool __CC_CLASSTYPE___::operator == (const __CC_CLASSTYPE___& o) const
{
    // compare address first, if it's the same, they're the same.
    if (this == &o)
        return true;

    // accquire all locks
    std::lock(m_lock, o.m_lock);
    std::lock_guard<T_Locker> lk1(m_lock,   std::adopt_lock);
    std::lock_guard<T_Locker> lk2(o.m_lock, std::adopt_lock);

    // check size first, if the sizes are different, they're not the same.
    if (m_data.size() != o.m_data.size())
        return false;

    // compare all element
    // find my elements in other
    for (auto&& itr : m_data) {
        if (o.m_data.find(itr) == o.m_data.end()) // if not found, not the same!
            return false;
    }

    return true;
}


__CC_TEMPLATE_DECLARE___
bool __CC_CLASSTYPE___::operator != (const __CC_CLASSTYPE___& o) const
{
    return !(*this == o);
}


// constructor from copying a container
__CC_TEMPLATE_DECLARE___
__CC_CLASSTYPE___::ControllerContainer(const T_Container& data)
{
    std::lock(m_lock, data.m_lock);
    std::lock_guard<T_Locker> lk1(m_lock, std::adopt_lock);
    std::lock_guard<T_Locker> lk2(data.m_lock, std::adopt_lock);

    std::copy(data.begin(), data.end(), std::back_inserter(m_data));
}


// constructor from moving a container
__CC_TEMPLATE_DECLARE___
__CC_CLASSTYPE___::ControllerContainer(T_Container&& data)
{
    m_data = std::move(data);
}


// copy constructor
__CC_TEMPLATE_DECLARE___
__CC_CLASSTYPE___::ControllerContainer(const __CC_CLASSTYPE___& o)
{
    *this = o;
}


// move constructor
__CC_TEMPLATE_DECLARE___
__CC_CLASSTYPE___::ControllerContainer(__CC_CLASSTYPE___&& o)
{
    *this = std::move(o);
}
#undef __CC_TEMPLATE_DECLARE___
#undef __CC_CLASSTYPE___
