//===================================================================================================================
// emit-cpp.cc -- This file is responsible for emitting the CPP source
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
// 2016-04-10    N/A     v0.1    ADCL  second version of the ast language
// 2016-09-27   #297     v0.1    ADCL  The Empty() static function also has a const qualifier; remove const.
// 2016-09-27   #299     v0.1    ADCL  The constructor parameters are const, but that is not really accurate.
//                                     Removing the const qualifier.
// 2016-09-27   #300     v0.1    ADCL  Cleaning up some spacing.
//
//===================================================================================================================

#include "lists.hh"
#include "ast-cc.hh"
#include "parser.hh"
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitHeader() -- Emit Header data for the target file
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitHeader(std::ofstream &os)
{
    extern char *curr_file;

    os << "//===============================================================================================" << std::endl;
    os << "//" << std::endl;
    os << "// " << outputFile << " -- The defined nodes for the Abstract Syntax Tree" << std::endl;
    os << "//" << std::endl;
    os << "// This file is automatically generated using `ast-cc` against the source file " << curr_file << "." << std::endl;
    os << "//" << std::endl;
    os << "// Do not modify this file directly as your changes will likely be lost." << std::endl;
    os << "//" << std::endl;
    os << "//===============================================================================================" << std::endl;
    os << std::endl << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitForwards() -- Emit the forward definitions for the class names
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitForwards(std::ofstream &os)
{
    os << "//-----------------------------------------------------------------------------------------------" << std::endl;
    os << "// The following are forward declarations for the nodes that are defined in the source file" << std::endl;
    os << "//-----------------------------------------------------------------------------------------------" << std::endl;

    SymTable *syms;

    for (syms = First(symtab); More(syms); syms = Next(syms)) {
        Symbol *sym = syms->elem();

        if (sym->Get_Kind() == NODE) {
            os << "class " << sym->Get_Name() << ";" << std::endl;
        }
    }

    os << std::endl << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitIncludes() -- Emit the include files
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitIncludes(std::ofstream &os)
{
    os << "//-----------------------------------------------------------------------------------------------" << std::endl;
    os << "// These include files are specified in the source file" << std::endl;
    os << "//-----------------------------------------------------------------------------------------------" << std::endl;

    IncludeList *l;

    for (l = First(includes); More(l); l = Next(l)) {
        os << "#include " << l->elem() << std::endl;
    }

    os << std::endl << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitNodeTypes() -- Emit the node types as a enumeration
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitNodeTypes(std::ofstream &os)
{
    os << "//-----------------------------------------------------------------------------------------------" << std::endl;
    os << "// This enumeration is used to identify the types of nodes" << std::endl;
    os << "//-----------------------------------------------------------------------------------------------" << std::endl;

    NodeList *nl;

    os << "typedef enum {" << std::endl;
    for (nl = First(nodes); More(nl); nl = Next(nl)) {
        Node *n = nl->elem();

        if (n->Get_Flags() & ABSTRACT) continue;
        os << "\tNODE_TYPE_" << n->Get_Name()->Get_Name() << "," << std::endl;
    }

    os << "} ASTNodeType;";
    os << std::endl << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitConstructorParms() -- Emit the class Constructor parameter list -- returns whether a parm was printed
//-------------------------------------------------------------------------------------------------------------------
static bool cpp_EmitConstructorParms(std::ofstream &os, Node *node)
{
    if (!node) return false;

    bool parmPrinted = cpp_EmitConstructorParms(os, node->Get_Parent());
    AttrList *al;

    for (al = First(node->Get_Attrs()); More(al); al = Next(al)) {
        Attribute *a = al->elem();

        if (a->Get_Flags() & NOINIT) continue;
        if (parmPrinted) os << "," << std::endl << "\t\t";
        os << a->Get_Type()->Get_Name() << (a->Get_Type()->Get_Kind()==NODE?" *":" ")
                << "__init__" << a->Get_Name();
        parmPrinted = true;
    }

    return parmPrinted;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitConstructorArgs() -- Emit the class Constructor argument list -- returns whether an arg was printed
//-------------------------------------------------------------------------------------------------------------------
static bool cpp_EmitConstructorArgs(std::ofstream &os, Node *node)
{
    if (!node) return false;

    bool parmPrinted = cpp_EmitConstructorArgs(os, node->Get_Parent());
    AttrList *al;

    for (al = First(node->Get_Attrs()); More(al); al = Next(al)) {
        Attribute *a = al->elem();

        if (a->Get_Flags() & NOINIT) continue;
        if (parmPrinted) os << "," << std::endl << "\t\t";
        os << "__init__" << a->Get_Name();
        parmPrinted = true;
    }

    return parmPrinted;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitConstructorBase() -- Emit the class constructor base class initializer
//-------------------------------------------------------------------------------------------------------------------
static bool cpp_EmitConstructorBase(std::ofstream &os, Node *node)
{
    if (!node) return false;

    bool parmPrinted = cpp_EmitConstructorBase(os, node->Get_Parent());
    AttrList *al;

    for (al = First(node->Get_Attrs()); More(al); al = Next(al)) {
        Attribute *a = al->elem();

        if (a->Get_Flags() & NOINIT) continue;
        if (parmPrinted) os << "," << std::endl << "\t\t";
        os << "__init__" << a->Get_Name();
        parmPrinted = true;
    }

    return parmPrinted;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitConstructor() -- Emit the class Constructor
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitConstructor(std::ofstream &os, Node *node)
{
    bool needComma = false;

    os << "\t//" << std::endl;
    os << "\t// -- The " << node->Get_Name()->Get_Name() << " constructor" << std::endl;
    os << "\t//----------------------------------------------------------------------------------" << std::endl;

    //
    // -- Constructors are always going to be protected in access so that inherited classes can be initialized
    //    ----------------------------------------------------------------------------------------------------
    os << "protected:" << std::endl;

    //
    // -- Start the constructor definition up to the parameters
    //    -----------------------------------------------------
    os << "\texplicit " << node->Get_Name()->Get_Name() << "(";
    if (!cpp_EmitConstructorParms(os, node)) os << "void";
    os << ")";

    //
    // -- Now, the number of initializers is dependent on the attribute count.  all will be initialized,
    //    whether statically or from a parameter.
    //    ----------------------------------------------------------------------------------------------
    if (node->GetAttrCount() == 0) goto exit;

    //
    // -- call the base class initializer
    //    -------------------------------
    os << " :" << std::endl << "\t\t";
    if (node->Get_Parent()) {
        os << node->Get_Parent()->Get_Name()->Get_Name() << "(";
        cpp_EmitConstructorBase(os, node->Get_Parent());
        os << ")";
        needComma = true;
    }

    //
    // -- now, run through the list of attributes for this class and perform the initialization
    //    -------------------------------------------------------------------------------------
    AttrList *al;

    for (al = First(node->Get_Attrs()); More(al); al = Next(al)) {
        Attribute *a = al->elem();

        if (needComma) os << "," << std::endl << "\t\t";
        os << a->Get_Name() << "(";
        if (a->Get_Flags() & NOINIT) {
            os << a->Get_Code();
        } else {
            os << "__init__" << a->Get_Name();
        }
        os << ")";
        needComma = true;
    }

exit:
    os << " { }" << std::endl;
    os << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitDestructor() -- Emit the class Destructor
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitDestructor(std::ofstream &os, Node *node)
{
    os << "\t//" << std::endl;
    os << "\t// -- The " << node->Get_Name()->Get_Name() << " destructor" << std::endl;
    os << "\t//--------------------------------------------------------------------------------" << std::endl;

    os << "public:" << std::endl;
    os << "\tvirtual ~" << node->Get_Name()->Get_Name() << "(void) { }" << std::endl << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitAttributes() -- Emit the class Attributes
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitAttributes(std::ofstream &os, AttrList *attrs)
{
    AttrList *al;

    for (al = First(attrs); More(al); al = Next(al)) {
        Attribute *a = al->elem();

        //
        // -- first emit the attribute
        //    ------------------------
        os << "\t//" << std::endl;
        os << "\t// -- The " << a->Get_Name() << " attribute" << std::endl;
        os << "\t//---------------------------------------------------------------------------------" << std::endl;

        if (a->Get_Flags() & PUBLIC) os << "public:" << std::endl;
        else if (a->Get_Flags() & PROTECTED) os << "protected:" << std::endl;
        else os << "private:" << std::endl;

        os << "\t" << (a->Get_Flags()&STATIC?"static ":"") << a->Get_Type()->Get_Name() << " "
                << (a->Get_Type()->Get_Kind()==NODE?"*":"") << a->Get_Name() << ";" << std::endl << std::endl;

        //
        // -- if not disabled, emit the access methods
        //    ----------------------------------------
        if (a->Get_Flags() & NOINLINES) continue;

        os << "public:" << std::endl;
        os << "\t" << a->Get_Type()->Get_Name() << " " << (a->Get_Type()->Get_Kind()==NODE?"*":"")
                << "Get_" << a->Get_Name() << "(void) { return " << a->Get_Name() << "; }" << std::endl;
        os << "\tvoid Set_"<< a->Get_Name() << "(" << a->Get_Type()->Get_Name() << " "
                << (a->Get_Type()->Get_Kind()==NODE?"*":"") << "val) { "
                << a->Get_Name() << " = val; }" << std::endl << std::endl;
    }
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitMethods() -- Emit the class Methods
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitMethods(std::ofstream &os, MethList *meths)
{
    MethList *ml;

    for (ml = First(meths); More(ml); ml = Next(ml)) {
        Method *m = ml->elem();

        //
        // -- first emit the method
        //    ---------------------
        os << "\t//" << std::endl;
        os << "\t// -- The " << m->Get_Name() << " method" << std::endl;
        os << "\t//---------------------------------------------------------------------------------" << std::endl;

        if (m->Get_Flags() & PRIVATE) os << "private:" << std::endl;
        else if (m->Get_Flags() & PROTECTED) os << "protected:" << std::endl;
        else os << "public:" << std::endl;

        os << (m->Get_Flags()&STATIC?"\tstatic ":"\tvirtual ") << m->Get_Type()->Get_Name() << " "
                << (m->Get_Type()->Get_Kind()==NODE?"*":"") << m->Get_Name() << "(";
        if (Len(m->Get_Parms()) == 0) os << "void";
        else {
            ParmList *pl;
            bool needComma = false;

            for (pl = First(m->Get_Parms()); More(pl); pl = Next(pl)) {
                Parameter *p = pl->elem();

                if (needComma) os << ", ";
                os << p->Get_Type()->Get_Name() << " " << (p->Get_Type()->Get_Kind()==NODE?"*":"")
                        << p->Get_Name();
                needComma = true;
            }
        }
        os << ")";

        if (m->Get_Flags() & ABSTRACT) os << " = 0;" << std::endl << std::endl;
        else if (m->Get_Flags() & EXTERNAL) os << ";" << std::endl << std::endl;
        else os << " " << m->Get_Code() << std::endl << std::endl;
    }
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitEmptyFunc() -- Emit the static class empty function
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitEmptyFunc(std::ofstream &os, Node *node)
{
    os << "\t//" << std::endl;
    os << "\t// -- The " << node->Get_Name()->Get_Name() << " static empty value function" << std::endl;
    os << "\t//---------------------------------------------------------------------------------" << std::endl;

    os << "public:" << std::endl;
    os << "\tstatic " << node->Get_Name()->Get_Name() << " *Empty(void) { return NULL; }" << std::endl << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitFactoryFunc() -- Emit the static class Factory function
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitFactoryFunc(std::ofstream &os, Node *node)
{
    if (node->Get_Flags() & ABSTRACT) return;

    os << "\t//" << std::endl;
    os << "\t// -- The " << node->Get_Name()->Get_Name() << " Factory function" << std::endl;
    os << "\t//----------------------------------------------------------------------------------" << std::endl;

    //
    // -- Constructors are always going to be protected in access so that inherited classes can be initialized
    //    ----------------------------------------------------------------------------------------------------
    os << "public:" << std::endl;

    //
    // -- Start the constructor definition up to the parameters
    //    -----------------------------------------------------
    os << "\tstatic " << node->Get_Name()->Get_Name() << " *Factory(";
    if (!cpp_EmitConstructorParms(os, node)) os << "void";
    os << ")";

    //
    // -- create a new object
    //    -------------------
    os << " { return new " << node->Get_Name()->Get_Name() << "(";
    cpp_EmitConstructorArgs(os, node);
    os << "); }" << std::endl;
    os << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitGetType() -- Emit the static get node type method
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitGetType(std::ofstream &os, Node *node)
{
    os << "\t//" << std::endl;
    os << "\t// -- The " << node->Get_Name()->Get_Name() << " get node type function" << std::endl;
    os << "\t//---------------------------------------------------------------------------------" << std::endl;

    os << "public:" << std::endl;
    os << "\tvirtual ASTNodeType _GetType(void) const ";

    if (node->Get_Flags() & ABSTRACT) os << " = 0;" << std::endl << std::endl;
    else os << "{ return NODE_TYPE_" << node->Get_Name()->Get_Name() << "; }" << std::endl << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitGetTypeString() -- Emit the static get node type as a string method
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitGetTypeString(std::ofstream &os, Node *node)
{
    os << "\t//" << std::endl;
    os << "\t// -- The " << node->Get_Name()->Get_Name() << " get node type as string function" << std::endl;
    os << "\t//---------------------------------------------------------------------------------" << std::endl;

    os << "public:" << std::endl;
    os << "\tvirtual const char *_GetTypeString(void) const ";

    if (node->Get_Flags() & ABSTRACT) os << " = 0;" << std::endl << std::endl;
    else os << "{ return \"" << node->Get_Name()->Get_Name() << "\"; }" << std::endl << std::endl;
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitNodeContents() -- Emit the contents of a node definition
//
// A node will be emitted in a consistent order:
// A) Constructor
// B) Desctructor
// C) Attributes
// D) Methods
// E) Static Empty() function
// F) Static Factory() function
// G) Static _GetType() function
// H) Static _GetTypeString() function
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitNodeContents(std::ofstream &os, Node *node)
{
    cpp_EmitConstructor(os, node);
    cpp_EmitDestructor(os, node);
    cpp_EmitAttributes(os, node->Get_Attrs());
    cpp_EmitMethods(os, node->Get_Meths());
    cpp_EmitEmptyFunc(os, node);
    cpp_EmitFactoryFunc(os, node);
    cpp_EmitGetType(os, node);
    cpp_EmitGetTypeString(os, node);
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_EmitNodes() -- Now for the bulk of the code emitting...  the classes defined
//-------------------------------------------------------------------------------------------------------------------
static void cpp_EmitNodes(std::ofstream &os)
{
    os << "//-----------------------------------------------------------------------------------------------" << std::endl;
    os << "// now to emit each of the nodes in turn" << std::endl;
    os << "//-----------------------------------------------------------------------------------------------" << std::endl;
    os << std::endl << std::endl;

    NodeList *nl;

    for (nl = First(nodes); More(nl); nl = Next(nl)) {
        Node *n = nl->elem();

        os << "//-----------------------------------------------------------------------------------------------" << std::endl;
        os << "// The " << n->Get_Name()->Get_Name() << " node" << std::endl;
        os << "//-----------------------------------------------------------------------------------------------" << std::endl;

        os << "class " << n->Get_Name()->Get_Name();
        if (n->Get_Parent()) {
            os << " : public " << n->Get_Parent()->Get_Name()->Get_Name();
        }
        os << " {" << std::endl;

        cpp_EmitNodeContents(os, n);

        os << "};" << std::endl;
        os << std::endl << std::endl;
    }
}


//-------------------------------------------------------------------------------------------------------------------
// cpp_Emit() -- Emit the CPP code for the AST tree nodes
//
// This will happen in several stages.  The first stage is to create forward definitions for every node in the
// AST source.  For C++, this is in the form "class <node>;".
//
// The second stage is to include all the include files that were defined.  This is in the form "#include <filename>"
// or "#include \"filename\"".  The \" or < > delimeters are included in the filename from the source.
//
// The third stage is to emit the class definitions in their entirety.  This will include attributes, inline methods,
// external method definitions, constructors, and descructors.
//
// The final stage is to emit the ending code that was established in the AST source.
//-------------------------------------------------------------------------------------------------------------------
void cpp_Emit(void)
{
    static std::ofstream os;
    os.open (outputFile.c_str());

    cpp_EmitHeader(os);
    cpp_EmitForwards(os);
    cpp_EmitNodeTypes(os);
    cpp_EmitIncludes(os);
    cpp_EmitNodes(os);

    os << endingCode;

    os.close();
}