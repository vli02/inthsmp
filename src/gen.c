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
#include <stdarg.h>
#include <assert.h>
#include "gen.h"

extern const char *input_filename;

extern char *g_output_fname;
extern int   g_opt_d;
extern int   g_opt_o;
extern int   g_opt_v;
extern int   g_opt_l;

extern event_t *wildc_ev;
extern state_t *start_st;
extern link_t event_link;
extern link_t state_link;

char *output_file_name = NULL;
FILE *output_file_ptr  = NULL;
char *output_h_name    = NULL;
FILE *output_h_fptr    = NULL;
char *output_v_name    = NULL;
FILE *output_v_fptr    = NULL;
FILE *output_file      = NULL;

int max_eid      = -1;
int max_sid      = -1;
int max_leaf_sid = -1;
int max_super_sid  = -1;

int max_depth    = 1;
int max_regions  = 1;
int max_inits    = 1;

static void (*const gen_code_fp[OUTPUT_LANG_NULL])(void) = {
    gen_code_c,   // OUTPUT_LANG_C
    gen_code_py
};

static int (*const add_ext_fp[OUTPUT_LANG_NULL])(char *, int) = {
    append_ext_c,
    append_ext_py
};


static void
assign_event_ids()
{
    event_t *ev;

    /* all deferenced events */
    ev = event_link.head;
    while (ev) {
        if (ev == wildc_ev) {
            /* skip it */
        } else if (CHECK_FLAGS(ev, FLAG_BE_USED)) {
            ev->id = ++max_eid;
        }
        ev = ev->link;
    }

    /* all unreferenced events */
    ev = event_link.head;
    while (ev) {
        if (ev == wildc_ev) {
            /* skip it */
        } else if (ev->id == -1) {
            ev->id = ++max_eid;
        }
        ev = ev->link;
    }
}

static void
assign_state_ids()
{   
    state_t *st;

    st = state_link.head;
    /* set default start state */
    if (start_st == NULL) {
        start_st = st;
        while (start_st &&
               start_st->sub) {
            start_st = start_st->sub;
        }
    }

    /* leaf states */
    while (st) {
        if (!st->sub) {
            st->id = ++max_sid;
        }
        st = st->link;
    }
    max_leaf_sid = max_sid;

    /* super states */
    st = state_link.head;
    while (st) {
        if (st->id == -1 &&
            (!st->init ||
             !st->init->sibling)) {
            st->id = ++max_sid;
        }
        st = st->link;
    }
    max_super_sid = max_sid;

    /* zones */
    st = state_link.head;
    while (st) {
        if (st->id == -1) {
            st->id = ++max_sid;
        }
        st = st->link;
    }
}

static void
dump_events()
{
    event_t *ev = event_link.head;

    write2file("event list:\n");
    while (ev) {
        if (ev != wildc_ev) {
            write2file("%d:\t%s[%c%c]\n",
                       ev->id,
                       ev->name->txt,
                       CHECK_FLAGS(ev, FLAG_DEFINED) ? 'd' : ' ',
                       CHECK_FLAGS(ev, FLAG_BE_USED) ? 'u' : ' ');
        }
        ev = ev->link;
    }
    write2file("max: %d\n", max_eid);
    write2file("<end>\n");
}

static void
dump_one_state(state_t *st, int level)
{
    int i = level;
    state_t *s = st->sub;
    
    write2file("%d: ", st->id);
        
    while (i > 0) {
        write2file("\t");
        i --;
    }
    if (start_st == st) {
        write2file("*");
    }          
    write2file("%s[%c%c%c%c]\n",
               st->name->txt,
               CHECK_FLAGS(st, FLAG_DEFINED) ? 'd' : ' ',
               CHECK_FLAGS(st, FLAG_BE_USED) ? 'u' : ' ',
               (st->entry) ? 'e' : ' ',
               (st->exit)  ? 'x' : ' ');
        
    while (s) {
        assert(s->super == st);
        dump_one_state(s, level + 1);
        s = s->sibling;
    }
}

static void
dump_states()
{
    state_t *st = state_link.head;

    write2file("state list:\n");
    while (st) {
        if (st->super == NULL) {
            dump_one_state(st, 0);
        }
        st = st->link;
    }
    write2file("max_leaf: %d, max_super: %d, max: %d\n",
               max_leaf_sid, max_super_sid, max_sid);
    write2file("<end>\n");
}

static void
dump_evlist(plist_t *evl)
{
    int i;
    event_t *ev;

    write2file("(");
    i = 0;
    while (i < evl->count) {
        ev = evl->pa[i];
        if (i != 0) {
            write2file(", ");
        }
        if (ev == wildc_ev) {
            write2file("-:%s", ev->name->txt);
        } else {
            write2file("%d:%s", ev->id, ev->name->txt);
        }
        i ++;
    }
    write2file(")\n");
}

