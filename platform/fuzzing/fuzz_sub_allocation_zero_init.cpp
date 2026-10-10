/**
 * @file fuzzing/fuzz_sub_allocation_zero_init.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <aleph/platform/allocation.hpp>

using namespace aleph::platform::allocation;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < sizeof(std::size_t)) return 0;

    std::size_t alloc_size;
    std::memcpy(&alloc_size, data, sizeof(std::size_t));
    alloc_size %= (1 << 20);

    // Align to int boundary
    alloc_size = (alloc_size / sizeof(int)) * sizeof(int);
    if (alloc_size == 0) return 0;

    static alignas(std::max_align_t) std::byte buf[1 << 20];

    // Fill with garbage to ensure zeroing is actually happening
    std::memset(buf, 0xFF, alloc_size);

    try {
        SubAllocation<int> sub(buf, alloc_size);

        // Every element must be zero regardless of prior buffer contents
        for (std::size_t i = 0; i < sub.getSize(); ++i) {
            if (sub[i] != 0) __builtin_trap();
        }
    } catch (const std::runtime_error&) {
        // Should not throw since we aligned the size above
        __builtin_trap();
    }

    return 0;
}