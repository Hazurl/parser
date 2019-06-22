#include <iostream>
#include <cctype>
#include <optional>
#include <sstream>
#include <variant>
#include <type_traits>
#include <experimental/type_traits>

#include <module/module.h>
#include <json.hpp>
#include <ws/parser2/ParserInternal.hpp>
#include <ws/parser2/Storage.hpp>
#include <ws/parser2/Details.hpp>

struct StringReader {
    std::string_view str;
    std::size_t cursor{ 0 };

    bool empty() const {
        return cursor >= str.size();
    }

    char get() const {
        return str.at(cursor);
    }

    void next() {
        ++cursor;
    }

    char peek() const {
        return get();
    }

    StringReader copy_with_cursor(std::size_t c) const {
        return {
            str,
            c
        };
    }
};

template<typename R, typename P, std::size_t N>
void test(char const* message, char const (&input)[N], R expected, P) {
    static_assert(ws::parser2::details::is_parser_v<P, StringReader>, "Unfortunetly, P is not a parser...");

    StringReader reader{ std::string_view{ input } };
    auto res = P::parse(reader);

    if (res.is_error()) {
        ws::module::errorln(
            "[", ws::module::style::bold, message, ws::module::style::reset, "]", 
            " with [", ws::module::style::bold, ws::module::colour::fg::cyan, input, ws::module::style::reset, "]", 
            ", expected [", ws::module::style::bold, ws::module::colour::fg::green, expected, ws::module::style::reset, "]",
            " but got [", ws::module::style::bold, ws::module::colour::fg::red, res.what(), ws::module::style::reset, "]");
        return;
    }

    auto& value = res.success(); 
    if (!(value == expected)) {
        ws::module::errorln(
            "[", ws::module::style::bold, message, ws::module::style::reset, "]", 
            " with [", ws::module::style::bold, ws::module::colour::fg::cyan, input, ws::module::style::reset, "]",
            ", expected [", ws::module::style::bold, ws::module::colour::fg::green, expected, ws::module::style::reset, "]",
            " but got [", ws::module::style::bold, ws::module::colour::fg::green, value, ws::module::style::reset, "]");
        return;
    }

    ws::module::successln(
        "[", ws::module::style::bold, message, ws::module::style::reset, "]", 
        " with [", ws::module::style::bold, ws::module::colour::fg::cyan, input, ws::module::style::reset, "] parsed successfully");
}

template<typename E, typename P, std::size_t N>
void test_err(char const* message, char const (&input)[N], E error, P) {
    static_assert(ws::parser2::details::is_parser_v<P, StringReader>, "Unfortunetly, P is not a parser...");

    StringReader reader{ std::string_view{ input } };
    auto res = P::parse(reader);

    if (res.is_success()) {
        ws::module::errorln(
            "[", ws::module::style::bold, message, ws::module::style::reset, "]", 
            " with [", ws::module::style::bold, ws::module::colour::fg::cyan, input, ws::module::style::reset, "]", 
            ", expected [", ws::module::style::bold, ws::module::colour::fg::red, error, ws::module::style::reset, "] but",
            " got [", ws::module::style::bold, ws::module::colour::fg::green, res.success(), ws::module::style::reset, "]");
        return;
    }

    if (!res.template is_error<E>()) {
        ws::module::errorln(
            "[", ws::module::style::bold, message, ws::module::style::reset, "]", 
            " with [", ws::module::style::bold, ws::module::colour::fg::cyan, input, ws::module::style::reset, "]", 
            ", expected [", ws::module::style::bold, ws::module::colour::fg::red, error, ws::module::style::reset, "] but",
            " got [", ws::module::style::bold, ws::module::colour::fg::red, res.what(), ws::module::style::reset, "]");
        return;
    }

    auto& value = res.template error<E>(); 
    if (!(value == error)) {
        ws::module::errorln(
            "[", ws::module::style::bold, message, ws::module::style::reset, "]", 
            " with [", ws::module::style::bold, ws::module::colour::fg::cyan, input, ws::module::style::reset, "]", 
            ", expected [", ws::module::style::bold, ws::module::colour::fg::red, error, ws::module::style::reset, "] but",
            " got [", ws::module::style::bold, ws::module::colour::fg::red, value, ws::module::style::reset, "]");
        return;
    }

    ws::module::successln(
        "[", ws::module::style::bold, message, ws::module::style::reset, "]", 
        " with [", ws::module::style::bold, ws::module::colour::fg::cyan, input, ws::module::style::reset, "] successfully failed");
}

int main() {
    test(
        "Simple Next",
        "something",
        's',
        ws::parser2::nextc
    );

    test_err(
        "Next with EOF",
        "",
        ws::parser2::error::EndOfFile{},
        ws::parser2::nextc
    );

    test(
        "Optional Next",
        "something",
        ws::parser2::Maybe<char>{ 's' },
        ws::parser2::opt<ws::parser2::nextc>
    );

    test(
        "Optional Next with EOF",
        "",
        ws::parser2::Maybe<char>{},
        ws::parser2::opt<ws::parser2::nextc>
    );

    return 0;
}
