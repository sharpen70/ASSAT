/* 
 * File:   GLTranslator.cpp
 * Author: sharpen
 * 
 * Created on October 13, 2013, 10:28 PM
 */

#include <vector>
#include "GLTranslator.h"
#include "Rule.h"
#include "Vocabulary.h"
#include <queue>

GLTranslator::GLTranslator(vector<Rule> _nlp) :
        nlp(_nlp) {
}

GLTranslator::GLTranslator(const GLTranslator& orig) {
}

GLTranslator::~GLTranslator() {
}

set<int> GLTranslator::getComplementSet(set<int> Mset) {
    vector<Rule> _nlp = nlp;
    set<int> GL_nlp;
    queue<int> facts;
    set<int> cons;
    
    //GL_transformation
    for(vector<Rule>::iterator it = _nlp.begin(); it != _nlp.end(); it++) {
        if(it->type == RULE) {
            for(set<int>::iterator nit = it->body_lits.begin();
                    nit != it->body_lits.end(); nit++) {
                if(*nit > 0) break;

                if(Mset.find(-1 * (*nit)) == Mset.end()) {
                    it->body_lits.erase(nit);
                }            
            }
        }
    }
    
    for(int i = 0; i < _nlp.size(); i++) {
        if(_nlp[i].type == FACT) {
            facts.push(_nlp[i].head);
        }
        else {
            if(_nlp[i].type == RULE) {
                if(*(_nlp[i].body_lits.begin()) > 0) {
                    GL_nlp.insert(i);
                }
            }   
        }
    }
    
    while(!facts.empty()) {
        int fact = facts.front();
        facts.pop();
        cons.insert(fact);
        for(set<int>::iterator nit = GL_nlp.begin(); nit != GL_nlp.end(); nit++) {
            if(fact == _nlp[*nit].head) {
                GL_nlp.erase(nit);
                continue;
            }
            for(set<int>::iterator pit = _nlp[*nit].body_lits.begin();
                    pit != _nlp[*nit].body_lits.end(); pit++) {
                if(fact == *pit) {
                    _nlp[*nit].body_lits.erase(pit);
                    break;
                }
            }
            if(_nlp[*nit].body_lits.size() == 0) {
                facts.push(_nlp[*nit].head);
                cons.insert(_nlp[*nit].head);
                GL_nlp.erase(nit);
            }
        }
    }
    
    set<int> Cset;
    set<int>::iterator cit = cons.begin();
    set<int>::iterator mit = Mset.begin();

    while(mit != Mset.end()) {
        if(*cit != *mit) {
            Cset.insert(*mit);
            mit++;
        }
        else {
            mit++;
            cit++;
        }
    }
    
    return Cset;
}