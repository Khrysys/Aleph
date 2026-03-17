/**
 * @file include/aleph/platform/allocation/init.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstddef>
#include <cstdint>

#include <boost/predef.h>

// Get OS-Specific headers for Large/huge pages support. MacOS is not supported at this moment.
#if BOOST_OS_WINDOWS
    #include <windows.h>
#elif BOOST_OS_LINUX
    #include <fstream>
    #include <unistd.h>

    #include <sys/mman.h>
#endif

namespace aleph::platform::allocation {
    /**
     * Returns true if large/huge pages are available on this system.
     *
     * On Windows, checks `GetLargePageMinimum()` is non-zero.
     * On Linux, checks `/sys/kernel/mm/hugepages/hugepages-2048kB/hugepages-total`.
     * Result is cached after the first call.
     *
     * @return true if large pages are available on this system.
     */
    [[nodiscard]] inline auto isHugePagesAvailable() noexcept -> bool;

    /**
     * Attempts to acquire the privileges necessary for large page allocation.
     *
     * On Windows, requests `SeLockMemoryPrivilege` for the current process token.
     * On Linux, this is a no-op — huge page availability is determined by system
     * configuration, not process privileges. Instead, this function becomes equivalent
     * to `isHugePagesAvailable()`.
     *
     * Should be called once at engine startup, after logging is initialized,
     * before any calls to `allocate`.
     *
     * @return true if large pages are available and ready to use, false otherwise.
     */
    [[nodiscard]] inline auto requestHugePages() noexcept -> bool;

    /**
     * Returns the preferred page size for allocations on this system.
     *
     * On Windows, queries `GetLargePageMinimum()`. Falls back to
     * `GetSystemInfo()` standard page size if large pages are unavailable.
     * On Linux, returns 2MB if huge pages are available, otherwise
     * `sysconf(_SC_PAGESIZE)`. Result is cached after the first call.
     *
     * @return Page size in bytes.
     */
    [[nodiscard]] inline auto getPageSize() noexcept -> std::size_t;

}  // namespace aleph::platform::allocation

// NOLINTNEXTLINE
#include "init.inl"