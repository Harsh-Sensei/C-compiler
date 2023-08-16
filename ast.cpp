#include "ast.hh"


static bool is_array_ref = false;

bool is_array_type(string type)
{
    // cout << "array type checking " << type << endl;
    long unsigned int found;
    found = type.find('[');
    if(found == string::npos)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int get_size(string type)
{
    int mult = 1;
    long unsigned int found;
    while(true)
    {      
        found = type.find('[');
        if(found == string::npos)
        {
            return mult;
        }
        int found2 = type.find(']');
        int num = stoi(type.substr(found+1, found2-found-1));
        type.erase(found, found2-found+1);
        mult *= num;
    } 
}

std::string get_base_type(string type)
{
    long unsigned int found;
    found = type.find('(');
    if(found == string::npos)
    {
        found = type.find('[');
        return type.substr(0, found);
    }
    return type.substr(0, found);
}


std::string get_deref_type_ast(std::string type)
{
	long unsigned int found;
	found = type.find('(');
	if(found == std::string::npos)
	{
		found = type.find('[');
		if(found == std::string::npos)
		{
			found = type.find('*');
			if(found == std::string::npos)
			{
				return std::string();
			}
			else
			{
				type.erase(type.begin() + found);
				return type;
			}
		}else
		{
			unsigned long int found2 = type.find(']');
			type.erase(found, found2-found+1);
			return type;
		}
	
		return type;
	}
	unsigned long int found2 = type.find(')');
	type.erase(found, found2-found+1);
	return type;
}

int get_array_multiplier(string type)
{
    int mult = 1;
    long unsigned int found;
    // cout << " multiplier type " << type << endl;
    string deref_type = get_deref_type_ast(type);
    while(true)
    {      
        found = deref_type.find('[');
        if(found == string::npos)
        {
            return mult;
        }
        int found2 = deref_type.find(']');
        int num = stoi(deref_type.substr(found+1, found2-found-1));
        deref_type.erase(found, found2-found+1);
        mult *= num;
    }
}


string get_operator(string op)
{
    if (op == "PLUS_INT")
    {
        return string("addl");
    }
    else if (op == "MINUS_INT")
    {
        return string("subl");
    }
    else if (op == "MULT_INT")
    {
        return string("imull");
    }
    else if(op == "DIV_INT")
    {
        return string("idivl");
    }
}

int get_exp_offset(exp_astnode* exp, SymbTab* st)
{
    if(exp->astnode_type == e_identifier_astnode)
    {
        identifier_astnode* iden_astnode = dynamic_cast<identifier_astnode*>(exp); 
        return st->Entries[iden_astnode->id]->offset;
    }
    else if(exp->astnode_type == e_member_astnode)
    {
        member_astnode* mem_astnode = dynamic_cast<member_astnode*>(exp);
        exp_astnode* ref = mem_astnode->ref;
        string id = mem_astnode->id->id;
        string type = ref->type;

        int local_offset = gst.Entries[type]->localSBT->Entries[id]->offset;
        int ref_offset = get_exp_offset(ref, st);
        return ref_offset + local_offset;
    }
    else if(exp->astnode_type == e_arrow_astnode)
    {
        arrow_astnode* arr_astnode = dynamic_cast<arrow_astnode*>(exp);
        exp_astnode* ref = arr_astnode->arrow;
        string id = arr_astnode->id->id;
        string type = ref->type;
        int local_offset = gst.Entries[type]->localSBT->Entries[id]->offset;
        int ref_offset = get_exp_offset(ref, st);
        return ref_offset + local_offset;
    }
    else
    {
        cout<<"Invalid astnode"<<endl;
        exit(1);
    }

    return 0;
}

string get_mem_loc(exp_astnode* exp, SymbTab* st)
{
    
    int x = get_exp_offset(exp, st);
    return to_string(x) + "(%ebp)";
}

void get_mem_loc_reg(exp_astnode* exp, SymbTab* st)
{
    string type = exp->type;
    // if(type.find('*')!=string::npos) cout << " found pointer " << type << " " << is_array_ref<<endl;
    if(exp->astnode_type == e_identifier_astnode)
    {
        // cout <<"iden" << endl;
        identifier_astnode* iden_astnode = dynamic_cast<identifier_astnode*>(exp); 
        string type = iden_astnode->type;
        if(is_array_type(type) && st->Entries[iden_astnode->id]->scp == PARAM)
        {
            gencode.gen("\tleal\t"+to_string(st->Entries[iden_astnode->id]->offset) + "(%ebp), " + rstack.back());
            gencode.gen("\tmovl\t(" + rstack.back() + "), " + rstack.back());
        }
        else if (is_array_ref && type.find('*')!=string::npos)
        {
            gencode.gen("\tleal\t"+to_string(st->Entries[iden_astnode->id]->offset) + "(%ebp), " + rstack.back());
            gencode.gen("\tmovl\t(" + rstack.back() + "), " + rstack.back());
            is_array_ref = false;
        }
        else
        {
            gencode.gen("\tleal\t"+to_string(st->Entries[iden_astnode->id]->offset) + "(%ebp), " + rstack.back());
        }
    }
    else if(exp->astnode_type == e_member_astnode)
    {
        // cout <<"mem" << endl;

        member_astnode* mem_astnode = dynamic_cast<member_astnode*>(exp);
        exp_astnode* ref = mem_astnode->ref;
        string id = mem_astnode->id->id;
        string type = ref->type;

        get_mem_loc_reg(ref, st);
        int local_offset = gst.Entries[type]->localSBT->Entries[id]->offset;

        gencode.gen("\tleal\t" + to_string(local_offset) +"(" + rstack.back() + "), " + rstack.back());
    }
    else if(exp->astnode_type == e_arrow_astnode)
    {
        // cout <<"arrow" << endl;

        arrow_astnode* arr_astnode = dynamic_cast<arrow_astnode*>(exp);
        exp_astnode* ref = arr_astnode->arrow;
        string id = arr_astnode->id->id;
        string type = ref->type;
        ref->gencode_node(st);
        int local_offset = gst.Entries[type]->localSBT->Entries[id]->offset;
        gencode.gen("\tleal\t" + to_string(local_offset) + "(" + rstack.back() + "), " + rstack.back());
    }
    else if(exp->astnode_type == e_arrayref_astnode)
    {
        arrayref_astnode* arr_astnode = dynamic_cast<arrayref_astnode*>(exp);
        arr_astnode->index->gencode_node(st);
        // cout<<"arref type "<< arr_astnode->array->type <<endl;
        int mult = get_array_multiplier(arr_astnode->array->type);
        // cout << "multiplier " << mult << endl;
        string base_type = get_base_type(arr_astnode->type);
        if(base_type[base_type.size()-1] == '*' || base_type == "int")
        {
            mult *= 4;
        }
        else
        {
            mult *= gst.Entries[base_type]->sz;
        }
        gencode.gen("\timull\t$" + to_string(mult) + ", " + rstack.back());
        gencode.gen("\tpushl\t" + rstack.back());
        is_array_ref = true;
        get_mem_loc_reg(arr_astnode->array, st);
        gencode.gen("\taddl\t(%esp), " + rstack.back());
        gencode.gen("\taddl\t$4, %esp");
    }
    else if (exp->astnode_type == e_op_unary_astnode)
    {
        op_unary_astnode* unary_astnode = dynamic_cast<op_unary_astnode*>(exp);
        assert(unary_astnode->s == "DEREF");
        unary_astnode->exp->gencode_node(st);
    }
    else
    {
        assert(0);
    }
}


void empty_astnode::gencode_node(SymbTab* st)
{

}

void seq_astnode::gencode_node(SymbTab* st)
{
    for(auto it=this->vec_statement_astnode.begin(); it != this->vec_statement_astnode.end(); it++)
    {
        (*it)->gencode_node(st);
    }
}

void assignS_astnode::gencode_node(SymbTab* st)
{
    right->gencode_node(st);
    // identifier_astnode* tmp = dynamic_cast<identifier_astnode*>(left);
    string rtop = rstack.back();
    // cout<<"all good" << endl;
    rstack.pop_back();
    // cout<<"all good" << endl;

    get_mem_loc_reg(left, st);
    // cout<<"all good" << endl;

    gencode.gen("\tmovl\t"+rtop+", (" + rstack.back() + ")");
    // cout<<"all good" << endl;

    rstack.push_back(rtop);
}

void return_astnode::gencode_node(SymbTab* st)
{
    return_val->gencode_node(st);
    gencode.gen("\tmovl\t"+rstack.back()+", %eax");
    gencode.gen("\tleave");
    gencode.gen("\tret");
}

void if_astnode::gencode_node(SymbTab* st)
{
    condition->gencode_node(st);
    gencode.gen("\tcmpl\t$0, " + rstack.back());
    int jump1 = gencode.jump();
    gencode.gen("\tje\t" + string(".L") + to_string(jump1));
    then->gencode_node(st);
    int jump2 = gencode.jump();
    gencode.gen("\tjmp\t" + string(".L") + to_string(jump2));
    gencode.gen(".L" + to_string(jump1) + ":");
    exp->gencode_node(st);
    gencode.gen(".L" + to_string(jump2) + ":");
}

void while_astnode::gencode_node(SymbTab* st)
{

    int j1 = gencode.jump();
    gencode.gen(".L" + to_string(j1) + ":");
    condition->gencode_node(st);
    gencode.gen("\tcmpl\t$0, " + rstack.back());
    int j2 = gencode.jump();
    gencode.gen("\tje\t.L" + to_string(j2));
    exp->gencode_node(st);
    gencode.gen("\tjmp\t.L" + to_string(j1));
    gencode.gen(".L" + to_string(j2) + ":");
}

void for_astnode::gencode_node(SymbTab* st)
{
    exp->gencode_node(st);
    int j1 = gencode.jump();
    gencode.gen(".L" + to_string(j1) + ":");
    guard->gencode_node(st);
    gencode.gen("\tcmpl\t$0, " + rstack.back());
    int j2 = gencode.jump();
    gencode.gen("\tje\t.L"+to_string(j2));  
    body->gencode_node(st);
    step->gencode_node(st);
    gencode.gen("\tjmp\t.L" + to_string(j1));
    gencode.gen(".L" + to_string(j2) + ":");
}

void proccall_astnode::gencode_node(SymbTab* st)
{
    // for(auto elem : all_reg)
    // {
    //     gencode.gen("\tpushl\t"+elem);
    // }
    bool need_swap = false;
    if(rstack.back()!="%eax")
    {
        need_swap = true;
        swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
    }  


    int offset = 4;
    for(int i = params.size()-1; i>=0; i--)
    {  
        if(is_array_type(params[i]->type))
        {
            get_mem_loc_reg(params[i], st);
            gencode.gen("\tpushl\t" + rstack.back());
            offset += 4;
        }
        else
        {       
            params[i]->gencode_node(st);
            gencode.gen("\tpushl\t" + rstack.back());
            offset += 4;
        }
    }
    gencode.gen("\tsubl\t$4, %esp");
    gencode.gen(string("\tcall ")+fname->id);
    gencode.gen("\taddl\t$" + to_string(offset) + ", %esp");
    if(need_swap)
    {
        gencode.gen("\tmovl\t%eax, %ebx");
        swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
    }
}

void ref_astnode::gencode_node(SymbTab* st)
{

}

void identifier_astnode::gencode_node(SymbTab* st)
{
    gencode.gen("\tmovl\t" + get_mem_loc(this, st) + ", " + rstack.back());
}

void arrayref_astnode::gencode_node(SymbTab* st)
{
    is_array_ref = true;
    get_mem_loc_reg(array, st);
    gencode.gen("\tpushl\t" + rstack.back());
    index->gencode_node(st);
    int mult = get_array_multiplier(array->type);
    string base_type = get_base_type(this->type);
    if(base_type[base_type.size()-1] == '*' || base_type == "int")
    {
        mult *= 4;
    }
    else
    {
        mult *= gst.Entries[base_type]->sz;
    }
    gencode.gen("\timull\t$" + to_string(mult) + ", " + rstack.back());
    gencode.gen("\taddl\t" + rstack.back() + ", " + "(%esp)");
    gencode.gen("\tpopl\t" + rstack.back());
    gencode.gen("\tmovl\t(" + rstack.back() + "), " + rstack.back());
}

void member_astnode::gencode_node(SymbTab* st)
{
    string struct_type = ref->type;
    string member_id = id->id;
    int mem_offset = gst.Entries[struct_type]->localSBT->Entries[member_id]->offset;
    get_mem_loc_reg(ref, st);
    gencode.gen("\tmovl\t" + to_string(mem_offset) + "(" + rstack.back() + ")" + ", " + rstack.back());
}

void arrow_astnode::gencode_node(SymbTab* st)
{
    string struct_type = arrow->type;
    string member_id = id->id;
    int mem_offset = gst.Entries[struct_type]->localSBT->Entries[member_id]->offset;
    arrow->gencode_node(st);
    gencode.gen("\tmovl\t" + to_string(mem_offset) + "(" + rstack.back() + ")" + ", " + rstack.back());
}

void op_binary_astnode::gencode_node(SymbTab* st)
{
    int r = exp2->su_label;
    int l = exp1->su_label;
    // cout<< op << l << r << rstack.size() << endl;
    // cout<< " top " << rstack.back() <<endl;
    if(!get_deref_type_ast(exp1->type).empty() && exp2->type == "int" && (op == "PLUS_INT" || op == "MINUS_INT"))
    {
        exp1->gencode_node(st);
        string rtop = rstack.back();
        int mult = get_size(exp1->type);
        string base_type = get_base_type(exp1->type);
        int base_size = 1;
        // cout << "base_type "<< base_type<<endl;
        if(base_type.find('*') != string::npos)
        {
            base_size = 4;
        }
        else if(base_type == "int")
        {
            base_size = 4;
        }
        else if(st->Entries.find(base_type) != st->Entries.end())
        {
            base_size = st->Entries[base_type]->sz;
        }
        else
        {
            assert(0);
        }
        mult *= base_size;
        string opcode = get_operator(op);
        gencode.gen("\t" + opcode +"\t$" + to_string(mult) + ", " + rstack.back());
        return;
    }
    if (op == "PLUS_INT" || op == "MULT_INT" || op == "MINUS_INT")
    {

        string opcode = get_operator(op);

        if(l<TOTAL_REG  && r<TOTAL_REG)
        {   
            if (r<=l)
            {
                exp1->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp2->gencode_node(st);

                gencode.gen("\t" + opcode + "\t" + rstack.back() + ", " + rtop);
                rstack.push_back(rtop);
            }
            else 
            {
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
                exp2->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp1->gencode_node(st);
                gencode.gen("\t"+opcode+"\t" + rtop + ", " + rstack.back());
                rstack.push_back(rtop);
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]); 
            }
            
        }
        else if(r >= TOTAL_REG && l>= TOTAL_REG)
        {
            // cout<< "pop mode" << endl;
            exp2->gencode_node(st);
            gencode.gen("\tpushl\t"+rstack.back());
            exp1->gencode_node(st);
            gencode.gen("\t"+opcode+"\t" + "(%esp), " + rstack.back());
            gencode.gen("\taddl\t$4, %esp");
        }
        else if(l < r)
        {
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
            exp2->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp1->gencode_node(st);
            gencode.gen("\t"+opcode+"\t" + rtop + ", " + rstack.back());
            rstack.push_back(rtop);
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        }
        else if(r < l)
        {
            exp1->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp2->gencode_node(st);
            gencode.gen("\t"+opcode+"\t" + rstack.back() + ", " +rtop);
            rstack.push_back(rtop);
        }
    }
    else if(op == "DIV_INT")
    {
        bool remove = false;
        bool do_swap = false;
        string rtop = rstack.back();
        string alt = rstack[0];
        int sz = rstack.size();
        // cout<<"div top "<< rtop<<endl;
        gencode.gen("\tmovl\t$0, %edx");
        if(find(rstack.begin(), rstack.end(), string("%eax")) == rstack.end())
        {   
            remove = true;
            // cout <<"could not find in rstack"<<endl;
            gencode.gen("\tpushl\t%eax");
            rstack.push_back("%eax");
        }
        else if(rstack.back() != "%eax")
        {
            do_swap = true;
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        }
        swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        exp2->gencode_node(st);
        gencode.gen("\tpushl\t"+rstack.back());
        swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        exp1->gencode_node(st);
        gencode.gen("\tpopl\t" + rstack[rstack.size()-2]);
        gencode.gen("\tcltd");
        gencode.gen("\tidivl\t" + rstack[rstack.size()-2]);
        gencode.gen("\tmovl\t%eax, " + rtop);
        if(do_swap)
        {
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        
        }
        if(remove) 
        {
            gencode.gen("\tpopl\t%eax");
            rstack.pop_back();
        }
        

    }
    else if(op == "AND_OP")
    {
        exp1->gencode_node(st);
        string rtop = rstack.back();
        gencode.gen("\tcmpl\t$0, "+rtop);
        int j1 = gencode.jump();
        gencode.gen("\tje\t.L" + to_string(j1));
        exp2->gencode_node(st);
        gencode.gen("\tcmpl\t$0, "+rstack.back());
        gencode.gen("\tje\t.L" + to_string(j1));
        gencode.gen("\tmovl\t$1, " + rstack.back());
        int j2 = gencode.jump();
        gencode.gen("\tjmp\t.L" + to_string(j2));
        gencode.gen(".L" + to_string(j1) + ":");
        gencode.gen("\tmovl\t$0, " + rstack.back());
        gencode.gen(".L" + to_string(j2) + ":");
    }
    else if(op == "OR_OP")
    {
        exp1->gencode_node(st);
        string rtop = rstack.back();
        gencode.gen("\tcmpl\t$0, "+rtop);
        int j1 = gencode.jump();
        gencode.gen("\tjne\t.L" + to_string(j1));
        exp2->gencode_node(st);
        gencode.gen("\tcmpl\t$0, "+rstack.back());
        int j2 = gencode.jump();
        gencode.gen("\tje\t.L" + to_string(j2));
        gencode.gen(".L" + to_string(j1) + ":");
        gencode.gen("\tmovl\t$1, " + rstack.back());
        int j3 = gencode.jump();
        gencode.gen("\tjmp\t.L" + to_string(j3));
        gencode.gen(".L" + to_string(j2) + ":");
        gencode.gen("\tmovl\t$0, " + rstack.back());
        gencode.gen(".L" + to_string(j3) + ":");
    }
    else if(op == "EQ_OP_INT")
    {
        
        if(l<TOTAL_REG  && r<TOTAL_REG)
        {   
            if (r<=l)
            {
                exp1->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp2->gencode_node(st);

                gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
                gencode.gen("\tsete\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
            }
            else 
            {
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
                exp2->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp1->gencode_node(st);
                gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
                gencode.gen("\tsete\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]); 
            }
            
        }
        else if(r >= TOTAL_REG && l>= TOTAL_REG)
        {
            exp2->gencode_node(st);
            gencode.gen("\tpushl\t"+rstack.back());
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t(%esp), " + rstack.back());
            gencode.gen("\tsete\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rstack.back());                    
            gencode.gen("\taddl\t$4, %esp");
        }
        else if(l < r)
        {
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
            exp2->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
            gencode.gen("\tsete\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        }
        else if(r < l)
        {
            exp1->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp2->gencode_node(st);
            gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
            gencode.gen("\tsete\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
        }
    }
    else if(op == "NE_OP_INT")
    {

        if(l<TOTAL_REG  && r<TOTAL_REG)
        {   
            if (r<=l)
            {
                exp1->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp2->gencode_node(st);

                gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
                gencode.gen("\tsetne\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
            }
            else 
            {
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
                exp2->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp1->gencode_node(st);
                gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
                gencode.gen("\tsetne\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]); 
            }
            
        }
        else if(r >= TOTAL_REG && l>= TOTAL_REG)
        {
            exp2->gencode_node(st);
            gencode.gen("\tpushl\t"+rstack.back());
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t(%esp), " + rstack.back());
            gencode.gen("\tsetne\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rstack.back());                    
            gencode.gen("\taddl\t$4, %esp");
        }
        else if(l < r)
        {
            string alt_top = rstack.back();
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
            exp2->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
            gencode.gen("\tsetne\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + alt_top);   
            rstack.push_back(rtop);
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);

        }
        else if(r < l)
        {
            exp1->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp2->gencode_node(st);
            gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
            gencode.gen("\tsetne\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
        }
    }
    else if(op == "LT_OP_INT")
    {
        if(l<TOTAL_REG  && r<TOTAL_REG)
        {   
            if (r<=l)
            {
                // cout<<"inside lt"<<endl;
                exp1->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp2->gencode_node(st);

                gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
                gencode.gen("\tsetl\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
            }
            else 
            {
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
                exp2->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp1->gencode_node(st);
                gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
                gencode.gen("\tsetl\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]); 
            }
            
        }
        else if(r >= TOTAL_REG && l>= TOTAL_REG)
        {
            exp2->gencode_node(st);
            gencode.gen("\tpushl\t"+rstack.back());
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t(%esp), " + rstack.back());
            gencode.gen("\tsetl\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rstack.back());                    
            gencode.gen("\taddl\t$4, %esp");
        }
        else if(l < r)
        {
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
            exp2->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
            gencode.gen("\tsetl\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        }
        else if(r < l)
        {
            exp1->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp2->gencode_node(st);
            gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
            gencode.gen("\tsetl\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
        }
    }
    else if(op == "GT_OP_INT")
    {
        if(l<TOTAL_REG  && r<TOTAL_REG)
        {   
            if (r<=l)
            {
                exp1->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp2->gencode_node(st);

                gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
                gencode.gen("\tsetg\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
            }
            else 
            {
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
                exp2->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp1->gencode_node(st);
                gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
                gencode.gen("\tsetg\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]); 
            }
            
        }
        else if(r >= TOTAL_REG && l>= TOTAL_REG)
        {
            exp2->gencode_node(st);
            gencode.gen("\tpushl\t"+rstack.back());
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t(%esp), " + rstack.back());
            gencode.gen("\tsetg\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rstack.back());                    
            gencode.gen("\taddl\t$4, %esp");
        }
        else if(l < r)
        {
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
            exp2->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
            gencode.gen("\tsetg\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        }
        else if(r < l)
        {
            exp1->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp2->gencode_node(st);
            gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
            gencode.gen("\tsetg\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
        }
    }
    else if(op == "LE_OP_INT")
    {
        if(l<TOTAL_REG  && r<TOTAL_REG)
        {   
            if (r<=l)
            {
                exp1->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp2->gencode_node(st);

                gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
                gencode.gen("\tsetle\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
            }
            else 
            {
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
                exp2->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp1->gencode_node(st);
                gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
                gencode.gen("\tsetle\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]); 
            }
            
        }
        else if(r >= TOTAL_REG && l>= TOTAL_REG)
        {
            exp2->gencode_node(st);
            gencode.gen("\tpushl\t"+rstack.back());
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t(%esp), " + rstack.back());
            gencode.gen("\tsetle\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rstack.back());                    
            gencode.gen("\taddl\t$4, %esp");
        }
        else if(l < r)
        {
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
            exp2->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
            gencode.gen("\tsetle\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        }
        else if(r < l)
        {
            exp1->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp2->gencode_node(st);
            gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
            gencode.gen("\tsetle\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
        }
    }
    else if(op == "GE_OP_INT")
    {
        if(l<TOTAL_REG  && r<TOTAL_REG)
        {   
            if (r<=l)
            {
                exp1->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp2->gencode_node(st);

                gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
                gencode.gen("\tsetge\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
            }
            else 
            {
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
                exp2->gencode_node(st);
                string rtop = rstack.back();
                rstack.pop_back();
                exp1->gencode_node(st);
                gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
                gencode.gen("\tsetge\t%cl");
                gencode.gen("\tmovzbl\t%cl, " + rtop);                    
                rstack.push_back(rtop);
                swap(rstack[rstack.size()-1], rstack[rstack.size()-2]); 
            }
            
        }
        else if(r >= TOTAL_REG && l>= TOTAL_REG)
        {
            exp2->gencode_node(st);
            gencode.gen("\tpushl\t"+rstack.back());
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t(%esp), " + rstack.back());
            gencode.gen("\tsetge\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rstack.back());                    
            gencode.gen("\taddl\t$4, %esp");
        }
        else if(l < r)
        {
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
            exp2->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp1->gencode_node(st);
            gencode.gen("\tcmpl\t" + rtop + ", " + rstack.back());
            gencode.gen("\tsetge\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
            swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
        }
        else if(r < l)
        {
            exp1->gencode_node(st);
            string rtop = rstack.back();
            rstack.pop_back();
            exp2->gencode_node(st);
            gencode.gen("\tcmpl\t" + rstack.back() + ", " + rtop);
            gencode.gen("\tsetge\t%cl");
            gencode.gen("\tmovzbl\t%cl, " + rtop);   
            rstack.push_back(rtop);
        }
    }
}

void op_unary_astnode::gencode_node(SymbTab* st)
{
    if(s == "UMINUS")
    {
        exp->gencode_node(st);
        gencode.gen("\tnegl\t" + rstack.back());
    }
    else if( s == "NOT")
    {
        exp->gencode_node(st);
        gencode.gen("\tcmpl\t$0, " + rstack.back());
        gencode.gen("\tsete\t%cl");
        gencode.gen("\tmovzbl\t%cl, " + rstack.back());
    }
    else if(s == "ADDRESS")
    {
        get_mem_loc_reg(exp, st);
    }
    else if(s == "DEREF")
    {
        exp->gencode_node(st);
        gencode.gen("\tmovl\t("+ rstack.back() + "), " + rstack.back());
    }else if( s == "PP")
    {
        exp->gencode_node(st);
        string rtop = rstack.back();
        rstack.pop_back();
        get_mem_loc_reg(exp, st);
        gencode.gen("\taddl\t$1, " + string("(") + rstack.back() + ")");
        rstack.push_back(rtop);
    }
}

void assignE_astnode::gencode_node(SymbTab* st)
{
    right->gencode_node(st);
    assert(left->astnode_type == e_identifier_astnode);
    // identifier_astnode* tmp = dynamic_cast<identifier_astnode*>(left);
    gencode.gen("\tmovl\t"+rstack.back()+", "+get_mem_loc(left, st));
}

void funcall_astnode::gencode_node(SymbTab* st)
{
    // for(auto elem : all_reg)
    // {
    //     gencode.gen("\tpushl\t"+elem);
    // }
    bool need_swap = false;
    if(rstack.back()!="%eax")
    {
        need_swap = true;
        swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
    }  
    int offset = 4;
    for(int i = params.size()-1; i>=0; i--)
    {
        if(is_array_type(params[i]->type))
        {
            get_mem_loc_reg(params[i], st);
            gencode.gen("\tpushl\t" + rstack.back());
            offset += 4;
        }
        else
        {       
            params[i]->gencode_node(st);
            gencode.gen("\tpushl\t" + rstack.back());
            offset += 4;
        }
    }
    gencode.gen("\tpushl %eax");
    gencode.gen(string("\tcall ")+fname->id);
    gencode.gen("\taddl\t$" + to_string(offset) + ", %esp");
    if(need_swap)
    {
        gencode.gen("\tmovl\t%eax, %ebx");
        swap(rstack[rstack.size()-1], rstack[rstack.size()-2]);
    }
    // reverse(all_reg.begin(), all_reg.end());
    // for(auto elem : all_reg)
    // {
    //     gencode.gen("\tpopl\t"+elem);
    // }
    // reverse(all_reg.begin(), all_reg.end());
}

void intconst_astnode::gencode_node(SymbTab* st)
{
    gencode.gen("\tmovl\t$" + to_string(s) + ", " + rstack.back());

}

void floatconst_astnode::gencode_node(SymbTab* st)
{

}

void stringconst_astnode::gencode_node(SymbTab* st)
  {

  }  
void printcall_astnode::gencode_node(SymbTab* st)
{
    int idx = gencode.push_rodata_str_literal(dynamic_cast<stringconst_astnode*>(params.back())->s);
    int offset = 4; 
    
    for(int i = params.size()-2; i>=0; i--)
    {
        params[i]->gencode_node(st);

        offset += 4;
        gencode.gen("\tpushl\t" + rstack.back());
    }
    gencode.gen(string("\tpushl\t$.LC") + to_string(idx));
    gencode.gen(string("\tcall printf"));
    gencode.gen("\taddl\t$"+ to_string(offset) + ", " + "%esp");
}