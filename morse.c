/**@file morse.c
   @brief A Morse encoder/decoder library
   @author Richard James Howe
   @license MIT
  
   <https://en.wikipedia.org/wiki/Morse_code>
   <https://en.wikipedia.org/wiki/Prosigns_for_Morse_code>
  
   Notes on timings:
  
   - Duration of a '.' is one unit
   - Duration of a '_' is three units
   - Space between components of a character is one unit
   - Space between characters is three units
   - Space between words is seven units
 
   @todo Common prosigns should be mapped to ASCII control characters

   Start of work (CT)          _._._.
   Invitation to transit (K)   _._
   End of message (AR)         ._._.
   Error                       ........
   End of work (VA)            ..._._
   Invitation for a particular station to transmit (KN)      _.__.
   Wait                        ._...
   Understood                  ..._.

   Of note, a second level of encoding/decoding could be achieved as
   well that would take into account common Morse code conventions. For
   example a single '?' means repeat, whilst one at the end of a message
   is a request/question. Likewise, 'Q Codes' could be decoded and other
   prosigns.

   Q Code (?) Meaning
   QSL        I acknowledge receipt
   QSL?       Do you acknowledge?
   QRX        Wait
   QRX?       Should I wait?
   QRV        I am ready to copy
   QRV?       Are you ready to copy?
   QRL        The frequency is in use
   QRL?       Is the frequency in use?
   QTH        My location is...
   QTH?       What is your location?

   @todo Add a Morse code decoder */
#include "morse.h"
#include <avr/pgmspace.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#define MORSE_USE_ABBREVIATED_NUMBERS (0)
#define MORSE_TABLE_COUNT             (128)
#define MORSE_CHARACTER_LENGTH        (7)

/**@todo encode this table more efficiently */
static const PROGMEM char morse_table[MORSE_TABLE_COUNT][MORSE_CHARACTER_LENGTH] = {
	['a']  = "._", /*     "Letter A, a" */ 
	['b']  = "_...", /*   "Letter B, b" */ 
	['c']  = "_._.", /*   "Letter C, c" */ 
	['d']  = "_..", /*    "Letter D, d" */ 
	['e']  = ".", /*      "Letter E, e" */ 
	['f']  = ".._.", /*   "Letter F, f" */ 
	['g']  = "__.", /*    "Letter G, g" */ 
	['h']  = "....", /*   "Letter H, h" */ 
	['i']  = "..", /*     "Letter I, i" */ 
	['j']  = ".___", /*   "Letter J, j" */ 
	['k']  = "_._", /*    "Letter K, k" */ 
	['l']  = "._..", /*   "Letter L, l" */ 
	['m']  = "__", /*     "Letter M, m" */ 
	['n']  = "_.", /*     "Letter N, n" */ 
	['o']  = "___", /*    "Letter O, o" */ 
	['p']  = ".__.", /*   "Letter P, p" */ 
	['q']  = "__._", /*   "Letter Q, q" */ 
	['r']  = "._.", /*    "Letter R, r" */ 
	['s']  = "...", /*    "Letter S, s" */ 
	['t']  = "_", /*      "Letter T, t" */ 
	['u']  = ".._", /*    "Letter U, u" */ 
	['v']  = "..._", /*   "Letter V, v" */ 
	['w']  = ".__", /*    "Letter W, w" */ 
	['x']  = "_.._", /*   "Letter X, x" */ 
	['y']  = "_.__", /*   "Letter Y, y" */ 
	['z']  = "__..", /*   "Letter Z, z" */ 
	['A']  = "._", /*     "Letter A, a" */ 
	['B']  = "_...", /*   "Letter B, b" */ 
	['C']  = "_._.", /*   "Letter C, c" */ 
	['D']  = "_..", /*    "Letter D, d" */ 
	['E']  = ".", /*      "Letter E, e" */ 
	['F']  = ".._.", /*   "Letter F, f" */ 
	['G']  = "__.", /*    "Letter G, g" */ 
	['H']  = "....", /*   "Letter H, h" */ 
	['I']  = "..", /*     "Letter I, i" */ 
	['J']  = ".___", /*   "Letter J, j" */ 
	['K']  = "_._", /*    "Letter K, k" */ 
	['L']  = "._..", /*   "Letter L, l" */ 
	['M']  = "__", /*     "Letter M, m" */ 
	['N']  = "_.", /*     "Letter N, n" */ 
	['O']  = "___", /*    "Letter O, o" */ 
	['P']  = ".__.", /*   "Letter P, p" */ 
	['Q']  = "__._", /*   "Letter Q, q" */ 
	['R']  = "._.", /*    "Letter R, r" */ 
	['S']  = "...", /*    "Letter S, s" */ 
	['T']  = "_", /*      "Letter T, t" */ 
	['U']  = ".._", /*    "Letter U, u" */ 
	['V']  = "..._", /*   "Letter V, v" */ 
	['W']  = ".__", /*    "Letter W, w" */ 
	['X']  = "_.._", /*   "Letter X, x" */ 
	['Y']  = "_.__", /*   "Letter Y, y" */ 
	['Z']  = "__..", /*   "Letter Z, z" */ 
#if MORSE_USE_ABBREVIATED_NUMBERS
	['0']  = "_", /*      "Abbreviated Number 0" */  //(sometimes a long dash is used)
	['1']  = "._", /*     "Abbreviated Number 1" */ 
	['2']  = ".._", /*    "Abbreviated Number 2" */ 
	['3']  = "..._", /*   "Abbreviated Number 3" */ 
	['4']  = "...._", /*  "Abbreviated Number 4" */ 
	['5']  = ".", /*      "Abbreviated Number 5" */ 
	['6']  = "_....", /*  "Abbreviated Number 6" */ 
	['7']  = "_...", /*   "Abbreviated Number 7" */ 
	['8']  = "_..", /*    "Abbreviated Number 8" */ 
	['9']  = "_.", /*     "Abbreviated Number 9" */ 
#else
	['0']  = "_____", /*  "Number 0" */ 
	['1']  = ".____", /*  "Number 1" */ 
	['2']  = "..___", /*  "Number 2" */ 
	['3']  = "...__", /*  "Number 3" */ 
	['4']  = "...._", /*  "Number 4" */ 
	['5']  = ".....", /*  "Number 5" */ 
	['6']  = "_....", /*  "Number 6" */ 
	['7']  = "__...", /*  "Number 7" */ 
	['8']  = "___..", /*  "Number 8" */ 
	['9']  = "____.", /*  "Number 9" */ 
#endif
	[',']  = "__..__", /* "Comma ," */ 
	['.']  = "._._._", /* "Full stop (period) ." */ 
	['?']  = "..__..", /* "Question mark ?" */ 
	[';']  = "_._._.", /* "Semicolon ;" */ 
	[':']  = "___...", /* "Colon :" */  /*(or division sign)*/
	['/']  = "_.._.", /*  "Slash /" */ 
	['-']  = "_...._", /* "Dash -" */ 
	['\''] = ".____.", /* "Apostrophe '" */ 
	['"']  = "._.._.", /* "Inverted commas \""  */ 
	['_']  = "..__._", /* "Underline _" */ 
	['(']  = "_.__.", /*  "Left bracket or parenthesis (" */ 
	[')']  = "_.__._", /* "Right bracket or parenthesis )" */ 
	['=']  = "_..._", /*  "Double hyphen = equals sign" */ 
	['+']  = "._._.", /*  "Addition sign +" */ 
	['*']  = "_.._", /*   "Multiplication sign *" */ 
	['@']  = ".__._.", /* "Commercial at @" */ 
	['!']  = "_._.__", /* "Exclamation Point !" */ 
	['\n'] = "._._", /*   "Start new line" */ 
#ifdef LARGE_TABLE
	/* <https://en.wikipedia.org/wiki/ISO/IEC_8859-1> */
	[0XC0] = ".__._", /*  "Letter 'A' with accent" */ 
	[0XC4] = "._._", /*   "Letter 'A' with umlaut" */ 
	[0XD1] = "__.__", /*  "Letter 'N' with tilde" */ 
	[0XC9] = ".._..", /*  "Letter 'E' with accent" */ 
	[0XD6] = "___.", /*   "Letter 'O' with umlaut" */ 
	[0XDC] = "..__", /*   "Letter 'U' with umlaut" */ 
#endif
};

