#ifndef __AST_HH__
#define __AST_HH__

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <cassert>
#include <algorithm>

#include "symbtab.hh"
#include "gencode.hh"

using namespace std;

enum typeExp {
    e_statement_astnode,
    e_exp_astnode,
    e_empty_astnode,
    e_seq_astnode,
    e_assignS_astnode,
    e_return_astnode,
    e_if_astnode,
    e_while_astnode,
    e_for_astnode,
    e_proccall_astnode,
    e_ref_astnode,
    e_identifier_astnode,
    e_arrayref_astnode,
    e_member_astnode,
    e_arrow_astnode,
    e_op_binary_astnode,
    e_op_unary_astnode,
    e_assignE_astnode,
    e_funcall_astnode,
    e_intconst_astnode,
    e_floatconst_astnode,
    e_stringconst_astnode
};

extern vector<string> all_reg;
extern vector<string> rstack;
extern const int TOTAL_REG;
extern Gencode gencode;
extern SymbTab gst;

class identifier_astnode;
class exp_astnode;

class abstract_astnode
{
public:
    virtual void print(int blanks) = 0;
    enum typeExp astnode_type;
    int su_label;
    bool is_zero;
protected:
    abstract_astnode()
    {
        is_zero = false;
        su_label = 1;
    }
};


class statement_astnode : public abstract_astnode
{
public:
    statement_astnode(){
        this->astnode_type=e_statement_astnode;
    };
    void print(int b){;}
    virtual void gencode_node(SymbTab* st) {cout<<"not overriden"<<endl;};
};


class exp_astnode : public abstract_astnode
{
public:
    string type;
    bool is_lval;
    exp_astnode(){
        this->astnode_type=e_exp_astnode;
        this->is_lval = false;
    };
    void print(int b){;}
    virtual void gencode_node(SymbTab* st){cout<<"not overriden"<<endl;};

};


class empty_astnode : public statement_astnode
{
public:
    empty_astnode(){
        this->astnode_type = e_empty_astnode;
    };
    void print(int b){
        cout<<"\"empty\" "<<endl;
    }
    void gencode_node(SymbTab* st) override;
};

class seq_astnode : public  statement_astnode
{
public:
    seq_astnode(){
        this->astnode_type=e_seq_astnode;
    };
    vector<statement_astnode*> vec_statement_astnode;
    void print(int b){
        cout<<"\"seq\": ["<<endl;
        for(unsigned int i=0;i<vec_statement_astnode.size();i++){
            // std::cout<<vec_statement_astnode[i]->astnode_type<<std::endl;
            if(vec_statement_astnode[i]->astnode_type == e_empty_astnode)
            {
                vec_statement_astnode[i]->print(b);
                if(i == vec_statement_astnode.size()-1)
                {
                    cout<<"\n";
                }
                else {cout<<",";}  

                continue;
            }
            cout<<"{";
            vec_statement_astnode[i]->print(b);
            if(i == vec_statement_astnode.size()-1)
            {
                cout<<"}\n";
            }
            
            else {cout<<"}\n,\n";}
        }
        cout<<"]"<<endl;
    }

    void gencode_node(SymbTab* st) override;

};



class return_astnode : public  statement_astnode
{
public:
    return_astnode(){
        this->astnode_type=e_return_astnode;
    };
    exp_astnode* return_val;
    void print(int b)
    {
        cout<<"\"return\": "<<endl;
        if(return_val->astnode_type==e_empty_astnode)
            return_val->print(b);
        else{
            cout<<"{";
            return_val->print(b);
            cout<<"}"<<endl;
        }
    }
    void gencode_node(SymbTab* st) override;


};

class if_astnode : public statement_astnode
{
public:
    if_astnode(){
        this->astnode_type=e_if_astnode;
    };
    exp_astnode* condition;
    statement_astnode* then;
    statement_astnode* exp;
    void print(int b){
        cout<<"\"if\": {"<<endl;
        cout<<"\"cond\": "<<endl;
        if(condition->astnode_type==e_empty_astnode)
            condition->print(b);
        else{
            cout<<"{";
            condition->print(b);
            cout<<"}"<<endl;
        }
        cout<<","<<endl;
        cout<<"\"then\": "<<endl;
        if(then->astnode_type==e_empty_astnode)
            then->print(b);
        else{
            cout<<"{";
            then->print(b);
            cout<<"}"<<endl;
        }
        cout<<","<<endl;
        cout<<"\"else\": "<<endl;
        if(exp->astnode_type != e_empty_astnode)
        {
            cout<<"{";
            exp->print(b);
            cout<<"}"<<endl;
        }
        else {
            exp->print(b);
        }

        cout<<"}"<<endl;

    }
    void gencode_node(SymbTab* st) override;

};

