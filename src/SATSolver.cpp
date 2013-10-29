/* 
 * File:   SATSolver.cpp
 * Author: sharpen
 * 
 * Created on September 4, 2013, 9:04 PM
 */

#include "SATSolver.h"
#include "Vocabulary.h"

SATSolver::SATSolver(int num_lits): init_num(num_lits), num_lits_in_NLP(num_lits) {
    badEnd = false;
    
    while(num_lits > sat.nVars) {
        sat.newVar();
    }
}

SATSolver::~SATSolver() {
}

void SATSolver::addNewVar(int num) {
    num_lits_in_NLP += num;
    
    while(num-- > 0) {
        sat.newVar();
    }
}

int SATSolver::invokeSAT() {
    int i = 0;
    while(isExistModel() && !badEnd)
        i++;
    return i;
}

bool SATSolver::addNewClause(set<int>& newClause) {
    int var;
    vec<Lit> lits;

    for(set<int>::iterator l_it = newClause.begin(); l_it != newClause.end(); l_it++) {
        var = abs(*l_it) - 1;
        lits.push((*l_it > 0) ? Lit(var) : ~Lit(var));
    }

    if(lits.size() == 1) {
        if(!sat.addUnit(lits[0])) {
            printf("addUnit failed");
            badEnd = true;
            return false;
        }
    }
    else {
        if(!sat.addClause(lits)) {
            printf("addClause failed");
            badEnd = true;
            return false;
        }
    }
    
    return true;
}

bool SATSolver::addNewClauses(vector< set<int> > newClauses) {
    for(vector< set<int> >::iterator it = newClauses.begin(); it != newClauses.end(); it++) {
        if(!addNewClause(*it)) return false;
    } 
    
    return true;
}

bool SATSolver::isExistModel() {
    bool res;
    set<int> model_set;
    vec<Lit> new_clause;
    
    res = sat.solve();
    
    if(res) {
        for(int i = 0; i < num_lits_in_NLP; i++) {
            if(sat.model[i]) {
                if(i < init_num) model_set.insert(i + 1);
                new_clause.push(~Lit(i));
            }
            else {
                new_clause.push(Lit(i));
            }
        }        
        models.push_back(model_set);
        
//        for(set<int>::iterator it = model_set.begin(); it != model_set.end(); it++) {
//            printf("%d ", *it);
//        }
//        printf("\n");
        if(!sat.addClause(new_clause)) {
            badEnd = true;
        }
        return true;
    }
    else {
        return false;
    }
}

void SATSolver::outputResult() {
    int i = 1;
    
    for(vector< set<int> >::iterator s_it = models.begin(); s_it != models.end();
            s_it++) {
        printf("Model %d:\n", i);
        i++;
        for(set<int>::iterator m_it = s_it->begin(); m_it != s_it->end(); m_it++) {
            printf("%s", Vocabulary::instance().getAtom(*m_it));
        }
        printf("\n");
    }
}