#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <aleph/chess.hpp>

#include "../common/types.hpp"

namespace aleph::caching::policy {
    /**
     * Byte Layout
     * ---
     * [0:19] Move moves[10]
     * [20:39] half scores[10]
     */
    struct EntryPolicy {
        public:
            std::array<chess::Move, common::MAX_POLICY_SIZE> moves;
            std::array<common::float_type, common::MAX_POLICY_SIZE> scores;
    };

    /**
     * Byte layout
     * -----------
     * [0:1] half w
     * [2:3] half l
     * [4:5] uint16_t generation
     * [6:7] half movesLeft
     * [8:47] EntryPolicy policy;
     */
    struct Entry {
        public:
            common::float_type w;
            common::float_type l;
            std::uint16_t generation;
            common::float_type movesLeft;
            EntryPolicy policy;
    };

    /**
     * Byte Layout
     * ---
     * [0:7] atomic<uint64_t> version
     * [8:15] uint64_t hash
     * [16:63] RawEntry entry
     */
    struct alignas(64) EntryContainer {
        public:
            std::atomic<uint64_t> version;
            uint64_t hash;
            Entry entry;
    };

    static_assert(sizeof(EntryContainer) == 64);
}  // namespace aleph::caching::policy