class while_astnode : public  statement_astnode
{
public:
    while_astnode(){
        this->astnode_type = e_while_astnode;
    };
    exp_astnode* condition;
    statement_astnode* exp;

    void print(int b){
        cout<<"\"while\": {"<<endl;
        cout<<"\"cond\":"<<endl;
        if(condition->astnode_type==e_empty_astnode)
            condition->print(b);
        else{
            cout<<"{";
            condition->print(b);
            cout<<"}"<<endl;
        }
        cout<<","<<endl;
        cout<<"\"stmt\": "<<endl;
        if(exp->astnode_type==e_empty_astnode)
            exp->print(b);
        else{
            cout<<"{"<<endl;
            exp->print(b);
            cout<<"}"<<endl;
        }
        cout<<"}"<<endl;
    }
    void gencode_node(SymbTab* st) override;

};

class for_astnode : public statement_astnode
{
public:
    for_astnode(){
        this->astnode_type=e_for_astnode;
    };
    exp_astnode* exp;
    exp_astnode* guard;
    exp_astnode* step;
    statement_astnode* body;

    void print(int b){
        cout<<"\"for\": {"<<endl;
        cout<<"\"init\":"<<endl;
        if(exp->astnode_type == e_empty_astnode)
        {
            exp->print(b);
        }
        else
        {
            cout<<"{";
            exp->print(b);
            cout<<"}"<<endl;   
        }
        cout<<","<<endl;
        cout<<"\"guard\": "<<endl;
        if(guard->astnode_type == e_empty_astnode)
        {
            guard->print(b);
        }
        else
        {
            cout<<"{";
            guard->print(b);
            cout<<"}"<<endl;   
        }
        cout<<","<<endl;
        cout<<"\"step\": "<<endl;
        if(step->astnode_type == e_empty_astnode)
        {
            step->print(b);
        }
        else
        {
            cout<<"{";
            step->print(b);
            cout<<"}"<<endl;   
        }
        cout<<","<<endl;
        cout<<"\"body\": "<<endl;
        if(body->astnode_type == e_empty_astnode)
        {
            body->print(b);
        }
        else
        {
            cout<<"{";
            body->print(b);
            cout<<"}"<<endl;   
        }
        cout<<"}"<<endl;
    }
    void gencode_node(SymbTab* st) override;


};


class ref_astnode : public exp_astnode
{
public:
    ref_astnode(){
        this->astnode_type = e_ref_astnode;
    };
    void gencode_node(SymbTab* st) override;

};

class identifier_astnode : public ref_astnode
{
public:
    identifier_astnode(){
        this->astnode_type=e_identifier_astnode;
    };
    string id;
    void print(int b){
        cout<<"\"identifier\": \""<<id<<"\""<<endl;
    }
    void gencode_node(SymbTab* st) override;

};

class arrayref_astnode : public ref_astnode
{
public:
    arrayref_astnode(){
        this->astnode_type=e_arrayref_astnode;
    };
    exp_astnode* index;
    exp_astnode* array;
    void print(int b){
        cout<<"\"arrayref\": {"<<endl;
        cout<<"\"array\":\n";
        if(index->astnode_type==e_empty_astnode)
            array->print(b);
        else{
            cout<<"{\n";
            array->print(b);
            cout<<"},\n";
        }
        cout<<"\"index\":\n";
        if(index->astnode_type==e_empty_astnode)
            index->print(b);
        else{
            cout<<"{";
            index->print(b);
            cout<<"}";
        }
        cout<<"}\n";
    }
    void gencode_node(SymbTab* st) override;

};

