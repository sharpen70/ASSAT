#include "Rule.h"
#include "structs.h"
#include "Vocabulary.h"
#include <assert.h>
#include <cstdlib>

Rule::Rule(_rule* r) : 
        head(r->head), type(r->type), body_length(r->length) {
    for(int i = 0; i < (r->length); i++) {
        body_lits.insert(r->body[i]);
    }
}
Rule::Rule(const Rule& _rhs) : 
        head(_rhs.head),
        type(_rhs.type),
        body_lits(_rhs.body_lits) {
}
Rule::~Rule() {
    positive_literals.clear();
    negative_literals.clear();
}
Rule& Rule::operator = (const Rule& _rhs) {
    head = _rhs.head;
    type = _rhs.type;
    body_length = _rhs.body_length;
    body_lits = _rhs.body_lits;
    return *this;
}

void Rule::output(FILE* _out) const {
    if(head > 0)
        fprintf(_out, "%s", Vocabulary::instance().getAtom(head));
    
    if(type != FACT) {
        fprintf(_out, " :- ");
        for(set<int>::iterator pit = body_lits.begin(); pit != 
                body_lits.end(); pit++) {
            fprintf(_out, "%s", Vocabulary::instance().getAtom(*pit));
            if(pit != (--positive_literals.end())) {
                fprintf(_out, ",");
            }
        }
    }
    
    fprintf(_out, "\n");
}