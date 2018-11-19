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
#include <stdarg.h>
#include <assert.h>

#include "lex.h"
#include "defines.h"

extern int g_opt_W;

event_t *wildc_ev = NULL;
state_t *start_st = NULL;
link_t state_link = { 0 };
link_t event_link = { 0 };
link_t trans_list = { 0 };
link_t init_link  = { 0 };
link_t dest_link  = { 0 };
text_t *prolog_code = NULL;
text_t *epilog_code = NULL;
text_t *start_code = NULL;
const char *api_prefix = NULL;
char *macro_prefix = NULL;
int   deferral_is_used = 0;

#define TEXT_ARRAY_SZ          100
typedef struct text_array_s {
    text_t               array[TEXT_ARRAY_SZ];
    unsigned int         index;
    struct text_array_s *prev;
} text_array_t;
static text_array_t *text_link;

static plist_t evt_buff = { 0 };

static struct {
    text_t *guard;
    text_t *action;
} a_tfunc = { 0 };

static text_t wildc_text = { "*", 0 };

#define WL_IMPLICIT    2
#define WL_DUPLICATE   1
#define WL_UNUSED      1

static void
warning(int level, const char *format, ...)
{   
    if (level <= g_opt_W) {
        fprintf(stderr, "Warning: ");
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);
    }
}

static event_t *
new_event(text_t *name)
{
    event_t *ev = calloc(1, sizeof(event_t));
    if (!ev) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }
    /* add to link */
    if (event_link.head == NULL) {
        event_link.head = event_link.tail = ev;
    } else {
        ((event_t *)event_link.tail)->link = ev;
        event_link.tail = ev;
    }
    event_link.count ++;

    ev->type = DEF_TYPE_EVENT;
    ev->name = name;
    ev->id   = -1;

    return ev;
}

static state_t *
new_state(text_t *name)
{
    state_t *st = calloc(1, sizeof(state_t));
    if (!st) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }
    /* add to link */
    if (state_link.head == NULL) {
        state_link.head = state_link.tail = st;
    } else {
        ((state_t *)state_link.tail)->link = st;
        state_link.tail = st;
    }
    state_link.count ++;

    st->type = DEF_TYPE_STATE;
    st->name = name;
    st->id   = -1; /* assign id in the end of parsing */

    return st;
}

static void
free_events()
{
    event_t *ev = event_link.head;
    while (ev) {
        event_t *p = ev->link;
        free(ev);
        ev = p;
    }
    //printf("%d events have been freed.\n", event_link.count);
    event_link.head = event_link.tail = NULL;
    event_link.count = 0;
}

static void
free_states()
{
    state_t *st = state_link.head;
    while (st) {
        state_t *p = st->link;
        free(st);
        st = p;
    }
    //printf("%d states have been freed.\n", state_link.count);
    state_link.head = state_link.tail = NULL;
    state_link.count = 0;
}

static void
free_dests()
{
    dest_t *dt = dest_link.head;
    while (dt) {
        dest_t *p = dt->link;
        free(dt);
        dt = p;
    }
    //printf("%d dests have been freed.\n", dest_link.count);
    dest_link.head = dest_link.tail = NULL;
    dest_link.count = 0;

    dt = init_link.head;
    while (dt) {
        dest_t *p = dt->link;
        free(dt);
        dt = p;
    }
    //printf("%d inits have been freed.\n", init_link.count);
    init_link.head = init_link.tail = NULL;
    init_link.count = 0;
}

static void
free_evlist()
{
    if (evt_buff.pa) {
        free(evt_buff.pa);
        //printf("%d sz in evt_buff have been freed.\n", evt_buff.sz);
    }
    evt_buff.pa    = NULL;
    evt_buff.sz    = 0;
    evt_buff.count = 0;
}

static void
free_trans()
{
    trans_t *trs = trans_list.head;
    while (trs) {
        trans_t *p = trs->link;
        free(trs->evl.pa);
        free(trs);
        trs = p;
    }
    //printf("%d trans have been freed.\n", trans_list.count);
    trans_list.head = trans_list.tail = NULL;
    trans_list.count = 0;
}

static void
free_text_link()
{
    text_array_t *p;
    while (text_link != NULL) {
        p = text_link->prev;
        free(text_link);
        text_link = p;
    }
}

