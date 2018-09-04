#pragma once

#include <memory>

#include <ws/parser/AST.hpp>

namespace ws::parser {

class UnaryOperator : public AST {
public:

    UnaryOperator(std::string const& name, std::unique_ptr<AST> operand);

    std::string compile(unsigned indentation) const override;

private:

    std::string name;
    std::unique_ptr<AST> operand;

};

}