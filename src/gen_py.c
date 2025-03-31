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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "gen.h"

extern const int  prod_ver[3];
extern const char *prod_name;

extern event_t *wildc_ev;
extern state_t *start_st;
extern link_t state_link;
extern link_t dest_link;
extern link_t init_link;
extern text_t     *prolog_code;
extern text_t     *epilog_code;
extern text_t     *start_code;

extern const char *output_file_name;
extern const FILE *output_file_ptr;
extern const FILE *output_file;

extern int max_eid;
extern int max_sid;
extern int max_leaf_sid;
extern int max_super_sid;


static void
reassign_dst_id()
{
    state_t *st;
    trans_t *trs;
    dest_t  *dst;
    int id;

    for (st = state_link.head; st; st = st->link) {
        id = 0;
        for (trs = st->trans; trs; trs = trs->sibling) {
            for (dst = trs->dst; dst; dst = dst->sibling) {
                if (dst->guard || dst->action) {
                    dst->id = id ++;
                } else {
                    dst->id = -1;
                }
            }
        }
        id = 0;
        for (dst = st->init; dst; dst = dst->sibling) {
            if (dst->action) {
                dst->id = id ++;
            } else {
                dst->id = -1;
            }
        }
    }
}

static const char *
make_hsm_name()
{
    unsigned int l;
    char *name;
    const char *start, *end;

    l = strlen(output_file_name);
    if (l < 8) l = 8; /* Default */
    name = malloc(l * sizeof(name[0]));
    assert(name);

    start = output_file_name;
    end = strchr(start, '.');
    assert(end); /* it must end with .py */

    while (start < end &&
           (*start < 'A' || *start > 'z' ||
           (*start > 'z' && *start < 'a'))) {
        start ++;
    }

    l = end - start;
    if (l != 0) {
        strncpy(name, start, l);
        name[l] = 0;
        if (name[0] >= 'a') name[0] = 'A' + name[0] - 'a';
    } else {
        strcpy(name, "MyHSM");
    }

    return name;
}

static void
free_hsm_name(const char *name)
{
    free((void *)name);
}

static void
print_prod_info()
{
    write2file("# Generated code, don't edit.\n");
    write2file("# %s\n", prod_name);
    write2file("# Version %d.%d.%d\n\n", prod_ver[0], prod_ver[1], prod_ver[2]);
}

static char *
print_one_line(const char *indent, char *head, unsigned int offset, int first)
{
    char *next;

    next = strchr(head, '\n');
    if (next) {
        *next = 0;
        next ++;
    }

    while (*head == ' ' && offset) {
        head ++;
        offset --;
    }

    if (*head) {
        write2file("%s%s\n", indent, head);
    } else if (first) {
        write2file("%spass\n", indent);
    } else {
        write2file("\n");
    }

    return next;
}

