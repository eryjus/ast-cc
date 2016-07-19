/*===================================================================================================================
// lexer.ll -- This file is responsible for tokenizing the ast-cc input for the parser.
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
// Reviewing the test/sample.ast file, these are the tokens we need to be able to recognize:
// (punctuation)
// ::
// :
// ;
// <
// >
// (
// )
// {
// }
// %%
// "
//
// (keywords)
// node
// type
// include
// attr
// meth
// no-init
// no-inlines
// abstract
// public
// protected
// private
// void
// inline
// static
// external
//
// (text)
// name
// code
// filename
//
// -----------------------------------------------------------------------------------------------------------------
//
//    Date     Tracker  Version  Pgmr  Modification
// ----------  -------  -------  ----  -----------------------------------------------------------------------------
// 2016-03-29    N/A     v0.1    ADCL  second version of the ast language
//
//=================================================================================================================*/

%option yylineno
 /* %option debug */

%{
    #include "ast-cc.hh"
    #include "parser.hh"

    extern int depth;
    extern int markerCnt;
%}

WS          [ \t]
DIG         [0-9]
LET         [_a-zA-Z]
LF          (\n|\r|\n\r|\r\n)

%x          DUMP
%x          CODE
%x          SKIP
%x          FN1
%x          FN2
%x          VAL
%x          TYP

%%

<TYP,INITIAL>{WS}   { }
{LF}                { }
\/\/[^\n\r]*{LF}    { }

"::"                { return TOK_COLONCOLON; }
":"                 { BEGIN(TYP); return TOK_COLON; }
";"                 { return TOK_SEMI; }
","                 { return TOK_COMMA; }
"<"                 { BEGIN(FN1); yymore(); }
">"                 { yylval.msg = "extra '>' character when not expecting"; return TOK_ERROR; }
"("                 { return TOK_LPAREN; }
")"                 { return TOK_RPAREN; }
"{"                 { BEGIN(CODE); depth = 1; yymore(); }
"}"                 { yylval.msg = "extra '}' character when not expecting"; return TOK_ERROR; }
"%%"                { if (++markerCnt >= 2) { BEGIN(DUMP); } return TOK_PCTPCT; }
\"                  { BEGIN(FN2); yymore(); }

(?i:abstract)       { return TOK_ABSTRACT; }
(?i:attr)           { return TOK_ATTR; }
(?i:node)           { return TOK_NODE; }
(?i:type)           { return TOK_TYPE; }
(?i:include)        { return TOK_INCLUDE; }
(?i:meth)           { return TOK_METH; }
(?i:no-init)        { BEGIN(VAL); depth = 0; return TOK_NOINIT; }
(?i:no-inlines)     { return TOK_NOINLINES; }
(?i:public)         { return TOK_PUBLIC; }
(?i:protected)      { return TOK_PROTECTED; }
(?i:private)        { return TOK_PRIVATE; }
(?i:void)           { return TOK_VOID; }
(?i:inline)         { return TOK_INLINE; }
(?i:static)         { return TOK_STATIC; }
(?i:external)       { return TOK_EXTERNAL; }
<TYP>(?i:void)      { BEGIN(INITIAL); yylval.name = strdup(yytext); return TOK_NAME; }

<TYP,INITIAL>{LET}({LET}|{DIG})* {
                        BEGIN(INITIAL); yylval.name = strdup(yytext); return TOK_NAME;
                    }

.                   { BEGIN(INITIAL); yylval.msg = "Unregocnized character"; return TOK_ERROR; }


<FN1>">"            { yylval.file = strdup(yytext); BEGIN(INITIAL); return TOK_FILENAME; }
<FN1>\"             { yylval.msg = "INCLUDE contains mismatched \" and > delimiters"; BEGIN(SKIP); }

<FN2>\"             { yylval.file = strdup(yytext); BEGIN(INITIAL); return TOK_FILENAME; }
<FN2>">"            { yylval.msg = "INCLUDE contains mismatched < and \" delimiters"; BEGIN(SKIP); }

<FN1,FN2>{LF}       { yylval.msg = "End Of Line found in INCLUDE line"; BEGIN(INITIAL); return TOK_ERROR; }
<FN1,FN2><<EOF>>    { yylval.msg = "End Of File found in INCLUDE line"; BEGIN(INITIAL); return TOK_ERROR; }
<FN1,FN2>.          { yymore(); }

<SKIP>.             { }
<SKIP><<EOF>>       { BEGIN(INITIAL); return TOK_ERROR; }

<CODE>"{"           { depth ++; yymore(); }
<CODE>"}"           { if (--depth == 0) {
                        yylval.code = strdup(yytext);
                        BEGIN(INITIAL);
                        return TOK_CODE;
                      } else yymore();
                    }
<CODE><<EOF>>       { yylval.msg = "Unexpected EOF in AST source"; BEGIN(INITIAL); return TOK_ERROR; }
<CODE>.             { yymore(); }

<VAL>"("            { depth ++; yymore(); }
<VAL>")"            { if (--depth == 0) {
                        yylval.code = strdup(yytext);
                        BEGIN(INITIAL);
                        return TOK_CODE;
                      } else yymore();
                    }
<VAL><<EOF>>        { yylval.msg = "Unexpected EOF in AST source"; BEGIN(INITIAL); return TOK_ERROR; }
<VAL>.              { yymore(); }

<DUMP>(.|{LF})*     { yylval.code = strdup(yytext); BEGIN(INITIAL); return TOK_CODE; }


%%

int depth;
int markerCnt = 0;

int yywrap(void) { return 1; }

int openBuffer(const char *name)
{
    yyin = fopen(name, "r");

    if (!yyin) return 0;

    yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));

    return 1;
}


// -- fixes complaints from gcc; never called.
void dummy(void) { unput(0); }
