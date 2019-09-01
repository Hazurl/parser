#include <iostream>
#include <cctype>
#include <optional>
#include <sstream>
#include <variant>
#include <type_traits>
#include <experimental/type_traits>
#include <cassert>

#include <logger/logger.h>
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

    char const* begin{ nullptr };
    char const* end{ nullptr };
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
            ", expected ", ws::module::style::bold, ws::module::colour::fg::green, "[", wsp::describe(expected), "]", ws::module::style::reset,
            " but got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(res), "]", ws::module::style::reset);
        return;
    }

    auto& value = res.success(); 
    if (!(value == expected)) {
        ws::module::errorln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset,
            ", expected ", ws::module::style::bold, ws::module::colour::fg::green, "[", wsp::describe(expected), "]", ws::module::style::reset,
            " but got ", ws::module::style::bold, ws::module::colour::fg::green, "[", wsp::describe(value), "]", ws::module::style::reset);
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
            ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(error), "]", ws::module::style::reset, " but",
            " got ", ws::module::style::bold, ws::module::colour::fg::green, "[", wsp::describe(res.success()), "]", ws::module::style::reset);
        return;
    }

    if constexpr (P::can_fail) {
        auto& value = res.template error(); 

        if constexpr (wspd::is_same_HK_type_v<wspd::error_of_t<wspd::result_type_t<P>>, wsp::Sum>) {
            if (res.template is_error() && !std::holds_alternative<E>(res.error())) {
                ws::module::errorln(
                    ws::module::style::bold, "[", message, "]", ws::module::style::reset, "]", 
                    " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
                    ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(error), "]", ws::module::style::reset, " but",
                    " got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(res), "]", ws::module::style::reset);
                return;
            }

            if (!(std::get<E>(value) == error)) {
                ws::module::errorln(
                    ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
                    " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
                    ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(error), "]", ws::module::style::reset, " but",
                    " got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(value), "]", ws::module::style::reset);
                return;
            }
        } else {
            if (!(value == error)) {
                ws::module::errorln(
                    ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
                    " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, 
                    ", expected ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(error), "]", ws::module::style::reset, " but",
                    " got ", ws::module::style::bold, ws::module::colour::fg::red, "[", wsp::describe(value), "]", ws::module::style::reset);
                return;
            }
        }


        ws::module::successln(
            ws::module::style::bold, "[", message, "]", ws::module::style::reset, 
            " with ", ws::module::style::bold, ws::module::colour::fg::cyan, "[", input, "]", ws::module::style::reset, " successfully failed");
    }
}

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

