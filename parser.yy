%skeleton "lalr1.cc"
%require  "3.0.1"

%defines 
%define api.namespace {IPL}
%define api.parser.class {Parser}

%define parse.trace

%code requires{
   #include "location.hh"
   #include "ast.hh"
   #include "type.hh"
   #include "gencode.hh"
   namespace IPL {
      class Scanner;
   }
   extern SymbTab gst;
   extern SymbTab* localst;
   extern std::vector<std::pair<std::string, int>> params;
   extern int offset;
   extern std::string type;
   extern std::string basictype;
   extern std::string ret_type;
   extern std::map<std::string,abstract_astnode*> ast;
   extern std::vector<statement_astnode> gvec_statment_astnode;
   extern std::vector<exp_astnode> gvec_exp_astnode; 

   extern std::string get_deref_type(std::string type) ;   
   extern std::string arr_to_ptr(std::string type);
   extern std::string get_type(exp_astnode*, exp_astnode*);
   extern std::string get_ref_type(std::string type);
   extern bool check_compatibility(exp_astnode* &astnode, std::string type);
   extern bool check_compatibility_prime(exp_astnode* &astnode, std::string type, bool t);

   extern std::vector<std::string> predefined;

   extern Gencode gencode;
   extern std::vector<std::string> rstack;
   extern const int TOTAL_REG;

}

%printer { std::cerr << $$; } STRUCT
%printer { std::cerr << $$; } WHILE
%printer { std::cerr << $$; } FOR
%printer { std::cerr << $$; } IF
%printer { std::cerr << $$; } ELSE
%printer { std::cerr << $$; } PRINTF
%printer { std::cerr << $$; } VOID
%printer { std::cerr << $$; } INT
%printer { std::cerr << $$; } FLOAT
%printer { std::cerr << $$; } INT_CONSTANT
%printer { std::cerr << $$; } FLOAT_CONSTANT
%printer { std::cerr << $$; } IDENTIFIER
%printer { std::cerr << $$; } STRING_LITERAL
%printer { std::cerr << $$; } RETURN



%parse-param { Scanner  &scanner  }
%locations
%code{
   #include <iostream>
   #include <cstdlib>
   #include <fstream>
   #include <string>
   #include <algorithm>
    
   #include "scanner.hh"
   bool is_struct = false;
   using namespace std;
#undef yylex
#define yylex IPL::Parser::scanner.yylex
#ifndef POINTER_SZ
#define POINTER_SZ 4
#endif

}

%define api.value.type variant
%define parse.assert

%start program

%token <std::string> MAIN
%token <std::string> STRUCT
%token <std::string> WHILE
%token <std::string> FOR 
%token <std::string> IF
%token <std::string> ELSE
%token <std::string> PRINTF
%token <std::string> INT
%token <std::string> FLOAT
%token <std::string> VOID
%token <std::string> INT_CONSTANT
%token <std::string> FLOAT_CONSTANT
%token <std::string> IDENTIFIER
%token <std::string> STRING_LITERAL
%token <std::string> LE_OP
%token <std::string> GE_OP
%token <std::string> AND_OP
%token <std::string> OR_OP
%token <std::string> EQ_OP
%token <std::string> NE_OP
%token <std::string> PTR_OP
%token <std::string> INC_OP
%token <std::string> OTHERS
%token <std::string> RETURN 
%token ',' '(' ')' '+' '*' '-' '/' '[' ']' '&' '!' '>' '<' ';' '{' '}' '=' '.'

%nterm <translation_unit_class*> translation_unit
%nterm <struct_specifier_class*> struct_specifier
%nterm <function_definition_class*> function_definition
%nterm <declaration_list_class*> declaration_list
%nterm <fun_declarator_class*> fun_declarator
%nterm <compound_statement_class*> compound_statement
%nterm <type_specifier_class*> type_specifier
%nterm <parameter_declaration_class*> parameter_declaration
%nterm <parameter_list_class*> parameter_list
%nterm <declarator_class*> declarator
%nterm <declarator_arr_class*> declarator_arr
%nterm <declarator_list_class*> declarator_list
%nterm <declaration_class*> declaration
%nterm <statement_list_class*> statement_list
%nterm <statement_class*> statement
%nterm <assignment_expression_class*> assignment_expression
%nterm <assignment_statement_class*> assignment_statement
%nterm <procedure_call_class*> procedure_call
%nterm <logical_and_expression_class*> logical_and_expression
%nterm <equality_expression_class*> equality_expression
%nterm <relational_expression_class*> relational_expression
%nterm <additive_expression_class*> additive_expression
%nterm <unary_expression_class*> unary_expression
%nterm <multiplicative_expression_class*> multiplicative_expression
%nterm <postfix_expression_class*> postfix_expression
%nterm <primary_expression_class*> primary_expression
%nterm <expression_list_class*> expression_list
%nterm <expression_class*> expression
%nterm <unary_operator_class*> unary_operator
%nterm <selection_statement_class*> selection_statement
%nterm <iteration_statement_class*> iteration_statement
%nterm <printf_call_class*> printf_call


%%
program: 
   main_definition
   {

   }
  | translation_unit main_definition
  {

  }

translation_unit: 
               struct_specifier
               {
                  
               }
               | function_definition
               {
                  
               }
               | translation_unit struct_specifier
               {
                  
               }
               | translation_unit function_definition
               {
                  
               }

main_definition: 
               INT 
               {localst = new SymbTab(); is_struct = false;offset=0;ret_type = "int";} 
               MAIN 
               {
                  gencode.genfuncheader($3);
               }
               '(' ')' 
               compound_statement
               {
                  SymbTab_entry* tmp = new SymbTab_entry($3, GLOBAL, "fun", 0, 0, "int", localst);
                  gst.Entries.insert({$3, tmp});            

                  offset = 0;
                  ast.insert({$3, $7->astnode});
                  gencode.gen("\tleave");
                  gencode.gen("\tret");
                  gencode.gen("\t.size\t"+$3+" ,.-"+$3);
                  gencode.print_rodata_str_literals();
                  gencode.print_assembly();
                  // cout<<"no seg fault"<<endl;
               }

struct_specifier: STRUCT IDENTIFIER '{' {localst = new SymbTab();is_struct = true;} declaration_list '}' ';'
                  {
                     // TODO : change size
                     SymbTab_entry* tmp = new SymbTab_entry("struct " + $2, GLOBAL, "struct", offset, -1, "-", localst);
                     gst.Entries.insert({"struct " + $2, tmp});
                     offset = 0;
                  }

function_definition: type_specifier {localst = new SymbTab(); is_struct = false;offset=0;ret_type = $1->type;}
               fun_declarator
               {
                  // std::reverse(params.begin(), params.end());
                  int param_offset = 12;
                  for (const auto &it : params)
                  {
                     //cout<<"here?"<<it.first<< " " << params.size()<<endl;

                     SymbTab_entry* params_entry = localst->Entries[it.first];
                     params_entry->offset = param_offset;
                     // cout << $3->name << " " << it.first <<  " " << param_offset << endl; 
                     param_offset += it.second;
                  }

                  params.clear();
                  gencode.genfuncheader($3->name);
                  SymbTab_entry* tmp = new SymbTab_entry($3->name, GLOBAL, "fun", 0, 0, $1->type, localst);
                  gst.Entries.insert({$3->name, tmp});            

               }
               compound_statement         
               {
                  SymbTab_entry* tmp = new SymbTab_entry($3->name, GLOBAL, "fun", 0, 0, $1->type, localst);
                  gst.Entries.insert({$3->name, tmp});            
                  
                  // std::reverse(params.begin(), params.end());
                  // int param_offset = 12;
                  // for (const auto &it : params)
                  // {
                  //    //cout<<"here?"<<it.first<< " " << params.size()<<endl;

                  //    SymbTab_entry* params_entry = localst->Entries[it.first];
                  //    params_entry->offset = param_offset;
                  //    param_offset += it.second;
                  // }

                  // params.clear();

                  offset = 0;
                  ast.insert({$3->name, $5->astnode});
                  gencode.gen("\tleave");
                  gencode.gen("\tret");
                  gencode.gen("\t.size\t"+$3->name+" ,.-"+$3->name);
                  gencode.print_rodata_str_literals();
                  gencode.print_assembly();
               }

