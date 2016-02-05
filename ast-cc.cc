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

// ============================================================================================
//
//    File: ast-cc.cc
//    This is the main source file for the ast-cc program.  ast-cc is an Abstract Syntax
//    Tree compiler that will translate a specification file into a target language --
//    initially C++.
//
//    Date: 03-Jun-14
//    Programmer: Adam Clark
//
// ============================================================================================

// --------------------------------------------------------------------------------------------
// The following provides a sample of the "source" and the resulting C++ code:
//
// $class Expression $abstract ${
//    $attr int line_no
//    $attr string file
//    $attr int type $no-init
//
//    $func void semant(void) = $virtual
//    $func void print(ostream &s) = { s << file << ": " << line_no; }
// $}
//
// $class ID : Expression ${
//    $factory Expression
//    $attr string id_string $no-inlines
//
//    $func void semant(void) = $external
//    $func Symbol *GetSymbol(void) = { return SymTable->Get(id_string); }
// $}
//
// --------------------------------------------------------------------------------------------
//
// class Expression {
// protected:
//    Expression(int v1, string v2) : line_no(v1), file(v2) {}
//
// public:
//    static Expression *empty(void) { return (Expression *)0; }
//
// protected:
//    int line_no;
//    string file;
//    int type;
//
// public:
//    virtual void semant(void) = 0;
//    virtual void print(ostream &s) { s << file << ": " << line_no; }
//
// public:
//    virtual int Get_line_no(void) const { return line_no; }
//    virtual void Set_line_no(int v) { line_no = v; }
//
// public:
//    virtual string Get_file(void) const { return file; }
//    virtual void Set_file(string v) { file = v; }
//
// public:
//    virtual int Get_type(void) const { return type; }
//    virtual void Set_type(int v) { type = v; }
// };
//
//
// class ID : public Expression {
// public:
//    static Expression *factory(int v1, string v2, string v3) { return new ID(v1, v2, v3); }
//    static Expression *empty(void) { return (Expression *)0; }
//
// protected:
//    ID(int v1, string v2, string v3) : Expression(v1, v2), id_string(v3) {}
//
// protected:
//    string id_string;
//
// public:
//    virtual void semant(void);
//    virtual Symbol *GetSymbol(void) { return SymTable->Get(id_string); }
// };
//
// --------------------------------------------------------------------------------------------

#include "ast-cc.h"
#include "parser.h"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>

using namespace std;

extern int openBuffer(const char *name);
extern int yylex(void);
extern int parse_error;
extern int yydebug;
extern char *curr_file;

char *dft_val = 0;
bool prtComma = false;

Ast *tree;


// -- Semantic checking is extremely simple...  pass any errors on to the C++ code
//    ----------------------------------------------------------------------------
void semant(NodeList *list)
{
    for (NodeList *l = list; l; l = l->Get_prev()) {
        Node *n = l->Get_node();

        n->Set_factType(n->Get_name());

        if (n) {
            for (FeatureList *f = n->Get_feats(); f; f = f->Get_prev()) {
                Feature *ff = f->Get_feat();

                if (ff->IsFactory()) {
                    Factory *fact = (Factory *)ff;

                    n->Set_factType(fact->Get_type());
                    break;
                }
            }
        }
    }
}

void semant(Ast *tree)
{
    semant(tree->Get_nodes());
}

void PrintTok(int tok)
{
    switch (tok) {
    case TOKEN_ABSTRACT:
        cout << "ABSTRACT" << endl;
        break;

    case TOKEN_ATTR:
        cout << "ATTR" << endl;
        break;

    case TOKEN_CLASS:
        cout << "CLASS" << endl;
        break;

    case TOKEN_EXTERNAL:
        cout << "EXTERNAL" << endl;
        break;

    case TOKEN_FACTORY:
        cout << "FACTORY" << endl;
        break;

    case TOKEN_FUNC:
        cout << "FUNC" << endl;
        break;

    case TOKEN_INHERITS:
        cout << "INHERITS" << endl;
        break;

    case TOKEN_NAME:
        cout << "NAME" << endl;
        break;

    case TOKEN_NO_INIT:
        cout << "NO_INIT" << endl;
        break;

    case TOKEN_NO_INLINES:
        cout << "NO_INLINES" << endl;
        break;

    case TOKEN_SPEC:
        cout << "SPEC" << endl;
        break;

    case TOKEN_TYPE:
        cout << "TYPE" << endl;
        break;

    case TOKEN_VIRTUAL:
        cout << "VIRTUAL" << endl;
        break;

    case TOKEN_OPEN:
        cout << "OPEN" << endl;
        break;

    case TOKEN_CLOSE:
        cout << "CLOSE" << endl;
        break;

    case TOKEN_LPAREN:
        cout << "LPAREN" << endl;
        break;

    case TOKEN_RPAREN:
        cout << "RPAREN" << endl;
        break;

    case TOKEN_EQ:
        cout << "EQ" << endl;
        break;

    case TOKEN_CODE_LIT:
        cout << "CODE_LIT: " << yylval.code_lit << endl;
        break;

    case TOKEN_CODE:
        cout << "CODE: " << yylval.code << endl;
        break;

    case TOKEN_ERROR:
        cout << "ERROR";
        break;

    }
}

