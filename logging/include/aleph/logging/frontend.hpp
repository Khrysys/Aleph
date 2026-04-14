#include <atomic>
#include <array>

#include "log_entry.hpp"

template <size_t Capacity>
class Frontend {
public:
    Frontend() : write_(0), read_(0) {}

    void log(const char* msg, uint64_t ts, uint32_t tid) {
        size_t w = write_.load(std::memory_order_relaxed);
        size_t next = inc(w);

        // overwrite oldest if full
        if (next == read_.load(std::memory_order_acquire)) {
            read_.store(inc(read_.load(std::memory_order_relaxed)),
                        std::memory_order_release);
        }

        buffer_[w].set(msg, ts, tid);

        write_.store(next, std::memory_order_release);
    }

    bool pop(LogEntry& out) {
        size_t r = read_.load(std::memory_order_relaxed);

        if (r == write_.load(std::memory_order_acquire))
            return false;

        out = buffer_[r];
        read_.store(inc(r), std::memory_order_release);
        return true;
    }

private:
    static constexpr size_t inc(size_t i) {
        return (i + 1) % Capacity;
    }

    alignas(64) std::array<LogEntry, Capacity> buffer_;
    alignas(64) std::atomic<size_t> write_;
    alignas(64) std::atomic<size_t> read_;
};