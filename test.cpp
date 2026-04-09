#include <iostream>
#include <vector>

class HardwareTopology {
    public:
        HardwareTopology() {
            hwloc_topology_init(&rawTopology);
            hwloc_topology_set_io_types_filter(rawTopology, HWLOC_TYPE_FILTER_KEEP_ALL);
            hwloc_topology_load(rawTopology);

            int numaNodeCount = hwloc_get_nbobjs_by_type(rawTopology, HWLOC_OBJ_NUMANODE);
            std::cout << "NUMA nodes: " << numaNodeCount << "\n\n";

            for (int i = 0; i < numaNodeCount; i++) {
                auto* numa = hwloc_get_obj_by_type(rawTopology, HWLOC_OBJ_NUMANODE, i);

                std::cout << "NUMA Node " << i << ":\n";

                // --- Logical cores (PUs)
                int logical =
                    hwloc_get_nbobjs_inside_cpuset_by_type(rawTopology, numa->cpuset, HWLOC_OBJ_PU);

                // --- Physical cores
                int physical = hwloc_get_nbobjs_inside_cpuset_by_type(rawTopology, numa->cpuset,
                                                                      HWLOC_OBJ_CORE);

                std::cout << "  Physical cores: " << physical << "\n";
                std::cout << "  Logical cores:  " << logical << "\n";

                if (numa->attr) {
                    auto mem = numa->attr->numanode.local_memory;

                    if (mem > 0) {
                        double gb = mem / (1024.0 * 1024.0 * 1024.0);
                        std::cout << "  Local memory:   " << gb << " GB\n";
                    } else {
                        std::cout << "  Local memory:   Unknown\n";
                    }
                }

                // --- Memory bandwidth / speed (not standardized)
                // hwloc usually cannot give DRAM speed directly
                // You would need platform-specific APIs for this
                std::cout << "  Memory speed:   (not available via hwloc)\n";

                // --- GPU / accelerator detection
                int accelCount = 0;

                int osDevCount = hwloc_get_nbobjs_by_type(rawTopology, HWLOC_OBJ_OS_DEVICE);
                std::cout << "  OS Devices:     " << osDevCount << "\n";
                for (int j = 0; j < osDevCount; j++) {
                    auto* pci = hwloc_get_obj_by_type(rawTopology, HWLOC_OBJ_OS_DEVICE, j);

                    // Class 0x03 = display controller (GPU)
                    if ((pci->attr->pcidev.class_id >> 8) == 0x03) {
                        // Avoid double-counting multi-function devices
                        if (pci->parent && pci->parent->type == HWLOC_OBJ_BRIDGE) continue;

                        accelCount++;
                    }
                }

                std::cout << "  Accelerators:   " << accelCount << "\n";

                std::cout << std::endl;
            }
        }
        HardwareTopology(const HardwareTopology&)                    = delete;
        auto operator=(const HardwareTopology&) -> HardwareTopology& = delete;
        HardwareTopology(HardwareTopology&& other) {
            nodes             = std::move(other.nodes);
            rawTopology       = std::move(other.rawTopology);
            other.rawTopology = nullptr;
            other.nodes.clear();
        }
        auto operator=(HardwareTopology&& other) -> HardwareTopology& {
            nodes             = std::move(other.nodes);
            rawTopology       = std::move(other.rawTopology);
            other.rawTopology = nullptr;
            other.nodes.clear();
            return *this;
        }
        ~HardwareTopology() {
            if (rawTopology != nullptr) hwloc_topology_destroy(rawTopology);
        }

    private:
        std::vector<int> nodes;
        hwloc_topology_t rawTopology;
};

auto main() -> int {
    HardwareTopology topology{};
    return 0;
}