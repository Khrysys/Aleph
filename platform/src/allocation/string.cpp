#include <aleph/platform.hpp>

namespace aleph::platform::allocation {
    thread_local std::array<std::size_t, detail::STRING_ARENA_TLS_SIZE> tlsCache{};
    thread_local std::size_t tlsCacheSize = 0;

    NUMAString::NUMAString(StringArena* arena, char* ptr, std::size_t idx)
        : arena(arena), ptr(ptr), filled(0), idx(idx) {}

    NUMAString::NUMAString(const NUMAString& other)
        : arena(other.arena), ptr(other.ptr), filled(other.filled), idx(other.idx) {
        arena->refs[idx]++;
    }

    NUMAString& NUMAString::operator=(const NUMAString& other) {
        if (this == &other) return *this;

        arena  = other.arena;
        ptr    = other.ptr;
        filled = other.filled;
        idx    = other.idx;

        arena->refs[idx]++;

        return *this;
    }

    NUMAString::~NUMAString() {
        if (arena) {
            arena->release(idx);
            arena = nullptr;
        }
    }

    StringArena::StringArena(Allocation* pAlloc)
        : alloc(pAlloc->getSubAllocation<char>(detail::STRING_ALLOCATION_SIZE)) {
        for (auto i = 0; i < detail::STRING_ARENA_SIZE; i++) {
            freeIndices[i] = i;
        }
        freeSize = detail::STRING_ARENA_SIZE;
    }

    NUMAString StringArena::allocate() {
        // 1. Fast path: TLS cache
        if (tlsCacheSize > 0) {
            std::size_t idx = tlsCache[--tlsCacheSize];
            refs[idx].store(1, std::memory_order_relaxed);

            return NUMAString(this, &alloc[idx * detail::STRING_BUFFER_SIZE], idx);
        }

        // 2. Global pool
        std::lock_guard<std::mutex> lock(freeMutex);

        if (freeSize == 0) {
            throw std::runtime_error("StringArena exhausted");
        }

        std::size_t idx = freeIndices[--freeSize];
        refs[idx].store(1, std::memory_order_relaxed);

        return NUMAString(this, &alloc[idx * detail::STRING_BUFFER_SIZE], idx);
    }

    void StringArena::release(std::size_t idx) {
        // decrement ownership
        int prev = refs[idx].fetch_sub(1, std::memory_order_acq_rel);

        if (prev != 1) {
            return;  // still referenced elsewhere
        }

        // Slot is now globally free — push to TLS first if possible
        if (tlsCacheSize < tlsCache.size()) {
            tlsCache[tlsCacheSize++] = idx;
            return;
        }

        // fallback: global free list
        std::lock_guard<std::mutex> lock(freeMutex);
        freeIndices[freeSize++] = idx;
    }
}  // namespace aleph::platform::allocation