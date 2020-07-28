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
 * File name:  RfxObject.h
 * Author: Jun Liu (MTK80064)
 * Description:
 * Header of RfxObject which is base object class of RIL proxy framework
 */

#ifndef __RFX_OBJECT_H__
#define __RFX_OBJECT_H__

/***************************************************************************** 
 * Include
 *****************************************************************************/

#include <utils/RefBase.h>
#include <utils/Vector.h>

#include "RfxBasics.h"
#include "RfxClassInfo.h"
#include "RfxDebugInfo.h"
#include "RfxDefs.h"

using ::android::sp;
using ::android::wp;
using ::android::RefBase;
using ::android::Vector;

/***************************************************************************** 
 * Define
 *****************************************************************************/

// The RfxObject registered name
#define RFX_OBJECT_CLASS_NAME           "Object"


// Internal used. RfxObject guard pattern for a created object.
#define RFX_OBJ_CREATED_GUARD_PATTERN   0xBABEC0DE
// Internal used. RfxObject guard pattern for a destroyed object.
#define RFX_OBJ_DESTROYED_GUARD_PATTERN 0xDEADBEEF

class RfxObject;

/*
 * Macro to check an object is valid. Assert if the pointer is NULL or 
 *  the pointer to the object is invalid.
 *
 * EXAMPLE:
 * <code>
 *  void MyObject::test(MyObject *other)
 *  {
 *      // Assert if the other object is NULL or invalid.
 *      RFX_OBJ_ASSERT_VALID(other);
 *  }
 * </code>
 */
#define RFX_OBJ_ASSERT_VALID(_ptr)       RFX_ASSERT(_ptr != NULL && (_ptr)->isValid())


/*
 * Macro to get the class information ID with a class name.
 *
 * NOTE:
 *  If the _className has not use macro RFX_DECLARE_CLASS, it will cause
 *  compile error.
 *
 * SEE ALSO: RFX_DECLARE_CLASS, RFX_IMPLEMENT_CLASS
 */
#define RFX_OBJ_CLASS_INFO(_class_name)  (_class_name::get##_class_name##ClassInfo())


/*
 * Macro to declare class information.
 */
#define RFX_DECLARE_CLASS(_class_name)                                           \
    public:                                                                     \
        static const rfx_class_info_struct s_class_info;                         \
                                                                                \
    public:                                                                     \
        static RfxObject *createInstance(RfxObject *parent);                    \
        static RfxClassInfo *get##_class_name##ClassInfo();                      \
        static RfxClassInfo *getClassInfoInstance()                             \
        {                                                                       \
            return get##_class_name##ClassInfo();                                \
        }                                                                       \
                                                                                \
    protected:                                                                  \
        virtual RfxClassInfo *onGetClassInfo() const

/*
 * Macro to implement class information.
 *
 * SEE ALSO: RFX_DECLARE_CLASS
 */
#define RFX_IMPLEMENT_CLASS(_register_name, _class_name, _parent_class_name)        \
                                                                                \
    RfxObject *_class_name::createInstance(RfxObject *parent)                    \
    {                                                                           \
        _class_name *ret;                                                        \
        __RFX_OBJ_CREATE(ret, _class_name, parent);                              \
        return ret;                                                             \
    }                                                                           \
                                                                                \
    RfxClassInfo *_class_name::get##_class_name##ClassInfo()                      \
    {                                                                           \
        return (RfxClassInfo *)&s_class_info;                                     \
    }                                                                           \
                                                                                \
    RfxClassInfo *_class_name::onGetClassInfo() const                            \
    {                                                                           \
        return get##_class_name##ClassInfo();                                    \
    }                                                                           \
                                                                                \
    const rfx_class_info_struct _class_name::s_class_info = {_register_name, &_parent_class_name::s_class_info, &_class_name::createInstance};


/*
 * Macro to implement class information.
 *
 * SEE ALSO: RFX_DECLARE_CLASS
 */
