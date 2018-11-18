/*
 *  Copyright (C) 2015, 2016  Victor Li (vli02@hotmail.com)
 *
 *  This file is part of inthsmp/hsmp.
 *
 *  inthsmp/hsmp is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lex.h"
#include "defines.h"
#include "parser.h"

#define EC 0      /* error character */

extern const char *input_filename;

/* global variables */
char         *yytext = NULL;
unsigned int  yylength = 0;
yyinput_t    *yyinput = NULL;

/* local variables */
static yyinput_t a_yyinput = { 0 };

typedef struct bigbuff_s {
    char             *buff;
    size_t            pos;
    size_t            sz;
    struct bigbuff_s *link;
} bigbuff_t;
static bigbuff_t *bigbuff = NULL;

static int readsaved = 0;
static int savedch = 0;
static int section = 0;

static void
close_file()
{
    if (yyinput && yyinput->fp) {
        //printf("Closing file %s...\n", yyinput->filename);
        fclose(yyinput->fp);
        yyinput->fp = NULL;
    }
}

static int
readchar()
{
    char ch;

    if (readsaved == 2) {
        return EC;           /* error */
    }

    if (readsaved == 1) {
        ch = savedch;
        readsaved = 0;
    } else {
        ch = fgetc(yyinput->fp);
    }

    if (ch == EOF) {
        return ch;
    }

    /* enlarge buff */
    if ((bigbuff->pos + 1) >= bigbuff->sz) {
        size_t newbuffsz = bigbuff->sz * 2;
        bigbuff_t *newbuff = malloc(sizeof(bigbuff_t));
        if (newbuff) {
            newbuff->buff = malloc(newbuffsz);
        }
        if (!newbuff || !newbuff->buff) {
            readsaved = 2;   /* error */
            fprintf(stderr, "Memory allocation for size %lu failed!\n",
                    sizeof(bigbuff_t) + newbuffsz);
            return EC;
        }
        memcpy(&newbuff->buff[0], &yytext[0], yylength);
        newbuff->buff[yylength] = 0;
        newbuff->pos = yylength;
        newbuff->sz = newbuffsz;
        newbuff->link = bigbuff;
        bigbuff = newbuff;
        yytext = &bigbuff->buff[0];
    }

    bigbuff->pos ++;

    yytext[yylength] = ch;
    yytext[++ yylength] = 0;
    if (ch == '\n') {
        yyinput->lineno ++;
    }
    return ch;
}

static void
print_invalid_ch(int c)
{
    fprintf(stderr, "%s:%d, Invalid character %c.\n",
            yyinput->filename,
            yyinput->lineno,
            c);
}

static void
print_unexpect_eof()
{
    fprintf(stderr, "%s:%d, Unexpected eof.\n",
            yyinput->filename,
            yyinput->lineno);
}

static int
parse_dash()
{
    int tok = 0;
    int c = readchar();

    if (c == '>') {
        tok = TOK_ENTER;
    } else if (c == '-') {
        tok = TOK_DASH;
    } else if (c != EC) {
        print_invalid_ch('-');
    }

    return tok;
}

static int
parse_exit()
{
    int tok = 0;
    int c = readchar();

    if (c == '-') {
        tok = TOK_EXIT;
    } else if (c == '<') {
        tok = TOK_DEFER;
    } else if (c != EC) {
        print_invalid_ch('<');
    }

    return tok;
}

static int
parse_from_super()
{
    int tok = 0;
    int c = readchar();

    if (c == '>') {
        tok = TOK_LFROM;
    } else if (c != EC) {
        print_invalid_ch(c);
    }

    return tok;
}

static int
parse_to_super()
{
    int tok = 0;
    int c = readchar();

    if (c == '|') {
        tok = TOK_LTO;
    } else if (c != EC) {
        print_invalid_ch(c);
    }

    return tok;
}

static int
parse_identifier()
{
    int tok = 0;
    int c;

    do {
        c = readchar();
    } while (c == '_' ||
             isalpha(c) ||
             isdigit(c));

    if (c != EC) {
        /* save it for use in next read */
        readsaved = 1;
        savedch = c;
        if (c == '\n') {
            yyinput->lineno --;
        }

        /* discard last read */
        yylength --;
        yytext[yylength] = 0;

        tok = TOK_IDENTIFIER;
    }

    return tok;
}

static int
parse_comment_2(int block)
{
    int tok = 0;
    int p, c;

    p = 0;
    do {
        c = readchar();
        if (c == '\n') {
            if (!block) {
                tok = TOK_L_COMMENT;
                break;
            }
        }
        if (block &&
            p == '*' &&
            c == '/') {
            tok = TOK_B_COMMENT;
            break;
        }
        p = c;
    } while (c != EOF &&
             c != EC);

    if (c == EOF) {
        print_unexpect_eof();
    }

    return tok;
}

