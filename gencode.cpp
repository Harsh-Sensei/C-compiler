#include "gencode.hh"

Gencode::Gencode()
{
    this->rodata_count = 0;
}


void Gencode::genforloop()
{

}

void Gencode::print_rodata_str_literals()
{
    if(this->rodata_string_literals.size()!=0)
    {
        cout<<"\t.text"<<endl;
        cout<<"\t.section\t.rodata"<<endl;
    }
    for(auto it = this->rodata_string_literals.begin(); it != this->rodata_string_literals.end(); ++it)
    {
        cout<<".LC"<<it->second<<":"<<endl;
        cout<<"\t.string\t"<<it->first<<endl;
        this->rodata_count++;
    }
    this->rodata_string_literals.clear();
}

void printcode(const vector<string>& vec)
{
    for(auto it = vec.begin(); it != vec.end(); ++it)
    {
        cout<<*it<<endl;
    }
} 

void Gencode::gen(string str)
{
    this->assembly_code.push_back(str);
}

void Gencode::print_assembly()
{
    for(auto it = this->assembly_code.begin(); it != this->assembly_code.end(); ++it)
    {
        cout<<*it<<endl;
    }

    this->assembly_code.clear();
}

int Gencode::push_rodata_str_literal(string str)
{
    this->rodata_string_literals.push_back(make_pair(str, this->rodata_count));
    this->rodata_count++;
    return this->rodata_count - 1;
}

void Gencode::genfuncheader(string name)
{
    this->gen("\t.text");
    this->gen("\t.globl\t"+name);
    this->gen("\t.type\t"+name+", @function");
    this->gen(name+":");
    this->gen("\tpushl\t%ebp");
    this->gen("\tmovl\t%esp, %ebp");
}

int Gencode::jump()
{
    int ret = this->jump_count;
    this->jump_count++;
    this->jump_stack.push_back(ret);

    return ret;
}

int Gencode::jump_label()
{
    int ret = this->jump_stack.back();
    this->jump_stack.pop_back();
    return ret;
}
