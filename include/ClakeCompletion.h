#ifndef CLAKECOMPLETION_H
#define CLAKECOMPLETION_H

#include <vector>
#include <map>
#include "structs.h"
#include "Rule.h"
#include "Utils.h"

using namespace std;
class ClakeCompletion {
private:
    map< int, vector<int> > ipf_atoms_rules;
    vector<int> no_ipf_atoms;
    vector<int> constrants;
public:
    ClakeCompletion();
    ~ClakeCompletion();
    void convert();
    void test();
    void testCompletion();
    
    vector< set<int> > completion;
    int newVar;
};
#endif