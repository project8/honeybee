// demo-evaluator.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <kebap/Kebap.h>

using namespace std;
using namespace kebap;


int main(int argc, char** argv)
{
    KPEvaluator f("A * exp(-x/pi) * sin(2*pi*x) + B");
    f["A"] = 10;
    f["B"] = 3;

    try {
        for (double x = 0; x < 10; x += 0.01) {
            cout << x << '\t' << f(x) << endl;
        }
    }
    catch (KPException &e) {
        cerr << "ERROR: " << e.what() << endl;
        return -1;
    }

    return 0;
}