#define RFX_IMPLEMENT_VIRTUAL_CLASS(_register_name, _class_name, _parent_class_name) \
                                                                                \
    RfxClassInfo *_class_name::get##_class_name##ClassInfo()                      \
    {                                                                           \
        return (RfxClassInfo *)&s_class_info;                                     \
    }                                                                           \
                                                                                \
    RfxClassInfo *_class_name::onGetClassInfo() const                            \
    {                                                                           \
        return get##_class_name##ClassInfo();                                    \
    }                                                                           \
                                                                                \
    const rfx_class_info_struct _class_name::s_class_info = {_register_name, &_parent_class_name::s_class_info, NULL};

/*
 * Dynamic cast an object type. Retrun NULL if the object can not cast to the
 * given class name.
 *
 * EXAMPLE:
 * <code>
 *  RfxMyObject *object = RFX_OBJ_DYNAMIC_CAST(control, RfxMyObject);
 *  if (object != NULL)
 *  {
 *      object->click();
 *  }
 * </code>
 */
#define RFX_OBJ_DYNAMIC_CAST(_ptr, _class_name)                                  \
    ((_ptr) ? ((_ptr)->isKindOf(RFX_OBJ_CLASS_INFO(_class_name)) ?              \
        static_cast<_class_name *>(_ptr) :                                     \
        NULL) : NULL)


/*
 * Macro to create an object. _className must inherit from RfxObject.
 *
 * SEE ALSO: RFX_OBJ_CREATE_EX, RFX_OBJ_CLOSE
 *
 * EXAMPLE:
 * <code>
 *  MyObject *obj;
 *  RFX_OBJ_CREATE(obj, MyObject, this);
 * </code>
 */
template <class _class_name>
_class_name *rfxObjCreate(RfxObject *_parent)
{
    _class_name *ptr = new _class_name();
    ptr->_init(_parent);
    return ptr;
}

#define RFX_OBJ_CREATE(_ptr, _class_name, _parent)                               \
    do                                                                          \
    {                                                                           \
        (_ptr) = rfxObjCreate<_class_name>(_parent);                             \
    } while (0)

#define __RFX_OBJ_CREATE(_ptr, _class_name, _parent)                             \
    do                                                                          \
    {                                                                           \
        (_ptr) = new _class_name();                                               \
        (_ptr)->_init(_parent);                                                 \
    } while (0)

/*
 * Macro to create an object with constructor parameters. 
 * _class_name must inherit from RfxObject.
 *
 * SEE ALSO: RFX_OBJ_CREATE, RFX_OBJ_CLOSE
 *
 * EXAMPLE:
 * <code>
 *  MyObject *obj;
 *  RFX_OBJ_CREATE_EX(obj, MyObject, this, (param1, param2));
 * </code>
 */
#define RFX_OBJ_CREATE_EX(_ptr, _class_name, _parent, _param)                    \
    do                                                                          \
    {                                                                           \
        (_ptr) = new _class_name _param;                                        \
        (_ptr)->_init(_parent); \
    } while (0)


/*
 * Macro to close an object. _ptr must be a RfxObject
 *
 * SEE ALSO: RFX_OBJ_CREATE
 */
extern void rfxObjClose(RfxObject *ptr);
class RfxObject;

#define RFX_OBJ_CLOSE(_ptr)                                          \
    do                                                               \
    {                                                                \
        rfxObjClose(_ptr);                                           \
        (_ptr) = NULL;                                               \
    } while (0)


/*
 * Macro to declare a signalton class
 *
 * SEE ALSO: RFX_OBJ_IMPLEMENT_SINGLETON_CLASS
 */ 
#define RFX_OBJ_DECLARE_SINGLETON_CLASS(_class_name)                             \
    public:                                                                     \
        static _class_name *getInstance();                                       \
        static void closeInstance();                                            \
    private:                                                                    \
        static unsigned char s_instance_buf[];                                   \
        static _class_name *s_instance

/*
 * Macro to implement the signalton class.
 *
 * SEE ALSO: RFX_OBJ_DECLARE_SINGLETON_CLASS
 */
