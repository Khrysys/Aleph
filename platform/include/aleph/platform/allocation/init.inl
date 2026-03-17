/**
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstddef>

#include "../compiler.hpp"
#include "init.hpp"

namespace aleph::platform::allocation {

    inline bool isHugePagesAvailable() noexcept {
        static const bool available = []() noexcept -> bool {
#if BOOST_OS_WINDOWS
            return GetLargePageMinimum() != 0;
#elif BOOST_OS_LINUX
            std::ifstream f("/sys/kernel/mm/hugepages/hugepages-2048kB/hugepages-total");
            if (!f.is_open()) return false;
            std::size_t count = 0;
            f >> count;
            return count > 0;
#endif
        }();
        return available;
    }

    inline bool requestHugePages() noexcept {
#if BOOST_OS_WINDOWS
        if (!isHugePagesAvailable()) {
            ALEPH_LOG(WARNING) << "Large pages are not supported on this system.";
            return false;
        }

        HANDLE token;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
            ALEPH_LOG(WARNING) << "Failed to open process token for SeLockMemoryPrivilege. "
                         << "Large pages unavailable. Error: " << GetLastError();
            return false;
        }

        LUID luid;
        if (!LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &luid)) {
            ALEPH_LOG(WARNING) << "Failed to look up SeLockMemoryPrivilege LUID. "
                         << "Error: " << GetLastError();
            CloseHandle(token);
            return false;
        }

        TOKEN_PRIVILEGES tp;
        tp.PrivilegeCount           = 1;
        tp.Privileges[0].Luid       = luid;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (!AdjustTokenPrivileges(token, FALSE, &tp, 0, nullptr, nullptr) ||
            GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
            ALEPH_LOG(WARNING)
                << "Failed to acquire SeLockMemoryPrivilege. "
                << "Large pages unavailable. Grant the privilege via Local Security Policy. "
                << "Error: " << GetLastError();
            CloseHandle(token);
            return false;
        }

        CloseHandle(token);
        ALEPH_LOG(INFO) << "SeLockMemoryPrivilege acquired. Large pages enabled.";
        return true;
#elif BOOST_OS_LINUX
        if (!isHugePagesAvailable()) {
            ALEPH_LOG(WARNING) << "Huge pages unavailable. "
                         << "Check hugepages-total > 0 in "
                         << "/sys/kernel/mm/hugepages/hugepages-2048kB/. "
                         << "Falling back to standard pages.";
            return false;
        }
        ALEPH_LOG(INFO) << "Huge pages available. Large pages enabled.";
        return true;
#endif
    }

    inline std::size_t getPageSize() noexcept {
        static const std::size_t page_size = []() noexcept -> std::size_t {
#if BOOST_OS_WINDOWS
            std::size_t large = GetLargePageMinimum();
            if (large != 0) return static_cast<size_t>(large);
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            return static_cast<size_t>(si.dwPageSize);

#elif BOOST_OS_LINUX
            if (isHugePagesAvailable()) {
                return static_cast<size_t>(2 * 1024 * 1024);
            }
            return static_cast<size_t>(sysconf(_SC_PAGESIZE));
#endif
        }();
        return page_size;
    }
}  // namespace aleph::platform::allocation