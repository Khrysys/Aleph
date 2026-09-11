#pragma once

#include <cstdint>
#include <cstddef>
#include <limits>

namespace aleph::caching::common {
    using float_type = std::uint16_t;

    constexpr std::size_t MAX_POLICY_SIZE = 10;
    constexpr float FLOAT_STEP_SIZE = 1.0F / std::numeric_limits<float_type>::max();
}