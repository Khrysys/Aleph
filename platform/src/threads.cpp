#include <thread>
#include <type_traits>
#include <tuple>
#include <stop_token>
#include <cstddef>

#include <aleph/platform.hpp>

#include "os_dependent.hpp"

namespace aleph::platform {
    auto bindThread(std::size_t numaNode, std::size_t coreID) noexcept -> bool {
#if BOOST_OS_WINDOWS  // Get current thread handle
        HANDLE thread                = GetCurrentThread();

        // Step 1: Get processor mask for the NUMA node
        GROUP_AFFINITY groupAffinity = {};
        if (!GetNumaNodeProcessorMaskEx(static_cast<USHORT>(numaNode), &groupAffinity)) {
            return false;
        }

        // Step 2: Iterate bits in mask to find the requested coreID
        const auto mask           = groupAffinity.Mask;
        std::size_t currentIndex = 0;
        auto targetProcessor    = DWORD(-1);

        for (DWORD i = 0; i < sizeof(KAFFINITY) * 8; ++i) {
            if (mask & (KAFFINITY(1) << i)) {
                if (currentIndex == coreID) {
                    targetProcessor = i;
                    break;
                }
                ++currentIndex;
            }
        }

        if (targetProcessor == DWORD(-1)) {
            // coreID out of range for this NUMA node
            return false;
        }

        // Step 3: Set group affinity to this processor only
        GROUP_AFFINITY targetAffinity = {};
        targetAffinity.Group          = groupAffinity.Group;
        targetAffinity.Mask           = (static_cast<KAFFINITY>(1) << targetProcessor);

        if (!SetThreadGroupAffinity(thread, &targetAffinity, nullptr)) {
            return false;
        }

        // Step 4 (optional but recommended): set ideal processor
        PROCESSOR_NUMBER procNum = {};
        procNum.Group            = groupAffinity.Group;
        procNum.Number           = static_cast<BYTE>(targetProcessor);

        return SetThreadIdealProcessorEx(thread, &procNum, nullptr) != 0;
#elif BOOST_OS_LINUX
        if (numa_available() == -1) {
            return false;  // NUMA not supported
        }

        // Step 1: get allowed CPUs for NUMA node
        struct bitmask* nodeMask = numa_allocate_cpumask();
        if (!nodeMask) {
            return false;
        }

        numa_node_to_cpus(static_cast<int>(numaNode), nodeMask);

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);

        // Step 2: extract CPUs from NUMA mask
        std::size_t currentIndex = 0;
        int targetCpu            = -1;

        for (int cpu = 0; cpu < numa_num_possible_cpus(); ++cpu) {
            if (numa_bitmask_isbitset(nodeMask, cpu)) {
                if (currentIndex == coreID) {
                    targetCpu = cpu;
                    break;
                }
                ++currentIndex;
            }
        }

        numa_free_cpumask(nodeMask);

        if (targetCpu == -1) {
            return false;  // invalid coreID for node
        }

        CPU_SET(targetCpu, &cpuset);

        // Step 3: pin thread
        pthread_t thread = pthread_self();
        int result       = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

        return result == 0;
#else
        return false;
#endif
    }

    // --- createThread ---
    template <typename Fn, typename... Args>
    auto createThread(std::size_t node, std::size_t core, Fn&& fn, Args&&... args) -> std::jthread {
        // Decay/capture everything safely
        using FnT       = std::decay_t<Fn>;
        using ArgsTuple = std::tuple<std::decay_t<Args>...>;

        FnT fnCopy(std::forward<Fn>(fn));
        ArgsTuple argsCopy(std::forward<Args>(args)...);

        return std::jthread([node, core, fn = std::move(fnCopy),
                             args = std::move(argsCopy)](std::stop_token st) mutable -> auto {
// Bind first, before doing any real work. We ignore this on platforms other than Windows and Linux
#if !(BOOST_OS_WINDOWS || BOOST_OS_LINUX)
            if (!bindThread(node, core)) {
                std::terminate();
            }
#endif

            // Invoke user function
            if constexpr (std::is_invocable_v<FnT, std::stop_token, Args...>) {
                std::apply(
                    [&](auto&&... unpacked) -> auto {
                        fn(st, std::forward<decltype(unpacked)>(unpacked)...);
                    },
                    args);
            } else {
                std::apply(fn, args);
            }
        });
    }
}  // namespace aleph::platform