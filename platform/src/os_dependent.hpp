// NOLINTBEGIN
#pragma once

#include <boost/predef.h>

#if BOOST_OS_WINDOWS
    #include <intrin.h>
    #include <windows.h>
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