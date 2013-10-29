#include "Rule.h"
#include "structs.h"
#include "Vocabulary.h"
#include <assert.h>
#include <cstdlib>

Rule::Rule(_rule* r) : 
        head(r->head), type(r->type), body_length(r->length) {
    for(int i = 0; i < (r->length); i++) {
        if(r->body[i] > 0) {
            this->positive_literals.insert(r->body[i]);
        }
        else {
            this->negative_literals.insert(-1 * r->body[i]);
        }
    }
}
Rule::Rule(const Rule& _rhs) : 
        head(_rhs.head),
        type(_rhs.type),
        positive_literals(_rhs.positive_literals),
        negative_literals(_rhs.negative_literals) {
}
Rule::~Rule() {
    positive_literals.clear();
    negative_literals.clear();
}
Rule& Rule::operator = (const Rule& _rhs) {
    head = _rhs.head;
    positive_literals = _rhs.positive_literals;
    negative_literals = _rhs.negative_literals;
    return *this;
}

void Rule::output(FILE* _out) const {
    if(head > 0)
        fprintf(_out, "%s", Vocabulary::instance().getAtom(head));
    
    if(type != FACT) {
        fprintf(_out, " :- ");
        for(set<int>::iterator pit = positive_literals.begin(); pit != 
                positive_literals.end(); pit++) {
            fprintf(_out, "%s", Vocabulary::instance().getAtom(*pit));
            if(pit != (--positive_literals.end())) {
                fprintf(_out, ",");
            }
        }
        for(set<int>::iterator nit = negative_literals.begin(); nit !=
                negative_literals.end(); nit++) {
            if(positive_literals.size() != 0) {
                fprintf(_out, ",");
            }
            fprintf(_out, "not %s", Vocabulary::instance().getAtom(*nit));            
        }
    }
    
    fprintf(_out, "\n");
}