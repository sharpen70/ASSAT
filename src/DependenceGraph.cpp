/* 
 * File:   DependenceGraph.cpp
 * Author: sharpen
 * 
 * Created on September 3, 2013, 11:34 PM
 */

#include "DependenceGraph.h"
#include "Utils.h"
#include <cstring>
#include <assert.h>
#include <vector>
#include "Rule.h"
#include <map>
#include "structs.h"
#include "Vocabulary.h"
#include "CNFUtils.h"
#include <string.h>
#include <set>
#include <algorithm>
#include <functional>

extern vector<Rule> G_NLP;
extern int *atomState;
extern int *ruleState;

DependenceGraph::DependenceGraph() {
    set<int> nodeSet;
     
    for(int i = 0; i < G_NLP.size(); i++) {
        Rule r = G_NLP[i];
        if(r.type == RULE) {
            nodeSet.insert(r.head);
            dpdRules[r.head].insert(i);
            for(set<int>::iterator p_it = r.body_lits.begin(); p_it != r.body_lits.end(); p_it++) {
                if(*p_it > 0) {
                    dpdGraph[r.head].insert(*p_it);
                    nodeSet.insert(*p_it);
                }
            }
        }
    }
    
    nodeNumber = nodeSet.size();
    maxNode = *(--nodeSet.end());
    visit = new bool[maxNode + 1];
    memset(visit, false, sizeof(bool) * (maxNode + 1));
    DFN = new int[maxNode + 1];
    memset(DFN, 0, sizeof(int) * (maxNode + 1));
    Low = new int[maxNode + 1];
    memset(Low, 0, sizeof(int) * (maxNode + 1));
    involved = new bool[maxNode + 1];
    memset(involved, true, sizeof(bool) * (maxNode + 1));
}

DependenceGraph::~DependenceGraph() {
    dpdGraph.clear();
    dpdRules.clear();
    delete[] visit;
    delete[] DFN;
    delete[] Low;
}

vector<Loop> DependenceGraph::findSCC() {
    vector<Loop> loops;
    memset(visit, false, sizeof(bool) * (maxNode + 1));

    for(map<int, set<int> >::iterator it = dpdGraph.begin(); it != dpdGraph.end(); it++) {
        if(!visit[it->first] && involved[it->first]) {
            Index = 0;
            tarjan(it->first, loops);
        }
    }
    
    return loops;
}

void DependenceGraph::tarjan(int u, vector<Loop>& loops) {
    DFN[u] = Low[u] = ++Index;
    vs.push(u);
    visit[u] = true;
    for(set<int>::iterator it = dpdGraph[u].begin(); it != dpdGraph[u].end(); it++) {
        if(!involved[*it]) continue;
        
        if(!visit[*it]) {
            tarjan(*it, loops);
            if(Low[u] > Low[*it]) Low[u] = Low[*it];
        }
        else {
            if(Low[u] > DFN[*it]) Low[u] = DFN[*it];
        }
    }
    if(Low[u] == DFN[u]) {
        if(vs.top() != u) {
            Loop l;
            while(vs.top() != u) {                  
                l.loopNodes.insert(vs.top());
                vs.pop();
            }
            l.loopNodes.insert(u);
            vs.pop();
            loops.push_back(l);
        }
        else {
            vs.pop();
        }
    }
}

void DependenceGraph::findESRules(Loop& loop) {
    for(set<int>::iterator it = loop.loopNodes.begin(); it != loop.loopNodes.end();
            it++) {
        map<int, set<int> >::iterator dit = dpdRules.find(*it);
        if(dit != dpdRules.end()) {
            for(set<int>::iterator rit = dit->second.begin(); rit != dit->second.end();
                    rit++) {
                bool in = false;
                for(set<int>::iterator ait = G_NLP[*rit].body_lits.begin(); ait 
                        != G_NLP[*rit].body_lits.end(); ait++) {
                    if(*ait > 0) {
                        if(loop.loopNodes.find(*ait) != loop.loopNodes.end()) 
                            in = true;
                    }
                }
                if(!in) loop.ESRules.insert(*rit);
            }
        }
    }
}

void DependenceGraph::findAllESRules() {
    for(vector<Loop>::iterator it = SCCs.begin(); it != SCCs.end(); it++) {
        findESRules(*it);
    }
}

void DependenceGraph::printfLoop() {
    for(vector<Loop>::iterator it = SCCs.begin(); it != SCCs.end(); it++) {
        for(set<int>::iterator nit = it->loopNodes.begin(); nit != it->loopNodes.end(); nit++) {
            printf("%d %s", *nit, Vocabulary::instance().getAtom(*nit));
        }
        printf("\n");
    }
}

