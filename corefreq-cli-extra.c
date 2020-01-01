/*
 * CoreFreq (C) 2015-2020 CYRIL INGENIERIE
 * Contributors: Andrew Gurinovich ; CyrIng
 * Licenses: GPL2
 *
 * Some ideas taken from https://github.com/cesanta/frozen/ 
 * under Apache 2.0 license
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "corefreq-cli-extra.h"

struct json_state;

enum JSON_STATE {
	DEFAULT, IN_ARRAY, IN_ARRAY2, IN_OBJECT, IN_OBJECT2
};

int json_writer_stdout(struct json_state * state, const char *str, size_t len)
{
	return fwrite(str, len, 1, stdout);
}

int get_utf8_char_len(unsigned char ch)
{
	if ((ch & 0x80) == 0)
		return 1;
	switch (ch & 0xf0) {
	case 0xf0:
		return 4;
	case 0xe0:
		return 3;
	default:
		return 2;
	}
}

/* This routine does NOT handle unicode finely - everything above 0x7f should
 * be \u0000 encoded, but this requires us to be utf8 capable. Check the
 * following tiny libraries(for future improvement):
 * https://github.com/git/git/blob/master/utf8.c
 * https://github.com/JeffBezanson/cutef8/blob/master/utf8.c
 * https://github.com/sheredom/utf8.h
 * https://github.com/JuliaStrings/utf8proc
 */

int json_escape(struct json_state *state, const char *p, size_t len)
{
	size_t i, cl, n = 0;
	const char *hex_digits = "0123456789abcdef";
	const char *specials = "btnvfr";

    for (i = 0; i < len; i++) {
	unsigned char ch = ((unsigned char *) p)[i];
	if (ch == '"' || ch == '\\') {
		n += state->write(state, "\\", 1);
		n += state->write(state, p + i, 1);
	} else if (ch >= '\b' && ch <= '\r') {
		n += state->write(state, "\\", 1);
		n += state->write(state, &specials[ch - '\b'], 1);
	} else if (isprint(ch)) {
		n += state->write(state, p + i, 1);
	} else if ((cl = get_utf8_char_len(ch)) == 1) {
		n += state->write(state, "\\u00", 4);
		n += state->write(state, &hex_digits[(ch >> 4) % 0xf], 1);
		n += state->write(state, &hex_digits[ch % 0xf], 1);
	} else {
		n += state->write(state, p + i, cl);
		i += cl - 1;
	}
    }
	return n;
}

void json_start_object(struct json_state *state)
{
	assert(state->depth < JSON_MAX_DEPTH);
	/* TODO: assert(state->nested_state[state->depth] != IN_OBJECT); */
	if (state->nested_state[state->depth] == IN_ARRAY2 ) {
		state->write(state, ", {", 3);
	} else {
		state->write(state, "{", 1);
	}
	if (state->nested_state[state->depth] == IN_ARRAY) {
		state->nested_state[state->depth] = IN_ARRAY2;
	}
	if (state->nested_state[state->depth] == IN_OBJECT) {
		state->nested_state[state->depth] = IN_OBJECT2;
	}
	state->nested_state[++state->depth] = IN_OBJECT;
}

void json_end_object(struct json_state *state)
{
	assert(state->depth >= 0);
	assert(state->nested_state[state->depth] ==			\
		IN_OBJECT || state->nested_state[state->depth] == IN_OBJECT2);

	state->write(state, "}", 1);
	state->nested_state[state->depth--] = DEFAULT;
}

void json_start_arr(struct json_state *state)
{
	assert(state->depth < JSON_MAX_DEPTH);

	if (state->nested_state[state->depth] == IN_ARRAY2) {
		state->write(state, ", [", 3);
	} else {
		state->write(state, "[", 1);
	}
	if (state->nested_state[state->depth] == IN_ARRAY) {
		state->nested_state[state->depth] = IN_ARRAY2;
	}
	if (state->nested_state[state->depth] == IN_OBJECT) {
		state->nested_state[state->depth] = IN_OBJECT2;
	}
	state->nested_state[++state->depth] = IN_ARRAY;
}

void json_end_arr(struct json_state *state)
{
	assert(state->depth >= 0);
	assert(state->nested_state[state->depth] ==			\
		IN_ARRAY || state->nested_state[state->depth] == IN_ARRAY2);

	state->write(state, "]", 1);
	state->nested_state[state->depth--] = DEFAULT;
}

void json_key(struct json_state *state, char * key)
{
	assert(state->nested_state[state->depth] ==			\
		IN_OBJECT || state->nested_state[state->depth] == IN_OBJECT2);

	if (state->nested_state[state->depth] == IN_OBJECT2) {
		state->write(state, ", ", 1);
	}
	state->write(state, "\"", 1);
	json_escape(state, key, strlen(key));
	state->write(state, "\":", 2);
}

void json_string(struct json_state *state, char * value)
{
	assert(state->nested_state[state->depth] != DEFAULT);

	if (state->nested_state[state->depth] == IN_ARRAY2) {
		state->write(state, ", ", 2);
	}
	if (state->nested_state[state->depth] == IN_ARRAY) {
		state->nested_state[state->depth] = IN_ARRAY2;
	}
	if (state->nested_state[state->depth] == IN_OBJECT) {
		state->nested_state[state->depth] = IN_OBJECT2;
	}
	state->write(state, "\"", 1);
	json_escape(state, value, strlen(value));
	state->write(state, "\"", 1);
}

void json_literal(struct json_state *state, char * format, ...)
{
	assert(state->nested_state[state->depth] != DEFAULT);

	if (state->nested_state[state->depth] == IN_ARRAY2) {
		state->write(state, ", ", 2);
	}
	if (state->nested_state[state->depth] == IN_ARRAY) {
		state->nested_state[state->depth] = IN_ARRAY2;
	}
	if (state->nested_state[state->depth] == IN_OBJECT) {
		state->nested_state[state->depth] = IN_OBJECT2;
	}
	va_list args;
	va_start(args, format);
	char buf[JSON_MAX_VALUE];
	size_t bufsz = vsprintf(buf, format, args);
	state->write(state, buf, bufsz);
	va_end(args);
}
