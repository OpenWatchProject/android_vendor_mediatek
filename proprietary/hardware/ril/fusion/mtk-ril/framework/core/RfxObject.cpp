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
 * File name:  rfx_object.cpp
 * Author: Jun Liu (MTK80064)
 * Description:
 * Define RfxObject, the base class of Venus UI Framework.
 */


/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxObject.h"
#include "RfxLog.h"
#include "utils/String8.h"

using ::android::String8;
/*****************************************************************************
 * Class RfxObject
 *****************************************************************************/

#define RFX_LOG_TAG "RfxObject"

const rfx_class_info_struct RfxObject::s_class_info = {RFX_OBJECT_CLASS_NAME, NULL, NULL};


RfxClassInfo *RfxObject::getRfxObjectClassInfo()
{
    return (RfxClassInfo *)&s_class_info;
}

int RfxObject::static_isObjCheck(void* ptr, void** parent_p)
{
    RfxObject *obj = (RfxObject *)ptr;

    if (obj->m_guard == RFX_OBJ_CREATED_GUARD_PATTERN)
    {
        *parent_p = (void*) obj->m_parent_obj;
        return 1;
    }
    else
    {
        return 0;
    }

}

RfxObject::RfxObject() :
    m_guard(RFX_OBJ_CREATED_GUARD_PATTERN),
    m_obj_flags(0),
    m_parent_obj(NULL),
    m_first_child_obj(NULL),
    m_last_child_obj(NULL),
    m_next_obj(NULL),
    m_prev_obj(NULL) {
#ifdef RFX_OBJ_DEBUG
    if (RfxDebugInfo::isRfxDebugInfoEnabled()) {
        m_debug_info = new RfxDebugInfo(static_cast<IRfxDebugLogger *>(this), this);
    } else {
        m_debug_info = NULL;
    }
#endif
}


RfxObject::~RfxObject()
{
    RFX_ASSERT(m_guard == RFX_OBJ_CREATED_GUARD_PATTERN);

    // NOTE: Must use RFX_CLOSE() to destroy an object or it will cause this assert
    RFX_ASSERT(m_parent_obj == NULL);
#ifdef RFX_OBJ_DEBUG
    if (m_debug_info) {
        delete(m_debug_info);
    }
#endif
    // Update the guard patten
    m_guard = RFX_OBJ_DESTROYED_GUARD_PATTERN;
}


void RfxObject::_init(RfxObject *objParent)
{
    // set flags to init
    RFX_FLAG_SET(m_obj_flags, FLAGS_OBJ_INITING);

    mHoldSelf = this;

#ifdef RFX_OBJ_DEBUG
    if (m_debug_info != NULL && objParent == NULL) {
        if (s_root_obj_debug_info == NULL) {
            s_root_obj_debug_info = new Vector<RfxDebugInfo*>();
        }
        RFX_LOG_D(RFX_LOG_TAG, "%p, %s is added into debug info list", this, getClassInfo()->getClassName());
        s_root_obj_debug_info->add(m_debug_info);
    }
#endif //#ifdef RFX_OBJ_DEBUG

    // Assign the parent object
    setParent(objParent);

    // Callback onInit()
    onInit();
//    onAfterInit();
    onObjectNotify(RFX_OBJECT_NOTIFY_ID_AFTER_INIT, NULL);

    // clear flags
    RFX_FLAG_CLEAR(m_obj_flags, FLAGS_OBJ_INITING);
}


