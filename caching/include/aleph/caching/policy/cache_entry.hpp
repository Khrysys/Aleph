#pragma once

#include <array>
#include <cstddef>

#include <half.hpp>

#include <aleph/chess.hpp>

namespace aleph::caching::policy {
    namespace detail {
        inline constexpr std::size_t CACHED_POLICY_SIZE = 12;
    }

    struct CacheEntry {
        half_float::half w;
        half_float::half l;
        std::array<chess::Move, detail::CACHED_POLICY_SIZE> moves;
        std::array<half_float::half, detail::CACHED_POLICY_SIZE> scores;
        uint8_t policyCount;
        uint8_t generation;
        half_float::half movesLeft;
    };

    static_assert(sizeof(CacheEntry) == 56);
}