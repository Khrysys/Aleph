#pragma once

#include <iomanip>
#include <iostream>

#include "compiler.hpp"
#include "intrinsics.hpp"

namespace aleph::platform {

    namespace detail {

        /** Column width for flag name alignment in diagnostic output. */
        constexpr int PRINT_WIDTH = 36;

        inline void print_flag(const char* name, bool value) {
            std::cout << std::left << std::setw(PRINT_WIDTH) << name << " | "
                      << (value ? "Yes" : "No") << '\n';
        }

        inline void print_section(const char* title) {
            std::cout << '\n' << title << '\n';
            std::cout << std::string(PRINT_WIDTH + 6, '-') << '\n';
        }
    }  // namespace detail

    /**
     * Prints compiler and language feature configuration to stdout.
     */
    inline void printCompilerConfig() {
        detail::print_section("Language / Feature Support");

        detail::print_flag("[[assume]]",
#if defined(ALEPH_HAS_ASSUME)
                           true
#else
                           false
#endif
        );
    }

    /**
     * Prints CPU intrinsics availability to stdout.
     */
    inline void printIntrinsicsConfig() {
        detail::print_section("CPU Intrinsics Configuration");

        detail::print_flag("x86intrin.h",
#if defined(ALEPH_HAS_X86INTRIN_H)
                           true
#else
                           false
#endif
        );
        detail::print_flag("intrin.h (MSVC)",
#if defined(ALEPH_HAS_INTRIN_H)
                           true
#else
                           false
#endif
        );

        detail::print_flag("BMI2 / PEXT",
#if defined(ALEPH_HAS_BMI2)
                           true
#else
                           false
#endif
        );
    }

    /**
     * Prints all platform diagnostic information to stdout.
     * Call after `loggingInit()` during engine startup.
     */
    inline void printPlatformConfig() {
        printCompilerConfig();
        printIntrinsicsConfig();
    }
}  // namespace aleph::platform