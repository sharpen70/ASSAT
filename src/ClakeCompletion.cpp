#include "ClakeCompletion.h"
#include "CNFUtils.h"
#include "NNFUtils.h"
#include "Utils.h"
#include <string.h>

extern vector<Rule> G_NLP;
extern int *atomState;
extern int *ruleState;

ClakeCompletion::ClakeCompletion() {
    newVar = 0;
    
    for(int i = 0; i < G_NLP.size(); i++) {
        int h = G_NLP[i].head;
        if(G_NLP[i].type == RULE && atomState[h] != -1) {
            ipf_atoms_rules[h].push_back(i);
            atomState[h] = 1;
        }
        else if(G_NLP[i].type == FACT) {
            ipf_atoms_rules[h].clear();
            ipf_atoms_rules[h].push_back(i);
            atomState[h] = -1;
        }
        else {
            constrants.push_back(i);
        }
    }
    
    for(int i = 1; i <= Vocabulary::instance().apSize(); i++) {
        if(atomState[i] == 0) no_ipf_atoms.push_back(i);
    }  
    
    convert();
}

ClakeCompletion::~ClakeCompletion() {
    no_ipf_atoms.clear();
    constrants.clear();
    ipf_atoms_rules.clear(); 
    delete[] atomState;
}

void ClakeCompletion::convert() {
    for(map<int, vector<int> >::iterator it = ipf_atoms_rules.begin(); it != 
            ipf_atoms_rules.end(); it++) {
        set<int> ltset;
        
        if(atomState[it->first] == -1) {           
            ltset.insert(it->first);
            completion.push_back(ltset);
        }
        else {
            ltset.insert(-1 * (it->first));
            
            for(vector<int>::iterator ait = it->second.begin(); ait != it->second.end();
                ait++) {
                set<int> r_sub_set;
                r_sub_set.insert(it->first);
                
                if(G_NLP[*ait].body_length == 1) {
                    int singleAtom = *(G_NLP[*ait].body_lits.begin());
                    ltset.insert(singleAtom);
                    r_sub_set.insert(-1 * singleAtom);
                    completion.push_back(r_sub_set);
                }          
                else {  
                    int id = ruleState[*ait];
                    if(id == 0) {
                        newVar++;
                        char newAtom[MAX_ATOM_LENGTH];
                        sprintf(newAtom, "Rule_%d", *ait);
                        id = Vocabulary::instance().addAtom(strdup(newAtom));
                        ruleState[*ait] = id;
                        
                        set<int> rightEqual;                                          
                        rightEqual.insert(id);

                        for(set<int>::iterator eit = G_NLP[*ait].body_lits.begin(); 
                                eit != G_NLP[*ait].body_lits.end(); eit++) {
                            set<int> leftEqual;
                            leftEqual.insert(-1 * id);
                            leftEqual.insert(*eit);
                            completion.push_back(leftEqual);
                            rightEqual.insert(-1 * (*eit));
                        }

                        completion.push_back(rightEqual);
                    }
                    ltset.insert(id);
                    r_sub_set.insert(-1 * id);
                    completion.push_back(r_sub_set);                   
                }
            }
            completion.push_back(ltset);
        }
    }      
    
    for(vector<int>::iterator it = no_ipf_atoms.begin(); it != no_ipf_atoms.end();
            it++) {
        set<int> tset;
        tset.insert(-1 * (*it));
        
        completion.push_back(tset);
    }
    
    for(vector<int>::iterator it = constrants.begin(); it != constrants.end(); 
            it++) {
        set<int> cset;
        
        for(set<int>::iterator cit = G_NLP[*it].body_lits.begin(); cit !=
                G_NLP[*it].body_lits.end(); cit++) {
            cset.insert(-1 * (*cit));
        }
        completion.push_back(cset);
    }
}

void ClakeCompletion::testCompletion() {    
    for(vector< set<int> >::iterator it = completion.begin(); it != completion.end(); it++) {
        for(set<int>::iterator s_it = it->begin(); s_it != it->end(); s_it++) {
            printf("%d ", *s_it);
        }
        printf("\n");
    }
}
void ClakeCompletion::test() {
    printf("\nno_ipf_atoms:");
    for(int i = 0; i < no_ipf_atoms.size(); i++) {
        printf("%s ", Vocabulary::instance().getAtom(no_ipf_atoms[i]));
    }
    printf("\nipf_atoms_rules:\n");
    for(map<int, vector<int> >::iterator it = ipf_atoms_rules.begin(); it != ipf_atoms_rules.end(); it++) {
        printf("%s: ", Vocabulary::instance().getAtom(it->first));
        for(int i = 0; i < (it->second).size(); i++) {
            G_NLP[(it->second)[i]].output(stdout);
        }
    }
    printf("\nconstrants\n");
    for(int i = 0; i < constrants.size(); i++) {
        G_NLP[constrants[i]].output(stdout);
    }
    
    testCompletion();
}