int DependenceGraph::computeLoopFormulas(Loop& loop) {
    int newVar = 0;
    set<int> lf;
        
    for(set<int>::iterator hit = loop.loopNodes.begin(); hit != loop.loopNodes.end();
            hit++) {
        lf.insert(-1 * (*hit));
    }
    
    for(set<int>::iterator rit = loop.ESRules.begin(); rit != loop.ESRules.end();
            rit++) {
        Rule rule = G_NLP[*rit];
        
        if(rule.body_length <= 1) {
            lf.insert(*(rule.body_lits.begin()));          
        }
        else {
            int id = ruleState[*rit];
            if(id == 0) {
                char newAtom[MAX_ATOM_LENGTH];
                sprintf(newAtom, "Rule_%d", *rit);
                id = Vocabulary::instance().addAtom(strdup(newAtom));
                ruleState[*rit] = id;
                
                newVar++;
                set<int> rightEqual;                                          
                rightEqual.insert(-1 * id);

                for(set<int>::iterator eit = G_NLP[*rit].body_lits.begin(); 
                        eit != G_NLP[*rit].body_lits.end(); eit++) {
                    set<int> leftEqual;
                    leftEqual.insert(id);
                    leftEqual.insert(-1 * (*eit));
                    loop.loopFormulas.push_back(leftEqual);
                    rightEqual.insert(*eit);
                }
                
                loop.loopFormulas.push_back(rightEqual);
            }
            lf.insert(id);
        }
    }

    loop.loopFormulas.push_back(lf); 
    
    return newVar;
} 

vector<Loop> DependenceGraph::findCompMaximal(set<int> comp) {
    vector<Loop> maximals;
    
    memset(involved, false, sizeof(bool) * (maxNode + 1));
    for(set<int>::iterator it = comp.begin(); it != comp.end(); it++) {
        involved[*it] = true;
    }
    vector<Loop> sccs = findSCC();
    for(vector<Loop>::iterator it = sccs.begin(); it != sccs.end(); it++) {
        findESRules(*it); 
        maximals.push_back(findLoopMaximal(*it));
    }
    
    return maximals;
 //   return sccs;
}


Loop DependenceGraph::findLoopMaximal(Loop scc) {      
    if(scc.ESRules.size() == 0) return scc;
    
    memset(involved, false, sizeof(bool) * (maxNode + 1));
    
    for(set<int>::iterator it = scc.loopNodes.begin(); it != scc.loopNodes.end(); it++) {
        involved[*it] = true;      
    }  
    
    int tempR = 0;
    
    for(set<int>::iterator it = scc.ESRules.begin(); it != scc.ESRules.end(); it++) {
        int r = G_NLP[*it].head;
        involved[tempR] = true;
        involved[r] = false;
        tempR = r;
        
        vector<Loop> sccs = findSCC();
        if(sccs.size() == 0) continue;  
        else {
            for(vector<Loop>::iterator sit = sccs.begin(); sit != sccs.end(); sit++) {
                findESRules(*sit);
                set<int> moreoverRules;
                for(set<int>::iterator es = sit->ESRules.begin(); es != sit->ESRules.end();
                        es++) {
                    if(scc.ESRules.find(*es) == scc.ESRules.end()) {
                        moreoverRules.insert(*es);
                    }
                }
                if(moreoverRules.size() == 0) {
                    return findLoopMaximal(*sit);
                }
                else {
                    memset(involved, false, sizeof(bool) * (maxNode + 1));
    
                    for(set<int>::iterator lit = sit->loopNodes.begin(); 
                            lit != sit->loopNodes.end(); lit++) {
                        involved[*lit] = true;      
                    } 
                    for(set<int>::iterator mit = moreoverRules.begin(); 
                            mit != moreoverRules.end(); mit++) {
                        involved[G_NLP[*mit].head] = false;      
                    }
                    
                    vector<Loop> mscc = findSCC();
                    if(mscc.size() != 0) {
                        findESRules(mscc.front());
                        return findLoopMaximal(mscc.front());
                    }
                }
            }
        }
    }
   
    return scc;    
}

void DependenceGraph::operateGraph() {
    SCCs = findSCC();     //If use function DependenceGraph::test(), delete this line.
    findAllESRules();
       
    printf("The loops size : %d\n", SCCs.size());
    
    for(vector<Loop>::iterator it = SCCs.begin(); it != SCCs.end(); it++) {
        loopWithESSize[it->ESRules.size()].push_back(*it);
    }
}


vector<Loop> DependenceGraph::getLoopWithESRuleSize(int k) {
    return this->loopWithESSize[k];
}

vector<int> DependenceGraph::getESRSizes() {
    vector<int> result;
    
    for(map<int, vector<Loop> >::iterator it = loopWithESSize.begin();
            it != loopWithESSize.end(); it++) {
        result.push_back(it->first);
    }
    
    sort(result.begin(), result.end());    //default comp is operator < 
    
    return result;
}