void RfxObject::_deinit()
{
    // NOTE: Assert here if the object has been closed
    RFX_ASSERT(m_guard == RFX_OBJ_CREATED_GUARD_PATTERN);

    // NOTE: Assert here if the object enter _deinit twice
    RFX_ASSERT(!RFX_FLAG_HAS(m_obj_flags, FLAGS_OBJ_DEINITING));

    // NOTE: Assert here if the object be closed when creating it.
    RFX_ASSERT(!RFX_FLAG_HAS(m_obj_flags, FLAGS_OBJ_INITING));

    // set flags to deinit
    RFX_FLAG_SET(m_obj_flags, FLAGS_OBJ_DEINITING);

#ifdef RFX_OBJ_DEBUG
    if (m_debug_info != NULL && m_parent_obj == NULL) {
        size_t size = s_root_obj_debug_info->size();
        for (size_t i = 0; i < size; i++) {
            const RfxDebugInfo *item = s_root_obj_debug_info->itemAt(i);
            if (m_debug_info == item) {
                RFX_LOG_D(RFX_LOG_TAG, "%p, %s is removed from debug info list", this, getClassInfo()->getClassName());
                s_root_obj_debug_info->removeAt(i);
                break;
            }
        }
    }
#endif //#ifdef RFX_OBJ_DEBUG

    // Callback onDeinit()
//    onBeforeDeinit();
    onObjectNotify(RFX_OBJECT_NOTIFY_ID_BEFORE_DEINIT, NULL);
    onDeinit();

    // Close all child objects
    releaseAllChild();

    // Callback onAfterDeinit()
//    onAfterDeinit();
    onObjectNotify(RFX_OBJECT_NOTIFY_ID_AFTER_DEINIT, NULL);

    // Remove the relation to parent object
    removeFromParent();
}


void RfxObject::_close()
{
    _deinit();
    /* M: This is a workaround for SP<>::cleare reentry error;
     * When call sp<>::clear(), it will follow the following way:
     * sp<>::clear()->RefBase::decStrong()->RfxObject::~RfxObject()->
     * (mHoldSelf)sp<>::~sp()->RefBase::decStrong()
     * It will cause the mStrong of mHoldSelf would be extra reduced by once.
     */
    sp<RfxObject> temp = mHoldSelf;

    mHoldSelf.clear();
}


void RfxObject::setParent(RfxObject *obj_parent)
{
    if (obj_parent == NULL)
    {
        removeFromParent();
        return;
    }

    obj_parent->addChild(this);
}


// TODO: move all this implement to onRemoveChild()
void RfxObject::removeFromParent()
{
    if (m_parent_obj == NULL)
    {
        // Do nothing
        return;
    }

    // Callback the virtual function
//    m_parent_obj->onRemoveChild(this);
    m_parent_obj->onObjectNotify(RFX_OBJECT_NOTIFY_ID_REMOVE_CHILD, (void *)this);

    // Remove from child list of parent
    if (m_prev_obj != NULL)
    {
        m_prev_obj->m_next_obj = m_next_obj;
    }
    else
    {
        RFX_ASSERT(m_parent_obj->m_first_child_obj == this);
        m_parent_obj->m_first_child_obj = m_next_obj;
    }

    if (m_next_obj != NULL)
    {
        m_next_obj->m_prev_obj = m_prev_obj;
    }
    else
    {
        RFX_ASSERT(m_parent_obj->m_last_child_obj == this);
        m_parent_obj->m_last_child_obj = m_prev_obj;
    }

    m_parent_obj = NULL;
    m_prev_obj = NULL;
    m_next_obj = NULL;
}


// TODO: move all this implement to onAddChild()
void RfxObject::addChild(RfxObject *child_obj)
{
    RFX_OBJ_ASSERT_VALID(child_obj);
    RFX_ASSERT(child_obj != this);

    if (child_obj->m_parent_obj == this)
    {
        // Do nothing
        return;
    }

    // Add reference and make sure the input newFrame has no relation
    child_obj->removeFromParent();

    RFX_ASSERT(child_obj->m_parent_obj == NULL);
    RFX_ASSERT(child_obj->m_prev_obj == NULL);
    RFX_ASSERT(child_obj->m_next_obj == NULL);

    // Assign parent object
    child_obj->m_parent_obj = this;

    // Append to tail of list
    child_obj->m_prev_obj = m_last_child_obj;
    child_obj->m_next_obj = NULL;

    if (m_first_child_obj == NULL)
    {
        m_first_child_obj = child_obj;
    }
    if (m_last_child_obj != NULL)
    {
        m_last_child_obj->m_next_obj = child_obj;
    }
    m_last_child_obj = child_obj;

    // Callback the virtual function
//    onAddChild(childObj);
    onObjectNotify(RFX_OBJECT_NOTIFY_ID_ADD_CHILD, (void *)child_obj);
}


int RfxObject::getChildCount() const
{
    int count = 0;
    RfxObject *i;
    for (i = m_first_child_obj; i != NULL; i = i->m_next_obj)
    {
        count++;
    }
    return count;
}


