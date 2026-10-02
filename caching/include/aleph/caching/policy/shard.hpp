/**
 * @file include/aleph/caching/policy/shard.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>

#include <aleph/platform.hpp>

#include "entry.hpp"

namespace aleph::caching::policy {
    class Shard {
        public:
            Shard(platform::SubAllocation<EntryContainer> alloc, std::uint64_t maxDelta) : data(alloc), maxDelta(maxDelta) { }

            [[nodiscard]] auto get(std::uint64_t key) const -> std::optional<Entry> {
                const auto size = data.getSize();
                const auto idx = platform::hi_mul64(key, size);

                for (std::size_t delta = 0; delta < size; ++delta) {
                    Entry entry;
                    const auto realIdx = (idx + delta) % size;

                    {
                        auto ref = std::atomic_ref(data[realIdx].version);
                        uint64_t v1;
                        uint64_t v2;
                        do {
                            v1 = ref.load(std::memory_order_acquire);
                            if((v1 & 1) == 0) {
                                continue;
                            }

                            entry = data[realIdx].entry;
                            v2 = ref.load(std::memory_order_acquire);
                        } while(v1 != v2);
                    }

                    if (entry.hash == 0) [[unlikely]] {
                        return std::nullopt;
                    }

                    if (entry.hash == key) [[unlikely]] {
                        return entry;
                    }

                    const auto otherIdx = platform::hi_mul64(entry.hash, size);
                    const auto otherDelta = (realIdx + size - otherIdx) % size;

                    if (otherDelta < delta) [[unlikely]] {
                        return std::nullopt;
                    }
                }

                return std::nullopt;
            }

            void put(std::uint64_t key, Entry entry) {
                const auto size = data.getSize();
                const auto idx = platform::hi_mul64(key, size);

                for (std::size_t delta = 0; delta < size; ++delta) {
                    const auto realIdx = (idx + delta) % size;
                    {
                        auto ref = std::atomic_ref(data[realIdx].version);
                        std::uint64_t v1;
                        do {
                            v1 = ref.load(std::memory_order_acquire);
                            if((v1 & 1) == 0) {
                                continue;
                            }
                        } while(ref.compare_exchange_weak(v1, v1+1));
                    }
                    Entry entry = data[realIdx].entry;
                    
                    if (entry.moves = ) {
                    }

                }
            }

        private:
            platform::SubAllocation<EntryContainer> data;
            std::uint64_t maxDelta;
    };
}