#define RFX_OBJ_IMPLEMENT_SINGLETON_CLASS(_class_name) \
    _class_name *_class_name::getInstance() \
    { \
        if (s_instance == NULL) \
        {\
            s_instance = new (s_instance_buf) _class_name(); \
            s_instance->_init(NULL); \
        } \
        return s_instance; \
    } \
    \
    void _class_name::closeInstance() \
    { \
        if (s_instance != NULL) \
        { \
            s_instance->_deinit(); \
            s_instance = NULL; \
        } \
    } \
    \
    unsigned char _class_name::s_instance_buf[sizeof(_class_name)] __attribute__((__aligned__(4))); \
    _class_name *_class_name::s_instance = NULL

/*
 * Macro to get the singalton
 */    
#define RFX_OBJ_GET_INSTANCE(_class_name) (_class_name::getInstance())

// TODO: the interface will be phace out.
/*
 * Macro to destroy the signalton
 */
#define RFX_OBJ_CLOSE_INSTANCE(_class_name) (_class_name::closeInstance())


/***************************************************************************** 
 * Typedef
 *****************************************************************************/

// Event id for onObjectNotify()
//
// SEE ALSO: RfxObject::onObjectNotify()
enum RfxObjectNotifyIdEnum {
    RFX_OBJECT_NOTIFY_ID_AFTER_INIT,
    RFX_OBJECT_NOTIFY_ID_BEFORE_DEINIT,
    RFX_OBJECT_NOTIFY_ID_AFTER_DEINIT,
    RFX_OBJECT_NOTIFY_ID_ADD_CHILD,
    RFX_OBJECT_NOTIFY_ID_REMOVE_CHILD,

    RFX_OBJECT_NOTIFY_ID_END_OF_ENUM
};

/***************************************************************************** 
 * Class RfxObject
 *****************************************************************************/

/*
 * RfxObject is the base class of all Venus UI objects
 */
