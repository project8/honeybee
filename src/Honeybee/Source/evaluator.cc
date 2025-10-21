/*
 * evaluator.cc
 *
 *  Created on: Jun 5, 2025
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#include <string>
#include <vector>
#include <map>
#include <kebap/Kebap.h>
#include "evaluator.hh"
#include "data_source.hh"


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

// Nobel: User-defined function object implementation
kebap::KPUserDefinedFunctionObject::KPUserDefinedFunctionObject(): 
    KPObjectPrototype("UserFunctions"), f_next_method_id(KPObjectPrototype::fNumberOfMethods) 
{
    // No need to pre-register since we do dynamic lookup
    for (const auto& udf : honeybee::g_user_calibrate_functions) {
        std::cerr << "INFO: Available UDF: " << udf.first << std::endl;
    }
}

// Nobel: Convert function name to numeric ID for Kebap's function call
// So you have MethodIdOf(name) -> ID, then InvokeMethod(ID, args) -> result
// later: call functions by ID instead of string lookup every time
int kebap::KPUserDefinedFunctionObject::MethodIdOf(const std::string& MethodName) {
    // Check if this is one of our user-defined functions
    auto iter = honeybee::g_user_calibrate_functions.find(MethodName);
    if (iter != honeybee::g_user_calibrate_functions.end()) {
        // generated ID for this function name (simple hashed)
        return std::hash<std::string>{}(MethodName) % 10000 + fNumberOfMethods;
    }
    // if not our function
    return KPObjectPrototype::MethodIdOf(MethodName);
}

// Nobel: Helper function to find function name by method ID by hash approach
std::string kebap::KPUserDefinedFunctionObject::find_function_by_method_id(int MethodId) {
    for (const auto& udf : honeybee::g_user_calibrate_functions) {
        int expected_id = std::hash<std::string>{}(udf.first) % 10000 + fNumberOfMethods;
        if (expected_id == MethodId) {
            return udf.first;
        }
    }
    return "";
}

// Nobel: Execute simple functions using current variable substitution method
int kebap::KPUserDefinedFunctionObject::execute_simple_function(
    const honeybee::UserCalibrateFunction& udf,
    std::vector<KPValue*>& ArgumentList, 
    KPValue& ReturnValue) {
    
    std::string expression = udf.body_expr;
    
    // Nobel: Replace parameter names with actual values using current approach
    for (int i = 0; i < udf.arg_names.size(); i++) {
        double arg_value = ArgumentList[i]->AsDouble();
        std::string var_name = udf.arg_names[i];
        std::string var_value = std::to_string(arg_value);
        
        // Simple variable substitution with whole word checking
        size_t pos = 0;
        while ((pos = expression.find(var_name, pos)) != std::string::npos) {
            // Check if it's a whole word (not part of another identifier)
            bool is_whole_word = true;
            if (pos > 0 && (std::isalnum(expression[pos-1]) || expression[pos-1] == '_')) {
                is_whole_word = false;
            }
            if (pos + var_name.length() < expression.length() && 
                (std::isalnum(expression[pos + var_name.length()]) || expression[pos + var_name.length()] == '_')) {
                is_whole_word = false;
            }
            
            if (is_whole_word) {
                expression.replace(pos, var_name.length(), var_value);
                pos += var_value.length();
            } else {
                pos += var_name.length();
            }
        }
    }
    
    // Execute using simple evaluator (current approach)
    try {
        kebap::KPEvaluator evaluator(expression);
        double result = evaluator(0.0);
        ReturnValue = kebap::KPValue(result);
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Simple function execution error in " << udf.name << ": " << e.what() << std::endl;
        throw kebap::KPException() << "Simple function execution error in " << udf.name << ": " << e.what();
    }
}

// Nobel: Execute complex function using dedicated Kebap parser instance
int kebap::KPUserDefinedFunctionObject::execute_complex_function(
    const honeybee::UserCalibrateFunction& udf,
    std::vector<KPValue*>& ArgumentList,
    KPValue& ReturnValue) {
    
    // Check if we have the KPFunction instance stored
    if (!udf.f_is_complex_function || !udf.f_kebap_function) {
        throw kebap::KPException() << "Function marked as complex but missing KPFunction";
    }
    
    try {
        // Nobel: Use KPFunction's built-in execution - it handles parameters and return values automatically
        
        // Create a symbol table for execution context
        kebap::KPStandardParser parser;
        kebap::KPSymbolTable* symbol_table = parser.GetSymbolTable();
        
        // Execute the function - KPFunction handles parameter mapping and return values
        KPValue result = udf.f_kebap_function->Execute(ArgumentList, symbol_table);
        
        ReturnValue = result;
        return 1;
        
    } catch (const kebap::KPException& e) {
        std::cerr << "ERROR: Complex function execution error in " << udf.name << ": " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: General execution error in complex function " << udf.name << ": " << e.what() << std::endl;
        throw kebap::KPException() << "Complex function execution error in " << udf.name << ": " << e.what();
    }
}

// Nobel: function execution with dual path for simple vs complex functions
int kebap::KPUserDefinedFunctionObject::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) {
    //Find function by method ID using current hash approach
    std::string func_name = find_function_by_method_id(MethodId);
    
    if (func_name.empty()) {
        return KPObjectPrototype::InvokeMethod(MethodId, ArgumentList, ReturnValue);
    }
    
    // Get function def and its existence
    auto udf_iter = honeybee::g_user_calibrate_functions.find(func_name);
    if (udf_iter == honeybee::g_user_calibrate_functions.end()) {
        std::cerr << "ERROR: UDF not found: " << func_name << std::endl;
        throw kebap::KPException() << "UDF not found: " << func_name;
    }
    
    const honeybee::UserCalibrateFunction& udf = udf_iter->second;
    
    // check argument count matches function signature
    if (ArgumentList.size() != udf.arg_names.size()) {
        std::cerr << "ERROR: Argument count mismatch for " << func_name 
                  << ": expected " << udf.arg_names.size() 
                  << ", got " << ArgumentList.size() << std::endl;
        throw kebap::KPException() << func_name << "(): expected " << udf.arg_names.size() 
                                   << " arguments, got " << ArgumentList.size();
    }
    
    // Step 4: Choose execution path based on function label
    try {
        if (udf.f_is_complex_function) {
            return execute_complex_function(udf, ArgumentList, ReturnValue);
        } else {
            return execute_simple_function(udf, ArgumentList, ReturnValue);
        }
    } catch (const kebap::KPException& e) {
        // Enhanced error handling for parsing, compilation, and runtime issues
        std::cerr << "ERROR: Function execution failed for " << func_name << ": " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: General function error for " << func_name << ": " << e.what() << std::endl;
        throw kebap::KPException() << "Function execution error for " << func_name << ": " << e.what();
    }
}