type_specifier: VOID
               {
                  $$ = new type_specifier_class("void");
                  type = "";
                  basictype = "void";
               }
               | INT
               {
                  $$ = new type_specifier_class("int");
                  type = "";
                  basictype = "int";
               }
               | FLOAT
               {
                  $$ = new type_specifier_class("float");
                  type = "";
                  basictype = "float";
               }
               | STRUCT IDENTIFIER
               {
                  $$ = new type_specifier_class("struct " + $2);
                  type = "";
                  basictype = "struct " + $2;
               }

fun_declarator: IDENTIFIER '(' parameter_list ')'
            {
               $$ = new fun_declarator_class();
               $$->name = $1;
               offset = 0;
            }
            | IDENTIFIER '(' ')'
            {
               $$ = new fun_declarator_class();
               $$->name = $1;
               offset = 0;         
            }

parameter_list: parameter_declaration
            {
               int factor = 1;
               int tmp_sz = 1;

               if(type[0] == '*')
               {
                  factor = POINTER_SZ;
               }
               else
               {
                  if(basictype.compare("int")==0) factor = 4;
                  else if(basictype.compare("float")==0) factor = 4;
                  else if(basictype.compare("void")==0) factor = 4;
                  else if(gst.Entries.find(basictype) != gst.Entries.end())
                  {
                     SymbTab_entry* tmp_factor_e = gst.Entries[basictype];
                     factor = tmp_factor_e->sz;
                  }
                  else
                  {
                     std::cout<<"No type exists"<<std::endl;
                     exit(1);
                  }
               }
               tmp_sz = $1->size*factor;
               params.push_back(std::make_pair($1->name, tmp_sz));
               if((basictype+type).compare("void")==0) error(@$, "Cannot declare variable of type \"void\"");
               SymbTab_entry* tmp_e = new SymbTab_entry($1->name, PARAM, "var", tmp_sz, -1, basictype + type, NULL);
               localst->Entries.insert({$1->name, tmp_e});
               offset += tmp_sz;
            }
            | parameter_list ',' parameter_declaration
            {
               int factor = 1;
               int tmp_sz = 1;

               if(type[0] == '*')
               {
                  factor = POINTER_SZ;
               }
               else
               {
                  if(basictype.compare("int")==0) factor = 4;
                  else if(basictype.compare("float")==0) factor = 4;
                  else if(basictype.compare("void")==0) factor = 4;
                  else if(gst.Entries.find(basictype) != gst.Entries.end())
                  {
                     SymbTab_entry* tmp_factor_e = gst.Entries[basictype];
                     factor = tmp_factor_e->sz;
                  }
                  else
                  {
                     std::cout<<"No type exists"<<std::endl;
                     exit(1);
                  }
               }
               tmp_sz = $3->size*factor;
               params.push_back(std::make_pair($3->name, tmp_sz));
               
               if((basictype+type).compare("void")==0) error(@$, "Cannot declare variable of type \"void\"");

               SymbTab_entry* tmp_e = new SymbTab_entry($3->name, PARAM, "var", tmp_sz, -1, basictype + type, NULL);
               localst->Entries.insert({$3->name, tmp_e});
               offset += tmp_sz;
            }

parameter_declaration: type_specifier declarator
               {
                  $$ = new parameter_declaration_class();
                  $$->size = $2->size;
                  $$->name = $2->name;
               }

declarator_arr: IDENTIFIER
               {  
                  $$ = new declarator_arr_class();
                  $$->size = 1;
                  $$->name = $1;
               }
            | declarator_arr '[' INT_CONSTANT ']'
            {
               $$ = new declarator_arr_class();
               type = type + "[" + $3 + "]";
               $$->size = std::stoi($3)*($1->size);
               $$->name = $1->name;
            }

declarator: declarator_arr
               {
                  $$ = new declarator_class();
                  $$->size = $1->size;
                  $$->name = $1->name;
               }
            | '*' declarator
               {
                  $$ = new declarator_class();
                  $$->size = $2->size;
                  $$->name = $2->name;
                  type = "*" + type;
               }

compound_statement: '{' '}'
               {
                  seq_astnode* tmp = new seq_astnode();
                  tmp->vec_statement_astnode = std::vector<statement_astnode*>();
                  $$ = new compound_statement_class();
                  $$->astnode = tmp;        
               }
                  | '{' statement_list 
                  {
                  // cout<<"statement_list start" << $2->vec_statement_astnode.size()<< endl;

                     for(auto it = $2->vec_statement_astnode.begin(); it!=$2->vec_statement_astnode.end(); it++)
                     {
                        // cout << (*it)->astnode_type << endl;
                        (*it)->gencode_node(localst);
                     }
                  // cout<<"statement_list start" << endl;

                  }
                  '}'
               {
                  seq_astnode* tmp = new seq_astnode();
                  tmp->vec_statement_astnode = $2->vec_statement_astnode;
                  $$ = new compound_statement_class();
                  $$->astnode = tmp;
               }
                  | '{' declaration_list '}'
               {
                  seq_astnode* tmp = new seq_astnode();
                  tmp->vec_statement_astnode = std::vector<statement_astnode*>();
                  $$ = new compound_statement_class();
                  $$->astnode = tmp;    
                  gencode.gen("\tsubl\t$" + to_string(-offset)+", %esp");
               }
               | '{' declaration_list 
                  {
                     gencode.gen("\tsubl\t$" + to_string(-offset)+", %esp");
                  }
               statement_list 
               {
                  // cout<<"statement_list start" << $4->vec_statement_astnode.size()<<endl;
                  for(auto it = $4->vec_statement_astnode.begin(); it!=$4->vec_statement_astnode.end(); it++)
                  {
                     // cout << (*it)->astnode_type << endl;
                     (*it)->gencode_node(localst);
                  }
                  // cout<<"statement_list end" << endl;

               }  
                  '}'
               {
                  seq_astnode* tmp = new seq_astnode();
                  tmp->vec_statement_astnode = $4->vec_statement_astnode;
                  $$ = new compound_statement_class();
                  $$->astnode = tmp;

               }

statement_list: statement
               {
                  $$ = new statement_list_class();
                  $$->vec_statement_astnode.push_back($1->astnode);
               }
            | statement_list statement
               {
                  $$ = new statement_list_class();
                  $1->vec_statement_astnode.push_back($2->astnode);
                  $$->vec_statement_astnode = $1->vec_statement_astnode;                
               }

