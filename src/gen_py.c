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

extern event_t *wildc_ev;
extern state_t *start_st;
extern link_t dest_link;
extern text_t     *prolog_code;
extern text_t     *epilog_code;
extern text_t     *start_code;

extern FILE *output_file_ptr;
extern FILE *output_file;

extern int max_eid;
extern int max_sid;
extern int max_leaf_sid;
extern int max_super_sid;


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

static void
print_event_list()
{
    int i;
    event_t *ev;

    write2file("__hh_events = [");
    i = 0;
    while (i <= max_eid) {
        ev = find_event_by_eid(i);
        if (ev != wildc_ev) {
            if (i > 0) {
                write2file(",");
            }
            write2file("\n    \"%s\"", ev->name->txt);
        }
        i ++;
    }
    write2file("\n]\n\n");
}

static void
print_state_list()
{
    int i;
    state_t *st;

    write2file("__hh_states = [");
    i = 0;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        if (i > 0) {
            write2file(",");
        }
        write2file("\n    \"%s\"", st->name->txt);
        i ++;
    }
    write2file("\n]\n\n");
}

static void
print_super_states()
{
    int i, sid;
    state_t *st;

    write2file("__hh_super = [");
    i = 0;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        assert(st);
        sid = (st->super) ? st->super->id : -1;
        if (i != 0) {
            write2file(",");
        }
        if ((i % 8) == 0) {
            write2file("\n");
        }
        write2file("%4d", sid);
        i ++;
    }
    write2file("\n]\n\n");
}

static void
print_sub_states()
{
    int i, j, sid;
    state_t *st;

    write2file("__hh_sub = [");
    i = max_leaf_sid + 1;
    j = 0;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        assert(st && st->sub);
        sid = st->sub->id;
        if (j != 0) {
            write2file(",");
        }
        if ((j % 8) == 0) {
            write2file("\n");
        }
        write2file("%4d", sid);
        i ++;
        j ++;
    }
    write2file("\n]\n\n");
}

static int
print_parent_sid(state_t *st)
{
    int n = 0;
    if (st->super) {
        n = print_parent_sid(st->super);
    }
    if (n != 0) {
        write2file(",");
        if ((n % 8) == 0) {
            write2file("\n ");
        }
    }
    write2file("%4d", st->id);
    return ++n;
}

static void
print_entry_path()
{
    int i;
    state_t *st;

    write2file("__hh_entry_path = [");
    i = 0;
    while (i <= max_leaf_sid) {
        st = find_state_by_sid(i);
        if (i != 0) {
            write2file(",");
        }
        write2file("\n[");
        print_parent_sid(st);
        write2file("]");
        i ++;
    }
    write2file("\n]\n\n");
}


int append_ext_py(char *str, int len)
{
    /* append .py if needed */
    if (len <= 3 ||
        str[len - 3] != '.' ||
        str[len - 2] != 'p' ||
        str[len - 1] != 'y') {
        str[len ++] = '.';
        str[len ++] = 'p';
        str[len ++] = 'y';
        str[len   ] = 0;
    }

    return len;
}

void
gen_code_py()
{
    output_file = output_file_ptr;
    print_prod_info();
    print_prolog();

    print_event_list();
    print_state_list();
    print_super_states();
    print_sub_states();
    print_entry_path();

    print_epilog();
}