class RfxObject : public virtual RefBase
                #ifdef RFX_OBJ_DEBUG
                  , public IRfxDebugLogger
                #endif
{
    RFX_DECLARE_CLASS(RfxObject);
    RFX_DECLARE_NO_COPY_CLASS(RfxObject);

// Object-tree hierarchy method
public:
    // Return the parent object of the object
    //
    // RETURNS: The parent object of the object
    RfxObject *getParent() const
    {
        return m_parent_obj;
    }

    // Caculate the number of child objects.
    //
    // RETURNS: The number of child objects.
    int getChildCount() const;

// Object-tree hierarchy method
private:
    // Internal Used.
    void setParent(
        RfxObject *objParent        // [IN] New object to be set as parent
    );

    // Internal Used.
    void removeFromParent();

    // Internal Used.
    void addChild(
        RfxObject *childObj         // [IN] The child object to be added
    );

    // Internal Used.
    void removeChild(
        RfxObject *childObj         // [IN] The child object to be removed
    );

// Framework methods
public:
    RfxObject *getSelf() {
        return this;
    }

    // Internal used. Initialize the object.
    void _init(RfxObject *obj_parent);

    // Internal used. Deinitialize the object.
    void _deinit();

    // Internal used. Deinitialize and delete this object.
    // Use macro RFX_OBJ_CLOSE directly.
    void _close();

    // Internal used. Get class info of the instance.
    // Concrete class must handle virtual function onGetClassInfo to
    // return its class info.
    //
    // RETURNS: Pointer to RfxClassInfo of the object
    //
    // SEE ALSO: RFX_OBJ_CLASS_INFO
    RfxClassInfo *getClassInfo() const {
        return onGetClassInfo();
    }

    // Check the object whatever inherit from the given class.
    //
    // RETURNS: Return true if the object inherit from the given class.
    //  Otherwise, return false.
    bool isKindOf(const RfxClassInfo *classInfo) const
    {
        return onGetClassInfo()->isKindOf(classInfo);
    }

    // Internal used.
    bool isValid() const
    {
        return m_guard == RFX_OBJ_CREATED_GUARD_PATTERN;
    }

    RfxObject *getFirstChildObj() const;

    RfxObject *getLastChildObj() const;

    RfxObject *getNextObj() const;

    RfxObject *getPrevObj() const;

    // To check if ptr is a RfxObject
    static int static_isObjCheck(void* ptr, void** parent_p);

    RfxObject *findObject(RfxObject *parent, const RfxClassInfo *class_info);

// Constructor / Destructor
public:
    // Default constructor
    RfxObject();

public:
    // Destructor
    virtual ~RfxObject();

// Overridable
protected:
    // Overridable virtual function. Called after construct the object.
    //
    // SEE ALSO: RFX_OBJ_CREATE
    virtual void onInit();

    // Overridable virtual function. Called before destruct the object.
    //
    // SEE ALSO: RFX_OBJ_CLOSE
    virtual void onDeinit();

    // Overridable virtual function. Called when there is notification about the object.
    //
    // SEE ALSO: RfxObjectNotifyIdEnum
    virtual void onObjectNotify(RfxObjectNotifyIdEnum id, void *userData);

    // Called after the object is initialized.
    //
    // SEE ALSO: RFX_OBJ_CREATE, onInit
    void onAfterInit();

    // Called before the onDeinit is invoked.
    //
    // SEE ALSO: RFX_OBJ_CLOSE, onDeinit
    void onBeforeDeinit();

    // Called after the onDeinit is invoked.
    //
    // NOTE: Remove the object relation at this stage
    //
    // SEE ALSO: RFX_OBJ_CLOSE, onDeinit
    void onAfterDeinit();

    // Called on adding a child object.
    //
    // SEE ALSO: setParent, addChild
    void onAddChild(
        RfxObject *child_obj         // [IN] The child object to be added
    );

    // Called on removing a child object.
    //
    // SEE ALSO: setParent, removeFromParent, removeChild,
    void onRemoveChild(
        RfxObject *child_obj         // [IN] The child object to be removed
    );

// Implemetation
private:
    enum {
        // If object occupy unique handle
        FLAGS_HAS_HANDLE        = 1 << 0,
        FLAGS_OBJ_INITING       = 1 << 1,
        FLAGS_OBJ_DEINITING     = 1 << 2,
        FLAGS_HAS_PROXY         = 1 << 3,
        FLAGS_IS_PROXY          = 1 << 4
    };

// Implemetation
private:
    // Guard pattern
    uint32_t m_guard;

    // Object Flags
    mutable int m_obj_flags;

    RfxObject *m_parent_obj;
    RfxObject *m_first_child_obj;
    RfxObject *m_last_child_obj;
    RfxObject *m_next_obj;
    RfxObject *m_prev_obj;
    sp<RfxObject> mHoldSelf;

    void releaseAllChild();

#ifdef RFX_OBJ_DEBUG
public:
    virtual void dump(int level = 0) const;

    static void dumpAllObjTree();

    static void dumpObjConstructionCallStack(void *obj_address);

private:

    static const RfxObject *matchObj(const RfxObject *obj, const void *obj_address);

private:
    RfxDebugInfo *m_debug_info;
    static Vector<RfxDebugInfo*> *s_root_obj_debug_info;
#endif //#ifdef RFX_OBJ_DEBUG
};

inline
RfxObject *RfxObject::getFirstChildObj() const {
    return m_first_child_obj;
}

inline
RfxObject *RfxObject::getLastChildObj() const {
    return m_last_child_obj;
}

inline
RfxObject *RfxObject::getNextObj() const {
    return m_next_obj;
}

inline
RfxObject *RfxObject::getPrevObj() const {
    return m_prev_obj;
}


inline
void RfxObject::removeChild(RfxObject *child_obj)
{
    RFX_OBJ_ASSERT_VALID(child_obj);
    RFX_ASSERT(child_obj->m_parent_obj == this);
    child_obj->removeFromParent();
}

#endif /* __RFX_OBJECT_H__ */

