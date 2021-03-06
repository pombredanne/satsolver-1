#include <set>
#include <map>
#include <iostream>
#include <cassert>
#include <memory>
#include <iomanip>

#include <exception>
#include <stdexcept>

#include "structures/affectation.h"
#include "structures/deductions.h"
#include "structures/CL_proof.h"
#include "structures/formula.h"
#include "structures/clause.h"
#include "solvers/dpll.h"
#include "solvers/void_assistant.h"
#include "config.h"

#define CL_PROOF_FILE_NAME "/tmp/CL_proof.tex"
#define STEPS_BETWEEN_STATS 5

using namespace satsolver;

#define AS_AFF(a, l) (a.is_true(l) ? l : -l)

unsigned int cl_interac(const Deductions &deductions, const Affectation &aff, int last_bet, int literal, bool *with_proof) {
    char mode;
    unsigned int steps;
    assert(deductions.has_variable(abs(literal)));
    std::cout << "Conflict on literal " << literal << ". Action? [g/r/c/s/t] ";
    assert(with_proof);
    *with_proof = false;
    while (true) {
        std::cin >> mode;
        switch (mode) {
            case 'g':
                deductions.make_conflict_graph(aff, last_bet, literal);
                return 1;
            case 'r':
                if (!WITH_CL) {
                    std::cout << "Clause learning is not enabled. Use -CL. [g/r/c/s/t]. " << std::endl;
                    continue;
                }
                *with_proof = true;
                return 1;
            case 'c':
                return 1;
            case 's':
                std::cin >> steps;
                return steps;
            case 't':
                CL_INTERACT = false;
                return 0;
            case 0:
                exit(0);
            default:
                std::cout << "Invalid character [g/r/c/s/t]. ";
                continue;
        }
    }
}


Affectation* satsolver::solve(std::shared_ptr<Formula> formula) {
    theorysolver::VoidAssistant assistant;
    return satsolver::solve(formula, &assistant);
}

