#ifndef FRAME_CACHE_HPP
#define FRAME_CACHE_HPP

#include "MiscUtil/ErrorHandling.hpp"

#include <boost/interprocess/containers/vector.hpp>

/* TODO: add template parameter Hash and Pred when people need it. (use std::hash<KeyType>, std::equal_to<KeyType> for now)*/
template <typename KeyType, typename ValueType>
class Cache {
public:
    using HashValueType = typename std::hash<KeyType>::result_type; /* this is equal to 'size_t' */
    static_assert(sizeof(HashValueType) == 8, "size of HashValueType should be 8 byte");
    static_assert( std::is_same<size_t, HashValueType>::value, "" );

    using Hash = std::hash<KeyType>; /* developer should give implementation for std::hash<KeyType> */
    using Pred = std::equal_to<KeyType>; /* for comparing elem in same bucket */

    class InsertFailureByKeyDuplication;

    explicit Cache(uint32_t initialCapacity) : elemMap(), capacity(initialCapacity) {}
    virtual ~Cache() {}

    inline std::size_t howManyCached() const { return elemMap.size(); }
    inline bool empty() const { return elemMap.empty(); }
    inline bool contains(const KeyType &key) const { return elemMap.find(key) != elemMap.end(); }
    inline uint32_t currentCapacity() const { return capacity; }

    /* getters */
    boost::optional<ValueType> getCopyOf(const KeyType &key);
    /* WARNING: returned address can be deallocated by evict!!
     * TODO: fixme0 implement fin & unpin (by using atomic int and remove from lru list) */
    ValueType *getAddressOf(const KeyType &key); /* returns nullptr if cache miss. */ /* TODO: would consider return type boost::optional<ValueType&> ? */

    /* putters */
    void insert(const KeyType &key, const ValueType &newValue);
    void insert(const KeyType &key, std::unique_ptr<ValueType> value);

    /* for testing purpose only */
    void printDebugInfo(std::ostream &where);
    void printBuckets(std::ostream &where);

private:
    typedef std::list<KeyType> LRU_List;
    typedef typename LRU_List::iterator ListIterator;
    struct Bucket;

    /*typedef boost::interprocess::vector<Bucket, std::allocator<Bucket> > MyVector;*/

    void evictOneElement();
    void setToTheMostRecent(Bucket &whichBucket, const KeyType &bucketKey);

    std::unordered_map<KeyType, Bucket> elemMap;
    LRU_List lruList; /* most recently used one is moved forward to front, least recently used elem goes back. */
    uint32_t capacity;
};

#include "Cache/CacheInternal.hpp"

#endif //FRAME_CACHE_HPP
