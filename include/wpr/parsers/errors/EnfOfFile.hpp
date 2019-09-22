#pragma once

#include <string>
#include <ostream>

#include <wpr/details/Describe.hpp>

namespace wpr::error {

/*
    EnfOfFile type
    -- represents an unexpected reach of the end of the file
 */
struct EndOfFile {
    std::string what() const {
        return "End of file reached!";
    }
};




/*
    Operator <<
 */

std::ostream& operator <<(std::ostream& os, EndOfFile) {
    return os << "End of file";
}





/*
    Operator ==
 */

bool operator ==(EndOfFile, EndOfFile) {
    return true;
}

}

namespace wpr {





/*
    Describe
 */

template<>
struct Describe<error::EndOfFile> {
    std::string operator()(error::EndOfFile const&) {
        return "End of file reached!";
    }
};




}