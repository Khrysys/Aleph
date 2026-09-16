/**
 * @file include/aleph/platform/threads.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <cstdint>
#include <cstddef>
#include <thread>

namespace aleph::platform {
    [[nodiscard]] auto bindThread(std::uint32_t numa_node, std::uint32_t core_id) noexcept -> bool;

    template <typename Fn, typename... Args>
    [[nodiscard]]
    auto createThread(std::size_t node, std::size_t core, Fn&& fn, Args&&... args) -> std::jthread;
}  // namespace aleph::platform