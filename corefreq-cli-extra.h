/*
 * CoreFreq (C) 2015-2020 CYRIL INGENIERIE
 * Contributors: Andrew Gurinovich ; CyrIng
 * Licenses: GPL2
 *
 * Some ideas taken from github.com/cesanta/frozen
 * under Apache 2.0 license
 */

#define JSON_MAX_DEPTH 32
#define JSON_MAX_VALUE 4096

struct json_state {
	int (*write)(struct json_state* state, const char *str, size_t len);
	uint8_t nested_state[JSON_MAX_DEPTH];
	uint8_t depth;
};

extern int json_writer_stdout(	struct json_state * state,
				const char *str,
				size_t len) ;
extern void json_start_object(struct json_state *state) ;
extern void json_end_object(struct json_state *state) ;
extern void json_start_arr(struct json_state *state) ;
extern void json_end_arr(struct json_state *state) ;
extern void json_key(struct json_state *state, char * key) ;
extern void json_string(struct json_state *state, char * value) ;
extern void json_literal(struct json_state *state, char * format, ...) ;
