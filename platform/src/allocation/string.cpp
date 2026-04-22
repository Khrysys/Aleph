/**
 * @file src/allocation/string.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <array>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <stdexcept>

#include <aleph/platform/allocation/string.hpp>

namespace aleph::platform::allocation {
    thread_local std::array<std::size_t, detail::STRING_ARENA_TLS_SIZE> tlsCache{};
    thread_local std::size_t tlsCacheSize = 0;

    NUMAString::NUMAString(StringArena* arena, char* ptr, std::size_t idx)
        : arena(arena), ptr(ptr), idx(idx) {}

    NUMAString::NUMAString(const NUMAString& other)
        : arena(other.arena), ptr(other.ptr), filled(other.filled), idx(other.idx) {
        arena->refs[idx].value.fetch_add(1, std::memory_order_relaxed);
    }

    auto NUMAString::operator=(const NUMAString& other) -> NUMAString& {
        if (this == &other) {
            return *this;
        }

        if (arena != nullptr) {
            arena->release(idx);
        }

        arena  = other.arena;
        ptr    = other.ptr;
        filled = other.filled;
        idx    = other.idx;

        arena->refs[idx].value.fetch_add(1, std::memory_order_acquire);

        return *this;
    }

    NUMAString::~NUMAString() {
        if (arena != nullptr) {
            arena->release(idx);
        }
    }

    StringArena::StringArena(Allocation* pAlloc)
        : alloc(pAlloc->getSubAllocation<char>(detail::STRING_ALLOCATION_SIZE)) {
            for(auto i = 0; i < fl.maxSize(); i++) {
                fl.push(i);
            }
        }

    auto StringArena::allocate() -> NUMAString {
        // 1. Fast path: TLS cache
        if (tlsCacheSize > 0) {
            std::size_t idx = tlsCache[--tlsCacheSize];
            refs[idx].value.fetch_add(1, std::memory_order_relaxed);

            return {this, &alloc[idx * detail::STRING_BUFFER_SIZE], idx};
        }

        auto idx = fl.pop();
        refs[idx].value.fetch_add(1, std::memory_order_relaxed);

        return {this, &alloc[idx * detail::STRING_BUFFER_SIZE], idx};
    }

    void StringArena::release(std::size_t idx) {
        // decrement ownership
        auto prev = refs[idx].value.fetch_sub(1, std::memory_order_relaxed);

        if (prev != 1) {
            return;  // still referenced elsewhere
        }

        // Slot is now globally free — push to TLS first if possible
        if (tlsCacheSize < tlsCache.size()) {
            tlsCache[tlsCacheSize++] = idx;
            return;
        }

        // fallback: global free list
        fl.push(idx);
    }
}  // namespace aleph::platform::allocation