void RfxObject::releaseAllChild()
{
    // Because the child will remove itself from the child object list,
    // So we only need to see if the list is empty
    while (m_last_child_obj != NULL)
    {
        RfxObject *obj = m_last_child_obj;
        RFX_OBJ_CLOSE(obj);
    }

    // The list must be empty
    RFX_ASSERT(m_last_child_obj == NULL);
}


RfxClassInfo *RfxObject::onGetClassInfo() const
{
    return getRfxObjectClassInfo();
}


void RfxObject::onInit()
{
    // do nothing
}


void RfxObject::onDeinit()
{
    // do nothing
}


void RfxObject::onObjectNotify(RfxObjectNotifyIdEnum id, void *userData)
{
    RFX_UNUSED(id);
    RFX_UNUSED(userData);
    // do nothing
}


void RfxObject::onAfterInit()
{
    // do nothing
}


void RfxObject::onBeforeDeinit()
{
    // do nothing
}


void RfxObject::onAfterDeinit()
{
    // do nothing
}


void RfxObject::onAddChild(RfxObject *child)
{
    RFX_UNUSED(child);
    // do nothing
}


void RfxObject::onRemoveChild(RfxObject *child)
{
    RFX_UNUSED(child);
    // do nothing
}

RfxObject *RfxObject::findObject(RfxObject *parent, const RfxClassInfo *class_info)
{
    if (parent == NULL) {
        return NULL;
    }

    if (parent->isKindOf(class_info)) {
        return parent;
    }

    for (RfxObject *i = parent->getFirstChildObj(); i != NULL; i = i->getNextObj()) {

        RfxObject *ret = findObject(i, class_info);

        if (ret != NULL) {
            return ret;
        }
    }

    return NULL;
}


void rfxObjClose(RfxObject *ptr)
{
    if (ptr != NULL)
    {
        ptr->_close();
    }
}

#ifdef RFX_OBJ_DEBUG

Vector<RfxDebugInfo*> *RfxObject::s_root_obj_debug_info = NULL;

void RfxObject::dump(int level) const {
    String8 level_string("");
    for (int i = 0; i < level; i++) {
        level_string.append("    ");
    }
    RFX_LOG_D(RFX_DEBUG_INFO_TAG, "%s%p, %s", level_string.string(), this, getClassInfo()->getClassName());

    for (RfxObject *i = getFirstChildObj(); i != NULL; i = i->getNextObj()) {
        i->dump(level + 1);
    }
}

void RfxObject::dumpAllObjTree() {
    size_t size = RfxObject::s_root_obj_debug_info->size();
    RFX_LOG_D(RFX_DEBUG_INFO_TAG, "dumpAllObjTree() Object count is %zu", size);
    for (size_t i = 0; i < size; i++) {
        const RfxDebugInfo *item = RfxObject::s_root_obj_debug_info->itemAt(i);
        item->getLogger()->dump();
    }
}

void RfxObject::dumpObjConstructionCallStack(void *obj_address) {
    size_t size = RfxObject::s_root_obj_debug_info->size();
    for (size_t i = 0; i < size; i++) {
        const RfxDebugInfo *item = RfxObject::s_root_obj_debug_info->itemAt(i);
        const RfxObject *obj = RfxObject::matchObj((const RfxObject *)item->getUserData(), obj_address);
        if (obj) {
            RFX_LOG_D(RFX_DEBUG_INFO_TAG, "dumpObjConstructionCallStack() Found obj, start to dump callstack");
            obj->m_debug_info->dump();
            return;
        }
    }
    RFX_LOG_D(RFX_DEBUG_INFO_TAG, "dumpObjConstructionCallStack() Can't find object %p", obj_address);
}

const RfxObject *RfxObject::matchObj(const RfxObject *obj, const void *obj_address) {
    if (obj == obj_address) {
        return obj;
    } else {
        for (const RfxObject *i = obj->getFirstChildObj(); i != NULL; i = i->getNextObj()) {
            const RfxObject *ret = RfxObject::matchObj(i, obj_address);
            if (ret) {
                return ret;
            }
        }
        return NULL;
    }
}
#endif //#ifdef RFX_OBJ_DEBUG