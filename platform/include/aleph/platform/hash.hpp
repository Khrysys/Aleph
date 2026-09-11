#pragma once

#include <cstdint>

namespace aleph::platform {
    [[nodiscard]] constexpr auto splitMix64(std::uint64_t& key) {
        std::uint64_t val = (key += 0x9e3779b97f4a7c15);
        val               = (val ^ (val >> 30)) * 0xbf58476d1ce4e5b9;
        val               = (val ^ (val >> 27)) * 0x94d049bb133111eb;
        return val ^ (val >> 31);
    }
}  // namespace aleph::platform