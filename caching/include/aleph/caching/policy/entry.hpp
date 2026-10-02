/**
 * @file include/aleph/caching/policy/entry.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

#include <half.hpp>

#include <aleph/chess.hpp>

namespace aleph::caching::policy {
    constexpr std::uint64_t MAX_POLICY_SIZE = 10;
    /**
     * Byte layout
     * -----------
     * [0:7] uint64_t hash
     * [8:9] half w
     * [10:11] half l
     * [12:13] uint16_t generation
     * [14:15] half movesLeft
     * [16:35] Move policy[10]
     * [36:55] half scores[10]
     */
    struct Entry {
        public:
            std::uint64_t hash;
            half_float::half w;
            half_float::half l;
            std::uint16_t generation;
            half_float::half movesLeft;
            std::array<chess::Move, MAX_POLICY_SIZE> moves;
            std::array<half_float::half, MAX_POLICY_SIZE> scores;
    };

    /**
     * Byte Layout
     * ---
     * [0:7] uint64_t version
     * [8:63] Entry entry
     */
    struct alignas(64) EntryContainer {
        public:
            std::uint64_t version;
            Entry entry;
    };

    static_assert(sizeof(Entry)          == 56);
    static_assert(sizeof(EntryContainer) == 64);
    static_assert(alignof(EntryContainer) == 64);
    static_assert(offsetof(Entry, hash)        == 0);
    static_assert(offsetof(Entry, w)           == 8);
    static_assert(offsetof(Entry, l)           == 10);
    static_assert(offsetof(Entry, generation)  == 12);
    static_assert(offsetof(Entry, movesLeft)   == 14);
    static_assert(offsetof(Entry, moves)       == 16);
    static_assert(offsetof(Entry, scores)      == 36);
    static_assert(std::is_trivially_copyable_v<Entry>);
    static_assert(std::is_trivially_copyable_v<EntryContainer>);
    static_assert(std::is_standard_layout_v<Entry>);
    static_assert(std::is_standard_layout_v<EntryContainer>);
}  // namespace aleph::caching::policy

