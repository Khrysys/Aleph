/**
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <stddef.h>

#include "../compiler.hpp"
#include "runtime.hpp"

namespace aleph::platform::allocation {
#if defined(BOOST_OS_WINDOWS)
    inline AllocationResult allocate(size_t size) {

        // Attempt large pages first — requires SeLockMemoryPrivilege,
        // which should have been acquired via requestHugePages() at startup.
        if (isHugePagesAvailable()) {
            void* ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES,
                                     PAGE_READWRITE);
            if (ptr != nullptr) {
                return {ptr, size, PageSize::Large};
            }
        }

        // Fall back to standard pages.
        void* ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        return {ptr, size, PageSize::Standard};
    }

    inline void deallocate(AllocationResult alloc) noexcept {
        if (alloc.ptr != nullptr) {
            VirtualFree(alloc.ptr, 0, MEM_RELEASE);
        }
    }

#elif defined(BOOST_OS_LINUX)

    inline AllocationResult allocate(size_t size) {
        // Attempt explicit huge pages first.
        if (isHugePagesAvailable()) {
            void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
            if (ptr != MAP_FAILED) {
                return {ptr, size, PageSize::Large};
            }
        }

        // Fall back to standard pages.
        void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (ptr == MAP_FAILED) {
            return {nullptr, 0, PageSize::Standard};
        }

        return {ptr, size, PageSize::Standard};
    }

    inline void deallocate(AllocationResult alloc) noexcept {
        if (alloc.ptr != nullptr) {
            munmap(alloc.ptr, alloc.size);
        }
    }

#endif

}  // namespace aleph::platform::allocation