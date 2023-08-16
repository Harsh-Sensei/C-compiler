#include "symbtab.hh"
#include <iostream>
void SymbTab::print()
{
    std::string output = "[";
    int i = 0;
    for (const auto &entry : this->Entries)
    {
        
        output += "[\"" + entry.second->name + "\",\t\"" + entry.second->gtype + "\",\t";
    
        if (entry.second->scp == GLOBAL) output += "\"global\",\t";
        else if (entry.second->scp == LOCAL) output += "\"local\",\t";
        else if (entry.second->scp == PARAM) output += "\"param\",\t";

        output += std::to_string(entry.second->sz) + ",\t" + (entry.second->offset == -1?"\"-\"":std::to_string(entry.second->offset)) +",";
        output += "\"" + entry.second->return_var_type + "\""; 
        if (i == this->Entries.size()-1){output += "\n]\n";}
        else {output += "\n],\n";}
        
        i++;
    }
    output += "]";

    std::cout << output << std::endl;

    return;
}
