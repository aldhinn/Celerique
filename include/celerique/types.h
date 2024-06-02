/*

File: ./include/celerique/types.h
Author: Aldhinn Espinas
Description: This header file contains common types.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_TYPES_HEADER_FILE)
#define CELERIQUE_TYPES_HEADER_FILE

#include <stdint.h>
#include <stddef.h>

#if !defined(__cplusplus)
/// @brief Boolean data type definition.
typedef enum bool {
    false = 0,
    true = 1
} bool;
#endif

/// @brief The type for a pointer container.
typedef uintptr_t CeleriquePointer;
/// @brief The type for the number of pixel units in the screen.
typedef int32_t CeleriquePixelUnits;
/// @brief Type for a byte character.
typedef char CeleriqueByte;
/// @brief A value of this type describes the size of a stack allocated array.
typedef uint8_t CeleriqueArraySize;

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.