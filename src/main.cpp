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
#include <ctime>

using namespace std;

FILE* summary;
FILE* unit;
extern FILE* yyin;
extern vector<Rule> G_NLP;
extern int *atomState;
extern int *ruleState;
extern int yyparse();

void io(const char* iPathName, const char* oPathName, const char* sPathName) {
    yyin = fopen(iPathName, "r");
    unit = fopen(oPathName, "w+");
    summary = fopen(sPathName, "a+");
    
    if (! yyin) {
        printf("IO Error: cannot open the input file.\n" );
        assert(0);
    }
    if (!summary || !unit) {
        printf("IO Error: cannot open the output file.\n");
        assert(0);
    }
}

int main(int argc, char** argv) {
    
    if(argc < 3) {
        //    yyin = fopen("res/input/hc2grgai.in", "r");
        yyin = fopen("benchmark/hc/gringo_input/nv50a238.input", "r");
    //    unit = fopen("res/output/assat.out", "w");
        unit = fopen("benchmark/hc/result/nv50a238.input", "w+");
        summary = fopen("benchmark/hc/summary", "a+");
    //    unit = fopen("res/output/sample.out", "w");
    }
    else {
        io(argv[1], argv[2], argv[3]);
    }
    
    clock_t begin, end;
    
    begin = clock();
    
    yyparse();
    fclose(yyin);

    atomState = new int[Vocabulary::instance().apSize() + 1];
    memset(atomState, 0, sizeof(int) * (Vocabulary::instance().apSize() + 1));
    ruleState = new int[G_NLP.size()];
    memset(ruleState, 0, sizeof(int) * G_NLP.size());
    
    SATSolver sat(Vocabulary::instance().apSize());
    
    ClakeCompletion cc;
//    cc.test();
    
    sat.addNewVar(cc.newVar);
    sat.addNewClauses(cc.completion);
    
    bool isTight = false;
    int loadCount = 0;
    int loopFormulaCount = 0;
    
    DependenceGraph dpg;
    vector<Loop> pre = dpg.findSCC();
    
    if(pre.size() == 0) isTight = true;
    
    vector< set<int> > AnswerSets;
    
    if(isTight) {
       sat.invokeSAT();
       AnswerSets = sat.models;
    }
// ASSAT
    else {
        GLTranslator glt(G_NLP);
        while(sat.isExistModel()) {
            set<int> model = sat.models.back();

            fprintf(unit, "Completion model %d size %d ", ++loadCount, model.size());
    //        for(set<int>::iterator it = model.begin(); it != model.end(); it++) {
    //            fprintf(fout, "%s ", Vocabulary::instance().getAtom(*it));
    //        }
            set<int> comp = glt.getComplementSet(model);
            fprintf(unit, "comp size: %d ", comp.size());
    //        for(set<int>::iterator it = comp.begin(); it != comp.end(); it++) {
    //            fprintf(fout, "%s ", Vocabulary::instance().getAtom(*it));
    //        }
    //        fprintf(fout, "\n");
            if(comp.size() == 0) {
                AnswerSets.push_back(model);
                fprintf(unit, "is Answerset\n");
                break;
            }
            else {
                fprintf(unit, "is't Answerset\n");
                vector<Loop> mls = dpg.findCompMaximal(comp);
                //dpg.findESRules(ml);
                
                loopFormulaCount += mls.size();
                
                for(vector<Loop>::iterator imls = mls.begin(); imls != mls.end();
                        imls++) {                   
//                    fprintf(fout, "\nmaximal loop ");
//                    for(set<int>::iterator ct = imls->loopNodes.begin(); ct != imls->loopNodes.end();
//                            ct++) {
//                        fprintf(fout, "%s ", Vocabulary::instance().getAtom(*ct));
//                    }
//                    fprintf(fout, "\n");
                    int newVarNum = dpg.computeLoopFormulas(*imls);

                    sat.addNewVar(newVarNum);
    //                printf("loopFormula size %d", imls->loopFormulas.size());
                    for(vector< set<int> >::iterator ilfs = imls->loopFormulas.begin(); 
                            ilfs != imls->loopFormulas.end(); ilfs++) {
//                        printf("\nlits\n");
//                        for(set<int>::iterator dt = ilfs->begin(); dt != ilfs->end(); dt++) {
//                            printf("%d ", *dt);
//                        }
//                        printf("\n");
                        sat.addNewClause(*ilfs);
                    }
                }
            }
            if(sat.badEnd) break;
        }
    }
//    for(vector< set<int> >::iterator it = AnswerSets.begin(); it != AnswerSets.end();
//            it++) {
//        fprintf(fout, "Answerset:");
//        for(set<int>::iterator sit = it->begin(); sit != it->end(); sit++) {
//            fprintf(fout, "%d ", *sit);
//        }
//        fprintf(fout, "\n");
//    }
    end = clock();
    fprintf(summary, "%s: loading: %d loop formula: %d time: %lf\n", argv[1], loadCount
            , loopFormulaCount, (float)(end - begin)/1000000);
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