static int morse_populated(const char *buffer, size_t length) {
	if(!length || !buffer[0])
		return 0;
	return 1;
}

int morse_encode_character(unsigned char c, char *buffer, size_t length) {
	if(length < MORSE_CHARACTER_LENGTH)
		return -1;
	if((c & 0x80))
		return -1;

	for(size_t i = 0; i < length; i++) {
		char ch = pgm_read_byte(&(morse_table[c][i]));
		buffer[i] = ch;
	}
	
	if(!morse_populated(buffer, length))
		return -1;
	return 0;
}

#if 0
int morse_decode_character(const char *s, const char **endptr) {
	size_t i = 0;
	char buf[16] = { 0 };
	char c = 0;

	if(endptr)
		*endptr = NULL;

	while(i < (sizeof(buf) - 1)) {
		size_t j = 0;
		while((c = *s++)) {
			j++;
			if(isspace(c))
				continue;
			else
				break;
		}
		if((c != '.' && c != '_') || j > 2)
			break;
		buf[i++] = c;
	}
	if(endptr)
		*endptr = s + i;

	for(size_t k = 0; k < MORSE_TABLE_COUNT; k++) {
		const morse_character_t *mc = &morse_table[k];
		if(!morse_populated(mc))
			continue;
		if(!strcmp(buf, mc->encoding))
			return k;
	}
	
	return -1;
}
#endif

#if 0
/* character: { { '.' | '_' } { ' ' }x[0-2] }+
 * word:      { character { ' ' }x[1-4] }+
 * line:      { word { ' ' }+ } */
int morse_decode_string(char *buf, size_t length, const char *s) {
	return -1;
}
#endif

#ifdef MORSE_TEST

#include <stdio.h>
void test_decode(FILE *out, const char *s) {
	const char *end = NULL;
	int ch = morse_decode_character(s, &end);
	fprintf(out, "decoded(\"%s\") = '%c'/%d\n", s, isgraph(ch) ? ch : '.', ch);
}

int main(int argc, char **argv) {
	if(argc == 1) {
		test_decode(stdout, " . _ ");
		test_decode(stdout, " . _ _ ");
		test_decode(stdout, " _ ");
		test_decode(stdout, " .");
		test_decode(stdout, "_____");
		test_decode(stdout, "_ __ _ _ ");
		return 0;
	}

	if(argc != 2) {
		fprintf(stderr, "usage: %s string\n", argv[0]);
		return -1;
	}


	if(morse_print_string(stdout, argv[1]) < 0)
		return -1;
	fputc('\n', stdout);
	return 0;
}
#endif
