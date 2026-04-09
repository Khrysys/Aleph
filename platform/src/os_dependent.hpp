// NOLINTBEGIN
#pragma once

#include <boost/predef.h>

// Topology support. MacOS does not have anything to support NUMA or multiple nodes at all.
#if BOOST_OS_WINDOWS
    #include <windows.h>
#elif BOOST_OS_LINUX
    #include <numa.h>
    #include <numaif.h>
#endif

// Get OS-Specific headers for Large/huge pages support. Windows.h has the huge pages support, so
// that's handled by the topology support branches.
#if BOOST_OS_LINUX || BOOST_OS_MACOS
    #include <fstream>
    #include <unistd.h>

    #include <sys/mman.h>
#endif

// NOLINTEND