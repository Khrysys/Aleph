/**
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace aleph::platform::allocation {

    /**
     * Indicates whether an allocation was backed by large (huge) pages
     * or standard system pages.
     */
    enum class PageSize : uint8_t { Standard, Large };

    /**
     * Result of a platform allocation.
     *
     * `size` reflects the caller-provided size, which must already be
     * rounded to the appropriate page boundary via `roundToPage`.
     * `page_size` reflects what was actually granted by the OS —
     * Large is preferred but Standard is the silent fallback.
     */
    struct AllocationResult {
            /** Pointer to the allocated memory. nullptr on total failure. */
            void* ptr;
            /** Size of the allocation in bytes, as provided by the caller */
            size_t size;
            /** Page size actually granted by the OS. */
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
     * This function has undefined behavior if called before `aleph::platform::loggingInit()`.
     *
     * @param size Number of bytes to allocate. Must be a multiple of
     *             `getPageSize()` — asserted in debug builds.
     * @return AllocationResult with ptr, size, and granted page size.
     *         ptr will be nullptr on total failure.
     */
    [[nodiscard]] auto allocate(size_t size) -> AllocationResult;

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
    [[nodiscard]] constexpr auto roundToPage(size_t size, size_t page_size) noexcept -> size_t {
        return (size + page_size - 1) & ~(page_size - 1);
    }

}  // namespace aleph::platform::allocation

// NOLINTNEXTLINE
#include "runtime.inl"