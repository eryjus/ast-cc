/*===================================================================================================================
// parser.yy -- This file is responsible for parsing the ast-cc input into an AST tree for the AST compiler.
//
//    ast-cc is an Abstract Syntax Tree compiler
//    Copyright (C) 2014  Adam Clark
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// This file represents v0.1 of the ast grammar.  This also represents a nearly complete rewrite of the
// ast-cc compiler.  The reason for this is taht in implementing a very abstract AST in the pascal compiler,
// I noticed several shortcomings in the feature/functionality of the current language (along with all those
// damned dollar signs).
//
// This version is going to use '%%' as separators between sections so that it is closer to that of the
// flex and bison source formatting.
//
// -----------------------------------------------------------------------------------------------------------------
//
//    Date     Tracker  Version  Pgmr  Modification
// ----------  -------  -------  ----  -----------------------------------------------------------------------------
// 2016-03-29    N/A     v0.1    ADCL  second version of the ast language
//
//=================================================================================================================*/

%error-verbose

%{
    #include "lists.hh"
    #include "ast-cc.hh"
    #include <stdio.h>
    #include <cstring>

    extern int parse_error;
    extern char *curr_file;
    extern int yylineno;

    extern int yylex(void);

    #define yyerror(x)                                                                              \
            do {                                                                                    \
                extern int yylineno;                                                                \
                fprintf(stderr, "%s[%d]: %s\n", curr_file, yylineno, (x));                          \
                parse_error ++;                                                                     \
            } while (0)
%}

%token          TOK_COLONCOLON          "::"
%token          TOK_COLON               ":"
%token          TOK_SEMI                ";"
%token          TOK_COMMA               ","
%token          TOK_PCTPCT              "%%"
%token          TOK_LPAREN              "("
%token          TOK_RPAREN              ")"

%token          TOK_ABSTRACT            "ABSTRACT"
%token          TOK_ATTR                "ATTR"
%token          TOK_NODE                "NODE"
%token          TOK_TYPE                "TYPE"
%token          TOK_INCLUDE             "INCLUDE"
%token          TOK_METH                "METH"
%token          TOK_NOINIT              "NO-INIT"
%token          TOK_NOINLINES           "NO-INLINES"
%token          TOK_PUBLIC              "PUBLIC"
%token          TOK_PROTECTED           "PROTECTED"
%token          TOK_PRIVATE             "PRIVATE"
%token          TOK_VOID                "VOID"
%token          TOK_INLINE              "INLINE"
%token          TOK_STATIC              "STATIC"
%token          TOK_EXTERNAL            "EXTERNAL"

%token  <name>  TOK_NAME                "name"
%token  <file>  TOK_FILENAME            "filename"
%token  <code>  TOK_CODE                "code"

%token  <msg>   TOK_ERROR


%type   <flags> AttrList AttrSpec MethList MethSpec AttrSpecifiers MethSpecifiers
%type   <parm>  Parm
%type   <pLst>  ParmList Parms

%union {
    const char *msg;
    char *name;
    char *file;
    char *code;
    int flags;

    Parameter *parm;
    ParmList *pLst;
}

%%

target
    : declarations TOK_PCTPCT definitions
    | declarations TOK_PCTPCT definitions TOK_PCTPCT TOK_CODE
        {
            endingCode = $5;
        }

declarations
    : /* empty */
    | declarations declaration

declaration
    : nodedeclaration
    | typedeclaration
    | includedeclaration
    | error TOK_SEMI
        {
            parse_error ++;
        }

