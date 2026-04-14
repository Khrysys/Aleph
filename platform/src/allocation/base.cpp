#include <cstddef>
#include <stdexcept>

#include <aleph/platform.hpp>

#include "../os_dependent.hpp"

namespace aleph::platform::allocation {

    Allocation::Allocation(std::size_t requestedSize, std::size_t numaNode)
        : ptr(nullptr), numaNode(numaNode) {
        auto pageSize = getPageSize();
        size          = (requestedSize + pageSize - 1) & ~(pageSize - 1);
#if BOOST_OS_WINDOWS
        // ----------------------------
        // NUMA + Large Pages path
        // ----------------------------
        if (areLargePagesAvailable()) {
            // NOTE: Must use NUMA-aware allocation call
            ptr = VirtualAllocExNuma(GetCurrentProcess(), nullptr, size,
                                     MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE,
                                     static_cast<DWORD>(numaNode));

            if (ptr == nullptr) {
                // fallback: try non-large NUMA allocation
                ptr =
                    VirtualAllocExNuma(GetCurrentProcess(), nullptr, size, MEM_RESERVE | MEM_COMMIT,
                                       PAGE_READWRITE, static_cast<DWORD>(numaNode));
            }
        }

        // ----------------------------
        // NUMA normal pages fallback
        // ----------------------------
        if (ptr == nullptr) {
            ptr = VirtualAllocExNuma(GetCurrentProcess(), nullptr, size, MEM_RESERVE | MEM_COMMIT,
                                     PAGE_READWRITE, static_cast<DWORD>(numaNode));
        }
#elif BOOST_OS_LINUX

        bool numaAvailable = (numa_available() != -1);

        // ----------------------------
        // NUMA + HugePages path
        // ----------------------------
        if (numaAvailable && areLargePagesAvailable()) {
            // Set NUMA policy BEFORE allocation (critical)
            unsigned long nodemask = (1UL << numaNode);

            if (set_mempolicy(MPOL_BIND, &nodemask, sizeof(nodemask) * 8) != 0) {
                // fall through if policy cannot be set
            }

            ptr                       = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

            // reset policy to default (important!)
            unsigned long defaultMask = ~0UL;
            set_mempolicy(MPOL_DEFAULT, &defaultMask, sizeof(defaultMask) * 8);

            if (ptr == MAP_FAILED) {
                ptr = nullptr;
            }
        }

        // ----------------------------
        // NUMA normal pages fallback
        // ----------------------------
        if (ptr == nullptr && numaAvailable) {
            unsigned long nodemask = (1UL << numaNode);

            ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

            if (ptr != MAP_FAILED && ptr != nullptr) {
                mbind(ptr, size, MPOL_BIND, &nodemask, sizeof(nodemask) * 8, MPOL_MF_STRICT);
            } else {
                ptr = nullptr;
            }
        }

#endif

        if (ptr == nullptr) {
#if BOOST_OS_WINDOWS
            DWORD flags = MEM_RESERVE | MEM_COMMIT;
            if (areLargePagesAvailable()) {
                flags |= MEM_LARGE_PAGES;
            }

            ptr = VirtualAlloc(nullptr, size, flags, PAGE_READWRITE);
#elif BOOST_OS_LINUX
            auto flags = PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS;
            if (areLargePagesAvailable()) {
                flags |= MAP_HUGETLB;
            }

            ptr = mmap(nullptr, size, flags, -1, 0);
#endif
        }
#if BOOST_OS_LINUX
        if (ptr == nullptr || ptr == MAP_FAILED)
#else
        if (ptr == nullptr)
#endif
        {
            throw std::runtime_error("NUMA + HugePage allocation failed.");
        }
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

}  // namespace aleph::platform::allocation