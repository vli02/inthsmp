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
#include <libgen.h>
#include <getopt.h>
#include <assert.h>

#include "lex.h"
#include "defines.h"
#include "gen.h"

extern int yyparse();
extern void check_license(int);
extern void print_license(int);
extern void add_license();

const int prod_ver[3] = { 1, 0, 0 };
const char *prod_name = "Intuitive Hierarchical State Machine Programming";

static const char *optString = "do:l:W:nvShVLAi";
static const struct option longOpts[] = {
    { "define", no_argument, NULL, 'd' },
    { "output", required_argument, NULL, 'o' },
    { "lang", required_argument, NULL, 'l' },
    { "Warning", required_argument, NULL, 'W' },
    { "no_lines", no_argument, NULL, 'n' },
    { "verbose", no_argument, NULL, 'v' },
    { "Strings", no_argument, NULL, 'S' },
    { "help", no_argument, NULL, 'h' },
    { "version", no_argument, NULL, 'V' },
    { "license", no_argument, NULL, 'L' },
    { "add_license", no_argument, NULL, 'A' },
    { "silent", no_argument, NULL, 'i' },
    { 0, 0, 0, 0 }
};

const char *input_filename   = NULL;

/* option parameters */
char *g_output_fname = NULL;
int   g_opt_d        = 0;
int   g_opt_o        = 0;
int   g_opt_n        = 1;
int   g_opt_v        = 0;
int   g_opt_S        = 0;
int   g_opt_W        = 0xFF;

static void
print_usage(const char *exename)
{
    static int usage_printed = 0;

    if (usage_printed == 0) {
        printf("usage: %s [options] <input file>\n", exename);
    }
    usage_printed = 1;
}

static void
print_version()
{
    printf("%s\n", prod_name);
    printf("Version %d.%d.%d.\n", prod_ver[0], prod_ver[1], prod_ver[2]);
    printf("Copyright (C) 2016 Victor Li (vli02@hotmail.com).\n");
}

static void
print_help(const char *exename)
{
    static int help_printed = 0;

    if (help_printed != 0) {
        return;
    }
    help_printed = 1;

    print_usage(exename);

    /* option details */
    printf("\noptions:\n");
    printf("\t-d, --define\t\tproduce a header file.\n");
    printf("\t-o, --output <file>\tspecify output file name.\n");
    printf("\t-W, --Warning <n>\tspecify warning level, default warning level is all warnings.\n");
    printf("\t-n, --no_lines\t\tdo not generate `#line' directives.\n");
    printf("\t-v, --verbose\t\tproduce a .output file.\n");
    printf("\t-S, --Strings\t\tgenerate string tables and utility functions for events and states.\n");
    printf("\t-h, --help\t\tprint this help.\n");
    printf("\t-V, --version\t\tprint version information.\n");
    printf("\t-i, --silent\t\tdo not show version and license information.\n");
    printf("\t-L, --license\t\tprint license information.\n");
    printf("\t-A, --add_license\tapply a license to the software.\n");
}

static void
parse_option(int argc, char *const *argv)
{
    int opt;
    int longIndex;
    int where = 1;
    char *p;

    int o_V = 0;
    int o_i = 0;
    int o_L = 0;
    int o_A = 0;
    int o_h = 0;
    char *unknown_opt = NULL;

    opterr = 0;
    longIndex = 0;
    while ((opt = getopt_long(argc, argv, optString, longOpts, &longIndex)) != -1) {
        where ++; // optind, optarg have trouble on mac.
        switch (opt) {
            case 'd':
                g_opt_d = 1;
                break;
            case 'o':
                g_opt_o = 1;
                g_output_fname = argv[where];
                where ++;
                break;
            case 'l':
                break;
            case 'W':
                p = argv[where];
                g_opt_W = atoi(p);
                where ++;
                break;
            case 'n':
                g_opt_n = 0;
                break;
            case 'v':
                g_opt_v = 1;
                break;
            case 'S':
                g_opt_S = 1;
                break;
            case 'V':
                o_V = 1;
                break;
            case 'i':
                o_i = 1;
                break;
            case 'L':
                o_L = 1;
                break;
            case 'A':
                o_A = 1;
                break;
            case 'h':
                o_h = 1;
                break;
            case '?':
            default:
                unknown_opt = argv[where - 1];
                o_h = 1;
                break;
        }
    }

    if (o_L) {
        o_i = 0;
    }
    if (o_i == 0 || o_V) {
        print_version();
        printf("\n");
    }
    check_license(o_i);
    if (unknown_opt) {
        printf("Invalid option: %s\n", unknown_opt);
    }
    if (o_L) {
        print_license(1);
        printf("\n");
    }
    if (o_h) {
        print_help(argv[0]);
    }
    if (o_A) {
        add_license();
    }

    if (argc == 1 ||
        (where + 1) != argc) {
        print_usage(argv[0]);
        exit(0);
    } else {
        input_filename = argv[where];
    }
}

int
main(int argc, char **argv)
{
    int retval = 0;

    /* parse options */
    parse_option(argc, argv);

    if (init_lex() != 0) {
        retval = -1;
        goto cu0;
    }

    if (init_output() != 0) {
        retval = -1;
        goto cu0;
    }

    if (init_objects() != 0) {
        retval = -1;
        goto cu0;
    }

    /* yyparse */
#if 0
    while (1) {
        int token = yylex();
        if (token <= 0) {
            break;
        } else {
            print_token(token);
        }
    }
#else
    if (yyparse() != 0) {
        retval = -1;
        goto cu0;
    }
    gen_output();
#endif

cu0:
    free_objects();
    deinit_output(retval);
    deinit_lex();

    return retval;
}
