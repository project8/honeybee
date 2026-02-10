/*
 * evaluator.cc
 *
 *  Created on: Jun 5, 2025
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#include <string>
#include <vector>
#include <stdexcept>
#include <kebap/Kebap.h>
#include "evaluator.hh"

using namespace std;
using namespace honeybee;


template<typename T> static inline T sqr(const T& x) { return x*x; };
template<typename T> static inline T cub(const T& x) { return x*x*x; };


int kebap::KPHoneybeeObject::pt100(std::vector<KPValue*>& ArgumentList, kebap::KPValue& ReturnValue)
{
    if (ArgumentList.size() != 1) {
        throw kebap::KPException() << "pt100(): invalid number of argument[s]";
    }

    double x = ArgumentList[0]->AsDouble(), y = 0;
    if (x < 8.00) {
        y = 0.0648 * cub(x) - 1.555176 * sqr(x) + 15.01325304 * x - 7.1030334872;
    }
    else if (x < 40.00) {
        y = 7.61e-5 * cub(x) - 0.0073364 * sqr(x) + 2.6227712 * x + 25.9483968;
    }
    else {
        y = -4.61e-6 * cub(x) + 0.0023532 * sqr(x) + 2.233872 * x + 31.17504;
    }
    
    ReturnValue = kebap::KPValue(y);
    return 1;
}


evaluator::evaluator(kebap::KPExpression* a_expression, kebap::KPSymbolTable* a_symbol_table)
    : f_expression(a_expression), f_symbol_table(a_symbol_table)
{
}

evaluator::~evaluator()
{
    delete f_expression;
}

double evaluator::operator()(double x)
{
    if (!f_expression || !f_symbol_table) {
        throw runtime_error("evaluator: expression or symbol table not initialized");
    }
    try {
        // DEBUG: Check if pi is in symbol table
        long pi_id = f_symbol_table->NameToId("pi");
        cerr << "debug [evaluator] for global var" << pi_id << endl;
        if (pi_id >= 0) {
            kebap::KPValue* pi_val = f_symbol_table->GetVariable(pi_id);
            if (pi_val) {
                cerr << "debug [evaluator]: pi found, value=" << pi_val->AsDouble() << endl;
            } else {
                cerr << "debug [evaluator]: pi id exists but returning nullptr" << endl;
            }
        } else {
            cerr << "debug [evaluator]:   pi not in symbol table" << endl;
        }
        
        // Set x variable in the symbol table before evaluating
        // using id for "x" to modify the KPValue object
        long x_var_id = f_symbol_table->NameToId("x");
        kebap::KPValue* x_var = f_symbol_table->GetVariable(x_var_id);
        if (!x_var) {
            // If x doesn't exist, register it
            x_var_id = f_symbol_table->RegisterVariable("x", kebap::KPValue(x));
            x_var = f_symbol_table->GetVariable(x_var_id);
        } 

        x_var->AssignDouble(x); // values can change, hence outside the else

        // Evaluate the expression using the symbol table with updated x
        // now should have times5(x) -> times5(4)
        kebap::KPValue& result = f_expression->Evaluate(f_symbol_table);
        return result.AsDouble();
    }
    catch (kebap::KPException& e) {
        throw runtime_error(string("evaluator: ") + e.what());
    }
}

