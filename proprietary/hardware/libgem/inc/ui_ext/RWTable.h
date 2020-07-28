#ifndef ANDROID_MTK_RWTABLE_H
#define ANDROID_MTK_RWTABLE_H
#include <utils/Singleton.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/RWLock.h>

namespace android {
template <typename KeyType, typename ValueType>
class RWTable {
public:
    RWTable(const ValueType& defValue = ValueType()) :
        mMap(defValue) {}
    virtual ~RWTable() {}

    ValueType& operator[] (const KeyType& key) {
        {
            RWLock::AutoRLock l(mLock);
            ssize_t idx = mMap.indexOfKey(key);
            if (idx >= 0) {
                return mMap.editValueAt(idx);
            }
        }
        {
            RWLock::AutoWLock l(mLock);
            static ValueType empty = ValueType();
            mMap.add(key, empty);
            return mMap.editValueFor(key);
        }
    }

    ValueType& valueAt(const size_t& idx) {
        RWLock::AutoRLock l(mLock);
        if (idx >= mMap.size()) {
            // FIXME: Do something
        }
        return mMap.editValueAt(idx);
    }

    bool remove(const KeyType& key) {
        RWLock::AutoWLock l(mLock);
        return mMap.removeItem(key) >= 0;
    }

    size_t size() const {
        RWLock::AutoRLock l(mLock);
        return mMap.size();
    }

protected:
    DefaultKeyedVector<KeyType, ValueType> mMap;
    mutable RWLock mLock;
};

}; // namespace android
#endif // ANDROID_MTK_RWTABLE_H
