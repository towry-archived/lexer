
// Selang

#ifndef _LEXER_H
#define _LEXER_H

#include <assert.h>
#include <stdio.h>

#define SE_TOKEN
#define TOKEN_MAX_LEN 1024

#define TOKEN_TYPE_LIST \
    tok(TOK_IDENT, "<identifier>") \
    tok(TOK_STRING, "<string>") \
    tok(TOK_NUMBER, "<number>") \
    tok(TOK_LBRACE, "{") \
    tok(TOK_RBRACE, "}") \
    tok(TOK_LPAREN, "( ") \
    tok(TOK_RPAREN, ")") \
    tok(TOK_LBRACK, "[") \
    tok(TOK_RBRACK, "]") \
    tok(TOK_COLON, ":") \
    tok(TOK_QMARK, "?") \
    tok(TOK_SEMICOLON, ";") \
    tok(TOK_COMMA, ",") \
    tok(TOK_DOT, ".") \
    tok(TOK_NOT, "!") \
    tok(TOK_SNOT, "not") \
    tok(TOK_PLUS, "+ ") \
    tok(TOK_INCR, "++") \
    tok(TOK_MINUS, "- ") \
    tok(TOK_DECR, "-- ") \
    tok(TOK_MUL, "* ") \
    tok(TOK_DIV, "/ ") \
    tok(TOK_MOD, "%") \
    tok(TOK_POW, "** ") \
    tok(TOK_GT, "> ") \
    tok(TOK_LT, "< ") \
    tok(TOK_ASSIGN, "=") \
    tok(TOK_GTE, ">= ") \
    tok(TOK_LTE, "<= ") \
    tok(TOK_EQ, "==") \
    tok(TOK_NEQ, "!= ") \
    tok(TOK_AND, "&& ") \
    tok(TOK_OR, "||") \
    tok(TOK_PLUS_ASSIGN, "+=") \
    tok(TOK_MINUS_ASSIGN, "-=") \
    tok(TOK_MUL_ASSIGN, "*=") \
    tok(TOK_DIV_ASSIGN, "/=") \
    tok(TOK_OR_ASSIGN, "||=") \
    tok(TOK_BIT_AND, "&") \
    tok(TOK_BIT_OR, "|") \
    tok(TOK_BIT_SHL, "<<") \
    tok(TOK_BIT_SHR, ">>") \
    tok(TOK_EOF, "<EOF>")

typedef enum {
#define tok(t, s) t,
TOKEN_TYPE_LIST
#undef tok
} token_type;

static const char* token_string_ary[] = {
#define tok(t, s) s,
TOKEN_TYPE_LIST
#undef tok
};


const char *token2str(int);

#define SE_LEXER
#define SOURCE_MAX_LEN 1024L

// the lexer state
typedef struct {
    FILE *stream;
    char *source; /* source buffer */
    char *_ptr;
    int source_len; /* source len */

    token_type ttype;
    char tokbuf[TOKEN_MAX_LEN];
    int toklen;
    char curc;

    int lineno;
    int column;
    char *filename;
} lexer_state;

void lexer_state_init(lexer_state *, FILE *, const char *);
void lexer_state_destroy(lexer_state *);
int scan_token(lexer_state *);
const char *token(lexer_state *);
const char *lexme(lexer_state *);
#endif
