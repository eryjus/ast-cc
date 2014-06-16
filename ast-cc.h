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


#ifndef __AST_CC_H__
#define __AST_CC_H__

#include "lists.h"

#include <iostream>
using namespace std;

typedef enum {
        OPT_NONE = 0,
        OPT_NO_INIT = (1 << 0),
        OPT_NO_INLINES = (1 << 1),
        OPT_VIRTUAL = (1 << 4),
        OPT_EXTERNAL = (1 << 5),
        OPT_ABSTRACT = (1 << 7),
    } Opts;

typedef enum {
        NODE_Feature,
        NODE_Attr,
        NODE_Factory,
        NODE_Func,
        NODE_FeatureList,
        NODE_Node,
        NODE_NodeList,
    } NodeType;

class Feature {
public:
    static Feature *empty() { return (Feature *)0; }

public:
    virtual bool IsFactory(void) = 0;
    virtual NodeType GetNodeType(void) { return NODE_Feature; }

protected:
    Feature() {};

public:
    virtual int EmitAttrsAsFormal(ostream &os, int vars) = 0;
    virtual int GetAttrCount(void) = 0;
    virtual int EmitConstruct(ostream &os, int var) = 0;
    virtual void EmitAttrCode(ostream &os) = 0;
    virtual void EmitAttrInline(ostream &os) = 0;
    virtual void EmitMethod(ostream &os) = 0;
    virtual bool NeedsColon(void) = 0;
};

class Attr : public Feature {
protected:
    char *type;

public:
    virtual char *Get_type(void) const { return type; }
    virtual void Set_type(char *v) { type = v; }

protected:
    char *name;

public:
    virtual char *Get_name(void) const { return name; }
    virtual void Set_name(char *v) { name = v; }

protected:
    Opts opts;

public:
    virtual Opts Get_opts(void) const { return opts; }
    virtual void Set_opts(Opts v) { opts = v; }

protected:
    char *dft;

public:
    virtual char *Get_dft(void) const { return dft; }
    virtual void Set_dft(char *v) { dft = v; }

public:
    virtual bool IsFactory(void) { return false; }
    virtual NodeType GetNodeType(void) { return NODE_Attr; }
    virtual int EmitAttrsAsFormal(ostream &os, int vars);
    virtual int GetAttrCount(void) { return (opts&OPT_NO_INIT?0:1); }
    virtual int EmitConstruct(ostream &os, int var);
    virtual void EmitAttrCode(ostream &os);
    virtual void EmitAttrInline(ostream &os);
    virtual void EmitMethod(ostream &os) { };
    virtual bool NeedsColon(void) { return true; }

public:
    static Feature *factory(char *v1, char *v2, Opts v3, char *v4) { return new Attr(v1, v2, v3, v4); }
    static Feature *empty() { return (Feature *)0; }

protected:
    Attr(char *v1, char *v2, Opts v3, char *v4) : type(v1), name(v2), opts(v3), dft(v4) {}
};

class Factory : public Feature {
protected:
    char *type;

public:
    virtual char *Get_type(void) const { return type; }
    virtual void Set_type(char *v) { type = v; }

public:
    virtual bool IsFactory(void) { return true; }
    virtual NodeType GetNodeType(void) { return NODE_Factory; }

public:
    static Feature *factory(char *v1) { return new Factory(v1); }
    static Feature *empty() { return (Feature *)0; }

protected:
    Factory(char *v1) : type(v1) {}

public:
    virtual int EmitAttrsAsFormal(ostream &os, int vars) { return vars; }
    virtual int GetAttrCount(void) { return 0; }
    virtual int EmitConstruct(ostream &os, int var) { return var; }
    virtual void EmitAttrCode(ostream &os) { }
    virtual void EmitAttrInline(ostream &os) { }
    virtual void EmitMethod(ostream &os) { };
    virtual bool NeedsColon(void) { return false; }
};

class Func : public Feature {
protected:
    char *spec;

public:
    virtual char *Get_spec(void) const { return spec; }
    virtual void Set_spec(char *v) { spec = v; }

protected:
    char *code;

public:
    virtual char *Get_code(void) const { return code; }
    virtual void Set_code(char *v) { code = v; }

protected:
    Opts opts;

public:
    virtual Opts Get_opts(void) const { return opts; }
    virtual void Set_opts(Opts v) { opts = v; }

public:
    virtual bool IsFactory(void) { return false; }
    virtual NodeType GetNodeType(void) { return NODE_Func; }

public:
    static Feature *factory(char *v1, char *v2, Opts v3) { return new Func(v1, v2, v3); }
    static Feature *empty() { return (Feature *)0; }

protected:
    Func(char *v1, char *v2, Opts v3) : spec(v1), code(v2), opts(v3) {}

public:
    virtual int EmitAttrsAsFormal(ostream &os, int vars) { return vars; }
    virtual int GetAttrCount(void) { return 0; }
    virtual int EmitConstruct(ostream &os, int var) { return var; }
    virtual void EmitAttrCode(ostream &os) { }
    virtual void EmitAttrInline(ostream &os) { }
    virtual void EmitMethod(ostream &os);
    virtual bool NeedsColon(void) { return false; }
};

