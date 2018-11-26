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

#include "defines.h"
#include "gen.h"

#define WRITE_LINENO(TEXT) \
do {\
    if (g_opt_n) {\
        write2file("#line %d \"%s\"\n", (TEXT)->line, input_filename);\
    }\
} while (0);

#define WRITE_PREFIX(PREFIX) \
do {\
    if (PREFIX) {\
        write2file("%s", PREFIX);\
    }\
} while (0);

#define WRITE_HSM_T(APPENDIX) \
do {\
    if (api_prefix) {\
        write2file("%s%s%s", api_prefix, "hsm_t", APPENDIX);\
    } else {\
        write2file("%s%s", "hsm_t", APPENDIX);\
    }\
} while (0);

#define WRITE_HEADER_DEF(APPENDIX) \
do {\
    if (macro_prefix) {\
        write2file("__%s%s%s", macro_prefix, "HSM_AUTO_H__", APPENDIX);\
    } else {\
        write2file("__%s%s", "HSM_AUTO_H__", APPENDIX);\
    }\
} while (0);

extern const int  prod_ver[3];
extern const char *prod_name;

extern const char *input_filename;
extern event_t *wildc_ev;
extern state_t *start_st;
extern link_t dest_link;
extern text_t     *prolog_code;
extern text_t     *epilog_code;
extern text_t     *start_code;
extern const char *api_prefix;
extern const char *macro_prefix;
extern int   deferral_is_used;

extern int manual_eid;
extern int manual_sid;

extern int   g_opt_n;
extern int   g_opt_S;

extern FILE *output_file_ptr;
extern FILE *output_h_fptr;
extern FILE *output_file;

extern int max_eid;
extern int max_sid;
extern int max_leaf_sid;
extern int max_super_sid;

extern int max_depth;
extern int max_regions;
extern int max_inits;

static int max_zone_depth = 1;


static void
print_warn_def(const char *p)
{
#if 0
    write2file("#ifdef %s\n", p);
    write2file("  Error: %s is reserved for internal use, please undefine it.\n", p);
    write2file("#endif\n");
#endif
}

