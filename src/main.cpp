/* 
 * File:   main.cpp
 * Author: yzf
 *
 * Created on July 10, 2013, 2:17 PM
 */

#include <cstdlib>
#include <cstdio>
#include <assert.h>
#include "Vocabulary.h"
#include "structs.h"
#include "ClakeCompletion.h"
#include "SATSolver.h"
#include "DependenceGraph.h"
#include "CNFUtils.h"
#include "GLTranslator.h"
#include <set>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <string>
#include <sstream>


using namespace std;

FILE* fout;
extern FILE* yyin;
extern vector<Rule> G_NLP;
extern int yyparse();

void io(const char* iPathName, const char* oPathName) {
    yyin = fopen (iPathName, "r");
    fout = fopen (oPathName, "w+");

    if (! yyin) {
        printf("IO Error: cannot open the input file.\n" );
        assert(0);
    }
    if (! fout) {
        printf("IO Error: cannot open the output file.\n");
        assert(0);
    }
}

int main(int argc, char** argv) {
    
//    if(argc < 3) {
//        io("res/input/sample.in","res/output/sample.out");
//    }
//    else {
//        io(argv[1], argv[2]);
//    }
//    
//    yyparse();
//    fclose(yyin);
    
//    yyin = fopen("res/input/labyin.in", "r");
    yyin = fopen("benchmark/hc/nv50a238.lp", "r");
//    fout = fopen("res/output/assat.out", "w");
    fout = stdout;
    yyparse();
    fclose(yyin);

//    for(int i = 0; i < G_NLP.size(); i++) {
//        G_NLP.at(i).output(stdout);
//    }
//    printf("\n");

    ClakeCompletion::instance().setDlp(G_NLP);
    vector<_formula*> completion = ClakeCompletion::instance().convert();
    vector< set<int> > input = Utils::convertToSATInput(completion);
    
//    SATSolver sat(input, Vocabulary::instance().apSize());
//    sat.invokeSAT();
//    printf("Models: %d\n", sat.models.size());
 //   sat.outputResult();
   
    DependenceGraph dpg(G_NLP);
 //   dpg.operateGraph();
    
 //   dpg.printfLoop();

// ASSAT
    vector< set<int> > AnswerSets;
    GLTranslator glt(G_NLP);
    
    SATSolver sat(Vocabulary::instance().apSize());
    sat.addNewClauses(input);
//    sat.invokeSAT();
//    sat.outputResult();
    int id = 0;
    while(sat.isExistModel()) {
        set<int> model = sat.models.back();
       // printf("%d", sat.models.size());
        fprintf(fout, "Completion model %d ", id++);
//        for(set<int>::iterator it = model.begin(); it != model.end(); it++) {
//            printf("%s ", Vocabulary::instance().getAtom(*it));
//        }
        set<int> comp = glt.getComplementSet(model);
        fprintf(fout, "comp size: %d\n", comp.size());
//        for(set<int>::iterator it = comp.begin(); it != comp.end(); it++) {
//            fprintf(fout, "%s ", Vocabulary::instance().getAtom(*it));
//        }
//        fprintf(fout, "\n");
        if(comp.size() == 0) {
            AnswerSets.push_back(model);
            fprintf(fout, "is Answerset\n");
            break;
        }
        else {
            fprintf(fout, "is't Answerset\n");
            vector<Loop> mls = dpg.findCompMaximal(comp);
            //dpg.findESRules(ml);
            
            for(vector<Loop>::iterator imls = mls.begin(); imls != mls.end();
                    imls++) {
                fprintf(fout, "\nmaximal loop ");
                for(set<int>::iterator ct = imls->loopNodes.begin(); ct != imls->loopNodes.end();
                        ct++) {
                    fprintf(fout, "%s ", Vocabulary::instance().getAtom(*ct));
                }
                fprintf(fout, "\n");
                int newVarNum = dpg.computeLoopFormulas(*imls);
                
                sat.addNewVar(newVarNum);
                for(vector< set<int> >::iterator ilfs = imls->loopFormulas.begin(); 
                        ilfs != imls->loopFormulas.end(); ilfs++) {
//                    printf("\nlits\n");
//                    for(set<int>::iterator dt = lits.begin(); dt != lits.end(); dt++) {
//                        printf("%d ", *dt);
//                    }
                    sat.addNewClause(*ilfs);
                }
            }
        }
        if(sat.badEnd) break;
    }
    
    for(vector< set<int> >::iterator it = AnswerSets.begin(); it != AnswerSets.end();
            it++) {
        fprintf(fout, "Answerset:");
        for(set<int>::iterator sit = it->begin(); sit != it->end(); sit++) {
            fprintf(fout, "%d ", *sit);
        }
        fprintf(fout, "\n");
    }
// use SCC to cancel model
//    vector<int> k = dpg.getESRSizes();
//    
//    for(vector<int>::iterator kit = k.begin(); kit != k.end(); kit++) {
//        vector<Loop> loops = dpg.getLoopWithESRuleSize(*kit);
//        for(vector<Loop>::iterator it = loops.begin(); it != loops.end(); it++) {
//            vector<_formula*> lfs = dpg.computeLoopFormulas(*it);
//
//            for(vector<_formula*>::iterator ilfs = lfs.begin(); ilfs != lfs.end(); ilfs++) {
//                set<int>  lits;
//                Utils::convertCNFformulaToLits(*ilfs, lits);
//                input.push_back(lits);
//            }            
//        }
//
//        SATSolver sats(input, Vocabulary::instance().apSize());
//        sats.invokeSAT();
//        printf("Models: %d\n", sats.models.size());
//    }
//    
    return 0;
}