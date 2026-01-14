// factory-test.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <tabree/KTreeFile.h>

using namespace std;
using namespace tabree;


// my class //
class TQuantity {
  public:
    TQuantity(const std::string& name);
    void AddUnit(const std::string& symbol);
    void Print(void);
  private:
    std::string fName;
    std::vector<std::string> fUnitList;
};


// define how a tree node is converted into an object of my class //
namespace tabree {
    template<> struct KTreeDecoder<TQuantity*> {
        static TQuantity* As(const KTree& tree) {
            // from a tree node, create an object of my node //
            string name = tree["Name"].As<string>();
            TQuantity* q = new TQuantity(name);
            for (unsigned i = 0; i < tree["Unit"].Length(); i++) {
                string symbol = tree["Unit"][i]["Symbol"].As<string>();
                q->AddUnit(symbol);
            }
            return q;
        }
    };
}



TQuantity::TQuantity(const std::string& name)
{
    fName = name;
}

void TQuantity::AddUnit(const std::string& symbol)
{
    fUnitList.push_back(symbol);
}

void TQuantity::Print(void)
{
    cout << fName << ": ";
    for (unsigned i = 0; i < fUnitList.size(); i++) {
        cout << fUnitList[i] << " ";
    }
    cout << endl;
}



int main(int argc, char** argv)
{
    KTree config;
    try {
        KTreeFile("UnitTable.ktf").Read(config);
    }
    catch (KException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }

    vector<TQuantity*> quantityList;
    for (unsigned i = 0; i < config["Quantity"].Length(); i++) {
        // here "my class" is just like other Tree scalar types //
        TQuantity* q = config["Quantity"][i].As<TQuantity*>();
        quantityList.push_back(q);
    }

    for (unsigned i = 0; i < quantityList.size(); i++) {
        quantityList[i]->Print();
        delete quantityList[i];
    }

    return 0;
}
