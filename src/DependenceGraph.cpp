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

DependenceGraph::DependenceGraph(vector<Rule> _nlp) :
                nlp(_nlp) {
    set<int> nodeSet;
     
    for(vector<Rule>::iterator it = _nlp.begin(); it != _nlp.end(); it++) {        
        if(it->type == RULE) {
            nodeSet.insert(it->head);
            for(set<int>::iterator p_it = it->positive_literals.begin(); p_it != it->positive_literals.end(); p_it++) {
                dpdGraph[it->head].insert(*p_it);
                nodeSet.insert(*p_it);
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
    int index = -1;
    
    for(vector<Rule>::iterator r = nlp.begin(); r != nlp.end(); r++) {
        index++;

        if(r->type == RULE) {
            if(loop.loopNodes.find(r->head) != loop.loopNodes.end() && 
                !(Utils::crossSet(r->positive_literals, loop.loopNodes))) {
                loop.ESRules.insert(index); 
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
    _formula* _head = NULL;
    int newVar = 0;
    set<int> lf;
        
    for(set<int>::iterator hit = loop.loopNodes.begin(); hit != loop.loopNodes.end();
            hit++) {
//        if(_head == NULL) _head = Utils::compositeByConnective(NEGA,
//                Utils::compositeToAtom(*hit));
//        else {
//            _formula* newhead = Utils::compositeByConnective(NEGA,
//                Utils::compositeToAtom(*hit));
//            _head = Utils::compositeByConnective(DISJ, _head, newhead);
//        }
        lf.insert(-1 * (*hit));
    }
    _formula* _body = NULL;
    for(set<int>::iterator rit = loop.ESRules.begin(); rit != loop.ESRules.end();
            rit++) {
        Rule rule = nlp.at(*rit);
        
        if(rule.body_length <= 1) {
            if(rule.positive_literals.size() == 0) {               
                lf.insert(-1 * (*(rule.negative_literals.begin())));  
            }
            else {
                lf.insert(*(rule.positive_literals.begin())); 
            }            
        }
        else {
            char newAtom[MAX_ATOM_LENGTH];
            sprintf(newAtom, "Rule_%d", *rit);
            int id = Vocabulary::instance().queryAtom(newAtom);
            if(id < 0) {
                newVar++;
                set<int> rightEqual;            

                id = Vocabulary::instance().addAtom(strdup(newAtom));
                rightEqual.insert(id);

                for(set<int>::iterator eit = nlp.at(*rit).positive_literals.begin(); 
                        eit != nlp.at(*rit).positive_literals.end(); eit++) {
                    set<int> leftEqual;
                    leftEqual.insert(-1 * id);
                    leftEqual.insert(*eit);
                    
                }
                _formula* rule = Utils::convertRuleBodyToFormula(nlp.at(*rit));
                _formula* nega = Utils::compositeByConnective(NEGA, 
                        Utils::copyFormula(rule));
                _formula* l1 = Utils::compositeByConnective(DISJ, nega,
                        Utils::compositeToAtom(id));
                _formula* l2 = Utils::compositeByConnective(DISJ, rule,
                        Utils::compositeByConnective(NEGA, Utils::compositeToAtom(id)));
                Utils::joinFormulas(loop.loopFormulas, CNFUtils::convertCNF(l1));
                Utils::joinFormulas(loop.loopFormulas, CNFUtils::convertCNF(l2));
            }
        }
        if(_body == NULL) {
            _body = Utils::compositeToAtom(id);
        } 
        else {
            _body = Utils::compositeByConnective(DISJ, _body, Utils::compositeToAtom(id));
        }
    }
    _formula* lf;
    if(_body == NULL) lf = _head;
    else lf = Utils::compositeByConnective(DISJ, _head, _body);

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
        int r = nlp.at(*it).head;
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
                        involved[nlp.at(*mit).head] = false;      
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