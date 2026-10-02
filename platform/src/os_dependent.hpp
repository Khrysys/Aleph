/**
 * @file src/os_dependent.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
// NOLINTBEGIN
#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__TOS_WIN__) || \
    defined(__WINDOWS__)
    #define ALEPH_OS_WINDOWS
#elif defined(linux) || defined(__linux) || defined(__linux__) || defined(__gnu_linux__)
    #define ALEPH_OS_LINUX
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
    #define ALEPH_OS_MACOS
#endif

#if defined(ALEPH_OS_WINDOWS)
    #pragma warning(push, 0)
    #include <intrin.h>
    #include <windows.h>
    #pragma warning(pop)
#elif defined(ALEPH_OS_LINUX)
    #include <fstream>
    #include <numa.h>
    #include <numaif.h>
    #include <pthread.h>
    #include <sched.h>
    #include <unistd.h>
    #include <x86intrin.h>

    #include <sys/mman.h>
#elif defined(ALEPH_OS_MACOS)
    #include <fstream>
    #include <unistd.h>
    
    #include <sys/mman.h>
#endif
// NOLINTEND