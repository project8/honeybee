// argment-list-dump.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <iostream>
#include <cmath>
#include <tabree/KArgumentList.h>

using namespace std;
using namespace tabree;


int main(int argc, char** argv)
{
    KArgumentList args(argc, argv);
    args.Dump();

    return 0;
}
