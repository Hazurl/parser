# Introduction

This library implements several parsers, that, combined can create much complex parser.

During all the book, I will use another syntax to describe parsers in order to avoid complex and misleading c++ template syntax. `Parser S E` is the type of parsers that either successfully parse a value of type `S`, or fail with the error `E`. *In c++*, all parsers `P` inherit from `Parser<P, S, E>`. In addition `Parser S` is the type of parser that can't fail. If you see c++ code you can expect type to start with a upper case letter and values with a lower case letter.

Because an example is much better than words, here's a parser for expressions containing signed integer numbers and addition:
```cpp
using namespace wpr;

bool is_digit(char c) { return c >= '0' && c <= '9'; }
int make_signed_number(std::vector<char> sign, int number) {
    return number * (sign.size() % 2 ? 1 : -1);
}

int evaluate(int number, std::vector<int> numbers) {
    return std::accumulate(std::begin(numbers), std::end(numbers), number);
}

// digit :: Parser char (Sum PredicateFailure EndOfFile)
constexpr auto 
    digit 
=   nextc [ filter<is_digit> ];

// number :: Parser int (Sum PredicateFailure EndOfFile)
constexpr auto 
    number 
=   some<digit, std::basic_string> [ map<std::stoi> ];

// signed_number :: Parser int (Sum PredicateFailure EndOfFile)
constexpr auto 
    signed_number 
=   seq< many<ch<'-'>>, number > [ map_apply<make_signed_number> ];

// expression :: Parser int (Sum PredicateFailure EndOfFile)
constexpr auto 
    expression
=   seq< signed_number, many<seq< ch<'+'>, signed_number >>> [ map_apply<evaluate> ]; 



int main() {
    auto res = parse(expression, BoundedReader("42+-8+----45"));
    return res.is_success() ? res.success() : -1;
}
```
Output `79`.