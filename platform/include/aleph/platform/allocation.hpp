/**
 * @file include/aleph/platform/allocation.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstddef>

#include <spdlog/spdlog.h>

namespace aleph::platform {
    class Allocation {
        public:
            Allocation(std::size_t requestedSize);

            Allocation(const Allocation& other) = delete;
            Allocation(Allocation&& other);
            auto operator=(const Allocation& other) -> Allocation& = delete;
            auto operator=(Allocation&& other) -> Allocation&;

            static auto areLargePagesAvailable() -> bool;
            static auto getPageSize() -> std::size_t;

            ~Allocation();

        private:
            void* ptr;
            std::size_t size;
    };

    template <typename T>
    class SubAllocation {
        public:
            SubAllocation(void* p, std::size_t s) : ptr(p), size(s) {
                if (size % sizeof(T) != 0) {
                    spdlog::error(
                        "Size of type for SubAllocation did not divide size of SubAllocation! "
                        "(Allocated Size {} | Must divide {})",
                        size, sizeof(T));
                }
            }

            auto operator[](std::size_t idx) -> T& { return reinterpret_cast<T*>(ptr)[idx]; }

            auto getSize() const { return size / sizeof(T); }

        private:
            void* ptr;
            std::size_t size;
    };
}  // namespace aleph::platform