statement: ';'
            {
               $$ = new statement_class();
               $$->astnode = new empty_astnode;
            }
         | '{' statement_list '}'
            {
               $$ = new statement_class();
               seq_astnode* tmp = new seq_astnode();
               tmp->vec_statement_astnode = $2->vec_statement_astnode;

               $$->astnode = tmp;
            }
         | selection_statement
            {
               $$ = new statement_class();
               $$->astnode = $1->astnode;
            }
         | iteration_statement
            {
               $$ = new statement_class();
               $$->astnode = $1->astnode;
            }
         | assignment_statement
            {
               $$ = new statement_class();
               $$->astnode = $1->astnode;
            }
         | procedure_call
            {
               $$ = new statement_class();
               $$->astnode = $1->astnode;
            }
         | printf_call
         {
            $$ = new statement_class();
            $$->astnode = $1->astnode;
         }
         | RETURN expression ';'
            {
               $$ = new statement_class();
               return_astnode* tmp = new return_astnode();
               tmp->return_val = $2->astnode;
               // cout<<"ret_type : " << ret_type <<endl;
               if(ret_type.compare("float") == 0)
               {
                  if($2->astnode->type.compare("int") == 0)
                  {
                     op_unary_astnode* tmp_change = new op_unary_astnode();
                     tmp_change->s = "TO_FLOAT";
                     tmp_change->exp = $2->astnode;
                     tmp->return_val = tmp_change;
                  }
               }
               if(ret_type.compare("int") == 0)
               {
                  if($2->astnode->type.compare("float") == 0)
                  {
                     op_unary_astnode* tmp_change = new op_unary_astnode();
                     tmp_change->s = "TO_INT";
                     tmp_change->exp = $2->astnode;
                     tmp->return_val = tmp_change;
                  }
               }
               if(ret_type.compare("void")==0)
               {
                  error(@1, "Incompatible type \"" + ret_type + "\" returned, expected \"" +  + "\"");
               }
               // TODO : type checking 
               $$->astnode = tmp;
            }

assignment_expression: unary_expression '=' expression
            {
               $$ = new assignment_expression_class();
               assignE_astnode* tmp = new assignE_astnode();
               std::string type_l, type_r;
               type_l = $1->astnode->type;
               type_r = $3->astnode->type;
               if(!$1->astnode->is_lval)
               {
                  error(@1, "Left operand of assignment should have an lvalue");
               }
               if($1->astnode->type.compare("float") == 0)
               {
                  if($3->astnode->type.compare("int") == 0)
                  {
                     op_unary_astnode* tmp_exp = new op_unary_astnode();
                     tmp_exp->s = "TO_FLOAT";
                     tmp_exp->exp = $3->astnode;
                     tmp_exp->type = "float";
                     $3->astnode = tmp_exp;
                     type_r = "float";
                  }
               }
               if($3->astnode->type.compare("float") == 0)
               {
                  if($1->astnode->type.compare("int") == 0)
                  {
                     op_unary_astnode* tmp_exp = new op_unary_astnode();
                     tmp_exp->s = "TO_INT";
                     tmp_exp->exp = $3->astnode;
                     $3->astnode = tmp_exp;
                     type_l = "float";
                  }
               }

               if(!(type_l.compare("void*") == 0 || type_r.compare("void*") == 0))
               {

                  if(type_l.compare(type_r) != 0)
                  {
                       
                     if(!($3->astnode->astnode_type==e_intconst_astnode  && type_l.back()=='*' && $3->astnode->is_zero))
                     {
                        if(!check_compatibility_prime($3->astnode, type_l, false) && !$3->astnode->is_zero)
                        {
                           error(@1, "Incompatible assignment when assigning to type \"" + type_l + "\" from type \"" + type_r + "\"");
                        }
                     }
                  }
               } 

               tmp->left = $1->astnode;
               $$->left = $1->astnode;
               tmp->right = $3->astnode;
               $$->right = $3->astnode;
               $$->astnode = tmp;

               // cout<<"no seg fault" << endl;
               // code generation part
               // $$->astnode->gencode_node(localst);
               // cout<<"no seg fault" << endl;

            }

assignment_statement: assignment_expression ';'
            {
               $$ = new assignment_statement_class();
               assignS_astnode* tmp = new assignS_astnode();
               tmp->left = $1->left;
               tmp->right = $1->right;
               $$->astnode = tmp;
            }

procedure_call: IDENTIFIER '(' ')' ';'
            {
               bool is_predefined = false;
               if(gst.Entries.find($1) == gst.Entries.end() && std::find(predefined.begin(), predefined.end(), $1)==predefined.end())
               {
                  error(@1, "Function \"" + $1 + "\" not declared");
               }
               if(std::find(predefined.begin(), predefined.end(), $1)!=predefined.end())
               {
                  is_predefined = true;
               }
                  int arg_count = 0;
               if(!is_predefined)
               {
                  for(auto& it : gst.Entries[$1]->localSBT->Entries)
                  {
                     if(it.second->scp == PARAM)
                        arg_count++;
                  }
                  if(arg_count > 0)
                  {
                     error(@1, "Procedure \"" + $1 +  "\"  called with too few arguments");
                  }              
               }

               $$ = new procedure_call_class();
               proccall_astnode* tmp = new proccall_astnode();
               tmp->fname = new identifier_astnode();
               tmp->fname->id = $1;

               $$->astnode = tmp;
            }
            | IDENTIFIER '(' expression_list ')' ';'
            {
               bool is_predefined = false;
               if(gst.Entries.find($1) == gst.Entries.end() && std::find(predefined.begin(), predefined.end(), $1)==predefined.end())
               {
                  error(@1, "Function \"" + $1 + "\" not declared");
               }
               unsigned long int arg_count = 0;
               vector<pair<int, string> > arguments;

               if(std::find(predefined.begin(), predefined.end(), $1)!=predefined.end())
               {
                  is_predefined = true;
               }
               if(!is_predefined)
               {
                  for(auto& it : gst.Entries[$1]->localSBT->Entries)
                  {
                     if(it.second->scp == PARAM)
                     {
                        arg_count++;
                        arguments.push_back(make_pair(it.second->offset, it.second->return_var_type));
                     }
                     sort(arguments.begin(), arguments.end(),[](pair<int,string>p1, pair<int, string>p2){return p1<p2;});
                  }
                  if(arg_count > $3->vec_exp_astnode.size())
                  {
                     error(@1, "Procedure \"" + $1 +  "\"  called with too few arguments");
                  }
                  if(arg_count < $3->vec_exp_astnode.size())
                  {
                     error(@1, "Procedure \"" + $1 +  "\"  called with too many arguments");
                  }
                  for(int i=0; i<arguments.size(); i++)
                  {
                     if(!check_compatibility($3->vec_exp_astnode[i], arguments[i].second))
                     {
                        error(@3, "Expected \"" + arguments[i].second + "\" but argument is of type \""+$3->vec_exp_astnode[i]->type+"\"");
                     }

                  }          
               }


               $$ = new procedure_call_class();
               proccall_astnode* tmp = new proccall_astnode();
               tmp->fname = new identifier_astnode();
               tmp->fname->id = $1;
               tmp->params = $3->vec_exp_astnode;

               $$->astnode = tmp;
            }

expression: logical_and_expression
            {
               $$ = new expression_class();
               $$->astnode = $1->astnode;
            }
         | expression OR_OP logical_and_expression
            {
               $$ = new expression_class();
               op_binary_astnode* tmp = new op_binary_astnode();
               tmp->exp1 = $1->astnode;
               tmp->exp2 = $3->astnode;
               std::string type_l = arr_to_ptr($1->astnode->type);
               std::string type_r = arr_to_ptr($3->astnode->type);
               if(type_l!="int" && type_l!="float" && get_deref_type(type_l).empty())
               {
                  error(@1, "Invalid operand of ||,  not scalar or pointer");
               }
               if(type_r!="int" && type_r!="float" && get_deref_type(type_r).empty())
               {
                  error(@1, "Invalid operand of ||,  not scalar or pointer");
               }

               tmp->op = std::string("OR_OP");
               tmp->type = "int";
               $$->astnode = tmp;
               $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
            }

