#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <stdexcept>

#include <aleph/platform/allocation/fifo_queue.hpp>

using aleph::platform::allocation::FIFOQueue;

TEST(FIFOQueueTest, PushPopSingleElement) {
    FIFOQueue<int, 8> q;

    q.push(42);
    EXPECT_EQ(q.pop(), 42);
}

TEST(FIFOQueueTest, FIFOOrderPreserved) {
    FIFOQueue<int, 8> q;

    for (int i = 0; i < 5; ++i) {
        q.push(i);
    }

    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(q.pop(), i);
    }
}

TEST(FIFOQueueTest, WraparoundBehavior) {
    constexpr std::size_t N = 4;
    FIFOQueue<std::size_t, N> q;

    // Fill queue to capacity
    for (std::size_t i = 0; i < N; ++i) {
        q.push(i);
    }

    // Pop 2 elements
    EXPECT_EQ(q.pop(), 0);
    EXPECT_EQ(q.pop(), 1);

    // Push more elements to force wraparound of head index
    q.push(100);
    q.push(101);

    // Remaining original + new values
    EXPECT_EQ(q.pop(), 2);
    EXPECT_EQ(q.pop(), 3);
    EXPECT_EQ(q.pop(), 100);
    EXPECT_EQ(q.pop(), 101);
}

TEST(FIFOQueueTest, PopOnEmptyThrows) {
    FIFOQueue<int, 8> q;

    EXPECT_THROW(q.pop(), std::exception);
}

TEST(FIFOQueueTest, AlternatingPushPopWorks) {
    FIFOQueue<int, 8> q;

    q.push(1);
    EXPECT_EQ(q.pop(), 1);

    q.push(2);
    q.push(3);
    EXPECT_EQ(q.pop(), 2);

    q.push(4);
    EXPECT_EQ(q.pop(), 3);
    EXPECT_EQ(q.pop(), 4);
}

TEST(FIFOQueueTest, StressSequentialOperations) {
    FIFOQueue<int, 16> q;

    for (int i = 0; i < 1000; ++i) {
        q.push(i);
        EXPECT_EQ(q.pop(), i);
    }
}

TEST(FIFOQueueTest, MultiThreadedPushPopBasic) {
    FIFOQueue<int, 64> q;

    constexpr int count = 1000;

    std::thread producer([&]() {
        for (int i = 0; i < count; ++i) {
            q.push(i);
        }
    });

    std::thread consumer([&]() {
        int received = 0;
        while (received < count) {
            try {
                (void)q.pop();
                ++received;
            } catch (...) {
                // ignore empty queue
            }
        }
    });

    producer.join();
    consumer.join();

    SUCCEED();
}