Node *NodeList::FindNodeByName(char *n)
{
    if (!n) return 0;
    if (strcmp(Get_node()->Get_name(), n) == 0) return Get_node();
    else if (Get_prev()) return Get_prev()->FindNodeByName(n);
    else return 0;
}

void NodeList::BuildParents(void)
{
    if (Get_prev()) Get_prev()->BuildParents();

    Get_node()->Set_parent(FindNodeByName(Get_node()->Get_inherits()));
}

void NodeList::EmitCode(ostream &os)
{
    if (prev) prev->EmitCode(os);
    node->EmitCode(os);
}

int Attr::EmitAttrsAsFormal(ostream &os, int vars)
{
    if (opts & OPT_NO_INIT) return vars;

    if (vars) os << ", ";
    ++ vars;
    os << type << " " << "__" << vars << "__";

    return vars;
}

bool FeatureList::NeedsColon(void)
{
    bool rv = false;
    if (prev) rv = prev->NeedsColon();
    return rv || feat->NeedsColon();
}

void Func::EmitMethod(ostream &os)
{
    os << "    virtual " << spec;

    if (opts & OPT_VIRTUAL) os << " = 0;" << endl;
    else if (opts & OPT_EXTERNAL) os << ";" << endl;
    else if (code) os << " " << code << endl;
    else os << ";" << endl;
}

void FeatureList::EmitMethod(ostream &os)
{
    if (prev) prev->EmitMethod(os);
    feat->EmitMethod(os);
}

void Attr::EmitAttrInline(ostream &os)
{
    // --     virtual <type> Get_<name>(void) const { return <name>; }
    // --     virtual void Set_<name>(type __val__) { <name> = __val__; }
    if (opts & OPT_NO_INLINES) return;

    os << "    virtual " << type << " Get_" << name << "(void) const { return " << name << "; }" << endl;
    os << "    virtual void Set_" << name << "(" << type << " __val__) { " << name << " = __val__; }" << endl;
}

void FeatureList::EmitAttrInline(ostream &os)
{
    if (prev) prev->EmitAttrInline(os);
    feat->EmitAttrInline(os);
}

void Attr::EmitAttrCode(ostream &os)
{
    os << "    " << type << " " << name << ";" << endl;
}

void FeatureList::EmitAttrCode(ostream &os)
{
    if (prev) prev->EmitAttrCode(os);
    feat->EmitAttrCode(os);
}

int Attr::EmitConstruct(ostream &os, int var)
{
    if (prtComma) os << ", ";
    os << name << "(";
    prtComma = true;
    if (opts & OPT_NO_INIT) os << dft;
    else os << "__" << var ++ << "__";
    os << ")";

    return var;
}

int FeatureList::EmitConstruct(ostream &os, int var)
{
    if (prev) var = prev->EmitConstruct(os, var);
    var = feat->EmitConstruct(os, var);
    return var;
}

int FeatureList::EmitAttrsAsFormal(ostream &os, int vars)
{
    if (prev) vars = prev->EmitAttrsAsFormal(os, vars);
    vars = Get_feat()->EmitAttrsAsFormal(os, vars);
    return vars;
}

int Node::EmitAttrsAsFormal(ostream &os)
{
    int rv = 0;

    if (parent) rv = parent->EmitAttrsAsFormal(os);

    if (feats) rv = feats->EmitAttrsAsFormal(os, rv);

    return rv;
}

int FeatureList::GetLocalAttrCount(void)
{
    int rv = 0;
    if (prev) rv = prev->GetLocalAttrCount();
    rv += feat->GetAttrCount();
    return rv;
}

char *Node::GetReturnType(void)
{
    FeatureList *wrk = feats;

    while (wrk) {
        Feature *f = wrk->Get_feat();
        if (f->GetNodeType() == NODE_Factory) {
            Factory *fact = (Factory *)f;
            return fact->Get_type();
        }
        wrk = wrk->Get_prev();
    }

    return name;
}