Affectation* satsolver::solve(std::shared_ptr<Formula> formula, theorysolver::AbstractAssistant *assistant) {
    int literal, literal_sav=0;
    Clause learned_clause(formula->get_nb_variables(), std::unordered_set<int>());
    unsigned int nb_learned_clauses=0, nb_learned_literals=0;
    unsigned int steps_before_next_stats_print = STEPS_BETWEEN_STATS;
    bool with_proof;
    CLProof *proof;
    int clause_id, tmp, clause_assistant;
    bool contains_false_clause;
    unsigned int skip_conflicts = 1; // Number of conflicts we will skip before showing another prompt
    int last_bet = 0; // Used for generating the graph.
    unsigned int depth_back ;
    while(formula->get_aff()->get_nb_unknown() != 0 && !formula->only_true_clauses(NULL)) {
        if(!WITH_WL && (literal = formula->monome(&clause_id))) {
            formula->deduce_true(literal, clause_id,&clause_id,NULL,NULL, assistant,&clause_assistant);
            contains_false_clause = formula->contains_false_clause(&clause_id);
        }
        else if(assistant->detect_isolated_literals() && (literal = formula->isolated_literal(&clause_id))) {
            formula->deduce_true(literal,-1,NULL,NULL,NULL, assistant,&clause_assistant) ;
        }
        else {
            literal = formula->choose_literal(HEURISTIC) ;
            if (WITH_WL) {
                contains_false_clause = !formula->bet_true(literal,&clause_id,&tmp,&literal, assistant, &clause_assistant);
            }
            else {
                formula->bet_true(literal,&clause_id,NULL,NULL, assistant, &clause_assistant);
                contains_false_clause = formula->contains_false_clause(&clause_id);
            }
        }
        while(contains_false_clause || !assistant->is_state_consistent()) {
            if(!assistant->is_state_consistent()) {
                // On backtrack au niveau du dernier littéral de la clause apprise (éventuellement aucun backtrack)
                if((depth_back = assistant->get_depth_back())< static_cast<unsigned int>(formula->get_ded_depth())) {
                    formula->back(assistant,depth_back) ;
                    // On est sur que seul literal est unknown dans la clause apprise, tous les autres sont faux
                    if (WITH_WL) {
                        contains_false_clause = !formula->deduce_true(-literal,clause_assistant,&clause_id,&tmp,&literal, assistant, &clause_assistant);
                    }
                    else {
                        formula->deduce_true(-literal, clause_assistant,&clause_id,NULL,NULL, assistant,&clause_assistant);
                        contains_false_clause = formula->contains_false_clause(&clause_id);
                    }
                    continue ; // on n'a peut être plus de clause fausse
                }
                // sinon on fait un backtrack classique
            }
            // On met à jour la deduction "artificiellement" (ça n'impacte que la déduction, pas le reste de la formule)
            // Cette mise à jour sera annulée par le backtrack
            // On sauvegarde avant l'indice de la clause ayant permi de déduire le littéral
            if(formula->get_ded_depth() == 0) { // clause fausse sans aucun paris, donc insatisfiable
                throw Conflict() ;
            }
            if(CL_INTERACT || WITH_CL) {
                if(!WITH_WL) { // formalités administratives un peu moches
                    tmp = formula->get_ded()->get_clause_id(literal) ;
                    formula->get_ded()->remove_deduction(literal);
                }
                formula->get_ded()->add_deduction(literal, formula->to_clauses_vector()[clause_id]->whole_to_set(),clause_id,formula->get_ded_depth());
                literal_sav = literal ;
                clause_id = tmp ;
                if(tmp!=-1 && formula->to_clauses_vector()[clause_id]->get_size() <= 1)
                    throw Conflict() ;
            }
            with_proof = false;
            if (CL_INTERACT && --skip_conflicts == 0) {
                last_bet = formula->last_bet() ;
                assert(last_bet);
                skip_conflicts = cl_interac(*formula->get_ded(), formula->get_aff(), last_bet, literal, &with_proof);
            }
            if (WITH_CL) { // pas de CL si l'état est inconsistent
                if(clause_id == -1 && !assistant->is_state_consistent()) clause_id = clause_assistant ;
                proof = new CLProof();
                literal = formula->learn_clause(proof,&clause_id, &depth_back, literal, &learned_clause);
                nb_learned_clauses++;
                nb_learned_literals += static_cast<unsigned int>(learned_clause.whole_to_set().size());
                if (CL_STATS && !--steps_before_next_stats_print) {
                    steps_before_next_stats_print = STEPS_BETWEEN_STATS;
                    std::cout << "Stats: " << nb_learned_clauses << " clauses learned (";
                    printf("%.2g", static_cast<double>(nb_learned_clauses)*100./static_cast<double>(formula->get_size()));
                    std::cout << "% of all clauses), averaging ";
                    printf("%.3g", static_cast<double>(nb_learned_literals)/static_cast<double>(nb_learned_clauses));
                    std::cout << " literals per clause." << std::endl;
                }
                if (with_proof)
                    proof->to_latex_file(CL_PROOF_FILE_NAME);
                delete proof;
                formula->back(assistant, depth_back);
            }
            else {
                literal = -formula->back(assistant) ;
                clause_id = -1 ;
            }
            if(WITH_WL && (WITH_CL || CL_INTERACT)) { // nettoyage
                formula->get_ded()->remove_deduction(literal_sav) ;
                formula->get_aff()->set_unknown(literal_sav) ;
            }
            if(literal == 0)
                throw Conflict() ;
            if (WITH_WL) {
                contains_false_clause = !formula->deduce_true(literal,clause_id,&clause_id,&tmp,&literal, assistant, &clause_assistant);
            }
            else {
                formula->deduce_true(literal,clause_id,&clause_id,&tmp,&literal, assistant, &clause_assistant);
                contains_false_clause = formula->contains_false_clause(&clause_id);
            }
        }
    }
    return formula->get_aff() ;
}
