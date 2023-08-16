#ifndef __SYMBTAB_HH__
#define __SYMBTAB_HH__ 

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>

enum SCOPE {GLOBAL, LOCAL, PARAM};


class SymbTab;
class SymbTab_entry;

class SymbTab
{
public:
    std::map<std::string, SymbTab_entry*> Entries;
    void print();
};

class SymbTab_entry
{
public:
    std::string name;
    SCOPE scp;
    std::string gtype;
    std::string varfun;
    int sz;
    int offset;
    std::string return_var_type;

    SymbTab* localSBT;

    SymbTab_entry(std::string name, SCOPE scp, std::string gtype, int sz, int offset, std::string return_var_type,SymbTab* localSBT)
    {
        this->name = name;
        this->scp = scp; //Global or local
        this->gtype = gtype; // function or struct
        this->varfun = gtype; 
        this->sz = sz; 
        this->offset = offset;
        this->return_var_type = return_var_type;
        this->localSBT = localSBT;
    }
};

#endif