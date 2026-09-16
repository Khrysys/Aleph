#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

#include "../common/types.hpp"

namespace aleph::caching::mcts {
    struct alignas(64) Entry {
        public:
            std::array<std::uint32_t, common::MAX_POLICY_SIZE> visits;
            std::array<std::uint32_t, common::MAX_POLICY_SIZE> edges;
            std::array<float, common::MAX_POLICY_SIZE> values;
            std::uint32_t totalVisits;
            float overallValue;
    };

    static_assert(sizeof(Entry) == 128);

    struct EntryGuard {
        public:
            EntryGuard(Entry* entry) : entry(entry) {}

            auto getVisits(std::size_t idx) { return std::atomic_ref(entry->visits[idx]); }
            auto getEdges(std::size_t idx) { return std::atomic_ref(entry->edges[idx]); }
            auto getValues(std::size_t idx) { return std::atomic_ref(entry->values[idx]); }

            auto getTotalVisits() { return std::atomic_ref(entry->totalVisits); }
            auto getOverallValue() { return std::atomic_ref(entry->overallValue); }

        private:
            Entry* entry;
    };
}  // namespace aleph::caching::mcts