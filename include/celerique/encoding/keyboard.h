/*

File: ./include/celerique/encoding/keyboard.h
Author: Aldhinn Espinas
Description: This header file contains encoding definitions of
keyboard key codes.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_ENCODING_KEYBOARD_HEADER_FILE)
#define CELERIQUE_ENCODING_KEYBOARD_HEADER_FILE

#include <celerique/types.h>

/// @brief The encoding of a key pressed in the keyboard.
typedef uint8_t CeleriqueKeyCode;

/// @brief The null value for type `CeleriqueKeyCode`.
#define CELERIQUE_KEYBOARD_KEY_NULL                                 0x00

/// @brief Ordinary keyboard key for the numeral '0'.
#define CELERIQUE_KEYBOARD_KEY_0                                    0x30
/// @brief Ordinary keyboard key for the numeral '1'.
#define CELERIQUE_KEYBOARD_KEY_1                                    0x31
/// @brief Ordinary keyboard key for the numeral '2'.
#define CELERIQUE_KEYBOARD_KEY_2                                    0x32
/// @brief Ordinary keyboard key for the numeral '3'.
#define CELERIQUE_KEYBOARD_KEY_3                                    0x33
/// @brief Ordinary keyboard key for the numeral '4'.
#define CELERIQUE_KEYBOARD_KEY_4                                    0x34
/// @brief Ordinary keyboard key for the numeral '5'.
#define CELERIQUE_KEYBOARD_KEY_5                                    0x35
/// @brief Ordinary keyboard key for the numeral '6'.
#define CELERIQUE_KEYBOARD_KEY_6                                    0x36
/// @brief Ordinary keyboard key for the numeral '7'.
#define CELERIQUE_KEYBOARD_KEY_7                                    0x37
/// @brief Ordinary keyboard key for the numeral '8'.
#define CELERIQUE_KEYBOARD_KEY_8                                    0x38
/// @brief Ordinary keyboard key for the numeral '9'.
#define CELERIQUE_KEYBOARD_KEY_9                                    0x39

/// @brief Keyboard key for the numpad key '0'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_0                             0x61
/// @brief Keyboard key for the numpad key '1'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_1                             0x62
/// @brief Keyboard key for the numpad key '2'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_2                             0x63
/// @brief Keyboard key for the numpad key '3'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_3                             0x64
/// @brief Keyboard key for the numpad key '4'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_4                             0x65
/// @brief Keyboard key for the numpad key '5'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_5                             0x66
/// @brief Keyboard key for the numpad key '6'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_6                             0x67
/// @brief Keyboard key for the numpad key '7'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_7                             0x68
/// @brief Keyboard key for the numpad key '8'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_8                             0x69
/// @brief Keyboard key for the numpad key '9'.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_9                             0x6A
/// @brief Keyboard key for the numpad lock key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_LOCK                          0x6B
/// @brief Keyboard key for the numpad space key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_SPACE                         0x6C
/// @brief Keyboard key for the numpad tab key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_TAB                           0x6D
/// @brief Keyboard key for the numpad enter key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_ENTER                         0x6E
/// @brief Keyboard key for the numpad equal key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_EQUAL                         0x6F
/// @brief Keyboard key for the numpad multiply key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_MULTIPLY                      0x70
/// @brief Keyboard key for the numpad plus key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_PLUS                          0x71
/// @brief Keyboard key for the numpad minus key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_MINUS                         0x72
/// @brief Keyboard key for the numpad divide key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_DIVIDE                        0x73
/// @brief Keyboard key for the numpad decimal key.
#define CELERIQUE_KEYBOARD_KEY_NUMPAD_DECIMAL                       0x74

/// @brief Keyboard key for the letter 'A'.
#define CELERIQUE_KEYBOARD_KEY_A                                    0x41
/// @brief Keyboard key for the letter 'B'.
#define CELERIQUE_KEYBOARD_KEY_B                                    0x42
/// @brief Keyboard key for the letter 'C'.
#define CELERIQUE_KEYBOARD_KEY_C                                    0x43
/// @brief Keyboard key for the letter 'D'.
#define CELERIQUE_KEYBOARD_KEY_D                                    0x44
/// @brief Keyboard key for the letter 'E'.
#define CELERIQUE_KEYBOARD_KEY_E                                    0x45
/// @brief Keyboard key for the letter 'F'.
#define CELERIQUE_KEYBOARD_KEY_F                                    0x46
/// @brief Keyboard key for the letter 'G'.
#define CELERIQUE_KEYBOARD_KEY_G                                    0x47
/// @brief Keyboard key for the letter 'H'.
#define CELERIQUE_KEYBOARD_KEY_H                                    0x48
/// @brief Keyboard key for the letter 'I'.
#define CELERIQUE_KEYBOARD_KEY_I                                    0x49
/// @brief Keyboard key for the letter 'J'.
#define CELERIQUE_KEYBOARD_KEY_J                                    0x4A
/// @brief Keyboard key for the letter 'K'.
#define CELERIQUE_KEYBOARD_KEY_K                                    0x4B
/// @brief Keyboard key for the letter 'L'.
#define CELERIQUE_KEYBOARD_KEY_L                                    0x4C
/// @brief Keyboard key for the letter 'M'.
#define CELERIQUE_KEYBOARD_KEY_M                                    0x4D
/// @brief Keyboard key for the letter 'N'.
#define CELERIQUE_KEYBOARD_KEY_N                                    0x4E
/// @brief Keyboard key for the letter 'O'.
#define CELERIQUE_KEYBOARD_KEY_O                                    0x4F
/// @brief Keyboard key for the letter 'P'.
#define CELERIQUE_KEYBOARD_KEY_P                                    0x50
/// @brief Keyboard key for the letter 'Q'.
#define CELERIQUE_KEYBOARD_KEY_Q                                    0x51
/// @brief Keyboard key for the letter 'R'.
#define CELERIQUE_KEYBOARD_KEY_R                                    0x52
/// @brief Keyboard key for the letter 'S'.
#define CELERIQUE_KEYBOARD_KEY_S                                    0x53
/// @brief Keyboard key for the letter 'T'.
#define CELERIQUE_KEYBOARD_KEY_T                                    0x54
/// @brief Keyboard key for the letter 'U'.
#define CELERIQUE_KEYBOARD_KEY_U                                    0x55
/// @brief Keyboard key for the letter 'V'.
#define CELERIQUE_KEYBOARD_KEY_V                                    0x56
/// @brief Keyboard key for the letter 'W'.
#define CELERIQUE_KEYBOARD_KEY_W                                    0x57
/// @brief Keyboard key for the letter 'X'.
#define CELERIQUE_KEYBOARD_KEY_X                                    0x58
/// @brief Keyboard key for the letter 'Y'.
#define CELERIQUE_KEYBOARD_KEY_Y                                    0x59
/// @brief Keyboard key for the letter 'Z'.
#define CELERIQUE_KEYBOARD_KEY_Z                                    0x5A

/// @brief Keyboard key for escape.
#define CELERIQUE_KEYBOARD_KEY_ESC                                  0x1B
/// @brief Keyboard key for tab.
#define CELERIQUE_KEYBOARD_KEY_TAB                                  0x09
/// @brief Keyboard key for the caps lock.
#define CELERIQUE_KEYBOARD_KEY_CAPS_LOCK                            0x21
/// @brief Keyboard key for the left shift key.
#define CELERIQUE_KEYBOARD_KEY_LEFT_SHIFT                           0x0E
/// @brief Keyboard key for the left control key.
#define CELERIQUE_KEYBOARD_KEY_LEFT_CONTROL                         0x11
/// @brief Keyboard key for the left alt key.
#define CELERIQUE_KEYBOARD_KEY_LEFT_ALT                             0x12
/// @brief Keyboard key for the space bar.
#define CELERIQUE_KEYBOARD_KEY_SPACE_BAR                            0x20
/// @brief Keyboard key for the right alt key.
#define CELERIQUE_KEYBOARD_KEY_RIGHT_ALT                            0x13
/// @brief Keyboard key for the right control key.
#define CELERIQUE_KEYBOARD_KEY_RIGHT_CONTROL                        0x14
/// @brief Keyboard key for the right shift key.
#define CELERIQUE_KEYBOARD_KEY_RIGHT_SHIFT                          0x0F
/// @brief Keyboard key for the enter key.
#define CELERIQUE_KEYBOARD_KEY_ENTER                                0x0D
/// @brief Keyboard key for backspace.
#define CELERIQUE_KEYBOARD_KEY_BACKSPACE                            0x08
/// @brief Keyboard key for the delete key.
#define CELERIQUE_KEYBOARD_KEY_DELETE                               0x7F

/// @brief Keyboard key for grave accent.
#define CELERIQUE_KEYBOARD_KEY_GRAVE_ACCENT                         0x60
/// @brief Keyboard key for hyphen or minus.
#define CELERIQUE_KEYBOARD_KEY_HYPHEN_OR_MINUS                      0x2D
/// @brief Keyboard key for equal sign.
#define CELERIQUE_KEYBOARD_KEY_EQUAL                                0x3D
/// @brief Keyboard key for opening bracket.
#define CELERIQUE_KEYBOARD_KEY_OPENING_BRACKET                      0x5B
/// @brief Keyboard key for closing bracket.
#define CELERIQUE_KEYBOARD_KEY_CLOSING_BRACKET                      0x5D
/// @brief Keyboard key for backslash.
#define CELERIQUE_KEYBOARD_KEY_BACKSLASH                            0x5C
/// @brief Keyboard key for semicolon.
#define CELERIQUE_KEYBOARD_KEY_SEMICOLON                            0x3B
/// @brief Keyboard key for apostrophe.
#define CELERIQUE_KEYBOARD_KEY_APOSTROPHE                           0x27
/// @brief Keyboard key for comma.
#define CELERIQUE_KEYBOARD_KEY_COMMA                                0x2C
/// @brief Keyboard key for period.
#define CELERIQUE_KEYBOARD_KEY_PERIOD                               0x2E
/// @brief Keyboard key for forward slash.
#define CELERIQUE_KEYBOARD_KEY_FORWARD_SLASH                        0x2F

/// @brief Keyboard key for up.
#define CELERIQUE_KEYBOARD_KEY_UP                                   0xE1
/// @brief Keyboard key for down.
#define CELERIQUE_KEYBOARD_KEY_DOWN                                 0xE2
/// @brief Keyboard key for left.
#define CELERIQUE_KEYBOARD_KEY_LEFT                                 0xE3
/// @brief Keyboard key for right.
#define CELERIQUE_KEYBOARD_KEY_RIGHT                                0xE4

/// @brief Keyboard key for F1
#define CELERIQUE_KEYBOARD_KEY_F1                                   0xF1
/// @brief Keyboard key for F2
#define CELERIQUE_KEYBOARD_KEY_F2                                   0xF2
/// @brief Keyboard key for F3
#define CELERIQUE_KEYBOARD_KEY_F3                                   0xF3
/// @brief Keyboard key for F4
#define CELERIQUE_KEYBOARD_KEY_F4                                   0xF4
/// @brief Keyboard key for F5
#define CELERIQUE_KEYBOARD_KEY_F5                                   0xF5
/// @brief Keyboard key for F6
#define CELERIQUE_KEYBOARD_KEY_F6                                   0xF6
/// @brief Keyboard key for F7
#define CELERIQUE_KEYBOARD_KEY_F7                                   0xF7
/// @brief Keyboard key for F8
#define CELERIQUE_KEYBOARD_KEY_F8                                   0xF8
/// @brief Keyboard key for F9
#define CELERIQUE_KEYBOARD_KEY_F9                                   0xF9
/// @brief Keyboard key for F10
#define CELERIQUE_KEYBOARD_KEY_F10                                  0xFA
/// @brief Keyboard key for F11
#define CELERIQUE_KEYBOARD_KEY_F11                                  0xFB
/// @brief Keyboard key for F12
#define CELERIQUE_KEYBOARD_KEY_F12                                  0xFC

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.