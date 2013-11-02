/* 
 * File:   Rule.h
 * Author: yzf
 *
 * Created on July 30, 2013, 7:59 AM
 */

#ifndef RULE_H
#define	RULE_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include "structs.h"

using namespace std;
/*
 * 规则类
 */
class Rule {
public:
    int head;
    int body_length;
    set<int> body_lits;
    RULE_TYPE type;
public:
    Rule(_rule* rule);
    Rule(const Rule& _rhs);
    ~Rule();
    Rule& operator = (const Rule& _rhs);
    void output(FILE* _out) const;
};

#endif	/* RULE_H */

