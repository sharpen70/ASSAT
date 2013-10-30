/* 
 * File:   SATSolver.h
 * Author: sharpen
 *
 * Created on September 4, 2013, 9:04 PM
 */

#ifndef SATSOLVER_H
#define	SATSOLVER_H

#include <vector>
#include "Solver.h"

using namespace std;

class SATSolver {
public:
    SATSolver(int num_lits);
    SATSolver(const SATSolver& orig);
    ~SATSolver();
    
//    void invokeSAT();
    int invokeSAT();
    void addNewVar(int num);
    bool isExistModel();
    bool addNewClauses(vector< set<int> >& newClauses);
    bool addNewClause(set<int>& newClause);
    void outputResult();
    vector< set<int> > models;
    
    bool badEnd;
    
private:
    Solver sat; 
    int num_lits_in_NLP; 
    int init_num;
    
};

#endif	/* SATSOLVER_H */

