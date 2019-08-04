#include <iostream>
#include <cctype>
#include <optional>
#include <sstream>
#include <variant>
#include <type_traits>
#include <experimental/type_traits>
#include <cassert>

#include <module/module.h>
#include <json.hpp>
#include <ws/parser2/Parsers.hpp>
#include <ws/parser2/Containers.hpp>
#include <ws/parser2/Details.hpp>

namespace wsp = ws::parser2;
namespace wspe = wsp::error;
namespace wspd = wsp::details;

struct BoundedReader {

    BoundedReader(char const* begin_, char const* end_) 
        : begin{ begin_                          }, end{ end_               }, index { 0 } {}
    template<std::size_t N>
    BoundedReader(char const (&str)[N]) 
        : begin{ str                             }, end{ str + N - 1        }, index { 0 } {}
    BoundedReader(std::string const& str) 
        : begin{ str.empty() ? nullptr : &str[0] }, end{ begin + str.size() }, index { 0 } {}
    BoundedReader(std::string_view const& str) 
        : begin{ str.empty() ? nullptr : &str[0] }, end{ begin + str.size() }, index { 0 } {}

    static BoundedReader copy_move(BoundedReader r, std::size_t advance) {
        r.index += advance;
        return r;
    }

    static BoundedReader from_cursor(BoundedReader r, std::size_t cursor) {
        r.index = cursor;
        return r;
    }

    std::size_t cursor() const {
        return index;
    }

    bool is_end(std::size_t i = 0) const {
        return begin + index + i >= end;
    }

    char const& operator[](std::size_t i) const {
        assert(!is_end(i));
        return begin[index + i];
    }

    char peek() const {
        return operator[](0);
    }


    BoundedReader& operator++() {
        ++index;
        return *this;
    }

private:

    char const* const begin{ nullptr };
    char const* const end{ nullptr };
    std::size_t index{ 0 };

};
/*
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
};*/

template<typename...>
struct debug_t;

template<typename R, typename P, std::size_t N>
void test(char const* message, char const (&input)[N], R expected, P) {
    static_assert(wspd::is_parser_v<P, BoundedReader>, "Unfortunetly, P is not a parser...");

    BoundedReader reader(input);
    auto res = P::parse(reader);

    if (res.is_error()) {
        ws::module::errorln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
            ", expected ", ws::module::style::bold, ws::module::colour::fg::green, "[", expected, "]", ws::module::style::reset,
            " but got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(res), "]", ws::module::style::reset);
        return;
    }

    auto& value = res.success(); 
    if (!(value == expected)) {
        ws::module::errorln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with [", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset,
            ", expected ", ws::module::style::bold, ws::module::colour::fg::green, "[", expected, "]", ws::module::style::reset,
            " but got ", ws::module::style::bold, ws::module::colour::fg::green, value, "]", ws::module::style::reset);
        return;
    }

    ws::module::successln(
        ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
        " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, " parsed successfully");
}

template<typename E, typename P, std::size_t N>
void test_err(char const* message, char const (&input)[N], E error, P) {
    static_assert(wspd::is_parser_v<P, BoundedReader>, "Unfortunetly, P is not a parser...");

    BoundedReader reader(input);
    auto res = P::parse(reader);

    if (res.is_success()) {
        ws::module::errorln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
            ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", error, "]", ws::module::style::reset, " but",
            " got ", ws::module::style::bold, ws::module::colour::fg::green, "[", res.success(), "]", ws::module::style::reset);
        return;
    }

    if (!res.template is_error<E>()) {
        ws::module::errorln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, "]", 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
            ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", error, "]", ws::module::style::reset, " but",
            " got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(res), "]", ws::module::style::reset);
        return;
    }

    auto& value = res.template error<E>(); 
    if (!(value == error)) {
        ws::module::errorln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
            ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", error, "]", ws::module::style::reset, " but",
            " got ", ws::module::style::bold, ws::module::colour::fg::red, "[", value, "]", ws::module::style::reset);
        return;
    }

    ws::module::successln(
        ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
        " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, " successfully failed");
}

template<char c>
struct NotMatching {};

namespace ws::parser2 {
template<char c>
struct Describe<NotMatching<c>> {
    std::string operator()(NotMatching<c> const&) {
        return "Not matching '" + std::string(1, c) + "' (" + std::to_string(static_cast<int>(c)) + ")";
    }
};
}

template<char c>
std::ostream& operator <<(std::ostream& os, NotMatching<c>) {
    return os << "Not matching '" << c << "' (" << static_cast<int>(c) << ')';
}

template<char c>
bool operator ==(NotMatching<c>, NotMatching<c>) {
    return true;
}


template<char c>
struct Match : wsp::Parser<Match<c>, char, NotMatching<c>, wspe::EndOfFile> {
    template<typename R> 
    static wspd::result_type_t<Match<c>> parse(R reader) {
        auto res = wsp::NextC::parse(reader);
        if (res.is_success()) {
            if (res.success() == c) {
                return wsp::success(res.cursor, res.success());
            }

            return wsp::fail<NotMatching<c>>(res.cursor);
        }

        return wsp::fail<wspe::EndOfFile>(res.cursor);
    }
};


/*

    These are all manual tests to test the parser validator
    Uncomment each of them to see if the assertion fail with the correct message
 */

