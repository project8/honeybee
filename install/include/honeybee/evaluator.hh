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

    //Nobel: To register the parsed user-def function as callable functoin using kebap logic
    class KPUserDefinedFunctionObject: public KPObjectPrototype {
    public:
        KPUserDefinedFunctionObject();
        ~KPUserDefinedFunctionObject() override {}
        KPObjectPrototype* Clone() override { return new KPUserDefinedFunctionObject(); }
        int MethodIdOf(const std::string& MethodName) override;
        int InvokeMethod(int MethodId, std::vector<KPValue*>& ArgumentList, KPValue& ReturnValue) override;
    private:
        int f_next_method_id;  // Still used for base calculation
    };
}


namespace honeybee {
    class evaluator: public kebap::KPEvaluator {
    public:
        evaluator(const std::string& Expression): kebap::KPEvaluator(Expression) {
            fBuiltinFunctionTable->RegisterStaticObject(new kebap::KPHoneybeeObject());
            fBuiltinFunctionTable->RegisterStaticObject(new kebap::KPUserDefinedFunctionObject()); // Nobel: register udf with evaluator, globally available to all endpoints
        }
    };
}


#endif
