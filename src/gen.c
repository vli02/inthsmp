/*
 *  Copyright (C) 2015, 2016, 2017, 2018  Victor Li (vli02@hotmail.com)
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
//#include <stdarg.h>
#include <assert.h>
#include "gen.h"

extern const char *input_filename;

extern char *g_output_fname;
extern int   g_opt_d;
extern int   g_opt_o;
extern int   g_opt_v;

char *output_file_name = NULL;
FILE *output_file_ptr  = NULL;

char *output_h_name    = NULL;
FILE *output_h_fptr    = NULL;

char *output_v_name    = NULL;
FILE *output_v_fptr    = NULL;


int
init_output()
{
    int retval = 0;
    int i;
    const char *base;
    char *fname, *hname, *vname;
    char tab, dot, x;
    FILE *ofile, *hfile, *vfile;

    fname = hname = vname = NULL;
    ofile = hfile = vfile = NULL;

    if (g_opt_o != 0 &&
        g_output_fname != NULL) {
        tab = 0;
        base = g_output_fname;
    } else {
        tab = 1;
        base = input_filename;
    }
    i = strlen(base) + 1;
    assert(i > 1);
    fname = malloc(i + 16);
    if (!fname) {
        fprintf(stderr, "Failed to allocate memory of size %d.\n", i + 16);
        retval = -1;
        goto output_err;
    }
    if (g_opt_d != 0) {
        hname = malloc(i + 16);
        if (!fname) {
            fprintf(stderr, "Failed to allocate memory of size %d.\n", i + 16);
            retval = -1;
            goto output_err;
        }
    }
    if (g_opt_v != 0) {
        vname = malloc(i + 16);
        if (!fname) {
            fprintf(stderr, "Failed to allocate memory of size %d.\n", i + 16);
            retval = -1;
            goto output_err;
        }
    }

    strcpy(fname, base);
    if (tab) {
        strcat(fname, ".tab");
        i += 4;
    }

    /* append or replace with .c if needed */
    if (i > 3) {
        dot = fname[i - 3];
        x = fname[i - 2];
    } else {
        dot = 0;
    }
    if (dot == 0 ||
        dot != '.' || (x != 'c' && x != 'h')) {
        fname[i - 1] = '.';
        fname[i ++] = 'c';
        fname[i ++] = 0;
    } else if (x == 'h') {
        fname[i - 2] = 'c';
        g_opt_d = 1;
    }

    /* open .c file for writing */
    ofile = fopen(fname, "w");
    if (ofile == NULL) {
        fprintf(stderr, "Failed to open file for writing: %s.\n", fname);
        retval = -1;
        goto output_err;
    }

    /* open .h file for writing */
    if (hname) {
        strcpy(hname, fname);
        /* replace with .h */
        hname[i - 2] = 'h';

        hfile = fopen(hname, "w");
        if (hfile == NULL) {
            fprintf(stderr, "Failed to open file for writing: %s.\n", hname);
            retval = -1;
            goto output_err;
        }
    }

    /* open .output file for writing */
    if (vname) {
        strcpy(vname, fname);
        /* replace with .h */
        vname[i - 3] = 0;
        strcat(vname, ".output");

        vfile = fopen(vname, "w");
        if (vfile == NULL) {
            fprintf(stderr, "Failed to open file for writing: %s.\n", vname);
            retval = -1;
        }
    }

output_err:
    if (retval != 0) {
        free(fname);
        if (hname) free(hname);
        if (vname) free(vname);
    } else {
        output_file_name = fname;
        output_h_name = hname;
        output_v_name = vname;
        output_file_ptr = ofile;
        output_h_fptr = hfile;
        output_v_fptr = vfile;
    }
    return retval;
}

void
deinit_output(int bad)
{
    if (output_v_fptr) {
        fclose(output_v_fptr);
    }
    if (output_h_fptr) {
        fclose(output_h_fptr);
    }
    if (output_file_ptr) {
        fclose(output_file_ptr);
    }
    if (output_v_name) {
        if (bad) {
            unlink(output_v_name);
        }
        free(output_v_name);
        output_v_name = NULL;
    }
    if (output_h_name) {
        if (bad) {
            unlink(output_h_name);
        }
        free(output_h_name);
        output_h_name = NULL;
    }
    if (output_file_name) {
        if (bad) {
            unlink(output_file_name);
        }
        free(output_file_name);
        output_file_name = NULL;
    }
}
