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
#ifndef __DEFINES_H__
#define __DEFINES_H__

typedef struct list_s {
    void   *head;
    void   *tail;
    int     count;
} link_t;

typedef struct plist_s {
    void  **pa;
    int     sz;
    int     count;
} plist_t;

enum {
    DEF_TYPE_NULL = 0,
    DEF_TYPE_STATE,
    DEF_TYPE_EVENT,
    DEF_TYPE_TFUNC,
    DEF_TYPE_TRAN,
    DEF_TYPE_DEFER,

    DST_TYPE_INIT,
    DST_TYPE_INT,
    DST_TYPE_DEFER,
    DST_TYPE_EXT,
    DST_TYPE_LFROM,
    DST_TYPE_LTO
};

typedef struct text_s {
    const char     *txt;
    unsigned int    line;
    unsigned int    col;
} text_t;

/* event */
typedef struct event_s {
    int             type;
    text_t         *name;
    int             id;
    int             flags;
    struct event_s *link;
} event_t;

/* state */
typedef struct state_s {
    int             type;
    text_t         *name;
    int             id;
    int             flags;
    int             regions;
    text_t         *entry;
    text_t         *exit;
    struct dest_s  *init;
    struct trans_s *trans;     /* pointer to a list of transitions */
    struct state_s *super;     /* pointer to super state */
    struct state_s *sub;       /* pointer to sub states */
    struct state_s *sibling;   /* pointer to sibling states */
    struct state_s *last;      /* pointer to last sibling state */
    struct state_s *link;      /* link of all of states */
} state_t;

/* transition destination */
typedef struct dest_s {
    int             type;
    int             id;
    struct state_s *st;
    text_t         *guard;
    text_t         *action;
    struct dest_s  *sibling;
    struct dest_s  *last;
    struct dest_s  *link;
} dest_t;

/* transition */
typedef struct trans_s {
    int             type;
    int             id;
    plist_t         evl;
    dest_t         *dst;
    struct trans_s *sibling;
    struct trans_s *last;
    struct trans_s *link;
} trans_t;

/* flags */
#define FLAG_DEFINED    (1 << 0)
#define FLAG_BE_USED    (1 << 1)
#define FLAG_EV_STAR    (1 << 2)

#define CHECK_FLAGS(P, FLAGS)   (((P)->flags & (FLAGS)) == (FLAGS))
#define SET_FLAGS(P, FLAGS)      ((P)->flags |= (FLAGS))

int init_objects();
void free_objects();

void save_prolog(text_t *);
void save_epilog(text_t *);
void save_start_code(text_t *);
event_t *find_event(const char *);
event_t *define_event(text_t *);
event_t *define_incomplete_event(text_t *);
state_t *define_complete_state(text_t *, text_t *, text_t *);
state_t *define_incomplete_state(text_t *);
state_t *state_add_sibling(state_t *, state_t *);
state_t *state_add_sub(state_t *, state_t *);
int define_start_state(state_t *);

void build_guard(text_t *);
void build_action(text_t *);

dest_t *build_destination(state_t *, int);
void add_destination(dest_t *, dest_t *);

trans_t *build_transition(plist_t *, dest_t *);
void add_transition(trans_t *, trans_t *);
void state_add_init(state_t *, dest_t *);
void state_add_transition(state_t *, trans_t *);

plist_t *build_evlist(event_t *);
plist_t *add_evlist(plist_t *, event_t *);
int test_ev_in_list(event_t *, plist_t *);
state_t *search_cross_state(state_t *, state_t *);
event_t *find_event_by_eid(int);
state_t *find_state_by_sid(int);

char *save_api_prefix(const char *, unsigned int);

text_t *build_text(const char *, unsigned int, unsigned int);

int validate_input();

#endif /* __DEFINES_H__ */
