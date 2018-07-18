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

   @todo Port this to the Arduino and do something useful with it
 */
#include "morse.h"
#include <avr/pgmspace.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#define MORSE_USE_ABBREVIATED_NUMBERS (0)
#define MORSE_TABLE_COUNT             (128)

//#define F(X) ((const __flash char[]) { X })
#define F(X) (X)
#define MORSE_CHARACTER_LENGTH (7)

/**@todo encode this table more efficiently */
static const PROGMEM char morse_table[MORSE_TABLE_COUNT][MORSE_CHARACTER_LENGTH] = {
	['a']  = F("._"), /*     "Letter A, a" */ 
	['b']  = F("_..."), /*   "Letter B, b" */ 
	['c']  = F("_._."), /*   "Letter C, c" */ 
	['d']  = F("_.."), /*    "Letter D, d" */ 
	['e']  = F("."), /*      "Letter E, e" */ 
	['f']  = F(".._."), /*   "Letter F, f" */ 
	['g']  = F("__."), /*    "Letter G, g" */ 
	['h']  = F("...."), /*   "Letter H, h" */ 
	['i']  = F(".."), /*     "Letter I, i" */ 
	['j']  = F(".___"), /*   "Letter J, j" */ 
	['k']  = F("_._"), /*    "Letter K, k" */ 
	['l']  = F("._.."), /*   "Letter L, l" */ 
	['m']  = F("__"), /*     "Letter M, m" */ 
	['n']  = F("_."), /*     "Letter N, n" */ 
	['o']  = F("___"), /*    "Letter O, o" */ 
	['p']  = F(".__."), /*   "Letter P, p" */ 
	['q']  = F("__._"), /*   "Letter Q, q" */ 
	['r']  = F("._."), /*    "Letter R, r" */ 
	['s']  = F("..."), /*    "Letter S, s" */ 
	['t']  = F("_"), /*      "Letter T, t" */ 
	['u']  = F(".._"), /*    "Letter U, u" */ 
	['v']  = F("..._"), /*   "Letter V, v" */ 
	['w']  = F(".__"), /*    "Letter W, w" */ 
	['x']  = F("_.._"), /*   "Letter X, x" */ 
	['y']  = F("_.__"), /*   "Letter Y, y" */ 
	['z']  = F("__.."), /*   "Letter Z, z" */ 
	['A']  = F("._"), /*     "Letter A, a" */ 
	['B']  = F("_..."), /*   "Letter B, b" */ 
	['C']  = F("_._."), /*   "Letter C, c" */ 
	['D']  = F("_.."), /*    "Letter D, d" */ 
	['E']  = F("."), /*      "Letter E, e" */ 
	['F']  = F(".._."), /*   "Letter F, f" */ 
	['G']  = F("__."), /*    "Letter G, g" */ 
	['H']  = F("...."), /*   "Letter H, h" */ 
	['I']  = F(".."), /*     "Letter I, i" */ 
	['J']  = F(".___"), /*   "Letter J, j" */ 
	['K']  = F("_._"), /*    "Letter K, k" */ 
	['L']  = F("._.."), /*   "Letter L, l" */ 
	['M']  = F("__"), /*     "Letter M, m" */ 
	['N']  = F("_."), /*     "Letter N, n" */ 
	['O']  = F("___"), /*    "Letter O, o" */ 
	['P']  = F(".__."), /*   "Letter P, p" */ 
	['Q']  = F("__._"), /*   "Letter Q, q" */ 
	['R']  = F("._."), /*    "Letter R, r" */ 
	['S']  = F("..."), /*    "Letter S, s" */ 
	['T']  = F("_"), /*      "Letter T, t" */ 
	['U']  = F(".._"), /*    "Letter U, u" */ 
	['V']  = F("..._"), /*   "Letter V, v" */ 
	['W']  = F(".__"), /*    "Letter W, w" */ 
	['X']  = F("_.._"), /*   "Letter X, x" */ 
	['Y']  = F("_.__"), /*   "Letter Y, y" */ 
	['Z']  = F("__.."), /*   "Letter Z, z" */ 
#if MORSE_USE_ABBREVIATED_NUMBERS
	['0']  = F("_"), /*      "Abbreviated Number 0" */  //(sometimes a long dash is used)
	['1']  = F("._"), /*     "Abbreviated Number 1" */ 
	['2']  = F(".._"), /*    "Abbreviated Number 2" */ 
	['3']  = F("..._"), /*   "Abbreviated Number 3" */ 
	['4']  = F("...._"), /*  "Abbreviated Number 4" */ 
	['5']  = F("."), /*      "Abbreviated Number 5" */ 
	['6']  = F("_...."), /*  "Abbreviated Number 6" */ 
	['7']  = F("_..."), /*   "Abbreviated Number 7" */ 
	['8']  = F("_.."), /*    "Abbreviated Number 8" */ 
	['9']  = F("_."), /*     "Abbreviated Number 9" */ 
#else
	['0']  = F("_____"), /*  "Number 0" */ 
	['1']  = F(".____"), /*  "Number 1" */ 
	['2']  = F("..___"), /*  "Number 2" */ 
	['3']  = F("...__"), /*  "Number 3" */ 
	['4']  = F("...._"), /*  "Number 4" */ 
	['5']  = F("....."), /*  "Number 5" */ 
	['6']  = F("_...."), /*  "Number 6" */ 
	['7']  = F("__..."), /*  "Number 7" */ 
	['8']  = F("___.."), /*  "Number 8" */ 
	['9']  = F("____."), /*  "Number 9" */ 
#endif
	[',']  = F("__..__"), /* "Comma ," */ 
	['.']  = F("._._._"), /* "Full stop (period) ." */ 
	['?']  = F("..__.."), /* "Question mark ?" */ 
	[';']  = F("_._._."), /* "Semicolon ;" */ 
	[':']  = F("___..."), /* "Colon :" */  /*(or division sign)*/
	['/']  = F("_.._."), /*  "Slash /" */ 
	['-']  = F("_...._"), /* "Dash -" */ 
	['\''] = F(".____."), /* "Apostrophe '" */ 
	['"']  = F("._.._."), /* "Inverted commas \""  */ 
	['_']  = F("..__._"), /* "Underline _" */ 
	['(']  = F("_.__."), /*  "Left bracket or parenthesis (" */ 
	[')']  = F("_.__._"), /* "Right bracket or parenthesis )" */ 
	['=']  = F("_..._"), /*  "Double hyphen = equals sign" */ 
	['+']  = F("._._."), /*  "Addition sign +" */ 
	['*']  = F("_.._"), /*   "Multiplication sign *" */ 
	['@']  = F(".__._."), /* "Commercial at @" */ 
	['!']  = F("_._.__"), /* "Exclamation Point !" */ 
	['\n'] = F("._._"), /*   "Start new line" */ 
#ifdef LARGE_TABLE
	/* <https://en.wikipedia.org/wiki/ISO/IEC_8859-1> */
	[0XC0] = F(".__._"), /*  "Letter 'A' with accent" */ 
	[0XC4] = F("._._"), /*   "Letter 'A' with umlaut" */ 
	[0XD1] = F("__.__"), /*  "Letter 'N' with tilde" */ 
	[0XC9] = F(".._.."), /*  "Letter 'E' with accent" */ 
	[0XD6] = F("___."), /*   "Letter 'O' with umlaut" */ 
	[0XDC] = F("..__"), /*   "Letter 'U' with umlaut" */ 
#endif
};

static int morse_populated(char *buffer, size_t length) {
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