static void
print_h_header()
{
    char *p;
    event_t *ev;
    state_t *st;
    int i;

    write2file("#ifndef ");
    WRITE_HEADER_DEF("\n");
    write2file("#define ");
    WRITE_HEADER_DEF("\n\n");

    if (manual_eid == 0) {
        write2file("/* auto generated event IDs */\n");
        i = 0;
        while (i <= max_eid) {
            ev = find_event_by_eid(i);
            if (ev != wildc_ev) {
                write2file("#define %s\t%d\n", ev->name->txt, i);
            }
            i ++;
        }
        write2file("\n");
    }

    if (manual_sid == 0) {
        write2file("/* auto generated state IDs */\n");
        i = 0;
        while (i <= max_sid) {
            st = find_state_by_sid(i);
            write2file("#define %s\t%d\n", st->name->txt, i);
            i ++;
        }
        write2file("\n");
    }

    write2file("#define ");
    WRITE_PREFIX(macro_prefix);
    write2file("%s\t%d\n\n", "NUM_OF_EVENTS", max_eid + 1);

    write2file("#define ");
    WRITE_PREFIX(macro_prefix);
    write2file("%s\t%d\n\n", "NUM_OF_STATES", max_sid + 1);

    write2file("#define ");
    WRITE_PREFIX(macro_prefix);
    write2file("%s\t%d\n\n", "MAX_REGIONS", max_regions);

    write2file("#define ");
    write2file("%s\t%d\n\n", "HHTRUE", 1);

    write2file("#define ");
    write2file("%s\t%d\n\n", "HHFALSE", 0);

    p = "typedef struct ";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_s {\n";
    write2file(p);

    p = "  unsigned int (*hh_event_func)(void *, void **);\n"
        "  void         (*hh_deferral_func)(void *, unsigned int, void *, int);\n"
        "  void         (*hh_lock_func)(void *);\n"
        "  void         (*hh_unlock_func)(void *);\n"
        "  void          *hh_private_data;\n"
        "  int            hh_state_id[%d];\n"
        "  int            hh_running;\n"
        "} ";
    write2file(p, max_regions, max_regions);

    WRITE_HSM_T(";\n\n");

    p = "int ";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_init(";
    write2file(p);

    WRITE_HSM_T(" *,\n");
    p = "             unsigned int (*)(void *, void **),\n"
        "             void         (*)(void *, unsigned int, void *, int),\n"
        "             void         (*)(void *),\n"
        "             void         (*)(void *),\n"
        "             void          *);\n\n";
    write2file(p);
 
    p = "int ";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_run(";
    write2file(p);

    WRITE_HSM_T(" *);\n");

    p = "int ";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_run_to(";
    write2file(p);

    WRITE_HSM_T(" *,");
    p = " const int *,"
        " unsigned int);\n\n";
    write2file(p);

    p = "int ";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_get_state(";
    write2file(p);

    WRITE_HSM_T(" *);\n");

    p = "unsigned int ";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_get_all_states(";
    write2file(p);

    WRITE_HSM_T(" *,");
    p = " int *,"
        " unsigned int);\n\n";
    write2file(p);

    if (g_opt_S) {
        p = "const char *";
        write2file(p);
        WRITE_PREFIX(api_prefix);
        p = "hsm_event_str(unsigned int);\n";
        write2file(p);

        p = "const char *";
        write2file(p);
        WRITE_PREFIX(api_prefix);
        p = "hsm_state_str(int);\n\n";
        write2file(p);
    }

    write2file("#endif\n");
}

static void
print_c_header()
{
    const char *p;
    int i;
    event_t *ev;
    state_t *st;

    p = "\n"
        "#ifdef __GNUC__\n"
        "#define HHUSE(A) (void)(A)\n"
        "#define HHUSELAB __attribute__ ((unused))\n";
    write2file(p);

    p = "#else\n"
        "#define HHUSE(A)\n"
        "#define HHUSELAB /* label may not be used */\n"
        "#endif\n\n";
    write2file(p);

    p = "HHACCEPT";
    print_warn_def(p);
    write2file("#define %s\t%s\n\n", p, "goto hhacceptlab");

    p = "HHABORT";
    print_warn_def(p);
    write2file("#define %s\t\t%s\n\n", p, "goto hhabortlab");

    p = "HHPDATA";
    print_warn_def(p);
    write2file("#define %s\t\t%s\n\n", p, "hhinst->hh_private_data");

    p = "HHEVTARG";
    print_warn_def(p);
    write2file("#define %s\t%s\n\n", p, "hhevp");

    p = "HHEVENT";
    print_warn_def(p);
    write2file("#define %s\t\t", p);
    write2file("((hheid <= %d) ? hheventmap[hheid] : hheid)\n\n", max_eid);

    p = "HHSSTATE";
    print_warn_def(p);
    write2file("#define %s\t%s\n\n", p, "((hhsrcst >= 0) ? hhstatemap[hhsrcst] : -1)");

    p = "HHESTATE";
    print_warn_def(p);
    write2file("#define %s\t%s\n\n", p, "((hheffst >= 0) ? hhstatemap[hheffst] : -1)");

    p = "HHDSTATE";
    print_warn_def(p);
    write2file("#define %s\t%s\n\n", p, "((hhdstst >= 0) ? hhstatemap[hhdstst] : -1)");

    p = "HHCSTATE";
    print_warn_def(p);
    write2file("#define %s\t%s\n\n", p, "((hhcurst >= 0) ? hhstatemap[hhcurst] : -1)");

    p = "HHRINDEX";
    print_warn_def(p);
    write2file("#define %s\t%s\n\n", p, "(const int)hhregthis");

    p = "HH_PROCESS_INVALID_EVENT";
    write2file("#ifndef %s\n", p);
    write2file("#define %s\n", p);
    write2file("#endif\n\n");

    p = "HH_PROCESS_IGNORED_EVENT";
    write2file("#ifndef %s\n", p);
    write2file("#define %s\n", p);
    write2file("#endif\n\n");

    p = "HH_TRACE_START";
    write2file("#ifndef %s\n", p);
    write2file("#define %s\n", p);
    write2file("#endif\n\n");

    p = "HH_TRACE_EXIT";
    write2file("#ifndef %s\n", p);
    write2file("#define %s\n", p);
    write2file("#endif\n\n");

    p = "HH_TRACE_ACTION";
    write2file("#ifndef %s\n", p);
    write2file("#define %s\n", p);
    write2file("#endif\n\n");

    p = "HH_TRACE_ENTRY";
    write2file("#ifndef %s\n", p);
    write2file("#define %s\n", p);
    write2file("#endif\n\n");

    p = "HH_TRACE_INIT";
    write2file("#ifndef %s\n", p);
    write2file("#define %s\n", p);
    write2file("#endif\n\n");

    p = "HH_TRACE_TRANS";
    write2file("#ifndef %s\n", p);
    write2file("#define %s(X)\n", p);
    write2file("#endif\n\n");

    p = "HH_REGION_GONE";
    write2file("#ifndef %s\n", p);
    write2file("#define %s(I)\n", p);
    write2file("#endif\n\n");
 
    write2file("static const unsigned int hheventmap[] = {");
    i = 0;
    while (i <= max_eid) {
        ev = find_event_by_eid(i);
        if (ev != wildc_ev) {
            if (i > 0) {
                write2file(",");
            }
            write2file("\n  %s", ev->name->txt);
        }
        i ++;
    }
    write2file("\n};\n\n");

    if (g_opt_S) {
        write2file("static const char *const hheventstr[] = {");
        i = 0;
        while (i <= max_eid) {
            ev = find_event_by_eid(i);
            if (ev != wildc_ev) {
                if (i > 0) {
                    write2file(",");
                }
                write2file("\n  \"%s\"", ev->name->txt);
            }
            i ++;
        }
        write2file("\n};\n\n");
    }

    write2file("static const int hhstatemap[] = {");
    i = 0;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        if (i > 0) {
            write2file(",");
        }
        write2file("\n  %s", st->name->txt);
        i ++;
    }
    write2file("\n};\n\n");

    if (g_opt_S) {
        write2file("static const char *const hhstatestr[] = {");
        i = 0;
        while (i <= max_sid) {
            st = find_state_by_sid(i);
            if (i > 0) {
                write2file(",");
            }
            write2file("\n  \"%s\"", st->name->txt);
            i ++;
        }
        write2file("\n};\n\n");
    }
}

static void
print_prod_info()
{
    write2file("/* Generated code, don't edit. */\n");
    write2file("/* %s */\n", prod_name);
    write2file("/* Version %d.%d.%d */\n\n", prod_ver[0], prod_ver[1], prod_ver[2]);
}

static void
print_prolog()
{
    if (prolog_code) {
        WRITE_LINENO(prolog_code);
        write2file("%s\n\n", prolog_code->txt);
    }
}

static void
print_epilog()
{
    if (epilog_code) {
        WRITE_LINENO(epilog_code);
        write2file("%s\n", epilog_code->txt);
    }
}

static void
print_super_states()
{
    int i, sid;
    state_t *st;

    write2file("static const int hhsuper[] = {");
    i = 0;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        assert(st);
        sid = (st->super) ? st->super->id : -1;
        if (i != 0) {
            write2file(",");
        }
        if ((i % 8) == 0) {
            write2file("\n  %d", sid);
        } else {
            write2file("\t%d", sid);
        }
        i ++;
    }
    write2file("\n};\n\n");
}

static void
print_sub_states()
{
    int i, j, sid;
    state_t *st;

    write2file("static const int hhsub[] = {");
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
            write2file("\n  %d", sid);
        } else {
            write2file("\t%d", sid);
        }
        i ++;
        j ++;
    }
    write2file("\n};\n\n");
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
            write2file("\n  ");
        } else {
            write2file("\t");
        }
    }
    write2file("%d", st->id);
    return ++n;
}

static void
print_entry_path()
{
    int i;
    state_t *st;

    write2file("static const int hhentrypath[][%d] = {", max_depth);
    i = 0;
    while (i <= max_leaf_sid) {
        st = find_state_by_sid(i);
        if (i != 0) {
            write2file(",");
        }
        write2file("\n{ ");
        print_parent_sid(st);
        write2file(" }");
        i ++;
    }
    write2file("\n};\n\n");
}

