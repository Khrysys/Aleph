#include <cstdint>
#include <type_traits>

#include <aleph/platform.hpp>

#include "os_dependent.hpp"

/**
 * Defined when BMI2 is available on the target platform.
 * Enables hardware-accelerated `pext` via `_pext_u64`.
 */
#ifdef __BMI2__
    #define ALEPH_HAS_BMI2
#endif

namespace aleph::platform {
    constexpr auto pext(std::uint64_t src, std::uint64_t mask) noexcept -> std::uint64_t {
        if (std::is_constant_evaluated()) {
            return detail::pext(src, mask);
        }
#if defined(ALEPH_HAS_BMI2) && (defined(ALEPH_HAS_X86INTRIN_H) || defined(ALEPH_HAS_INTRIN_H))
        return _pext_u64(src, mask);
#else
        return detail::pext(src, mask);
#endif
    }

    constexpr auto hi_mul64(std::uint64_t lhs, std::uint64_t rhs) -> std::uint64_t {
        if (std::is_constant_evaluated()) {
            return detail::hi_mul64(lhs, rhs);
        }
#if BOOST_OS_WINDOWS
        std::uint64_t highResult;
        // NOLINTNEXTLINE(readability-const-return-type)
        std::uint64_t lowResult = _umul128(lhs, rhs, &highResult);
        (void)lowResult;
        return highResult;
#elif defined(__SIZEOF_INT128__)
        return (static_cast<__uint128_t>(lhs) * static_cast<__uint128_t>(rhs)) >> 64;
#else
        return detail::hi_mul64(lhs, rhs);
#endif
    }
}  // namespace aleph::platform