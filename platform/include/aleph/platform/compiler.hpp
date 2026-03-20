/**
 * @file include/aleph/platform/compiler.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#if defined(__cpp_lib_constexpr_string) && __cpp_lib_constexpr_string >= 201907L
    #define ALEPH_CONSTEXPR_STRING
#endif