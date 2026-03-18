/**
 * @file include/aleph/platform/allocation/init.inl
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cassert>
#include <cstddef>
#include <fstream>

#include <spdlog/spdlog.h>

#include "init.hpp"

namespace aleph::platform::allocation {
    inline auto isHugePagesAvailable() noexcept -> bool {
        static const auto available = []() noexcept {
#if BOOST_OS_WINDOWS
            return GetLargePageMinimum() != 0;
#elif BOOST_OS_LINUX
            std::ifstream f("/sys/kernel/mm/hugepages/hugepages-2048kB/hugepages-total");
            if (!f.is_open()) return false;
            std::size_t count = 0;
            f >> count;
            return count > 0;
#else
            return false;
#endif
        }();
        return available;
    }

    inline auto requestHugePages() noexcept -> bool {
        if (!isHugePagesAvailable()) {
            spdlog::warn(
                "Large/huge pages are not available on this system. "
                "Falling back to standard pages.");
            return false;
        }
#if BOOST_OS_WINDOWS
        HANDLE token;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
            spdlog::warn(
                "Failed to open process token for SeLockMemoryPrivilege. "
                "Large pages unavailable. Error: {}",
                GetLastError());
            return false;
        }

        LUID luid;
        if (!LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &luid)) {
            spdlog::warn(
                "Failed to look up SeLockMemoryPrivilege LUID. "
                "Error: {}",
                GetLastError());
            CloseHandle(token);
            return false;
        }

        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount           = 1;
        tp.Privileges[0].Luid       = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (!AdjustTokenPrivileges(token, FALSE, &tp, 0, nullptr, nullptr) ||
            GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
            spdlog::warn(
                "Failed to acquire SeLockMemoryPrivilege. "
                "Grant the privilege via Local Security Policy. "
                "Error: {}",
                GetLastError());
            CloseHandle(token);
            return false;
        }

        CloseHandle(token);
        spdlog::info("SeLockMemoryPrivilege acquired. Large pages enabled.");
        return true;
#elif BOOST_OS_LINUX
        spdlog::info("Huge pages available. Large pages enabled.");
        return true;
#else
        spdlog::warn(
            "Large pages not supported on this platform. "
            "Falling back to standard pages.");
        return false;
#endif
    }

    inline auto getPageSize() noexcept -> std::size_t {
        static const auto page_size = []() noexcept {
#if BOOST_OS_WINDOWS
            if (std::size_t largeSize = GetLargePageMinimum(); largeSize != 0) {
                return largeSize;
            }
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            return static_cast<std::size_t>(si.dwPageSize);
#elif BOOST_OS_LINUX
            if (isHugePagesAvailable()) {
                return static_cast<std::size_t>(2 * 1024 * 1024);
            }
            return static_cast<std::size_t>(sysconf(_SC_PAGESIZE));
#else
            return static_cast<std::size_t>(4096);
#endif
        }();
        return page_size;
    }
}  // namespace aleph::platform::allocation