int char_to_int(char c) { return static_cast<int>(c); }
int digit_to_int(char c) { return c - '0'; }
char default_maybe(wsp::Maybe<char>&& m) { return m.value_or(' '); };
bool is_digit(char c) { return c >= '0' && c <= '9'; }
char EOF_to_space(wspe::EndOfFile) { return ' '; };
std::string select_middle(wsp::Product<char, std::string, char>&& p) {
    return std::get<1>(p);
}
int string_to_int(std::string&& str) {
    int i{ 0 };
    for(auto c : str) {
        i *= 10;
        i += digit_to_int(c);
    }
    return i;
}

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
        "Match maybe 's'",
        "something",
        wsp::Maybe<char>('s'),
        wsp::opt<wsp::ch<'s'>>
    );

    test(
        "Match maybe 's' on EOF",
        "",
        wsp::Maybe<char>(std::nullopt),
        wsp::opt<wsp::ch<'s'>>
    );

    test_err(
        "No Match 's'",
        "a",
        wspe::NotMatching<'s'>{},
        wsp::ch<'s'>
    );

    test(
        "Simple ch",
        "something",
        's',
        wsp::ch<'s'>
    );

    test_err(
        "ch on EOF",
        "",
        wspe::EndOfFile{},
        wsp::ch<'s'>
    );

    test_err(
        "ch not matching",
        "something",
        wspe::NotMatching<'x'>{},
        wsp::ch<'x'>
    );

    test(
        "Match 's' after 'a' failed",
        "something",
        wsp::Sum<char>('s'),
        wsp::first<wsp::ch<'a'>, wsp::ch<'s'>>
    );

    test(
        "Match 's' before 'a' failed",
        "something",
        wsp::Sum<char>('s'),
        wsp::first<wsp::ch<'s'>, wsp::ch<'a'>>
    );

    test(
        "Match 's' before `nextc` works",
        "something",
        wsp::Sum<char>('s'),
        wsp::first<wsp::ch<'s'>, wsp::nextc>
    );

    test_err(
        "No Match 's' and 'a'",
        "foo",
        wsp::Product<wsp::Sum<wspe::NotMatching<'s'>, wspe::EndOfFile>, wsp::Sum<wspe::NotMatching<'a'>, wspe::EndOfFile>>(),
        wsp::first<wsp::ch<'s'>, wsp::ch<'a'>>
    );

    test(
        "Simple Next with transformer",
        "something",
        char_to_int('s'),
        wsp::nextc [ wsp::map<char_to_int> ]
    );

    test_err(
        "Next with transformer on EOF",
        "",
        wspe::EndOfFile{},
        wsp::nextc [ wsp::map<char_to_int> ]
    );

    test(
        "Match 's'",
        "something",
        's',
        wsp::opt<wsp::ch<'s'>> [ wsp::map<default_maybe> ]
    );

    test(
        "many Next",
        "something",
        std::string{ "something" },
        wsp::many<wsp::nextc, std::basic_string>
    );

    test(
        "repeat at least 5 times Next",
        "something",
        std::string{ "something" },
        wsp::repeat<wsp::nextc, 5, wsp::open_maximum, std::basic_string>
    );

    test(
        "repeat between 5 and 6 times Next",
        "something",
        std::string{ "someth" },
        wsp::repeat<wsp::nextc, 5, 6, std::basic_string>
    );

    test(
        "Exactly 3 times Next",
        "something",
        std::string{ "som" },
        wsp::exact<wsp::nextc, 3, std::basic_string>
    );

    test_err(
        "Some Next on EOF",
        "",
        wspe::Expected<wsp::NextC>{},
        wsp::some<wsp::nextc>
    );

    test_err(
        "repeat at least 5 times Next on 4 characters",
        "some",
        wspe::Expected<wsp::NextC>{},
        wsp::repeat<wsp::nextc, 5, wsp::open_maximum, std::basic_string>
    );

    test(
        "Next which satisfy is_digit",
        "123",
        '1',
        wsp::nextc [ wsp::filter<is_digit> ]
    );

    test_err(
        "Next which does not satisfy is_digit",
        "something",
        wspe::PredicateFailure{},
        wsp::nextc [ wsp::filter<is_digit> ]
    );

    test_err(
        "Next with satisfy `is_digit` on EOF",
        "",
        wspe::EndOfFile{},
        wsp::nextc [ wsp::filter<is_digit> ]
    );

    test(
        "Next which satisfy is_digit and transform to an int",
        "123",
        1,
        wsp::nextc [ wsp::filter<is_digit> ] [ wsp::map<digit_to_int> ]
    );

    test(
        "Next on EOF with handle",
        "",
        ' ',
        wsp::nextc [ wsp::handler<EOF_to_space> ]
    );

    test(
        "Next on EOF with default",
        "",
        ' ',
        wsp::nextc [ wsp::or_else<' '> ]
    );

    constexpr auto integer = 
        wsp::seq<
            wsp::ch<'('>, 
            wsp::many<wsp::nextc [ wsp::filter<is_digit> ], std::basic_string>, 
            wsp::ch<')'>
        > [ wsp::peek<1> ] [ wsp::map<string_to_int> ];

    test(
        "parse integer in parenthesis",
        "(123456)",
        123456,
        integer
    );

    test_err(
        "parse integer in parenthesis with no '('",
        "13)",
        wsp::Sum<wspe::NotMatching<'('>, wspe::EndOfFile>{ wspe::NotMatching<'('>{} },
        integer
    );

    test_err(
        "parse integer in parenthesis with no '(' but EOF",
        "",
        wsp::Sum<wspe::NotMatching<'('>, wspe::EndOfFile>{ wspe::EndOfFile{} },
        integer
    );

    test_err(
        "parse integer in parenthesis with no ')'",
        "(13 ",
        wsp::Sum<wspe::NotMatching<')'>, wspe::EndOfFile>{ wspe::NotMatching<')'>{} },
        integer
    );

    test_err(
        "parse integer in parenthesis with no ')' but EOF",
        "(13",
        wsp::Sum<wspe::NotMatching<')'>, wspe::EndOfFile>{ wspe::EndOfFile{} },
        integer
    );

    test(
        "Simple peek",
        "something",
        'm',
        wsp::seq<wsp::nextc, wsp::nextc, wsp::nextc> [ wsp::peek<2> ]
    );

    test(
        "Simple select",
        "something",
        wsp::Product('m', 'o', 'm'),
        wsp::seq<wsp::nextc, wsp::nextc, wsp::nextc> [ wsp::select<2, 1, 2> ]
    );

    test_err(
        "Simple peek_err",
        "",
        wspe::EndOfFile{},
        wsp::first<wsp::nextc, wsp::nextc, wsp::nextc> [ wsp::peek_err<2> ]
    );

    test_err(
        "Simple select_err",
        "",
        wsp::Product(wspe::EndOfFile{}, wspe::EndOfFile{}, wspe::EndOfFile{}),
        wsp::first<wsp::nextc, wsp::nextc> [ wsp::select_err<1, 0, 1> ]
    );

/*
    debug_t<

    > _;
*/

    static_assert(
        wspd::is_parser_valid_v<BoundedReader, wsp::NextC> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Opt<wsp::NextC>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Seq<wsp::NextC, wsp::NextC>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Opt<wsp::Opt<wsp::NextC>>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Seq<wsp::Seq<wsp::NextC, wsp::NextC>, wsp::NextC>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Seq<wsp::Opt<wsp::NextC>, wsp::Seq<wsp::NextC, wsp::NextC>>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::First<decltype(wsp::ch<'a'>), wsp::NextC>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::First<wsp::Opt<wsp::NextC>, wsp::Opt<wsp::NextC>>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Transformer<wsp::NextC, char_to_int>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Satisfy<wsp::NextC, is_digit>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Handle<wsp::NextC, EOF_to_space>> &&
        wspd::is_parser_valid_v<BoundedReader, wsp::Repeat<wsp::NextC, 2, wsp::open_maximum, std::basic_string>> &&
        true, 
        "Something is wrong...");

    return 0;
}
