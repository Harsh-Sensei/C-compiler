#ifndef __UTILS_HH__
#define __UTILS_HH__

SymbTab gst, gstfun, gststruct; 
string filename;
SymbTab* localst;
int offset = 0;
std::string type = "";
std::string basictype = "";
std::string ret_type = "";
std::vector<std::pair<std::string, int> > params;
std::map<string,abstract_astnode*> ast;
std::vector<std::string> predefined = {"printf","scanf"};

std::vector<std::string> all_reg = {"%ebx", "%eax"};
std::vector<std::string> rstack = {"%ebx", "%eax"};
const int TOTAL_REG = 2;

Gencode gencode = Gencode();

std::string get_type(exp_astnode* exp1, exp_astnode* exp2)
{
    if(exp1->type.compare("float")==0||exp2->type.compare("float")==0)
    {
        return std::string("FLOAT");
    }

    return std::string("INT");
}

std::string get_deref_type(std::string type)
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

std::string arr_to_ptr(std::string type)
{
	long unsigned int found;
	found = type.find('(');
	if(found != std::string::npos)
	{
		return type;
	}
	found = type.find('[');
	if(found == std::string::npos)
	{
		long unsigned int found2;
		found2 = type.find('*');
		if(found2 == std::string::npos)
		{
			return type;
		}
		type[type.size()-1] = '(';
		type += "*)";
		return type;
	}
	long unsigned int found2;

	found2 = type.find(']');
	type.erase(found+1, found2-found-2);
	type[found] = '(';
	type[found+1] = '*';
	type[found+2] = ')';
	return type;
}

std::string get_ref_type(std::string type)
{
	std::string struct_str("struct");
	std::string str1, str2;

	long unsigned int i = 0;
	while(i<type.size())
	{
		if(type[i] == '[' )
		{
			break;
		}
		str1 += type[i];
		i++;
	}
	if (i == type.size())
	{
		return str1 + "*";
	}
	type.erase(0,i);
	str2 = type;
	
	return str1 + "(*)" + str2;
}

bool check_compatibility(exp_astnode* &astnode, std::string type)
{
	std::string given_type = astnode->type;
	type = arr_to_ptr(type);
	given_type = arr_to_ptr(given_type); 
	
	if(type.compare("int")==0 && given_type.compare("float")==0)
	{
		op_unary_astnode* tmp = new op_unary_astnode();
		tmp->s = "TO_INT";
		tmp->exp = astnode;
		tmp->type = "int";
		astnode = tmp;
		return true;
	}
	if(type.compare("float")==0 && given_type.compare("int")==0)
	{
		op_unary_astnode* tmp = new op_unary_astnode();
		tmp->s = "TO_FLOAT";
		tmp->exp = astnode;
		tmp->type = "float";
		astnode = tmp;
		return true;
	}
	if(type.compare(given_type)==0)
	{
		return true;
	}
	if(type.compare("void(*)")==0 && !get_deref_type(given_type).empty())
	{
		return true;
	}
	if(given_type.compare("void(*)")==0 && !get_deref_type(type).empty())
	{
		return true;
	}
	return false;
}

bool check_compatibility_prime(exp_astnode* &astnode, std::string type, bool t)
{
	long unsigned int found ;
	found = type.find('[');
	if(found != std::string::npos)
	{
		return false;
	}
	std::string given_type = astnode->type;
	type = arr_to_ptr(type);
	given_type = arr_to_ptr(given_type); 
	// cout<<"type : "<<type<<endl;
	// cout<<"given_type : "<<given_type<<endl; 

	if(type.compare("int")==0 && given_type.compare("float")==0)
	{
		op_unary_astnode* tmp = new op_unary_astnode();
		tmp->s = "TO_INT";
		tmp->exp = astnode;
		tmp->type = "int";
		astnode = tmp;
		return true;
	}
	if(type.compare("float")==0 && given_type.compare("int")==0)
	{
		op_unary_astnode* tmp = new op_unary_astnode();
		tmp->s = "TO_FLOAT";
		tmp->exp = astnode;
		tmp->type = "float";
		astnode = tmp;
		return true;
	}
	if(type.compare(given_type)==0)
	{
		return true;
	}
	if(type.compare("void(*)")==0 && !get_deref_type(given_type).empty())
	{
		return true;
	}
	if(given_type.compare("void(*)")==0 && !get_deref_type(type).empty())
	{
		return true;
	}
	return false;
}


#endif