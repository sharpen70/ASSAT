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

GLTranslator::GLTranslator(vector<Rule> _nlp) :
        nlp(_nlp) {
}

GLTranslator::GLTranslator(const GLTranslator& orig) {
}

GLTranslator::~GLTranslator() {
}

//void GLTranslator::simplifyLP(vector<Rule>& GL_nlp, int fact) {
//    
//}
// don't have tp examine constrant
//bool GLTranslator::satisfyConstrant(Rule cons, set<int> ans) {
//    for(set<int>::iterator pit = cons.positive_literals.begin(); pit != 
//            cons.positive_literals.end(); pit++) {
//        if(ans.find(*pit) == ans.end()) return true;
//    }
//    for(set<int>::iterator nit = cons.negative_literals.begin(); nit != 
//            cons.negative_literals.end(); nit++) {
//        if(ans.find(*nit) != ans.end()) return true;
//    }
//    
//    return false;
//}

set<int> GLTranslator::getComplementSet(set<int> Mset) {
    vector<Rule> _nlp = nlp;
    vector<Rule> GL_nlp;
    vector<int> facts;
    set<int> cons;
    
    //GL_transformation
    for(vector<Rule>::iterator it = _nlp.begin(); it != _nlp.end(); it++) {
        for(set<int>::iterator nit = it->negative_literals.begin();
                nit != it->negative_literals.end(); nit++) {
            if(Mset.find(*nit) == Mset.end()) {
                it->negative_literals.erase(nit);
            }
        }
    }
    
    for(vector<Rule>::iterator it = _nlp.begin(); it != _nlp.end(); it++) {
        if(it->type == FACT) {
            facts.push_back(it->head);
        }
        else {
            if(it->type == RULE && it->negative_literals.size() == 0) {
               GL_nlp.push_back(*it);
            }   
        }
    }  
    
//    printf("GL:\n");
//    for(vector<Rule>::iterator it = GL_nlp.begin(); it != GL_nlp.end(); it++) {
//        it->output(stdout);
//    }
    for(int i = 0; i < facts.size(); i++) {
        cons.insert(facts.at(i));
        for(vector<Rule>::iterator nit = GL_nlp.begin(); nit != GL_nlp.end();) {
            if(facts.at(i) == nit->head) {
                nit = GL_nlp.erase(nit);
                continue;
            }
            for(set<int>::iterator pit = nit->positive_literals.begin();
                    pit != nit->positive_literals.end(); pit++) {
                if(facts.at(i) == *pit) {
                    nit->positive_literals.erase(pit);
                    break;
                }
            }
            if(nit->positive_literals.size() == 0) {
                facts.push_back(nit->head);
                cons.insert(nit->head);
                nit = GL_nlp.erase(nit);
            }
            else {
                nit++;
            }
        }
    }
//    for(set<int>::iterator it = cons.begin(); it != cons.end(); it++) {
//        printf("%s ", Vocabulary::instance().getAtom(*it));
//    }
//    printf("\n");
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
//    for(set<int>::iterator it = Mset.begin(); it != Mset.end(); it++) {
//        printf("%s ", Vocabulary::instance().getAtom(*it));
//       // if(Mset.find(*it) == Mset.end()) return false;
//    }
//    printf("\n");
    
//  cons must be in Mset, don't need to judge
//    for(set<int>::iterator it = cons.begin(); it != cons.end(); it++) {
//        printf("%s ", Vocabulary::instance().getAtom(*it));
//        if(Mset.find(*it) == Mset.end()) return false;
//    }
    
}