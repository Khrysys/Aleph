/**
 * @file tests/fuzz_sub_allocation_double.cpp
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

extern "C" auto LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) -> int {
    if (size < sizeof(std::size_t)) {return 0;}

    std::size_t count;
    std::memcpy(&count, data, sizeof(std::size_t));
    count %= 1024;

    alignas(alignof(double)) static std::byte buf[sizeof(double) * 1024];
    std::size_t alloc_size = count * sizeof(double);

    try {
        SubAllocation<double> sub(buf, alloc_size);
        if (sub.getSize() != count) {
            __builtin_trap();
        }
        for (std::size_t i = 0; i < sub.getSize(); ++i) {
            sub[i] = static_cast<double>(i);
            if (sub[i] != static_cast<double>(i)) {
                __builtin_trap();
            }
        }
    } catch (const std::runtime_error&) {
        if (alloc_size % sizeof(double) == 0) {
            __builtin_trap();
        }
    }

    return 0;
}