#ifndef INCLUDE_LIBQB_LIBQB_COMMON_H
#define INCLUDE_LIBQB_LIBQB_COMMON_H

/**
 * @file libqb-common.h
 * @brief Common platform and compiler detection macros for QB64-PE
 * 
 * This header should be included at the top of every .cpp file.
 * It provides platform and compiler detection macros for cross-platform compatibility.
 * 
 * @name Platform Detection Macros
 * @brief Macros identifying the target platform
 * 
 * - QB64_WINDOWS: Windows system
 * - QB64_LINUX: Linux system
 * - QB64_MACOSX: macOS system
 * - QB64_UNIX: Unix-flavored system (Linux, macOS, etc.)
 * 
 * @name Compiler Detection Macros
 * @brief Macros identifying the compiler
 * 
 * - QB64_MICROSOFT: Visual Studio compiler
 * - QB64_GCC: GCC compiler
 * - QB64_MINGW: MinGW compiler (set in addition to QB64_GCC)
 * 
 * @name System Configuration Macros
 * @brief Macros for system configuration
 * 
 * - QB64_BACKSLASH_FILESYSTEM: System uses \ for file paths (Windows)
 * - QB64_32: 32-bit system (default)
 * - QB64_64: 64-bit system
 * - QB64_NOT_X86: Not an x86/x64 architecture
 * - QB64_ARM: ARM architecture
 */
#ifdef WIN32
#    define QB64_WINDOWS
#    ifndef _WIN32_WINNT
// This supports Windows Vista and up
#        define _WIN32_WINNT 0x0600
#        define WINVER 0x0600
#    endif

#    define QB64_BACKSLASH_FILESYSTEM
#    ifdef _MSC_VER
// Do we even support non-mingw compilers on Windows?
#        define QB64_MICROSOFT
#    else
#        define QB64_GCC
#        define QB64_MINGW
#    endif
#elif defined(__APPLE__)
#    define QB64_MACOSX
#    define QB64_UNIX
#    define QB64_GCC
#elif defined(__linux__)
#    define QB64_LINUX
#    define QB64_UNIX
#    define QB64_GCC
#else
#    error "Unknown system; refusing to build. Edit os.h if needed"
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(QB64_MACOSX) || defined(__aarch64__)
#    define QB64_64
#else
#    define QB64_32
#endif

#if !defined(i386) && !defined(__x86_64__)
#    define QB64_NOT_X86
#    if defined(__arm__) || defined(__aarch64__) || defined(_M_ARM64)
#        define QB64_ARM
#    endif
#endif

/**
 * @name QB64 Boolean Constants
 * @brief Boolean values used in QB64
 */
///@{
#define QB_FALSE 0   ///< QB64 false value
#define QB_TRUE -1   ///< QB64 true value
///@}

/**
 * @brief Count of array elements
 * @param Array_ Array to count elements of
 * @return Number of elements in the array
 * @note Provides a safe way to get array size. Works for both C and C++.
 */
#ifndef _countof
#    ifdef __cplusplus
#        include <cstddef>

/**
 * @brief C++ template version of _countof
 * @tparam T Array element type
 * @tparam N Array size
 * @param Array Array reference
 * @return Number of elements
 */
template <typename T, size_t N> static inline constexpr size_t _countof(T const (&)[N]) noexcept {
    return N;
}
#    else
#        define _countof(Array_) (sizeof(Array_) / sizeof(Array_[0]))
#    endif
#endif

#endif
