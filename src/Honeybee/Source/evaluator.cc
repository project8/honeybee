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
    
    f_symbol_table->SetVariable("x", x);
    
    try {
        kebap::KPValue result = f_expression->Evaluate(f_symbol_table);
        return result.AsDouble();
    }
    catch (kebap::KPException& e) {
        throw runtime_error(string("evaluator: ") + e.what());
    }
}

