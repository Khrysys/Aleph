/**
 * @file include/aleph/chess/move_list.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

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
            using iterator       = storage_type::iterator;
            /** Constant iterator type, used for range-for loop support. */
            using const_iterator = storage_type::const_iterator;

            /** Constructs an empty move list. */
            constexpr MoveList() noexcept = default;

            // --- Capacity / size ---

            /** Returns the number of moves currently in the list. */
            [[nodiscard]] constexpr auto size() const noexcept -> std::size_t { return _size; }

            /** Returns true if the list contains no moves. */
            [[nodiscard]] constexpr auto empty() const noexcept -> bool { return _size == 0; }

            /** Returns the maximum number of moves this list can hold. */
            [[nodiscard]] constexpr static auto capacity() noexcept -> std::size_t { return Capacity; }

            // --- Element access ---

            /** Returns a reference to the move at index `i`. Asserts bounds in debug builds. */
            [[nodiscard]] constexpr auto operator[](std::size_t index) noexcept -> Move& {
                DEBUG_ASSERT(index < _size);
                return _moves[index];
            }

            /** Returns a const reference to the move at index `i`. Asserts bounds in debug builds.
             */
            [[nodiscard]] constexpr auto operator[](std::size_t index) const noexcept -> const Move& {
                DEBUG_ASSERT(index < _size);
                return _moves[index];
            }

            /**
             * Returns true if the list contains the given move.
             * Comparison is performed via `uint16_t` conversion.
             */
            [[nodiscard]] constexpr auto contains(const Move& move) const noexcept -> bool {
                for (std::size_t i = 0; i < _size; ++i) {
                    if (static_cast<uint16_t>(_moves[i]) == static_cast<uint16_t>(move)) {
                        return true;
                    }
                }
                return false;
            }

            // --- Modifiers ---

            /** Clears the list without deallocating storage. */
            constexpr void clear() noexcept { _size = 0; }

            /**
             * Appends a move to the list.
             * Asserts that capacity is not exceeded in debug builds.
             */
            constexpr void push_back(const Move& move) noexcept {
                DEBUG_ASSERT(_size < Capacity);
                _moves[_size++] = move;
            }

            // --- Iteration ---

            /** Accessor method to MoveList._moves.begin(). */
            [[nodiscard]] constexpr auto begin() noexcept -> iterator { return _moves.begin(); }

            /** Accessor method to MoveList._moves.end(). */
            [[nodiscard]] constexpr auto end() noexcept -> iterator { return _moves.begin() + _size; }

            /** Accessor method to MoveList._moves.begin(). */
            [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator { return _moves.begin(); }

            /** Accessor method to MoveList._moves.end(). */
            [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
                return _moves.begin() + _size;
            }

            /** Accessor method to MoveList._moves.cbegin(). */
            [[nodiscard]] constexpr auto cbegin() const noexcept -> const_iterator {
                return _moves.begin();
            }

            /** Accessor method to MoveList._moves.cend(). */
            [[nodiscard]] constexpr auto cend() const noexcept -> const_iterator {
                return _moves.begin() + _size;
            }

            // --- Operator += ---

            /**
             * Appends a single move to this list.
             * Asserts that capacity is not exceeded in debug builds.
             */
            constexpr auto operator+=(const Move& move) noexcept -> MoveList<Capacity>& {
                push_back(move);
                return *this;
            }

            /**
             * Appends all moves from another `MoveList` into this one.
             * Asserts that the combined size does not exceed capacity in debug builds.
             */
            template <std::size_t OtherCap>
            constexpr auto operator+=(const MoveList<OtherCap>& other) noexcept -> MoveList<Capacity>& {
                DEBUG_ASSERT(_size + other.size() <= Capacity);
                for (std::size_t i = 0; i < other.size(); ++i) {
                    _moves[_size++] = other[i];
                }
                return *this;
            }

            // --- Operator + ---

            /** Returns a new list with the given move appended. */
            [[nodiscard]] constexpr auto operator+(const Move& move) const noexcept -> MoveList<Capacity> {
                MoveList result  = *this;
                result          += move;
                return result;
            }

            /** Returns a new list with all moves from `other` appended. */
            template <std::size_t OtherCap>
            [[nodiscard]] constexpr auto operator+(
                const MoveList<OtherCap>& other) const noexcept -> MoveList<Capacity> {
                MoveList result  = *this;
                result          += other;
                return result;
            }

        private:
            /** Storage container for the moves in the move list. */
            storage_type _moves;
            /** Number of slots filled in the move list. */
            std::size_t _size = 0;
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

        auto format(const aleph::chess::MoveList<Capacity>& moveList,
                    fmt::format_context& ctx) const {
            auto out = ctx.out();
            for (std::size_t i = 0; i < moveList.size(); ++i) {
                if (i > 0) {
                    out = fmt::format_to(out, " ");
                }
                out = fmt::format_to(out, "{}", moveList[i]);
            }
            return out;
        }
};
/// @endcond