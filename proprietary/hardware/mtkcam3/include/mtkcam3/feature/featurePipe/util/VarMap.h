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

#define USE_NEW_VAR_MAP
#ifndef USE_NEW_VAR_MAP

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

class VarMap
{
private:
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

#else // USE_NEW_VAR_MAP

///////////////////////////////////////////////////////////////////
#include <unordered_map>
#include <mutex>

#define DECLARE_VAR_MAP_INTERFACE(map, KEY_T, setF, getF, tryGetF, clearF)  \
  template <typename T>                                                     \
  void setF(KEY_T key, const T &var)                                        \
  {                                                                         \
    map.set<T>(key, var);                                                   \
  }                                                                         \
  template <typename T>                                                     \
  T getF(KEY_T key, const T &var) const                                     \
  {                                                                         \
    return map.get<T>(key, var);                                            \
  }                                                                         \
  template <typename T>                                                     \
  bool tryGetF(KEY_T key, T &var) const                                     \
  {                                                                         \
    return map.tryGet<T>(key, var);                                         \
  }                                                                         \
  template <typename T>                                                     \
  void clearF(KEY_T key)                                                    \
  {                                                                         \
    map.clear<T>(key);                                                      \
  }

namespace NSCam {
namespace NSCamFeature {

template <typename KEY_T>
class VarMap
{
//////// declaration ////////
public:
  VarMap();
  VarMap(const VarMap &src);
  VarMap& operator=(const VarMap &src);
  template <typename VAR_T> void set(KEY_T key, const VAR_T &var);
  template <typename VAR_T> bool tryGet(KEY_T key, VAR_T &var) const;
  template <typename VAR_T> VAR_T get(KEY_T key, VAR_T var) const;
  template <typename VAR_T> void clear(KEY_T key);

private:
  class VarBase;
  class VarHash;
  typedef const char* TYPE_KEY_T;
  typedef std::pair<TYPE_KEY_T, KEY_T> MAP_KEY_T;
  typedef std::unordered_map<MAP_KEY_T, std::shared_ptr<VarBase>, VarHash> CONTAINER_T;

private:
  template <typename VAR_T> static MAP_KEY_T toMapKey(KEY_T key);

private:
  mutable std::mutex mMutex;
  CONTAINER_T mMap;

private:
  class VarBase
  {
  public:
    VarBase() {}
    virtual ~VarBase() {}
    virtual void* getPtr() const = 0;
  };

  template<typename VAR_T>
  class VarHolder : public VarBase
  {
  public:
    VarHolder(const VAR_T &var) : mVar(var) {}
    virtual ~VarHolder() {}
    virtual void* getPtr() const { return (void*)&mVar; }
  private:
    VAR_T mVar;
  };

  class VarHash
  {
  public:
    std::size_t operator() (const MAP_KEY_T &mapKey) const;
  };

};

#define DEBUG_LOG(fmt, arg...)

template <typename KEY_T>
VarMap<KEY_T>::VarMap()
{
}

template <typename KEY_T>
VarMap<KEY_T>::VarMap(const VarMap &src)
{
  std::lock_guard<std::mutex> lock(src.mMutex);
  mMap = src.mMap;
}

template <typename KEY_T>
VarMap<KEY_T>& VarMap<KEY_T>::operator=(const VarMap &src)
{
  // lock in strict order to avoid deadlock
  // check to avoid self assignment
  if( this < &src )
  {
    std::lock_guard<std::mutex> lock1(mMutex);
    std::lock_guard<std::mutex> lock2(src.mMutex);
    mMap = src.mMap;
  }
  else if( this > &src )
  {
    std::lock_guard<std::mutex> lock1(src.mMutex);
    std::lock_guard<std::mutex> lock2(mMutex);
    mMap = src.mMap;
  }
  return *this;
}

template <typename KEY_T>
template <typename VAR_T>
void VarMap<KEY_T>::set(KEY_T key, const VAR_T &var)
{
  std::lock_guard<std::mutex> lock(mMutex);
  MAP_KEY_T index = toMapKey<VAR_T>(key);
  mMap[index] = std::make_shared<VarHolder<VAR_T>>(var);
}

template <typename KEY_T>
template <typename VAR_T>
bool VarMap<KEY_T>::tryGet(KEY_T key, VAR_T &var) const
{
  bool ret = false;
  std::lock_guard<std::mutex> lock(mMutex);
  MAP_KEY_T index = toMapKey<VAR_T>(key);
  typename CONTAINER_T::const_iterator it = mMap.find(index);
  VAR_T *holder = NULL;
  if( it == mMap.end() )
  {
    DEBUG_LOG(__PRETTY_FUNCTION__ ": cannot find entry [%d]", key);
  }
  else if( it->second == NULL )
  {
    DEBUG_LOG(__PRETTY_FUNCTION__ ": invalid entry [%d]", key);
  }
  else if( (holder = static_cast<VAR_T*>(it->second->getPtr())) == NULL )
  {
    DEBUG_LOG(__PRETTY_FUNCTION__ ": invalid holder [%d]", key);
  }
  else
  {
    var = *holder;
    ret = true;
  }
  return ret;
}

template <typename KEY_T>
template <typename VAR_T>
VAR_T VarMap<KEY_T>::get(KEY_T key, VAR_T var) const
{
  tryGet<VAR_T>(key, var);
  return var;
}

template <typename KEY_T>
template <typename VAR_T>
void VarMap<KEY_T>::clear(KEY_T key)
{
  std::lock_guard<std::mutex> lock(mMutex);
  MAP_KEY_T index = toMapKey<VAR_T>(key);
  typename CONTAINER_T::iterator it = mMap.find(index);
  if( it != mMap.end() )
  {
    it->second = NULL;
    mMap.erase(it);
  }
}

template <typename KEY_T>
template <typename VAR_T>
typename VarMap<KEY_T>::MAP_KEY_T VarMap<KEY_T>::toMapKey(KEY_T key)
{
  static const char *sName = __PRETTY_FUNCTION__;
  return std::pair<TYPE_KEY_T, KEY_T>(sName, key);
}

template <typename KEY_T>
std::size_t VarMap<KEY_T>::VarHash::operator() (const MAP_KEY_T &mapKey) const
{
  typedef typename std::underlying_type<KEY_T>::type eType;
  std::size_t ret = std::hash<TYPE_KEY_T>()(mapKey.first)/2 +
                    std::hash<eType>()(static_cast<eType>(mapKey.second))/2;
  return ret;
}

} // NSCamFeature
} // NSCam

#endif // USE_NEW_VAR_MAP

#endif // _MTK_CAMERA_FEATURE_PIPE_UTIL_VAR_MAP_H_
