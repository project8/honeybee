#ifndef KPExcetption_h__
#define KPExcetption_h__ 1

#include <exception>
#include <stdexcept>
#include <sstream>
#include <string>

namespace kebap {

    
class KPException:  public std::runtime_error {
  public:
    KPException(): std::runtime_error("") {
        fWhat = "KEBAP Script Error";
    }
    ~KPException() noexcept override = default;
    const char* what() const noexcept override {
        return fWhat.c_str();
    }
    template<typename XValue> KPException& operator<<(const XValue& message) {
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