static void
print_trans_path()
{
    int i, j, k, l, m, n;
    state_t *st;
    trans_t *tr, *sibling;
    plist_t *evl;
    event_t *ev;
    dest_t  *dst;
    state_t *cross;

    int has_wildc, print_num;
    int comma1 = 0, comma2 = 0;

    int  trans_table_sz = 0;
    int *trans_table    = NULL;
    int  trans_flags_sz = 0;
    int *trans_flags    = NULL;
    int  trans_depth_sz = 0;
    int *trans_depth    = NULL;

    int max_trans_id = 0;
    int max_trans_depth = 0;

    trans_table_sz =  (max_sid + 1) *
                      (max_eid + 1) *
                       sizeof(trans_table[0]);
    trans_flags_sz =  (max_sid + 1) *
                     ((max_eid + 1 + sizeof(trans_flags[0]) * 8 - 1) /
                                    (sizeof(trans_flags[0]) * 8)) *
                       sizeof(trans_flags[0]);
    trans_depth_sz = (max_eid + 1) * sizeof(trans_depth[0]);

    trans_table = malloc(trans_table_sz);
    trans_flags = malloc(trans_flags_sz);
    trans_depth = malloc(trans_depth_sz);
    if (!trans_table || !trans_flags || !trans_depth) {
        fprintf(stderr, "Error: failed to allocate memory of size %d.\n",
                trans_table_sz + trans_flags_sz + trans_depth_sz);
        exit(-1);
    }
    memset(trans_table, 0xff, trans_table_sz);
    memset(trans_flags, 0x00, trans_flags_sz);

    /* build the transition index table */
    i = 0;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        assert(st);
        tr = st->trans;
        while (tr) {
            evl = &tr->evl;
            j = 0;
            while (j < evl->count) {
                ev = evl->pa[j];
                if (ev == wildc_ev) {
                    has_wildc = 0;
                    k = 0;
                    while (k <= max_eid) {
                        l = i * (max_eid + 1) + k;
                        if (trans_table[l] == 0xffffffff) {
                            trans_table[l] = max_trans_id;
                            has_wildc = 1;
                        }
                        k ++;
                    }
                    if (has_wildc != 0) {
                        max_trans_id ++;
                    }
                } else {
                    l = i * (max_eid + 1) + ev->id;
                    m = i * ((max_eid + 1 + sizeof(trans_flags[0]) * 8 - 1) /
                                           (sizeof(trans_flags[0]) * 8)) +
                        ev->id / (sizeof(trans_flags[0]) * 8);
                    n = 1 << (ev->id % (sizeof(trans_flags[0]) * 8));
                    if (trans_table[l] == 0xffffffff ||
                      !(trans_flags[m] & n)) {
                        trans_table[l] = max_trans_id ++;
                        trans_flags[m] |= n;
                    }
                }
                j ++;
            }
            tr = tr->sibling;
        }
        i ++;
    }

    /* print transition table */
    write2file("static const int hhtrindex[] = {");

    i = 0;
    while (i < trans_table_sz / sizeof(trans_table[0])) {
        if (i != 0) {
            write2file(",");
        }
        if ((i % 8) == 0) {
        //if ((i % (max_eid + 1)) == 0) {
            write2file("\n  %d", trans_table[i]);
        } else {
            write2file("\t%d", trans_table[i]);
        }
        i ++;
    }

    write2file("\n};\n\n");

    /* find max depth */
    i = 0;
    while (i <= max_sid) {
        memset(trans_depth, 0x00, trans_depth_sz);
        st = find_state_by_sid(i);
        tr = st->trans;
        while (tr) {
            dst = tr->dst;
            l = 0;
            do {
                l ++;
                dst = dst->sibling;
            } while (dst);
            evl = &tr->evl;
            j = 0;
            while (j < evl->count) {
                ev = evl->pa[j];
                dst = tr->dst;
                if (ev == wildc_ev) {
                    k = 0;
                    while (k <= max_eid) {
                        trans_depth[k] += l;
                        if (max_trans_depth < trans_depth[k]) {
                            max_trans_depth = trans_depth[k];
                        }
                        k ++;
                    }
                } else {
                    trans_depth[ev->id] += l;
                    if (max_trans_depth < trans_depth[ev->id]) {
                        max_trans_depth = trans_depth[ev->id];
                    }
                }
                j ++;
            }
            tr = tr->sibling;
        }
        i ++;
    }

    /* print transition path */
    memset(trans_flags, 0x00, trans_flags_sz);
    write2file("static const int hhtrselect[][%d] = {", max_trans_depth * 3 + 1);
    i = 0;
    while (i <= max_sid) {
        has_wildc = 0;
        st = find_state_by_sid(i);
        tr = st->trans;
        while (tr) {
            evl = &tr->evl;
            j = 0;
            while (j < evl->count) {
                print_num = 0;
                ev = evl->pa[j ++];
                if (ev == wildc_ev) {
                    if (has_wildc == 0) {
                        print_num = 1;
                    }
                } else {
                    m = i * ((max_eid + 1 + sizeof(trans_flags[0]) * 8 - 1) /
                                           (sizeof(trans_flags[0]) * 8)) +
                        ev->id / (sizeof(trans_flags[0]) * 8);
                    n = 1 << (ev->id % (sizeof(trans_flags[0]) * 8));
                    if (!(trans_flags[m] & n)) {
                        trans_flags[m] |= n;
                        print_num = 1;
                    }
                }
                if (print_num != 0) {
                    if (comma1) {
                        write2file(",");
                    } else {
                        comma1 = 1;
                    }
                    if (has_wildc != 0) {
                        sibling = st->trans;
                    } else {
                        sibling = tr;
                    }
                    comma2 = 0;
                    write2file("\n{ ");
                    do {
                        if (test_ev_in_list(ev, &sibling->evl)) {
                            dst = sibling->dst;
                            assert(dst);
                            do {
                                if (comma2) {
                                    if ((comma2 % 3) == 0) {
                                        write2file("\n  ");
                                    } else {
                                        write2file("\t");
                                    }
                                }
                                comma2 ++;
                                if (dst->st) {
                                    if (dst->type == DST_TYPE_LFROM) {
                                        cross = st;
                                    } else if (dst->type == DST_TYPE_LTO) {
                                        cross = dst->st;
                                    } else {
                                        cross = search_cross_state(st, dst->st);
                                    }
                                    write2file("%d,\t%d,\t%d,",
                                               dst->id,
                                               dst->st->id,
                                               cross ? cross->id : -1);
                                } else {
                                    write2file("%d,\t%d,\t%d,",
                                               dst->id,
                                               (dst->type == DST_TYPE_INT) ? -1 : -2,
                                               -1);
                                }
                                dst = dst->sibling;
                            } while (dst);
                        }
                        sibling = sibling->sibling;
                    } while (sibling);
                    write2file("\t-1 }");
                    if (ev == wildc_ev) {
                        has_wildc = 1;
                    }
                }
            }
            tr = tr->sibling;
        }
        i ++;
    }
    write2file("\n};\n\n");

    free(trans_table);
    free(trans_flags);
}

