#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "lexer.h"


#ifdef DEBUG
# define xdebug(format, ...) \
    do { fprintf(stderr, "[debug] "format, ##__VA_ARGS__); } while (0)
#else
# define xdebug(format, ...)
#endif


#define XTOKEN(t) ( (l->ttype = t), t )


/* routines */

const char * token2str(int index) 
{
    assert(index <= TOK_EOF && index >= 0);

    return token_string_ary[index];
}

void lexer_state_init(lexer_state *l, FILE *stream, const char *filename)
{
    l->stream = stream;

    if (filename) {
        l->filename = strdup(filename);
    }

    l->source = NULL;
    l->source_len = 0;
    l->_ptr = NULL;
    l->curc = '\0';
    l->toklen = 0;
    l->lineno = 1;
    l->column = 0;
    l->ttype = -1;
}

void lexer_state_destroy(lexer_state *l) 
{
    if (l->stream) {
        fclose(l->stream);
    }

    // free the buffer
    if (l->_ptr) {
        free(l->_ptr);
        l->source = NULL;
    }

    if (l->filename) {
        free(l->filename);
    }
}

static void token_finish(lexer_state *l)
{
    if (l->toklen >= 1024) {
        // trigger error
    }
    l->tokbuf[l->toklen] = '\0';
}

static void token_pushc(lexer_state *l, int c)
{
    if (l->toklen < TOKEN_MAX_LEN) {
        l->tokbuf[l->toklen++] = c;
    }
}

/* if source buffer in lexer_state is empty, read a block
 * from the stream(file|stdin) into the buffer */
void new_source(lexer_state *l, const char *s)
{
    long end, curr, size;

    if (l->_ptr != NULL) {
        xdebug("free _ptr\n");
        free(l->_ptr);
        l->_ptr = NULL;
    }

    if (l->stream) {
        curr = ftell(l->stream);
        if (fseek(l->stream, 0L, SEEK_END) != 0) {
            // handle error
            xdebug("Error\n");
        }

        end = ftell(l->stream);

        size = end - curr;

        if (size == 0) {
            // reaching the end of file
            l->curc = EOF;
            return;
        }

        // go back to the current postion of the file
        if (fseek(l->stream, curr, SEEK_SET) != 0) {
            // handle error
            xdebug("Error in %d\n", __LINE__);
        }
        
        if (size > SOURCE_MAX_LEN) {
            size = SOURCE_MAX_LEN;
        }

        l->source = malloc(sizeof(char) * (size+1));
        l->source_len = fread(l->source, sizeof(char), size, l->stream);

        if (l->source_len == 0 && !ferror(l->stream)) {
            l->curc = EOF;
        } else {
            // handle error
        }

        l->source[l->source_len] = '\0';
    } else if (s) {
        l->source_len = strlen(s);
        l->source = strdup(s);
    } else {
        l->curc = EOF;
        return;
    }

    l->_ptr = l->source;

    /* please close the file
     * and free the source in the end
     */
}


// get next char
static inline int nextc(lexer_state *l) 
{
    int c;
begin:
    if (l->source_len == 0 && l->curc != EOF) {
        new_source(l, NULL);
        goto begin;
    } else if (l->source_len == 0 && l->curc == EOF) {
        return -1;
    }

    assert(l->source != NULL);

    c = *l->source++;
    l->source_len--;

    if (c == '\n') {
        l->lineno++;
        l->column = 0;
    } else {
        l->column++;
    }

    l->curc = c;
    l->tokbuf[l->toklen++] = c;
    return c;
}

static void until(lexer_state *l, int ch)
{
    int c;
    while ( (c = nextc(l)) != ch) {
    }
}


// !fixme
// something wrong
// after unputc, the last character is changed.
/*
NOTE: the `unputc` func doesn't remove the last character
we can set the last character to '\0' by l->tokbuf[l->toklen],
but currently it's not necessary.
So, in the `lexme` func, we don't use l->tokbuf if the l->ttype
is not a TOK_STRING|TOK_NUMBER|TOK_IDENT etc.
*/
static inline void unputc(lexer_state *l)
{
    if (l->curc == '\n') return;
    l->column--;

    l->source--;
    l->source_len++;

    l->toklen--;

}


/*
 * fixd: missing the first character, add token type.
 */

static int scan_ident(lexer_state *l)
{
    int c;

    while ( (c = nextc(l) )) {
        if (! isdigit(c) && ! isalpha(c) && c != '_') {
            unputc(l);
            break;
        }
    }
    token_finish(l);

    return XTOKEN(TOK_IDENT);
}

static int scan_digit(lexer_state *l)
{
    int c;

    while ( (c = nextc(l) )) {
        if (! isdigit(c)) {
            unputc(l);
            break;
        }
    }

    token_finish(l);

    return XTOKEN(TOK_NUMBER);
}

// skip characters
static void skip(lexer_state *l, char ch)
{
    int c;

    while ( (c = nextc(l)) != ch) {
    }
}

static int scan_string(lexer_state *l, int cb)
{
    until(l, cb);
    token_finish(l);

    return XTOKEN(TOK_STRING);
}

/*
if the ttype is not TOK_STRING|TOKNUMBER|TOK_IDENT
then the tokbuf will be a string without terminate 
character. 
see `unputc` doc
*/
const char* lexme(lexer_state *l)
{
    if (l->ttype == TOK_STRING ||
        l->ttype == TOK_NUMBER ||
        l->ttype == TOK_IDENT) {
        return l->tokbuf;
    }

    return token2str(l->ttype);
}

const char* token(lexer_state *l) {
    return token2str(l->ttype);
}

static inline void new_token(lexer_state *l)
{
    if (l->tokbuf[0] != '0') {
        memset(l->tokbuf, 0, TOKEN_MAX_LEN);
    }
    l->toklen = 0;
    l->ttype = -1;
}

int scan_token(lexer_state *l)
{
    int c;
start:
    new_token(l);
    switch (c = nextc(l)) {
    case '\0':
    case '\004':
    case '\032':
    case -1:
        return XTOKEN(TOK_EOF);

    case ' ':
    case '\t':
    case '\f':
    case '\r':
    case '\13':
        // see whitespace? restart
        goto start;

    case '\n':
        goto start;

    // comment
    case '#':
        skip(l, '\n');
        goto start;

    case '{': return XTOKEN(TOK_LBRACE);
    case '}': return XTOKEN(TOK_RBRACE);
    case '(': return XTOKEN(TOK_LPAREN);
    case ')': return XTOKEN(TOK_RPAREN);
    case '[': return XTOKEN(TOK_LBRACK);
    case ']': return XTOKEN(TOK_RBRACK);
    case ':': return XTOKEN(TOK_COLON);
    case '?': return XTOKEN(TOK_QMARK);
    case ';': return XTOKEN(TOK_SEMICOLON);
    case ',': return XTOKEN(TOK_COMMA);
    case '.': return XTOKEN(TOK_DOT);
    case '!': 
        switch (nextc(l)) {
            case '=': return XTOKEN(TOK_NEQ);
            default:
                unputc(l); return XTOKEN(TOK_NOT);
        }
    case '=':
        switch (nextc(l)) {
            case '=': return XTOKEN(TOK_EQ);
            default:
                unputc(l); return XTOKEN(TOK_ASSIGN);
        }
    case '+':
        switch (nextc(l)) {
            case '+': return XTOKEN(TOK_INCR);
            case '=': return XTOKEN(TOK_PLUS_ASSIGN);
            default:
                unputc(l); return XTOKEN(TOK_PLUS);
        }
    case '-':
        switch(nextc(l)) {
            case '-': return XTOKEN(TOK_DECR);
            case '=': return XTOKEN(TOK_MINUS_ASSIGN);
            default:
                unputc(l); return XTOKEN(TOK_MINUS);
        }
    case '*':
        switch(nextc(l)) {
            case '*': return XTOKEN(TOK_POW);
            case '=': return XTOKEN(TOK_MUL_ASSIGN);
            default:
                unputc(l); return XTOKEN(TOK_MUL);
        }
    case '/':
        switch(nextc(l)) {
            case '=': return XTOKEN(TOK_DIV_ASSIGN);
            default: unputc(l); return XTOKEN(TOK_DIV);
        }
    case '%': return XTOKEN(TOK_MOD);
    case '|':
        switch(nextc(l)) {
            case '|': return '=' == nextc(l) ? XTOKEN(TOK_OR_ASSIGN) : (unputc(l), XTOKEN(TOK_OR));
            default: return unputc(l), XTOKEN(TOK_BIT_OR);
        }
    case '<':
        switch (nextc(l)) {
            case '=': return XTOKEN(TOK_LTE);
            case '<': return XTOKEN(TOK_BIT_SHL);
            default: return unputc(l), XTOKEN(TOK_LT);
        }
    case '>':
        switch (nextc(l)) {
            case '=': return XTOKEN(TOK_GTE);
            case '>': return XTOKEN(TOK_BIT_SHR);
            default: return unputc(l), XTOKEN(TOK_GT);
        }

    // scan string
    case '"':
    case '\'':
        return scan_string(l, c);

    default:
        if (isalpha(c) || c == '_') return scan_ident(l);
        if (isdigit(c) || c == '.') return scan_digit(l);
    } /* end switch */
    
    return XTOKEN(TOK_EOF);
}

int
main(int argc, char *argv[]) 
{
    if (argc < 2) {
        fprintf(stderr, "Missing input file.\n");
        return -1;
    }

    const char* input = argv[1];
    lexer_state l;
    FILE* fp;

    fp = fopen(input, "r");
    assert(fp != NULL);

    lexer_state_init(&l, fp, input);

    while (scan_token(&l) != TOK_EOF) {
        printf("%d %s: %s\n", l.lineno, token(&l), lexme(&l));
    }

    lexer_state_destroy(&l);

    return 0;
}
