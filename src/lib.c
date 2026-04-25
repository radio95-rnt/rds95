#include "lib.h"

extern int nanosleep(const struct timespec *req, struct timespec *rem);
void msleep(unsigned long ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000ul;
	ts.tv_nsec = (ms % 1000ul) * 1000;
	nanosleep(&ts, NULL);
}

inline int _strnlen(const char *s, int maxlen) {
	int len = 0;
	while (s[len] != 0 && len < maxlen) len++;
	return len;
}
inline int _strncpy(char *dest, const char *src, int n) {
	int i = 0;
	while (i < n && src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}
	while (i < n) {
		dest[i] = '\0';
		i++;
	}
	return i;
}
uint16_t crc16_ccitt(const char* data, uint16_t len) {
	uint16_t crc = 0xffff;

	for (size_t i = 0; i < len; i++) {
		crc = (crc >> 8) | (crc << 8);
		crc ^= data[i];
		crc ^= (crc & 0xff) >> 4;
		crc ^= (crc << 8) << 4;
		crc ^= ((crc & 0xff) << 4) << 1;
	}

	return crc ^ 0xffff;
}

inline uint16_t get_block_from_group(RDSGroup *group, uint8_t block) {
	switch (block) {
	case 0: return group->a;
	case 1: return group->b;
	case 2: return group->c;
	case 3: return group->d;
	default: return 0;
	}
}

uint8_t add_rds_af(RDSAFs *af_list, float freq) {
	uint16_t af;

	uint8_t entries_reqd = 1;
	if (freq < 87.6f || freq > 107.9f) entries_reqd = 2;

	if (af_list->num_afs + entries_reqd > MAX_AFS) return 1;

	if (freq >= 87.6f && freq <= 107.9f) {
		af = (uint16_t)(freq * 10.0f) - 875;
		af_list->afs[af_list->num_entries] = af;
	} else if (freq >= 153.0f && freq <= 279.0f) {
		af = (uint16_t)(freq - 153.0f) / 9 + 1;
		af_list->afs[af_list->num_entries + 0] = AF_CODE_LFMF_FOLLOWS;
		af_list->afs[af_list->num_entries + 1] = af;
	} else if (freq >= 531.0f && freq <= 1602.0f) {
		af = (uint16_t)(freq - 531.0f) / 9 + 16;
		af_list->afs[af_list->num_entries + 0] = AF_CODE_LFMF_FOLLOWS;
		af_list->afs[af_list->num_entries + 1] = af;
	} else return 1;

	af_list->num_entries += entries_reqd;
	af_list->num_afs++;

	return 0;
}

