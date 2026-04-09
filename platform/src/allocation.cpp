#include <cstddef>

#include <spdlog/spdlog.h>

#include <aleph/platform.hpp>

#include "os_dependent.hpp"

namespace aleph::platform {
    Allocation::Allocation(std::size_t requestedSize) : ptr(nullptr) {
        auto pageSize = getPageSize();
        size          = (requestedSize + pageSize - 1) & ~(pageSize - 1);

        if (areLargePagesAvailable()) {
#if BOOST_OS_WINDOWS
            ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES,
                               PAGE_READWRITE);
#elif BOOST_OS_LINUX || BOOST_OS_MACOS
            ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANANYMOUS | MAP_HUGETLB, -1, 0);

#else
#endif
        }

        if (ptr == nullptr) {
#if BOOST_OS_WINDOWS
            ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
#elif BOOST_OS_LINUX || BOOST_OS_MACOS
            ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
#endif
        }

        if (ptr == nullptr) {
            spdlog::error("Standard page allocation failed.");
        }
    }

    Allocation::Allocation(Allocation&& other) {
        ptr        = other.ptr;
        size       = other.size;

        other.ptr  = nullptr;
        other.size = 0;
    }

    auto Allocation::operator=(Allocation&& other) -> Allocation& {
        ptr        = other.ptr;
        size       = other.size;

        other.ptr  = nullptr;
        other.size = 0;
        return *this;
    }

    auto Allocation::areLargePagesAvailable() -> bool {
        static const auto available = []() noexcept -> bool {
#if BOOST_OS_WINDOWS
            auto largePageMinimum = GetLargePageMinimum();
            if (largePageMinimum == 0) {
                return false;
            }

            HANDLE token;
            if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                 &token) == 0) {
                return false;
            }

            LUID luid;
            if (LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &luid)) {
                CloseHandle(token);
                return false;
            }

            TOKEN_PRIVILEGES privileges;
            privileges.PrivilegeCount           = 1;
            privileges.Privileges[0].Luid       = luid;
            privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            if (AdjustTokenPrivileges(token, FALSE, &privileges, 0, nullptr, nullptr) == 0) {
                CloseHandle(token);
                return false;
            }
            return true;
#elif BOOST_OS_LINUX || BOOST_OS_MACOS
            std::ifstream f("/sys/kernel/mm/hugepages/hugepages-2048kB/hugepages-total");
            if (!f.is_open()) {
                return false;
            }
            std::size_t count = 0;
            f >> count;
            return count > 0;
#else
            return false;
#endif
        }();
        return available;
    }

    inline auto Allocation::getPageSize() -> std::size_t {
        static const auto page_size = []() noexcept -> std::size_t {
#if BOOST_OS_WINDOWS
            if (std::size_t largeSize = GetLargePageMinimum(); largeSize != 0) {
                return largeSize;
            }
            SYSTEM_INFO info;
            GetSystemInfo(&info);
            return static_cast<std::size_t>(info.dwPageSize);
#elif BOOST_OS_LINUX || BOOST_OS_MACOS
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

    Allocation::~Allocation() {
        if (ptr != nullptr) {
#if BOOST_OS_WINDOWS
            VirtualFree(ptr, 0, MEM_RELEASE);
#elif BOOST_OS_LINUX || BOOST_OS_MACOS
            munmap(ptr, size);
#endif
        }
    }
}  // namespace aleph::platform
