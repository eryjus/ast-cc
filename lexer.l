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
	#include "parser.h"

	extern int curr_lineno;
	extern int depth;

	#define MAX_STR		10240

	char string_buf[MAX_STR + 1];
	char *string_ptr = 0;
%}

WS							[ \t]
DIG                         [0-9]
LET                         [_a-zA-Z]

%x      CODE
%x      SPEC
%x      SKIP

%%

\/\/[^\n]*\n                { curr_lineno ++; }
{WS}                        { }
\n                          { curr_lineno ++; }

\$(?i:abstract)             { return TOKEN_ABSTRACT; }
\$(?i:attr)                 { return TOKEN_ATTR; }
\$(?i:class)                { return TOKEN_CLASS; }
\$(?i:defines)              { return TOKEN_DEFINES; }
\$(?i:external)             { return TOKEN_EXTERNAL; }
\$(?i:factory)              { return TOKEN_FACTORY; }
\$(?i:func)                 { return TOKEN_FUNC; }
\$(?i:inherits)             { return TOKEN_INHERITS; }
\$(?i:name)                 { return TOKEN_NAME; }
\$(?i:no-init)              { return TOKEN_NO_INIT; }
\$(?i:no-inlines)           { return TOKEN_NO_INLINES; }
\$(?i:spec)                 { return TOKEN_SPEC; }
\$(?i:type)                 { return TOKEN_TYPE; }
\$(?i:virtual)              { return TOKEN_VIRTUAL; }

\$\{                        { return TOKEN_OPEN; }
\$\}                        { return TOKEN_CLOSE; }
\(                          { depth = 1; string_ptr = string_buf; BEGIN(SPEC); return TOKEN_LPAREN; }
\)                          { return TOKEN_RPAREN; }
=                           { return TOKEN_EQ; }



\{                          {   depth = 1;
                                string_ptr = string_buf;
                                *string_ptr ++ = '{';
                                BEGIN (CODE);
                            }

.                           { yylval.error_msg = "Unrecognized input"; return TOKEN_ERROR; }

<CODE>\{                    {   depth ++;
                                *string_ptr ++ = '{';
                                if (string_ptr - string_buf > MAX_STR) {
									yylval.error_msg = "String literal too long";
									BEGIN(SKIP);
								}
                            }

<CODE>\}                    {   *string_ptr ++ = '}';
                                if (string_ptr - string_buf > MAX_STR) {
                                    yylval.error_msg = "String literal too long";
                                    BEGIN(SKIP);
                                }

                                if (!-- depth) {
                                    *string_ptr = 0;
                                    BEGIN(INITIAL);
                                    yylval.code = strdup(string_buf);
                                    return (TOKEN_CODE);
                                }
                            }

<CODE>\n                    {   *string_ptr ++ = '\n';
                                curr_lineno ++;
                                if (string_ptr - string_buf > MAX_STR) {
									yylval.error_msg = "String literal too long";
									BEGIN(SKIP);
								}
                            }

<CODE>[^{}\n]               {   *string_ptr ++ = yytext[0];
                                if (string_ptr - string_buf > MAX_STR) {
									yylval.error_msg = "String literal too long";
									BEGIN(SKIP);
								}
                            }


<SKIP>[^}]*			        { }
<SKIP>\}   			        {	if (!-- depth) {
                                    BEGIN(INITIAL);
                                    return (TOKEN_ERROR);
                                }
							}

<SPEC>\(                    {   depth ++;
                                *string_ptr ++ = '(';
                                if (string_ptr - string_buf > MAX_STR) {
									yylval.error_msg = "String literal too long";
									BEGIN(SKIP);
								}
                            }

<SPEC>\)                    {   if (!-- depth) {
                                    unput(yytext[0]);
                                    *string_ptr = 0;
                                    BEGIN(INITIAL);
                                    yylval.code_lit = strdup(string_buf);
                                    return (TOKEN_CODE_LIT);
                                }

                                *string_ptr ++ = ')';
                                if (string_ptr - string_buf > MAX_STR) {
									yylval.error_msg = "String literal too long";
									BEGIN(SKIP);
								}
                            }

<SPEC>[^()\n]               {   *string_ptr ++ = yytext[0];
                                if (string_ptr - string_buf > MAX_STR) {
									yylval.error_msg = "String literal too long";
									BEGIN(SKIP);
								}
                            }

<SPEC>\n                    {   *string_ptr ++ = '\n';
                                curr_lineno ++;
                                if (string_ptr - string_buf > MAX_STR) {
									yylval.error_msg = "String literal too long";
									BEGIN(SKIP);
								}
                            }

%%

int depth;

int yywrap(void) { return 1; }

int openBuffer(const char *name)
{
	yyin = fopen(name, "r");

	if (!yyin) return 0;

	yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
	curr_lineno = 1;

	return 1;
}


// -- fixes complaints from gcc; never called.
void dummy(void) { unput(0); }
