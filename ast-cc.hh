//===================================================================================================================
// ast-cc.hh -- This file is contains the necessary constructs to maintain the structures of the AST for ast-cc.
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
// 2016-03-27    N/A     v0.1    ADCL  second version of the ast language
//
//===================================================================================================================


#include "lists.hh"
#include <string>

extern char *endingCode;

//
// == The following definitions are used as flags for structures
//    ==========================================================

//
// -- the flags for nodes, attributes, and methods
//    --------------------------------------------
typedef enum {
    NONE        = 0x0000,
    ABSTRACT    = 0x0001,
    PRIVATE     = 0x0002,
    PUBLIC      = 0x0004,
    PROTECTED   = 0x0008,
    STATIC      = 0x0010,
    NOINLINES   = 0x0020,
    INLINE      = 0x0040,
    EXTERNAL    = 0x0080,
    NOINIT      = 0x0100,
} Flags;


//
// -- kinds of symbols we can define
//    ------------------------------
typedef enum {
    UNKNOWN,
    TYPE,
    NODE,
} Kind;


//-------------------------------------------------------------------------------------------------------------------


//
// == This is the list of files that need to be included in the target source.  They are trivial and have no
//    validation rules.  The compilation of the target source file will take care of that.
//    ======================================================================================================

//
// -- Files that need to be included into the target source file.
//    -----------------------------------------------------------
typedef List<char> IncludeList;
extern IncludeList *includes;

//-------------------------------------------------------------------------------------------------------------------


//
// == The following structures are used to keep track of a trivial symbol table.  Since there is only one
//    scope (global), there is no need for scope management.  Its implementation is a simple linked list
//    (no hash tables or other complex structures).  For our language, a symbol is either a TYPE or a NODE,
//    and that is know at time of declaraion which it is.
//
//    There are jsut a few symbol table management functions:
//    AddSymbol() to add a symbol to the table, ensuring no duplication (a TYPE and a NODE flavor)
//    LookupSymbol() will find a symbol given a name and report whether it exists
//    GetSymbol() will return the requested symbol, or NULL if it does not
//    =====================================================================================================

//
// -- This is a symbol definition
//    ---------------------------
class Symbol {
private:
    std::string name;

public:
    std::string &Get_Name(void) { return name; }

private:
    Kind kind;

public:
    Kind Get_Kind(void) const { return kind; }

protected:
    Symbol(Kind k, const std::string &n) : name(n), kind(k) {}

public:
    static Symbol *Factory(Kind k, const std::string &n) { return new Symbol(k, n); }

public:
    static Symbol *NewType(const std::string &n) { return Factory(TYPE, n); }

public:
    static Symbol *NewNode(const std::string &n) { return Factory(NODE, n); }

public:
    virtual ~Symbol(void) {}
};


//
// -- This is the symbol table
//    ------------------------
typedef List<Symbol> SymTable;
extern SymTable *symtab;


//
// -- Function prototypes
//    -------------------
Symbol *AddTypeSymbol(const std::string &n);
Symbol *AddNodeSymbol(const std::string &n);
bool LookupSymbol(const std::string &n);
Symbol *GetSymbol(const std::string &n);


//-------------------------------------------------------------------------------------------------------------------


//
// == The following structure is used to keep track of an attribute element of a node.
//    ================================================================================

//
// -- This is an attribute for a node.
//    --------------------------------
class Attribute {
private:
    int flags;

public:
    void Set_Flag(Flags f) { flags |= f; }
    void Unset_Flag(Flags f) { flags = flags & (~f); }
    int Get_Flags(void) const { return flags; }
    void Clear_Flags(void) { flags = 0; }
    bool Is_Flag_Set(Flags f) { return ((flags & f) != 0); }

private:
    Symbol *type;

public:
    Symbol *Get_Type(void) { return type; }

private:
    std::string name;

public:
    std::string &Get_Name(void) { return name; }

protected:
    Attribute(const std::string &n, Symbol *t) : flags(NONE), type(t), name(n) {}

public:
    static Attribute *Factory(const std::string &n, Symbol *t) { return new Attribute(n, t); }

public:
    virtual ~Attribute(void) {}

private:
    std::string code;

public:
    void Set_Code(const std::string &c) { code = c; }
    std::string &Get_Code(void) { return code; }
};


//
// -- A list of attributes
//    --------------------
typedef List<Attribute> AttrList;


//-------------------------------------------------------------------------------------------------------------------


