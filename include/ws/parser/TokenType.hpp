#pragma once

namespace ws { namespace parser {

enum class TokenType {
    Parenthesis,
    Operator,
    Literal

};

enum class TokenSubType {
    Left, Right,
    Plus, Minus, Multiplication, Division,
    Float

};

}}