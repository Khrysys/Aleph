/**
 * @file include/aleph/caching/mcts/tree.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <atomic>
#include <cstddef>

#include <libassert/assert.hpp>

#include <aleph/platform.hpp>

#include "entry.hpp"

namespace aleph::caching::mcts {
    class Tree {
        public:
            auto get(std::size_t idx) -> EntryGuard {
                auto* node = &table[idx];
                return node;
            }

            auto getNewEntry() {
                auto idx = size.fetch_add(1);
                return get(idx);
            }

        private:
            platform::SubAllocation<Entry> table;
            std::atomic<std::size_t> size = 0;
    };
}  // namespace aleph::caching::mcts