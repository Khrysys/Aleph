/**
 * @file tests/fuzz_sub_allocation_alignment.cpp
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
    if (size < sizeof(std::size_t)) return 0;

    std::size_t count;
    std::memcpy(&count, data, sizeof(std::size_t));
    count %= 256;

    std::size_t page = Allocation::getPageSize();
    Allocation alloc(page * 4, 0);

    try {
        auto sub = alloc.getSubAllocation<int>(count);
        if (reinterpret_cast<std::uintptr_t>(&sub[0]) % alignof(int) != 0)
            __builtin_trap();
    } catch (const std::bad_alloc&) {}

    try {
        auto sub = alloc.getSubAllocation<double>(count);
        if (reinterpret_cast<std::uintptr_t>(&sub[0]) % alignof(double) != 0)
            __builtin_trap();
    } catch (const std::bad_alloc&) {}

    try {
        auto sub = alloc.getSubAllocation<std::max_align_t>(count);
        if (reinterpret_cast<std::uintptr_t>(&sub[0]) % alignof(std::max_align_t) != 0)
            __builtin_trap();
    } catch (const std::bad_alloc&) {}

    return 0;
}