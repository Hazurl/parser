#pragma once

#include <string>
#include <ostream>

namespace ws::parser2::error {

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