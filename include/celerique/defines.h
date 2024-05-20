/*

File: ./include/celerique/defines.h
Author: Aldhinn Espinas
Description: This header file contains common preprocessor definitions.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_DEFINES_HEADER_FILE)
#define CELERIQUE_DEFINES_HEADER_FILE

// CELERIQUE_SHARED_SYMBOL Definition.
#if !defined(CELERIQUE_SHARED_SYMBOL)
// Dynamic libraries.
#if defined(CELERIQUE_ENGINE_LINKED_SHARED)

// MSVC Compilers.
#if defined(_MSC_VER)
// When building the shared library.
#if defined(CELERIQUE_ENGINE_BUILDING_SHARED)
/// @brief Shared library export symbol.
#define CELERIQUE_SHARED_SYMBOL __declspec(dllexport)
// Importing.
#else
/// @brief Shared library export symbol.
#define CELERIQUE_SHARED_SYMBOL __declspec(dllimport)
#endif
// End MSVC.

// Begin gcc or clang compilers.
#elif defined(__GNUC__) || defined(__clang__)
/// @brief Shared library export symbol.
#define CELERIQUE_SHARED_SYMBOL __attribute__((visibility("default")))
// END gcc or clang compilers.

#else
#error "Compiler currently unsupported."
#endif

// Static libraries.
#else
/// @brief Shared library export symbol.
#define CELERIQUE_SHARED_SYMBOL
#endif
#endif
// CELERIQUE_SHARED_SYMBOL Definition END.

// CELERIQUE_LEFT_BIT_SHIFT_1 Definition
#if !defined(CELERIQUE_LEFT_BIT_SHIFT_1)
/// @brief Bit shift binary 1 to the left by a specified number of places.
/// @param places How much to bit-shift.
#define CELERIQUE_LEFT_BIT_SHIFT_1(places) \
1 << places
#endif
// CELERIQUE_LEFT_BIT_SHIFT_1 Definition END.

// CELERIQUE_DEBUG_MODE Definition.
#if !defined(CELERIQUE_DEBUG_MODE)
#if defined(DEBUG) || defined(_DEBUG)
#define CELERIQUE_DEBUG_MODE
#endif
#endif
// CELERIQUE_DEBUG_MODE Definition END.

// If targeting a posix compliant system.
#if !defined(CELERIQUE_FOR_POSIX_SYSTEMS)
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || \
defined(__NetBSD__) || defined(__BSD__) || defined(BSD)
#define CELERIQUE_FOR_POSIX_SYSTEMS
#endif
#endif

// If targeting a BSD system in general.
#if !defined(CELERIQUE_FOR_BSD_SYSTEMS)
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__BSD__) || defined(BSD)
#define CELERIQUE_FOR_BSD_SYSTEMS
#endif
#endif

// If targeting FreeBSD.
#if !defined(CELERIQUE_FOR_FREEBSD)
#if defined(__FreeBSD__)
#define CELERIQUE_FOR_FREEBSD
#endif
#endif

// If targeting OpenBSD.
#if !defined(CELERIQUE_FOR_OPENBSD)
#if defined(__OpenBSD__)
#define CELERIQUE_FOR_OPENBSD
#endif
#endif

// If targeting NetBSD.
#if !defined(CELERIQUE_FOR_NETBSD)
#if defined(__NetBSD__)
#define CELERIQUE_FOR_NETBSD
#endif
#endif

// If targeting linux.
#if !defined(CELERIQUE_FOR_LINUX_SYSTEMS)
#if defined(__linux__)
#define CELERIQUE_FOR_LINUX_SYSTEMS
#endif
#endif

// If targeting windows.
#if !defined(CELERIQUE_FOR_WINDOWS)
#if defined(_WIN32) || defined(_WIN64)
#define CELERIQUE_FOR_WINDOWS
#endif
#endif

// If targeting android.
#if !defined(CELERIQUE_FOR_ANDROID)
#if defined(__ANDROID__)
#define CELERIQUE_FOR_ANDROID
#endif
#endif

// If targeting macos.
#if !defined(CELERIQUE_FOR_MACOS)
#if defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
#if defined(TARGET_OS_MAC) && !defined(TARGET_OS_IPHONE) && !defined(TARGET_IPHONE_SIMULATOR)
#define CELERIQUE_FOR_MACOS
#endif
#endif
#endif

// If targeting iOs.
#if !defined(CELERIQUE_FOR_IOS)
#if defined(__APPLE__) || defined(__MACH__)
#include <TargetConditionals.h>
#if defined(TARGET_OS_IPHONE)
#define CELERIQUE_FOR_IOS
#endif
#endif
#endif

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.