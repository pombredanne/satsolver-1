#ifndef DPLL_H
#define DPLL_H

#include "structures/affectation.h"
#include "structures/formula.h"

namespace satsolver {

    class Conflict: public std::exception {};

    Affectation* solve(const Formula *formula);

}

#endif
