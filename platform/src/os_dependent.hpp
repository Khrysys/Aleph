/**
 * @file src/os_dependent.hpp
 *
 * Copyright (c) Aleph Engine Project
 * SPDX-License-Identifier: GPL-3.0-only
 */
// NOLINTBEGIN
#pragma once

#include <aleph/platform.hpp>

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
    #include <sys/mman.h>
    #include <unistd.h>
    #include <x86intrin.h>
#elif defined(ALEPH_OS_MACOS)
    #include <fstream>

    #include <sys/mman.h>
    #include <unistd.h>
#endif
// NOLINTEND