/**
 * @file include/aleph/platform/allocation.hpp
 * @copyright Aleph Engine Project
 * 
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <atomic>
#include <cstddef>
#include <string>
#include <stdexcept>

#include <libassert/assert.hpp>

namespace aleph::platform {
    /**
     * @brief A chunk of dynamic memory from a larger Allocation.
     * 
     * `SubAllocation` is created via `Allocation::getSubAllocation` and provides
     * indexed access to a contiguous block of `T` elements carved from a larger
     * `Allocation`. No allocation or deallocation occurs — the backing memory is
     * owned by the parent `Allocation`. Data is not zeroed on creation or destruction;
     * the caller is responsible for managing element lifetimes.
     *
     * @tparam T Element type. Must be trivially copyable.
     */
    template <typename T>
    class SubAllocation {
        public:
            /**
             * @brief Constructs a `SubAllocation` over an existing memory region.
             *
             * @param p Pointer to the start of the memory region.
             * @param s Size of the region in bytes. Must be a multiple of `sizeof(T)`.
             * @throws std::runtime_error if `s` is not divisible by `sizeof(T)`.
             */
            SubAllocation(void* p, std::size_t s) : ptr(reinterpret_cast<T*>(p)), size(s) {
                if (size % sizeof(T) != 0) {
                    throw std::runtime_error(
                        std::string(
                            "Size of type for SubAllocation did not divide size of SubAllocation! "
                            "(Allocated Size ") +
                        std::to_string(size) + " | Must divide " + std::to_string(sizeof(T)) + ")");
                }
            }

            /**
             * @brief Returns a reference to the element at `idx`.
             *
             * @param idx Zero-based index into the allocation. Asserted to be
             *            within bounds in debug builds.
             */
            auto operator[](std::size_t idx) -> T& {
                DEBUG_ASSERT(idx < getSize());
                return ptr[idx];
            }

            /**
             * @brief Returns the number of `T` elements in this `SubAllocation`.
             */
            auto getSize() const noexcept { return size / sizeof(T); }

        private:
            T* ptr;
            std::size_t size;
    };


    /**
     * A single large contiguous memory allocation, optionally backed by huge pages.
     *
     * `Allocation` acquires a block of memory from the OS on construction and
     * releases it on destruction. Sub-regions are carved out via `getSubAllocation`,
     * which uses a lock-free atomic bump allocator. The allocation is not resizable
     * and sub-regions are never individually freed — the entire block is released
     * when the `Allocation` is destroyed.
     *
     * Copy construction and copy assignment are explicitly deleted. Move construction
     * and move assignment transfer ownership of the underlying memory.
     */
    class Allocation {
        public:
            /**
             * @brief Constructs an `Allocation` of at least `requestedSize` bytes on the
             * given NUMA node, rounded up to the system page size.
             *
             * Attempts huge page backing first, falling back to standard pages
             * silently if unavailable.
             *
             * @param requestedSize Minimum number of bytes to allocate.
             * @param numaNode      NUMA node to allocate memory on.
             * @throws std::bad_alloc if the OS allocation fails entirely.
             */
            Allocation(std::size_t requestedSize, std::size_t numaNode);

            Allocation(const Allocation& other) = delete;

            /**
             * Move constructs an `Allocation`, transferring ownership of the
             * underlying memory. The moved-from `Allocation` is left in a valid
             * but empty state with a null pointer.
             */
            Allocation(Allocation&& other) noexcept {
                ptr            = other.ptr;
                numaNode       = other.numaNode;
                size           = other.size;
                filled.store(other.filled.load());

                other.ptr      = nullptr;
                other.numaNode = 0;
                other.size     = 0;
                other.filled   = 0;
            }
            auto operator=(const Allocation& other) -> Allocation& = delete;

            /**
             * Move assigns an `Allocation`, transferring ownership of the
             * underlying memory. The moved-from `Allocation` is left in a valid
             * but empty state with a null pointer.
             */
            auto operator=(Allocation&& other) noexcept -> Allocation& {
                ptr            = other.ptr;
                numaNode       = other.numaNode;
                size           = other.size;
                filled.store(other.filled.load());

                other.ptr      = nullptr;
                other.numaNode = 0;
                other.size     = 0;
                other.filled   = 0;

                return *this;
            }

            /**
             * @brief Returns true if large/huge pages are available on this system.
             * Result is cached after the first call.
             */
            static auto areLargePagesAvailable() -> bool;

            /**
             * @brief Returns the system page size in bytes, preferring the large page
             * size where available. Result is cached after the first call.
             */
            static auto getPageSize() -> std::size_t;

            /**
             * @brief Carves out a sub-region of `count` elements of type `T` from this
             * `Allocation` using a lock-free atomic bump allocator.
             *
             * The returned `SubAllocation` points into the backing memory of this
             * `Allocation` and must not outlive it. Alignment of `T` is respected
             * automatically.
             *
             * @tparam T    Element type to allocate for.
             * @param count Number of elements of type `T` to allocate.
             * @throws std::bad_alloc if insufficient space remains.
             */
            template <typename T>
            auto getSubAllocation(std::size_t count) -> SubAllocation<T> {
                std::size_t bytes = count * sizeof(T);
                std::size_t align = alignof(T);

                std::size_t old   = filled.load(std::memory_order_relaxed);

                while (true) {
                    std::size_t aligned = (old + align - 1) & ~(align - 1);
                    std::size_t next    = aligned + bytes;

                    if (next > size) {
                        throw std::bad_alloc();
                    }

                    if (filled.compare_exchange_weak(old, next, std::memory_order_acq_rel,
                                                     std::memory_order_relaxed)) {
                        return SubAllocation<T>(reinterpret_cast<T*>(static_cast<std::byte*>(ptr) + aligned), bytes);
                    }
                }
            }

            /**
             * @brief Releases the underlying memory back to the OS.
             */
            ~Allocation();

        private:
            void* ptr;
            std::size_t numaNode;
            std::size_t size;
            std::atomic<std::size_t> filled;
    };
}  // namespace aleph::platform