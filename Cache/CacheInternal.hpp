#ifndef FRAME_CACHEINTERNAL_HPP
#define FRAME_CACHEINTERNAL_HPP

#include "Cache/Cache.hpp" /* include just for IDE to apply syntax highlight */

/*
 * Definitions of Cache Inner classes
 * */

template<typename KeyType, typename ValueType>
struct Cache<KeyType, ValueType>::Bucket {
    explicit Bucket(const ValueType &initialValue)
            : value(new ValueType(initialValue)), lruIterator(nullptr) {}
    explicit Bucket(std::unique_ptr<ValueType> initialValue)
            : value(std::move(initialValue)), lruIterator(nullptr) {}
    Bucket(Bucket &&m) : value(std::move(m.value)), lruIterator(std::move(m.lruIterator)) {}
    ~Bucket(){}

    inline void replaceIteratorTo(const ListIterator &newIterator){
        lruIterator.reset(new ListIterator(newIterator));
    }

    inline ValueType valueCopy() const {
        ASSERT(value != nullptr);
        return *value;
    }

    inline ValueType *valueAddress() {
        ASSERT(value != nullptr);
        return value.get();
    }

    inline ListIterator getIterator() const { return *lruIterator; }

private:
    std::unique_ptr<ListIterator> lruIterator;
    std::unique_ptr<ValueType> value;
};

template<typename KeyType, typename ValueType>
class Cache<KeyType, ValueType>::InsertFailureByKeyDuplication : public Exception {
public:
    explicit InsertFailureByKeyDuplication(const KeyType &key) : Exception(), duplicatedKey(key) {
        std::stringstream errStream;
        errStream << "Cannot insert a duplicated key<" << key << "> to the cache" <<std::endl;
        errMsg.append(errStream.str());
        errMsg.append(stackTraceToString());
    }

    ExceptionDescription what() const throw() override { return errMsg.c_str(); }

private:
    std::string errMsg;
    const KeyType duplicatedKey;
};



/*
 * Definitions of Cache methods
 * */

template<typename KeyType, typename ValueType>
boost::optional<ValueType> Cache<KeyType, ValueType>::getCopyOf(const KeyType &key) {
    auto found = elemMap.find(key);
    if(found == elemMap.end()) /* cache miss */
        return boost::none;

    Bucket &bucket(found->second);
    ListIterator lruIter = bucket.getIterator();

    if(lruIter == lruList.begin()) /* the item is already the most recently used one. */
        return bucket.valueCopy();

    lruList.erase(lruIter);
    setToTheMostRecent(bucket, key); /* set the item to be the most recently used. */

    return bucket.valueCopy();
}

template<typename KeyType, typename ValueType>
ValueType *Cache<KeyType, ValueType>::getAddressOf(const KeyType &key) {
    auto found = elemMap.find(key);
    if(found == elemMap.end()) /* cache miss */
        return nullptr;

    Bucket &bucket(found->second);
    ListIterator lruIter = bucket.getIterator();

    if(lruIter == lruList.begin()) /* the item is already the most recently used one. */
        return bucket.valueAddress();

    lruList.erase(lruIter);
    setToTheMostRecent(bucket, key); /* set the item to be the most recently used. */

    return bucket.valueAddress();
}

template<typename KeyType, typename ValueType>
void Cache<KeyType, ValueType>::insert(const KeyType &key, const ValueType &newValue) {
    if(contains(key))
        throw InsertFailureByKeyDuplication(key);

    if(howManyCached() >= currentCapacity())
        evictOneElement();

    auto insertResult = elemMap.insert(std::make_pair(key, Bucket(newValue)));
    ASSERT(insertResult.second); /* ensure uniqueness of insertion */

    /* inserted elem is considered to be used just now. */
    Bucket &justAddedBucket = insertResult.first->second;
    setToTheMostRecent(justAddedBucket, key);
}

template<typename KeyType, typename ValueType>
void Cache<KeyType, ValueType>::insert(const KeyType &key, std::unique_ptr<ValueType> value) {
    if(contains(key))
        throw InsertFailureByKeyDuplication(key);

    if(howManyCached() >= currentCapacity())
        evictOneElement();

    auto insertResult = elemMap.insert(std::make_pair(key, Bucket(std::move(value))));
    ASSERT(insertResult.second); /* ensure uniqueness of insertion */

    /* inserted elem is considered to be used just now. */
    Bucket &justAddedBucket = insertResult.first->second;
    setToTheMostRecent(justAddedBucket, key);
}

/* cannot be a 'const' method because of Bucket::valueAddress call. */
template<typename KeyType, typename ValueType>
void Cache<KeyType, ValueType>::printDebugInfo(std::ostream &where) {
    where << "Dump debug info of cache (size: "<<howManyCached()<<", capacity: "<<currentCapacity()<<"){";
    for(auto &eachElem : elemMap) {
        const KeyType &key(eachElem.first);
        Bucket &bucket(eachElem.second);

        where << "(" << key << ": " << bucket.valueCopy() <<"(" << bucket.valueAddress() << ")" << ") ";
    }
    where << "} "<<std::endl;

    where << "Dump debug info of LRU (key) list < Trendy ";
    for(auto &eachKey: lruList) {
        where << "(" << eachKey << ") ";
    }
    where << " Unpopular >"<<std::endl;
}

template<typename KeyType, typename ValueType>
void Cache<KeyType, ValueType>::printBuckets(std::ostream &where) {
//    where << "Dump bucket image (bucket_count: "<<elemMap.bucket_count() <<", capacity: "<<currentCapacity()<<"){";
//    where << std::endl;
//    for ( unsigned i = 0; i < elemMap.bucket_count(); ++i) {
//        where << "bucket #" << i << " contains:";
//        for ( auto eachBuc = elemMap.begin(i); eachBuc!= elemMap.end(i); ++eachBuc ) {
//            const KeyType &key(eachBuc->first);
//            Bucket &bucket(eachBuc->second);
//
//            where << "(" << key << ": " << bucket.valueCopy() <<"(" << bucket.valueAddress() << ")" << ") ";
//        }
//        where << std::endl;
//    }
//    where << "} "<<std::endl;
}

template<typename KeyType, typename ValueType>
void Cache<KeyType, ValueType>::evictOneElement() {
    /* evict item from the end of most recently used list */
    ListIterator i = std::prev(lruList.end());
    ASSERT(elemMap.erase(*i) == 1);
    lruList.erase(i);
}

template<typename KeyType, typename ValueType>
void Cache<KeyType, ValueType>::setToTheMostRecent(Cache::Bucket &whichBucket, const KeyType &bucketKey) {
    lruList.push_front(bucketKey);
    ListIterator mostRecentlyUsed = lruList.begin();
    whichBucket.replaceIteratorTo(mostRecentlyUsed);
}

#endif //FRAME_CACHEINTERNAL_HPP
