#include <cstdint>

namespace aleph::platform {
    [[nodiscard]] auto bindThread(std::uint32_t numa_node, std::uint32_t core_id) noexcept
        -> bool;
}