static void
print_init_trans()
{
    int i, j = 0;
    state_t *st;
    dest_t  *init;
    int tid, sid;

    if (max_regions > 1) {
        write2file("static const int hhinitpath[][%d] = {",
                   max_inits * 2 + 1);
    } else {
        write2file("static const int hhinitpath[] = {");
    }

    i = max_leaf_sid + 1;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        init = st->init;
        if (max_regions > 1) {
            if (i == max_leaf_sid + 1) {
                write2file("\n{ ");
            } else {
                write2file(",\n{ ");
            }
            j = 0;
        } else {
            if (i == max_leaf_sid + 1) {
                write2file("\n  ");
            }
        }
        do {
            if (init) {
                tid = init->id;
                sid = init->st->id;
            } else {
                tid = -2;
                sid = st->sub->id;
            }
            if (j != 0) {
                if ((j % 4) == 0) {
                    write2file(",\n  ");
                } else {
                    write2file(",\t");
                }
            }
            write2file("%d,\t%d", tid, sid);
            j ++;
        } while (init &&
                (init = init->sibling));
        if (max_regions > 1) {
            write2file(",\t-1 }");
        }
        i ++;
    }

    write2file("\n};\n\n");
}

static void
print_zone_map()
{
    int i;
    state_t *st, *super;
    int zid;

    if (max_regions == 1) {
        return;
    }

    write2file("static const int hhzoneedge[] = {");

    i = 0;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        super = st->super;
        while (super &&
              (!super->init ||
               !super->init->sibling)) {
            super = super->super;
        }

        zid = super ? super->id : -1;

        if (i != 0) {
            write2file(",");
        }
        if ((i % 8) == 0) {
            write2file("\n  %d", zid - max_super_sid - 1);
        } else {
            write2file("\t%d", zid - max_super_sid - 1);
        }
        i ++;
    }

    write2file("\n};\n\n");
}

static void
print_zone_depth()
{
    int i, j;
    state_t *st;
    int zdep;

    if (max_regions == 1) {
        return;
    }

    write2file("static const int hhzonedepth[] = {");

    j = 0;
    i = max_super_sid + 1;
    while (i <= max_sid) {
        st = find_state_by_sid(i);
        st = st->super;
        zdep = 0;
        while (st) {
            if (st->init &&
                st->init->sibling) {
                zdep ++;
            }
            st = st->super;
        }
        if (max_zone_depth < zdep + 1) {
            max_zone_depth = zdep + 1;
        }
        if (j != 0) {
            write2file(",");
        }
        if ((j % 8) == 0) {
            write2file("\n  %d", zdep);
        } else {
            write2file("\t%d", zdep);
        }
        i ++;
        j ++;
    }

    write2file("\n};\n\n");
}