int
init_objects()
{
    event_t *ev = new_event(&wildc_text);
    if (!ev) {
        return -1;
    }
    SET_FLAGS(ev, FLAG_EV_STAR |
                  FLAG_DEFINED);
    wildc_ev = ev;

    evt_buff.pa = calloc(10, sizeof(void *));
    if (!evt_buff.pa) {
        return -1;
    }
    evt_buff.sz = 10;
    evt_buff.count = 0;

    text_link = malloc(sizeof(text_array_t));
    if (!text_link) {
        return -1;
    }
    text_link->index = 0;
    text_link->prev = NULL;

    return 0;
}

void
free_objects()
{
    free_events();
    free_states();
    free_dests();
    free_evlist();
    free_trans();
    free_text_link();

    if (macro_prefix) {
        free(macro_prefix);
    }
}

event_t *
find_event(const char *name)
{
    event_t *ev = event_link.head;

    while (ev && (strcmp(ev->name->txt, name) != 0)) {
        ev = ev->link;
    }

    return ev;
}

static state_t *
find_state(const char *name)
{
    state_t *st = state_link.head;

    while (st && (strcmp(st->name->txt, name) != 0)) {
        st = st->link;
    }

    return st;
}

void
save_prolog(text_t *c_stmt)
{
    prolog_code = c_stmt;
}

void
save_epilog(text_t *c_stmt)
{
    epilog_code = c_stmt;
}

void
save_start_code(text_t *c_stmt)
{
    start_code = c_stmt;
}

event_t *
define_event(text_t *name)
{
    event_t *ev = NULL;

    if (find_event(name->txt)) {
        fprintf(stderr, "Event (%s) has already been defined.\n", name->txt);
        return NULL;
    }

    if (find_state(name->txt)) {
        fprintf(stderr, "Event (%s) has been defined as a state.\n", name->txt);
        return NULL;
    }

    ev = new_event(name);
    SET_FLAGS(ev, FLAG_DEFINED);

    return ev;
}

event_t *
define_incomplete_event(text_t *name)
{
    event_t *ev = NULL;

    if (find_state(name->txt)) {
        fprintf(stderr, "(%s) has been defined as a state.\n", name->txt);
        return NULL;
    }

    ev = find_event(name->txt);
    if (!ev) {
        ev = new_event(name);
    }

    return ev;
}

state_t *
define_complete_state(text_t *name,
                      text_t *entry,
                      text_t *exit)
{
    state_t *st = NULL;

    if (find_event(name->txt)) {
        fprintf(stderr, "State (%s) has been defined as an event.\n", name->txt);
        return NULL;
    }

    st = find_state(name->txt);
    if (st) {
        if (CHECK_FLAGS(st, FLAG_DEFINED)) {
            fprintf(stderr, "State (%s) has already been completely defined.\n", name->txt);
            return NULL;
        }
    } else {
        st = new_state(name);
    }
    if (st) {
        assert(st->entry == NULL);
        assert(st->exit == NULL);
        st->entry = entry;
        st->init = NULL;
        st->exit  = exit;
        SET_FLAGS(st, FLAG_DEFINED);
    }

    return st;
}

state_t *
define_incomplete_state(text_t *name)
{
    state_t *st = NULL;

    if (find_event(name->txt)) {
        fprintf(stderr, "(%s) has been defined as an event.\n", name->txt);
        return NULL;
    }

    st = find_state(name->txt);
    if (!st) {
        st = new_state(name);
    }

    return st;
}

state_t *
state_add_sibling(state_t *st,
                  state_t *sib)
{
    if (sib->super) {
        fprintf(stderr,
                "State (%s) has been defined as a sub state of state (%s).\n",
                sib->name->txt, sib->super->name->txt);
        return NULL;
    }

    if (st == NULL) {
        st = sib;
        st->last = sib;
    } else {
        if (st->sibling == NULL) {
            st->sibling = sib;
        } else {
            st->last->sibling = sib;
        }
        st->last = sib;
    }

    return st;
}

state_t *
state_add_sub(state_t *st,
              state_t *sub)
{
    if (st->sub) {
        fprintf(stderr,
                "State (%s) has been defined with sub states already.\n",
                st->name->txt);
        return NULL;
    } else {
        /* set super states */
        state_t *b = sub;
        while (b) {
            assert(b->super == NULL);
            /* check loop */
            state_t *p = st;
            while (p) {
                if (b == p) {
                    fprintf(stderr,
                            "Sub state (%s) and super state (%s) forms a loop.\n",
                            b->name->txt,
                            st->name->txt);
                    return NULL;
                }
                p = p->super;
            }
            b->super = st;
            b = b->sibling;
        }
    }
    /* set sub states */
    st->sub = sub;

    return st;
}