logical_and_expression: equality_expression
            {
               $$ = new logical_and_expression_class();
               $$->astnode = $1->astnode;
            }
            | logical_and_expression AND_OP equality_expression
            {
               $$ = new logical_and_expression_class();
               op_binary_astnode* tmp = new op_binary_astnode();
               tmp->exp1 = $1->astnode;
               tmp->exp2 = $3->astnode;
               std::string type_l = arr_to_ptr($1->astnode->type);
               std::string type_r = arr_to_ptr($3->astnode->type);
               if(type_l!="int" && type_l!="float" && get_deref_type(type_l).empty())
               {
                  error(@1, "Invalid operand of &&,  not scalar or pointer");
               }
               if(type_r!="int" && type_r!="float" && get_deref_type(type_r).empty())
               {
                  error(@1, "Invalid operand of &&,  not scalar or pointer");
               }
               tmp->op = std::string("AND_OP");
               tmp->type = "int";
               $$->astnode = tmp;
               $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
            }


equality_expression: relational_expression
                  {
                     $$ = new equality_expression_class();
                     $$->astnode = $1->astnode;
                  }
                  | equality_expression EQ_OP relational_expression
                  {
                     $$ = new equality_expression_class();
                     op_binary_astnode* tmp = new op_binary_astnode();
                     // TODO : Type checking
                     tmp->exp1 = $1->astnode;
                     tmp->exp2 = $3->astnode;
                     std::string type_l = arr_to_ptr($1->astnode->type);
                     std::string type_r = arr_to_ptr($3->astnode->type);
                     bool throw_error = false;
                     if(type_l.compare("int") !=0 && type_l.compare("float")!=0)
                     {
                        throw_error = true;
                     }
                     if(type_r.compare("int") !=0 && type_r.compare("float")!=0)
                     {
                        throw_error = true;
                     }
                     if(type_l.compare(type_r)==0 && !get_deref_type(type_l).empty())
                     {
                        throw_error = false;
                     }
                     if(throw_error)
                     {
                           error(@1, "Invalid operand types for binary == , \"" + $1->astnode->type + "\" and \"" + $3->astnode->type +"\"");
                     }
                     if(tmp->exp1->type.compare("float") == 0)
                     {
                        if(tmp->exp2->type.compare("int") == 0)
                        {
                           op_unary_astnode* tmp_exp = new op_unary_astnode();
                           tmp_exp->s = "TO_FLOAT";
                           tmp_exp->exp = tmp->exp2;
                           tmp_exp->type = "float";
                           tmp->exp2 = tmp_exp;
                        }
                     }
                     if(tmp->exp2->type.compare("float") == 0)
                     {
                        if(tmp->exp1->type.compare("int") == 0)
                        {
                           op_unary_astnode* tmp_exp = new op_unary_astnode();
                           tmp_exp->s = "TO_FLOAT";
                           tmp_exp->exp = tmp->exp1;
                           tmp->exp1 = tmp_exp;
                        }
                     }
                     tmp->op = std::string("EQ_OP_" + get_type($1->astnode, $3->astnode));
                     tmp->type = "int";

                     $$->astnode = tmp;  
                     if($1->astnode->su_label != $3->astnode->su_label)
                        $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                     else $$->astnode->su_label = $1->astnode->su_label + 1;     
                  }
                  | equality_expression NE_OP relational_expression
                  {
                     $$ = new equality_expression_class();
                     op_binary_astnode* tmp = new op_binary_astnode();
                     tmp->exp1 = $1->astnode;
                     tmp->exp2 = $3->astnode;
                        std::string type_l = arr_to_ptr($1->astnode->type);
                        std::string type_r = arr_to_ptr($3->astnode->type);
                        bool throw_error = false;
                        if(type_l.compare("int") !=0 && type_l.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_r.compare("int") !=0 && type_r.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_l.compare(type_r)==0 && !get_deref_type(type_l).empty())
                        {
                           throw_error = false;
                        }
                        if(type_l == "int" && !get_deref_type(type_r).empty() && $1->astnode->is_zero)
                        {
                           throw_error = false;
                        }
                        if(type_r == "int" && !get_deref_type(type_l).empty() && $3->astnode->is_zero)
                        {
                           throw_error = false;
                        }
                        if(throw_error)
                        {
                           error(@1, "Invalid operand types for binary != , \"" + $1->astnode->type + "\" and \"" + $3->astnode->type +"\"");
                        }
                        if(tmp->exp1->type.compare("float") == 0)
                        {
                           if(tmp->exp2->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp2;
                              tmp_exp->type = "float";
                              tmp->exp2 = tmp_exp;
                           }
                        }
                        if(tmp->exp2->type.compare("float") == 0)
                        {
                           if(tmp->exp1->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp1;
                              tmp->exp1 = tmp_exp;
                           }
                        }
                     tmp->op = std::string("NE_OP_" + get_type($1->astnode, $3->astnode));

                     tmp->type = "int";
                     $$->astnode = tmp;
                     if($1->astnode->su_label != $3->astnode->su_label)
                        $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                     else $$->astnode->su_label = $1->astnode->su_label + 1;
                  }


