/**
 * @file tests/fuzz_sub_allocation_construction.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

#include <aleph/platform/allocation/base.hpp>

using namespace aleph::platform::allocation;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < sizeof(std::size_t)) {
        return 0;
    }

    std::size_t alloc_size;
    std::memcpy(&alloc_size, data, sizeof(std::size_t));
    alloc_size %= (1 << 20);

    static unsigned char buf[1 << 20];

    try {
        SubAllocation<int> sub(reinterpret_cast<std::byte*>(buf), alloc_size);
        if (alloc_size % sizeof(int) != 0) {
            __builtin_trap();
        }
        if (sub.getSize() != alloc_size / sizeof(int)) {
            __builtin_trap();
        }
    } catch (const std::runtime_error&) {
        if (alloc_size % sizeof(int) == 0) {
            __builtin_trap();
        }
    }

    return 0;
}