#pragma once

#include "../logging.hpp"
#include "init.hpp"

namespace aleph::platform::allocation {

    /**
     * Indicates whether an allocation was backed by large (huge) pages
     * or standard system pages.
     */
    enum class PageSize {
        Standard,
        Large,
    };

    /**
     * Result of a platform allocation.
     *
     * `size` reflects the caller-provided size, which must already be
     * rounded to the appropriate page boundary via `roundToPage`.
     * `page_size` reflects what was actually granted by the OS —
     * Large is preferred but Standard is the silent fallback.
     */
    struct AllocationResult {
        void*    ptr;
        size_t   size;
        PageSize page_size;
    };

    /**
     * Allocates `size` bytes of memory, preferring large page backing.
     *
     * Attempts large page allocation first. Falls back to standard pages
     * silently if the OS denies the request (e.g. insufficient privileges
     * on Windows, or huge pages unavailable on Linux). `AllocationResult::page_size`
     * reflects what was actually granted.
     *
     * @param size Number of bytes to allocate. Must be a multiple of
     *             `getPageSize()` — asserted in debug builds.
     * @return AllocationResult with ptr, size, and granted page size.
     *         ptr will be nullptr on total failure.
     */
    [[nodiscard]] AllocationResult allocate(size_t size) noexcept;

    /**
     * Releases memory previously returned by `allocate`.
     *
     * @param alloc The AllocationResult returned by `allocate`.
     *              No-op if ptr is nullptr.
     */
    void deallocate(AllocationResult alloc) noexcept;

    /**
     * Rounds `size` up to the nearest multiple of `page_size`.
     *
     * @param size      The requested allocation size in bytes.
     * @param page_size The page size boundary, typically from `getPageSize()`.
     * @return Rounded size, guaranteed to be a multiple of `page_size`.
     */
    [[nodiscard]] constexpr size_t roundToPage(size_t size, size_t page_size) noexcept {
        return (size + page_size - 1) & ~(page_size - 1);
    }

} // namespace aleph::platform

#include "runtime.inl"