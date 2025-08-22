/*
 * evaluator.cc
 *
 *  Created on: Jun 5, 2025
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#include <string>
#include <vector>
#include <map>
//#include <regex> //for preprocessing of tenrary cond
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

// Nobel: idea to convert ternary expressions to boolean logic that Kebap understands for tenrary condition extension
//another option is to change kapap
// static std::string preprocess_ternary(const std::string& expression) {
//     std::string result = expression;
    
//     // Regex to match: condition ? true_value : false_value
//     std::regex ternary_regex(R"((.+?)\s*\?\s*(.+?)\s*:\s*(.+))");
//     std::smatch match;
    
//     if (std::regex_match(result, match, ternary_regex)) {
//         std::string condition = match[1].str();
//         std::string true_val = match[2].str();
//         std::string false_val = match[3].str();
        
//         // Convert to: (condition) && (true_value) || !(condition) && (false_value)
//         result = "(" + condition + ") && (" + true_val + ") || !(" + condition + ") && (" + false_val + ")";
//     }
    
//     return result;
// }

//todo: Put more commenting on it's section for easier understanding 
// Nobel: User-defined function object implementation
kebap::KPUserDefinedFunctionObject::KPUserDefinedFunctionObject(): 
    KPObjectPrototype("UserFunctions"), f_next_method_id(KPObjectPrototype::fNumberOfMethods) 
{
    // std::cerr << "INFO: Initializing UDF object, found " << honeybee::g_user_calibrate_functions.size() << " UDFs" << std::endl;
    // No need to pre-register since we do dynamic lookup
    for (const auto& udf : honeybee::g_user_calibrate_functions) {
        std::cerr << "INFO: Available UDF: " << udf.first << std::endl;
    }
}

int kebap::KPUserDefinedFunctionObject::MethodIdOf(const std::string& MethodName) {
    // Dynamic lookup in the global UDF map instead of using cached map
    auto iter = honeybee::g_user_calibrate_functions.find(MethodName);
    if (iter != honeybee::g_user_calibrate_functions.end()) {
        // Return a consistent ID for this function name (hash-based or simple mapping)
        return std::hash<std::string>{}(MethodName) % 10000 + fNumberOfMethods;
    }
    return KPObjectPrototype::MethodIdOf(MethodName);
}

int kebap::KPUserDefinedFunctionObject::InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) {
    // Dynamic lookup by searching through all UDFs for matching ID
    std::string func_name;
    for (const auto& udf : honeybee::g_user_calibrate_functions) {
        int expected_id = std::hash<std::string>{}(udf.first) % 10000 + fNumberOfMethods;
        if (expected_id == MethodId) {
            func_name = udf.first;
            break;
        }
    }
    
    if (func_name.empty()) {
        return KPObjectPrototype::InvokeMethod(MethodId, ArgumentList, ReturnValue);
    }
    
    auto udf_iter = honeybee::g_user_calibrate_functions.find(func_name);
    if (udf_iter == honeybee::g_user_calibrate_functions.end()) {
        throw kebap::KPException() << "UDF not found: " << func_name;
    }
    
    const honeybee::UserCalibrateFunction& udf = udf_iter->second;
    
    // Validate argument count
    if (ArgumentList.size() != udf.arg_names.size()) {
        throw kebap::KPException() << func_name << "(): expected " << udf.arg_names.size() 
                                   << " arguments, got " << ArgumentList.size();
    }
    
    // Create expression with variable sub
    std::string expression = udf.body_expr;
    // std::cerr << "DEBUG: Starting variable substitution for " << func_name << std::endl;
    // std::cerr << "DEBUG: Original expression: " << expression << std::endl;
    // std::cerr << "DEBUG: Number of arguments: " << ArgumentList.size() << std::endl;
    
    for (size_t i = 0; i < udf.arg_names.size(); i++) {
        double arg_value = ArgumentList[i]->AsDouble();
        // std::cerr << "DEBUG: Arg " << i << " (" << udf.arg_names[i] << ") = " << arg_value << std::endl;
        
        // Replace variable name with actual value 
        std::string var_name = udf.arg_names[i];
        std::string var_value = std::to_string(arg_value);
        // std::cerr << "DEBUG: Replacing '" << var_name << "' with '" << var_value << "'" << std::endl;
        
        // Simple variable substitution (could be improved with proper regex)
        size_t pos = 0;
        while ((pos = expression.find(var_name, pos)) != std::string::npos) {
            // std::cerr << "DEBUG: Found variable at position " << pos << std::endl;
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
                // std::cerr << "DEBUG: Replacing whole word" << std::endl;
                expression.replace(pos, var_name.length(), var_value);
                pos += var_value.length();
            } else {
                // std::cerr << "DEBUG: Skipping partial match" << std::endl;
                pos += var_name.length();
            }
        }
        // std::cerr << "DEBUG: Expression after substitution: " << expression << std::endl;
    }
    
    // Nobel: Idea, actually preprocess ternary operators before Kebap evaluation
    // expression = preprocess_ternary(expression);
    
    // Evaluate it using Kebap
    try {
        // std::cerr << "DEBUG: Evaluating UDF " << func_name << " with expression: " << expression << std::endl;
        kebap::KPEvaluator evaluator(expression);
        double result = evaluator(0.0);  // No x-variable needed since we substituted everything
        // std::cerr << "DEBUG: UDF " << func_name << " result: " << result << std::endl;
        ReturnValue = kebap::KPValue(result);
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: UDF evaluation error in " << func_name << ": " << e.what() << std::endl;
        throw kebap::KPException() << "UDF evaluation error in " << func_name << ": " << e.what();
    }
}
