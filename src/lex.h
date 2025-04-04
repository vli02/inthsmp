/*
 *  Copyright (C) 2015, 2016  Victor Li (vli02us@gmail.com)
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
#ifndef __LEX_H__
#define __LEX_H__

#include <stdio.h>

#include "gen.h"

extern int g_opt_l;

#define IS_C_COMMENT(C)  ((C) == '/'  && g_opt_l == OUTPUT_LANG_C)
#define IS_C_CHAR(C)     ((C) == '\'' && g_opt_l == OUTPUT_LANG_C)

#define IS_PY_COMMENT(C) ((C) == '#'  && g_opt_l == OUTPUT_LANG_PY)

#define IS_STRING(C)     ((C) == '\"' || ((C) == '\'' && g_opt_l == OUTPUT_LANG_PY))


typedef struct yyinput_s {
    FILE *fp;
    const char *filename;
    unsigned int lastlineno;
    unsigned int lineno;
    unsigned int lastcolno;
    unsigned int colno;
} yyinput_t;

int init_lex();
void deinit_lex();
void print_errmsg(const char *);
void print_token(int);
int yylex();

#endif /* __LEX_H__ */
