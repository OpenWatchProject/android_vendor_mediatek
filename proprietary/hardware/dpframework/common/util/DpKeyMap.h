#ifndef __DP_KEY_MAP__
#define __DP_KEY_MAP__

//-----------------------------------------------------------------------------
// Key Map
//-----------------------------------------------------------------------------
template<typename T>
class DpKeyMap
{
public:
    DpKeyMap(int32_t key, T* ptr)
        : mKey(key),
          mPtr(ptr)
    {
    }

    ~KeyMap()
    {
    }

    inline T* getPtr()
    {
        return mPtr;
    }

    inline int getKey()
    {
        return mKey;
    }

    inline void setKey(int key)
    {
        mKey = key;
    }

    inline void setPtr(T* ptr)
    {
        mPtr = ptr;
    }

    inline bool operator== (const KeyMap& rhs)
    {
        return this->mKey == rhs.mKey;
    }

private:
    int32_t mKey;
    T*      mPtr;
};

#endif  // __DP_KEY_MAP__
