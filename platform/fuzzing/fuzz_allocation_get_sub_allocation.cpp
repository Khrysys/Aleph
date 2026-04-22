/**
 * @file tests/fuzz_get_sub_allocation.cpp
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
    if (size < sizeof(std::size_t)) {
        return 0;
    }

    std::size_t count;
    std::memcpy(&count, data, sizeof(std::size_t));

    std::size_t page = Allocation::getPageSize();

    try {
        Allocation alloc(page, 0);
        auto sub = alloc.getSubAllocation<std::byte>(count % (page * 2));
        if (sub.getSize() != count % (page * 2)) {
            __builtin_trap();
        }
    } catch (const std::bad_alloc&) {
    }

    return 0;
}