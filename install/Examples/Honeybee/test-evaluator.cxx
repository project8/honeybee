#include <iostream>
#include <string>
#include <honeybee/evaluator.hh>


int main(int argc, char** argv)
{
    if (argc <= 2) {
        std::cerr << "USAGE: " << argv[0] << " EXP VALUE" << std::endl;
        std::cerr << "  ex: " << argv[0] << " '2*acos(x)' 0" << std::endl;
        return -1;
    }
    
    try {
        std::string exp = argv[1];
        double x = std::stod(argv[2]);
        honeybee::evaluator f(exp);
        std::cout << exp << ",x=" << x << " --> " << f(x) << std::endl;
    }
    catch (std::exception &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
