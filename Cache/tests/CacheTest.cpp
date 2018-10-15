#include <gtest/gtest.h>

#include "Cache/Cache.hpp"


TEST(Cache, testVector) {
    typedef boost::interprocess::vector<int, std::allocator<int> > MyVector;
    MyVector a(30);

    a.push_back(3);
    a.push_back(77);

    std::hash<int> h;
    std::cerr << h(3) <<std::endl;
    std::cerr << h(4) <<std::endl;
    std::cerr << h(5) <<std::endl;
    std::cerr << h(6) <<std::endl;

}


TEST(Cache, initializeWtihBuckets) {
    uint32_t initialCapacity = 100;
    Cache<int, int> intCache(initialCapacity);

    for (int i = 0; i < 20; ++i) {
        intCache.insert(i, 7777);
    }

    intCache.printDebugInfo(std::cerr);
    intCache.printBuckets(std::cerr);


}


TEST(Cache, testGet) {
    uint32_t initialCapacity = 1;
    Cache<int, int> intCache(initialCapacity);

    intCache.insert(1, 7777);
    boost::optional<int> result = intCache.getCopyOf(1);
    ASSERT_TRUE(result);
    EXPECT_EQ(7777, result.get());
}

TEST(Cache, InsertByMoveAndGetByAddress) {
    uint32_t initialCapacity = 3;
    Cache<int, int> intCache(initialCapacity);
    int passByValue = 3333;

    std::unique_ptr<int> movee(new int(7777));
    int *originalAddr = movee.get();

    intCache.insert(1, passByValue);
    intCache.insert(2, std::move(movee));

    EXPECT_EQ(3333, *intCache.getAddressOf(1));
    EXPECT_EQ(7777, *intCache.getAddressOf(2));

    EXPECT_NE(&passByValue, intCache.getAddressOf(1));
    EXPECT_EQ(originalAddr, intCache.getAddressOf(2));
}

TEST(Cache, duplicatedKeyInsertFail) {
    uint32_t initialCapacity = 3;
    Cache<int, int> intCache(initialCapacity);
    using ExpectedException = Cache<int, int>::InsertFailureByKeyDuplication;

    intCache.insert(1, 777);
    EXPECT_THROW(intCache.insert(1, 444), ExpectedException);
}

TEST(Cache, testLRU) {
    uint32_t initialCapacity = 3;
    Cache<int, int> intCache(initialCapacity);
    int dummyVal = 0;

    intCache.insert(1, dummyVal);
    intCache.insert(2, dummyVal);
    intCache.insert(3, dummyVal);
    intCache.insert(4, dummyVal); /* will evict 1 */

    EXPECT_FALSE(intCache.contains(1));
    EXPECT_TRUE(intCache.getCopyOf(2)); /* 2 is the most recently used! */

    intCache.insert(5, dummyVal); /* will evict 3 */
    EXPECT_FALSE(intCache.contains(3));
}

namespace CacheTests {
    class CacheTestFixture : public ::testing::Test {
    public:
        explicit CacheTestFixture() : intCache(100) {}
        virtual ~CacheTestFixture() {}
        void SetUp() override {}
        void TearDown() override {}

        Cache<int, int> intCache;
        std::mutex m;
    };
}
using namespace CacheTests;

TEST_F(CacheTestFixture, insertConcurrently) {
    std::vector<std::future<int>> threads(100);

    for (int i = 0; i < threads.size(); ++i) {
        threads[i] = std::async(std::launch::async, [i, this](){
            int howManyInsertion = 10;
            for (int j = 0; j < howManyInsertion; ++j) {
                std::lock_guard<std::mutex> guard(m);
                intCache.insert(i * 10 + j, i * i);
            }

            return howManyInsertion;
        });
    }

    /* join all threads */
    for (int i = 0; i < threads.size(); ++i) {
        EXPECT_EQ(10, threads[i].get());
    }

    intCache.printDebugInfo(std::cerr);
}