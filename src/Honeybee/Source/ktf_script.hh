#ifndef HONEYBEE_KTF_SCRIPT_HH_
#define HONEYBEE_KTF_SCRIPT_HH_ 1

#include <string>
#include <map>
#include <memory>

namespace kebap { class KPStandardParser; }

namespace honeybee {
    using namespace std;

    // Per-file KTF script context (combined "#%" lines + parser instance)
    struct KTFScriptContext {
        string filename;
        string combined_script;
        std::unique_ptr<kebap::KPStandardParser> parser;
        bool parsed = false;
    };

    // Single global registry (defined in data_source.cc)
    extern std::map<std::string, KTFScriptContext> g_ktf_script_contexts;
}

#endif
