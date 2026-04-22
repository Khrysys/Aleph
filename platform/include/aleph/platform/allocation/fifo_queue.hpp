/**
 * @file include/aleph/platform/allocation/free_list.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace aleph::platform::allocation {
    template <typename T, std::size_t S>
    class FIFOQueue {
        public:
            void push(T elem) {
                std::size_t oldHead;
                std::size_t newHead;

                // Ensure exclusive access to idx from other pushes,
                // with fast loopback
                do {
                    oldHead = head.load();
                    newHead = oldHead + 1;
                    if(newHead >= S) {
                        newHead = 0;
                    }
                } while(!head.compare_exchange_weak(oldHead, newHead));

                // Write data
                data[oldHead] = elem;

                // Allow pops
                size.fetch_add(1);
            }
            auto pop() -> T {
                std::size_t oldTail;
                std::size_t newTail;

                // Ensure exclusive access to idx from other pops
                do {
                    oldTail = tail.load();
                    newTail = oldTail + 1;
                    if(newTail >= S) {
                        newTail = 0;
                    }
                    auto s = size.load();
                    if(s == 0) {
                        throw std::exception("FIFOQueue out of data");
                    }
                } while(!tail.compare_exchange_weak(oldTail, newTail));

                auto val = data[oldTail];
                size.fetch_sub(1);
                return val;
            }

            auto maxSize() {
                return S;
            }
        private:
            std::array<T, S> data{};
            std::atomic<std::size_t> head{0};
            std::atomic<std::size_t> tail{0};
            std::atomic<std::size_t> size{0};
    };
}  // namespace aleph::platform::allocation
