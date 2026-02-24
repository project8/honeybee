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
        // Register or update the input variable 'x' in the symbol table for expression evaluation
        long x_var_id = f_symbol_table->NameToId("x");
        kebap::KPValue* x_var = f_symbol_table->GetVariable(x_var_id);
        if (!x_var) {
            // If x doesn't exist, register it as a new variable
            x_var_id = f_symbol_table->RegisterVariable("x", kebap::KPValue(x));
            x_var = f_symbol_table->GetVariable(x_var_id);
        } 

        x_var->AssignDouble(x);

        // Evaluate the calibration expression with updated x value and any global variables/constants
        // going from times5(x) -> times5(10)
        kebap::KPValue& result = f_expression->Evaluate(f_symbol_table);
        return result.AsDouble();
    }
    catch (kebap::KPException& e) {
        throw runtime_error(string("evaluator: ") + e.what());
    }
}