relational_expression: additive_expression
                     {
                        $$ = new relational_expression_class();
                        $$->astnode = $1->astnode;
                     }
                     | relational_expression '<' additive_expression
                     {
                        $$ = new relational_expression_class();
                        op_binary_astnode* tmp = new op_binary_astnode();
                        tmp->exp1 = $1->astnode;
                        tmp->exp2 = $3->astnode;
                        // TODO : Type checking
                        std::string type_l = arr_to_ptr($1->astnode->type);
                        std::string type_r = arr_to_ptr($3->astnode->type);
                        bool throw_error = false;
                        if(type_l.compare("int") !=0 && type_l.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_r.compare("int") !=0 && type_r.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_l.compare(type_r)==0 && !get_deref_type(type_l).empty())
                        {
                           throw_error = false;
                        }
                        if(throw_error)
                        {
                           error(@1, "Invalid operand types for binary < , \"" + $1->astnode->type + "\" and \"" + $3->astnode->type +"\"");
                        }
                        if(tmp->exp1->type.compare("float") == 0)
                        {
                           if(tmp->exp2->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp2;
                              tmp_exp->type = "float";
                              tmp->exp2 = tmp_exp;
                           }
                        }
                        if(tmp->exp2->type.compare("float") == 0)
                        {
                           if(tmp->exp1->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp1;
                              tmp->exp1 = tmp_exp;
                           }
                        }

                        tmp->type = "int";
                        tmp->op = std::string("LT_OP_" + get_type($1->astnode, $3->astnode));
                        $$->astnode = tmp;  
                     if($1->astnode->su_label != $3->astnode->su_label)
                        $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                     else $$->astnode->su_label = $1->astnode->su_label + 1;

                     }
                     | relational_expression '>' additive_expression
                     {
                        $$ = new relational_expression_class();
                        op_binary_astnode* tmp = new op_binary_astnode();
                        tmp->exp1 = $1->astnode;
                        tmp->exp2 = $3->astnode;
                        // TODO : Type checking

                        std::string type_l = arr_to_ptr($1->astnode->type);
                        std::string type_r = arr_to_ptr($3->astnode->type);
                        bool throw_error = false;
                        if(type_l.compare("int") !=0 && type_l.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_r.compare("int") !=0 && type_r.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_l.compare(type_r)==0 && !get_deref_type(type_l).empty())
                        {
                           throw_error = false;
                        }
                        if(throw_error)
                        {
                           error(@1, "Invalid operand types for binary > , \"" + $1->astnode->type + "\" and \"" + $3->astnode->type +"\"");
                        }
                        if(tmp->exp1->type.compare("float") == 0)
                        {
                           if(tmp->exp2->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp2;
                              tmp_exp->type = "float";
                              tmp->exp2 = tmp_exp;
                           }
                        }
                        if(tmp->exp2->type.compare("float") == 0)
                        {
                           if(tmp->exp1->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp1;
                              tmp->exp1 = tmp_exp;
                           }
                        }
                        tmp->op = std::string("GT_OP_" + get_type($1->astnode, $3->astnode));
                        tmp->type = "int";
                        $$->astnode = tmp;   
                     if($1->astnode->su_label != $3->astnode->su_label)
                        $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                     else $$->astnode->su_label = $1->astnode->su_label + 1;

                     }
                     | relational_expression LE_OP additive_expression
                     {
                        $$ = new relational_expression_class();
                        op_binary_astnode* tmp = new op_binary_astnode();
                        tmp->exp1 = $1->astnode;
                        tmp->exp2 = $3->astnode;
                        std::string type_l = arr_to_ptr($1->astnode->type);
                        std::string type_r = arr_to_ptr($3->astnode->type);
                        bool throw_error = false;
                        if(type_l.compare("int") !=0 && type_l.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_r.compare("int") !=0 && type_r.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_l.compare(type_r)==0 && !get_deref_type(type_l).empty())
                        {
                           throw_error = false;
                        }
                        if(throw_error)
                        {
                           error(@1, "Invalid operand types for binary <= , \"" + $1->astnode->type + "\" and \"" + $3->astnode->type +"\"");
                        }
                        if(tmp->exp1->type.compare("float") == 0)
                        {
                           if(tmp->exp2->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp2;
                              tmp_exp->type = "float";
                              tmp->exp2 = tmp_exp;
                           }
                        }
                        if(tmp->exp2->type.compare("float") == 0)
                        {
                           if(tmp->exp1->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp1;
                              tmp->exp1 = tmp_exp;
                           }
                        }
                        // TODO : Type checking
                        tmp->op = std::string("LE_OP_" + get_type($1->astnode, $3->astnode));
                        
                        tmp->type = "int";
                        $$->astnode = tmp;
                     if($1->astnode->su_label != $3->astnode->su_label)
                        $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                     else $$->astnode->su_label = $1->astnode->su_label + 1;
                     }
                     | relational_expression GE_OP additive_expression
                     {
                        $$ = new relational_expression_class();
                        op_binary_astnode* tmp = new op_binary_astnode();
                        tmp->exp1 = $1->astnode;
                        tmp->exp2 = $3->astnode;
                        std::string type_l = arr_to_ptr($1->astnode->type);
                        std::string type_r = arr_to_ptr($3->astnode->type);
                        bool throw_error = false;
                        if(type_l.compare("int") !=0 && type_l.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_r.compare("int") !=0 && type_r.compare("float")!=0)
                        {
                           throw_error = true;
                        }
                        if(type_l.compare(type_r)==0 && !get_deref_type(type_l).empty())
                        {
                           throw_error = false;
                        }
                        if(throw_error)
                        {
                           error(@1, "Invalid operand types for binary >= , \"" + $1->astnode->type + "\" and \"" + $3->astnode->type +"\"");
                        }
                        if(tmp->exp1->type.compare("float") == 0)
                        {
                           if(tmp->exp2->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp2;
                              tmp_exp->type = "float";
                              tmp->exp2 = tmp_exp;
                           }
                        }
                        if(tmp->exp2->type.compare("float") == 0)
                        {
                           if(tmp->exp1->type.compare("int") == 0)
                           {
                              op_unary_astnode* tmp_exp = new op_unary_astnode();
                              tmp_exp->s = "TO_FLOAT";
                              tmp_exp->exp = tmp->exp1;
                              tmp->exp1 = tmp_exp;
                           }
                        }                        
                        tmp->op = std::string("GE_OP_" + get_type($1->astnode, $3->astnode));
                        tmp->type = "int";
                        $$->astnode = tmp;  
                     if($1->astnode->su_label != $3->astnode->su_label)
                        $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                     else $$->astnode->su_label = $1->astnode->su_label + 1;
                     }


additive_expression: multiplicative_expression
                     {
                        $$ = new additive_expression_class();
                        $$->astnode = $1->astnode;
                     }
                  | additive_expression '+' multiplicative_expression
                  {
                     $$ = new additive_expression_class();
                     op_binary_astnode* tmp = new op_binary_astnode();
                     tmp->exp1 = $1->astnode;
                     tmp->exp2 = $3->astnode;
                     bool is_float = false;
                     std::string type_l = $1->astnode->type;
                     std::string type_r = $3->astnode->type;
                     bool throw_error = false;
                     if(type_l.compare("int") !=0 && type_l.compare("float")!=0)
                     {
                        throw_error = true;
                     }
                     if(type_r.compare("int") !=0 && type_r.compare("float")!=0)
                     {
                        throw_error = true;
                     }
                     if(type_r.compare("int")==0 && !get_deref_type(type_l).empty())
                     {
                        throw_error = false;
                     }
                     if(type_l.compare("int")==0 && !get_deref_type(type_r).empty())
                     {
                        throw_error = false;
                     }
                     if(throw_error)
                     {
                           error(@1, "Invalid operand types for binary + , \"" + $1->astnode->type + "\" and \"" + $3->astnode->type +"\"");
                     }
                     if(tmp->exp1->type.compare("float") == 0)
                     {
                        if(tmp->exp2->type.compare("int") == 0)
                        {
                           op_unary_astnode* tmp_exp = new op_unary_astnode();
                           tmp_exp->s = "TO_FLOAT";
                           tmp_exp->exp = tmp->exp2;
                           tmp_exp->type = "float";
                           tmp->exp2 = tmp_exp;
                        }
                        is_float = true;
                     }
                     if(tmp->exp2->type.compare("float") == 0)
                     {
                        if(tmp->exp1->type.compare("int") == 0)
                        {
                           op_unary_astnode* tmp_exp = new op_unary_astnode();
                           tmp_exp->s = "TO_FLOAT";
                           tmp_exp->exp = tmp->exp1;
                           tmp->exp1 = tmp_exp;
                        }
                        is_float = true;
                     }
                     if(is_float)
                     {
                        tmp->type = "float";
                     }
                     else if(type_l.compare("int")==0 && !get_deref_type(type_r).empty())
                     {
                        tmp->type = type_r;
                     }
                     else if(type_r.compare("int")==0 && !get_deref_type(type_l).empty())
                     {
                        tmp->type = type_l;
                     }
                     else
                     {
                        tmp->type = "int";
                     }
                     tmp->op = "PLUS_" + get_type($1->astnode, $3->astnode);
                     $$->astnode = tmp;
                     if($1->astnode->su_label != $3->astnode->su_label)
                        $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                     else $$->astnode->su_label = $1->astnode->su_label + 1;
                  }
                  | additive_expression '-' multiplicative_expression
                  {
                     $$ = new additive_expression_class();
                     op_binary_astnode* tmp = new op_binary_astnode();
                     tmp->exp1 = $1->astnode;
                     tmp->exp2 = $3->astnode;

                     bool is_float = false;
                     std::string type_l = arr_to_ptr($1->astnode->type);
                     std::string type_r = arr_to_ptr($3->astnode->type);
                     bool throw_error = false;
                     if(type_l.compare("int") !=0 && type_l.compare("float")!=0)
                     {
                        throw_error = true;
                     }
                     if(type_r.compare("int") !=0 && type_r.compare("float")!=0)
                     {
                        throw_error = true;
                     }
                     if(type_r.compare("int")==0 && !get_deref_type(type_r).empty())
                     {
                        throw_error = false;
                     }
                     if((!get_deref_type(type_l).empty() && type_r.compare("int")==0) ||(!get_deref_type(type_r).empty() && type_l.compare("int")==0) )
                     {
                        throw_error = false;
                     }
                     if(!get_deref_type(type_l).empty() && type_l.compare(type_r)==0)
                     {
                        throw_error = false;
                     }
                     if(throw_error)
                     {
                           error(@1, "Invalid operand types for binary - , \"" + $1->astnode->type + "\" and \"" + $3->astnode->type +"\""+type_l+type_r);
                     }
                     if(tmp->exp1->type.compare("float") == 0)
                     {
                        if(tmp->exp2->type.compare("int") == 0)
                        {
                           op_unary_astnode* tmp_exp = new op_unary_astnode();
                           tmp_exp->s = "TO_FLOAT";
                           tmp_exp->exp = tmp->exp2;
                           tmp_exp->type = "float";
                           tmp->exp2 = tmp_exp;
                        }
                        is_float = true;
                     }
                     if(tmp->exp2->type.compare("float") == 0)
                     {
                        if(tmp->exp1->type.compare("int") == 0)
                        {
                           op_unary_astnode* tmp_exp = new op_unary_astnode();
                           tmp_exp->s = "TO_FLOAT";
                           tmp_exp->exp = tmp->exp1;
                           tmp->exp1 = tmp_exp;
                        }
                        is_float = true;
                     }
                     if(is_float)
                     {
                        tmp->type = "float";
                     }
                     else if(type_l.compare("int")==0 && !get_deref_type(type_r).empty())
                     {
                        tmp->type = type_r;
                     }
                     else if(type_r.compare("int")==0 && !get_deref_type(type_l).empty())
                     {
                        tmp->type = type_l;
                     }
                     else
                     {
                        tmp->type = "int";
                     }   
                     tmp->op = "MINUS_" + get_type($1->astnode, $3->astnode);
                     $$->astnode = tmp;
                     if($1->astnode->su_label != $3->astnode->su_label)
                        $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                     else $$->astnode->su_label = $1->astnode->su_label + 1;
                  }

