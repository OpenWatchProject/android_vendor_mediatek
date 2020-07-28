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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef _MTK_CAMERA_FEATURE_PIPE_UTIL_VAR_MAP_H_
#define _MTK_CAMERA_FEATURE_PIPE_UTIL_VAR_MAP_H_

#include <utils/Mutex.h>
#include <utils/RefBase.h>

#include <map>
#include <string>

#define DECLARE_VAR_MAP_INTERFACE(mapName, setF, getF, tryGetF, clearF) \
  template <typename T>                                                 \
  bool setF(const char *name, const T &var)                             \
  {                                                                     \
    return mapName.set<T>(name, var);                                   \
  }                                                                     \
  template <typename T>                                                 \
  T getF(const char *name, const T &var) const                          \
  {                                                                     \
    return mapName.get<T>(name, var);                                   \
  }                                                                     \
  template <typename T>                                                 \
  bool tryGetF(const char *name, T &var) const                          \
  {                                                                     \
    return mapName.tryGet<T>(name, var);                                \
  }                                                                     \
  template <typename T>                                                 \
  void clearF(const char *name)                                         \
  {                                                                     \
    return mapName.clear<T>(name);                                      \
  }

namespace NSCam {
namespace NSCamFeature {

template <typename T>
const char* getTypeNameID()
{
  const char *name = __PRETTY_FUNCTION__;
  return name;
}

class VarHolderBase : public virtual android::RefBase
{
public:
  VarHolderBase()
  {
  }

  virtual ~VarHolderBase()
  {
  }

  virtual void* getPtr() const = 0;
};

template <typename T>
class VarHolder : public VarHolderBase
{
public:
  VarHolder(const T &var)
    : mVar(var)
  {
  }

  virtual ~VarHolder()
  {
  }

  virtual void* getPtr() const
  {
    return (void*)&mVar;
  }

private:
  T mVar;
};

class VarMap
{
public:
  VarMap()
  {
  }

  VarMap(const VarMap &src)
  {
    android::Mutex::Autolock lock(src.mMutex);
    mMap = src.mMap;
  }

  VarMap& operator=(const VarMap &src)
  {
    // lock in strict order to avoid deadlock
    // check to avoid self assignment
    if( this < &src )
    {
      android::Mutex::Autolock lock1(mMutex);
      android::Mutex::Autolock lock2(src.mMutex);
      mMap = src.mMap;
    }
    else if( this > &src )
    {
      android::Mutex::Autolock lock1(src.mMutex);
      android::Mutex::Autolock lock2(mMutex);
      mMap = src.mMap;
    }
    return *this;
  }

  template <typename T>
  bool set(const char *name, const T &var)
  {
    android::Mutex::Autolock lock(mMutex);
    bool ret = false;

    if( !name )
    {
      // LOGE("[VarMap::set] Invalid var name");
    }
    else
    {
      std::string id;
      android::sp<VarHolderBase> holder;
      id = std::string(getTypeNameID<T>()) + std::string(name);
      holder = new VarHolder<T>(var);

      if( holder == NULL )
      {
        // LOGE("[VarMap::set][OOM] Failed to allocate VarHolder");
      }
      else
      {
        mMap[id] = holder;
        ret = true;
      }
    }
    return ret;
  }

  template <typename T>
  T get(const char *name, T var) const
  {
    tryGet(name, var);
    return var;
  }

  template <typename T>
  bool tryGet(const char *name, T &var) const
  {
    android::Mutex::Autolock lock(mMutex);
    bool ret = false;
    T *holder = NULL;

    if( !name )
    {
      // LOGE("[VarMap::get] Invalid var name");
    }
    else
    {
      std::string id;
      id = std::string(getTypeNameID<T>()) + std::string(name);
      CONTAINER::const_iterator it;
      it = mMap.find(id);
      if( it == mMap.end() )
      {
        // LOGD("[VarMap::get] Cannot find var %s", id.c_str());
      }
      else if( it->second == NULL)
      {
        // LOGE("[VarMap::get] Invalid holder");
      }
      else if( (holder = static_cast<T*>(it->second->getPtr())) == NULL )
      {
        // LOGE("[VarMap::get] Invalid holder value");
      }
      else
      {
        var = *holder;
        ret = true;
      }
    }
    return ret;
  }

  template <typename T>
  void clear(const char *name)
  {
    android::Mutex::Autolock lock(mMutex);

    if( !name )
    {
      // LOGE("[VarMap::clear] Invalid var name");
    }
    else
    {
      std::string id;
      android::sp<VarHolderBase> holder;
      id = std::string(getTypeNameID<T>()) + std::string(name);
      CONTAINER::iterator it;
      it = mMap.find(id);
      if( it != mMap.end() )
      {
        it->second = NULL;
        mMap.erase(it);
      }
    }
  }

private:
  typedef std::map<std::string, android::sp<VarHolderBase> > CONTAINER;
  CONTAINER mMap;
  mutable android::Mutex mMutex;
};

} // NSCamFeature
} // NSCam

#endif // _MTK_CAMERA_FEATURE_PIPE_UTIL_VAR_MAP_H_
