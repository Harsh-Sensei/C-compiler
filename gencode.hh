#ifndef __GENCODE_HH__
#define __GENCODE_HH__
#include <iostream>
#include <vector>
#include <string>
#include <utility>

using namespace std;

class Gencode
{
public:
    vector<string> assembly_code;
    vector<pair<string,int> > rodata_string_literals;
    vector<int> jump_stack;
    int rodata_count;
    int jump_count;


    Gencode();
    void printcode(const vector<string>& vec);
    void print_rodata_str_literals();
    void print_assembly();
    void genprintf();
    void genfunc(string funcname, vector<pair<string,int> > locals);
    void genforloop();
    void genfuncheader(string name);
    void gen(string);
    int push_rodata_str_literal(string);
    int jump();
    int jump_label();

};

#endif