unary_expression: postfix_expression
                  {
                     $$ = new unary_expression_class();
                     $$->astnode = $1->astnode;
                  }
               | unary_operator unary_expression
               {
                  $$ = new unary_expression_class();
                  op_unary_astnode* tmp = new op_unary_astnode();
                  tmp->exp = $2->astnode;
                  tmp->s = $1->op;
                  std::string curr_type = $2->astnode->type;
                  if($1->op.compare("UMINUS") == 0)
                  {
                     if(!(curr_type.compare("float") == 0 || curr_type.compare("int") == 0))
                     {
                        error(@2, "Operand of unary - should be an int or float");
                     }
                  }
                  if($1->op.compare("DEREF") == 0)
                  {
                     std::string deref_type = get_deref_type(curr_type);
                     if(deref_type.empty() || deref_type=="void")
                     {
                        error(@2, "Invalid operand type \"" + curr_type + "\" of unary *");
                     }
                     else
                     {
                        curr_type = deref_type;
                     }
                     tmp->is_lval = true;
                  }
                  if($1->op.compare("ADDRESS") == 0)
                  {
                     // cout<<"before add "<<curr_type<<endl;
                     std::string ref_type = get_ref_type(curr_type);
                     curr_type = ref_type;
                  }
                  if($1->op.compare("NOT") == 0)
                  {
                     curr_type = "int";
                  }
                  tmp->su_label = $2->astnode->su_label;
                  tmp->type = curr_type;
                  $$->astnode = tmp;
               }

multiplicative_expression: unary_expression
                        {
                           $$ = new multiplicative_expression_class();
                           $$->astnode = $1->astnode;
                        }
                        | multiplicative_expression '*' unary_expression
                        {
                           $$ = new multiplicative_expression_class();
                           op_binary_astnode* tmp = new op_binary_astnode();
                           tmp->exp1 = $1->astnode;
                           tmp->exp2 = $3->astnode;
                           bool is_float = false;
                           if(tmp->exp1->type.compare("float") == 0)
                           {
                              if(tmp->exp2->type.compare("int") == 0)
                              {
                                 op_unary_astnode* tmp_exp = new op_unary_astnode();
                                 tmp_exp->s = "TO_FLOAT";
                                 tmp_exp->exp = tmp->exp2;
                                 tmp_exp->type = "float";
                                 tmp->exp2 = tmp_exp;
                              }
                              is_float = true;
                           }
                           if(tmp->exp2->type.compare("float") == 0)
                           {
                              if(tmp->exp1->type.compare("int") == 0)
                              {
                                 op_unary_astnode* tmp_exp = new op_unary_astnode();
                                 tmp_exp->s = "TO_FLOAT";
                                 tmp_exp->exp = tmp->exp1;
                                 tmp->exp1 = tmp_exp;
                              }
                              is_float = true;
                           }
                           if(is_float)
                           {
                              tmp->type = "float";
                           }
                           else
                           {
                              tmp->type = "int";
                           }
                           tmp->op = "MULT_" + get_type($1->astnode, $3->astnode);
                           $$->astnode = tmp;
                           if($1->astnode->su_label != $3->astnode->su_label)
                              $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                           else $$->astnode->su_label = $1->astnode->su_label + 1;
                        }
                        | multiplicative_expression '/' unary_expression
                        {
                           $$ = new multiplicative_expression_class();
                           op_binary_astnode* tmp = new op_binary_astnode();
                           tmp->exp1 = $1->astnode;
                           tmp->exp2 = $3->astnode;
                           bool is_float = false;
                           if(tmp->exp1->type.compare("float") == 0)
                           {
                              if(tmp->exp2->type.compare("int") == 0)
                              {
                                 op_unary_astnode* tmp_exp = new op_unary_astnode();
                                 tmp_exp->s = "TO_FLOAT";
                                 tmp_exp->exp = tmp->exp2;
                                 tmp_exp->type = "float";
                                 tmp->exp2 = tmp_exp;
                              }
                              is_float = true;
                           }
                           if(tmp->exp2->type.compare("float") == 0)
                           {
                              if(tmp->exp1->type.compare("int") == 0)
                              {
                                 op_unary_astnode* tmp_exp = new op_unary_astnode();
                                 tmp_exp->s = "TO_FLOAT";
                                 tmp_exp->exp = tmp->exp1;
                                 tmp->exp1 = tmp_exp;
                              }
                              is_float = true;
                           }
                           if(is_float)
                           {
                              tmp->type = "float";
                           }
                           else
                           {
                              tmp->type = "int";
                           }
                           tmp->op = "DIV_" + get_type($1->astnode, $3->astnode);
                           $$->astnode = tmp;
                           $$->astnode->su_label = max($1->astnode->su_label, $3->astnode->su_label);
                           $$->astnode->su_label = max($$->astnode->su_label, 2);
                           
                        }

