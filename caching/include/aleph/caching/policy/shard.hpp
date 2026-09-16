#pragma once

#include <cstdint>

#include <aleph/platform.hpp>

#include "entry.hpp"

namespace aleph::caching::policy {
    class Shard {
        public:
            [[nodiscard]] Entry get(std::uint64_t key) {

            }

        private:
            SubAllocation
    };
}