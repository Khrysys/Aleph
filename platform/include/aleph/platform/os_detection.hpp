/**
 * @file include/aleph/platform/os_detection.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__)
    #define ALEPH_OS_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
    #define ALEPH_OS_LINUX
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
    #define ALEPH_OS_MACOS
#endif

/**
 * Defined when BMI2 is available on the target platform.
 * Enables hardware-accelerated `pext` via `_pext_u64`.
 */
#ifdef __BMI2__
    #define ALEPH_HAS_BMI2
#endif