postfix_expression: primary_expression
                     {
                        $$ = new postfix_expression_class();
                        $$->astnode = $1->astnode;
                     }
                  | postfix_expression '[' expression ']'
                  {
                     if($3->astnode->type.compare("int")!=0)
                     {
                        error(@3, "Array subscript is not an integer");
                     }
                     std::string deref_type = get_deref_type($1->astnode->type);
                     if(deref_type.empty())
                     {
                        error(@1, "Subscripted value is neither array nor pointer");
                     }

                     $$ = new postfix_expression_class();
                     arrayref_astnode* tmp = new arrayref_astnode();
                     tmp->index = $3->astnode;
                     tmp->array = $1->astnode;
                     tmp->type = deref_type;

                     tmp->is_lval = true;
                     
                     $$->astnode = tmp;
                  }
                  | IDENTIFIER '(' ')'
                  {
                     bool is_predefined = false;
                     if(gst.Entries.find($1) == gst.Entries.end() && std::find(predefined.begin(), predefined.end(), $1)==predefined.end())
                     {
                        error(@1, "Function \"" + $1 + "\" not declared");
                     }
                     if(std::find(predefined.begin(), predefined.end(), $1)!=predefined.end())
                     {
                        is_predefined = true;
                     }
                        int arg_count = 0;
                     if(!is_predefined)
                     {
                        for(auto& it : gst.Entries[$1]->localSBT->Entries)
                        {
                           if(it.second->scp == PARAM)
                              arg_count++;
                        }
                        if(arg_count > 0)
                        {
                           error(@1, "Procedure \"" + $1 +  "\"  called with too few arguments");
                        }              
                     }
                     $$ = new postfix_expression_class();
                     funcall_astnode* tmp = new funcall_astnode();
                     tmp->fname = new identifier_astnode();
                     tmp->fname->id = $1;
                     // TODO : check presence
                     tmp->type = gst.Entries[$1]->return_var_type;
                     $$->astnode = tmp;
                  }
                  | IDENTIFIER '(' expression_list ')'
                  {
                     bool is_predefined = false;
                     if(gst.Entries.find($1) == gst.Entries.end() && std::find(predefined.begin(), predefined.end(), $1)==predefined.end())
                     {
                        error(@1, "Function \"" + $1 + "\" not declared");
                     }
                     unsigned long int arg_count = 0;
                     vector<pair<int, string> > arguments;

                     if(std::find(predefined.begin(), predefined.end(), $1)!=predefined.end())
                     {
                        is_predefined = true;
                     }
                     if(!is_predefined)
                     {
                        for(auto& it : gst.Entries[$1]->localSBT->Entries)
                        {
                           if(it.second->scp == PARAM)
                           {
                              arg_count++;
                              arguments.push_back(make_pair(it.second->offset, it.second->return_var_type));
                           }
                           sort(arguments.begin(), arguments.end(),[](pair<int,string>p1, pair<int, string>p2){return p1<p2;});
                        }
                        if(arg_count > $3->vec_exp_astnode.size())
                        {
                           error(@1, "Procedure \"" + $1 +  "\"  called with too few arguments");
                        }
                        if(arg_count < $3->vec_exp_astnode.size())
                        {
                           error(@1, "Procedure \"" + $1 +  "\"  called with too many arguments");
                        }
                        for(int i=0; i<arguments.size(); i++)
                        {
                           if(!check_compatibility($3->vec_exp_astnode[i], arguments[i].second))
                           {
                              error(@3, "Expected \"" + arguments[i].second + "\" but argument is of type \""+$3->vec_exp_astnode[i]->type+"\"");
                           }
                        }          
                     }
                     $$ = new postfix_expression_class();
                     funcall_astnode* tmp = new funcall_astnode();
                     tmp->fname = new identifier_astnode();
                     tmp->fname->id = $1;
                     tmp->params = $3->vec_exp_astnode;
                     // TODO : check expression_list and existence
                     tmp->type = gst.Entries[$1]->return_var_type;
                     $$->astnode = tmp;

                  }
                  | postfix_expression '.' IDENTIFIER
                  {
                     $$ = new postfix_expression_class();
                     member_astnode* tmp = new member_astnode();
                     tmp->ref = $1->astnode;
                     tmp->id = new identifier_astnode();
                     tmp->id->id = $3;
                     //check valid postfix_exp and existence
                     std::string tmp_type = $1->astnode->type;
                     std::string struct_str("struct");
                     if(tmp_type.compare(0, struct_str.size(), struct_str) != 0 || tmp_type.back() == '*' || tmp_type.back() == ']')
                     {
                        error(@1, "Left operand of \".\"  is not a  structure");
                     }
                     if(gst.Entries[$1->astnode->type]->localSBT->Entries.find($3) == gst.Entries[$1->astnode->type]->localSBT->Entries.end())
                     {
                        error(@3, ("Struct \"" + $1->astnode->type) + "\" has no member named \"" + ($3 + "\""));
                     }
                     
                     tmp->type = gst.Entries[$1->astnode->type]->localSBT->Entries[$3]->return_var_type;

                     tmp->is_lval = true;
                     $$->astnode = tmp;
                  }
                  | postfix_expression PTR_OP IDENTIFIER
                  {
                     $$ = new postfix_expression_class();
                     arrow_astnode* tmp = new arrow_astnode();
                     tmp->arrow = $1->astnode;
                     tmp->id = new identifier_astnode();
                     tmp->id->id = $3;
                     //check valid postfix_exp and existence
                     std::string tmp_type = arr_to_ptr($1->astnode->type);
                     $1->astnode->type = tmp_type;
                     std::string struct_str("struct");
                     tmp_type = arr_to_ptr(tmp_type);

                     if(tmp_type.compare(0, struct_str.size(), struct_str) != 0)
                     {
                        error(@1, "Left operand of \"->\" is not a pointer to structure");
                     }


                     // $1->astnode->type.erase($1->astnode->type.begin()+(tmp_type.size()-1));
                     std::string deref_type = get_deref_type(tmp_type);
                     if(deref_type.empty())
                     {
                        deref_type = tmp_type;
                     }
                     else if(!get_deref_type(deref_type).empty())
                     {
                        error(@1, "Left operand of \"->\" is not a pointer to structure"+tmp_type+struct_str);
                     }
                     $1->astnode->type = deref_type;
                     if(gst.Entries[$1->astnode->type]->localSBT->Entries.find($3) == gst.Entries[$1->astnode->type]->localSBT->Entries.end())
                     {
                        error(@3, ("Struct \"" + $1->astnode->type) + "\" has no member named \"" + ($3 + "\""));
                     }

                     tmp->type = gst.Entries[$1->astnode->type]->localSBT->Entries[$3]->return_var_type;

                     tmp->is_lval = true;
                     
                     $$->astnode = tmp;
                  }
                  | postfix_expression INC_OP
                  {
                     $$ = new postfix_expression_class();
                     op_unary_astnode* tmp = new op_unary_astnode();
                     tmp->s = "PP";
                     if(!$1->astnode->is_lval || ($1->astnode->type != "int" && $1->astnode->type != "float" && get_deref_type($1->astnode->type).empty()))
                     {
                        error(@1, "Operand of \"++\" should have lvalue");
                     }
                     tmp->exp = $1->astnode;
                     tmp->type = $1->astnode->type;
                     $$->astnode = tmp;
                     $$->astnode->su_label = 2;
                  }

