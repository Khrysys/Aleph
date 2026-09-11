// NOLINTBEGIN
#pragma once

#include <boost/predef.h>

#if BOOST_OS_WINDOWS
    #pragma warning(push, 0)
    #include <intrin.h>
    #include <windows.h>
    #pragma warning(pop)
#elif BOOST_OS_LINUX
    #include <numa.h>
    #include <numaif.h>
    #include <pthread.h>
    #include <sched.h>
    #include <fstream>

    #include <sys/mman.h>
    #include <x86intrin.h>
    #include <unistd.h>
#endif
// NOLINTEND