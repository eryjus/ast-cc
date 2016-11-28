//===================================================================================================================
// ast-cc.cc -- This file is contains the necessary functions to maintain the structures of the AST for ast-cc.
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
// 2016-03-28    N/A     v0.1    ADCL  second version of the ast language
// 2016-10-18   #305     v0.1    ADCL  Remove extra "()" in an initializer and allow an empty initializer.
//
//===================================================================================================================

#include "lists.hh"
#include "ast-cc.hh"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>


extern void cpp_Emit(void);


//-------------------------------------------------------------------------------------------------------------------
// Count the number of attributes that need initialization parameters
//-------------------------------------------------------------------------------------------------------------------
int Node::GetParmCount(void)
{
    AttrList *al;
    int rv = 0;

    if (parent) rv = parent->GetParmCount();

    for (al = First(attrs); More(al); al = Next(al)) {
        if (al->elem()->Get_Flags() & NOINIT) continue;
        else rv ++;
    }

    return rv;
}


//
// -- Initialize the global variables
//    -------------------------------
IncludeList *includes = NULL;
SymTable *symtab = NULL;
NodeList *nodes = NULL;
char *endingCode = NULL;
std::string outputFile = std::string("ast-nodes.hh");


//-------------------------------------------------------------------------------------------------------------------
// AddTypeSymbol(const std::string &) -- Create a TYPE symbol as long as the name does not exist
//-------------------------------------------------------------------------------------------------------------------
Symbol *AddTypeSymbol(const std::string &n)
{
    if (LookupSymbol(n) == false) {
        Symbol *rv = Symbol::Factory(TYPE, n);
        symtab = Append(symtab, new SymTable(rv, NULL));
        return rv;
    } else return NULL;
}


//-------------------------------------------------------------------------------------------------------------------
// AddNodeSymbol(const std::string &) -- Create a NODE symbol as long as the name does not exist
//-------------------------------------------------------------------------------------------------------------------
Symbol *AddNodeSymbol(const std::string &n)
{
    if (LookupSymbol(n) == false) {
        Symbol *rv = Symbol::Factory(NODE, n);
        symtab = Append(symtab, new SymTable(rv, NULL));
        return rv;
    } else return NULL;
}


//-------------------------------------------------------------------------------------------------------------------
// LookupSymbol(const std::string &) -- look for a symbol by name and return whether it exists
//-------------------------------------------------------------------------------------------------------------------
bool LookupSymbol(const std::string &n)
{
    SymTable *wrk;

    for (wrk = First(symtab); More(wrk); wrk = Next(wrk)) {
        Symbol *tmp = wrk->elem();
        if (tmp->Get_Name() == n) return true;
    }

    return false;
}


//-------------------------------------------------------------------------------------------------------------------
// GetSymbol(const std::string &) -- look for a symbol by name and return its structure
//-------------------------------------------------------------------------------------------------------------------
Symbol *GetSymbol(const std::string &n)
{
    SymTable *wrk;

    for (wrk = First(symtab); More(wrk); wrk = Next(wrk)) {
        Symbol *tmp = wrk->elem();
        if (tmp->Get_Name() == n) return tmp;
    }

    return NULL;
}