//
// == Parameter lists are required for methods.  A parameter is simply a name and a type.  The name cannot repeat
//    in any function and cannot be the same as any other known name for our rules (it may be allowed for some
//    target languages, but we will forbid it).  However, the parameter names will not be added to the symbol
//    table since there is no need to make sure a that parameter names to not clash between methods.
//    ===========================================================================================================

//
// -- This structure will hold the necessary information about a parameter -- its name and type.
//    ------------------------------------------------------------------------------------------
class Parameter {
private:
    Symbol *type;

public:
    Symbol *Get_Type(void) { return type; }

private:
    std::string name;

public:
    std::string &Get_Name(void) { return name; }

protected:
    Parameter(const std::string &n, Symbol *t) : type(t), name(n) {}

public:
    static Parameter *Factory(const std::string &n, Symbol *t) { return new Parameter(n, t); }

public:
    virtual ~Parameter(void) {}
};


//
// -- A list of parameters
//    --------------------
typedef List<Parameter> ParmList;


//-------------------------------------------------------------------------------------------------------------------


//
// == The following structure is used to keep track of a method eleement of a node.
//    =============================================================================

//
// -- This is a method for a node.
//    ----------------------------
class Method {
private:
    int flags;

public:
    void Set_Flag(Flags f) { flags |= f; }
    void Unset_Flag(Flags f) { flags = flags & (~f); }
    int Get_Flags(void) const { return flags; }
    void Clear_Flags(void) { flags = 0; }
    bool Is_Flag_Set(Flags f) { return ((flags & f) != 0); }

private:
    Symbol *type;

public:
    Symbol *Get_Type(void) { return type; }

private:
    std::string name;

public:
    std::string &Get_Name(void) { return name; }

private:
    ParmList *parms;

public:
    void Set_ParmList(ParmList *l) { parms = l; }
    Parameter *Get_Parm(int n) { ParmList *t = Nth(parms, n); return (t?t->elem():NULL); }
    ParmList *Get_Parms(void) { return parms; }

protected:
    Method(const std::string &n, Symbol *t) : flags(NONE), type(t), name(n) {}

public:
    static Method *Factory(const std::string &n, Symbol *t) { return new Method(n, t); }

public:
    virtual ~Method(void) {}

private:
    std::string code;

public:
    void Set_Code(const std::string &c) { code = c; }
    std::string &Get_Code(void) { return code; }
};


//
// -- A list of methods
//    -----------------
typedef List<Method> MethList;


//-------------------------------------------------------------------------------------------------------------------


//
// == The following structure is used to keep a node itself
//    =====================================================


class Node {
private:
    int flags;

public:
    void Set_Flag(Flags f) { flags |= f; }
    void Unset_Flag(Flags f) { flags = flags & (~f); }
    int Get_Flags(void) const { return flags; }
    void Clear_Flags(void) { flags = 0; }
    bool Is_Flag_Set(Flags f) { return ((flags & f) != 0); }

private:
    Node *parent;

public:
    void Set_Parent(Node *p) { parent = p; }
    Node *Get_Parent(void) { return parent; }

private:
    Symbol *name;

public:
    Symbol *Get_Name(void) { return name; }

private:
    MethList *methods;

public:
    void Add_Method(Method *m) { methods = Append(methods, new MethList(m, NULL)); }
    Method *Get_Method(int n) { MethList *t = Nth(methods, n); return (t?t->elem():NULL); }
    MethList *Get_Meths(void) { return methods; }

private:
    AttrList *attrs;

public:
    void Add_Attribute(Attribute *a) { attrs = Append(attrs, new AttrList(a, NULL)); }
    Attribute *Get_Attribute(int n) { AttrList *t = Nth(attrs, n); return (t?t->elem():NULL); }
    AttrList *Get_Attrs(void) { return attrs; }

protected:
    Node(Node *p, Symbol *n) : flags(NONE), parent(p), name(n) {}

public:
    static Node *Factory(Node *p, Symbol *n) { return new Node(p, n); }

public:
    virtual ~Node(void) {}

public:
    virtual int GetParmCount(void);

public:
    virtual int GetAttrCount(void) { return (parent?parent->GetParmCount():0) + Len(attrs); }
};

//
// -- The list of nodes
//    -----------------
typedef List<Node> NodeList;
extern NodeList *nodes;


//-------------------------------------------------------------------------------------------------------------------

extern std::string outputFile;
