#ifndef STRUCTURED_EXTENDED_FORMULE_H
#define STRUCTURED_EXTENDED_FORMULE_H

#include <string>
#include <memory>
#include <map>

#include "structures/formula.h"

namespace satsolver {

class ExtendedFormula {
    public: 
        enum Type {
            XOR, OR, AND, IMPLIES, NOT, LITERAL,
            TRUE, FALSE
        };

    private:
        static int last_id; // Used to make unique literals

        enum Type type;

        std::shared_ptr<ExtendedFormula> f1;
        std::shared_ptr<ExtendedFormula> f2;
        std::string literal;
        int id;

        std::string make_literal() const;

    public:

        ExtendedFormula(const ExtendedFormula *that);
        ExtendedFormula(Type type);
        ExtendedFormula(Type type, std::string literal);
        ExtendedFormula(Type type, std::shared_ptr<ExtendedFormula> f);
        ExtendedFormula(Type type, std::shared_ptr<ExtendedFormula> f1, std::shared_ptr<ExtendedFormula> f2);

        std::vector<std::vector<std::string>*>* reduce() const; // Returns a conjunction of disjonctions using Tseitin transformation
        std::shared_ptr<Formula> reduce_to_formula(std::shared_ptr<std::map<std::string, int>> *name_to_variable_ptr) const; // Calls reduce and returns a formula from it.

        std::string to_string() const;
        bool operator==(const ExtendedFormula &that) const;
        bool operator!=(const ExtendedFormula &that) const;
        std::shared_ptr<ExtendedFormula> simplify() const;
};

}

#endif