class FeatureList {
protected:
    FeatureList *prev;

public:
    virtual FeatureList *Get_prev(void) const { return prev; }
    virtual void Set_prev(FeatureList *v) { prev = v; }

protected:
    Feature *feat;

public:
    virtual Feature *Get_feat(void) const { return feat; }
    virtual void Set_feat(Feature *v) { feat = v; }

public:
    virtual NodeType GetNodeType(void) { return NODE_FeatureList; }

public:
    static FeatureList *factory(FeatureList *v1, Feature *v2) { return new FeatureList(v1, v2); }
    static FeatureList *empty() { return (FeatureList *)0; }

protected:
    FeatureList(FeatureList *v1, Feature *v2) : prev(v1), feat(v2) {}

public:
    virtual int EmitAttrsAsFormal(ostream &os, int vars);
    virtual int GetLocalAttrCount(void);
    virtual int EmitConstruct(ostream &os, int var);
    virtual void EmitAttrCode(ostream &os);
    virtual void EmitAttrInline(ostream &os);
    virtual void EmitMethod(ostream &os);
    virtual bool NeedsColon(void);
};

class Node {
protected:
    Node *parent;

public:
    virtual Node *Get_parent(void) const { return parent; }
    virtual void Set_parent(Node *v) { parent = v; }

protected:
    char *name;

public:
    virtual char *Get_name(void) const { return name; }
    virtual void Set_name(char *v) { name = v; }

protected:
    char *inherits;

public:
    virtual char *Get_inherits(void) const { return inherits; }
    virtual void Set_inherits(char *v) { inherits = v; }

protected:
    Opts opt;

public:
    virtual Opts Get_opt(void) const { return opt; }
    virtual void Set_opt(Opts v) { opt = v; }

protected:
    FeatureList *feats;

public:
    virtual FeatureList *Get_feats(void) const { return feats; }
    virtual void Set_feats(FeatureList *v) {feats = v; }

protected:
    char *factType;

public:
    virtual char *Get_factType(void) const { return factType; }
    virtual void Set_factType(char *v) {factType = v; }

public:
    virtual NodeType GetNodeType(void) { return NODE_Node; }

public:
    static Node *factory(char *v1, char *v2, Opts v3, FeatureList *v4) { return new Node(v1, v2, v3, v4); }
    static Node *empty() { return (Node *)0; }

protected:
    Node(char *v1, char *v2, Opts v3, FeatureList *v4) : parent(0), name(v1), inherits(v2), opt(v3), feats(v4) {}

public:
    virtual bool IsAbstract(void) { return (opt & OPT_ABSTRACT) != 0; }
    virtual void EmitCode(ostream &os);
    virtual char *GetReturnType(void);
    virtual int EmitAttrsAsFormal(ostream &os);
    virtual int GetLocalAttrCount(void) { return (feats?feats->GetLocalAttrCount():0); }
    virtual bool NeedsColon(void) { return (feats?feats->NeedsColon():false); }
};

class NodeList {
protected:
    NodeList *prev;

public:
    virtual NodeList *Get_prev(void) const { return prev; }
    virtual void Set_prev(NodeList *v) { prev = v; }

protected:
    Node *node;

public:
    virtual Node *Get_node(void) const { return node; }
    virtual void Set_node(Node *v) { node = v; }

public:
        virtual NodeType GetNodeType(void) { return NODE_NodeList; }

public:
    static NodeList *factory(NodeList *v1, Node *v2) { return new NodeList(v1, v2); }
    static NodeList *empty() { return (NodeList *)0; }

protected:
    NodeList(NodeList *v1, Node *v2) : prev(v1), node(v2) {}

public:
    virtual Node *FindNodeByName(char *n);
    virtual void BuildParents(void);

public:
    virtual void EmitEnum(ostream &os);
    virtual void EmitCode(ostream &os);
};

class Ast {
protected:
    char *defines;

public:
    virtual char *Get_defines(void) const { return defines; }
    virtual void Set_defines(char *__val__) { defines = __val__; }

protected:
    NodeList *nodes;

public:
    virtual NodeList *Get_nodes(void) const { return nodes; }
    virtual void Set_nodes(NodeList *__val__) { nodes = __val__; }

public:
    static Ast *factory(char *v1, NodeList *v2) { return new Ast(v1, v2); }
    static Ast *empty(void) { return (Ast *)0; }

protected:
    Ast(char *v1, NodeList *v2) : defines(v1), nodes(v2) { }

public:
    void EmitCode(ostream &os);
};

extern Ast *tree;
extern char *dft_val;

#endif
