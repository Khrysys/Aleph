/**
 * @file include/aleph/platform/allocation/runtime.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cassert>
#include <cstddef>

#include <spdlog/spdlog.h>

#include "../compiler.hpp"
#include "runtime.hpp"

namespace aleph::platform::allocation {

    inline auto allocate(std::size_t size) noexcept -> AllocationResult {
        assert(size % getPageSize() == 0);

#if BOOST_OS_WINDOWS
        // Attempt large pages first — requires SeLockMemoryPrivilege,
        // which should have been acquired via requestHugePages() at startup.
        if (isHugePagesAvailable()) {
            void* ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES,
                                     PAGE_READWRITE);
            if (ptr != nullptr) {
                return {ptr, size, PageSize::Large};
            }
            spdlog::warn(
                "Large page allocation failed despite SeLockMemoryPrivilege. "
                "Falling back to standard pages. Error: {}",
                GetLastError());
        }

        // Fall back to standard pages.
        void* ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (ptr == nullptr) {
            spdlog::error("Standard page allocation failed. Error: {}", GetLastError());
        }
        return {ptr, size, PageSize::Standard};

#elif BOOST_OS_LINUX
        // Attempt explicit huge pages first.
        if (isHugePagesAvailable()) {
            void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
            if (ptr != MAP_FAILED) {
                return {ptr, size, PageSize::Large};
            }
            spdlog::warn(
                "MAP_HUGETLB allocation failed despite huge pages being available. "
                "Falling back to standard pages.");
        }

        // Fall back to standard pages.
        void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (ptr == MAP_FAILED) {
            spdlog::error("Standard page allocation failed.");
            return {nullptr, 0, PageSize::Standard};
        }

        return {ptr, size, PageSize::Standard};

#else
        spdlog::error("Platform does not support large page allocation.");
        return {nullptr, 0, PageSize::Standard};
#endif
    }

    inline auto deallocate(AllocationResult alloc) noexcept -> void {
#if BOOST_OS_WINDOWS
        if (alloc.ptr != nullptr) {
            VirtualFree(alloc.ptr, 0, MEM_RELEASE);
        }
#elif BOOST_OS_LINUX
        if (alloc.ptr != nullptr) {
            munmap(alloc.ptr, alloc.size);
        }
#endif
    }

}  // namespace aleph::platform::allocation