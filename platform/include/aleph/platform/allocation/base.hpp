#pragma once

#include <atomic>
#include <cstddef>

#include <libassert/assert.hpp>

namespace aleph::platform::allocation {
    template <typename T>
    class SubAllocation {
        public:
            SubAllocation(void* p, std::size_t s) : ptr(reinterpret_cast<T*>(p)), size(s) {
                if (size % sizeof(T) != 0) {
                    throw std::runtime_error(
                        std::string(
                            "Size of type for SubAllocation did not divide size of SubAllocation! "
                            "(Allocated Size ") +
                        std::to_string(size) + " | Must divide " + std::to_string(sizeof(T)) + ")");
                }
            }

            auto operator[](std::size_t idx) -> T& {
                DEBUG_ASSERT(idx < getSize());
                return ptr[idx];
            }

            auto getSize() const { return size / sizeof(T); }

        private:
            T* ptr;
            std::size_t size;
    };

    class Allocation {
        public:
            Allocation(std::size_t requestedSize, std::size_t numaNode);

            Allocation(const Allocation& other) = delete;
            Allocation(Allocation&& other) {
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
            auto operator=(Allocation&& other) -> Allocation& {
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

            static auto areLargePagesAvailable() -> bool;
            static auto getPageSize() -> std::size_t;

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
                        return SubAllocation<T>(static_cast<T*>(ptr) + aligned, bytes);
                    }
                }
            }

            ~Allocation();

        private:
            void* ptr;
            std::size_t numaNode;
            std::size_t size;
            std::atomic<std::size_t> filled;
    };
}  // namespace aleph::platform::allocation