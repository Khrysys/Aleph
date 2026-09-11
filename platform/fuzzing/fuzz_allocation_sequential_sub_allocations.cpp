/**
 * @file tests/fuzz_sequential_sub_allocations.cpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

#include <aleph/platform/allocation/base.hpp>

using namespace aleph::platform::allocation;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size < sizeof(std::size_t)) {
        return 0;
    }

    std::size_t page = Allocation::getPageSize();
    Allocation alloc(page * 16, 0);

    const std::size_t n = size / sizeof(std::size_t);
    std::vector<SubAllocation<std::byte>> subs;

    for (std::size_t i = 0; i < n; ++i) {
        std::size_t count;
        std::memcpy(&count, data + (i * sizeof(std::size_t)), sizeof(std::size_t));
        count %= page * 16;

        try {
            subs.push_back(alloc.getSubAllocation<std::byte>(count));
        } catch (const std::bad_alloc&) {
            break;
        }
    }

    // Verify no two suballocations overlap
    for (std::size_t i = 0; i < subs.size(); ++i) {
        for (std::size_t j = i + 1; j < subs.size(); ++j) {
            const std::byte* a_begin = &subs[i][0];
            const std::byte* a_end   = a_begin + subs[i].getSize();
            const std::byte* b_begin = &subs[j][0];
            const std::byte* b_end   = b_begin + subs[j].getSize();
            if (a_end > b_begin && b_end > a_begin) {
                __builtin_trap();
            }
        }
    }

    return 0;
}