//-------------------------------------------------------------------------------------------------------------------
// Semant() -- Perform the semantic checks for the AST
//-------------------------------------------------------------------------------------------------------------------
bool Semant(void)
{
    bool rv = true;         // assume success

    //
    // -- By the time we get to this point, we know a few things.  We know that the type and node names are
    //    unique.  We also know that the types are defined for the attributes, method return types, and
    //    parameter types.  However, there are a lot of things that still need to be checked.
    //
    //    Start simple and check the included files for duplicates (however, not for existance).  Note that
    //    we do not strip out the punctuation, so <cstdio> and "cstdio" will compare as different files.
    //    -------------------------------------------------------------------------------------------------
    for (IncludeList *looper = First(includes); More(looper); looper = Next(looper)) {
        for (IncludeList *chk = Next(looper); More(chk); chk = Next(chk)) {
            if (strcmp(looper->elem(), chk->elem()) == 0) {
                fprintf(stderr, "Error: Include file %s specified more than once\n", looper->elem());
                rv = false;
            }
        }
    }

    //
    // -- Next we will loop through the classes and start checking them.  First we set up the loop on the
    //    Nodes.
    //    -----------------------------------------------------------------------------------------------
    for (NodeList *nl = First(nodes); More(nl); nl = Next(nl)) {
        Node *n = nl->elem();

        //
        // -- The first thing to do is loop through all the attribute names and make sure they are unique
        //    within themselves and the method names.  If they are not unique, we myst issue an error.
        //    -------------------------------------------------------------------------------------------
        for (AttrList *al = First(n->Get_Attrs()); More(al); al = Next(al)) {
            for(AttrList *chk = Next(al); More(chk); chk = Next(chk)) {
                if (al->elem()->Get_Name() == chk->elem()->Get_Name()) {
                    fprintf(stderr, "Error: Attrribute name %s in class %s is duplicated\n",
                            al->elem()->Get_Name().c_str(), n->Get_Name()->Get_Name().c_str());
                    rv = false;
                }
            }

            //
            // -- Loop through all the methods to make sure we have not duplicated a name
            //    -----------------------------------------------------------------------
            for(MethList *chk = First(n->Get_Meths()); More(chk); chk = Next(chk)) {
                if (al->elem()->Get_Name() == chk->elem()->Get_Name()) {
                    fprintf(stderr, "Error: Attrribute name %s in class %s is duplicated by method %s\n",
                            al->elem()->Get_Name().c_str(), n->Get_Name()->Get_Name().c_str(),
                            chk->elem()->Get_Name().c_str());
                    rv = false;
                }
            }

            //
            // -- check to make sure that the attributes are properly specified.
            //    --------------------------------------------------------------
            Attribute *a = al->elem();
            int f = a->Get_Flags();

            //
            // -- if nothing was specified for an attribute, we will assume PROTECTED
            //    -------------------------------------------------------------------
            if (!(f & PUBLIC) && !(f & PROTECTED) && !(f & PRIVATE)) {
                a->Set_Flag(PROTECTED);
            }

            //
            // -- check to make sure more than one access definition was not specified for an attribute
            //    -------------------------------------------------------------------------------------
            if ((f & PUBLIC) && (f & PROTECTED)) {
                fprintf(stderr, "Error: Cannot specify both PUBLIC and PROTECTED on attribute %s in class %s\n",
                        a->Get_Name().c_str(), n->Get_Name()->Get_Name().c_str());
                rv = false;
            }

            if ((f & PUBLIC) && (f & PRIVATE)) {
                fprintf(stderr, "Error: Cannot specify both PUBLIC and PRIVATE on attribute %s in class %s\n",
                        a->Get_Name().c_str(), n->Get_Name()->Get_Name().c_str());
                rv = false;
            }

            if ((f & PROTECTED) && (f & PRIVATE)) {
                fprintf(stderr, "Error: Cannot specify both PROTECTED and PRIVATE on attribute %s in class %s\n",
                        a->Get_Name().c_str(), n->Get_Name()->Get_Name().c_str());
                rv = false;
            }
        }

        //
        // -- At this point, we have taken care of the attribute checking.  Now to move on to the method
        //    checking, which will be quite a bit more complicated.  Each method signature must be unique.
        //    A method signature is its name with the types of the parameters passed in (not the return
        //    type).  Additionally, the parameter names must be unique within each method.  In the event a
        //    parameter name hides an attribute, we will issue a warning.
        //
        //    As we get into this, we know that attribute names and method names do not overlap, as these
        //    were checked with the attribute checks.
        //
        //    We will check method signatures against method signatures in the following manner:
        //    A) we assume that the 2 methods we are coparing are the same
        //    B) we compare the names of the 2 methods, and if different they are not the same
        //    C) we compare the number of parameters between the 2 methods, and if they are different
        //       they are not the same
        //    D) Now, we loop through the parameters and if the types are different for any one, then
        //       they are not the same.
        //    E) finally, if you reach this point, then we can confirm that they are the same
        //    --------------------------------------------------------------------------------------------
        for (MethList *ml = First(n->Get_Meths()); More(ml); ml = Next(ml)) {
            bool same = true;
            Method *meth = ml->elem();

            for(MethList *chk = Next(ml); More(chk); chk = Next(chk)) {
                Method *chkm = chk->elem();

                if (meth->Get_Name() == chkm->Get_Name()) {
                    if (Len(meth->Get_Parms()) == Len(chkm->Get_Parms())) {
                        int i;

                        for (i = 0; i < Len(meth->Get_Parms()); i ++) {
                            Parameter *p = meth->Get_Parm(i);
                            Parameter *c = chkm->Get_Parm(i);

                            if (p->Get_Type() != c->Get_Type()) same = false;
                        }

                        if (same) {
                            fprintf(stderr, "Error: Signature of method %s is duplicated\n",
                                    meth->Get_Name().c_str());
                            rv = false;
                        }
                    } else same = false;
                } else same = false;
            }

            if (meth->Get_Code() != "" && meth->Get_Flags() & EXTERNAL) {
                fprintf(stderr, "Error: EXTERNAL method specified when code is also provided in %s\n",
                        meth->Get_Name().c_str());
                rv = false;
            } else if (meth->Get_Code() == "" && meth->Get_Flags() & !EXTERNAL) {
                fprintf(stderr, "Error: EXTERNAL method not specified when no code is provided in %s\n",
                        meth->Get_Name().c_str());
                meth->Set_Flag(EXTERNAL);
                rv = false;
            }

            if (meth->Get_Code() == "") meth->Set_Flag(EXTERNAL);
        }
    }

    return rv;
}


//-------------------------------------------------------------------------------------------------------------------
// main() -- main entry point
//-------------------------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    extern int yydebug;
    extern int openBuffer(const char *name);
    extern char *curr_file;
    extern int parse_error;
    extern int yyparse(void);

    char *outfile = NULL;

    yydebug = 0;

    //
    // -- Initialize the compiler symbol table and Common node
    //    ----------------------------------------------------
    nodes = Append(nodes, new NodeList(Node::Factory(NULL, AddNodeSymbol(std::string("Common"))), NULL));
    nodes->elem()->Set_Flag(ABSTRACT);
    AddTypeSymbol(std::string("void"));

    //
    // -- parse the files
    //    ---------------
    for (int i = 1; i < argc; i ++) {
        if (strcmp(argv[i], "-o") == 0 && !outfile) {
            outfile = argv[++i];
            outputFile = std::string(outfile);
            continue;
        }

        if (openBuffer(argv[i])) {
            curr_file = argv[i];

            if (yyparse()) std::cerr << "ERROR parsing the ast source" << std::endl;
            break;
        }
    }

    if (parse_error) return 1;
    if (!Semant()) return 1;

    cpp_Emit();

    std::cout << "Done!" << std::endl;

    return 0;
}