nodedeclaration
    : TOK_NODE TOK_NAME TOK_SEMI
        {
            Symbol *n = NULL;
            Node *p = NULL;
            NodeList *wrk;

            for (wrk = First(nodes); More(wrk); wrk = Next(wrk)) {
                Node *w = wrk->elem();

                if (w->Get_Name()->Get_Name() == std::string("Common")) {
                    p = w;
                    break;
                }
            }

            if (LookupSymbol(std::string($2))) {
                parse_error ++;
                fprintf(stderr, "%d: Node name %s is already defined\n", yylineno, $2);
            } else {
                n = AddNodeSymbol(std::string($2));
            }

            nodes = Append(nodes, new NodeList(Node::Factory(p, n), NULL));
        }

    | TOK_NODE TOK_NAME TOK_COLON TOK_NAME TOK_SEMI
        {
            Symbol *n = NULL;
            Node *p = NULL;
            NodeList *wrk;

            for (wrk = First(nodes); More(wrk); wrk = Next(wrk)) {
                Node *w = wrk->elem();

                if (w->Get_Name()->Get_Name() == std::string($4)) {
                    p = w;
                    break;
                }
            }

            if (LookupSymbol(std::string($2))) {
                parse_error ++;
                fprintf(stderr, "%d: Node name %s is already defined\n", yylineno, $2);
            } else {
                n = AddNodeSymbol(std::string($2));
            }

            nodes = Append(nodes, new NodeList(Node::Factory(p, n), NULL));
        }

        | TOK_NODE TOK_NAME TOK_COLON TOK_NAME TOK_ABSTRACT TOK_SEMI
        {
            Symbol *n = NULL;
            Node *p = NULL;
            NodeList *wrk;

            for (wrk = First(nodes); More(wrk); wrk = Next(wrk)) {
                Node *w = wrk->elem();

                if (w->Get_Name()->Get_Name() == std::string($4)) {
                    p = w;
                    break;
                }
            }

            if (LookupSymbol(std::string($2))) {
                parse_error ++;
                fprintf(stderr, "%d: Node name %s is already defined\n", yylineno, $2);
            } else {
                n = AddNodeSymbol(std::string($2));
            }

            Node *t = Node::Factory(p, n);
            t->Set_Flag(ABSTRACT);
            nodes = Append(nodes, new NodeList(t, NULL));
        }

typedeclaration
    : TOK_TYPE TOK_NAME TOK_SEMI
        {
            if (LookupSymbol(std::string($2))) {
                parse_error ++;
                fprintf(stderr, "%d: Type name %s is already defined\n", yylineno, $2);
            } else {
                AddTypeSymbol(std::string($2));
            }
        }

includedeclaration
    : TOK_INCLUDE TOK_FILENAME
        {
            includes = Append(includes, new IncludeList($2, NULL));
        }

definitions
    : /* empty */
    | definitions definition

definition
    : attrdefinition
    | methdefinition
    | error TOK_SEMI
        {
            parse_error ++;
        }

attrdefinition
    : TOK_ATTR TOK_NAME TOK_COLONCOLON TOK_NAME TOK_COLON TOK_NAME AttrSpecifiers TOK_SEMI
        {
            Symbol *t = GetSymbol(std::string($6));

            if (!t) {
                parse_error ++;
                fprintf(stderr, "%d: Undefined attribute type in method %s::%s\n", yylineno, $2, $4);
            }

            Attribute *a = Attribute::Factory($4, t);
            a->Set_Flag((Flags)$7);

            NodeList *wrk;

            for (wrk = First(nodes); More(wrk); wrk = Next(wrk)) {
                Node *n = wrk->elem();
                Symbol *s = n->Get_Name();

                if (!s) {
                    parse_error ++;
                    fprintf(stderr, "%d, Unknown Node name %s\n", yylineno, $2);
                } else if (s->Get_Name() == std::string($2)) {
                    n->Add_Attribute(a);
                    break;
                }
            }
        }

    | TOK_ATTR TOK_NAME TOK_COLONCOLON TOK_NAME TOK_COLON TOK_NAME AttrSpecifiers TOK_NOINIT TOK_CODE TOK_SEMI
        {
            Symbol *t = GetSymbol(std::string($6));

            if (!t) {
                parse_error ++;
                fprintf(stderr, "%d: Undefined attribute type in method %s::%s\n", yylineno, $2, $4);
            }

            Attribute *a = Attribute::Factory($4, t);
            a->Set_Flag((Flags)$7);
            a->Set_Flag(NOINIT);
            a->Set_Code(std::string($9));

            NodeList *wrk;

            for (wrk = First(nodes); More(wrk); wrk = Next(wrk)) {
                Node *n = wrk->elem();
                Symbol *s = n->Get_Name();

                if (!s) {
                    parse_error ++;
                    fprintf(stderr, "%d: Unknown Node name %s\n", yylineno, $2);
                } else if (s->Get_Name() == std::string($2)) {
                    n->Add_Attribute(a);
                    break;
                }
            }
        }

AttrSpecifiers
    : /* empty */
        {
            $$ = 0;
        }

    | AttrList
        {
            $$ = $1;
        }

AttrList
    : AttrSpec
        {
            $$ = $1;
        }

    | AttrList AttrSpec
        {
            $$ = $1 | $2;
        }

AttrSpec
    : TOK_PRIVATE
        {
            $$ = PRIVATE;
        }

    | TOK_PUBLIC
        {
            $$ = PUBLIC;
        }

    | TOK_PROTECTED
        {
            $$ = PROTECTED;
        }

    | TOK_STATIC
        {
            $$ = STATIC;
        }

    | TOK_NOINLINES
        {
            $$ = NOINLINES;
        }