static void
dump_trans()
{   
    state_t *st;
    trans_t *tr;
    dest_t *dst;
        
    st = state_link.head;
    write2file("transition list:\n");
    while (st) {
        int lineno = 1;
        tr = st->trans;
        dst = st->init;
        if (tr || dst) {
            write2file("%d:%s:\t", st->id, st->name->txt);
        }   
        while (dst) {
            if (lineno != 1) {
                write2file("\t\t");
            }
            write2file(".\t[%d]->%d:%s\n", dst->id, dst->st->id, dst->st->name->txt);
            lineno ++;
            dst = dst->sibling;
        }   
        while (tr) {
            dst = tr->dst;
            if (lineno != 1) {
                write2file("\t\t");
            }       
            dump_evlist(&tr->evl);
            lineno ++;
            while (dst) {
                write2file("\t\t\t[%d]", dst->id);
                if (dst->st) {
                    if (dst->type == DST_TYPE_LFROM) {
                        write2file("|>");
                    } else if (dst->type == DST_TYPE_LTO) {
                        write2file(">|");
                    } else {
                        write2file("->");
                    }
                    write2file("%d:%s\n",
                                dst->st->id,
                                dst->st->name->txt);
                } else if (dst->type == DST_TYPE_INT) {
                    write2file("--\n");
                } else if (dst->type == DST_TYPE_DEFER) {
                    write2file("<<\n");
                } else {
                    assert(0);
                }
                dst = dst->sibling;
            }
            tr = tr->sibling;
        }
        st = st->link;
    }
    write2file("<end>\n");
}

static void
dump_all()
{
    dump_events();
    dump_states();
    dump_trans();
}

static void
compute_max_depth()
{   
    int i, depth;
    state_t *st;

    i = 0;
    while (i <= max_leaf_sid) {
        st = find_state_by_sid(i);
        depth = 0;
        do {
            depth ++;
            st = st->super;
        } while (st);
        if (max_depth < depth) {
            max_depth = depth;
        }
        i ++;
    }
}

static int
compute_st_regions(state_t *st)
{
    state_t *sub  = st->sub;
    dest_t  *init = st->init;
    int regions = 1;
    int local_inits = 1;

    while (sub) {
        if (sub->regions == 0) {
            sub->regions = compute_st_regions(sub);
        }
        if (regions < sub->regions) {
            regions = sub->regions;
        }
        sub = sub->sibling;
    }

    while(init &&
          init->sibling) {
        regions ++;
        local_inits ++;
        init = init->sibling;
    }
    if (max_inits < local_inits) {
        max_inits = local_inits;
    }

    return regions;
}

static void
compute_max_regions()
{
    int i;
    state_t *st;

    i = 0;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        if (st->regions == 0) {
            st->regions = compute_st_regions(st);
        }
        if (max_regions < st->regions) {
            max_regions = st->regions;
        }
        i ++;
    }
}


void
write2file(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(output_file, format, ap);
    va_end(ap);
}

int
init_output()
{
    int retval = 0;
    int len;
    const char *base;
    char *fname, *hname, *vname;
    char tab;
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
    len = strlen(base);
    assert(len > 0);
    fname = malloc(len + 16);
    if (!fname) {
        fprintf(stderr, "Failed to allocate memory of size %d.\n", len + 16);
        retval = -1;
        goto output_err;
    }
    if (g_opt_d != 0) {
        hname = malloc(len + 16);
        if (!fname) {
            fprintf(stderr, "Failed to allocate memory of size %d.\n", len + 16);
            retval = -1;
            goto output_err;
        }
    }
    if (g_opt_v != 0) {
        vname = malloc(len + 16);
        if (!fname) {
            fprintf(stderr, "Failed to allocate memory of size %d.\n", len + 16);
            retval = -1;
            goto output_err;
        }
    }

    strcpy(fname, base);
    if (tab) {
        strcat(fname, ".tab");
        len += 4;
    }

    /* append file extension */
    len = add_ext_fp[g_opt_l](fname, len);

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
        hname[len - 1] = 'h';

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
        vname[len - 2] = 0;
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
        if (ofile) fclose(ofile);
        if (hfile) fclose(hfile);
        if (vfile) fclose(vfile);
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

void
gen_output()
{
    assign_event_ids();
    assign_state_ids();

    if (output_v_fptr != NULL) {
        output_file = output_v_fptr;
        dump_all();
    }

    compute_max_depth();
    compute_max_regions();

    assert(gen_code_fp[g_opt_l]);
    gen_code_fp[g_opt_l]();
}