/*
struct GoodParser : wsp::Parser<GoodParser, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, GoodParser>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type should be a Result");
struct ReturnTypeNotResult : wsp::Parser<ReturnTypeNotResult, char, wspe::EndOfFile> {
    template<typename R> static char parse(R reader) { return '0'; }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, ReturnTypeNotResult>);
*/
/*
//    static_assert(parser_details::false_v<T>, "The parse method's return type is a Result with the wrong success type");
struct ResultSuccessWrong : wsp::Parser<ResultSuccessWrong, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<int, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, 1337); }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, ResultSuccessWrong>);
*/
/*
//    static_assert(parser_details::false_v<Es...>, "The parse method's return type is a Result with the wrong error list");
struct ResultErrorsWrong : wsp::Parser<ResultErrorsWrong, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<char, wspe::EndOfFile, int> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, ResultErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<T, Es...>, "The parse method's return type is a Result with the wrong success type and error list");
struct ResultSuccessAndErrorsWrong : wsp::Parser<ResultSuccessAndErrorsWrong, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<int, wspe::EndOfFile, int> parse(R reader) { return wsp::success(reader.cursor, 1337); }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, ResultSuccessAndErrorsWrong>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should be static");
struct ParseNotStatic : wsp::Parser<ParseNotStatic, char, wspe::EndOfFile> {
    template<typename R> wsp::Result<char, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, ParseNotStatic>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The parse member is detected but doesn't seems like a static method, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct ParseNotFunction : wsp::Parser<ParseNotFunction, char, wspe::EndOfFile> {
    template<typename R> R parse; 
};
static_assert(wspd::is_parser_valid_v<BoundedReader, ParseNotFunction>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method should only have one argument, the reader (taken by value)");
struct ParseWithMoreArg : wsp::Parser<ParseWithMoreArg, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse(R reader, int) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, ParseWithMoreArg>);
*/
/*
//    static_assert(parser_details::false_v<R>, "The parse method isn't detected, makes sure it has the signature: \ntemplate<typename Reader>\nstatic Result<T, Es...> parse(Reader reader)");
struct Typo : wsp::Parser<Typo, char, wspe::EndOfFile> {
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse_(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, Typo>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser, to be more exact, the parser_type alias isn't a parser");
struct ParserTypeNotParser : BoundedReader {
    using parser_type = BoundedReader;
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, ParserTypeNotParser>);
*/
/*
//    static_assert(parser_details::false_v<P>, "The type you wants to validate doesn't inherit from Parser");
struct NoBaseParser {
    template<typename R> static wsp::Result<char, wspe::EndOfFile> parse(R reader) { return wsp::success(reader.cursor, '0'); }
};
static_assert(wspd::is_parser_valid_v<BoundedReader, NoBaseParser>);
*/

int main() {
    test(
        "Simple Next",
        "something",
        's',
        wsp::nextc
    );

    test_err(
        "Next with EOF",
        "",
        wspe::EndOfFile{},
        wsp::nextc
    );

    test(
        "Optional Next",
        "something",
        wsp::Maybe<char>{ 's' },
        wsp::opt<wsp::nextc>
    );

    test(
        "Optional Next with EOF",
        "",
        wsp::Maybe<char>{},
        wsp::opt<wsp::nextc>
    );

    test(
        "Double Next",
        "something",
        wsp::Product('s', 'o'),
        wsp::seq<wsp::nextc, wsp::nextc>
    );

    test_err(
        "Seq, first error",
        "",
        wspe::EndOfFile{},
        wsp::seq<wsp::nextc, wsp::nextc>
    );

    test_err(
        "Seq, second error",
        "s",
        wspe::EndOfFile{},
        wsp::seq<wsp::nextc, wsp::nextc>
    );

    test(
        "Match 's'",
        "something",
        's',
        Match<'s'>{}
    );

    test_err(
        "Match 's' on EOF",
        "",
        wspe::EndOfFile{},
        Match<'s'>{}
    );

    test_err(
        "No Match 's'",
        "a",
        NotMatching<'s'>{},
        Match<'s'>{}
    );

    test(
        "Match 's' after 'a' failed",
        "something",
        wsp::Sum<char>('s'),
        wsp::First<Match<'a'>, Match<'s'>>{}
    );

    test(
        "Match 's' before 'a' failed",
        "something",
        wsp::Sum<char>('s'),
        wsp::First<Match<'s'>, Match<'a'>>{}
    );

    test(
        "Match 's' before `nextc` works",
        "something",
        wsp::Sum<char>('s'),
        wsp::First<Match<'s'>, wsp::NextC>{}
    );

    test_err(
        "No Match 's' and 'a'",
        "foo",
        wsp::Product<wsp::Sum<NotMatching<'s'>, wspe::EndOfFile>, wsp::Sum<NotMatching<'a'>, wspe::EndOfFile>>{},
        wsp::First<Match<'s'>, Match<'a'>>{}
    );

    static_assert(
        wspd::is_parser_valid_v<BoundedReader, wsp::NextC> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Opt<wsp::NextC>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Seq<wsp::NextC, wsp::NextC>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Opt<wsp::Opt<wsp::NextC>>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Seq<wsp::Seq<wsp::NextC, wsp::NextC>, wsp::NextC>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Seq<wsp::Opt<wsp::NextC>, wsp::Seq<wsp::NextC, wsp::NextC>>> &&
        wspd::is_parser_valid_v<BoundedReader, Match<'a'>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::First<Match<'a'>, wsp::NextC>> &&
        true, 
        "Something is wrong...");

    return 0;
}
