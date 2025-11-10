/*
 * evaluator.hh
 *
 *  Created on: Jun 5, 2025
 *      Author: Sanshiro Enomoto <sanshiro@uw.edu>
 */

#ifndef HONEYBEE_EVALUATOR_HH_
#define HONEYBEE_EVALUATOR_HH_ 1

#include <string>
#include <vector>
#include <map>
#include <kebap/Kebap.h>
#include "ktf_script.hh" // full KTFScriptContext type

// helpers for script-level parsing/execution
// Ensure these prototypes are visible to other translation units 
bool parse_script(honeybee::KTFScriptContext& ctx);
bool execute_script_call(honeybee::KTFScriptContext& ctx, const std::string& call_expression, const std::vector<double>& args, double& out_value);

namespace kebap {

    class KPHoneybeeObject: public KPObjectPrototype {
      public:
        KPHoneybeeObject(): KPObjectPrototype("Honeybee") {}
        ~KPHoneybeeObject() override {}
        KPObjectPrototype* Clone() override { return new KPHoneybeeObject(); }
    protected:
        enum {
            MethodId_pt100 = KPObjectPrototype::fNumberOfMethods,
            fNumberOfMethods
        };
        int pt100(std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue);
    public:
        int MethodIdOf(const std::string& MethodName) override {
            if (MethodName == "pt100") {
                return MethodId_pt100;
            }
            return KPObjectPrototype::MethodIdOf(MethodName);
        }
        int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override {
            int Result = 0;
            switch (MethodId) {
            case MethodId_pt100:
                Result = pt100(ArgumentList, ReturnValue);
                break;
            default:
                Result = 0;
            }
            return Result;
        }
    };

}

namespace honeybee {
    class evaluator: public kebap::KPEvaluator {
    public:
        evaluator(const std::string& Expression): kebap::KPEvaluator(Expression) {
            fBuiltinFunctionTable->RegisterStaticObject(new kebap::KPHoneybeeObject());
        }
    };
}

#endif
