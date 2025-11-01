// SigNum Version Information

#ifndef VERSION_HPP
#define VERSION_HPP

#include <string>

namespace SigNum {
    const std::string VERSION_MAJOR = "1";
    const std::string VERSION_MINOR = "1";
    const std::string VERSION_PATCH = "0";
    const std::string VERSION = VERSION_MAJOR + "." + VERSION_MINOR + "." + VERSION_PATCH;
    
    const std::string VERSION_STAGE = "beta";
    
    inline std::string getVersionString() {
        return "SigNum Interpreter Version " + VERSION + " " + VERSION_STAGE;
    }
    
    inline std::string getShortVersionString() {
        return VERSION + " " + VERSION_STAGE;
    }
}

#endif // VERSION_HPP