class member_astnode : public ref_astnode
{
public:
    member_astnode(){
        this->astnode_type=e_member_astnode;
    };
    exp_astnode* ref;
    identifier_astnode* id; 
    void print(int b)
    {

        cout<<"\"member\":{\n\"struct\":\n";
        if(ref->astnode_type==e_empty_astnode)
            ref->print(b);
        else{
            cout<<"{\n";
            ref->print(0);
            cout<<"}\n,\n";
        }
        cout<<"\"field\":";
        if(id->astnode_type==e_empty_astnode)
            id->print(b);
        else{
            cout<<"{\n";
            id->print(0);
            cout<<"}\n";
        }
        cout<<"}\n";
    }
    void gencode_node(SymbTab* st) override;

};

class arrow_astnode : public ref_astnode
{
public:
    arrow_astnode(){
        this->astnode_type=e_arrow_astnode;
    };
    exp_astnode* arrow;
    identifier_astnode* id;
    void print(int b)
    {
        cout<<"\"arrow\": {\n";
        cout<<"\"pointer\":\n";
        if(arrow->astnode_type==e_empty_astnode)
            arrow->print(b);
        else{
            cout<<"{\n";
            arrow->print(b);
            cout<<"}\n,\n";
        }
        cout<<"\"field\":\n";
        if(id->astnode_type==e_empty_astnode)
            id->print(b);
        else{
            cout<<"{\n";
            id->print(b);
            cout<<"}\n}\n";
        }
    }
    void gencode_node(SymbTab* st) override;

};

class op_binary_astnode : public exp_astnode
{
public:
    op_binary_astnode(){
        this->astnode_type=e_op_binary_astnode;
    };
    string op;
    exp_astnode* exp1;
    exp_astnode* exp2;
    void print(int b){
        cout<<"\"op_binary\": {"<<endl;
        cout<<"\"op\": \""<<op<<"\""<<endl;
        cout<<","<<endl;
        cout<<"\"left\":"<<endl;
        if(exp1->astnode_type==e_empty_astnode){
            exp1->print(b);
        }
        else{
            cout<<"{"<<endl;
            exp1->print(b);
            cout<<"}"<<endl;
        }
        cout<<","<<endl;
        cout<<"\"right\": "<<endl;
        if(exp2->astnode_type==e_empty_astnode){
            exp2->print(b);
        }
        else{
            cout<<"{\n";
            exp2->print(b);
            cout<<"}"<<endl;
        }
        cout<<"}"<<endl;
    }

    void gencode_node(SymbTab* st) override;

};

class op_unary_astnode : public exp_astnode
{
public:
    op_unary_astnode(){
        this->astnode_type=e_op_unary_astnode;
    };
    string s;
    exp_astnode* exp;
    void print(int b)
    {
        cout<<"\"op_unary\": {"<<endl;
        cout<<"\"op\": \""<<s<<"\"" <<endl;
        cout<<","<<endl;
        cout<<"\"child\" :"<<endl;
        if(exp->astnode_type==e_empty_astnode){
            exp->print(b);
        }
        else{
            cout<<"{"<<endl;
            exp->print(b);
            cout<< "}"<<endl;
        }
        cout<<"}"<<endl;
    }
    void gencode_node(SymbTab* st) override;

};

class assignE_astnode : public exp_astnode
{
public:
    assignE_astnode(){
        this->astnode_type=e_assignE_astnode;
    };
    exp_astnode* left;
    exp_astnode* right;
    void print(int b){
        cout<<"\"assignE\": {"<<endl;
        cout<<"\"left\":"<<endl;
        if(left->astnode_type==e_empty_astnode){
            left->print(b);
        }
        else{
            cout<<"{"<<endl;
            left->print(b);
            cout<<"}"<<endl;
        }
        cout<<","<<endl;
        cout<<"\"right\": "<<endl;
        if(right->astnode_type==e_empty_astnode){
            right->print(b);
        }
        else{
            cout<<"{\n";
            right->print(b);
            cout<<"}"<<endl;
        }
        cout<<"}"<<endl;

    }
    void gencode_node(SymbTab* st) override;

};

class funcall_astnode : public exp_astnode
{
public:
    funcall_astnode(){
        this->astnode_type=e_funcall_astnode;
        this->su_label = 2;
    };
    vector<exp_astnode*> params;
    identifier_astnode* fname;
    void print(int b)
    {
        cout<<"\"funcall\": {\n";
        cout<<"\"fname\": \n";
        if(fname->astnode_type==e_empty_astnode){
            fname->print(b);
        }
        else{
            cout<<"{\n";
            fname->print(b);
            cout<<"}\n,\n";
        }
        cout<<"\"params\": [\n";
        for(auto it = params.begin(); it!=params.end(); it++)
        {
            //if((*it)->astnode_type==e_empty_astnode){
            //(*it)->print(b);
            //}
            //else{
                cout<<"{\n";
                (*it)->print(b);
                if (next(it,1) != params.end()) {cout<<"}\n,\n";}
                else {cout<<"}\n";}
            //}
        }
        cout<<"]\n}\n";
    }
    void gencode_node(SymbTab* st) override;

};

