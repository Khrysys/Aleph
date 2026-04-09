#include <cstddef>

#include <aleph/platform/topology.hpp>

#include "os_dependent.hpp"

namespace aleph::platform::topology {
    Topology::Topology() {
#if BOOST_OS_WINDOWS
        // While Windows does support NUMA, it is complex and unusual, so I have put it on hold for
        // now.
        localMemory.emplace_back(0);
#endif
    }

    LocalMemory::LocalMemory(std::size_t nodeID) {
#if BOOST_OS_WINDOWS
        MEMORYSTATUSEX memoryStatus;
        if(!GlobalMemoryStatusEx(&memoryStatus)) {
            
        }
#endif
    }
}  // namespace aleph::platform::topology