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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "gen.h"

#define WRITE_PREFIX(PREFIX) \
do {\
    if (PREFIX) {\
        write2file("%s", PREFIX);\
    }\
} while (0);

extern const int  prod_ver[3];
extern const char *prod_name;

extern text_t     *prolog_code;
extern text_t     *epilog_code;

extern FILE *output_file_ptr;
extern FILE *output_file;


static void
print_prod_info()
{
    write2file("# Generated code, don't edit.\n");
    write2file("# %s\n", prod_name);
    write2file("# Version %d.%d.%d\n\n", prod_ver[0], prod_ver[1], prod_ver[2]);
}

static void
print_prolog()
{
    if (prolog_code) {
        write2file("%s\n\n", prolog_code->txt);
    }
}

static void
print_epilog()
{
    if (epilog_code) {
        write2file("%s\n", epilog_code->txt);
    }
}


void
gen_code_py()
{
    output_file = output_file_ptr;
    print_prod_info();
    print_prolog();

    print_epilog();
}