void convert_to_rdscharset(const char *str, char *output, size_t buffer_size) {
	size_t i = 0;

	while (*str != 0 && i < buffer_size) {
		// The following line is neccesary for the charset.py to work properly
		// CHARSET START
		switch(*str) {
			case (char)0x0a: output[i] = (char)0x0a; break; // N/A
			case (char)0x0d: output[i] = (char)0x0d; break; // N/A
			case (char)0x1f: output[i] = (char)0x1f; break; // N/A
			case (char)0x20: output[i] = (char)0x20; break; // SPACE
			case (char)0x21: output[i] = (char)0x21; break; // EXCLAMATION MARK
			case (char)0x22: output[i] = (char)0x22; break; // QUOTATION MARK
			case (char)0x23: output[i] = (char)0x23; break; // NUMBER SIGN
			case (char)0xc2:
				str++;
				switch(*str) {
					case (char)0xa4: output[i] = (char)0x24; break; // CURRENCY SIGN
					case (char)0xa1: output[i] = (char)0x8e; break; // INVERTED EXCLAMATION MARK
					case (char)0xaa: output[i] = (char)0xa0; break; // FEMININE ORDINAL INDICATOR
					case (char)0xa9: output[i] = (char)0xa2; break; // COPYRIGHT SIGN
					case (char)0xa3: output[i] = (char)0xaa; break; // POUND SIGN
					case (char)0xba: output[i] = (char)0xb0; break; // MASCULINE ORDINAL INDICATOR
					case (char)0xb9: output[i] = (char)0xb1; break; // SUPERSCRIPT ONE
					case (char)0xb2: output[i] = (char)0xb2; break; // SUPERSCRIPT TWO
					case (char)0xb3: output[i] = (char)0xb3; break; // SUPERSCRIPT THREE
					case (char)0xb1: output[i] = (char)0xb4; break; // PLUS-MINUS SIGN
					case (char)0xb5: output[i] = (char)0xb8; break; // MICRO SIGN
					case (char)0xbf: output[i] = (char)0xb9; break; // INVERTED QUESTION MARK
					case (char)0xb0: output[i] = (char)0xbb; break; // DEGREE SIGN
					case (char)0xbc: output[i] = (char)0xbc; break; // VULGAR FRACTION ONE QUARTER
					case (char)0xbd: output[i] = (char)0xbd; break; // VULGAR FRACTION ONE HALF
					case (char)0xbe: output[i] = (char)0xbe; break; // VULGAR FRACTION THREE QUARTERS
					case (char)0xa7: output[i] = (char)0xbf; break; // SECTION SIGN
					default: output[i] = (char)0x3f; break; // QUESTION MARK
				}
				break;
			case (char)0x25: output[i] = (char)0x25; break; // PERCENT SIGN
			case (char)0x26: output[i] = (char)0x26; break; // AMPERSAND
			case (char)0x27: output[i] = (char)0x27; break; // APOSTROPHE
			case (char)0x28: output[i] = (char)0x28; break; // LEFT PARENTHESIS
			case (char)0x29: output[i] = (char)0x29; break; // RIGHT PARENTHESIS
			case (char)0x2a: output[i] = (char)0x2a; break; // ASTERISK
			case (char)0x2b: output[i] = (char)0x2b; break; // PLUS SIGN
			case (char)0x2c: output[i] = (char)0x2c; break; // COMMA
			case (char)0x2d: output[i] = (char)0x2d; break; // HYPHEN-MINUS
			case (char)0x2e: output[i] = (char)0x2e; break; // FULL STOP
			case (char)0x2f: output[i] = (char)0x2f; break; // SOLIDUS
			case (char)0x30: output[i] = (char)0x30; break; // DIGIT ZERO
			case (char)0x31: output[i] = (char)0x31; break; // DIGIT ONE
			case (char)0x32: output[i] = (char)0x32; break; // DIGIT TWO
			case (char)0x33: output[i] = (char)0x33; break; // DIGIT THREE
			case (char)0x34: output[i] = (char)0x34; break; // DIGIT FOUR
			case (char)0x35: output[i] = (char)0x35; break; // DIGIT FIVE
			case (char)0x36: output[i] = (char)0x36; break; // DIGIT SIX
			case (char)0x37: output[i] = (char)0x37; break; // DIGIT SEVEN
			case (char)0x38: output[i] = (char)0x38; break; // DIGIT EIGHT
			case (char)0x39: output[i] = (char)0x39; break; // DIGIT NINE
			case (char)0x3a: output[i] = (char)0x3a; break; // COLON
			case (char)0x3b: output[i] = (char)0x3b; break; // SEMICOLON
			case (char)0x3c: output[i] = (char)0x3c; break; // LESS-THAN SIGN
			case (char)0x3d: output[i] = (char)0x3d; break; // EQUALS SIGN
			case (char)0x3e: output[i] = (char)0x3e; break; // GREATER-THAN SIGN
			case (char)0x3f: output[i] = (char)0x3f; break; // QUESTION MARK
			case (char)0x40: output[i] = (char)0x40; break; // COMMERCIAL AT
			case (char)0x41: output[i] = (char)0x41; break; // LATIN CAPITAL LETTER A
			case (char)0x42: output[i] = (char)0x42; break; // LATIN CAPITAL LETTER B
			case (char)0x43: output[i] = (char)0x43; break; // LATIN CAPITAL LETTER C
			case (char)0x44: output[i] = (char)0x44; break; // LATIN CAPITAL LETTER D
			case (char)0x45: output[i] = (char)0x45; break; // LATIN CAPITAL LETTER E
			case (char)0x46: output[i] = (char)0x46; break; // LATIN CAPITAL LETTER F
			case (char)0x47: output[i] = (char)0x47; break; // LATIN CAPITAL LETTER G
			case (char)0x48: output[i] = (char)0x48; break; // LATIN CAPITAL LETTER H
			case (char)0x49: output[i] = (char)0x49; break; // LATIN CAPITAL LETTER I
			case (char)0x4a: output[i] = (char)0x4a; break; // LATIN CAPITAL LETTER J
			case (char)0x4b: output[i] = (char)0x4b; break; // LATIN CAPITAL LETTER K
			case (char)0x4c: output[i] = (char)0x4c; break; // LATIN CAPITAL LETTER L
			case (char)0x4d: output[i] = (char)0x4d; break; // LATIN CAPITAL LETTER M
			case (char)0x4e: output[i] = (char)0x4e; break; // LATIN CAPITAL LETTER N
			case (char)0x4f: output[i] = (char)0x4f; break; // LATIN CAPITAL LETTER O
			case (char)0x50: output[i] = (char)0x50; break; // LATIN CAPITAL LETTER P
			case (char)0x51: output[i] = (char)0x51; break; // LATIN CAPITAL LETTER Q
			case (char)0x52: output[i] = (char)0x52; break; // LATIN CAPITAL LETTER R
			case (char)0x53: output[i] = (char)0x53; break; // LATIN CAPITAL LETTER S
			case (char)0x54: output[i] = (char)0x54; break; // LATIN CAPITAL LETTER T
			case (char)0x55: output[i] = (char)0x55; break; // LATIN CAPITAL LETTER U
			case (char)0x56: output[i] = (char)0x56; break; // LATIN CAPITAL LETTER V
			case (char)0x57: output[i] = (char)0x57; break; // LATIN CAPITAL LETTER W
			case (char)0x58: output[i] = (char)0x58; break; // LATIN CAPITAL LETTER X
			case (char)0x59: output[i] = (char)0x59; break; // LATIN CAPITAL LETTER Y
			case (char)0x5a: output[i] = (char)0x5a; break; // LATIN CAPITAL LETTER Z
			case (char)0x5b: output[i] = (char)0x5b; break; // LEFT SQUARE BRACKET
			case (char)0x5c: output[i] = (char)0x5c; break; // REVERSE SOLIDUS
			case (char)0x5d: output[i] = (char)0x5d; break; // RIGHT SQUARE BRACKET
			case (char)0xe2:
				str++;
				switch(*str) {
					case (char)0x80:
						str++;
						switch(*str) {
							case (char)0x95: output[i] = (char)0x5e; break; // HORIZONTAL BAR
							case (char)0x96: output[i] = (char)0x60; break; // DOUBLE VERTICAL LINE
							case (char)0xbe: output[i] = (char)0x7e; break; // OVERLINE
							case (char)0xb0: output[i] = (char)0xa3; break; // PER MILLE SIGN
							default: output[i] = (char)0x3f; break; // QUESTION MARK
						}
						break;
					case (char)0x82:
						str++;
						switch(*str) {
							case (char)0xac: output[i] = (char)0xa9; break; // EURO SIGN
							default: output[i] = (char)0x3f; break; // QUESTION MARK
						}
						break;
					case (char)0x86:
						str++;
						switch(*str) {
							case (char)0x90: output[i] = (char)0xac; break; // LEFTWARDS ARROW
							case (char)0x91: output[i] = (char)0xad; break; // UPWARDS ARROW
							case (char)0x92: output[i] = (char)0xae; break; // RIGHTWARDS ARROW
							case (char)0x93: output[i] = (char)0xaf; break; // DOWNWARDS ARROW
							default: output[i] = (char)0x3f; break; // QUESTION MARK
						}
						break;
					default: output[i] = (char)0x3f; break; // QUESTION MARK
				}
				break;
			case (char)0x5f: output[i] = (char)0x5f; break; // LOW LINE
			case (char)0x61: output[i] = (char)0x61; break; // LATIN SMALL LETTER A
			case (char)0x62: output[i] = (char)0x62; break; // LATIN SMALL LETTER B
			case (char)0x63: output[i] = (char)0x63; break; // LATIN SMALL LETTER C
			case (char)0x64: output[i] = (char)0x64; break; // LATIN SMALL LETTER D
			case (char)0x65: output[i] = (char)0x65; break; // LATIN SMALL LETTER E
			case (char)0x66: output[i] = (char)0x66; break; // LATIN SMALL LETTER F
			case (char)0x67: output[i] = (char)0x67; break; // LATIN SMALL LETTER G
			case (char)0x68: output[i] = (char)0x68; break; // LATIN SMALL LETTER H
			case (char)0x69: output[i] = (char)0x69; break; // LATIN SMALL LETTER I
			case (char)0x6a: output[i] = (char)0x6a; break; // LATIN SMALL LETTER J
			case (char)0x6b: output[i] = (char)0x6b; break; // LATIN SMALL LETTER K
			case (char)0x6c: output[i] = (char)0x6c; break; // LATIN SMALL LETTER L
			case (char)0x6d: output[i] = (char)0x6d; break; // LATIN SMALL LETTER M
			case (char)0x6e: output[i] = (char)0x6e; break; // LATIN SMALL LETTER N
			case (char)0x6f: output[i] = (char)0x6f; break; // LATIN SMALL LETTER O
			case (char)0x70: output[i] = (char)0x70; break; // LATIN SMALL LETTER P
			case (char)0x71: output[i] = (char)0x71; break; // LATIN SMALL LETTER Q
			case (char)0x72: output[i] = (char)0x72; break; // LATIN SMALL LETTER R
			case (char)0x73: output[i] = (char)0x73; break; // LATIN SMALL LETTER S
			case (char)0x74: output[i] = (char)0x74; break; // LATIN SMALL LETTER T
			case (char)0x75: output[i] = (char)0x75; break; // LATIN SMALL LETTER U
			case (char)0x76: output[i] = (char)0x76; break; // LATIN SMALL LETTER V
			case (char)0x77: output[i] = (char)0x77; break; // LATIN SMALL LETTER W
			case (char)0x78: output[i] = (char)0x78; break; // LATIN SMALL LETTER X
			case (char)0x79: output[i] = (char)0x79; break; // LATIN SMALL LETTER Y
			case (char)0x7a: output[i] = (char)0x7a; break; // LATIN SMALL LETTER Z
			case (char)0x7b: output[i] = (char)0x7b; break; // LEFT CURLY BRACKET
			case (char)0x7c: output[i] = (char)0x7c; break; // VERTICAL LINE
			case (char)0x7d: output[i] = (char)0x7d; break; // RIGHT CURLY BRACKET
			case (char)0xc3:
				str++;
				switch(*str) {
					case (char)0xa1: output[i] = (char)0x80; break; // LATIN SMALL LETTER A WITH ACUTE
					case (char)0xa0: output[i] = (char)0x81; break; // LATIN SMALL LETTER A WITH GRAVE
					case (char)0xa9: output[i] = (char)0x82; break; // LATIN SMALL LETTER E WITH ACUTE
					case (char)0xa8: output[i] = (char)0x83; break; // LATIN SMALL LETTER E WITH GRAVE
					case (char)0xad: output[i] = (char)0x84; break; // LATIN SMALL LETTER I WITH ACUTE
					case (char)0xac: output[i] = (char)0x85; break; // LATIN SMALL LETTER I WITH GRAVE
					case (char)0xb3: output[i] = (char)0x86; break; // LATIN SMALL LETTER O WITH ACUTE
					case (char)0xb2: output[i] = (char)0x87; break; // LATIN SMALL LETTER O WITH GRAVE
					case (char)0xba: output[i] = (char)0x88; break; // LATIN SMALL LETTER U WITH ACUTE
					case (char)0xb9: output[i] = (char)0x89; break; // LATIN SMALL LETTER U WITH GRAVE
					case (char)0x91: output[i] = (char)0x8a; break; // LATIN CAPITAL LETTER N WITH TILDE
					case (char)0x87: output[i] = (char)0x8b; break; // LATIN CAPITAL LETTER C WITH CEDILLA
					case (char)0x9f: output[i] = (char)0x8d; break; // LATIN SMALL LETTER SHARP S
					case (char)0xa2: output[i] = (char)0x90; break; // LATIN SMALL LETTER A WITH CIRCUMFLEX
					case (char)0xa4: output[i] = (char)0x91; break; // LATIN SMALL LETTER A WITH DIAERESIS
					case (char)0xaa: output[i] = (char)0x92; break; // LATIN SMALL LETTER E WITH CIRCUMFLEX
					case (char)0xab: output[i] = (char)0x93; break; // LATIN SMALL LETTER E WITH DIAERESIS
					case (char)0xae: output[i] = (char)0x94; break; // LATIN SMALL LETTER I WITH CIRCUMFLEX
					case (char)0xaf: output[i] = (char)0x95; break; // LATIN SMALL LETTER I WITH DIAERESIS
					case (char)0xb4: output[i] = (char)0x96; break; // LATIN SMALL LETTER O WITH CIRCUMFLEX
					case (char)0xb6: output[i] = (char)0x97; break; // LATIN SMALL LETTER O WITH DIAERESIS
					case (char)0xbb: output[i] = (char)0x98; break; // LATIN SMALL LETTER U WITH CIRCUMFLEX
					case (char)0xbc: output[i] = (char)0x99; break; // LATIN SMALL LETTER U WITH DIAERESIS
					case (char)0xb1: output[i] = (char)0x9a; break; // LATIN SMALL LETTER N WITH TILDE
					case (char)0xa7: output[i] = (char)0x9b; break; // LATIN SMALL LETTER C WITH CEDILLA
					case (char)0xb7: output[i] = (char)0xba; break; // DIVISION SIGN
					case (char)0x81: output[i] = (char)0xc0; break; // LATIN CAPITAL LETTER A WITH ACUTE
					case (char)0x80: output[i] = (char)0xc1; break; // LATIN CAPITAL LETTER A WITH GRAVE
					case (char)0x89: output[i] = (char)0xc2; break; // LATIN CAPITAL LETTER E WITH ACUTE
					case (char)0x88: output[i] = (char)0xc3; break; // LATIN CAPITAL LETTER E WITH GRAVE
					case (char)0x8d: output[i] = (char)0xc4; break; // LATIN CAPITAL LETTER I WITH ACUTE
					case (char)0x8c: output[i] = (char)0xc5; break; // LATIN CAPITAL LETTER I WITH GRAVE
					case (char)0x93: output[i] = (char)0xc6; break; // LATIN CAPITAL LETTER O WITH ACUTE
					case (char)0x92: output[i] = (char)0xc7; break; // LATIN CAPITAL LETTER O WITH GRAVE
					case (char)0x9a: output[i] = (char)0xc8; break; // LATIN CAPITAL LETTER U WITH ACUTE
					case (char)0x99: output[i] = (char)0xc9; break; // LATIN CAPITAL LETTER U WITH GRAVE
					case (char)0x90: output[i] = (char)0xce; break; // LATIN CAPITAL LETTER ETH
					case (char)0x82: output[i] = (char)0xd0; break; // LATIN CAPITAL LETTER A WITH CIRCUMFLEX
					case (char)0x84: output[i] = (char)0xd1; break; // LATIN CAPITAL LETTER A WITH DIAERESIS
					case (char)0x8a: output[i] = (char)0xd2; break; // LATIN CAPITAL LETTER E WITH CIRCUMFLEX
					case (char)0x8b: output[i] = (char)0xd3; break; // LATIN CAPITAL LETTER E WITH DIAERESIS
					case (char)0x8e: output[i] = (char)0xd4; break; // LATIN CAPITAL LETTER I WITH CIRCUMFLEX
					case (char)0x8f: output[i] = (char)0xd5; break; // LATIN CAPITAL LETTER I WITH DIAERESIS
					case (char)0x94: output[i] = (char)0xd6; break; // LATIN CAPITAL LETTER O WITH CIRCUMFLEX
					case (char)0x96: output[i] = (char)0xd7; break; // LATIN CAPITAL LETTER O WITH DIAERESIS
					case (char)0x9b: output[i] = (char)0xd8; break; // LATIN CAPITAL LETTER U WITH CIRCUMFLEX
					case (char)0x9c: output[i] = (char)0xd9; break; // LATIN CAPITAL LETTER U WITH DIAERESIS
					case (char)0x83: output[i] = (char)0xe0; break; // LATIN CAPITAL LETTER A WITH TILDE
					case (char)0x85: output[i] = (char)0xe1; break; // LATIN CAPITAL LETTER A WITH RING ABOVE
					case (char)0x86: output[i] = (char)0xe2; break; // LATIN CAPITAL LETTER AE
					case (char)0x9d: output[i] = (char)0xe5; break; // LATIN CAPITAL LETTER Y WITH ACUTE
					case (char)0x95: output[i] = (char)0xe6; break; // LATIN CAPITAL LETTER O WITH TILDE
					case (char)0x98: output[i] = (char)0xe7; break; // LATIN CAPITAL LETTER O WITH STROKE
					case (char)0x9e: output[i] = (char)0xe8; break; // LATIN CAPITAL LETTER THORN
					case (char)0xb0: output[i] = (char)0xef; break; // LATIN SMALL LETTER ETH
					case (char)0xa3: output[i] = (char)0xf0; break; // LATIN SMALL LETTER A WITH TILDE
					case (char)0xa5: output[i] = (char)0xf1; break; // LATIN SMALL LETTER A WITH RING ABOVE
					case (char)0xa6: output[i] = (char)0xf2; break; // LATIN SMALL LETTER AE
					case (char)0xbd: output[i] = (char)0xf5; break; // LATIN SMALL LETTER Y WITH ACUTE
					case (char)0xb5: output[i] = (char)0xf6; break; // LATIN SMALL LETTER O WITH TILDE
					case (char)0xb8: output[i] = (char)0xf7; break; // LATIN SMALL LETTER O WITH STROKE
					case (char)0xbe: output[i] = (char)0xf8; break; // LATIN SMALL LETTER THORN
					default: output[i] = (char)0x3f; break; // QUESTION MARK
				}
				break;
			case (char)0xc5:
				str++;
				switch(*str) {
					case (char)0x9e: output[i] = (char)0x8c; break; // LATIN CAPITAL LETTER S WITH CEDILLA
					case (char)0x9f: output[i] = (char)0x9c; break; // LATIN SMALL LETTER S WITH CEDILLA
					case (char)0x88: output[i] = (char)0xa6; break; // LATIN SMALL LETTER N WITH CARON
					case (char)0x91: output[i] = (char)0xa7; break; // LATIN SMALL LETTER O WITH DOUBLE ACUTE
					case (char)0x84: output[i] = (char)0xb6; break; // LATIN SMALL LETTER N WITH ACUTE
					case (char)0xb1: output[i] = (char)0xb7; break; // LATIN SMALL LETTER U WITH DOUBLE ACUTE
					case (char)0x98: output[i] = (char)0xca; break; // LATIN CAPITAL LETTER R WITH CARON
					case (char)0xa0: output[i] = (char)0xcc; break; // LATIN CAPITAL LETTER S WITH CARON
					case (char)0xbd: output[i] = (char)0xcd; break; // LATIN CAPITAL LETTER Z WITH CARON
					case (char)0x99: output[i] = (char)0xda; break; // LATIN SMALL LETTER R WITH CARON
					case (char)0xa1: output[i] = (char)0xdc; break; // LATIN SMALL LETTER S WITH CARON
					case (char)0xbe: output[i] = (char)0xdd; break; // LATIN SMALL LETTER Z WITH CARON
					case (char)0x80: output[i] = (char)0xdf; break; // LATIN SMALL LETTER L WITH MIDDLE DOT
					case (char)0x92: output[i] = (char)0xe3; break; // LATIN CAPITAL LIGATURE OE
					case (char)0xb7: output[i] = (char)0xe4; break; // LATIN SMALL LETTER Y WITH CIRCUMFLEX
					case (char)0x8a: output[i] = (char)0xe9; break; // LATIN CAPITAL LETTER ENG
					case (char)0x94: output[i] = (char)0xea; break; // LATIN CAPITAL LETTER R WITH ACUTE
					case (char)0x9a: output[i] = (char)0xec; break; // LATIN CAPITAL LETTER S WITH ACUTE
					case (char)0xb9: output[i] = (char)0xed; break; // LATIN CAPITAL LETTER Z WITH ACUTE
					case (char)0xa6: output[i] = (char)0xee; break; // LATIN CAPITAL LETTER T WITH STROKE
					case (char)0x93: output[i] = (char)0xf3; break; // LATIN SMALL LIGATURE OE
					case (char)0xb5: output[i] = (char)0xf4; break; // LATIN SMALL LETTER W WITH CIRCUMFLEX
					case (char)0x8b: output[i] = (char)0xf9; break; // LATIN SMALL LETTER ENG
					case (char)0x95: output[i] = (char)0xfa; break; // LATIN SMALL LETTER R WITH ACUTE
					case (char)0x9b: output[i] = (char)0xfc; break; // LATIN SMALL LETTER S WITH ACUTE
					case (char)0xba: output[i] = (char)0xfd; break; // LATIN SMALL LETTER Z WITH ACUTE
					case (char)0xa7: output[i] = (char)0xfe; break; // LATIN SMALL LETTER T WITH STROKE
					default: output[i] = (char)0x3f; break; // QUESTION MARK
				}
				break;
			case (char)0xc4:
				str++;
				switch(*str) {
					case (char)0xb2: output[i] = (char)0x8f; break; // LATIN CAPITAL LIGATURE IJ
					case (char)0x9f: output[i] = (char)0x9d; break; // LATIN SMALL LETTER G WITH BREVE
					case (char)0xb1: output[i] = (char)0x9e; break; // LATIN SMALL LETTER DOTLESS I
					case (char)0xb3: output[i] = (char)0x9f; break; // LATIN SMALL LIGATURE IJ
					case (char)0x9e: output[i] = (char)0xa4; break; // LATIN CAPITAL LETTER G WITH BREVE
					case (char)0x9b: output[i] = (char)0xa5; break; // LATIN SMALL LETTER E WITH CARON
					case (char)0xb0: output[i] = (char)0xb5; break; // LATIN CAPITAL LETTER I WITH DOT ABOVE
					case (char)0x8c: output[i] = (char)0xcb; break; // LATIN CAPITAL LETTER C WITH CARON
					case (char)0xbf: output[i] = (char)0xcf; break; // LATIN CAPITAL LETTER L WITH MIDDLE DOT
					case (char)0x8d: output[i] = (char)0xdb; break; // LATIN SMALL LETTER C WITH CARON
					case (char)0x91: output[i] = (char)0xde; break; // LATIN SMALL LETTER D WITH STROKE
					case (char)0x86: output[i] = (char)0xeb; break; // LATIN CAPITAL LETTER C WITH ACUTE
					case (char)0x87: output[i] = (char)0xfb; break; // LATIN SMALL LETTER C WITH ACUTE
					default: output[i] = (char)0x3f; break; // QUESTION MARK
				}
				break;
			case (char)0xce:
				str++;
				switch(*str) {
					case (char)0xb1: output[i] = (char)0xa1; break; // GREEK SMALL LETTER ALPHA
					default: output[i] = (char)0x3f; break; // QUESTION MARK
				}
				break;
			case (char)0xcf:
				str++;
				switch(*str) {
					case (char)0x80: output[i] = (char)0xa8; break; // GREEK SMALL LETTER PI
					default: output[i] = (char)0x3f; break; // QUESTION MARK
				}
				break;
			case (char)0x24: output[i] = (char)0xab; break; // DOLLAR SIGN
			default: output[i] = (char)0x3f; break; // QUESTION MARK
		}
		// CHARSET END

		i++;
		str++;
	}

	output[i] = 0;
}
