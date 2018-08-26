#pragma once

#include <string>

namespace ws { namespace parser {

class AST {
public:

    virtual std::string compile(unsigned indentation) const = 0;

private:

};

}}