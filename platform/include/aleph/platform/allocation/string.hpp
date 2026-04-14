#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <stdexcept>

#include "base.hpp"

namespace aleph::platform::allocation {
    namespace detail {
        constexpr std::size_t STRING_ARENA_SIZE      = 512;
        constexpr std::size_t STRING_BUFFER_SIZE     = 2048;
        constexpr std::size_t STRING_ALLOCATION_SIZE = STRING_ARENA_SIZE * STRING_BUFFER_SIZE;
        constexpr std::size_t STRING_ARENA_TLS_SIZE  = 64;
    }  // namespace detail

    class StringArena;

    class NUMAString {
        public:
            NUMAString(StringArena* arena, char* ptr, std::size_t idx);

            NUMAString(const NUMAString& other);
            NUMAString& operator=(const NUMAString& other);

            ~NUMAString();

            void push_back(char c) { ptr[filled++] = c; }

            const char* c_str() const { return ptr; }
            std::size_t size() const { return filled; }

        private:
            StringArena* arena = nullptr;
            char* ptr          = nullptr;
            std::size_t filled = 0;
            std::size_t idx    = 0;
    };

    class StringArena {
        public:
            StringArena(Allocation* alloc);

            NUMAString allocate();

            void release(std::size_t idx);

        private:
            SubAllocation<char> alloc;

            std::array<std::atomic<std::size_t>, detail::STRING_ARENA_SIZE> refs{};
            std::array<std::size_t, detail::STRING_ARENA_SIZE> freeIndices{};
            std::atomic<std::size_t> freeSize{0};

            std::mutex freeMutex;

            friend class NUMAString;
    };
}  // namespace aleph::platform::allocation