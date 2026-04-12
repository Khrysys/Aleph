#pragma once

#include <cstdint>
#include <limits>

#include "backend.hpp"

#ifndef ALEPH_LOGGING_MINIMUM_IMPORTANCE
    #define ALEPH_LOGGING_MINIMUM_IMPORTANCE 0
#endif

namespace aleph::logging {
    namespace detail {
        using ImportanceType                      = std::int8_t;
        constexpr ImportanceType IMPORTANCE_LEVEL = ALEPH_LOGGING_MINIMUM_IMPORTANCE;
    }  // namespace detail

    enum MessageImportance : detail::ImportanceType {
        TRACE    = -1,
        DEBUG    = 0,
        INFO     = 1,
        NOTICE   = 2,
        WARNING  = 10,
        ERROR    = 100,
        CRITICAL = std::numeric_limits<detail::ImportanceType>::max() - 1
    };

    class Frontend {
        private:
            Backend* backend;
    };

}  // namespace aleph::logging