methdefinition
    : TOK_METH TOK_NAME TOK_COLONCOLON TOK_NAME TOK_LPAREN ParmList TOK_RPAREN TOK_COLON TOK_NAME MethSpecifiers TOK_EXTERNAL TOK_SEMI
        {
            Symbol *t = GetSymbol(std::string($9));

            if (!t) {
                parse_error ++;
                fprintf(stderr, "%d: Undefined return type in method %s::%s\n", yylineno, $2, $4);
            }

            Method *m = Method::Factory($4, t);
            m->Set_ParmList($6);
            m->Set_Flag((Flags)$10);
            m->Set_Flag(EXTERNAL);

            NodeList *wrk;

            for (wrk = First(nodes); More(wrk); wrk = Next(wrk)) {
                Node *n = wrk->elem();
                Symbol *s = n->Get_Name();

                if (!s) {
                    parse_error ++;
                    fprintf(stderr, "%d: Unknown Node name %s\n", yylineno, $2);
                } else if (s->Get_Name() == std::string($2)) {
                    n->Add_Method(m);
                    break;
                }
            }
        }

    | TOK_METH TOK_NAME TOK_COLONCOLON TOK_NAME TOK_LPAREN ParmList TOK_RPAREN TOK_COLON TOK_NAME MethSpecifiers TOK_ABSTRACT TOK_SEMI
        {
            Symbol *t = GetSymbol(std::string($9));

            if (!t) {
                parse_error ++;
                fprintf(stderr, "%d: Undefined return type in method %s::%s\n", yylineno, $2, $4);
            }

            Method *m = Method::Factory($4, t);
            m->Set_ParmList($6);
            m->Set_Flag((Flags)$10);
            m->Set_Flag(ABSTRACT);

            NodeList *wrk;

            for (wrk = First(nodes); More(wrk); wrk = Next(wrk)) {
                Node *n = wrk->elem();
                Symbol *s = n->Get_Name();

                if (!s) {
                    parse_error ++;
                    fprintf(stderr, "%d: Unknown Node name %s\n", yylineno, $2);
                } else if (s->Get_Name() == std::string($2)) {
                    n->Add_Method(m);
                    break;
                }
            }
        }

    | TOK_METH TOK_NAME TOK_COLONCOLON TOK_NAME TOK_LPAREN ParmList TOK_RPAREN TOK_COLON TOK_NAME MethSpecifiers TOK_CODE
        {
            Symbol *t = GetSymbol(std::string($9));

            if (!t) {
                parse_error ++;
                fprintf(stderr, "%d: Undefined return type in method %s::%s\n", yylineno, $2, $4);
            }

            Method *m = Method::Factory($4, t);
            m->Set_ParmList($6);
            m->Set_Flag((Flags)$10);
            m->Set_Code(std::string($11));

            NodeList *wrk;

            for (wrk = First(nodes); More(wrk); wrk = Next(wrk)) {
                Node *n = wrk->elem();
                Symbol *s = n->Get_Name();

                if (!s) {
                    parse_error ++;
                    fprintf(stderr, "%d: Unknown Node name %s\n", yylineno, $2);
                } else if (s->Get_Name() == std::string($2)) {
                    n->Add_Method(m);
                    break;
                }
            }
        }


MethSpecifiers
    : /* empty */
        {
            $$ = 0;
        }

    | MethList
        {
            $$ = $1;
        }

MethList
    : MethSpec
        {
            $$ = $1;
        }

    | MethList MethSpec
        {
            $$ = $1 | $2;
        }

MethSpec
    : TOK_PRIVATE
        {
            $$ = PRIVATE;
        }

    | TOK_PUBLIC
        {
            $$ = PUBLIC;
        }

    | TOK_PROTECTED
        {
            $$ = PROTECTED;
        }

    | TOK_STATIC
        {
            $$ = STATIC;
        }

    | TOK_INLINE
        {
            $$ = INLINE;
        }

ParmList
    : Parms
        {
            $$ = $1;
        }

    | TOK_VOID
        {
            $$ = NULL;
        }

Parms
    : Parms TOK_COMMA Parm
        {
            $$ = Append($1, new ParmList($3, NULL));
        }

    | Parm
        {
            $$ = new ParmList($1, NULL);
        }

Parm
    : TOK_NAME TOK_COLON TOK_NAME
        {
            Symbol *t = GetSymbol(std::string($3));

            if (!t) {
                parse_error ++;
                fprintf(stderr, "%d: Unknown type %s in parameter definition\n", yylineno, $3);
            }

            $$ = Parameter::Factory(std::string($1), t);
        }


%%

int parse_error = 0;
char *curr_file = 0;