int
define_start_state(state_t *st)
{
    if (start_st != NULL) {
        fprintf(stderr,
                "Start state has already been defined as (%s).",
                start_st->name->txt);
        return -1;
    }

    start_st = st;
    SET_FLAGS(st, FLAG_BE_USED);
#if 0
    /* move to leaf sub state */
    while (start_st->sub) {
        start_st = start_st->sub;
    }
    SET_FLAGS(start_st, FLAG_BE_USED);
#endif

    return 0;
}

plist_t *
build_evlist(event_t *ev)
{
    plist_t *list;

    assert(evt_buff.count == 0);

    list = &evt_buff;

    list->pa[list->count] = ev;
    list->count ++;

    return list;
}

plist_t *
add_evlist(plist_t *list,
           event_t *ev)
{
    int i;

    /* ignore the rest of events after wildchar */
    assert(list->count > 0);
    if (list->pa[0] == wildc_ev) {
        return list;
    }

    /* ignore all preceeding events ahead of wildchar */
    if (ev == wildc_ev) {
        list->count = 0;
    }

    /* check for duplicated one */
    i = 0;
    while (i < list->count) {
        if (list->pa[i] == ev) {
            warning(WL_DUPLICATE, "event %s is already in the event list.\n",
                    ev->name->txt);
            return list;
        }
        i ++;
    }

    if (list->count == list->sz) {
        size_t newsz = 2 * list->sz;
        void **new_list = calloc(newsz, sizeof(void *));
        if (!new_list) {
            fprintf(stderr, "Memory allocation failed.\n");
            return NULL;
        }
        memcpy(new_list, &list->pa[0], list->sz * sizeof(void *));
        free(list->pa);
        list->pa = new_list;
        list->sz = newsz;
    }
 
    list->pa[list->count] = ev;
    list->count ++;

    return list;
}

trans_t *
build_transition(plist_t *evl,
                 dest_t *dst)
{
    trans_t *trs = calloc(1, sizeof(trans_t));
    void   **lst = calloc(evl->count, sizeof(void *));
    if (trs) {
        /* add to link */
        if (trans_list.head == NULL) {
            trans_list.head = trans_list.tail = trs;
        } else {
            ((trans_t *)trans_list.tail)->link = trs;
            trans_list.tail = trs;
        }
        trans_list.count ++;
    }
    if (!trs || !lst) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }

    memcpy(lst, evl->pa, evl->count * sizeof(void *));
    trs->type = DEF_TYPE_TRAN;
    trs->evl.pa = lst;
    trs->evl.sz    = evl->count;
    trs->evl.count = evl->count;
    trs->dst = dst;
    trs->last = trs;

    /* reset event list buffer */
    evl->count = 0;

    return trs;
}

void
add_transition(trans_t *trs,
               trans_t *sib)
{
    trs->last->sibling = sib;
    trs->last = sib->last;
}

void
state_add_init(state_t *st,
               dest_t *init)
{
    if (!st->init) {
        st->init = init;
    } else {
        st->init->last->sibling = init;
        st->init->last = init->last;
    }
}

void
state_add_transition(state_t *st,
                     trans_t *tr)
{
    if (!st->trans) {
        st->trans = tr;
    } else {
        st->trans->last->sibling = tr;
        st->trans->last = tr->last;
    }
    SET_FLAGS(st, FLAG_BE_USED);
    if (!start_st) {
        start_st = st;
        warning(WL_IMPLICIT, "start state is set to: (%s) implicitly.\n", st->name->txt);
    }
}

void
build_guard(text_t *guard)
{
    a_tfunc.guard  = guard;
}

void
build_action(text_t *action)
{
    a_tfunc.action = action;
}

dest_t *
build_destination(state_t *st, int type)
{
    link_t *link;
    dest_t *dt = NULL;

    if (st) {
        SET_FLAGS(st, FLAG_BE_USED);
    }

    dt = calloc(1, sizeof(dest_t));
    if (!dt) {
        fprintf(stderr, "Memory allocation failed.\n");
        return NULL;
    }
    /* add to link */
    if (type == DST_TYPE_INIT) {
        link = &init_link;
    } else {
        link = &dest_link;
    }
    if (link->head == NULL) {
        link->head = link->tail = dt;
    } else {
        ((dest_t *)link->tail)->link = dt;
        link->tail = dt;
    }

    dt->type = type;
    dt->st   = st;
    dt->guard  = a_tfunc.guard;
    dt->action = a_tfunc.action;
    dt->last = dt;

    dt->id = link->count ++;

    if (dt->type == DST_TYPE_DEFER) {
        deferral_is_used = 1;
    }

    return dt;
}