static void
print_stmt(char *indent, text_t *block)
{
    char *txt = block->txt;
    unsigned int len = block->len;
    unsigned int offset = block->col - 1;

    assert(block->col);

    /* remove '}', spaces and new line from tail */
    assert(len && txt[len - 1] == '}');
    txt[-- len] = 0;
    while (len &&
           (txt[len - 1] == ' ' ||
            txt[len - 1] == '\n')) {
        txt[-- len] = 0;
    }

    /* remove '{', spaces and new line from head */
    assert(len && *txt == '{');
    *txt = ' ';
    while (*txt == ' ' ||
           *txt == '\n') {
        if (*txt == '\n') {
            offset = 0;
        } else {
            offset ++;
        }
        txt ++;
    }

    /* print a line a time */
    txt = print_one_line(indent, txt, 0, 1);
    while (txt && *txt) {
        txt = print_one_line(indent, txt, offset, 0);
    }
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
print_state_classes()
{
    int i;
    state_t *st, *cross;
    trans_t *trs, *tr;
    plist_t *evl;
    event_t *ev;
    dest_t  *dst;

    int has_guard, has_action, has_init;
    int comma1, comma2, wc;

    int *flags = malloc((max_eid + 1) * sizeof(flags[0]));
    assert(flags);

    for (st = state_link.head; st; st = st->link) {
        write2file("    class _state_%s(BaseState):\n", st->name->txt);

        /* guard/action pairs */
        has_guard = has_action = 0;
        for (trs = st->trans; trs; trs = trs->sibling) {
            for (dst = trs->dst; dst; dst = dst->sibling) {
                if (dst->guard) {
                    has_guard = 1;
                    write2file("        def _guard_%d(pd):\n", dst->id);
                    write2file("            return%s\n\n", dst->guard->txt);
                }

                if (dst->action) {
                    has_action = 1;
                    write2file("        def _action_%d(pd):\n", dst->id);
                    print_stmt("            ", dst->action);
                    write2file("\n");
                }
            }
        }

        /* initial transitions */
        has_init = 0;
        for (dst = st->init; dst; dst = dst->sibling) {
            if (dst->action) {
                has_init = 1;
                write2file("        def _init_%d(pd):\n", dst->id);
                print_stmt("            ", dst->action);
                write2file("\n");
            }
        }

        if (has_guard) {
            comma2 = 0;
            write2file("        _guards = {\n");
            for (trs = st->trans; trs; trs = trs->sibling) {
                for (dst = trs->dst; dst; dst = dst->sibling) {
                    if (dst->guard) {
                        if (comma2) {
                            write2file(",\n");
                        }
                        write2file("            %d: _guard_%d", dst->id, dst->id);
                        comma2 = 1;
                    }
                }
            }
            write2file(" }\n\n");
        }

        if (has_action) {
            comma2 = 0;
            write2file("        _actions = {\n");
            for (trs = st->trans; trs; trs = trs->sibling) {
                for (dst = trs->dst; dst; dst = dst->sibling) {
                    if (dst->action) {
                        if (comma2) {
                            write2file(",\n");
                        }
                        write2file("            %d: _action_%d", dst->id, dst->id);
                        comma2 = 1;
                    }
                }
            }
            write2file(" }\n\n");
        }

        if (has_init) {
            comma2 = 0;
            write2file("        _inits = {\n");
            for (dst = st->init; dst; dst = dst->sibling) {
                if (dst->action) {
                    if (comma2) {
                        write2file(",\n");
                    }
                    write2file("            %d: _init_%d", dst->st->id, dst->id);
                    comma2 = 1;
                }
            }
            write2file(" }\n\n");
        }

        write2file("        def __init__(self, trace=None):\n");
        write2file("            super().__init__('%s', %d, %d,\n",
                                                 st->name->txt,
                                                 st->id,
                                                 st->super ? st->super->id : -1);
        /* transition table */
        write2file("                             { ");

        comma1 = 0;
        memset(flags, 0, (max_eid + 1) * sizeof(flags[0]));
        for (trs = st->trans; trs; trs = trs->sibling) {
            evl = &trs->evl;
            for (i = 0; i < evl->count; i ++) {
                ev = evl->pa[i];

                if (flags[ev->id] != 0 ||
                    ev == wildc_ev) continue;
                flags[ev->id] = 1;

                if (comma1) {
                    write2file(",\n                               ");
                }
                comma1 = 1;
                write2file("'%s': [ ", ev->name->txt);

                comma2 = 0;
                /* start from the beginning of all trans in order to include wildchar trans */
                for (tr = st->trans; tr; tr = tr->sibling) {
                    if (tr != trs && !test_ev_in_list(ev, &tr->evl)) continue;
                    for (dst = tr->dst; dst; dst = dst->sibling) {
                        if (comma2) {
                            write2file(", ");
                        }
                        comma2 = 1;
                        if (dst->st) {
                            if (dst->type == DST_TYPE_LFROM) {
                                cross = st;
                            } else if (dst->type == DST_TYPE_LTO) {
                                cross = dst->st;
                            } else {
                                cross = search_cross_state(st, dst->st);
                            }
                            write2file("[%d, %d, %d]", dst->id,
                                                       dst->st->id,
                                                       cross ? cross->id : -1);
                        } else {
                            write2file("[%d, %d, %d]", dst->id,
                                                      (dst->type == DST_TYPE_INT) ? -1 : -2,
                                                       -1);
                        }
                    }
                }

                write2file(" ]");
            }
        }

        /* the wildchar event tran go here */
        wc = comma2 = 0;
        for (trs = st->trans; trs; trs = trs->sibling) {
            if (!test_ev_in_list(wildc_ev, &trs->evl)) continue;
            if (!wc) {
                if (comma1) {
                    write2file(",\n                               ");
                }
                write2file("'%s': [ ", wildc_ev->name->txt);
                wc = 1;
            }

            for (dst = trs->dst; dst; dst = dst->sibling) {
                if (comma2) {
                    write2file(", ");
                }
                comma2 = 1;
                if (dst->st) {
                    if (dst->type == DST_TYPE_LFROM) {
                        cross = st;
                    } else if (dst->type == DST_TYPE_LTO) {
                        cross = dst->st;
                    } else {
                        cross = search_cross_state(st, dst->st);
                    }
                    write2file("[%d, %d, %d]", dst->id,
                                               dst->st->id,
                                               cross ? cross->id : -1);
                } else {
                    write2file("[%d, %d, %d]", dst->id,
                                              (dst->type == DST_TYPE_INT) ? -1 : -2,
                                               -1);
                }
            }
        }
        if (wc) {
            write2file(" ]");
        }
        write2file(" }");

        /* initial sub states */
        comma2 = 0;
        if (st->id > max_leaf_sid) {
            write2file(",\n                             subst=[");
            if (st->init) {
                for (dst = st->init; dst; dst = dst->sibling) {
                    if (comma2) {
                        write2file(", ");
                    }
                    comma2 = 1;
                    write2file("%d", dst->st->id);
                }
            } else {
                write2file("%d", st->sub->id);
            }
            write2file("]");
        }

        write2file(",\n                             trace=trace");

        write2file(")\n");
        write2file("\n");

        /* entry/exit function */
        if (st->entry) {
            write2file("        def _entry(self, pd):\n");
            print_stmt("            ", st->entry);
            write2file("\n");
        }
        if (st->exit) {
            write2file("        def _exit(self, pd):\n");
            print_stmt("            ", st->exit);
            write2file("\n");
        }
    }

    free(flags);
}

static void
print_main_class(const char *hsm_name)
{
    int i;
    event_t *ev;
    state_t *st;

    write2file("from inthsm import BaseHSM, BaseState, NullState, Trace\n\n");
    write2file("class %s(BaseHSM):\n", hsm_name);

    write2file("    _events = [");
    i = 0;
    while (i <= max_eid) {
        ev = find_event_by_eid(i);
        if (ev != wildc_ev) {
            if (i > 0) {
                write2file(",");
            }
            write2file("\n        '%s'", ev->name->txt);
        }
        i ++;
    }
    write2file(" ]\n\n");

    print_state_classes();

    write2file("    _trace = Trace()\n\n");

    write2file("    _states = [\n");
    for (i = 0; i <= max_sid; i ++) {
        st = find_state_by_sid(i);
        if (i > 0) {
            write2file(",\n");
        }
        write2file("        _state_%s(_trace)", st->name->txt);
    }
    if (i > 0) {
        write2file(",\n");
    }
    write2file("        NullState(_trace)");
    write2file(" ]\n\n");

    write2file("    def _start(self, pd):\n");
    if (start_code) {
        print_stmt("        ", start_code);
    } else {
        write2file("        pass\n");
    }
    write2file("\n");

    write2file("    _start_state = %d\n\n", start_st->id);

    write2file("    def __init__(self, cb, pd=None):\n");
    write2file("        super().__init__('%s', cb, pd)\n\n", hsm_name);
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
    const char *name;

    /* assign dst id per state */
    reassign_dst_id();

    output_file = output_file_ptr;
    print_prod_info();

    print_prolog();

    name = make_hsm_name();
    print_main_class(name);
    free_hsm_name(name);

    print_epilog();
}
