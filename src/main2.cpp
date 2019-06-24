#include <iostream>
#include <cctype>
#include <optional>
#include <sstream>
#include <variant>
#include <type_traits>
#include <experimental/type_traits>

#include <module/module.h>
#include <json.hpp>
#include <ws/parser2/Parsers.hpp>
#include <ws/parser2/Containers.hpp>
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

template<typename...>
struct debug_t;

template<typename R, typename P, std::size_t N>
void test(char const* message, char const (&input)[N], R expected, P) {
    static_assert(ws::parser2::details::is_parser_v<P, StringReader>, "Unfortunetly, P is not a parser...");

    StringReader reader{ std::string_view{ input }, 0 };
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

    StringReader reader{ std::string_view{ input }, 0 };
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

/*

    These are all manual tests to test the parser validator
    Uncomment each of them to see if the assertion fail with the correct message
 */

/*
struct GoodParser : ws::parser2::Parser<GoodParser, char, ws::parser2::error::EndOfFile> {
    template<typename R> static ws::parser2::Result<char, ws::parser2::error::EndOfFile> parse(R reader) { return ws::parser2::success(reader.cursor, '0'); }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, GoodParser>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type should be a Result");
struct ReturnTypeNotResult : ws::parser2::Parser<ReturnTypeNotResult, char, ws::parser2::error::EndOfFile> {
    template<typename R> static char parse(R reader) { return '0'; }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, ReturnTypeNotResult>);
*/
/*
//    static_assert(parser_details::false_v<T>, "The parse method's return type is a Result with the wrong success type");
struct ResultSuccessWrong : ws::parser2::Parser<ResultSuccessWrong, char, ws::parser2::error::EndOfFile> {
    template<typename R> static ws::parser2::Result<int, ws::parser2::error::EndOfFile> parse(R reader) { return ws::parser2::success(reader.cursor, 1337); }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, ResultSuccessWrong>);
*/
/*
//    static_assert(parser_details::false_v<Es...>, "The parse method's return type is a Result with the wrong error list");
struct ResultErrorsWrong : ws::parser2::Parser<ResultErrorsWrong, char, ws::parser2::error::EndOfFile> {
    template<typename R> static ws::parser2::Result<char, ws::parser2::error::EndOfFile, int> parse(R reader) { return ws::parser2::success(reader.cursor, '0'); }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, ResultErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type is a Result with the wrong success type and error list");
struct ResultSuccessAndErrorsWrong : ws::parser2::Parser<ResultSuccessAndErrorsWrong, char, ws::parser2::error::EndOfFile> {
    template<typename R> static ws::parser2::Result<int, ws::parser2::error::EndOfFile, int> parse(R reader) { return ws::parser2::success(reader.cursor, 1337); }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, ResultSuccessAndErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should be static");
struct ParseNotStatic : ws::parser2::Parser<ParseNotStatic, char, ws::parser2::error::EndOfFile> {
    template<typename R> ws::parser2::Result<char, ws::parser2::error::EndOfFile> parse(R reader) { return ws::parser2::success(reader.cursor, '0'); }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, ParseNotStatic>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The parse member is detected but doesn't seems like a static method, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct ParseNotFunction : ws::parser2::Parser<ParseNotFunction, char, ws::parser2::error::EndOfFile> {
    template<typename R> R parse; 
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, ParseNotFunction>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should only have one argument, the reader (taken by value)");
struct ParseWithMoreArg : ws::parser2::Parser<ParseWithMoreArg, char, ws::parser2::error::EndOfFile> {
    template<typename R> static ws::parser2::Result<char, ws::parser2::error::EndOfFile> parse(R reader, int) { return ws::parser2::success(reader.cursor, '0'); }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, ParseWithMoreArg>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method isn't detected, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct Typo : ws::parser2::Parser<Typo, char, ws::parser2::error::EndOfFile> {
    template<typename R> static ws::parser2::Result<char, ws::parser2::error::EndOfFile> parse_(R reader) { return ws::parser2::success(reader.cursor, '0'); }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, Typo>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser, to be more exact, the parser_type alias isn't a parser");
struct ParserTypeNotParser : StringReader {
    using parser_type = StringReader;
    template<typename R> static ws::parser2::Result<char, ws::parser2::error::EndOfFile> parse(R reader) { return ws::parser2::success(reader.cursor, '0'); }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, ParserTypeNotParser>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser");
struct NoBaseParser {
    template<typename R> static ws::parser2::Result<char, ws::parser2::error::EndOfFile> parse(R reader) { return ws::parser2::success(reader.cursor, '0'); }
};
static_assert(ws::parser2::details::is_parser_valid_v<StringReader, NoBaseParser>);
*/

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

    test(
        "Double Next",
        "something",
        ws::parser2::Product('s', 'o'),
        ws::parser2::seq<ws::parser2::nextc, ws::parser2::nextc>
    );

    test_err(
        "Seq, first error",
        "",
        ws::parser2::error::EndOfFile{},
        ws::parser2::seq<ws::parser2::nextc, ws::parser2::nextc>
    );

    test_err(
        "Seq, second error",
        "s",
        ws::parser2::error::EndOfFile{},
        ws::parser2::seq<ws::parser2::nextc, ws::parser2::nextc>
    );

    static_assert(
        ws::parser2::details::is_parser_valid_v<StringReader, ws::parser2::NextC> &&
        ws::parser2::details::is_parser_valid_v<StringReader, ws::parser2::Opt<ws::parser2::NextC>> &&
        ws::parser2::details::is_parser_valid_v<StringReader, ws::parser2::Seq<ws::parser2::NextC, ws::parser2::NextC>> &&
        ws::parser2::details::is_parser_valid_v<StringReader, ws::parser2::Opt<ws::parser2::Opt<ws::parser2::NextC>>> &&
        ws::parser2::details::is_parser_valid_v<StringReader, ws::parser2::Seq<ws::parser2::Seq<ws::parser2::NextC, ws::parser2::NextC>, ws::parser2::NextC>> &&
        ws::parser2::details::is_parser_valid_v<StringReader, ws::parser2::Seq<ws::parser2::Opt<ws::parser2::NextC>, ws::parser2::Seq<ws::parser2::NextC, ws::parser2::NextC>>> &&
        true, 
        "Something is wrong...");

    return 0;
}