primary_expression: IDENTIFIER
                  {
                     if(localst->Entries.find($1) == localst->Entries.end())
                     {
                        error(@1, "Variable \"" + $1 + "\" not declared");
                     }
                     $$ = new primary_expression_class();
                     identifier_astnode* tmp = new identifier_astnode();
                     tmp->id = $1;
                     // add scoping error here
                     tmp->type = localst->Entries[$1]->return_var_type;
                     tmp->is_lval = true;
                     tmp->su_label = 1;
                     $$->astnode = tmp; 
                  }
                  | INT_CONSTANT
                  {
                     $$ = new primary_expression_class();
                     intconst_astnode* tmp = new intconst_astnode();
                     tmp->s = atoi($1.c_str());
                     if(tmp->s == 0)
                     {
                        tmp->is_zero = true;
                     }
                     tmp->type = "int";
                     $$->astnode = tmp;
                     tmp->su_label = 1;
               
                  }
                  | FLOAT_CONSTANT
                  {
                     $$ = new primary_expression_class();
                     floatconst_astnode* tmp = new floatconst_astnode();
                     tmp->f = atof($1.c_str());
                     tmp->type = "float";
                     $$->astnode = tmp; 
                  }
                  | STRING_LITERAL
                  {
                     $$ = new primary_expression_class();
                     stringconst_astnode* tmp = new stringconst_astnode();
                     tmp->s = $1;
                     tmp->type = "string_literal";
                     $$->astnode = tmp;
                  }
                  | '(' expression ')'
                  {
                     $$ = new primary_expression_class();
                     $$->astnode = $2->astnode;
                  }

expression_list: expression 
               {
                  $$ = new expression_list_class();
                  $$->vec_exp_astnode.push_back($1->astnode);
               }
               | expression_list ',' expression 
               {
                  $$ = new expression_list_class();
                  $$->vec_exp_astnode = $1->vec_exp_astnode;
                  $$->vec_exp_astnode.push_back($3->astnode);
               }

unary_operator: '-'
            {
               $$ = new unary_operator_class();
               $$->op = "UMINUS";
            }
            | '!'
            {
               $$ = new unary_operator_class();
               $$->op = "NOT";
            }
            | '&'
            {
               $$ = new unary_operator_class();
               $$->op = "ADDRESS";
            }
            | '*'
            {
               $$ = new unary_operator_class();
               $$->op = "DEREF";
            }

selection_statement: IF '(' expression ')' statement ELSE statement
                  {
                     $$ = new selection_statement_class();
                     if_astnode* tmp = new if_astnode();
                     tmp->condition = $3->astnode;
                     tmp->then = $5->astnode;
                     tmp->exp = $7->astnode;
                     $$->astnode = tmp;
                  }

iteration_statement: WHILE '(' expression ')' statement
                  {
                     while_astnode* tmp_while = new while_astnode;
                     tmp_while->condition = $3->astnode;
                     tmp_while->exp = $5->astnode;

                     $$ = new iteration_statement_class();
                     $$->astnode = tmp_while;
                  }
                  | FOR '(' assignment_expression ';' expression ';' assignment_expression ')' statement
                  {

                     for_astnode* tmp_for = new for_astnode();
                     tmp_for->exp = $3->astnode;
                     tmp_for->guard = $5->astnode;
                     tmp_for->step = $7->astnode;
                     tmp_for->body = $9->astnode;
                     $$ = new iteration_statement_class();
                     // std::cout << "trying to print for\n";
                     // tmp_for->print(0);

                     $$->astnode = tmp_for;
                     // (*($$->astnode)).print(0);
                     //std::cout<<$$->astnode->astnode_type<<std::endl;
                  }

declaration_list: declaration
               {
                  $$ = new declaration_list_class();
               }
               | declaration_list declaration
               {
                  $$ = new declaration_list_class();
               }

declaration: type_specifier 
            declarator_list ';'
               {
               type = ""; basictype = "";
               }

declarator_list: declarator
               {
                  int factor = 1;
                  int tmp_sz = 1;

                  if(type[0] == '*')
                  {
                     factor = POINTER_SZ;
                  }
                  else
                  {
                     if(basictype.compare("int")==0) factor = 4;
                     else if(basictype.compare("float")==0) factor = 4;
                     else if(basictype.compare("void")==0) factor = 4;
                     else if(gst.Entries.find(basictype) != gst.Entries.end())
                     {
                        SymbTab_entry* tmp_factor_e = gst.Entries[basictype];
                        factor = tmp_factor_e->sz;
                        //cout<<"fetching the size of struct";
                     }
                     else
                     {
                        std::cout<<"No type exists"<<std::endl;
                        exit(1);
                     }
                  }
                  tmp_sz = $1->size*factor;
                  if((basictype+type).compare("void")==0) error(@$, "Cannot declare variable of type \"void\"");
                  if(!is_struct) offset -= tmp_sz;
                  SymbTab_entry* tmp_e = new SymbTab_entry($1->name, LOCAL, "var", tmp_sz, offset, basictype + type, NULL);

                  localst->Entries.insert({$1->name, tmp_e});
                  if(is_struct) offset += tmp_sz;
                  type = "";
               }
               | declarator_list ','
                declarator
                {
                  int factor = 1;
                  int tmp_sz = 1;

                  if(type[0] == '*')
                  {
                     factor = POINTER_SZ;
                  }
                  else
                  {
                     if(basictype.compare("int")==0) factor = 4;
                     else if(basictype.compare("float")==0) factor = 4;
                     else if(basictype.compare("void")==0) factor = 4;
                     else if(gst.Entries.find(basictype) != gst.Entries.end())
                     {
                        SymbTab_entry* tmp_factor_e = gst.Entries[basictype];
                        factor = tmp_factor_e->sz;
                     }
                     else
                     {
                        std::cout<<"No type exists"<<std::endl;
                        exit(1);
                     }
                  }
                  tmp_sz = $3->size*factor;
                  if((basictype+type).compare("void")==0) error(@$, "Cannot declare variable of type \"void\"");
                  if(!is_struct) offset -= tmp_sz;
                  SymbTab_entry* tmp_e = new SymbTab_entry($3->name, LOCAL, "var", tmp_sz, offset, basictype + type, NULL);
                  localst->Entries.insert({$3->name, tmp_e});
                  if(is_struct) offset += tmp_sz;
                  type = "";

                }
printf_call: 
   PRINTF '(' STRING_LITERAL ')' ';'
  {
     $$ = new printf_call_class();
      printcall_astnode* tmp = new printcall_astnode();
      tmp->fname = new identifier_astnode();
      tmp->fname->id = $1;
      vector<exp_astnode*> params ;
      stringconst_astnode* strtmp = new stringconst_astnode();
      strtmp->s = $3;
      params.push_back(strtmp);
      tmp->params = params;

      $$->astnode = tmp;
  }
  | PRINTF '(' STRING_LITERAL ',' expression_list ')' ';'
  {

      $$ = new printf_call_class();
      printcall_astnode* tmp = new printcall_astnode();
      stringconst_astnode* strtmp = new stringconst_astnode();
      strtmp->s = $3;
      tmp->fname = new identifier_astnode();       
      tmp->fname->id = $1;
      $5->vec_exp_astnode.push_back(strtmp);
      tmp->params = $5->vec_exp_astnode;

      $$->astnode = tmp;

  }

%%
void IPL::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cout << "Error at line " << l.end.line << ": " << err_message<<std::endl;
   exit(1);
}


