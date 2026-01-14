#ifndef KTabreeException_h__
#define KTabreeException_h__ 1

#include <exception>
#include <stdexcept>
#include <sstream>
#include <string>

namespace tabree {

    
class KException:  public std::runtime_error {
  public:
    KException(): std::runtime_error("") {}
    ~KException() noexcept override = default;
    const char* what() const noexcept override {
        return fWhat.c_str();
    }
    template<typename XValue> KException& operator<<(const XValue& message) {
        std::ostringstream os;
        os << message;
        fWhat += os.str();
        return *this;
    }
  protected:
    std::string fWhat;
};


}
#endif
