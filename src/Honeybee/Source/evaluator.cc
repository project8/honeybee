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
#include <sstream>                   
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

// Parse the combined KTF script stored in ctx into a KPStandardParser instance.
// This populates parser state (functions, variables, imports) for reuse.
// Returns true on success, false on parse error.
bool parse_script(honeybee::KTFScriptContext& ctx) {
    try {
        if (!ctx.parser) {
            ctx.parser = std::make_unique<kebap::KPStandardParser>();
        }
        std::istringstream script_stream(ctx.combined_script);
        ctx.parser->Parse(script_stream);
        ctx.parser->Execute();
        ctx.parsed = true;
        return true;
    }
    catch (const kebap::KPException &e) {
        std::cerr << "ERROR: KEBAP parse error: " << e.what() << std::endl;
        // Print a truncated view of the script for debugging
        const size_t max_chars = 4096;
        std::string s = ctx.combined_script;
        if (s.size() > max_chars) {
            std::cerr << "---- combined script (first " << max_chars << " chars) ----" << std::endl;
            std::cerr << s.substr(0, max_chars) << std::endl;
            std::cerr << "---- (truncated, total length " << s.size() << ") ----" << std::endl;
        } else {
            std::cerr << "---- combined script ----" << std::endl;
            std::cerr << s << std::endl;
            std::cerr << "---- end script ----" << std::endl;
        }
        ctx.parsed = false;
        return false;
    }
    catch (const std::exception &e) {
        std::cerr << "ERROR: parse_script exception: " << e.what() << std::endl;
        ctx.parsed = false;
        return false;
    }
}

// Execute a call expression using the per-file KPStandardParser so UDFs, imports
// and variables are resolved, on fails, fall back to evaluator-based strategies.
bool execute_script_call(
    honeybee::KTFScriptContext& ctx, 
    const std::string& call_expression, 
    const std::vector<double>& args, 
    double& out_value) {

    // ensure the script has been parsed into the parser state
    if (!ctx.parsed) {
        if (!parse_script(ctx)) {
            throw kebap::KPException() << "Failed to parse script prior to execution";
        }
    }

    if (!ctx.parser) {
        throw kebap::KPException() << "Parser not initialized in script context";
    }

    // evaluate the call expression directly in the top-level parser's
    // expression parser using the parser's symbol table.
    // parser handles udf, variables and imports without creating
    // temporary evaluators.
    try {
        kebap::KPStandardParser* p = ctx.parser.get();
        std::istringstream is(call_expression);
        kebap::KPTokenizer tokenizer(is, p->GetTokenTable());
        kebap::KPExpression* expr = p->GetExpressionParser()->Parse(&tokenizer, p->GetSymbolTable());
        try {
            kebap::KPValue v = expr->Evaluate(p->GetSymbolTable());
            out_value = v.AsDouble();
            delete expr;
            return true;
        }
        catch (...) {
            delete expr;
            throw;
        }
    }
    catch (const kebap::KPException &e_planA) {
        // If direct parser evaluation fails, fall back to evaluator-based strategies below.
    }

    // Minimal ParserAccessor used only to access the parser's builtin function table
    // for merge-based fallbacks, no nonportable variable readback
    struct ParserAccessor : public kebap::KPStandardParser {
        kebap::KPBuiltinFunctionTable* builtin_table() { return fBuiltinFunctionTable; }
    };

    // Create a temporary parser, parse/execute the combined script into it,
    // merge its builtin function table into a temporary evaluator and evaulate
    try {
        // Build temp parser and parse the combined script
        kebap::KPStandardParser temp_parser;
        {
            std::istringstream s(ctx.combined_script);
            temp_parser.Parse(s);
            temp_parser.Execute();
        }

        // Create evaluator for the call expression and merge function prototypes from temp parser
        struct EvalAccessor : public kebap::KPEvaluator {
            EvalAccessor(const std::string& expr): kebap::KPEvaluator(expr) {}
            kebap::KPBuiltinFunctionTable* builtin_table() { return fBuiltinFunctionTable; }
        };

        ParserAccessor* temp_acc = static_cast<ParserAccessor*>(&temp_parser);
        EvalAccessor eval_merged(call_expression);

        if (temp_acc->builtin_table() && eval_merged.builtin_table()) {
            eval_merged.builtin_table()->Merge(temp_acc->builtin_table());
        }

        // Bind args as arg0, arg1, ...
        for (size_t i = 0; i < args.size(); ++i) {
            std::string name = std::string("arg") + std::to_string(i);
            eval_merged[name] = args[i];
        }

        // Evaluate and return result
        out_value = eval_merged(0.0);
        return true;
    }
    catch (const std::exception &e_main) {
        // If this attempt fails, fall through to existing merge below
    }

    // merging: copy parser-registered functions into
    // the temp evaluator's symbol table so udf are visible at evaluation time. 
    struct EvalAccessor : public kebap::KPEvaluator {
        EvalAccessor(const std::string& expr): kebap::KPEvaluator(expr) {}
        kebap::KPBuiltinFunctionTable* builtin_table() { return fBuiltinFunctionTable; }

        // Import user-defined functions that were registered in parser_sym.
        // only register them to be called
        void import_functions_from_parser(kebap::KPSymbolTable* parser_sym, kebap::KPModule* module) {
            if (!parser_sym || !module) return;
            const std::vector<std::string>& names = module->EntryNameList();
            for (const std::string& name : names) {
                long id = parser_sym->NameToId(name);
                kebap::KPFunction* func = parser_sym->GetFunction(id);
                if (func) {
                    // Register the parser's function pointer into evaluator's symbol table
                    fSymbolTable->RegisterFunction(id, func);
                }
            }
        }
    };

    try {
        ParserAccessor* src = static_cast<ParserAccessor*>(ctx.parser.get());
        EvalAccessor merged_eval(call_expression);

        // Merge builtin table first so builtins are present
        if (src->builtin_table() && merged_eval.builtin_table()) {
            merged_eval.builtin_table()->Merge(src->builtin_table());
        }

        // Import user-defined functions from the parser's module
        merged_eval.import_functions_from_parser(ctx.parser->GetSymbolTable(), ctx.parser->GetModule());

        // Bind args as arg0, arg1, ...
        for (size_t i = 0; i < args.size(); ++i) {
            std::string name = std::string("arg") + std::to_string(i);
            merged_eval[name] = args[i];
        }

        // Evaluate and return result
        out_value = merged_eval(0.0);
        return true;
    }
    catch (...) {
        // Merge fallback failed; continue to below
    }

    // Last resort: evaluate via merged evaluator using the parser's builtin table again.
    try {
        ParserAccessor* src = static_cast<ParserAccessor*>(ctx.parser.get());
        EvalAccessor eval_reader(call_expression);
        if (src->builtin_table() && eval_reader.builtin_table()) {
            eval_reader.builtin_table()->Merge(src->builtin_table());
        }
        for (size_t i = 0; i < args.size(); ++i) {
            std::string name = std::string("arg") + std::to_string(i);
            eval_reader[name] = args[i];
        }
        out_value = eval_reader(0.0);
        return true;
    }
    catch (const std::exception &e_final) {
        // Nothing worked; propagate last error as KPException
        throw kebap::KPException() << "execute_script_call: failed to execute call: " << e_final.what();
    }
}

