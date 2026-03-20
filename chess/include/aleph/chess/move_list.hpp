/**
 * @file include/aleph/chess/move_list.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <array>
#include <cstddef>

#include <fmt/format.h>
#include <libassert/assert.hpp>

#include "move.hpp"

namespace aleph::chess {

    /**
     * A fixed-capacity, stack-allocated list of chess moves.
     *
     * Avoids heap allocation entirely, which is critical for MCTS node expansion
     * where move lists are created and discarded at very high frequency. The
     * capacity is a compile-time constant; `MoveList<256>` is used for legal
     * moves (theoretical maximum ~218) and `MoveList<512>` for pseudo-legal moves.
     *
     * Iteration follows standard begin/end conventions and is compatible with
     * range-for loops.
     */
    template <std::size_t Capacity>
    class MoveList {
        public:
            /** The type of the underlying container used to store the moves on the stack. */
            using storage_type   = std::array<Move, Capacity>;
            /** Iterator type, used for range-for loop support. */
            using iterator       = typename storage_type::iterator;
            /** Constant iterator type, used for range-for loop support. */
            using const_iterator = typename storage_type::const_iterator;

            /** Constructs an empty move list. */
            constexpr MoveList() noexcept : _size(0) {}

            // --- Capacity / size ---

            /** Returns the number of moves currently in the list. */
            [[nodiscard]] constexpr inline std::size_t size() const noexcept { return _size; }

            /** Returns true if the list contains no moves. */
            [[nodiscard]] constexpr inline bool empty() const noexcept { return _size == 0; }

            /** Returns the maximum number of moves this list can hold. */
            [[nodiscard]] constexpr static inline std::size_t capacity() noexcept { return Capacity; }

            // --- Element access ---

            /** Returns a reference to the move at index `i`. Asserts bounds in debug builds. */
            [[nodiscard]] constexpr inline Move& operator[](std::size_t i) noexcept {
                DEBUG_ASSERT(i < _size);
                return _moves[i];
            }

            /** Returns a const reference to the move at index `i`. Asserts bounds in debug builds.
             */
            [[nodiscard]] constexpr inline const Move& operator[](std::size_t i) const noexcept {
                DEBUG_ASSERT(i < _size);
                return _moves[i];
            }

            /**
             * Returns true if the list contains the given move.
             * Comparison is performed via `uint16_t` conversion.
             */
            [[nodiscard]] constexpr inline bool contains(const Move& m) const noexcept {
                for (std::size_t i = 0; i < _size; ++i)
                    if (static_cast<uint16_t>(_moves[i]) == static_cast<uint16_t>(m)) return true;
                return false;
            }

            // --- Modifiers ---

            /** Clears the list without deallocating storage. */
            constexpr inline void clear() noexcept { _size = 0; }

            /**
             * Appends a move to the list.
             * Asserts that capacity is not exceeded in debug builds.
             */
            constexpr inline void push_back(const Move& m) noexcept {
                DEBUG_ASSERT(_size < Capacity);
                _moves[_size++] = m;
            }

            // --- Iteration ---

            [[nodiscard]] constexpr inline iterator begin() noexcept { return _moves.begin(); }

            [[nodiscard]] constexpr inline iterator end() noexcept {
                return _moves.begin() + _size;
            }

            [[nodiscard]] constexpr inline const_iterator begin() const noexcept {
                return _moves.begin();
            }

            [[nodiscard]] constexpr inline const_iterator end() const noexcept {
                return _moves.begin() + _size;
            }

            [[nodiscard]] constexpr inline const_iterator cbegin() const noexcept {
                return _moves.begin();
            }

            [[nodiscard]] constexpr inline const_iterator cend() const noexcept {
                return _moves.begin() + _size;
            }

            // --- Operator += ---

            /**
             * Appends a single move to this list.
             * Asserts that capacity is not exceeded in debug builds.
             */
            constexpr inline MoveList<Capacity>& operator+=(const Move& m) noexcept {
                push_back(m);
                return *this;
            }

            /**
             * Appends all moves from another `MoveList` into this one.
             * Asserts that the combined size does not exceed capacity in debug builds.
             */
            template <std::size_t OtherCap>
            constexpr inline MoveList<Capacity>& operator+=(
                const MoveList<OtherCap>& other) noexcept {
                DEBUG_ASSERT(_size + other.size() <= Capacity);
                for (std::size_t i = 0; i < other.size(); ++i) _moves[_size++] = other[i];
                return *this;
            }

            // --- Operator + ---

            /** Returns a new list with the given move appended. */
            [[nodiscard]] constexpr MoveList<Capacity> operator+(const Move& m) const noexcept {
                MoveList result  = *this;
                result          += m;
                return result;
            }

            /** Returns a new list with all moves from `other` appended. */
            template <std::size_t OtherCap>
            [[nodiscard]] constexpr MoveList operator+(
                const MoveList<OtherCap>& other) const noexcept {
                MoveList result  = *this;
                result          += other;
                return result;
            }

        private:
            /** Storage container for the moves in the move list. */
            storage_type _moves;
            /** Number of slots filled in the move list. */
            std::size_t _size;
    };

}  // namespace aleph::chess

/// @cond INTERNAL
/**
 * Formats a `MoveList` as a space-separated sequence of UCI move strings,
 * e.g. "e2e4 d7d5 g1f3". Registered outside `aleph::chess` per fmtlib
 * specialization requirements.
 */
template <std::size_t Capacity>
struct fmt::formatter<aleph::chess::MoveList<Capacity>> {
        constexpr auto parse(fmt::format_parse_context& ctx) const { return ctx.begin(); }

        auto format(const aleph::chess::MoveList<Capacity>& ml, fmt::format_context& ctx) const {
            auto out = ctx.out();
            for (std::size_t i = 0; i < ml.size(); ++i) {
                if (i > 0) out = fmt::format_to(out, " ");
                out = fmt::format_to(out, "{}", ml[i]);
            }
            return out;
        }
};
/// @endcond