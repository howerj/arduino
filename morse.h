#ifndef MORSE_H
#define MORSE_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>

int morse_encode_character(unsigned char c, char *buffer, size_t length);

#define MORSE_DOT_DELAY_MULTIPLIER    (1)
#define MORSE_SPACE_DELAY_MULTIPLIER  (1)
#define MORSE_DASH_DELAY_MULTIPLIER   (3)

#define MORSE_SPACES_IN_ELEMENT_SEPARATOR (1)
#define MORSE_SPACES_IN_CHAR_SEPARATOR    (3)
#define MORSE_SPACES_IN_WORD_SEPARATOR    (7)

#ifdef __cplusplus
}
#endif
#endif
