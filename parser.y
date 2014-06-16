/*
 *  ast-cc is an Abstract Syntax Tree compiler
 *  Copyright (C) 2014  Adam Clark
 *
 *  This program is free software: you can redistribute it and/or modify
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
    #include "ast-cc.h"
	#include <stdio.h>
	#include <cstring>

	extern int curr_lineno;
	extern int parse_error;
	extern char *curr_file;

	extern int yylex(void);

	#define yyerror(x)                                                                              \
            do {                                                                                    \
                fprintf(stderr, "%s[%d]: %s\n", curr_file, curr_lineno, (x));                       \
                parse_error ++;                                                                     \
            } while (0)
%}

%token  TOKEN_ABSTRACT
%token  TOKEN_ATTR
%token  TOKEN_CLASS
%token  TOKEN_DEFINES
%token  TOKEN_EXTERNAL
%token  TOKEN_FACTORY
%token  TOKEN_FUNC
%token  TOKEN_INHERITS
%token  TOKEN_NAME
%token  TOKEN_NO_INIT
%token  TOKEN_NO_INLINES
%token  TOKEN_SPEC
%token  TOKEN_TYPE
%token  TOKEN_VIRTUAL


%token  TOKEN_OPEN
%token  TOKEN_CLOSE
%token  TOKEN_LPAREN
%token  TOKEN_RPAREN
%token  TOKEN_EQ

%token  TOKEN_CODE_LIT
%token  TOKEN_CODE

%token  TOKEN_ERROR

%union {
    const char *error_msg;
    char *code;
    char *code_lit;

    Opts opts;
    Feature *feat;
    FeatureList *featList;
    NodeList *nodes;
    Node *node;
    Ast *ast;
}

%type   <code_lit>  TOKEN_CODE_LIT inherits_opt block_opt
%type   <code>      TOKEN_CODE

%type   <opts>      abstract_opt attr_option_s attr_option func_option attr_option_opt
%type   <feat>      attr_feature factory_feature feature func_feature
%type   <featList>  feature_s
%type   <nodes>     node_s
%type   <node>      node
%type   <ast>       target

%%

target
    : block_opt node_s
        {
            $$ = Ast::factory($1, $2);
            tree = $$;
        }
    ;

block_opt
    :   // empty!!
        {
            $$ = (char *)0;
        }
    | TOKEN_DEFINES TOKEN_CODE
        {
            $2 ++;
            $2[strlen($2) - 1] = 0;
            $$ = $2;
        }
    ;

node_s
    : node_s node
        {
            $$ = NodeList::factory($1, $2);
        }
    | node
        {
            $$ = NodeList::factory(NodeList::empty(), $1);
        }
    ;

node
    : TOKEN_CLASS TOKEN_LPAREN TOKEN_CODE_LIT TOKEN_RPAREN inherits_opt abstract_opt TOKEN_OPEN feature_s TOKEN_CLOSE
        {
            $$ = Node::factory($3, $5, $6, $8);
        }
    | TOKEN_CLASS error TOKEN_CLOSE
        {
            $$ = Node::empty();
        }
    ;

inherits_opt
    :   // empty!!
        {
            $$ = (char *)0;
        }
    | TOKEN_INHERITS TOKEN_LPAREN TOKEN_CODE_LIT TOKEN_RPAREN
        {
            $$ = $3;
        }
    ;

abstract_opt
    :   // empty!!
        {
            $$ = OPT_NONE;
        }
    | TOKEN_ABSTRACT
        {
            $$ = OPT_ABSTRACT;
        }
    ;

feature_s
    :   // empty!!
        {
            $$ = FeatureList::empty();
        }
    | feature_s feature
        {
            $$ = FeatureList::factory($1, $2);
        }
    ;

feature
    : attr_feature
        {
            $$ = $1;
        }
    | func_feature
        {
            $$ = $1;
        }
    | factory_feature
        {
            $$ = $1;
        }
    ;

attr_feature
    : TOKEN_ATTR TOKEN_TYPE TOKEN_LPAREN TOKEN_CODE_LIT TOKEN_RPAREN TOKEN_NAME TOKEN_LPAREN TOKEN_CODE_LIT TOKEN_RPAREN attr_option_opt
        {
            $$ = Attr::factory($4, $8, $10, dft_val);
            dft_val = 0;
        }
    ;

attr_option_opt
    :   // empty!!
        {
            $$ = OPT_NONE;
        }
    | TOKEN_EQ attr_option_s
        {
            $$ = $2;
        }
    ;

attr_option_s
    : attr_option_s attr_option
        {
            $$ = (Opts)($1 | $2);
        }
    | attr_option
        {
            $$ = $1;
        }

attr_option
    : TOKEN_NO_INIT TOKEN_LPAREN TOKEN_CODE_LIT TOKEN_RPAREN
        {
            dft_val = $3;
            $$ = OPT_NO_INIT;
        }
    | TOKEN_NO_INLINES
        {
            $$ = OPT_NO_INLINES;
        }
    ;

factory_feature
    : TOKEN_FACTORY TOKEN_LPAREN TOKEN_CODE_LIT TOKEN_RPAREN
        {
            $$ = Factory::factory($3);
        }
    ;

func_feature
    : TOKEN_FUNC TOKEN_SPEC TOKEN_LPAREN TOKEN_CODE_LIT TOKEN_RPAREN TOKEN_EQ func_option
        {
            $$ = Func::factory($4, (char *)0, $7);
        }
    | TOKEN_FUNC TOKEN_SPEC TOKEN_LPAREN TOKEN_CODE_LIT TOKEN_RPAREN TOKEN_EQ TOKEN_CODE
        {
            $$ = Func::factory($4, $7, OPT_NONE);
        }
    ;

func_option
    : TOKEN_VIRTUAL
        {
            $$ = OPT_VIRTUAL;
        }
    | TOKEN_EXTERNAL
        {
            $$ = OPT_EXTERNAL;
        }
    ;

%%

int curr_lineno = 1;
int parse_error = 0;
char *curr_file = 0;