static void
print_run_i()
{
    dest_t  *dt;
    state_t *st;
    int      i;
    char    *p;

    p = "static int\n";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_run_i(";
    write2file(p);

    WRITE_HSM_T(" *hhinst)\n{\n");

    p = "  unsigned int  hheid;\n"
        "  void         *hhevp;\n"
        "  int           hhsrcst;\n"
        "  int           hhcurst;\n"
        "  int           hheffst;\n"
        "  int           hhdstst;\n"
        "  int           hhselect;\n"
        "  int           hhcross;\n"
        "  int           hhleafst;\n"
        "  int           hhinitst;\n"
        "  int           hhtridx;\n"
        "  const int    *hhselectset;\n"
        "  int           hhregthis;\n";
    write2file(p);

    if (max_regions > 1) {
        p = "  int           hhpassn;\n"
            "  int           hhzonestack[%d];\n"
            "  int           hhzonereg[%d];\n"
            "  int           hhzonedep;\n"
            "  int           hhregn;\n"
            "  int           hhregst;\n"
            "  const int    *hhpathstack[%d];\n"
            "  int           hhinitstack[%d];\n"
            "  int           hhinitdep;\n";
        write2file(p, max_zone_depth, max_zone_depth,  max_depth, max_depth);
    }

    p = "  int           hhgval = HHFALSE;\n"
        "  int           hhret = 0;\n\n";
    write2file(p);

    if (manual_eid != 0) {
        p = "  unsigned int  hhevt;\n\n";
        write2file(p);
    }

    p = "  hhinst->hh_running = 1;\n\n"
        "  hhregthis = 0;\n"
        "  hhsrcst = hhinst->hh_state_id[hhregthis];\n\n";
    write2file(p);

    if (max_regions > 1) {
        p = "  hhinitdep = 0;\n"
            "  hhregn = 0;\n"
            "  while (hhsrcst == -1 && hhregn < %d) {\n"
            "    hhsrcst = hhinst->hh_state_id[++hhregn];\n"
            "  }\n";
        write2file(p, max_regions - 1);
    }

    p = "  hhcurst = hhsrcst;\n"
        "  if (hhcurst != -1) {\n"
        "    goto hhtoplab;\n"
        "  }\n\n";
    write2file(p);

    p = "  hheid = 0;\n"
        "  hhevp = NULL;\n\n"
        "  hhdstst = %d;\n"
        "  hheffst = hhcross = -1;\n\n";
    write2file(p, start_st->id);

    p = "  do { HH_TRACE_START; } while (0);\n";
    write2file(p);
    if (start_code) {
        WRITE_LINENO(start_code);
        p = "  %s\n";
        write2file(p, start_code->txt);
    }

    p = "  goto hhrunentrylab;\n\n"
        "hhtoplab:\n";
    write2file(p);

    p = "  if (hhinst->hh_lock_func) {\n"
        "    hhinst->hh_unlock_func(hhinst->hh_private_data);\n"
        "  }\n";
    write2file(p);

    if (deferral_is_used) {
        p = "hhtopunlockedlab:\n";
        write2file(p);
    }
    p = "  hheid = hhinst->hh_event_func(hhinst->hh_private_data, &hhevp);\n";
    write2file(p);

    p = "  if (hhinst->hh_lock_func) {\n"
        "    hhinst->hh_lock_func(hhinst->hh_private_data);\n"
        "  }\n";
    write2file(p);

    if (manual_eid != 0) {
        p = "  hhevt = 0;\n"
            "  while (hhevt <= %d &&\n"
            "         hheventmap[hhevt] != hheid) {\n"
            "    hhevt ++;\n"
            "  }\n";
        write2file(p, max_eid);

        p = "  if (hhevt > %d) {\n"
            "    do { HH_PROCESS_INVALID_EVENT; } while (0);\n"
            "    goto hhtoplab;\n"
            "  }\n"
            "  hheid = hhevt;\n";
        write2file(p, max_eid);
    } else {
        p = "  if (hheid > %d) {\n"
            "    do { HH_PROCESS_INVALID_EVENT; } while (0);\n"
            "    goto hhtoplab;\n"
            "  }\n";
        write2file(p, max_eid);
    }

    p = "  hheffst = hhdstst = -1;\n";
    write2file(p);

    if (max_regions > 1) {
        p = "  hhzonestack[%d] = -1;\n";
        for (i = 0; i < max_zone_depth; i++) {
            write2file(p, i);
        }

        p = "  hhzonedep = %d;\n"
            "  hhpassn = 1;\n"
            "  hhregthis = -1;\n"
            "hhregionlab:\n"
            "  if (hhpassn < 3) {\n"
            "    if (++hhregthis >= %d) {\n"
            "      if (hhpassn == 1) {\n"
            "        hhregthis = -1;\n"
            "      }\n"
            "      hhpassn ++;\n"
            "      goto hhregionlab;\n"
            "    }\n"
            "    hhcurst = hhsrcst = hhinst->hh_state_id[hhregthis];\n"
            "    if (hhpassn == 2 && hhcurst != -1) {\n"
            "      hhcurst = hhsuper[hhcurst];\n"
            "    }\n"
            "  } else if (hhzonedep > 0) {\n"
            "    hhcurst = hhzonestack[--hhzonedep];\n"
            "    if (hhcurst != -1) {\n"
            "      hhregthis = hhzonereg[hhzonedep];\n"
            "      hhsrcst = hhinst->hh_state_id[hhregthis];\n"
            "    }\n"
            "  } else {\n"
            "    do { HH_PROCESS_IGNORED_EVENT; } while (0);\n"
            "    goto hhtoplab;\n"
            "  }\n";
        write2file(p, max_zone_depth, max_regions);

        p = "  if (hhcurst == -1) {\n"
            "    goto hhregionlab;\n"
            "  }\n";
        write2file(p);
    } else {
        p = "  hhcurst = hhsrcst = hhinst->hh_state_id[hhregthis];\n";
        write2file(p);
    }

    p = "hhtridxlab:\n"
        "  hhtridx = hhtrindex[hhcurst * %d + hheid];\n"
        "  if (hhtridx == -1) {\n"
        "hhsuperlab:\n";
    write2file(p, max_eid + 1);

    if (max_regions > 1) {
        p = "    if (hhpassn == 1) {\n"
            "      goto hhregionlab;\n"
            "    } else if (hhsuper[hhcurst] == hhzoneedge[hhcurst] + %d) {\n"
            "      if (hhsuper[hhcurst] != -1) {\n"
            "        hhzonestack[hhzonedepth[hhzoneedge[hhcurst]]] = hhsuper[hhcurst];\n"
            "        hhzonereg[hhzonedepth[hhzoneedge[hhcurst]]] = hhregthis;\n"
            "      }\n"
            "      goto hhregionlab;\n";
        write2file(p, max_super_sid + 1);
    } else {
        p = "    if (hhsuper[hhcurst] == -1) {\n"
            "      /* event not being handled on state */\n"
            "      do { HH_PROCESS_IGNORED_EVENT; } while (0);\n"
            "      goto hhtoplab;\n";
        write2file(p);
    }

    p = "    }\n"
        "    hhcurst = hhsuper[hhcurst];\n"
        "    goto hhtridxlab;\n"
        "  }\n\n";
    write2file(p);

    p = "  hhselectset = hhtrselect[hhtridx];\n"
        "hhselectlab:\n";
    write2file(p);

    p = "  hhselect = *hhselectset++;\n"
        "  if (hhselect == -1) {\n"
        "    /* none of guards is satisified for event on state */\n"
        "    goto hhsuperlab;\n"
        "  }\n";
    write2file(p);

    p = "  hhdstst = *hhselectset++;\n"
        "  hhcross = *hhselectset++;\n";
    write2file(p);

    p = "  hhgval = HHTRUE;\n"
        "  switch (hhselect) {\n";
    write2file(p);

    dt = dest_link.head;
    while (dt) {
        if (dt->guard) {
            p = "    case %d:\n";
            write2file(p, dt->id);

            WRITE_LINENO(dt->guard);

            p = "      hhgval =%s;\n"
                "      break;\n";
            write2file(p, dt->guard->txt);
        }
        dt = dt->link;
    }
    p = "    default: break;\n"
        "  }\n";
    write2file(p);

    p = "  if (hhgval == HHFALSE) {\n"
        "    goto hhselectlab;\n"
        "  }\n"
        "  hheffst = hhcurst;\n";
    write2file(p);

    if (deferral_is_used) {
        p = "  if (hhdstst == -2) {\n"
            "    if (hhinst->hh_lock_func) {\n"
            "      hhinst->hh_unlock_func(hhinst->hh_private_data);\n"
            "    }\n"
            "    hhinst->hh_deferral_func(hhinst->hh_private_data,\n"
            "                             hheventmap[hheid], hhevp, hhstatemap[hheffst]);\n"
            "    goto hhtopunlockedlab;\n"
            "  }\n";
        write2file(p);
    }

    p = "  if (hhdstst == -1) {\n"
        "    goto hhrunactionlab;\n"
        "  }\n"
        "  hhcurst = hhsrcst;\n";
    write2file(p);

    if (max_regions > 1) {
        p = "  hhzonestack[%d] = -1;\n";
        for (i = 0; i < max_zone_depth; i++) {
            write2file(p, i);
        }

        p = "  hhzonedep = %d;\n"
            "  hhpassn = 1;\n";
        write2file(p, max_zone_depth);
    }

    p = "hhrunexitlab:\n";
    write2file(p);
    p = "  do { HH_TRACE_EXIT; } while (0);\n";
    write2file(p);
    p = "  switch (hhcurst) {\n";
    write2file(p);

    for (i = 0; i <= max_sid; i++) {
        st = find_state_by_sid(i);
        assert(st);
        if (st->exit) {
            p = "    case %d:\n";
            write2file(p, i);

            WRITE_LINENO(st->exit);

            p = "      %s\n"
                "      break;\n";
            write2file(p, st->exit->txt);
        }
    }
    p = "    default: break;\n"
        "  }\n";
    write2file(p);

    if (max_regions > 1) {
        p = "  if (hhsuper[hhcurst] == hhcross) {\n"
            "  } else if (hhsuper[hhcurst] == hhzoneedge[hhcurst] + %d) {\n"
            "    hhzonestack[hhzonedepth[hhzoneedge[hhcurst]]] = hhsuper[hhcurst];\n"
            "    hhzonereg[hhzonedepth[hhzoneedge[hhcurst]]] = hhregthis;\n"
            "  } else {\n"
            "    hhcurst = hhsuper[hhcurst];\n"
            "    goto hhrunexitlab;\n"
            "  }\n";
        write2file(p, max_super_sid + 1);

        p = "  if (hhpassn == 1) {\n"
            "    do { HH_REGION_GONE((const int)hhregthis); } while (0);\n"
            "    hhsrcst = hhinst->hh_state_id[hhregthis];\n"
            "    hhinst->hh_state_id[hhregthis] = -1;\n"
            "    hhregn = -1;\n"
            "    while (++hhregn < %d) {\n";
        write2file(p, max_regions);

        p = "      hhregst = hhinst->hh_state_id[hhregn];\n"
            "      while (hhregst != -1 &&\n"
            "             hhsuper[hhregst] != hheffst) {\n"
            "        hhregst = hhsuper[hhregst];\n"
            "      }\n";
        write2file(p);

        p = "      if (hhregst != -1) {\n"
            "        hhregthis = hhregn;\n"
            "        hhcurst = hhsrcst = hhinst->hh_state_id[hhregthis];\n"
            "        goto hhrunexitlab;\n"
            "      }\n"
            "    }\n"
            "    hhpassn ++;\n"
            "  }\n";
        write2file(p);

        p = "  while (hhzonedep > 0) {\n"
            "    hhcurst = hhzonestack[--hhzonedep];\n"
            "    if (hhcurst != -1) {\n"
            "      hhregthis = hhzonereg[hhzonedep];\n"
            "      goto hhrunexitlab;\n"
            "    }\n"
            "  }\n";
        write2file(p);
    } else {
        p = "  if (hhsuper[hhcurst] != hhcross) {\n"
            "    hhcurst = hhsuper[hhcurst];\n"
            "    goto hhrunexitlab;\n"
            "  }\n";
        write2file(p);
    }

    p = "\nhhrunactionlab:\n";
    write2file(p);
    p = "  do { HH_TRACE_ACTION; } while (0);\n";
    write2file(p);
    p = "  switch (hhselect) {\n";
    write2file(p);

    dt = dest_link.head;
    while (dt) {
        if (dt->action) {
            p = "    case %d:\n";
            write2file(p, dt->id);

            WRITE_LINENO(dt->action);

            p = "      %s\n"
                "      break;\n";
            write2file(p, dt->action->txt);
        }
        dt = dt->link;
    }
    p = "    default: break;\n"
        "  }\n";
    write2file(p);

    p = "  if (hhdstst == -1) {\n"
        "    hhcurst = hhsrcst;\n"
        "    goto hhdonelab;\n"
        "  }\n\n";
    write2file(p);

    p = "hhrunentrylab:\n"
        "  hhleafst = hhinitst = hhdstst;\n"
        "  while (hhleafst > %d) {\n"
        "    hhleafst = hhsub[hhleafst - %d];\n"
        "  }\n";
    write2file(p, max_leaf_sid, max_leaf_sid + 1);

    p = "hhinitentrylab:\n"
        "  hhselectset = hhentrypath[hhleafst];\n"
        "hhnextentrylab:\n"
        "  hhcurst = *hhselectset++;\n"
        "  if (hhcross == -1) {\n";
    write2file(p);
    p = "    do { HH_TRACE_ENTRY; } while (0);\n";
    write2file(p);
    p = "    switch (hhcurst) {\n";
    write2file(p);

    for (i = 0; i <= max_sid; i++) {
        st = find_state_by_sid(i);
        assert(st);
        if (st->entry) {
            write2file("      case %d:\n", i);
            WRITE_LINENO(st->entry);
            write2file("        %s\n", st->entry->txt);
            write2file("        break;\n");
        }
    }
    p = "      default: break;\n"
        "    }\n";
    write2file(p);

    p = "  } else if (hhcurst == hhcross) {\n"
        "    hhcross = -1;\n"
        "  }\n";
    write2file(p);

    p = "  if (hhcurst != hhinitst) {\n"
        "    goto hhnextentrylab;\n"
        "  }\n";
    write2file(p);

    p = "  if (hhcurst <= %d) {\n"
        "    goto hhentrydonelab;\n"
        "  }\n"
        "  hhcross = hhcurst;\n";
    write2file(p, max_leaf_sid);

    if (max_regions > 1) {
        p = "  hhselectset = hhinitpath[hhcurst - %d];\n";
    } else {
        p = "  hhselectset = &hhinitpath[(hhcurst - %d) * 2];\n";
    }
    write2file(p, max_leaf_sid + 1);

    p = "  hhselect = *hhselectset++;\n"
        "  hhinitst = *hhselectset;\n";
    write2file(p);

    if (max_regions > 1) {
        p = "  hhinitstack[hhinitdep] = hhcurst;\n"
            "  hhpathstack[hhinitdep++] = ++hhselectset;\n";
        write2file(p);
    }

    p = "  if (hhinitst <= %d) {\n"
        "    hhleafst = hhinitst;\n"
        "  }\n";
    write2file(p, max_leaf_sid);

    if (max_regions > 1) {
        p = "hhregioninitlab:\n";
        write2file(p);
    }

    p = "  do { HH_TRACE_INIT; } while (0);\n";
    write2file(p);
    p = "  switch (hhselect) {\n";
    write2file(p);
    for (i = max_leaf_sid + 1; i <= max_sid; i++) {
        st = find_state_by_sid(i);
        assert(st);
        if (st->init &&
            st->init->action) {
            write2file("    case %d:\n", st->init->id);
            WRITE_LINENO(st->init->action);
            write2file("      %s\n", st->init->action->txt);
            write2file("      break;\n");
        }
    }
    p = "    default: break;\n"
        "  }\n";
    write2file(p);

    p = "  goto hhinitentrylab;\n\n";
    write2file(p);

    p = "hhentrydonelab:\n"
        "  hhinst->hh_state_id[hhregthis] = hhcurst;\n";
    write2file(p);

    if (max_regions > 1) {
        p = "  while (hhinitdep > 0) {\n"
            "    hhselectset = hhpathstack[--hhinitdep];\n"
            "    hhselect = *hhselectset++;\n"
            "    if (hhselect != -1) {\n"
            "      do { HH_TRACE_TRANS(0); } while (0);\n"
            "      hhinitst = *hhselectset;\n"
            "      hhcross = hhcurst = hhinitstack[hhinitdep];\n"
            "      hhpathstack[hhinitdep++] = ++hhselectset;\n"
            "      hhleafst = hhinitst;\n"
            "      while (hhleafst > %d) {\n"
            "        hhleafst = hhsub[hhleafst - %d];\n"
            "      }\n";
        write2file(p, max_leaf_sid, max_leaf_sid + 1);

        p = "      hhregn = 0;\n"
            "      while (hhregn < %d &&\n"
            "        hhinst->hh_state_id[hhregn] != -1) {\n"
            "        hhregn++;\n"
            "      }\n"
            "      hhregthis = hhregn;\n";
        write2file(p, max_regions);

        p = "      goto hhregioninitlab;\n"
            "    }\n"
            "  }\n";
        write2file(p);
    }

    p = "\nhhdonelab:\n"
        "  do { HH_TRACE_TRANS(1); } while (0);\n";
    write2file(p);

    p = "  goto hhtoplab;\n\n";
    write2file(p);

    p = "hhabortlab: HHUSELAB\n"
        "  hhret = -1;\n"
        "hhacceptlab: HHUSELAB\n"
        "  hhinst->hh_running = 0;\n"
        "  if (hhinst->hh_lock_func) {\n"
        "    hhinst->hh_unlock_func(hhinst->hh_private_data);\n"
        "  }\n"
        "  return hhret;\n"
        "}\n\n";
    write2file(p);
}

static void
print_run_api()
{
    int i;
    char *p;

    if (start_st) {
        print_run_i();
    }

    p = "int\n";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_run(";
    write2file(p);

    WRITE_HSM_T(" *hhinst)\n{\n");

    if (!start_st) {
        p = "  (void)hhinst;\n"
            "  return -1;\n";
        write2file(p);
    } else {
        p = "  if (!hhinst) {\n"
            "    return -1;\n"
            "  }\n\n";
        write2file(p);

        p = "  if (hhinst->hh_lock_func) {\n"
            "    hhinst->hh_lock_func(hhinst->hh_private_data);\n"
            "  }\n"
            "  if (hhinst->hh_running) {\n"
            "    if (hhinst->hh_lock_func) {\n"
            "      hhinst->hh_unlock_func(hhinst->hh_private_data);\n"
            "    }\n"
            "    return -1;\n"
            "  }\n\n";
        write2file(p);
 
        p = "  hhinst->hh_state_id[%d] = -1;\n";
        i = 0;
        while (i < max_regions) {
            write2file(p, i);
            i ++;
        }

        p = "  return ";
        write2file(p);

        WRITE_PREFIX(api_prefix);
        p = "hsm_run_i(hhinst);\n";
        write2file(p);
    }
    p = "}\n\n";
    write2file(p);

    p = "int\n";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_run_to(";
    write2file(p);

    WRITE_HSM_T(" *hhinst,\n");
    p = "           const int *hhptr,\n"
        "           unsigned int hhlen)\n{\n";
    write2file(p);

    if (!start_st) {
        p = "  (void)hhinst;\n"
            "  (void)hhptr;\n"
            "  (void)hhlen;\n"
            "  return -1;\n";
        write2file(p);
    } else {
        p = "  int hhx;\n"
            "  int hhsid;\n"
            "  if (!hhinst ||\n"
            "      (!hhptr && hhlen != 0) ||\n"
            "      hhlen > %d) {\n"
            "    return -1;\n"
            "  }\n\n";
        write2file(p, max_regions);

        p = "  if (hhinst->hh_lock_func) {\n"
            "    hhinst->hh_lock_func(hhinst->hh_private_data);\n"
            "  }\n"
            "  if (hhinst->hh_running) {\n"
            "    if (hhinst->hh_lock_func) {\n"
            "      hhinst->hh_unlock_func(hhinst->hh_private_data);\n"
            "    }\n"
            "    return -1;\n"
            "  }\n\n";
        write2file(p);

        p = "  hhx = -1;\n"
            "  while (++hhx < %d) {\n"
            "    if (hhx < hhlen) {\n"
            "      hhsid = 0;\n"
            "      while (hhsid <= %d && hhstatemap[hhsid] != hhptr[hhx]) {\n"
            "        hhsid ++;\n"
            "      }\n"
            "      if (hhsid > %d) {\n"
            "        if (hhinst->hh_lock_func) {\n"
            "          hhinst->hh_unlock_func(hhinst->hh_private_data);\n"
            "        }\n"
            "        return -1;\n"
            "      }\n"
            "    } else {\n"
            "      hhsid = -1;\n"
            "    }\n"
            "    hhinst->hh_state_id[hhx] = hhsid;\n"
            "  }\n";
        write2file(p, max_regions, max_leaf_sid, max_leaf_sid);
 
        p = "  return ";
        write2file(p);

        WRITE_PREFIX(api_prefix);
        p = "hsm_run_i(hhinst);\n";
        write2file(p);
    }
    p = "}\n\n";
    write2file(p);
}

static void
print_init_api()
{
    int i;
    char *p;

    p = "int\n";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_init(";
    write2file(p);

    WRITE_HSM_T("         *hhinst,\n");

    p = "         unsigned int (*hhevf)(void *, void **),\n"
        "         void         (*hhdeff)(void *, unsigned int, void *, int),\n"
        "         void         (*hhlockf)(void *),\n"
        "         void         (*hhunlockf)(void *),\n"
        "         void          *hhsmp)\n"
        "{\n";
    write2file(p);

    p = "  HHUSE(hheventmap);\n"
        "  HHUSE(hhstatemap);\n";
    write2file(p);

    p = "  if (hhinst &&\n"
        "      hhevf &&\n";
    write2file(p);

    if (deferral_is_used) {
        p = "      hhdeff &&\n";
        write2file(p);
    }

    p = "      (!hhlockf || (hhlockf && hhunlockf))) {\n";
    write2file(p);

    p = "    hhinst->hh_event_func   = hhevf;\n"
        "    hhinst->hh_deferral_func= hhdeff;\n"
        "    hhinst->hh_lock_func    = hhlockf;\n"
        "    hhinst->hh_unlock_func  = hhunlockf;\n"
        "    hhinst->hh_private_data = hhsmp;\n";
    write2file(p);

    p = "    hhinst->hh_state_id[%d]  = -1;\n";
    i = 0;
    while (i < max_regions) {
        write2file(p, i);
        i ++;
    }

    p = "    hhinst->hh_running      = 0;\n"
        "    return 0;\n"
        "  }\n"
        "  return -1;\n"
        "}\n\n";
    write2file(p);
}

static void
print_misc_api()
{
    char *p;

    p = "int\n";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_get_state(";
    write2file(p);

    WRITE_HSM_T(" *hhinst)\n{\n");

    p = "  int hhsid = -1;\n"
        "  if (hhinst) {\n"
        "    if (hhinst->hh_lock_func) {\n"
        "      hhinst->hh_lock_func(hhinst->hh_private_data);\n"
        "    }\n"
        "    hhsid = hhinst->hh_state_id[0];\n"
        "    if (hhinst->hh_lock_func) {\n"
        "      hhinst->hh_unlock_func(hhinst->hh_private_data);\n"
        "    }\n"
        "  }\n"
        "  if (hhsid != -1) {\n"
        "    hhsid = hhstatemap[hhsid];\n"
        "  }\n"
        "  return hhsid;\n"
        "}\n\n";
    write2file(p);

    p = "unsigned int\n";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_get_all_states(";
    write2file(p);

    WRITE_HSM_T(" *hhinst,\n");
    p = "                   int *hhptr,\n"
        "                   unsigned int hhlen)\n{\n";
    write2file(p);

    p = "  unsigned int hhnum = 0;\n"
        "  int hhx, hhsid;\n"
        "  if (hhinst && hhptr && hhlen >= %d) {\n"
        "    if (hhinst->hh_lock_func) {\n"
        "      hhinst->hh_lock_func(hhinst->hh_private_data);\n"
        "    }\n"
        "    hhx = -1;\n"
        "    while (++hhx < %d) {\n"
        "      hhsid = hhinst->hh_state_id[hhx];\n"
        "      if (hhsid != -1) {\n"
        "        hhptr[hhnum++] = hhstatemap[hhsid];\n"
        "      }\n"
        "    }\n"
        "    if (hhinst->hh_lock_func) {\n"
        "      hhinst->hh_unlock_func(hhinst->hh_private_data);\n"
        "    }\n"
        "    hhx = hhnum;\n"
        "    while (hhx < hhlen) {\n"
        "      hhptr[hhx++] = -1;\n"
        "    }\n"
        "  }\n";
    write2file(p, max_regions, max_regions);

    p = "  return hhnum;\n"
        "}\n\n";
    write2file(p);

    if (!g_opt_S) {
        return;
    }

    p = "const char *\n";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_event_str(unsigned int hhevt)\n{\n";
    write2file(p);

    if (manual_eid != 0) {
        p = "  unsigned int hheid = 0;\n"
            "  while (hheid <= %d &&\n"
            "         hheventmap[hheid] != hhevt) {\n"
            "    hheid ++;\n"
            "  }\n";
        write2file(p, max_eid);

        p = "  if (hheid <= %d) {\n"
            "    return hheventstr[hheid];\n"
            "  }\n";
        write2file(p, max_eid);
    } else {
        p = "  if (hhevt <= %d) {\n"
            "    return hheventstr[hhevt];\n"
            "  }\n";
        write2file(p, max_eid);
    }

    p = "  return NULL;\n}\n\n";
    write2file(p);

    p = "const char *\n";
    write2file(p);
    WRITE_PREFIX(api_prefix);
    p = "hsm_state_str(int hhst)\n{\n";
    write2file(p);

    if (manual_sid != 0) {
        p = "  int hhsid = 0;\n"
            "  while (hhsid <= %d &&\n"
            "         hhstatemap[hhsid] != hhst) {\n"
            "    hhsid ++;\n"
            "  }\n";
        write2file(p, max_sid);

        p = "  if (hhsid >= 0 && hhsid <= %d) {\n"
            "    return hhstatestr[hhsid];\n"
            "  }\n";
        write2file(p, max_sid);
    } else {
        p = "  if (hhst >= 0 && hhst <= %d) {\n"
            "    return hhstatestr[hhst];\n"
            "  }\n";
        write2file(p, max_sid);
    }

    p = "  return NULL;\n}\n";
    write2file(p);
}

int
append_ext_c(char *str, int len)
{
    /* append .c if needed */
    if (len <= 2 ||
        str[len - 2] != '.' ||
        str[len - 1] != 'c') {
        str[len ++] = '.';
        str[len ++] = 'c';
        str[len   ] = 0;
    }

    return len;
}

void
gen_code_c()
{
    output_file = output_file_ptr;
    print_prod_info();
    print_prolog();
    print_h_header();
    print_c_header();
    print_super_states();
    print_sub_states();
    print_entry_path();
    print_trans_path();
    print_init_trans();
    print_zone_map();
    print_zone_depth();

    print_run_api();
    print_init_api();
    print_misc_api();

    print_epilog();

    if (output_h_fptr != NULL) {
        output_file = output_h_fptr;
        print_prod_info();
        print_h_header();
    }
}
