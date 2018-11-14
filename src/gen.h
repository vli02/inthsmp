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
#ifndef __GEN_H__
#define __GEN_H__

#include "defines.h"

#define OUTPUT_LANG_C		0
#define OUTPUT_LANG_PY		1
#define OUTPUT_LANG_NULL	2

void write2file(const char *, ...);

int init_output();
void deinit_output();
void gen_output();

int append_ext_c(char *, int);
int append_ext_py(char *, int);

void gen_code_c();
void gen_code_py();

#endif // __GEN_H__
