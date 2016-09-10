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
%{
#include <stdio.h>
#include "lex.h"
#include "defines.h"
#include "gen.h"

void yyerror(char *);
extern char *yytext;
extern unsigned int yylength;
extern yyinput_t *yyinput;

int manual_eid = 0;
int manual_sid = 0;

//#define YYDEBUG 1
%}

%union {
    text_t  *txt;
    state_t *st;
    event_t *ev;
    dest_t  *dt;
    plist_t *pl;
    trans_t *tr;
}

/* tokens */
%token TOK_L_COMMENT
%token TOK_B_COMMENT
%token TOK_CHAR
%token TOK_STRING

%token TOK_C_STMT
%token TOK_C_EXPR
%token TOK_PP

%token TOK_PREFIX
%token TOK_MEID
%token TOK_MSID
%token TOK_EVENT
%token TOK_STATE
%token TOK_HIERA
%token TOK_START
%token TOK_GUARD
%token TOK_ACTION

%token TOK_ENTER
%token TOK_EXIT
%token TOK_DASH
%token TOK_DEFER

%token TOK_LFROM
%token TOK_LTO

%token TOK_IDENTIFIER

%type <txt> identifier
%type <txt> c_expr c_stmt
%type <txt> entry_func exit_func
%type <ev> one_event
%type <st> define_one_state
%type <st> sub_states one_hierachy
%type <dt> init_transitions init_tran
%type <dt> one_destination destination_list
%type <tr> one_event_transitions event_transitions
%type <pl> event_list set_of_event


%%

program:
         def_section trans_section sub_section
		{ if (validate_input() != 0) { YYABORT; } }
       | error
		{ YYABORT; }
       ;

def_section:
         /* empty */
       | c_stmt
		{ save_prolog($1); }
       | c_stmt definitions
		{ save_prolog($1); }
       |        definitions
       ;

definitions:
                     one_definition
       | definitions one_definition
       ;

one_definition:
         define_misc
       | define_events
       | define_states
       | define_hierachies
       | define_start
       ;

define_misc:
         TOK_PREFIX    identifier
		{ if (!save_api_prefix($2->txt, yylength)) { YYABORT; } }
       | TOK_MEID
		{ manual_eid = 1; }
       | TOK_MSID
		{ manual_sid = 1; }
       ;

define_events:
         TOK_EVENT     identifier
		{ event_t *ev = define_event($2);
		  if (ev == NULL) { YYABORT; } }
       | define_events identifier
		{ event_t *ev = define_event($2);
		  if (ev == NULL) { YYABORT; } }
       ;

define_states:
         TOK_STATE     define_one_state
       | define_states define_one_state
       ;

define_one_state:
         identifier
		{ $$ = define_complete_state($1, NULL, NULL);
		  if ($$ == NULL) { YYABORT; } }
       | identifier entry_func
		{ $$ = define_complete_state($1, $2, NULL);
		  if ($$ == NULL) { YYABORT; } }
       | identifier            exit_func
		{ $$ = define_complete_state($1, NULL, $2);
		  if ($$ == NULL) { YYABORT; } }
       | identifier entry_func exit_func
		{ $$ = define_complete_state($1, $2, $3);
		  if ($$ == NULL) { YYABORT; } }
       ;

define_hierachies:
         TOK_HIERA         one_hierachy
       | define_hierachies one_hierachy
       ;

one_hierachy:
         identifier '(' sub_states ')'
		{ $$ = define_incomplete_state($1);
		  if ($$ == NULL) { YYABORT; }
		  state_add_sub($$, $3); }
       ;

sub_states:
         /* empty */
		{ $$ = NULL; }
       |                identifier
		{ $$ = define_incomplete_state($1);
		  if ($$ == NULL) { YYABORT; } }
       | sub_states ',' identifier
		{ state_t *st = define_incomplete_state($3);
		  if (st == NULL) { YYABORT; }
		  state_add_sibling($1, st); }
       |                one_hierachy
       | sub_states ',' one_hierachy
		{ state_add_sibling($1, $3); }
       ;

define_start:
         TOK_START identifier
		{ state_t *st = define_incomplete_state($2);
		  if (!st || define_start_state(st)) { YYABORT; } }
       | TOK_START identifier c_stmt
		{ state_t *st = define_incomplete_state($2);
		  if (!st || define_start_state(st)) { YYABORT; }
                  save_start_code($3); }
       ;

trans_section:
         TOK_PP def_trans
       ;

def_trans:
         /* empty */
       | def_trans transition_spec
       ;

