// tree-literal-test.cxx //
// Author: Sanshiro Enomoto <sanshiro@uw.edu> //

#include <string>
#include <iostream>
#include <tabree/KTree.h>

using namespace std;
using namespace tabree;


int main(void)
{
    KJsonTreeSerializer(cout).Serialize(
        make_tree("key1", 123)("key2", "hello")("key3", 3.12)
    );

    KKtfTreeSerializer(cout).Serialize(make_tree
        ("title", "constant table")
        ("constants", make_tree
            ("pi", 3.14)
            ("e", 2.77)
        )
    );

    return 0;
}