static int
parse_comment()
{
    int tok = 0;

    int c = readchar();
    if (c == '/') {
        // line comment
        tok = parse_comment_2(0);
    } else if (c == '*') {
        /* block comment */
        tok = parse_comment_2(1);
    } else if (c != EC) {
        print_invalid_ch(c);
    }

    return tok;
}

static int
parse_char()
{
    int tok = 0;
    int c;
    int skip = 0;

    do {
        c = readchar();
        if (c == EC) {
            break;
        } else if (c == '\\' &&
            skip == 0) {
            skip = 1;
        } else if (skip == 1) {
            skip = 2;
        } else if (c == '\'') {
            tok = TOK_CHAR;
            break;
        } else if (skip == 0) {
            skip = 2;
        } else {
            print_invalid_ch(c);
            break;
        }
    } while (c != EOF &&
             c != '\n');

    return tok;
}

static int
parse_string()
{
    int tok = 0;
    int c;
    int skip = 0;

    do {
        c = readchar();
        if (c == EC) {
            break;
        } else if (skip == 1) {
            skip = 0;             /* reset skip */
        } else if (c == '\\') {
            skip = 1;             /* set skip */
        } else if (c == '\"') {
            tok = TOK_STRING;
            break;
        }
    } while (c != EOF &&
             c != '\n');

    /* not a valid string */
    if (tok == 0 &&
        c != EC) {
        print_invalid_ch('\"');
    }

    return tok;
}

static int
parse_c_stmt_2(int predef)
{
    int nesting = 0;
    int tok = 0;
    int p, c;

    p = 0;
    do {
        c = readchar();
        if (c == '\n') {
            tok = c;
        } else if (c == '/') {
            tok = parse_comment();
        } else if (c == '\'') {
            tok = parse_char();
        } else if (c == '\"') {
            tok = parse_string();
        } else if (c == '{') {
            nesting ++;
            tok = c;
        } else if (c == '}') {
            if (nesting > 0) {
                nesting --;
                tok = c;
            } else {
                if (predef == 0 || p == '%') {
                    if (predef != 0) {
                        yylength -= 2;
                        yytext[yylength] = 0;
                    }
                    tok = TOK_C_STMT;
                } else {
                    print_invalid_ch('}');
                    tok = 0;
                }
                break;
            }
        } else {
            tok = c;
        }
        p = c;
    } while (tok != 0 &&
             c != EC &&
             c != EOF);

    if (c == EOF) {
        print_unexpect_eof();
    }

    return tok;
}

static int
parse_c_stmt()
{
    return parse_c_stmt_2(0);
}

static int
parse_c_expr()
{
    int nesting = 0;
    int tok = 0;
    int c, p = 0;

    /* remove '?' */
    yytext[0] = ' ';

    do {
        c = readchar();
        if (c == '\n') {
            tok = c;
        } else if (c == '/') {
            tok = parse_comment();
        } else if (isspace(c)) {
            tok = c;
        } else if (!p && c != '(') {
            print_invalid_ch(c);
            tok = 0;
        } else if (c == '\'') {
            tok = parse_char();
        } else if (c == '\"') {
            tok = parse_string();
        } else if (c == '(') {
            if (!p) {
                p = c;
            } else {
                nesting ++;
            }
            tok = c;
        } else if (c == ')') {
            if (nesting > 0) {
                nesting --;
                tok = c;
            } else {
                tok = TOK_C_EXPR;
                break;
            }
        }
    } while (tok != 0 &&
             c != EC &&
             c != EOF);

    if (c == EOF) {
        print_unexpect_eof();
    }
 
    return tok;
}

static int
parse_predef()              /* %{, %}, %%, %keyword */
{
    int tok = 0;

    int c = readchar();
    if (c == '{') {
        /* parse c statement */
        yytext = &bigbuff->buff[++bigbuff->pos];    /* reset yytext */
        yylength = 0;
        tok = parse_c_stmt_2(1);
    } else if (c == '%') {
        section ++;
        tok = TOK_PP;
    } else if (isalpha(c)) {
        /*
         * %state, %event, %hiera, %start, %entry, %exit, %defer, %guard, %action
         */
        int t = parse_identifier();
        if (t == TOK_IDENTIFIER) {
            if (strcmp(yytext, "%state") == 0) {
                tok = TOK_STATE;
            } else if (strcmp(yytext, "%prefix") == 0) {
                tok = TOK_PREFIX;
            } else if (strcmp(yytext, "%manual_event_id") == 0) {
                tok = TOK_MEID;
            } else if (strcmp(yytext, "%manual_state_id") == 0) {
                tok = TOK_MSID;
            } else if (strcmp(yytext, "%event") == 0) {
                tok = TOK_EVENT;
            } else if (strcmp(yytext, "%hiera") == 0) {
                tok = TOK_HIERA;
            } else if (strcmp(yytext, "%start") == 0) {
                tok = TOK_START;
            } else if (strcmp(yytext, "%guard") == 0) {
                tok = TOK_GUARD;
            } else if (strcmp(yytext, "%action") == 0) {
                tok = TOK_ACTION;
            }
        }
    }

    if (c != EC && tok == 0) {
        fprintf(stderr, "%s:%d, Invalid predefined key: %s.\n",
                yyinput->filename,
                yyinput->lineno,
                yytext);
    }

    return tok;
}

