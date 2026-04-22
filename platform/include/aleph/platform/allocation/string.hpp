/**
 * @file include/aleph/platform/allocation/string.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <array>
#include <atomic>
#include <cstddef>

#include <libassert/assert.hpp>

#include "base.hpp"
#include "fifo_queue.hpp"

namespace aleph::platform::allocation {
    namespace detail {
        constexpr std::size_t STRING_ARENA_SIZE      = 512;
        constexpr std::size_t STRING_BUFFER_SIZE     = 2048;
        constexpr std::size_t STRING_ALLOCATION_SIZE = STRING_ARENA_SIZE * STRING_BUFFER_SIZE;
        constexpr std::size_t STRING_ARENA_TLS_SIZE  = 16;
    }  // namespace detail

    class StringArena;

    class NUMAString {
        public:
            NUMAString(StringArena* arena, char* ptr, std::size_t idx);

            NUMAString(const NUMAString& other);
            auto operator=(const NUMAString& other) -> NUMAString&;

            ~NUMAString();

            void push_back(char c) {
                DEBUG_ASSERT(filled < detail::STRING_BUFFER_SIZE);
                ptr[filled++] = c;
            }

            [[nodiscard]] auto c_str() const -> const char* { return ptr; }
            [[nodiscard]] auto size() const -> std::size_t { return filled; }

        private:
            StringArena* arena = nullptr;
            char* ptr          = nullptr;
            std::size_t filled = 0;
            std::size_t idx    = 0;
    };

    class StringArena {
        public:
            StringArena(Allocation* alloc);

            StringArena(const StringArena& other)                    = delete;
            StringArena(StringArena&& other)                         = delete;

            auto operator=(const StringArena& other) -> StringArena& = delete;
            auto operator=(StringArena&& other) -> StringArena&      = delete;

            auto allocate() -> NUMAString;

            void release(std::size_t idx);

        private:
            struct PaddedAtomic {
                std::atomic<std::size_t> value{0};
            };

            SubAllocation<char> alloc;

            std::array<PaddedAtomic, detail::STRING_ARENA_SIZE> refs{};
            FIFOQueue<std::size_t, detail::STRING_ARENA_SIZE> fl;

            friend class NUMAString;
    };
}  // namespace aleph::platform::allocation