transition_spec:
         identifier ':'                  event_transitions ';'
		{ state_t *st = define_incomplete_state($1);
		  if (!st) { YYABORT; }
		  state_add_transition(st, $3); }
       | identifier ':' init_transitions                   ';'
		{ state_t *st = define_incomplete_state($1);
		  if (!st) { YYABORT; }
		  state_add_init(st, $3); }
       | identifier ':' init_transitions event_transitions ';'
		{ state_t *st = define_incomplete_state($1);
		  if (!st) { YYABORT; }
		  state_add_init(st, $3);
		  state_add_transition(st, $4); }
       ;

init_transitions:
                          init_tran
       | init_transitions init_tran
		{ add_destination($1, $2); }
       ;

init_tran:
         '.' TOK_ENTER identifier trans_action
		{ state_t *st = define_incomplete_state($3);
		  build_guard(NULL);
		  if (st) { $$ = build_destination(st, DST_TYPE_INIT); }
		    else  { $$ = NULL; }
		  if ($$ == NULL) { YYABORT; } }
       ;

event_transitions:
                           one_event_transitions
       | event_transitions one_event_transitions
		{ add_transition($1, $2); }
       ;

one_event_transitions:
         set_of_event destination_list
		{ $$ = build_transition($1, $2);
		  if ($$ == NULL) { YYABORT; } }
       ;

destination_list:
                          one_destination
       | destination_list one_destination
		{ add_destination($1, $2); }
       ;

one_destination:
         TOK_DEFER
		{ build_guard(NULL);
		  build_action(NULL);
		  $$ = build_destination(NULL, DST_TYPE_DEFER);
		  if ($$ == NULL) { YYABORT; } }
       | guard_condition TOK_DASH             trans_action
		{ $$ = build_destination(NULL, DST_TYPE_INT);
		  if ($$ == NULL) { YYABORT; } }
       | guard_condition TOK_ENTER identifier trans_action
		{ state_t *st = define_incomplete_state($3);
		  if (st) { $$ = build_destination(st, DST_TYPE_EXT); }
		    else  { $$ = NULL; }
		  if ($$ == NULL) { YYABORT; } }
       | guard_condition TOK_LFROM identifier trans_action
		{ state_t *st = define_incomplete_state($3);
		  if (st) { $$ = build_destination(st, DST_TYPE_LFROM); }
		    else  { $$ = NULL; }
		  if ($$ == NULL) { YYABORT; } }
       | guard_condition TOK_LTO   identifier trans_action
		{ state_t *st = define_incomplete_state($3);
		  if (st) { $$ = build_destination(st, DST_TYPE_LTO); }
		    else  { $$ = NULL; }
		  if ($$ == NULL) { YYABORT; } }
       ;

guard_condition:
         /* empty */
		{ build_guard(NULL); }
       | c_expr
		{ build_guard($1); }
       ;

trans_action:
         /* empty */
		{ build_action(NULL); }
       | c_stmt
		{ build_action($1); }
       ;

set_of_event:
             event_list
       | '(' event_list ')'
		{ $$ = $2; }
       ;

event_list:
                        one_event
		{ $$ = build_evlist($1);
		  if ($$ == NULL) { YYABORT; } }
       | event_list ',' one_event
		{ $$ = add_evlist($1, $3);
		  if ($$ == NULL) { YYABORT; } }
       ;

one_event:
         identifier
		{ $$ = define_incomplete_event($1);
		  if ($$ == NULL) { YYABORT; }
		  SET_FLAGS($$, FLAG_BE_USED); }
       | '*'
		{ $$ = find_event("*");
		  if ($$ == NULL) { YYABORT; }
		  SET_FLAGS($$, FLAG_BE_USED); }
       ;

sub_section:
         TOK_PP sub_routines
       ;

sub_routines:
         /* empty */
       | c_stmt
		{ save_epilog($1); }
       ;

entry_func:
         TOK_ENTER c_stmt
		{ $$ = $2; }
       ;

exit_func:
         TOK_EXIT c_stmt
		{ $$ = $2; }
       ;

c_expr:
         TOK_C_EXPR
		{ $$ = build_text(yytext, yyinput->lastlineno); 
		  if (!$$) { YYABORT; } }
       ;

c_stmt:
         TOK_C_STMT
		{ $$ = build_text(yytext, yyinput->lastlineno); 
		  if (!$$) { YYABORT; } }
       ;

identifier:
        TOK_IDENTIFIER
		{ $$ = build_text(yytext, yyinput->lastlineno); 
		  if (!$$) { YYABORT; } }
       ;

%% /* sub routines */

void
yyerror(char *s)
{
    print_errmsg(s);
}