class intconst_astnode : public exp_astnode
{
public:
    intconst_astnode(){
        this->astnode_type=e_intconst_astnode;
    };
    int s;
    void print(int b){
        cout<<"\"intconst\": "<<s<<endl;
    }
    void gencode_node(SymbTab* st) override;

};

class floatconst_astnode : public exp_astnode
{
public:
    floatconst_astnode(){
        this->astnode_type=e_floatconst_astnode;
    };
    float f;
    void print(int b){
        cout<<"\"floatconst\": "<<f<<endl;
    }
    void gencode_node(SymbTab* st) override;

};

class stringconst_astnode : public exp_astnode
{
public:
    stringconst_astnode(){
        this->astnode_type=e_stringconst_astnode;
    };
    string s;
    void print(int b){
        cout<<"\"stringconst\": "<<s<<endl;
    }
    void gencode_node(SymbTab* st) override;

};

class proccall_astnode : public  statement_astnode
{
public:
    proccall_astnode(){
        this->astnode_type=e_proccall_astnode;
        this->su_label = 2;
    };
    vector<exp_astnode*> params;
    identifier_astnode* fname;

    void print(int b)
    {
        cout<<"\"proccall\": {\n";
        cout<<"\"fname\": \n";
        if(fname->astnode_type==e_empty_astnode){
            fname->print(b);
        }
        else{
            cout<<"{\n";
            fname->print(b);
            cout<<"}\n,\n";
        }
        cout<<"\"params\": [\n";
        for(auto it = params.begin(); it!=params.end(); it++)
        {
            //if((*it)->astnode_type==e_empty_astnode){
            //(*it)->print(b);
            //}
            //else{
                cout<<"{";
                (*it)->print(b);
                if (next(it,1) != params.end()) {cout<<"}\n,\n";}
                else {cout<<"}\n";}
           // }
        }
        cout<<"]\n}\n";
    }
    void gencode_node(SymbTab* st) override;

};


class printcall_astnode : public  statement_astnode
{
public:
    printcall_astnode(){
        this->astnode_type=e_proccall_astnode;
    };
    vector<exp_astnode*> params;
    identifier_astnode* fname;

    void print(int b)
    {
        cout<<"\"proccall\": {\n";
        cout<<"\"fname\": \n";
        if(fname->astnode_type==e_empty_astnode){
            fname->print(b);
        }
        else{
            cout<<"{\n";
            fname->print(b);
            cout<<"}\n,\n";
        }
        cout<<"\"params\": [\n";
        for(auto it = params.begin(); it!=params.end(); it++)
        {
            //if((*it)->astnode_type==e_empty_astnode){
            //(*it)->print(b);
            //}
            //else{
                cout<<"{";
                (*it)->print(b);
                if (next(it,1) != params.end()) {cout<<"}\n,\n";}
                else {cout<<"}\n";}
           // }
        }
        cout<<"]\n}\n";
    }
    void gencode_node(SymbTab* st) override;

};

class assignS_astnode : public statement_astnode
{
public:
    assignS_astnode(){
        this->astnode_type=e_assignS_astnode;
    };
    exp_astnode* left;
    exp_astnode* right;
    void print(int b){
        cout<<"\"assignS\": {"<<endl;
        cout<<"\"left\":"<<endl;
        if(left->astnode_type==e_empty_astnode)
            left->print(b);
        else{
            cout<<"{"<<endl;
            left->print(b);
            cout<<"}"<<endl;
        }
        cout<<","<<endl;
        cout<<"\"right\": "<<endl;
        if(right->astnode_type==e_empty_astnode)
            right->print(b);
        else{
            cout<<"{\n";
            right->print(b);
            cout<<"}"<<endl;
        }
        cout<<"}"<<endl;
    }

    void gencode_node(SymbTab* st) override;


};


#endif