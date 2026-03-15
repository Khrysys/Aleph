#pragma once

namespace aleph::platform {

    inline void loggingInit() noexcept {
        if (detail::logging_ready) return;
        absl::InitializeLog();
        detail::logging_ready = true;
    }

    inline bool isLoggingReady() noexcept {
        return detail::logging_ready;
    }

} // namespace aleph::platform