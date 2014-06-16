#ifndef __AST_NODES_H__
#define __AST_NODES_H__

typedef enum {
    NODE_Expression,
    NODE_ID,
} AstNodeType;


    class Expression;
    typedef Expression *Expr;


class Expression {
public:
    static Expression *empty(void) { return (Expression *)0; }
    virtual AstNodeType Get_AstNodeType(void) const { return NODE_Expression; }

protected:
    Expression(int __1__, string __2__, char * __3__) : line_no(__1__), file(__2__), name(__3__), type(0) { }

protected:
    int line_no;
    string file;
    char * name;
    int type;

public:
    virtual int Get_line_no(void) const { return line_no; }
    virtual void Set_line_no(int __val__) { line_no = __val__; }
    virtual string Get_file(void) const { return file; }
    virtual void Set_file(string __val__) { file = __val__; }
    virtual char * Get_name(void) const { return name; }
    virtual void Set_name(char * __val__) { name = __val__; }
    virtual int Get_type(void) const { return type; }
    virtual void Set_type(int __val__) { type = __val__; }

    virtual void semant(void) = 0;
    virtual void print(ostream &s) { s << file << ": " << line_no; }
};

class ID : public Expression {
public:
    static Expression *empty(void) { return (Expression *)0; }
    static Expression *factory(int __1__, string __2__, char * __3__, string __4__) { return new ID(__1__, __2__, __3__, __4__); }

    virtual AstNodeType Get_AstNodeType(void) const { return NODE_ID; }

protected:
    ID(int __1__, string __2__, char * __3__, string __4__) : Expression(__1__, __2__, __3__), id_string(__4__) { }

protected:
    string id_string;

public:

    virtual void semant(void);
    virtual Symbol *GetSymbol(void) { return SymTable->Get(id_string); }
};


#endif