void
add_destination(dest_t *dt,
                dest_t *sibling)
{
    assert(sibling->sibling == NULL);

    dt->last->sibling = sibling;
    dt->last = sibling;
}

int
test_ev_in_list(event_t *ev, plist_t *evl)
{
    int i = 0;
    while (i < evl->count) {
        if (evl->pa[i] == ev ||
            evl->pa[i] == wildc_ev) {
            return 1;
        }
        i ++;
    }
    return 0;
}

state_t *
search_cross_state(state_t *src, state_t *dst)
{
    state_t *s, *d;
    s = src->super;
    while (s) {
        d = dst->super; /* external transition only, no local transition */
        while (d) {
            if (s == d) {
                return s;
            }
            d = d->super;
        }
        s = s->super;
    }

    return NULL;
}

event_t *
find_event_by_eid(int eid)
{
    event_t *ev = event_link.head;
    while (ev &&
           ev->id != eid) {
        ev = ev->link;
    }

    return ev;
}

state_t *
find_state_by_sid(int sid)
{
    state_t *st = state_link.head;
    while (st &&
           st->id != sid) {
        st = st->link;
    }

    return st;
}

char *
save_api_prefix(const char *p, unsigned int len)
{
    assert(len > 0);

    api_prefix = p;

    macro_prefix = malloc(len + 1);
    if (!macro_prefix) {
        return NULL;
    }
    macro_prefix[len] = 0;

    do {
        len -= 1;
        macro_prefix[len] = toupper(api_prefix[len]);
    } while (len);

    return macro_prefix;
}

text_t *
build_text(char *s, unsigned int len, unsigned int line, unsigned int col)
{
    text_t *t;
    text_array_t *a;

    if (text_link->index >= TEXT_ARRAY_SZ) {
        a = malloc(sizeof(text_array_t));
        if (!a) {
            return NULL;
        }
        a->index = 0;
        a->prev = text_link;
        text_link = a;
    }

    t = &text_link->array[text_link->index ++];

    t->txt = s;
    t->len = len;
    t->line = line;
    t->col = col;

    return t;
}

static int
is_super_sub(state_t *super, state_t *sub)
{
    while (sub) {
        if (sub == super) {
            return 1;
        }
        sub = sub->super;
    }

    return 0;
}

static int
is_region_closed(state_t *st, state_t *edge)
{
    state_t *sub;
    trans_t *tr;
    dest_t  *dst;
    state_t *dstst = NULL;

    if (!st->init ||
        !st->init->sibling) {
        sub = st->sub;
        while (sub) {
            if (!is_region_closed(sub, edge)) {
                return 0;
            }
            sub = sub->sibling;
        }
    }

    tr = st->trans;
    while (tr) {
        dst = tr->dst;
        while (dst) {
            dstst = dst->st;
            if (!dstst) { /* internal transition or deferral */
            } else if (dstst == edge) {
                if (dst->type != DST_TYPE_LTO &&
                    dst->type != DST_TYPE_LFROM) {
                    fprintf(stderr, "Error: state (%s) transitions to it's region (%s) is not allowed.\n",
                            st->name->txt, edge->name->txt);
                    return 0;
                }
            } else {
                while (dstst &&
                       dstst->super != edge) {
                    dstst = dstst->super;
                }
                if (!dstst) {
                    fprintf(stderr, "Error: state (%s) transitions to outside of the region (%s) is not allowed.\n",
                            st->name->txt, edge->name->txt);
                    return 0;
                }
            }
            dst = dst->sibling;
        }
        tr = tr->sibling;
    }

    return 1;
}

static int
is_zone_closed(state_t *st)
{
    trans_t *tr = st->trans;
    dest_t  *dst;

    while (tr) {
        dst = tr->dst;
        while (dst) {
            if (dst->type == DST_TYPE_LFROM ||
               (dst->type == DST_TYPE_LTO &&
                dst->st == st)) {
                return 0;
            }
            dst = dst->sibling;
        }
        tr = tr->sibling;
    }

    return 1;
}