void Node::EmitCode(ostream &os)
{
    int vars;
    int inh;

    // -- emit the class code for an AST node
    //    -----------------------------------

    // -- class name [: inherited_class] {
    os << "class " << name;
    if (parent) os << " : public " << parent->name;
    os << " {" << endl;

    // -- public:
    // --    static <returntype> *empty(void) { return (<returntype> *)0; }
    os << "public:" << endl;
    os << "    static " << GetReturnType() << " *empty(void) { return (" << GetReturnType() << " *)0; }" << endl;

    if (opt & OPT_ABSTRACT) {
        vars = GetLocalAttrCount();
    } else {
        // --    static <returntype> *factory(<attr-inits>) { return new <name>(<attrs>); }
        os << "    static " << GetReturnType() << " *factory(";
        vars = EmitAttrsAsFormal(os);
        os << ") { return new " << name << "(";

        for (int i = 1; i <= vars; i ++) {
            if (i != 1) os << ", ";
            os << "__" << i << "__";
        }

        os << "); }" << endl;
        os << endl;
    }

    // --    virtual AstNodeType Get_AstNodeType(void) const { return <nodetype>; }
    os << "    virtual AstNodeType Get_AstNodeType(void) const { return NODE_" << name << "; }"<< endl;
    os << endl;

    // -- protected:
    // --    <name>(<attr-inits>) : <inherited-class>(<inits>), <attr>(<init>)... {}
    os << "protected:" << endl;
    os << "    explicit " << name << "(";
    EmitAttrsAsFormal(os);
    os << ") ";
    if (NeedsColon()  || (parent && parent->hasInitParms)  ) os << ": ";
    inh = (parent?parent->GetLocalAttrCount():0);

    if (inh) {
        os << parent->name << "(";
        for (int i = 1; i <= inh; i ++) {
            if (i != 1) os << ", ";
            os << "__" << i << "__";
        }
        os << ")";
    }

    if (inh && vars - inh) os << ", ";
    prtComma = false;
    if (feats && feats->EmitConstruct(os, inh + 1) > inh + 1) hasInitParms = true;
    os << " { }" << endl;
    os << endl;

    // -- protected:
    // --     type name;
    os << "protected:" << endl;
    if (feats) feats->EmitAttrCode(os);
    os << endl;

    // -- public:
    // --     virtual <type> Get_<name>(void) const { return <name>; }
    // --     virtual void Set_<name>(type __val__) { <name> = __val__; }
    os << "public:" << endl;
    if (feats) feats->EmitAttrInline(os);
    os << endl;

    // --     virtual <methodspec> { <name> = __val__; }
    // --     virtual <methodspec> = 0;
    // --     virtual <methodspec>;
    if (feats) feats->EmitMethod(os);

    // -- };
    os << "};" << endl;
    os << endl;
}

void Ast::EmitCode(ostream &os)
{
    if (defines) os << defines << endl;
    os << endl;

    nodes->EmitCode(os);
}

void NodeList::EmitEnum(ostream &os)
{
    if (prev) prev->EmitEnum(os);

    os << "    NODE_" << Get_node()->Get_name() << "," << endl;
}

int main(int argc, char *argv[])
{
    yydebug = 0;

	for (int i = 1; i < argc; i ++) {
		if (openBuffer(argv[i])) {
		    curr_file = argv[i];
		    cout << "Parsing " << argv[i] << "..." << endl;

			if (yyparse()) cerr << "ERROR parsing the ast source" << endl;
		}
	}

	if (parse_error) {
	    return 1;
	}

    cout << "Entering Semantic Phase..." << endl;
    tree->Get_nodes()->BuildParents();
	semant(tree);

    // -- we are emitting a list of classes.  Therefore let's start by opening a file
    //    and preparing to emit code for each class in turn.
    //    ---------------------------------------------------------------------------
    cout << "Emitting AST Code..." << endl;
    filebuf fb;
    fb.open ("ast-nodes.h",ios::out);
    ostream os(&fb);

    os << "#ifndef __AST_NODES_H__" << endl;
    os << "#define __AST_NODES_H__" << endl;
    os << endl;

    // -- Emit the enum types
    os << "typedef enum {" << endl;
    tree->Get_nodes()->EmitEnum(os);
    os << "} AstNodeType;" << endl;
    os << endl;

    // -- Emit the actual code...
    tree->EmitCode(os);
    os << endl;
    os << "#endif" << endl;
    fb.close();

    cout << "Done!" << endl;

	return 0;
}