int
init_lex()
{
    FILE *fp;

    yyinput = &a_yyinput;

    //printf("Opening file %s...\n", input_filename);
    fp = fopen(input_filename, "r");
    if (fp) {
        yyinput->fp = fp;
        yyinput->filename = input_filename;
        yyinput->lastlineno = 1;
        yyinput->lineno = 1;
    } else {
        fprintf(stderr, "File not found: %s!\n", input_filename);
        return -1;
    }

    bigbuff = malloc(sizeof(bigbuff_t));
    if (bigbuff == NULL) {
        fprintf(stderr, "Failed to allocate memory of size %lu!\n", sizeof(bigbuff_t));
        return -1;
    }
    bigbuff->link = NULL;
    bigbuff->pos = 0;
    bigbuff->sz = 100 * 1024;
    bigbuff->buff = malloc(bigbuff->sz);
    if (bigbuff->buff == NULL) {
        fprintf(stderr, "Failed to allocate memory of size %lu!\n", bigbuff->sz);
        return -1;
    }
    yytext = &bigbuff->buff[0];
    yytext[0] = 0;
    yylength = 0;

    return 0;
}

void
deinit_lex()
{
    bigbuff_t *p;
    while (bigbuff) {
        p = bigbuff->link;
        if (bigbuff->buff) {
            free(bigbuff->buff);
        }
        free(bigbuff);
        bigbuff = p;
    }
    yytext = NULL;
    yylength = 0;

    close_file();
}

void
print_errmsg(const char *s)
{
    fprintf(stderr, "Line %d: (%s): %s!\n",
            yyinput->lineno,
            yytext,
            s);
}

void
print_token(int token)
{
    printf("%d:\t", yyinput->lastlineno);

    if (token <= 255) {
        printf("%c\n", token);
    } else {
        printf("%d\t%s\n", token, yytext);
    }
}

int
yylex_internal()
{
    int c;
    int token = 0;

again:
    yyinput->lastlineno = yyinput->lineno;
    yytext = &bigbuff->buff[++bigbuff->pos];    /* reset yytext */
    yylength = 0;

    c = readchar();
    if (c == EOF ||
        c == 0 /* error */) {
        token = 0;
    } else if (c == '\n') {
        goto again;
    } else if (isspace(c)) {
        goto again;
    } else if (c == '/') {
        token = parse_comment();
        if (token == TOK_L_COMMENT ||
            token == TOK_B_COMMENT) {
            goto again;
        }
    } else if (c == '%') {
        token = parse_predef();
    } else if (c == '\'') {
        token = parse_char();
    } else if (c == '\"') {
        token = parse_string();
    } else if (c == '{') {
        token = parse_c_stmt();
    } else if (c == '?') {
        token = parse_c_expr();
    } else if (c == '-') {
        token = parse_dash();
    } else if (c == '<') {
        token = parse_exit();
    } else if (c == '|') {
        token = parse_from_super();
    } else if (c == '>') {
        token = parse_to_super();
    } else if (c == ':' ||
               c == '.' ||
               c == ',' ||
               c == '*' ||
               c == '(' ||
               c == ')' ||
               c == ';') {
        token = c;
    } else if (isalpha(c) ||
               c == '_') {
        token = parse_identifier();
    } else {
        /* invalid character */
        print_invalid_ch(c);
        token = 0;
    }

    return token;
}

int
yylex()
{
    int token = 0;
    int ch;

    if (section < 2) {
        token = yylex_internal();
    } else if (readsaved != 2) {
        yyinput->lastlineno = yyinput->lineno;
        yytext = &bigbuff->buff[++bigbuff->pos];    /* reset yytext */
        yylength = 0;
        do {
            ch = readchar();
            if (ch == EOF) {
                /* discard last read */
                yylength --;
                yytext[yylength] = 0;
                readsaved = 2;
                token = TOK_C_STMT;
                break;
            }
        } while (ch != EC);
    }

    return (token != 0) ? token : EOF;
}