int
validate_input()
{
    int ret = 0;
    event_t *ev;
    state_t *st, *sub;
    trans_t *tr;
    dest_t  *init, *dt;

    ev = event_link.head;
    while (ev) {
        if (ev != wildc_ev) {
            if (!CHECK_FLAGS(ev, FLAG_BE_USED)) {
                warning(WL_UNUSED, "event (%s) defined but not being used.\n", ev->name->txt);
            } else if (!CHECK_FLAGS(ev, FLAG_DEFINED)) {
                warning(WL_IMPLICIT, "implicitly defined event (%s).\n", ev->name->txt);
            }
        }
        ev = ev->link;
    }

    st = state_link.head;
    while (st) {
        if (!CHECK_FLAGS(st, FLAG_DEFINED)) {
            if (CHECK_FLAGS(st, FLAG_BE_USED)) {
                warning(WL_IMPLICIT, "implicitly defined state (%s).\n", st->name->txt);
            } else {
                warning(WL_UNUSED, "implicitly defined state (%s) but not being used.\n", st->name->txt);
            }
        } else if (!CHECK_FLAGS(st, FLAG_BE_USED)) {
            warning(WL_UNUSED, "explicitly defined state (%s) but not being used.\n", st->name->txt);
        }
        st = st->link;
    }

    /* validate local transition from/to super states */
    st = state_link.head;
    while (st) {
        tr = st->trans;
        while (tr) {
            dt = tr->dst;
            while (dt) {
                if (dt->type == DST_TYPE_LFROM &&
                    (!st->sub || !is_super_sub(st, dt->st))) {
                    fprintf(stderr, "Error: Local transition from state (%s) to state (%s) is not "
                                    "a super-sub state pair.\n",
                            st->name->txt, dt->st->name->txt);
                    ret = -1;
                } else if (dt->type == DST_TYPE_LTO &&
                           (!st->super || !is_super_sub(dt->st, st))) {
                    fprintf(stderr, "Error: Local transition from state (%s) to state (%s) is not "
                                    "a sub-super state pair.\n",
                            st->name->txt, dt->st->name->txt);
                    ret = -1;
                }
                dt = dt->sibling;
            }
            tr = tr->sibling;
        }
        st = st->link;
    }

    /* validate initial transition of super states */
    st = state_link.head;
    while (st) {
        init = st->init;
        if (init) {
            do {
                sub = init->st;
                while (sub && sub->super != st) {
                    sub = sub->super;
                }
                if (!sub) {
                    fprintf(stderr, "Error: State (%s) initial transition to a non-sub state (%s).\n",
                            st->name->txt, init->st->name->txt);
                    ret = -1;
                }
                init = init->sibling;
            } while (init);
        } else if (st->sub) {
            warning(WL_IMPLICIT, "state (%s) initial transition is set to the first sub state (%s) implicitly.\n",
                            st->name->txt, st->sub->name->txt);
        }
        st = st->link;
    }

    /* validate orthogonal regions */
    st = state_link.head;
    while (st) {
        init = st->init;
        if (!init ||
            !init->sibling) {
            st = st->link;
            continue;
        }

        /* validate zone re-entry */
        if (!is_zone_closed(st)) {
            fprintf(stderr, "Error: zone (%s) local transitions to inside or itself is not allowed.\n",
                    st->name->txt);
            ret = -1;
        }

        while (init) {
            if (init->st->super != st) {
                fprintf(stderr, "Error: region (%s) is not a first-level sub state of zone (%s).\n",
                        init->st->name->txt, st->name->txt);
                ret = -1;
                init = init->sibling;
                continue;
            }
            dt = st->init;
            while (dt != init &&
                   dt->st != init->st) {
                dt = dt->sibling;
            }
            if (dt != init) {
                fprintf(stderr, "Error: region (%s) is already in zone (%s).\n",
                        init->st->name->txt, st->name->txt);
                ret = -1;
                init = init->sibling;
                continue;
            }
            /* validate region self close */
            if (!is_region_closed(init->st, init->st)) {
                ret = -1;
            }

            init = init->sibling;
        }

        sub = st->sub;
        while (sub) {
            init = st->init;
            while (init &&
                   init->st != sub) {
                init = init->sibling;
            }
            if (!init) {
                fprintf(stderr, "Error: state (%s) is not a region state in zone (%s).\n",
                        sub->name->txt, st->name->txt);
                ret = -1;
            }
            sub = sub->sibling;
        }

        st = st->link;
    }

    return ret;
}
