#ifndef __TYPE_HH__
#define __TYPE_HH__

#include "symbtab.hh"


class declaration_list_class
{
public:
    SymbTab* symbtab;
    int size = 0;
};

class declaration_class
{
public:
    SymbTab* symbtab;
    std::string type;
};

class declarator_list_class
{
public:
    SymbTab* symbtab;
    std::string type; 
};

class declarator_class
{
public:
    SymbTab* symbtab;
    std::string type; 
    std::string name;
    int size = 1;
};

class declarator_arr_class
{
public:
    SymbTab* symbtab;
    std::string type; 
    std::string name;
    int size = 1;
};

class type_specifier_class
{
public:
    std::string type;
    type_specifier_class(std::string t)
    {
        this->type = t;
    }
};


// =================================//
class translation_unit_class
{
public:
    
};

class struct_specifier_class
{
public:
    
};

class function_definition_class
{
public:
    
};


class fun_declarator_class
{
public:
    std::string name;
};

class compound_statement_class
{
public:
    statement_astnode* astnode;
};


class parameter_declaration_class
{
public:
    int size = 1;
    std::string name;
};

class parameter_list_class
{
public:
    
};

class statement_list_class
{
public:

    vector<statement_astnode*> vec_statement_astnode;
};

class statement_class
{
public:
    statement_astnode* astnode;
};

class assignment_expression_class
{
public:
    exp_astnode* astnode;
    exp_astnode* left;
    exp_astnode* right;
};

class assignment_statement_class
{
public:
    statement_astnode* astnode;
};

class procedure_call_class
{
public:
    statement_astnode* astnode;
};

class logical_and_expression_class
{
public:
    exp_astnode* astnode;
};

class equality_expression_class
{
public:
    std::string type;
    exp_astnode* astnode;
};

class relational_expression_class
{
public:
    std::string type;
    exp_astnode* astnode;  
};

class additive_expression_class
{
public:
    std::string type;
    exp_astnode* astnode;
};

class unary_expression_class
{
public:
    std::string type;
    exp_astnode* astnode;  
};

class multiplicative_expression_class
{
public:
    std::string type;
    exp_astnode* astnode;
};

class postfix_expression_class
{
public:
    exp_astnode* astnode;
};

class primary_expression_class
{
public:
    exp_astnode* astnode;
};

class expression_list_class
{
public:
    vector<exp_astnode*> vec_exp_astnode;
};

class expression_class
{
public:
   exp_astnode* astnode; 
};

class unary_operator_class
{
public:
    std::string op;
};

class selection_statement_class
{
public:
    statement_astnode* astnode;
};

class iteration_statement_class
{
public:
    statement_astnode* astnode;
};

class printf_call_class
{
public:
    statement_astnode* astnode;
};


#endif
