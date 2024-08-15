
#include "gtest/gtest.h"

#include <iostream>
#include <array>
#include <thread>

#include "allocator/Allocator.hpp"

class AllocatorTest : public ::testing::Test {
protected:
    static constexpr uint32_t block_size = 37;
    static constexpr uint32_t num_blocks = 16;
    
    using Allocator = allocators::Allocator<uint16_t, block_size, num_blocks, alignof(uint16_t)>;

    virtual void SetUp();
    virtual void TearDown();

    static void test_task_handler(Allocator& allocator, uint8_t value);

    /*
        For some unknown reason atomic version randomly fails multithreaded test
    */
    static void test_task_handler_with_atomic(Allocator& allocator, uint8_t value);
};

void AllocatorTest::SetUp() {

}

void AllocatorTest::TearDown() {

}

TEST_F(AllocatorTest, Allocate) {
    Allocator allocator;
    std::array<void*, num_blocks> blocks;

    for(auto block : blocks) {
        block = allocator.allocate_with_mutex();

        ASSERT_NE(block, nullptr);
    }

    ASSERT_EQ(allocator.allocate_with_mutex(), nullptr);
    ASSERT_EQ(allocator.allocate_with_mutex(), nullptr);
}

TEST_F(AllocatorTest, AllocateDeallocate) {
    Allocator allocator;
    std::array<void*, num_blocks> blocks;

    for(void*& block : blocks) {
        block = allocator.allocate_with_mutex();

        ASSERT_NE(block, nullptr);
    }

    for(void*& block : blocks) {
        allocator.deallocate_with_mutex(block);
    }
}

void AllocatorTest::test_task_handler(Allocator& allocator, uint8_t value) {
    uint32_t counter = 10000;
    std::array<void*, 2> blocks;
    std::array<uint8_t, block_size> sample;

    usleep(5000);

    sample.fill(value);

    while(counter--) {
        for(void*& block : blocks) {
            block = allocator.allocate_with_mutex();

            ASSERT_NE(block, nullptr); 
        }

        for(void*& block : blocks) {
            memset(block, 0x00, block_size);
        }

        for(void*& block : blocks) {
            memset(block, value, block_size);
        }

        for(void*& block : blocks) {
            int cmp_result = memcmp(block, sample.data(), block_size);

            ASSERT_EQ(cmp_result, 0);
        }

        for(void*& block : blocks) {
            allocator.deallocate_with_mutex(block);
        }
    }
}

void AllocatorTest::test_task_handler_with_atomic(Allocator& allocator, uint8_t value) {
    uint32_t counter = 10000;
    std::array<void*, 2> blocks;
    std::array<uint8_t, block_size> sample;

    usleep(5000);

    sample.fill(value);

    while(counter--) {
        for(void*& block : blocks) {
            block = allocator.allocate_with_atomic();

            ASSERT_NE(block, nullptr); 
        }

        for(void*& block : blocks) {
            memset(block, 0x00, block_size);
        }

        for(void*& block : blocks) {
            memset(block, value, block_size);
        }

        for(void*& block : blocks) {
            int cmp_result = memcmp(block, sample.data(), block_size);

            ASSERT_EQ(cmp_result, 0);
        }

        for(void*& block : blocks) {
            allocator.deallocate_with_atomic(block);
        }
    }
}

TEST_F(AllocatorTest, Multithreading) {
    Allocator allocator;
    std::thread task1{test_task_handler, std::ref(allocator), 1};
    std::thread task2{test_task_handler, std::ref(allocator), 2};
    std::thread task3{test_task_handler, std::ref(allocator), 3};
    std::thread task4{test_task_handler, std::ref(allocator), 4};

    task1.join();
    task2.join();
    task3.join();
    task4.join();
}


/*
    For some unknown reason atomic version randomly fails multithreaded test
*/
TEST_F(AllocatorTest, Multithreading_with_atomic) {
    Allocator allocator;
    std::thread task1{test_task_handler_with_atomic, std::ref(allocator), 1};
    std::thread task2{test_task_handler_with_atomic, std::ref(allocator), 2};
    std::thread task3{test_task_handler_with_atomic, std::ref(allocator), 3};
    std::thread task4{test_task_handler_with_atomic, std::ref(allocator), 4};

    task1.join();
    task2.join();
    task3.join();
    task4.join();
}
